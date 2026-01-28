#include <algorithm>
#include <cmath>
#include <lightwave.hpp>
#include <vector>

namespace lightwave {
using Vector3 = TVector<float, 3>;

using Vector3 = TVector<float, 3>;
using Vector2 = TVector<float, 2>;

//inline float clamp(float x, float lo, float hi) {
//    return std::min(hi, std::max(lo, x));
//}
inline float clamp01(float x) { return clamp(x, 0.0f, 1.0f); }

inline Vector3 clamp01(const Vector3 &v) {
    return Vector3(clamp01(v.x()), clamp01(v.y()), clamp01(v.z()));
}

inline float mix(float a, float b, float t) { return a + (b - a) * t; }

static inline Vector3 powVec(const Vector3 &v, float p) {
    return Vector3(std::pow(v.x(), p), std::pow(v.y(), p), std::pow(v.z(), p));
}

static inline float luminance(const Vector3 &c) {
    return c.x() * 0.2126f + c.y() * 0.7152f + c.z() * 0.0722f;
}

static inline int clampInt(int v, int lo, int hi) {
    return std::min(hi, std::max(lo, v));
}

// Soft threshold
static inline Vector3 brightPass(const Vector3 &c, float threshold,
                                 float knee) {
    float l       = luminance(c);
    float soft    = clamp((l - threshold + knee) / (2.0f * knee), 0.0f, 1.0f);
    soft          = soft * soft * (3.0f - 2.0f * soft); // smoothstep
    float contrib = std::max(l - threshold, 0.0f) + soft * knee;
    if (l > 1e-6f)
        contrib /= l; // normalize so color direction preserved
    return c * contrib;
}

struct ImageF {
    int w = 0, h = 0;
    std::vector<Vector3> px;

    ImageF() = default;
    ImageF(int W, int H)
        : w(W), h(H), px(size_t(W) * size_t(H), Vector3(0.0f)) {}

    Vector3 &at(int x, int y) { return px[size_t(y) * size_t(w) + size_t(x)]; }
    const Vector3 &at(int x, int y) const {
        return px[size_t(y) * size_t(w) + size_t(x)];
    }
};

// Separable Gaussian blur with clamped edges.
static void gaussianBlurSeparable(const ImageF &src, ImageF &tmp, ImageF &dst,
                                  int radius, float sigma) {
    const float inv2s2 = 1.0f / (2.0f * sigma * sigma);

    // Build 1D kernel
    std::vector<float> k(size_t(radius) * 2 + 1);
    float sumW = 0.0f;
    for (int i = -radius; i <= radius; ++i) {
        float w               = std::exp(-(float(i * i)) * inv2s2);
        k[size_t(i + radius)] = w;
        sumW += w;
    }
    for (float &w : k)
        w /= sumW;

    // Horizontal
    for (int y = 0; y < src.h; ++y) {
        for (int x = 0; x < src.w; ++x) {
            Vector3 acc(0.0f);
            for (int i = -radius; i <= radius; ++i) {
                int xx = clampInt(x + i, 0, src.w - 1);
                acc += src.at(xx, y) * k[size_t(i + radius)];
            }
            tmp.at(x, y) = acc;
        }
    }

    // Vertical
    for (int y = 0; y < src.h; ++y) {
        for (int x = 0; x < src.w; ++x) {
            Vector3 acc(0.0f);
            for (int i = -radius; i <= radius; ++i) {
                int yy = clampInt(y + i, 0, src.h - 1);
                acc += tmp.at(x, yy) * k[size_t(i + radius)];
            }
            dst.at(x, y) = acc;
        }
    }
}

class Bloom : public Postprocess {

private:
    float m_threshold ;  // ~1.0 linear HDR-ish, LDR ~0.7
    float m_knee      ;  // softness around threshold
    float m_intensity ;  // bloom strength
    int   m_radius    ;  // blur radius
    float m_sigma     ;  // blur spread

public:
    Bloom(const Properties &properties) 
        : Postprocess(properties) 
    {
        m_threshold = properties.get<float>("threshold", 1.0f);
        m_knee      = properties.get<float>("knee", 0.5f);
        m_intensity = properties.get<float>("intensity", 5.f);
        m_radius    = properties.get<int>("radius", 20);
        m_sigma     = properties.get<float>("sigma", 10.0f);
    }

    void execute() override {
        if (!m_input || !m_output)
            return;

        m_output->copy(*m_input);

        const Point2i res = m_input->resolution();
        const int W       = res.x();
        const int H       = res.y();

        // Load input into float buffers
        lightwave::ImageF src(W, H);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                const Color &c = m_input->get({ x, y });
                lightwave::Vector3 v(c.r(), c.g(), c.b());
                src.at(x, y) = v;
            }
        }

        // Bright pass
        lightwave::ImageF bright(W, H);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                bright.at(x, y) =
                    lightwave::brightPass(src.at(x, y), m_threshold, m_knee);
            }
        }

        // Blur
        lightwave::ImageF tmp(W, H), blurred(W, H);
        lightwave::gaussianBlurSeparable(bright, tmp, blurred, m_radius, m_sigma);

        // Composite back
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                lightwave::Vector3 color =
                    src.at(x, y) + blurred.at(x, y) * m_intensity;

                color = lightwave::clamp01(color);
                Color &out = m_output->get({ x, y });
                out        = Color(color.x(), color.y(), color.z());
            }
        }

        Streaming stream{ *m_output };
        stream.update();
        m_output->save();
    }

    std::string toString() const override {
        return tfm::format(
            "Bloom[\n"
            "  input = %s,\n"
            "  output = %s,\n"
            "]",
            indent(m_input),
            indent(m_output));
    }
};

} // namespace lightwave

REGISTER_POSTPROCESS(Bloom, "bloom");
