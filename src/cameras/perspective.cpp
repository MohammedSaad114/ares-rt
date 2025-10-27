#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 *
 * In local coordinates (before applying m_transform), the camera looks in
 * positive z direction [0,0,1]. Pixels on the left side of the image ( @code
 * normalized.x < 0 @endcode ) are directed in negative x direction ( @code
 * ray.direction.x < 0 ), and pixels at the bottom of the image ( @code
 * normalized.y < 0 @endcode ) are directed in negative y direction ( @code
 * ray.direction.y < 0 ).
 */
class Perspective : public Camera {
    Vector2 m_scale;

public:
    Perspective(const Properties &properties) : Camera(properties) {
        const float fov           = properties.get<float>("fov");
        const std::string fovAxis = properties.get<std::string>("fovAxis");

        // the tangent of half the field of view in radians precomputed
        const float fovTanHalf = tan(fov * Pi / 360.0f);
        const float aspectRatio =
            m_resolution.x() / static_cast<float>(m_resolution.y());

        m_scale = (fovAxis == "x")
                      ? Vector2(fovTanHalf, fovTanHalf / aspectRatio)
                      : Vector2(fovTanHalf * aspectRatio, fovTanHalf);

        // hints:
        // * precompute any expensive operations here (most importantly
        // trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        Vector origin(0, 0, 0);
        Vector direction(
            m_scale.x() * normalized.x(), m_scale.y() * normalized.y(), 1.0f);
        direction       = direction.normalized();
        Ray transformed = m_transform->apply(Ray(origin, direction));
        return CameraSample{ transformed, Color(1.0f) };

        // hints:
        // * use m_transform to transform the local camera coordinate system
        // into the world coordinate system
    }

    std::string toString() const override {
        return tfm::format(
            "Perspective[\n"
            "  width = %d,\n"
            "  height = %d,\n"
            "  transform = %s,\n"
            "]",
            m_resolution.x(),
            m_resolution.y(),
            indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_CAMERA(Perspective, "perspective")
