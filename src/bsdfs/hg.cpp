#include <lightwave.hpp>

namespace lightwave {

class HenyeyGreenstein : public Bsdf {
    float m_g;
    Color m_albedo;

public:
    HenyeyGreenstein(const Properties &properties) {
        m_g      = properties.get<float>("g");
        m_albedo = properties.get<Color>("albedo");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // evaluation function term
        float cosTheta = Frame::cosTheta(wi);
        float piTerm   = 4.0f * Pi;
        float denom =
            piTerm * (pow(1.0f + sqr(m_g) + 2.0f * m_g * cosTheta, 1.5f));
        float numerator = 1.0f - sqr(m_g);

        Color weight = (numerator / denom) * m_albedo;
        return BsdfEval{ weight };
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        float phi = 2.0f * Pi * rng.next();
        float x   = rng.next();

        // given term
        float costheta;
        if (std::abs(m_g) < Epsilon) {
            costheta = 1.0f - 2.0f * x;
        } else {
            float sqrTerm = (1.0f - sqr(m_g)) / (1.0f + m_g - 2.0f * m_g * x);
            costheta =
                (-1.0f / (2.0f * m_g)) * (1.0f + sqr(m_g) - sqr(sqrTerm));
        }

        // lecture 4 slide 24
        float sinphi   = sin(phi);
        float cosphi   = cos(phi);
        float sin2     = std::max(0.0f, 1.0f - sqr(costheta));
        float sintheta = std::sqrt(sin2);

        Vector newWi = {
            cosphi * sintheta,
            sinphi * sintheta,
            costheta,
        };
        return BsdfSample{
            newWi,
            m_albedo,
        };
    }

    std::string toString() const override {
        return tfm::format(
            "HenyeyGreenstein[\n"
            "  albedo = %s\n"
            "]",
            indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(HenyeyGreenstein, "hg")
