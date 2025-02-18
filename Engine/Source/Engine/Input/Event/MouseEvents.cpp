#include "MouseEvents.hpp"

namespace eng
{
    MouseButtonPressEvent::MouseButtonPressEvent(MouseButton button, Modifiers modifiers) noexcept
        : Event(EventType::MouseButtonPress)
        , m_Button(button)
        , m_Modifiers(modifiers)
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



    MouseButtonReleaseEvent::MouseButtonReleaseEvent(MouseButton button, Modifiers modifiers) noexcept
        : Event(EventType::MouseButtonRelease)
        , m_Button(button)
        , m_Modifiers(modifiers)
    {

    }

    MouseButton MouseButtonReleaseEvent::GetButton() const noexcept
    {
        return m_Button;
    }

    Modifiers MouseButtonReleaseEvent::GetModifiers() const noexcept
    {
        return m_Modifiers;
    }



    MouseMoveEvent::MouseMoveEvent(float x, float y) noexcept
        : Event(EventType::MouseMove)
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
        : Event(EventType::MouseMove)
        , m_Entered(entered)
    {

    }

    bool MouseEnterEvent::IsEntered() const noexcept
    {
        return m_Entered;
    }



    MouseScrollEvent::MouseScrollEvent(float scrollX, float scrollY) noexcept
        : Event(EventType::MouseMove)
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
