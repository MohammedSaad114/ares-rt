#include <lightwave.hpp>

namespace lightwave {

class Tonemap : public Postprocess {

public:
    Tonemap(const Properties &properties) : Postprocess(properties) {}

    void execute() override {
        m_output->initialize(m_input->resolution());

        float width  = m_input->resolution().x();
        float height = m_input->resolution().y();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int rgb = 0; rgb < 3; rgb++) {
                    float color = m_input->get({ x, y })[rgb];
                    // Reinhard tonemapping
                    color = color / (color + 1.0f);

                    m_output->get({ x, y })[rgb] = color;
                }
            }
        }

        Streaming stream{ *m_output };
        stream.update();
        m_output->save();
    }

    std::string toString() const override {
        return tfm::format(
            "Tonemap[\n"
            "  input = %s,\n"
            "  output = %s,\n"
            "]",
            indent(m_input),
            indent(m_output));
    }
};

} // namespace lightwave

REGISTER_POSTPROCESS(Tonemap, "tonemap");
