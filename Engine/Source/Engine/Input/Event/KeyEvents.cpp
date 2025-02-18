#include "KeyEvents.hpp"

namespace eng
{
    KeyPressEvent::KeyPressEvent(Keycode keycode, Modifiers modifiers) noexcept
        : Event(GetStaticType())
        , m_Keycode(keycode)
        , m_Modifiers(modifiers)
    {

    }

    Keycode KeyPressEvent::GetKeycode() const noexcept
    {
        return m_Keycode;
    }

    Modifiers KeyPressEvent::GetModifiers() const noexcept
    {
        return m_Modifiers;
    }


    
    KeyReleaseEvent::KeyReleaseEvent(Keycode keycode, Modifiers modifiers) noexcept
        : Event(GetStaticType())
        , m_Keycode(keycode)
        , m_Modifiers(modifiers)
    {

    }

    Keycode KeyReleaseEvent::GetKeycode() const noexcept
    {
        return m_Keycode;
    }

    Modifiers KeyReleaseEvent::GetModifiers() const noexcept
    {
        return m_Modifiers;
    }



    KeyCharTypeEvent::KeyCharTypeEvent(std::uint32_t codepoint) noexcept
        : Event(GetStaticType())
        , m_Codepoint(codepoint)
    {

    }

    std::uint32_t KeyCharTypeEvent::GetCodepoint() const noexcept
    {
        return m_Codepoint;
    }
}
