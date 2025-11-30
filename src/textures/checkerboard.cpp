#include <lightwave.hpp>

namespace lightwave {

class CheckerboardTexture : public Texture {
    Color color0;
    Color color1;
    Vector2 scale;

public:
    CheckerboardTexture(const Properties &properties) {
        color0 = properties.get<Color>("color0", Color(0));
        color1 = properties.get<Color>("color1", Color(1));
        scale  = properties.get<Vector2>("scale");
    }

    Color evaluate(const Point2 &uv) const override {
        int s = floor(uv.x() * scale.x());
        int t = floor(uv.y() * scale.y());
        if ((s + t) % 2 == 0) {
            return color0;
        } else {
            return color1;
        }
    }

    std::string toString() const override {
        return tfm::format(
            "CheckerboardTexture[\n"
            "  color0 = %s\n"
            "]",
            indent(color0));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
