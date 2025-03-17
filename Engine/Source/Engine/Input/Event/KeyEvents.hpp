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
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::KeyPress, EventCategory::Key);
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



    class KeyTypeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::KeyType, EventCategory::Key);
        KeyTypeEvent(u32 codepoint) noexcept;

        u32 GetCodepoint() const noexcept;
    private:
        u32 m_Codepoint;
    };
    _ENG_ASSERT_EVENT_INTERFACE(KeyTypeEvent);
}

_ENG_EVENT_FORMAT(eng::KeyPressEvent,
    "KeyPressEvent(Keycode={}, Modifiers={}, Pressed={})",
    event.GetKeycode(), event.GetModifiers(), event.IsPressed()
);

_ENG_EVENT_FORMAT(eng::KeyTypeEvent,
    "KeyTypeEvent(Codepoint={})",
    event.GetCodepoint()
);
