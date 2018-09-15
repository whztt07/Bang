#include "Bang/Input.h"

#include BANG_SDL2_INCLUDE(SDL.h)

#include "Bang/GL.h"
#include "Bang/Time.h"
#include "Bang/Debug.h"
#include "Bang/Window.h"
#include "Bang/Application.h"

USING_NAMESPACE_BANG

Input::Input()
{
    Input::StartTextInput();
}

Input::~Input()
{
}

void Input::OnFrameFinished()
{
    m_lastMousePosWindow = GetMousePosition();

    // KEYS
    for (auto it = m_keyInfos.Begin(); it != m_keyInfos.End();)
    {
        ButtonInfo &kInfo = it->second;
        if (kInfo.down)
        {
            kInfo.down = false; // Not down anymore, just pressed.
            m_keysDown.Remove(it->first);
            m_keysDownRepeat.Remove(it->first);
        }

        if (kInfo.up) // After a frame where it was Up
        {
            m_keysUp.Remove(it->first);
            it = m_keyInfos.Remove(it);
        }
        else
        {
            ++it;
        }
    }
    //

    // MOUSE
    for (auto it = m_mouseInfo.Begin(); it != m_mouseInfo.End();)
    {
        ButtonInfo &mbInfo = it->second;
        if (mbInfo.down)
        {
            mbInfo.down = false; // Not down anymore, just pressed.
        }

        if (mbInfo.up)
        {
            it = m_mouseInfo.Remove(it);
        }
        else
        {
            ++it;
        }
    }
    //

    m_lastMouseWheelDelta = Vector2::Zero;
    m_eventInfoQueue.Clear();
}

void Input::ProcessEventInfo(const InputEvent &iev)
{
    if (iev.type == InputEvent::Type::WHEEL)
    {
        ProcessMouseWheelEventInfo(iev);
    }
    else if (iev.type == InputEvent::Type::MOUSE_MOVE)
    {
        ProcessMouseMoveEventInfo(iev);
    }
    else if (iev.type == InputEvent::Type::MOUSE_DOWN)
    {
        ProcessMouseDownEventInfo(iev);
    }
    else if (iev.type == InputEvent::Type::MOUSE_UP)
    {
        ProcessMouseUpEventInfo(iev);
    }
    else if (iev.type == InputEvent::Type::KEY_DOWN)
    {
        ProcessKeyDownEventInfo(iev);
    }
    else if (iev.type == InputEvent::Type::KEY_UP)
    {
        ProcessKeyUpEventInfo(iev);
    }
}

void Input::ProcessMouseWheelEventInfo(const InputEvent &iev)
{
    m_lastMouseWheelDelta = iev.wheelDelta;
}

void Input::ProcessMouseMoveEventInfo(const InputEvent &iev)
{
    BANG_UNUSED(iev);

    // Mouse wrapping
    if (m_isMouseWrapping)
    {
        Window *window          = Window::GetActive();
        Vector2i windowSize     = window->GetSize();
        Vector2i mouseCoords    = Input::GetMousePosition();

        constexpr int wrapPxMargin = 2;
        if (mouseCoords.x < wrapPxMargin)
        {
            Input::SetMousePositionWindow(windowSize.x - wrapPxMargin - 1,
                                          mouseCoords.y);
        }
        else if (mouseCoords.x > (windowSize.x - wrapPxMargin))
        {
            Input::SetMousePositionWindow(wrapPxMargin + 1,
                                          mouseCoords.y);
        }

        if (mouseCoords.y < wrapPxMargin)
        {
            Input::SetMousePositionWindow(mouseCoords.x,
                                          windowSize.y - wrapPxMargin - 1);
        }
        else if (mouseCoords.y > (windowSize.y - wrapPxMargin))
        {
            Input::SetMousePositionWindow(mouseCoords.x,
                                          wrapPxMargin + 1);
        }
    }
}

void Input::ProcessMouseDownEventInfo(const InputEvent &iev)
{
    MouseButton mb = iev.mouseButton;
    bool up = false;
    if (m_mouseInfo.ContainsKey(mb))
    {
        up = m_mouseInfo.Get(mb).up;
    }
    m_isADoubleClick = false; // Reset double click

    m_mouseInfo.Add(mb, ButtonInfo(up, true, true));
    if ((iev.timestampSecs - m_lastMouseDownTimestamp) <= DoubleClickMaxSeconds)
    {
        m_isADoubleClick = true;
    }

    m_lastClickMousePos = GetMousePosition();
    m_lastMouseDownTimestamp = iev.timestampSecs;
}

void Input::ProcessMouseUpEventInfo(const InputEvent &iev)
{
    MouseButton mb = iev.mouseButton;
    if (m_mouseInfo.ContainsKey(mb))
    {
        // We use these ifs, because down & pressed & up can happen
        // in the same frame (this does happen sometimes)
        if (m_mouseInfo[mb].down)
        {
            m_mouseInfo.Add(mb, ButtonInfo(true,
                                           m_mouseInfo[mb].down,
                                           m_mouseInfo[mb].pressed));
        }
        else
        {
            m_mouseInfo.Add(mb, ButtonInfo(true, false, false));
        }
    }
}


void Input::ProcessKeyDownEventInfo(const InputEvent &iev)
{
    Key k = iev.key;
    if (!m_keyInfos.ContainsKey(k))
    {
        m_keyInfos[k] = ButtonInfo();
    }
    m_keyInfos[k].down       = true;
    m_keyInfos[k].pressed    = true;
    m_keyInfos[k].autoRepeat = iev.autoRepeat;

    if (!iev.autoRepeat)
    {
        m_pressedKeys.PushBack(k);
        m_keysDown.PushBack(k);
        m_keysUp.Remove(k);
    }

    if (!m_keysDownRepeat.Contains(k))
    {
        m_keysDownRepeat.PushBack(k);
    }
}

void Input::ProcessKeyUpEventInfo(const InputEvent &iev)
{
    if (iev.autoRepeat) return;

    Key k = iev.key;
    if (!m_keyInfos.ContainsKey(k))
    {
        m_keyInfos[k] = ButtonInfo();
    }
    m_keyInfos[k].up = true;

    m_pressedKeys.Remove(k);
    m_keysDown.Remove(k);
    m_keysUp.PushBack(k);
    m_keysDownRepeat.Remove(k);
}

void Input::EnqueueEvent(const SDL_Event &event, const Window *window)
{
    if (!window->HasFocus())
    {
        Reset();
        return;
    }

    m_isMouseInside = window->HasFocus() && window->IsMouseOver();

    bool enqueue = false;
    InputEvent inputEvent;

    inputEvent.timestampSecs = event.key.timestamp / 1000.0f;
    inputEvent.mousePosWindow = Input::GetMousePositionWindowWithoutInvertY();

    switch (event.type)
    {
        case SDL_KEYDOWN:
            inputEvent.type       = InputEvent::Type::KEY_DOWN;
            inputEvent.autoRepeat = event.key.repeat;
            inputEvent.key        = SCAST<Key>(event.key.keysym.sym);
            enqueue = true;
        break;

        case SDL_KEYUP:
            inputEvent.type           = InputEvent::Type::KEY_UP;
            inputEvent.autoRepeat     = event.key.repeat;
            inputEvent.key            = SCAST<Key>(event.key.keysym.sym);
            enqueue = true;
        break;

        case SDL_TEXTINPUT:
            m_inputText += String(event.text.text);
        break;

        case SDL_MOUSEBUTTONDOWN:
            inputEvent.type           = InputEvent::Type::MOUSE_DOWN;
            inputEvent.mouseButton    = SCAST<MouseButton>(event.button.button);
            inputEvent.mousePosWindow = Vector2i(event.button.x, event.button.y);
            enqueue = true;
        break;

        case SDL_MOUSEBUTTONUP:
            inputEvent.type        = InputEvent::Type::MOUSE_UP;
            inputEvent.mouseButton = SCAST<MouseButton>(event.button.button);
            inputEvent.mousePosWindow = Vector2i(event.button.x, event.button.y);
            enqueue = true;
        break;

        case SDL_MOUSEWHEEL:
            inputEvent.type       = InputEvent::Type::WHEEL;
            inputEvent.wheelDelta = Vector2(event.wheel.x, event.wheel.y);
            inputEvent.mousePosWindow = Vector2i(event.button.x, event.button.y);
            enqueue = true;
        break;

        case SDL_MOUSEMOTION:
            inputEvent.type      = InputEvent::Type::MOUSE_MOVE;
            inputEvent.wheelDelta = Vector2(event.wheel.x, event.wheel.y);
            inputEvent.mousePosWindow = Vector2i(event.button.x, event.button.y);
            enqueue = true;
        break;
    }

    if (enqueue)
    {
        inputEvent.mousePosWindow = GetWindowPosInvertedY(inputEvent.mousePosWindow);
        m_eventInfoQueue.PushBack(inputEvent);
    }
}

void Input::ProcessEnqueuedEvents()
{
    for (const InputEvent &iev : m_eventInfoQueue)
    {
        ProcessEventInfo(iev);
    }
}

Input *Input::GetActive()
{
    Window *w = Window::GetActive();
    return w ? w->GetInput() : nullptr;
}

String KeyToString(Key k)
{
    return String( SDL_GetKeyName( SCAST<SDL_Keycode>(k) ) );
}

bool Input::GetKey(Key k)
{
    Input *inp = Input::GetActive();
    return inp->m_keyInfos.ContainsKey(k) && inp->m_keyInfos[k].pressed;
}

bool Input::GetKeyUp(Key k)
{
    Input *inp = Input::GetActive();
    return inp->m_keyInfos.ContainsKey(k) && inp-> m_keyInfos[k].up;
}

bool Input::GetKeyDown(Key k)
{
    Input *inp = Input::GetActive();
    return  inp->m_keyInfos.ContainsKey(k) && inp->m_keyInfos[k].down &&
           !inp->m_keyInfos[k].autoRepeat;
}

bool Input::GetKeyDownRepeat(Key k)
{
    Input *inp = Input::GetActive();
    return inp->m_keyInfos.ContainsKey(k) && inp->m_keyInfos[k].down;
}

const Array<Key> &Input::GetKeysUp()
{
    return Input::GetActive()->m_keysUp;
}

const Array<Key> &Input::GetKeysDown()
{
    return Input::GetActive()->m_keysDown;
}

const Array<Key>& Input::GetPressedKeys()
{
    return Input::GetActive()->m_pressedKeys;
}

const Array<Key> &Input::GetKeysDownRepeat()
{
    return Input::GetActive()->m_keysDownRepeat;
}

const Array<InputEvent> &Input::GetEnqueuedEvents()
{
    return Input::GetActive()->m_eventInfoQueue;
}

Vector2 Input::GetMouseWheel()
{
    Input *inp = Input::GetActive();
    return inp->m_lastMouseWheelDelta;
}

Array<MouseButton> Input::GetMouseButtons()
{
    Array<MouseButton> mouseButtons;
    Input *inp = Input::GetActive();
    for (auto it : inp->m_mouseInfo)
    {
        if (it.second.pressed)
        {
            mouseButtons.PushBack(it.first);
        }
    }
    return mouseButtons;
}
Array<MouseButton> Input::GetMouseButtonsUp()
{
    Array<MouseButton> mouseButtons;
    Input *inp = Input::GetActive();
    for (auto it : inp->m_mouseInfo)
    {
        if (it.second.up)
        {
            mouseButtons.PushBack(it.first);
        }
    }
    return mouseButtons;
}
Array<MouseButton> Input::GetMouseButtonsDown()
{
    Array<MouseButton> mouseButtons;
    Input *inp = Input::GetActive();
    for (auto it : inp->m_mouseInfo)
    {
        if (it.second.down)
        {
            mouseButtons.PushBack(it.first);
        }
    }
    return mouseButtons;
}


bool Input::GetMouseButton(MouseButton mb)
{
    Input *inp = Input::GetActive();
    return inp->m_mouseInfo.ContainsKey(mb) && inp->m_mouseInfo[mb].pressed;
}

bool Input::GetMouseButtonUp(MouseButton mb)
{
    Input *inp = Input::GetActive();
    return inp->m_mouseInfo.ContainsKey(mb) && inp->m_mouseInfo[mb].up;
}

bool Input::GetMouseButtonDown(MouseButton mb)
{
    Input *inp = Input::GetActive();
    return inp->m_mouseInfo.ContainsKey(mb) && inp->m_mouseInfo[mb].down;
}

bool Input::GetMouseButtonDoubleClick(MouseButton mb)
{
    Input *inp = Input::GetActive();
    return Input::GetMouseButtonUp(mb) && inp->m_isADoubleClick;
}

bool Input::IsMouseInsideWindow()
{
    return Input::GetActive()->m_isMouseInside;
}

float Input::GetMouseAxisX()
{
    return SCAST<float>(Input::GetMouseDeltaX()) / GL::GetViewportSize().x;
}

float Input::GetMouseAxisY()
{
    return SCAST<float>(Input::GetMouseDeltaY()) / GL::GetViewportSize().y;
}

Vector2 Input::GetMouseAxis()
{
    return Vector2(Input::GetMouseAxisX(), Input::GetMouseAxisY());
}

int Input::GetMouseDeltaX()
{
    Input *inp = Input::GetActive();
    int delta = inp->GetMousePositionWindow().x -
                Input::GetPreviousMousePositionWindow().x;
    if (Math::Abs(delta) > Window::GetActive()->GetWidth() * 0.8f)
    {
        delta = 0;
    }
    return delta;
}

int Input::GetMouseDeltaY()
{
    Input *inp = Input::GetActive();
    int delta = inp->GetMousePositionWindow().y -
                Input::GetPreviousMousePositionWindow().y;
    if (Math::Abs(delta) > Window::GetActive()->GetHeight() * 0.8f)
    {
        delta = 0;
    }
    return delta;
}

Vector2i Input::GetMouseDelta()
{
    return Vector2i(Input::GetMouseDeltaX(), Input::GetMouseDeltaY());
}

void Input::SetMouseWrapping(bool isMouseWrapping)
{
    Input::GetActive()->m_isMouseWrapping = isMouseWrapping;
}

bool Input::IsMouseWrapping()
{
    return Input::GetActive()->m_isMouseWrapping;
}

void Input::LockMouseMovement(bool lock)
{
    Input *inp = Input::GetActive();
    inp->m_lockMouseMovement = lock;
}

bool Input::IsLockMouseMovement()
{
    Input *inp = Input::GetActive();
    return inp->m_lockMouseMovement;
}

void Input::SetMousePositionWindow(int windowMousePosX, int windowMousePosY)
{
    Window *window = Window::GetActive();
    SDL_WarpMouseInWindow(window->GetSDLWindow(),
                          windowMousePosX,  windowMousePosY);
}

void Input::SetMousePositionWindow(const Vector2i &windowMousePosition)
{
    Input::SetMousePositionWindow(windowMousePosition.x, windowMousePosition.y);
}

Vector2i Input::GetMousePosition()
{
    return Vector2i(
              GL::FromWindowPointToViewportPoint( GetMousePositionWindow() ) );
}
Vector2 Input::GetMousePositionNDC()
{
    return GL::FromViewportPointToViewportPointNDC( Input::GetMousePosition() );
}

Vector2i Input::GetMousePositionWindow()
{
    Vector2i winPosInvertedY = GetWindowPosInvertedY(
                                    GetMousePositionWindowWithoutInvertY() );
    return winPosInvertedY;
}

Vector2i Input::GetWindowPosInvertedY(const Vector2i &winPos)
{
    Vector2i winPosInverted = winPos;
    if (Window *win = Window::GetActive())
    {
        winPosInverted.y = (win->GetHeight()-1) - winPos.y; // Invert Y
    }
    return winPosInverted;
}

Vector2 Input::GetMousePositionWindowNDC()
{
    return GL::FromWindowPointToWindowPointNDC( Input::GetMousePositionWindow() );
}

Vector2i Input::GetPreviousMousePositionWindow()
{
    Input *inp = Input::GetActive();
    return inp->m_lastMousePosWindow;
}

Vector2i Input::GetMousePositionWindowWithoutInvertY()
{
    Vector2i winPos;
    SDL_GetMouseState(&winPos.x, &winPos.y);
    return winPos;
}

void Input::StartTextInput()
{
    SDL_StartTextInput();
}

String Input::PollInputText()
{
    Input *input = Input::GetActive();
    String res = input->m_inputText;
    input->m_inputText = "";
    return res;
}

void Input::StopTextInput()
{
    SDL_StopTextInput();
}

void Input::Reset()
{
    m_isADoubleClick = m_lockMouseMovement = m_isMouseInside = false;
    m_lastMouseWheelDelta = Vector2::Zero;
    m_lastMousePosWindow = Vector2i(-1);
    m_lastMouseDownTimestamp = 0.0f;
    m_inputText = "";

    m_keysUp.Clear();
    m_keysDown.Clear();
    m_pressedKeys.Clear();
    m_keysDownRepeat.Clear();

    m_keyInfos.Clear();
    m_mouseInfo.Clear();
    m_eventInfoQueue.Clear();
}

bool InputEvent::IsMouseType() const
{
    return (type == Type::MOUSE_DOWN ||
            type == Type::MOUSE_MOVE ||
            type == Type::MOUSE_UP);
}

Vector2i InputEvent::GetMousePosWindow() const
{
    if (IsMouseType())
    {
        return mousePosWindow;
    }
    return Input::GetMousePositionWindow();
}

Input::ButtonInfo::ButtonInfo(bool up, bool down, bool pressed)
{
    this->up = up;
    this->down = down;
    this->pressed = pressed;
}
