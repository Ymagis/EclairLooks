#pragma once

#include "utils/generic.h"

#include <OpenImageIO/oiioversion.h>


enum class PixelType
{
    Uint8,
    Uint16,
    Half,
    Float,
    Double,
    Unknown
};

enum class PixelFormat
{
    GRAY,
    RGB,
    RGBA,
    Unknown
};

enum class RampType
{
    GRAY,
    NEUTRAL,
    RED,
    GREEN,
    BLUE
};

enum class LUTOrder
{
    RED_FAST,
};

namespace OIIO_NAMESPACE { class ImageBuf; class ImageSpec; }

class Image
{
  public:
    Image();
    Image(const Image &src);
    Image(Image &&src);
    Image & operator=(const Image &src);
    Image & operator=(Image &&src);
    ~Image();

  public:
    uint16_t width() const;
    uint16_t height() const;
    uint8_t channels() const;
    PixelType type() const;
    PixelFormat format() const;

    uint64_t count() const;

    uint8_t const *pixels() const;
    uint8_t *pixels();

    float *pixels_asfloat();
    float const *pixels_asfloat() const;

  public:
    Image to_type(PixelType type) const;

    Image resize(uint16_t w, uint16_t h, bool keepAspectRatio = true, const std::string &filter = "") const;

    bool read(const std::string &path);
    bool write(const std::string &path, PixelType type = PixelType::Uint16) const;

  public:
    static Image FromFile(const std::string &path);
    static Image FromBuffer(void *buffer, size_t size);
    static Image Ramp1D(uint16_t size, float min = 0.f, float max = 1.f, RampType t = RampType::NEUTRAL);
    static Image Lattice(uint16_t size, uint16_t maxwidth = 512, LUTOrder = LUTOrder::RED_FAST);

    static void PrintMetadata(const std::string &filepath, const OIIO::ImageSpec &spec);
    static std::vector<std::string> SupportedExtensions();

  public:
    explicit operator bool() const;

    Image operator+(const Image &rhs);
    Image operator-(const Image &rhs);
    Image operator*(const Image &rhs);
    Image operator*(float v) const;
    Image operator/(const Image &rhs);

  private:
    void to_rgba_format();

  private:
    UPtr<OIIO::ImageBuf> m_imgBuf;
};