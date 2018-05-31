#ifndef IVALUECHANGEDLISTENER_H
#define IVALUECHANGEDLISTENER_H

#include "Bang/Bang.h"

NAMESPACE_BANG_BEGIN

FORWARD class Object;

class IValueChangedListener
{
public:
    virtual void OnValueChanged(Object *object) = 0;
};

NAMESPACE_BANG_END

#endif // IVALUECHANGEDLISTENER_H
