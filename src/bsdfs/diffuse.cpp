#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;
    Vector m_normal{ 0, 0, 1 };

public:
    Diffuse(const Properties &properties) { m_albedo = properties.get<Texture>("albedo"); }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo, const Vector &wi) const override {
        // cos(theta) = N * L, where L is the light direction vector and N is
        // the surface normal vector dot product of tow unit vectors is equal to
        // the cosine of the angle between them ref:
        // https://www.youtube.com/watch?v=FiYDkMZCSF4&list=PLlrATfBNZ98edc5GshdBtREv5asFW3yXl&index=5
        // minute 25:00

        // incoming and outgoing directions must lie in the same hemisphere
        if (!Frame::sameHemisphere(wo, wi))
            return BsdfEval::invalid();

        Color brdf{ m_albedo->evaluate(uv) / Pi };
        float cosTheta = m_normal.dot(wi) / wi.length();

        // no reflection if light comes from behind the surface
        cosTheta = max(cosTheta, 0.0f);
        return BsdfEval{ brdf * cosTheta };
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo, Sampler &rng) const override {
        Color brdf{ m_albedo->evaluate(uv) };
        Vector newWi   = squareToCosineHemisphere(rng.next2D());
        float cosTheta = m_normal.dot(wo) / wo.length();

        // ensures new direction is in the same hemisphere as the outgoing
        newWi = cosTheta > 0.0f ? newWi : -newWi;
        // float pdf      = cosineHemispherePdf(newWi);
        return BsdfSample{ newWi.normalized(), brdf };
    }

    Color getAlbedo(const Point2 &uv) const override { return m_albedo->evaluate(uv); }
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
