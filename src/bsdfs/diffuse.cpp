#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    Diffuse(const Properties &properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        Vector normal{ 0, 0, 1 };
        Color brdf     = m_albedo->evaluate(uv) / Pi;
        float cosTheta = normal.dot(wi) / wi.length();
        return BsdfEval{ brdf * cosTheta };
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        Vector newWi = squareToUniformHemisphere(rng.next2D()).normalized();
        float pdf    = uniformHemispherePdf();
        return BsdfSample{ newWi, m_albedo->evaluate(uv) };
    }

    std::string toString() const override {
        return tfm::format(
            "Diffuse[\n"
            "  albedo = %s\n"
            "]",
            indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
