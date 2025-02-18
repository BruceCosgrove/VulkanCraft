#pragma once

#include "Engine/Input/Event/Event.hpp"
#include "Engine/Input/MouseButtons.hpp"
#include "Engine/Input/Modifiers.hpp"

namespace eng
{
    class MouseButtonPressEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::MouseButtonPress);
        MouseButtonPressEvent(MouseButton button, Modifiers modifiers) noexcept;

        MouseButton GetButton() const noexcept;
        Modifiers GetModifiers() const noexcept;
    private:
        MouseButton m_Button;
        Modifiers m_Modifiers;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseButtonPressEvent);



    class MouseButtonReleaseEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::MouseButtonRelease);
        MouseButtonReleaseEvent(MouseButton button, Modifiers modifiers) noexcept;

        MouseButton GetButton() const noexcept;
        Modifiers GetModifiers() const noexcept;
    private:
        MouseButton m_Button;
        Modifiers m_Modifiers;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseButtonReleaseEvent);

    

    class MouseMoveEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::MouseMove);
        MouseMoveEvent(float x, float y) noexcept;

        float GetX() const noexcept;
        float GetY() const noexcept;
    private:
        float m_X;
        float m_Y;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseMoveEvent);


    
    class MouseEnterEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::MouseEnter);
        MouseEnterEvent(bool entered) noexcept;

        bool IsEntered() const noexcept;
    private:
        bool m_Entered;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseEnterEvent);



    class MouseScrollEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::MouseScroll);
        MouseScrollEvent(float scrollX, float scrollY) noexcept;

        float GetScrollX() const noexcept;
        float GetScrollY() const noexcept;
    private:
        float m_ScrollX;
        float m_ScrollY;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseScrollEvent);
}
