#pragma once

#include <vector>

#include <utils/generic.h>


// ----------------------------------------------------------------------------

enum class SideBySide { A, B };

enum class Scale { Linear, Log };

// ----------------------------------------------------------------------------

template <typename T>
struct Point2D
{
    T x;
    T y;
};

using Point2F = Point2D<float>;

// ----------------------------------------------------------------------------

template <typename T>
struct Color
{
    T r;
    T g;
    T b;
    T a;
};

using ColorRGBA8 = Color<uint8_t>;
using ColorCallback = FuncT<ColorRGBA8(float x, float y)>;

// ----------------------------------------------------------------------------

using Matrix4x4 = std::array<float, 16>;