#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        Vector normal{ 0, 0, 1 };
        // incoming and outgoing directions must lie in the same hemisphere
        if (!Frame::sameHemisphere(wo, wi))
            return BsdfEval::invalid();

        Color brdf{ color / Pi };
        float cosTheta = normal.dot(wi) / wi.length();

        // no reflection if light comes from behind the surface
        cosTheta = max(cosTheta, 0.0f);
        return BsdfEval{ brdf * cosTheta };

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        Vector normal{ 0, 0, 1 };
        Color brdf{ color };
        Vector newWi   = squareToCosineHemisphere(rng.next2D());
        float cosTheta = normal.dot(wo) / wo.length();

        // ensures new direction is in the same hemisphere as the outgoing
        if (!Frame::cosTheta(wo) > 0.0f) {
            return BsdfSample::invalid();
        }
        if (!Frame::sameHemisphere(wo, newWi))
            return BsdfSample::invalid();
        // float pdf      = cosineHemispherePdf(newWi);
        return BsdfSample{ newWi, brdf };

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        if (!Frame::sameHemisphere(wo, wi))
            return BsdfEval::invalid();

        Color reflectance = color;
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

        // hints:
        // * copy your roughconductor bsdf evaluate here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        Vector halfVector = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        // float normalPdf   = microfacet::pdfGGXVNDF(alpha, halfVector, wo);

        Vector newWi = reflect(wo, halfVector);
        newWi        = Frame::cosTheta(wo) > 0.0f ? newWi : -newWi;

        float smithG1I = microfacet::smithG1(alpha, halfVector, newWi);

        // bsdf / pdf cancels out to:
        Color weight = color * smithG1I;

        // hints:
        // * do not forget to cancel out as many terms from your
        // equations as possible!
        //   (the resulting sample weight is only a product of two
        //   factors)
        return BsdfSample{ newWi, weight };

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha     = max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular  = m_specular->scalar(uv);
        const auto metallic  = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        PROFILE("Principled")

        const auto combination = combine(uv, wo);

        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
        BsdfEval diffuseEval  = combination.diffuse.evaluate(wo, wi);
        BsdfEval metallicEval = combination.metallic.evaluate(wo, wi);
        return BsdfEval{ diffuseEval.value + metallicEval.value };
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        PROFILE("Principled")

        const auto combination = combine(uv, wo);

        // hint: sample either `combination.diffuse` (probability
        // `combination.diffuseSelectionProb`) or `combination.metallic`

        BsdfSample sample = combination.metallic.sample(wo, rng);
        if (rng.next() < combination.diffuseSelectionProb) {
            sample = combination.diffuse.sample(wo, rng);
            return BsdfSample{
                sample.wi, sample.weight / combination.diffuseSelectionProb
            };
        }
        return BsdfSample{
            sample.wi, sample.weight / (1 - combination.diffuseSelectionProb)
        };
    }

    std::string toString() const override {
        return tfm::format(
            "Principled[\n"
            "  baseColor = %s,\n"
            "  roughness = %s,\n"
            "  metallic  = %s,\n"
            "  specular  = %s,\n"
            "]",
            indent(m_baseColor),
            indent(m_roughness),
            indent(m_metallic),
            indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
