#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {
    ref<Instance> m_shape;

public:
    AreaLight(const Properties &properties) : Light(properties) { this->m_shape = properties.getChild<Instance>(); }

    DirectLightSample sampleDirect(const Point &origin, Sampler &rng) const override {
        // sample a light
        AreaSample sampled = m_shape->sampleArea(rng);
        // direction is outgoing from light to surface y -> x
        Vector direction{ origin - sampled.position };
        const float distance = direction.length();
        direction            = direction / distance;
        // sampled direction in local coordinate for the evaluate function
        Vector sampledLocal = sampled.shadingFrame().toLocal(direction);
        // add the shapes contribution
        EmissionEval shapeEmission = m_shape->emission()->evaluate(sampled.uv, sampledLocal);
        Color intensity            = shapeEmission.value ;

        return DirectLightSample{
            -direction.normalized(),
            intensity / (sqr(distance) * sampled.pdf),
            distance - Epsilon,
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "AreaLight[\n"
            "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")