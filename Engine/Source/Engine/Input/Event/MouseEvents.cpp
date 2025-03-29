#include "MouseEvents.hpp"

namespace eng
{
    MouseButtonPressEvent::MouseButtonPressEvent(MouseButton button, Modifiers modifiers, bool pressed) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_Button(button)
        , m_Modifiers(modifiers)
        , m_Pressed(pressed)
    {

    }

    MouseButton MouseButtonPressEvent::GetButton() const noexcept
    {
        return m_Button;
    }

    Modifiers MouseButtonPressEvent::GetModifiers() const noexcept
    {
        return m_Modifiers;
    }

    bool MouseButtonPressEvent::IsPressed() const noexcept
    {
        return m_Pressed;
    }



    MouseMoveEvent::MouseMoveEvent(f32 x, f32 y) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_X(x)
        , m_Y(y)
    {

    }

    f32 MouseMoveEvent::GetX() const noexcept
    {
        return m_X;
    }

    f32 MouseMoveEvent::GetY() const noexcept
    {
        return m_Y;
    }



    MouseEnterEvent::MouseEnterEvent(bool entered) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_Entered(entered)
    {

    }

    bool MouseEnterEvent::IsEntered() const noexcept
    {
        return m_Entered;
    }



    MouseScrollEvent::MouseScrollEvent(f32 scrollX, f32 scrollY) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_ScrollX(scrollX)
        , m_ScrollY(scrollY)
    {

    }

    f32 MouseScrollEvent::GetScrollX() const noexcept
    {
        return m_ScrollX;
    }

    f32 MouseScrollEvent::GetScrollY() const noexcept
    {
        return m_ScrollY;
    }
}
