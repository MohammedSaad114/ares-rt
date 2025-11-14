#include <lightwave.hpp>

namespace lightwave {

class Lambertian : public Emission {
    ref<Texture> m_emission;

public:
    Lambertian(const Properties &properties) {
        m_emission = properties.get<Texture>("emission");
    }

    EmissionEval evaluate(const Point2 &uv, const Vector &wo) const override {
        Vector normal{ 0, 0, 1 };
        Color brdf     = m_emission->evaluate(uv);
        float cosTheta = normal.dot(wo) / wo.length();
        if (cosTheta <= 0) {
            brdf = Color(0);
        }
        return EmissionEval{ brdf };
    }

    std::string toString() const override {
        return tfm::format(
            "Lambertian[\n"
            "  emission = %s\n"
            "]",
            indent(m_emission));
    }
};

} // namespace lightwave

REGISTER_EMISSION(Lambertian, "lambertian")
