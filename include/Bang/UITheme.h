#ifndef UITHEME_H
#define UITHEME_H

#include "Bang/Bang.h"
#include "Bang/Color.h"

NAMESPACE_BANG_BEGIN

class UITheme
{
public:
    static Color GetOverColor();
    static Color GetSelectedColor();
    static Color GetFocusedBorderColor();
    static Color GetNotFocusedBorderColor();

    static float GetFocusedBorderStroke();
    static float GetNotFocusedBorderStroke();

    static Color GetArrowsColor();
    static Color GetInputsBackgroundColor();

    static Color GetTickColor();
    static Color GetCheckBoxBackgroundColor();

    static Color GetInputTextBackgroundColor();
    static Color GetInputTextBlockedBackgroundColor();
    static Color GetSelectionTextColor();

private:
    static Color GetBaseColor();

    UITheme() = delete;
};

NAMESPACE_BANG_END

#endif // UITHEME_H
