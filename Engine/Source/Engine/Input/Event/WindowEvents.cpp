#include "WindowEvents.hpp"

namespace eng
{
    WindowMoveEvent::WindowMoveEvent(i32 x, i32 y) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_X(x)
        , m_Y(y)
    {

    }

    i32 WindowMoveEvent::GetX() const noexcept
    {
        return m_X;
    }

    i32 WindowMoveEvent::GetY() const noexcept
    {
        return m_Y;
    }



    WindowResizeEvent::WindowResizeEvent(u32 width, u32 height) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Width(width)
        , m_Height(height)
    {

    }

    u32 WindowResizeEvent::GetWidth() const noexcept
    {
        return m_Width;
    }

    u32 WindowResizeEvent::GetHeight() const noexcept
    {
        return m_Height;
    }



    WindowCloseEvent::WindowCloseEvent() noexcept
        : Event(GetStaticType(), GetStaticCategories())
    {

    }



    WindowRefreshEvent::WindowRefreshEvent() noexcept
        : Event(GetStaticType(), GetStaticCategories())
    {

    }



    WindowFocusEvent::WindowFocusEvent(bool focused) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Focused(focused)
    {

    }

    bool WindowFocusEvent::IsFocused() const noexcept
    {
        return m_Focused;
    }



    WindowMinimizeEvent::WindowMinimizeEvent(bool minimized) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Minimized(minimized)
    {

    }

    bool WindowMinimizeEvent::IsMinimized() const noexcept
    {
        return m_Minimized;
    }



    WindowMaximizeEvent::WindowMaximizeEvent(bool maximized) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Maximized(maximized)
    {

    }

    bool WindowMaximizeEvent::IsMaximized() const noexcept
    {
        return m_Maximized;
    }



    WindowFramebufferResizeEvent::WindowFramebufferResizeEvent(u32 framebufferWidth, u32 framebufferHeight) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_FramebufferWidth(framebufferWidth)
        , m_FramebufferHeight(framebufferHeight)
    {

    }

    u32 WindowFramebufferResizeEvent::GetFramebufferWidth() const noexcept
    {
        return m_FramebufferWidth;
    }

    u32 WindowFramebufferResizeEvent::GetFramebufferHeight() const noexcept
    {
        return m_FramebufferHeight;
    }



    WindowContentScaleEvent::WindowContentScaleEvent(f32 contentScaleX, f32 contentScaleY) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_ContentScaleX(contentScaleX)
        , m_ContentScaleY(contentScaleY)
    {

    }

    f32 WindowContentScaleEvent::GetContentScaleX() const noexcept
    {
        return m_ContentScaleX;
    }

    f32 WindowContentScaleEvent::GetContentScaleY() const noexcept
    {
        return m_ContentScaleY;
    }



    WindowPathDropEvent::WindowPathDropEvent(std::span<char const*> paths) noexcept
        : Event(GetStaticType(), GetStaticCategories())
        , m_Paths(paths)
    {

    }

    std::span<char const* const> WindowPathDropEvent::GetPaths() const noexcept
    {
        return m_Paths;
    }
}
