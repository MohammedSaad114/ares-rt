#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        if (!Frame::sameHemisphere(wo, wi))
            return BsdfEval::invalid();

        const auto alpha  = max(float(1e-3), sqr(m_roughness->scalar(uv)));
        Color reflectance = m_reflectance->evaluate(uv);
        float cosThetaO   = abs(Frame::cosTheta(wo));
        // float cosThetaI   = abs(Frame::cosTheta(wi));
        Vector halfVector = (wi + wo).normalized();

        float smithG1O    = microfacet::smithG1(alpha, halfVector, wo);
        float smithG1I    = microfacet::smithG1(alpha, halfVector, wi);
        float microfacetD = microfacet::evaluateGGX(alpha, halfVector);

        // evaluate() → 𝑓𝑟(𝑥,𝜔o,𝜔i)|cos𝜃i| convention cancels out cosThetaI
        float denominator = 4 * cosThetaO;
        if (denominator == 0)
            return BsdfEval::invalid();

        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'
        return BsdfEval{ (reflectance * microfacetD * smithG1O * smithG1I) /
                         denominator };
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha  = max(float(1e-3), sqr(m_roughness->scalar(uv)));
        Vector halfVector = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        // float normalPdf   = microfacet::pdfGGXVNDF(alpha, halfVector, wo);

        Vector newWi = reflect(wo, halfVector);
        if (!Frame::cosTheta(wo) > 0.0f) {
            return BsdfSample::invalid();
        }

        float smithG1I = microfacet::smithG1(alpha, halfVector, newWi);

        // bsdf / pdf cancels out to:
        Color weight = m_reflectance->evaluate(uv) * smithG1I;
        if (!Frame::sameHemisphere(wo, newWi))
            return BsdfSample::invalid();
        // hints:
        // * do not forget to cancel out as many terms from your
        // equations as possible!
        //   (the resulting sample weight is only a product of two
        //   factors)
        return BsdfSample{ newWi, weight };
    }

    std::string toString() const override {
        return tfm::format(
            "RoughConductor[\n"
            "  reflectance = %s,\n"
            "  roughness = %s\n"
            "]",
            indent(m_reflectance),
            indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
