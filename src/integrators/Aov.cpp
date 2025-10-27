#include <lightwave.hpp>

namespace lightwave {
class AovIntegrator : public SamplingIntegrator {
    std::string m_variable;

public:
    AovIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
        m_variable = properties.get<std::string>("variable");
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Vector d = ray.direction;
        if (m_variable == "normals") {
            // remap the direction from [-1,+1]^3 to [0,+1]^3
            return Color((d + Vector(1)) / 2);
        }
        return Color();
    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "AovIntegrator[\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            indent(m_sampler),
            indent(m_image));
    }
};

} // namespace lightwave

// this informs lightwave to use our class AovIntegrator whenever a
// <integrator type="aov" /> is found in a scene file
REGISTER_INTEGRATOR(AovIntegrator, "aov")
