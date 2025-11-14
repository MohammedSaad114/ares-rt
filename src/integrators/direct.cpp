#include <lightwave.hpp>

namespace lightwave {
class DirectIntegrator : public SamplingIntegrator {

public:
    DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {}

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);
        Color cont       = its.evaluateEmission().value;

        if (!its) {
            return cont;
        }

        LightSample lightSample = m_scene->sampleLight(rng);
        const Light *light      = lightSample.light;
        if (light && !light->canBeIntersected()) {
            DirectLightSample directSample =
                light->sampleDirect(its.position, rng);

            Ray secRay{ its.position, directSample.wi };
            Intersection secIts = m_scene->intersect(secRay, rng);

            if (!secIts || secIts.t > directSample.distance) {
                cont += its.evaluateBsdf(secRay.direction).value *
                        directSample.weight / lightSample.probability;
            }
        }

        BsdfSample bsdfSample = its.sampleBsdf(rng);
        if (!bsdfSample.isInvalid()) {
            Ray newRay{ its.position, bsdfSample.wi };
            Intersection newShapes = m_scene->intersect(newRay, rng);
            cont += bsdfSample.weight * newShapes.evaluateEmission().value;
        }

        return cont;
    }

    /// @brief An optional textual representation of this class, which can
    /// be useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "DirectIntegrator[\n"
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
REGISTER_INTEGRATOR(DirectIntegrator, "direct")
