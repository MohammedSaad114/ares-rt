#include <OpenImageDenoise/oidn.hpp>
#include <lightwave.hpp>

namespace lightwave {

class Denoiser : public Postprocess {

public:
    Denoiser(const Properties &properties) : Postprocess(properties) {
        m_normals = properties.getOptional<Image>("normals");
        m_albedo  = properties.getOptional<Image>("albedo");
    }

    virtual void execute() override {
        m_output->initialize(m_input->resolution());

        const Point2i res         = m_input->resolution();
        const int width           = res.x();
        const int height          = res.y();
        const int bytePixelStride = m_input->getBytesPerPixel();
        const int byteRowStride   = res.x() * bytePixelStride;

        // Create an Open Image Denoise device
        oidn::DeviceRef device = oidn::newDevice(); // CPU or GPU if available
        device.commit();

        const size_t pixelCount    = size_t(width) * size_t(height);
        const size_t pixelByteSize = pixelCount * sizeof(Color); // = pixelCount * 3 * sizeof(float)

        oidn::BufferRef colorBuf = device.newBuffer(width * height * 3 * sizeof(float));
        colorBuf.write(0, pixelByteSize, m_input->data());

        // generic ray tracing filter
        oidn::FilterRef filter = device.newFilter("RT");
        filter.setImage("color", colorBuf, oidn::Format::Float3, width, height); // beauty
        if (m_albedo) {
            oidn::BufferRef albedoBuf = device.newBuffer(width * height * 3 * sizeof(float));
            albedoBuf.write(0, pixelByteSize, m_albedo->data());
            filter.setImage("albedo", albedoBuf, oidn::Format::Float3, width, height); // auxiliary
        }
        if (m_normals) {
            oidn::BufferRef normalBuf = device.newBuffer(width * height * 3 * sizeof(float));
            normalBuf.write(0, pixelByteSize, m_normals->data());
            filter.setImage("normal", normalBuf, oidn::Format::Float3, width, height); // auxiliary
        }
        filter.setImage("output", colorBuf, oidn::Format::Float3, width, height); // denoised

        // beauty
        filter.set("hdr", true); // beauty image is HDR
        filter.commit();
        filter.execute(); // Filter the beauty image

        // Check for errors
        const char *errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None)
            logger(EError, "%s", errorMessage);      

        // Fill the input image buffers
        void *outPtr = colorBuf.getData();
        Color *out   = m_output->data();
        memcpy(out, outPtr, pixelByteSize);

        m_output->save();
        Streaming stream{ *m_output };
        stream.update();
    }

    std::string toString() const override {
        return tfm::format(
            "Denoiser[\n"
            "  input = %s,\n"
            "  output = %s,\n"
            "]",
            indent(m_input),
            indent(m_output));
    }

private:
    ref<Image> m_normals;
    ref<Image> m_albedo;
};

} // namespace lightwave

REGISTER_POSTPROCESS(Denoiser, "denoiser");
