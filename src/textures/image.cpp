#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

private:
    int handleBorderMode(int x, int w) const {
        if (m_border == BorderMode::Repeat) {
            int t = x % w;
            return t < 0 ? t + w : t;
        }
        return std::clamp(x, 0, w - 1);
    }

    Color bilinear(const Point2 &uv) const {
        int img_w = m_image->resolution().x();
        int img_h = m_image->resolution().y();
        // shift by 0.5 to sample at pixel centers
        Point texel{ uv.x() * img_w - 0.5f, uv.y() * img_h - 0.5f };

        int x0 = int(std::floor(texel.x()));
        int y0 = int(std::floor(texel.y()));
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        Point2i t00{ handleBorderMode(x0, img_w), handleBorderMode(y0, img_h) };
        Point2i t01{ handleBorderMode(x0, img_w), handleBorderMode(y1, img_h) };
        Point2i t10{ handleBorderMode(x1, img_w), handleBorderMode(y0, img_h) };
        Point2i t11{ handleBorderMode(x1, img_w), handleBorderMode(y1, img_h) };

        float tx = texel.x() - std::floor(texel.x());
        float ty = texel.y() - std::floor(texel.y());

        Color c0 = m_image->get(t00);
        Color c1 = m_image->get(t10);
        Color c2 = m_image->get(t01);
        Color c3 = m_image->get(t11);

        Color t0 = (1 - tx) * c0 + tx * c1;
        Color t1 = (1 - tx) * c2 + tx * c3;

        return (1 - ty) * t0 + ty * t1;
    }

    Color nearest(const Point2 &uv) const {
        int img_w = m_image->resolution().x();
        int img_h = m_image->resolution().y();
        Point scaled{ uv.x() * img_w, uv.y() * img_h };

        Point2i pixel{ (handleBorderMode(int(std::floor(scaled.x())), img_w)),
                       (handleBorderMode(int(std::floor(scaled.y())), img_h)) };
        return m_image->get(pixel);
    }

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        // clang-format off
        m_border = properties.getEnum<BorderMode>("border", BorderMode::Repeat, {
            { "clamp", BorderMode::Clamp },
            { "repeat", BorderMode::Repeat },
        });

        m_filter = properties.getEnum<FilterMode>("filter", FilterMode::Bilinear, {
            { "nearest", FilterMode::Nearest },
            { "bilinear", FilterMode::Bilinear },
        });
        // clang-format on
    }

    Color evaluate(const Point2 &uv) const override {
        // flip the v coordinate to match y-axis at image coords
        Point2 newUV{ uv.x(), 1.0f - uv.y() };
        Color result;
        switch (m_filter) {
        case FilterMode::Bilinear:
            result = bilinear(newUV);
            break;
        default:
            result = nearest(newUV);
            break;
        }

        return result * m_exposure;
    }

    std::string toString() const override {
        return tfm::format(
            "ImageTexture[\n"
            "  image = %s,\n"
            "  exposure = %f,\n"
            "]",
            indent(m_image),
            m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
