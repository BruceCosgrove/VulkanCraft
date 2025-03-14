#pragma once

#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Event/Event.hpp"
#include "Engine/Input/Modifiers.hpp"
#include "Engine/Input/Keycode.hpp"

namespace eng
{
    class KeyPressEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::KeyPress);
        KeyPressEvent(Keycode keycode, Modifiers modifiers, bool pressed) noexcept;

        Keycode GetKeycode() const noexcept;
        Modifiers GetModifiers() const noexcept;
        bool IsPressed() const noexcept;
    private:
        Keycode m_Keycode;
        Modifiers m_Modifiers;
        bool m_Pressed;
    };
    _ENG_ASSERT_EVENT_INTERFACE(KeyPressEvent);



    class KeyCharTypeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::KeyCharType);
        KeyCharTypeEvent(u32 codepoint) noexcept;

        u32 GetCodepoint() const noexcept;
    private:
        u32 m_Codepoint;
    };
    _ENG_ASSERT_EVENT_INTERFACE(KeyCharTypeEvent);
}
