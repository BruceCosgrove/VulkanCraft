#pragma once

#include "Engine/Input/Event/Event.hpp"
#include <span>

namespace eng
{
    class WindowMoveEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowMove);
        WindowMoveEvent(std::int32_t x, std::int32_t y) noexcept;

        std::int32_t GetX() const noexcept;
        std::int32_t GetY() const noexcept;
    private:
        std::int32_t m_X;
        std::int32_t m_Y;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowMoveEvent);
    


    class WindowResizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowResize);
        WindowResizeEvent(std::uint32_t width, std::uint32_t height) noexcept;

        std::uint32_t GetWidth() const noexcept;
        std::uint32_t GetHeight() const noexcept;
    private:
        std::uint32_t m_Width;
        std::uint32_t m_Height;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowResizeEvent);



    class WindowCloseEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowClose);
        WindowCloseEvent() noexcept;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowCloseEvent);



    class WindowRefreshEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowRefresh);
        WindowRefreshEvent() noexcept;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowRefreshEvent);
    


    class WindowFocusEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowFocus);
        WindowFocusEvent(bool focused) noexcept;

        bool IsFocused() const noexcept;
    private:
        bool m_Focused;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowFocusEvent);



    class WindowMinimizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowMinimize);
        WindowMinimizeEvent(bool minimized) noexcept;

        bool IsMinimized() const noexcept;
    private:
        bool m_Minimized;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowMinimizeEvent);



    class WindowMaximizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowMaximize);
        WindowMaximizeEvent(bool maximized) noexcept;

        bool IsMaximized() const noexcept;
    private:
        bool m_Maximized;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowMaximizeEvent);



    class WindowFramebufferResizeEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowFramebufferResize);
        WindowFramebufferResizeEvent(std::uint32_t framebufferWidth, std::uint32_t framebufferHeight) noexcept;

        std::uint32_t GetFramebufferWidth() const noexcept;
        std::uint32_t GetFramebufferHeight() const noexcept;
    private:
        std::uint32_t m_FramebufferWidth;
        std::uint32_t m_FramebufferHeight;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowFramebufferResizeEvent);



    class WindowContentScaleEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowContentScale);
        WindowContentScaleEvent(float contentScaleX, float contentScaleY) noexcept;

        float GetContentScaleX() const noexcept;
        float GetContentScaleY() const noexcept;
    private:
        float m_ContentScaleX;
        float m_ContentScaleY;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowContentScaleEvent);



    class WindowPathDropEvent : public Event
    {
    public:
        _ENG_EVENT_GET_STATIC_TYPE(EventType::WindowPathDrop);
        WindowPathDropEvent(std::span<char const*> paths) noexcept;

        std::span<char const* const> GetPaths() const noexcept;
    private:
        std::span<char const*> m_Paths;
    };
    _ENG_ASSERT_EVENT_INTERFACE(WindowPathDropEvent);
}
