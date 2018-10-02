#ifndef TIMESINGLETON_H
#define TIMESINGLETON_H

#include "Bang/Bang.h"
#include "Bang/Time.h"

NAMESPACE_BANG_BEGIN

class TimeSingleton
{
public:
    TimeSingleton();

    static void SetDeltaTimeReferenceToNow();

private:
    Time m_initialTime;

    Time m_deltaTime;
    Time m_deltaTimeReference;

    static Time GetNow();
    static Time GetInit();
    static Time GetEllapsed();
    static Time GetDeltaTime();

    static void SetDeltaTime(Time time);
    static void SetInitTime(Time time);

    static TimeSingleton *GetInstance();

    friend class Time;
    friend class Application;
};

NAMESPACE_BANG_END

#endif // TIMESINGLETON_H

