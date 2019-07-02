#include "common.h"


QColor toQColor(const ColorRGBA8 &c)
{
    return QColor(c.r, c.g, c.b);
}