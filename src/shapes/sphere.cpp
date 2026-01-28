#include <lightwave.hpp>

namespace lightwave {
class Sphere : public Shape {
    /**
     * @brief Constructs a surface event for a given position, used by @ref
     * intersect to populate the @ref Intersection and by @ref sampleArea to
     * populate the @ref AreaSample .
     * @param surf The surface event to populate with texture coordinates,
     * shading frame and area pdf
     * @param position The hitpoint (i.e., point in [-1,-1,0] to [+1,+1,0]),
     * found via intersection or area sampling
     */
    inline void populate(SurfaceEvent &surf, const Point &position) const {
        surf.position = position;
        // the tangent always points in positive x direction
        surf.tangent = Vector(1, 0, 0);
        // since origin is at (0,0,0) and radius is 1
        surf.geometryNormal = Vector(position).normalized();
        surf.shadingNormal  = surf.geometryNormal;

        // https://en.wikipedia.org/wiki/UV_mapping
        surf.uv = {
            0.5f + (atan2f(position.z(), position.x()) / (2 * Pi)),
            0.5f + (std::asin(position.y()) / Pi),
        };
        // uniform in local space
        surf.pdf = 1.f / (4.f * Pi);
    }

public:
    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override {
        // quadratic equation:
        // (d⋅d)t^2+2(d⋅m)t+(m⋅m−r^2)=0
        // solution: t1,2=(-b±sqrt(b^2-4ac))/2a
        float a        = ray.direction.dot(ray.direction); // (d⋅d)
        Vector m       = ray.origin - getCentroid();
        float b        = 2 * ray.direction.dot(m); // 2(d⋅m)
        float c        = m.dot(m) - 1.0f;          // (m⋅m−r^2)
        float sqrtExpr = b * b - 4 * a * c;        // b^2-4ac
        // if sqrtExpr < 0, no intersection
        if (sqrtExpr < 0) {
            return false;
        }
        float sqrtVal = sqrtf(sqrtExpr);
        float t1      = (-b - sqrtVal) / (2 * a);
        float t2      = (-b + sqrtVal) / (2 * a);
        // intersection behind ray origin
        if (t1 < Epsilon && t2 < Epsilon) {
            return false;
        }
        // closest hit
        float t = std::min(t1, t2);
        if (t1 < Epsilon || t2 < Epsilon) {
            t = std::max(t1, t2);
        }
        if (t >= its.t) {
            return false;
        }
        its.t                = t;
        const Point position = ray.origin + t * ray.direction;
        populate(its,
                 position); // compute the shading frame, texture coordinates
                            // and area pdf (same as sampleArea)

        return true;
    }

    Bounds getBoundingBox() const override {
        // min corner = center - r
        // max corner = center + r
        return Bounds(Point(-1, -1, -1), Point(1, 1, 1));
    }

    Point getCentroid() const override { return Point(0, 0, 0); }

    AreaSample sampleArea(Sampler &rng) const override {
        Point2 rnd = rng.next2D(); // sample a random point in [0,0]..[1,1]
        // uniform sampling
        Point position = squareToUniformSphere(rnd);
        AreaSample sample;
        populate(sample, position); // compute the shading frame, texture coordinates
                                    // and area pdf (same as intersection)
        return sample;
    }
    std::string toString() const override { return "Sphere[]"; }
};
} // namespace lightwave
REGISTER_SHAPE(Sphere, "sphere")
