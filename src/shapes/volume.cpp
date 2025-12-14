#include <lightwave.hpp>

namespace lightwave {

class Volume : public Shape {
    float m_density;
    ref<Shape> m_boundary;

public:
    Volume(const Properties &properties) {
        m_density  = properties.get<float>("density");
        m_boundary = properties.getOptionalChild<Shape>();
    }

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        float tMin = Epsilon;
        float tMax = its.t;

        if (m_boundary) {
            Intersection firstIts, secondIts;

            // no hit with the volume boundary
            if (!m_boundary->intersect(ray, firstIts, rng))
                return false;
            float tEnter = tMin;
            // inside -> outside
            float tExit = firstIts.t;

            // second hit
            Ray insideRay{ ray.origin + ray.direction * firstIts.t,
                           ray.direction };
            if (m_boundary->intersect(insideRay, secondIts, rng)) {
                // outside -> inside -> outside
                tEnter = firstIts.t;
                tExit  = firstIts.t + secondIts.t;
                if (tExit < tEnter)
                    std::swap(tEnter, tExit);
            }

            // clip to min/max
            tMin = std::max(tEnter, tMin);
            tMax = std::min(tExit, tMax);
        }

        float x        = rng.next();
        float sampledT = tMin - std::log(1.f - x) / m_density;

        // sampled t is out of bounds
        if (sampledT <= tMin || sampledT >= tMax)
            return false;

        // intersection population
        its.t              = sampledT;
        its.position       = ray.origin + ray.direction * sampledT;
        Frame frame        = Frame((-ray.direction).normalized());
        its.shadingNormal  = frame.normal;
        its.geometryNormal = frame.normal;
        its.tangent        = frame.tangent;
        its.uv             = Point2(0.f);

        return true;
    }

    float transmittance(const Ray &ray, float tMax,
                        Sampler &rng) const override {
        if (m_boundary) {
            Intersection firstIts, secondIts;

            // no hit with the volume boundary
            if (!m_boundary->intersect(ray, firstIts, rng))
                return 1.f;
            float tEnter = 0.f;
            float tExit  = firstIts.t;

            // second hit
            Ray insideRay{ ray.origin + ray.direction * firstIts.t,
                           ray.direction };
            if (m_boundary->intersect(insideRay, secondIts, rng)) {
                // outside → inside → outside
                tEnter = firstIts.t;
                tExit  = firstIts.t + secondIts.t;
                if (tExit < tEnter)
                    std::swap(tEnter, tExit);
            }

            tEnter = std::max(0.f, tEnter);
            tExit  = std::min(tMax, tExit);

            // ray doesn't travel inside the volume
            if (tExit <= tEnter)
                return 1.f;

            tMax = tExit - tEnter;
        }

        return std::exp(-m_density * tMax);
    }

    Bounds getBoundingBox() const override {
        if (m_boundary) {
            return m_boundary->getBoundingBox();
        }
        return Bounds::full();
    }

    Point getCentroid() const override {
        if (m_boundary) {
            return m_boundary->getCentroid();
        }
        return Point(0, 0, 0);
    }

    std::string toString() const override { return "Volume[]"; }
};

} // namespace lightwave

REGISTER_SHAPE(Volume, "volume")
