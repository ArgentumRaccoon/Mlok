#include "Input.h"

#include "Event.h"

void InputSystem::Initialize()
{

}

void InputSystem::Shutdown()
{

}

void InputSystem::Update(double DeltaTime)
{
    KeyboardPreviousState = KeyboardCurrentState;
    MousePreviousState = MouseCurrentState;
}

bool InputSystem::IsKeyDown(KeyboardKey Key)
{
    return KeyboardCurrentState.Keys[Key];
}

bool InputSystem::IsKeyUp(KeyboardKey Key)
{
    return !KeyboardCurrentState.Keys[Key];
}

bool InputSystem::WasKeyDown(KeyboardKey Key)
{
    return KeyboardPreviousState.Keys[Key];
}

bool InputSystem::WasKeyUp(KeyboardKey Key)
{
    return !KeyboardPreviousState.Keys[Key];
}

bool InputSystem::IsKeyDown(const char KeyChar)
{
    return IsKeyDown(static_cast<KeyboardKey>(KeyChar));
}

bool InputSystem::IsKeyUp(const char KeyChar)
{
    return !IsKeyDown(static_cast<KeyboardKey>(KeyChar));
}

bool InputSystem::WasKeyDown(const char KeyChar)
{
    return WasKeyDown(static_cast<KeyboardKey>(KeyChar));
}

bool InputSystem::WasKeyUp(const char KeyChar)
{
    return !WasKeyDown(static_cast<KeyboardKey>(KeyChar));
}

void InputSystem::ProcessKey(KeyboardKey Key, bool bPressed)
{
    if (KeyboardCurrentState.Keys[Key] != bPressed)
    {
        KeyboardCurrentState.Keys[Key] = bPressed;

        EventContext Context;
        Context.Data.u16[0] = Key;
        EventSystem::Get()->FireEvent(bPressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, this, Context);
    }
}

bool InputSystem::IsMouseButtonDown(MouseButton Button)
{
    return MouseCurrentState.Buttons[static_cast<size_t>(Button)];
}

bool InputSystem::IsMouseButtonUp(MouseButton Button)
{
    return !MouseCurrentState.Buttons[static_cast<size_t>(Button)];
}

bool InputSystem::WasMouseButtonDown(MouseButton Button)
{
    return MousePreviousState.Buttons[static_cast<size_t>(Button)];
}

bool InputSystem::WasMouseButtonUp(MouseButton Button)
{
    return !MousePreviousState.Buttons[static_cast<size_t>(Button)];
}

void InputSystem::GetMousePosition(int32_t* X, int32_t* Y)
{
    *X = MouseCurrentState.XPos;
    *Y = MouseCurrentState.YPos;
}

void InputSystem::GetPreviousMousePosition(int32_t* X, int32_t* Y)
{
    *X = MousePreviousState.XPos;
    *Y = MousePreviousState.YPos;
}

void InputSystem::ProcessMouseButton(MouseButton Button, bool bPressed)
{
    if (MouseCurrentState.Buttons[static_cast<size_t>(Button)] != bPressed)
    {
        EventContext Context;
        Context.Data.u16[0] = static_cast<uint16_t>(Button);
        EventSystem::Get()->FireEvent(bPressed ? EVENT_CODE_MOUSE_BUTTON_PRESSED : EVENT_CODE_MOUSE_BUTTON_RELEASED, this, Context);
    }
}

void InputSystem::ProcessMouseMove(int16_t X, int16_t Y)
{
    if (MouseCurrentState.XPos != X || MouseCurrentState.YPos != Y)
    {
        MouseCurrentState.XPos = X;
        MouseCurrentState.YPos = Y;

        EventContext Context;
        Context.Data.u16[0] = X;
        Context.Data.u16[1] = Y;
        EventSystem::Get()->FireEvent(EVENT_CODE_MOUSE_MOVED, this, Context);
    }
}

void InputSystem::ProcessMouseWheel(int8_t WheelDelta)
{
    EventContext Context;
    Context.Data.u8[0] = WheelDelta;
    EventSystem::Get()->FireEvent(EVENT_CODE_MOUSE_WHEEL, this, Context);
}