#pragma once

#include "Engine/Input/Event/Event.hpp"
#include "Engine/Input/Modifiers.hpp"
#include "Engine/Input/Keycode.hpp"

namespace eng
{
    class KeyPressEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::KeyPress);
        KeyPressEvent(Keycode keycode, Modifiers modifiers) noexcept;

        Keycode GetKeycode() const noexcept;
        Modifiers GetModifiers() const noexcept;
    private:
        Keycode m_Keycode;
        Modifiers m_Modifiers;
    };
    _ENG_ASSERT_EVENT_INTERFACE(KeyPressEvent);



    class KeyReleaseEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::KeyRelease);
        KeyReleaseEvent(Keycode keycode, Modifiers modifiers) noexcept;

        Keycode GetKeycode() const noexcept;
        Modifiers GetModifiers() const noexcept;
    private:
        Keycode m_Keycode;
        Modifiers m_Modifiers;
    };
    _ENG_ASSERT_EVENT_INTERFACE(KeyReleaseEvent);



    class KeyCharTypeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::KeyCharType);
        KeyCharTypeEvent(std::uint32_t codepoint) noexcept;

        std::uint32_t GetCodepoint() const noexcept;
    private:
        std::uint32_t m_Codepoint;
    };
    _ENG_ASSERT_EVENT_INTERFACE(KeyCharTypeEvent);
}
