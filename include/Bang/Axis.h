#ifndef AXIS_H
#define AXIS_H

#include "Bang/Bang.h"
#include "Bang/Flags.h"

NAMESPACE_BANG_BEGIN

enum class Axis
{
    VERTICAL,
    HORIZONTAL
};

enum class Axis3D
{
    X,
    Y,
    Z
};

enum class Axis3DExt
{
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ,
    XYZ
};

NAMESPACE_BANG_END

#endif // AXIS_H
