#pragma once

#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Event/Event.hpp"
#include <span>

namespace eng
{
    class WindowMoveEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowMove, EventCategory::Window);
        WindowMoveEvent(i32 x, i32 y) noexcept;

        i32 GetX() const noexcept;
        i32 GetY() const noexcept;
    private:
        i32 m_X;
        i32 m_Y;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowMoveEvent);
    


    class WindowResizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowResize, EventCategory::Window);
        WindowResizeEvent(u32 width, u32 height) noexcept;

        u32 GetWidth() const noexcept;
        u32 GetHeight() const noexcept;
    private:
        u32 m_Width;
        u32 m_Height;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowResizeEvent);



    class WindowCloseEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowClose, EventCategory::Window);
        WindowCloseEvent() noexcept;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowCloseEvent);



    class WindowRefreshEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowRefresh, EventCategory::Window);
        WindowRefreshEvent() noexcept;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowRefreshEvent);
    


    class WindowFocusEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowFocus, EventCategory::Window);
        WindowFocusEvent(bool focused) noexcept;

        bool IsFocused() const noexcept;
    private:
        bool m_Focused;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowFocusEvent);



    class WindowMinimizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowMinimize, EventCategory::Window);
        WindowMinimizeEvent(bool minimized) noexcept;

        bool IsMinimized() const noexcept;
    private:
        bool m_Minimized;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowMinimizeEvent);



    class WindowMaximizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowMaximize, EventCategory::Window);
        WindowMaximizeEvent(bool maximized) noexcept;

        bool IsMaximized() const noexcept;
    private:
        bool m_Maximized;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowMaximizeEvent);



    class WindowFramebufferResizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowFramebufferResize, EventCategory::Window);
        WindowFramebufferResizeEvent(u32 framebufferWidth, u32 framebufferHeight) noexcept;

        u32 GetFramebufferWidth() const noexcept;
        u32 GetFramebufferHeight() const noexcept;
    private:
        u32 m_FramebufferWidth;
        u32 m_FramebufferHeight;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowFramebufferResizeEvent);



    class WindowContentScaleEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_FUNCS(EventType::WindowContentScale, EventCategory::Window);
        WindowContentScaleEvent(f32 contentScaleX, f32 contentScaleY) noexcept;

        f32 GetContentScaleX() const noexcept;
        f32 GetContentScaleY() const noexcept;
    private:
        f32 m_ContentScaleX;
        f32 m_ContentScaleY;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowContentScaleEvent);
}

_ENG_EVENT_FORMAT(eng::WindowMoveEvent,
    "WindowMoveEvent(X={}, Y={})",
    event.GetX(), event.GetY()
);

_ENG_EVENT_FORMAT(eng::WindowResizeEvent,
    "WindowResizeEvent(Width={}, Height={})",
    event.GetWidth(), event.GetHeight()
);

_ENG_EVENT_FORMAT(eng::WindowCloseEvent,
    "WindowCloseEvent"
);

_ENG_EVENT_FORMAT(eng::WindowRefreshEvent,
    "WindowRefreshEvent",
);

_ENG_EVENT_FORMAT(eng::WindowFocusEvent,
    "WindowFocusEvent(Focused={})",
    event.IsFocused()
);

_ENG_EVENT_FORMAT(eng::WindowMinimizeEvent,
    "WindowMinimizeEvent(Minimized={})",
    event.IsMinimized()
);

_ENG_EVENT_FORMAT(eng::WindowMaximizeEvent,
    "WindowMaximizeEvent(Maximized={})",
    event.IsMaximized()
);

_ENG_EVENT_FORMAT(eng::WindowFramebufferResizeEvent,
    "WindowFramebufferResizeEvent(FramebufferWidth={}, FramebufferHeight={})",
    event.GetFramebufferWidth(), event.GetFramebufferHeight()
);

_ENG_EVENT_FORMAT(eng::WindowContentScaleEvent,
    "WindowContentScaleEvent(ContentScaleX={}, ContentScaleX={})",
    event.GetContentScaleX(), event.GetContentScaleY()
);
