#include "MouseEvents.hpp"

namespace eng
{
    MouseButtonPressEvent::MouseButtonPressEvent(MouseButton button, Modifiers modifiers, bool pressed) noexcept
        : Event(GetStaticType())
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



    MouseMoveEvent::MouseMoveEvent(float x, float y) noexcept
        : Event(GetStaticType())
        , m_X(x)
        , m_Y(y)
    {

    }

    float MouseMoveEvent::GetX() const noexcept
    {
        return m_X;
    }

    float MouseMoveEvent::GetY() const noexcept
    {
        return m_Y;
    }



    MouseEnterEvent::MouseEnterEvent(bool entered) noexcept
        : Event(GetStaticType())
        , m_Entered(entered)
    {

    }

    bool MouseEnterEvent::IsEntered() const noexcept
    {
        return m_Entered;
    }



    MouseScrollEvent::MouseScrollEvent(float scrollX, float scrollY) noexcept
        : Event(GetStaticType())
        , m_ScrollX(scrollX)
        , m_ScrollY(scrollY)
    {

    }

    float MouseScrollEvent::GetScrollX() const noexcept
    {
        return m_ScrollX;
    }

    float MouseScrollEvent::GetScrollY() const noexcept
    {
        return m_ScrollY;
    }
}
