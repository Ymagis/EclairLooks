#include "image.h"

#include <QtCore/QDebug>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/filesystem.h>

#include <utils/pystring.h>

using namespace OIIO;


// ----------------------------------------------------------------------------

PixelType TypeDescToPixelType(TypeDesc type)
{
    switch(type.basetype) {
        case TypeDesc::UINT8:
            return PixelType::Uint8;
        case TypeDesc::UINT16:
            return PixelType::Uint16;
        case TypeDesc::HALF:
            return PixelType::Half;
        case TypeDesc::FLOAT:
            return PixelType::Float;
        case TypeDesc::DOUBLE:
            return PixelType::Double;
        default:
            return PixelType::Unknown;
    }
}

TypeDesc PixelTypeToTypeDesc(PixelType type)
{
    switch(type) {
        case PixelType::Uint8:
            return TypeDesc::UINT8;
        case PixelType::Uint16:
            return TypeDesc::UINT16;
        case PixelType::Half:
            return TypeDesc::HALF;
        case PixelType::Float:
            return TypeDesc::FLOAT;
        case PixelType::Double:
            return TypeDesc::DOUBLE;
        default:
            return TypeDesc::UNKNOWN;
    }
}

// ----------------------------------------------------------------------------

Image::Image()
: m_imgBuf(std::make_unique<ImageBuf>())
{

}

Image::Image(const Image &src)
: m_imgBuf(std::make_unique<ImageBuf>())
{
    m_imgBuf->copy(*src.m_imgBuf);
}

Image::Image(Image &&src)
: m_imgBuf(std::move(src.m_imgBuf))
{

}

Image& Image::operator=(const Image &src)
{
    m_imgBuf->copy(*src.m_imgBuf);
    return *this;
}

Image& Image::operator=(Image &&src)
{
    m_imgBuf = std::move(src.m_imgBuf);
    return *this;
}

Image::~Image()
{

}

uint16_t Image::width() const
{
    return m_imgBuf->spec().width;
}

uint16_t Image::height() const
{
    return m_imgBuf->spec().height;
}

uint8_t Image::channels() const
{
    return m_imgBuf->spec().nchannels;
}

PixelType Image::type() const
{
    return TypeDescToPixelType(m_imgBuf->spec().format);
}

PixelFormat Image::format() const
{
    switch(channels()) {
        case 1:
            return PixelFormat::GRAY;
        case 3:
            return PixelFormat::RGB;
        case 4:
            return PixelFormat::RGBA;
        default:
            return PixelFormat::Unknown;
    }
}

uint64_t Image::count() const
{
    return width() * height();
}

uint8_t const *Image::pixels() const
{
    return static_cast<uint8_t*>(m_imgBuf->localpixels());
}

uint8_t *Image::pixels()
{
    return static_cast<uint8_t*>(m_imgBuf->localpixels());
}

float *Image::pixels_asfloat()
{
    return static_cast<float*>(m_imgBuf->localpixels());
}

float const *Image::pixels_asfloat() const
{
    return static_cast<float*>(m_imgBuf->localpixels());
}

Image Image::to_type(PixelType t) const
{
    Image res;
    res.m_imgBuf->copy(*m_imgBuf, PixelTypeToTypeDesc(t));
    return res;
}

Image Image::resize(uint16_t w, uint16_t h, bool keepAspectRatio, const std::string &filter) const
{
    ImageSpec spec = m_imgBuf->spec();
    spec.width = w;
    spec.height = h;

    Image res;
    res.m_imgBuf = std::make_unique<ImageBuf>(spec);

    if (keepAspectRatio)
        ImageBufAlgo::fit(*res.m_imgBuf, *m_imgBuf, filter, 0.0f, true);
    else
        ImageBufAlgo::resize(*res.m_imgBuf, *m_imgBuf, filter, 0.0f);

    return res;
}

bool Image::read(const std::string &path)
{
    if (path.empty())
        return false;

    m_imgBuf.reset(new ImageBuf(path));

    if (!m_imgBuf->read(0, 0, true, TypeDesc::FLOAT)) {
        qWarning() << "Could not open image !";
        return false;
    }

    const ImageSpec &spec = m_imgBuf->nativespec();
    Image::PrintMetadata(path, spec);

    to_rgba_format();

    return true;
}

bool Image::write(const std::string &path, PixelType type) const
{
    m_imgBuf->set_write_format(PixelTypeToTypeDesc(type));
    return m_imgBuf->write(path);
}

Image Image::FromFile(const std::string &path)
{
    Image res;
    if (!res.read(path))
        return Image();

    return res;
}

Image Image::FromBuffer(void * buffer, size_t size)
{
    ImageSpec config;
    Filesystem::IOMemReader memreader(buffer, size);
    void *ptr = &memreader;
    config.attribute("oiio:ioproxy", TypeDesc::PTR, &ptr);

    auto in = ImageInput::open("in.exr", &config);
    if (!in) {
        qWarning() << "Could not open image !";
        return Image();
    }

    ImageSpec spec = in->spec();
    spec.set_format(TypeDesc::FLOAT);
    Image::PrintMetadata("Embeded", spec);

    Image res;
    res.m_imgBuf.reset(new ImageBuf(spec));
    in->read_image(
        TypeDesc::FLOAT,
        res.m_imgBuf->localpixels(),
        res.m_imgBuf->pixel_stride(),
        res.m_imgBuf->scanline_stride(),
        res.m_imgBuf->z_stride());

    res.to_rgba_format();

    return res;
}

Image Image::Ramp1D(uint16_t size, float min, float max, RampType t)
{
    ImageSpec spec;
    spec.width = size;
    spec.height = 1;
    spec.nchannels = t == RampType::GRAY ? 1 : 3;
    spec.set_format(TypeDesc::FLOAT);

    Image res;
    res.m_imgBuf.reset(new ImageBuf(spec));

    uint64_t i = 0;
    for (ImageBuf::Iterator<float> it(*res.m_imgBuf); !it.done(); ++it, ++i) {
        if (t == RampType::GRAY) {
            it[0] = 1.0f * i / (spec.width - 1);
        } else if (t == RampType::NEUTRAL) {
            it[0] = 1.0f * i / (spec.width - 1);
            it[1] = 1.0f * i / (spec.width - 1);
            it[2] = 1.0f * i / (spec.width - 1);
        } else if (t == RampType::RED) {
            it[0] = 1.0f * i / (spec.width - 1);
            it[1] = 0.0f;
            it[2] = 0.0f;
        } else if (t == RampType::GREEN) {
            it[0] = 0.0f;
            it[1] = 1.0f * i / (spec.width - 1);
            it[2] = 0.0f;
        } else if (t == RampType::BLUE) {
            it[0] = 0.0f;
            it[1] = 0.0f;
            it[2] = 1.0f * i / (spec.width - 1);
        }
    }

    return res;
}

Image Image::Lattice(uint16_t size, uint16_t maxwidth, LUTOrder order)
{
    if (order != LUTOrder::RED_FAST) {
        qInfo() << "LUT Odering not supported !";
        return Image();
    }

    uint64_t elem_count = size * size * size;
    uint16_t width = elem_count > maxwidth ? maxwidth : elem_count;
    uint16_t height = std::ceil(1.f * elem_count / width);
    qInfo() << "Lattice image for size" << size << ":" << width << "x" << height;

    ImageSpec spec;
    spec.width = width;
    spec.height = height;
    spec.nchannels = 3;
    spec.set_format(TypeDesc::FLOAT);

    Image res;
    res.m_imgBuf.reset(new ImageBuf(spec));

    uint32_t i = 0;
    float * pix = reinterpret_cast<float *>(res.pixels_asfloat());
    for (uint32_t b = 0; b < size; ++b) {
        float bnorm = 1.0f * b / (size - 1);
        for (uint32_t g = 0; g < size; ++g) {
            float gnorm = 1.0f * g / (size - 1);
            for (uint32_t r = 0; r < size; ++r) {
                float rnorm = 1.0f * r / (size - 1);
                pix[i * 3] = rnorm;
                pix[i * 3 + 1] = gnorm;
                pix[i * 3 + 2] = bnorm;
                i++;
            }
        }
    }

    return res;
}

void Image::PrintMetadata(const std::string &filepath, const ImageSpec &spec)
{
    qInfo() << "File -" << QString::fromStdString(filepath);
    qInfo() << "Dimensions :" << spec.width << "x" << spec.height << "~" << spec.nchannels;
    qInfo() << "Type :" << spec.format.elementsize() * 8 << "bits" << (spec.format.is_floating_point() ? "Float" : "Integer");

    for (size_t i = 0; i < spec.channelnames.size(); ++i)
        qInfo() << "Channel " << i << ":" << QString::fromStdString(spec.channelnames[i]);

    qInfo() << "-----------------";
    for (const ParamValue &p : spec.extra_attribs) {
        QDebug info = qInfo();
        info << p.name().c_str() << "\t";

        if (p.type() == TypeString)
            info << *(const char **)p.data();
        else if (p.type() == TypeFloat)
            info << *(const float *)p.data();
        else if (p.type() == TypeInt)
            info << *(const int *)p.data();
        else if (p.type() == TypeDesc::UINT)
            info << *(const unsigned int *)p.data();
        else if (p.type() == TypeMatrix) {
            const float *f = (const float *)p.data();
            info << f[0] << f[1] << f[2] << f[3] << f[4] << f[5] << f[6] << f[7] << f[8] << f[9] << f[10]
                 << f[11] << f[12] << f[13] << f[14] << f[15];
        }
    }
    qInfo() << "-----------------\n";
}

std::vector<std::string> Image::SupportedExtensions()
{
    std::vector<std::string> res;

    std::string exts;
    OIIO::getattribute("extension_list", exts);

    // Format : "tiff:tif;jpeg:jpg,jpeg;openexr:exr"
    std::vector<std::string> formats;
    pystring::split(exts, formats, ";");

    for (auto format : formats) {
        std::vector<std::string> desc;
        pystring::split(format, desc, ":");
        if (desc.size() != 2)
            continue;

        std::vector<std::string> exts;
        pystring::split(desc[1], exts, ",");
        for (auto ext: exts)
            res.push_back(ext);
    }

    // Additional known format
    res.push_back("ari");

    return res;
}

Image::operator bool() const { return (width() != 0 && height() != 0); }

Image Image::operator +(const Image & rhs)
{
    Image res = *this;
    ImageBufAlgo::add(*res.m_imgBuf, *m_imgBuf, *rhs.m_imgBuf);
    return res;
}

Image Image::operator -(const Image & rhs)
{
    Image res = *this;
    ImageBufAlgo::sub(*res.m_imgBuf, *m_imgBuf, *rhs.m_imgBuf);
    return res;
}

Image Image::operator *(const Image & rhs)
{
    Image res = *this;
    ImageBufAlgo::mul(*res.m_imgBuf, *m_imgBuf, *rhs.m_imgBuf);
    return res;
}

Image Image::operator *(float v) const
{
    Image res = *this;
    ImageBufAlgo::mul(*res.m_imgBuf, *m_imgBuf, v);
    return res;
}

Image Image::operator /(const Image & rhs)
{
    Image res = *this;
    ImageBufAlgo::div(*res.m_imgBuf, *m_imgBuf, *rhs.m_imgBuf);
    return res;
}

void Image::to_rgba_format()
{
    if (channels() == 1) {
        int channelorder[] = { 0, 0, 0, -1 /*use a float value*/ };
        float channelvalues[] = { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0 };
        std::string channelnames[] = { "R", "G", "B", "A" };
        ImageBufAlgo::channels(*m_imgBuf, *m_imgBuf, 4, channelorder, channelvalues, channelnames);
    }
    else if (channels() == 3) {
        int channelorder[] = { 0, 1, 2, -1 /*use a float value*/ };
        float channelvalues[] = { 0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0 };
        std::string channelnames[] = { "", "", "", "A" };
        ImageBufAlgo::channels(*m_imgBuf, *m_imgBuf, 4, channelorder, channelvalues, channelnames);
    }
}