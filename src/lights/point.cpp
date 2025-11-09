#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {
    Point m_position;
    Color m_power;

public:
    PointLight(const Properties &properties) : Light(properties) {
        m_position = properties.get<Point>("position");
        m_power    = properties.get<Color>("power");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector direction = m_position - origin;
        float distance   = direction.length();
        direction        = direction / distance;
        Color intensity  = m_power / (4 * Pi * distance * distance);
        return DirectLightSample{
            direction,
            intensity,
            distance,
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "PointLight[\n"
            "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
