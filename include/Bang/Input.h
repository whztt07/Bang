﻿#ifndef INPUT_H
#define INPUT_H

#include "Bang/Bang.h"

#include <iostream>
#include BANG_SDL2_INCLUDE(SDL.h)

#include "Bang/Key.h"
#include "Bang/UMap.h"
#include "Bang/Array.h"
#include "Bang/String.h"
#include "Bang/Vector2.h"
#include "Bang/IToString.h"
#include "Bang/MouseButton.h"

NAMESPACE_BANG_BEGIN

FORWARD class Window;

struct InputEvent
{
    enum class Type
    {
        NONE,
        KEY_DOWN,
        KEY_UP,
        MOUSE_DOWN,
        MOUSE_UP,
        MOUSE_MOVE,
        WHEEL
    };

    Type type               = Type::NONE;
    Key key                 = Key::NONE;
    MouseButton mouseButton = MouseButton::NONE;
    bool autoRepeat         = false;
    float timestampSecs     = 0;
    Vector2i mousePosWindow = Vector2i::Zero;
    Vector2 wheelDelta      = Vector2::Zero;

    bool IsMouseType() const;
    Vector2i GetMousePosWindow() const;
};

class Input
{
public:
    struct ButtonInfo
    {
        bool up = false;         // Just one frame
        bool down = false;       // Just one frame
        bool pressed = false;    // Long duration
        bool autoRepeat = false;

        ButtonInfo() = default;
        ButtonInfo(bool up, bool down, bool pressed);
    };

public:
    static String KeyToString(Key k);

    static bool GetKey(Key k);
    static bool GetKeyUp(Key k);
    static bool GetKeyDown(Key k);
    static bool GetKeyDownRepeat(Key k);

    static const Array<Key>& GetKeysUp();
    static const Array<Key>& GetKeysDown();
    static const Array<Key>& GetPressedKeys();
    static const Array<Key>& GetKeysDownRepeat();
    static const Array<InputEvent>& GetEnqueuedEvents();

    static Vector2 GetMouseWheel();

    static Array<MouseButton> GetMouseButtons();
    static Array<MouseButton> GetMouseButtonsUp();
    static Array<MouseButton> GetMouseButtonsDown();
    static bool GetMouseButton(MouseButton mb);
    static bool GetMouseButtonUp(MouseButton mb);
    static bool GetMouseButtonDown(MouseButton mb);
    static bool GetMouseButtonDoubleClick(MouseButton mb);

    static bool IsMouseInsideWindow();

    static float GetMouseAxisX();
    static float GetMouseAxisY();
    static Vector2 GetMouseAxis();

    static int GetMouseDeltaX();
    static int GetMouseDeltaY();
    static Vector2i GetMouseDelta();

    static void SetMouseWrapping(bool isMouseWrapping);
    static bool IsMouseWrapping();

    static void LockMouseMovement(bool lock);
    static bool IsLockMouseMovement();

    static void SetMousePositionWindow(int windowMousePosX,  int windowMousePosY);
    static void SetMousePositionWindow(const Vector2i &windowMousePosition);
    static Vector2i GetMousePosition();
    static Vector2  GetMousePositionNDC();
    static Vector2i GetMousePositionWindow();
    static Vector2  GetMousePositionWindowNDC();
    static Vector2i GetPreviousMousePositionWindow();
    static Vector2i GetMousePositionWindowWithoutInvertY();
    static Vector2i GetWindowPosInvertedY(const Vector2i &winPos);

    static void StartTextInput();
    static String PollInputText();
    static void StopTextInput();

    void Reset();

    static Input* GetActive();

private:
    static constexpr float DoubleClickMaxSeconds = 0.25f;

    float m_lastMouseDownTimestamp = 0;
    bool m_isMouseWrapping         = false;
    bool m_isADoubleClick          = false;
    bool m_lockMouseMovement       = false;
    bool m_isMouseInside           = false;
    Vector2 m_lastMouseWheelDelta  = Vector2::Zero;

    String m_inputText = "";

    Vector2i m_lastMousePosWindow = Vector2i::Zero;
    Vector2i m_lastClickMousePos  = Vector2i::Zero;

    Array<Key> m_keysUp;
    Array<Key> m_keysDown;
    Array<Key> m_pressedKeys;
    Array<Key> m_keysDownRepeat;

    Input();
    virtual ~Input();

    UMap<Key, ButtonInfo, EnumClassHash> m_keyInfos;
    UMap<MouseButton, ButtonInfo, EnumClassHash> m_mouseInfo;
    Array<InputEvent> m_eventInfoQueue;

    void ProcessMouseWheelEventInfo(const InputEvent &iev);
    void ProcessMouseMoveEventInfo(const InputEvent &iev);
    void ProcessMouseDownEventInfo(const InputEvent &iev);
    void ProcessMouseUpEventInfo(const InputEvent &iev);
    void ProcessKeyDownEventInfo(const InputEvent &iev);
    void ProcessKeyUpEventInfo(const InputEvent &iev);

    void EnqueueEvent(const SDL_Event &event, const Window *window);
    void ProcessEventInfo(const InputEvent &inputEvent);

    void ProcessEnqueuedEvents();
    void OnFrameFinished();

    friend class Window;
};

NAMESPACE_BANG_END

#endif // INPUT_H
