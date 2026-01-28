#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo, const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo, Sampler &rng) const override {
        Vector normal{ 0, 0, 1 };
        float ior = m_ior->scalar(uv);
        Color reflectance{ m_reflectance->evaluate(uv) };
        float cosTheta = Frame::cosTheta(wo);

        float eta         = cosTheta < 0 ? 1 / ior : ior; // direction of transition
        float fresnelTerm = fresnelDielectric(cosTheta, eta);

        if (rng.next() < fresnelTerm) {
            // reflection
            Vector reflectedWi = reflect(wo, normal);
            return BsdfSample{ reflectedWi, reflectance };
        } else {
            // refraction
            Vector refractedWi = refract(wo, normal, eta);
            Color transmittance{ m_transmittance->evaluate(uv) / (eta * eta) };
            return BsdfSample{ refractedWi.normalized(), transmittance };
        }
    }

    Color getAlbedo(const Point2 &uv) const override { return Color(0); }

    std::string toString() const override {
        return tfm::format(
            "Dielectric[\n"
            "  ior           = %s,\n"
            "  reflectance   = %s,\n"
            "  transmittance = %s\n"
            "]",
            indent(m_ior),
            indent(m_reflectance),
            indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
