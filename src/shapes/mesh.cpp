#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which
 * share an index and vertex buffer. Since individual triangles are rarely
 * needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of
     * the element corresponds to one vertex index (into @c m_vertices ) of the
     * triangle. This list will always contain as many elements as there are
     * triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be
     * fewer than @code 3 * numTriangles @endcode vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging
    /// purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the
    /// geometric normal instead.
    bool m_smoothNormals;

protected:
    int numberOfPrimitives() const override { return int(m_triangles.size()); }

    bool intersect(int primitiveIndex, const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        // Möller–Trumbore intersection algorithm:
        // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection.html
        const Vector3i &triangle = m_triangles[primitiveIndex];
        const Vertex vertex0     = m_vertices[triangle[0]];
        const Vertex vertex1     = m_vertices[triangle[1]];
        const Vertex vertex2     = m_vertices[triangle[2]];
        const Vector edge1       = vertex1.position - vertex0.position;
        const Vector edge2       = vertex2.position - vertex0.position;

        constexpr float kEpsilon = std::numeric_limits<float>::epsilon();

        const Vector pvec = ray.direction.cross(edge2);
        const float det   = edge1.dot(pvec);
        // If the determinant is negative, the triangle is back-facing.
        // If the determinant is close to 0, the ray misses the triangle.
        // If det is close to 0, the ray and triangle are parallel.
        if (det < kEpsilon && det > -kEpsilon)
            return false;
        const float invDet = 1 / det;
        const Vector tvec  = ray.origin - vertex0.position;
        const float u      = tvec.dot(pvec) * invDet;
        if (u < 0 || u > 1)
            return false;

        const Vector qvec = tvec.cross(edge1);
        const float v     = ray.direction.dot(qvec) * invDet;
        if (v < 0 || u + v > 1)
            return false;
        const float t = edge2.dot(qvec) * invDet;
        if (t < Epsilon || t >= its.t)
            return false;
        // interpolate normals
        const Vertex vInterpolated =
            Vertex::interpolate({ u, v }, vertex0, vertex1, vertex2);
        its.position   = ray.origin + t * ray.direction;
        Vector tangent = edge1.normalized();
        tangent = tangent - tangent.dot(its.shadingNormal) * its.shadingNormal;
        its.tangent        = tangent;
        its.t              = t;
        its.geometryNormal = edge1.cross(edge2).normalized();
        its.shadingNormal  = m_smoothNormals ? vInterpolated.normal.normalized()
                                             : its.geometryNormal;
        its.pdf            = 0;
        its.uv             = vInterpolated.uv;
        return true;

        // hints:
        // * use m_triangles[primitiveIndex] to get the vertex indices of the
        // triangle that should be intersected
        // * if m_smoothNormals is true, interpolate the vertex normals from
        // m_vertices
        //   * make sure that your shading frame stays orthonormal!
        // * if m_smoothNormals is false, use the geometrical normal (can be
        // computed from the vertex positions)
    }

    float transmittance(int primitiveIndex, const Ray &ray, float tMax,
                        Sampler &rng) const override {
        Intersection its(-ray.direction, tMax);
        return intersect(ray, its, rng) ? 0.f : 1.f;
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        const Vector3i &triangle = m_triangles[primitiveIndex];
        const Point a            = m_vertices[triangle[0]].position;
        const Point b            = m_vertices[triangle[1]].position;
        const Point c            = m_vertices[triangle[2]].position;
        Bounds bound;
        bound.extend(a);
        bound.extend(b);
        bound.extend(c);
        return bound;
    }

    Point getCentroid(int primitiveIndex) const override {
        const Vector3i &triangle = m_triangles[primitiveIndex];
        const Point a            = m_vertices[triangle[0]].position;
        const Point b            = m_vertices[triangle[1]].position;
        const Point c            = m_vertices[triangle[2]].position;
        return (Vector(a) + Vector(b) + Vector(c)) / 3;
    }

public:
    TriangleMesh(const Properties &properties) {
        m_originalPath  = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath, m_triangles, m_vertices);
        logger(EInfo,
               "loaded ply with %d triangles, %d vertices",
               m_triangles.size(),
               m_vertices.size());
        buildAccelerationStructure();
    }

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        PROFILE("Triangle mesh")
        return AccelerationStructure::intersect(ray, its, rng);
    }

    AreaSample sampleArea(Sampler &rng) const override{
        // only implement this if you need triangle mesh area light sampling for
        // your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
            "Mesh[\n"
            "  vertices = %d,\n"
            "  triangles = %d,\n"
            "  filename = \"%s\"\n"
            "]",
            m_vertices.size(),
            m_triangles.size(),
            m_originalPath.generic_string());
    }
};

} // namespace lightwave

REGISTER_SHAPE(TriangleMesh, "mesh")
