#ifndef REFLECTVARIABLEHINTS_H
#define REFLECTVARIABLEHINTS_H

#include "Bang/Bang.h"
#include "Bang/Math.h"
#include "Bang/String.h"
#include "Bang/Vector4.h"

namespace Bang
{
class ReflectVariableHints
{
public:
    static const String KeyMinValue;
    static const String KeyMaxValue;
    static const String KeyExtension;
    static const String KeyZoomablePreview;
    static const String KeyIsSlider;
    static const String KeyIsEnum;

    ReflectVariableHints(const String &hintsString = "");
    ~ReflectVariableHints();

    bool GetZoomablePreview() const;
    const Array<String> &GetExtensions() const;
    const Vector4 &GetMinValue() const;
    const Vector4 &GetMaxValue() const;
    bool GetIsSlider() const;
    bool GetIsEnum() const;

private:
    bool m_zoomablePreview = false;
    String m_hintsString = "";
    Vector4 m_minValue = Vector4::NInfinity;
    Vector4 m_maxValue = Vector4::Infinity;
    Array<String> m_extensions;
    bool m_isSlider = false;
    bool m_isEnum = false;
};
}

#endif  // REFLECTVARIABLEHINTS_H