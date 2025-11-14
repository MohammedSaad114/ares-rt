#include <lightwave.hpp>

namespace lightwave {
class Pathtracer : public SamplingIntegrator {
    int m_maxDepth;
    bool m_nee;

public:
    Pathtracer(const Properties &properties) : SamplingIntegrator(properties) {
        m_maxDepth = properties.get<int>("depth", 2);
        m_nee      = properties.get<bool>("nee", true);
        if (!m_scene->hasLights()) {
            m_nee = false;
        }
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Color contribution(0.0f);
        Color accumalted(1.0f);
        Ray newRay = ray;
        int depth  = 1;

        while (depth < m_maxDepth) {
            Intersection its = m_scene->intersect(newRay, rng);
            contribution += accumalted * its.evaluateEmission().value;

            if (!its) {
                break;
            }

            // Next Event Estimation
            if (m_nee) {
                LightSample lightSample = m_scene->sampleLight(rng);
                const Light *light      = lightSample.light;
                if (light && !light->canBeIntersected()) {
                    DirectLightSample directSample =
                        light->sampleDirect(its.position, rng);

                    Ray shadowRay{ its.position, directSample.wi };
                    Intersection shadowIts = m_scene->intersect(shadowRay, rng);
                    // no occlusion or light is closer than the occluder
                    if (!shadowIts || shadowIts.t > directSample.distance) {
                        contribution +=
                            accumalted *
                            (its.evaluateBsdf(shadowRay.direction).value *
                             directSample.weight / lightSample.probability);
                    }
                }
            }

            BsdfSample bsdfSample = its.sampleBsdf(rng);
            if (bsdfSample.isInvalid()) {
                break;
            }

            accumalted *= bsdfSample.weight;
            newRay.origin    = its.position;
            newRay.direction = bsdfSample.wi;
            depth++;
        }

        // loop terminated before max depth was reached
        if (depth < m_maxDepth) {
            return contribution;
        }

        // loop terminated due to max depth, emission needs to be added
        Intersection its = m_scene->intersect(newRay, rng);
        contribution += accumalted * its.evaluateEmission().value;
        return contribution;
    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "Pathtracer[\n"
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
REGISTER_INTEGRATOR(Pathtracer, "pathtracer")
