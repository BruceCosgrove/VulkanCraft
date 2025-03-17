#pragma once

#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Event/Event.hpp"
#include "Engine/Input/MouseButtons.hpp"
#include "Engine/Input/Modifiers.hpp"

namespace eng
{
    class MouseButtonPressEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::MouseButtonPress, EventCategory::Mouse);
        MouseButtonPressEvent(MouseButton button, Modifiers modifiers, bool pressed) noexcept;

        MouseButton GetButton() const noexcept;
        Modifiers GetModifiers() const noexcept;
        bool IsPressed() const noexcept;
    private:
        MouseButton m_Button;
        Modifiers m_Modifiers;
        bool m_Pressed;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseButtonPressEvent);

    

    class MouseMoveEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::MouseMove, EventCategory::Mouse);
        MouseMoveEvent(f32 x, f32 y) noexcept;

        f32 GetX() const noexcept;
        f32 GetY() const noexcept;
    private:
        f32 m_X;
        f32 m_Y;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseMoveEvent);


    
    class MouseEnterEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::MouseEnter, EventCategory::Mouse);
        MouseEnterEvent(bool entered) noexcept;

        bool IsEntered() const noexcept;
    private:
        bool m_Entered;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseEnterEvent);



    class MouseScrollEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::MouseScroll, EventCategory::Mouse);
        MouseScrollEvent(f32 scrollX, f32 scrollY) noexcept;

        f32 GetScrollX() const noexcept;
        f32 GetScrollY() const noexcept;
    private:
        f32 m_ScrollX;
        f32 m_ScrollY;
    };
    _ENG_ASSERT_EVENT_INTERFACE(MouseScrollEvent);
}

_ENG_EVENT_FORMAT(eng::MouseButtonPressEvent,
    "MouseButtonPressEvent(Button={}, Modifiers={}, Pressed={})",
    event.GetButton(), event.GetModifiers(), event.IsPressed()
);

_ENG_EVENT_FORMAT(eng::MouseMoveEvent,
    "MouseMoveEvent(X={}, Y={})",
    event.GetX(), event.GetY()
);

_ENG_EVENT_FORMAT(eng::MouseEnterEvent,
    "MouseEnterEvent(Entered={})",
    event.IsEntered()
);

_ENG_EVENT_FORMAT(eng::MouseScrollEvent,
    "MouseScrollEvent(ScrollX={}, ScrollY={})",
    event.GetScrollX(), event.GetScrollY()
);
