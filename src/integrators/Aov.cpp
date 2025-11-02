#include <lightwave.hpp>

namespace lightwave {
class AovIntegrator : public SamplingIntegrator {
    std::string m_variable;
    float m_scale;

public:
    AovIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
        m_variable = properties.get<std::string>("variable");
        m_scale    = properties.get<float>("scale", 1.0f);
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);

        Vector d = ray.direction;
        if (m_variable == "normals") {
            if (its) {
                // n' = (n + 1) / 2
                return (Color(its.shadingNormal) + Color(1, 1, 1)) / 2;
            }
            // no intersection, 0 normal
            return Color(0.5f);
        } else if (m_variable == "bvh") {
            int bCounter = its.stats.bvhCounter;
            int pCounter = its.stats.primCounter;
            return Color(bCounter / m_scale, pCounter / m_scale, 0);
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
