#include "KeyEvents.hpp"

namespace eng
{
    KeyPressEvent::KeyPressEvent(Keycode keycode, Modifiers modifiers, bool pressed) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Keycode(keycode)
        , m_Modifiers(modifiers)
        , m_Pressed(pressed)
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

    bool KeyPressEvent::IsPressed() const noexcept
    {
        return m_Pressed;
    }



    KeyTypeEvent::KeyTypeEvent(u32 codepoint) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Codepoint(codepoint)
    {

    }

    u32 KeyTypeEvent::GetCodepoint() const noexcept
    {
        return m_Codepoint;
    }
}
