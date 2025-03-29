#include "WindowEvents.hpp"

namespace eng
{
    WindowMoveEvent::WindowMoveEvent(i32 x, i32 y) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
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
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
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
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
    {

    }



    WindowRefreshEvent::WindowRefreshEvent() noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
    {

    }



    WindowFocusEvent::WindowFocusEvent(bool focused) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_Focused(focused)
    {

    }

    bool WindowFocusEvent::IsFocused() const noexcept
    {
        return m_Focused;
    }



    WindowMinimizeEvent::WindowMinimizeEvent(bool minimized) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_Minimized(minimized)
    {

    }

    bool WindowMinimizeEvent::IsMinimized() const noexcept
    {
        return m_Minimized;
    }



    WindowMaximizeEvent::WindowMaximizeEvent(bool maximized) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
        , m_Maximized(maximized)
    {

    }

    bool WindowMaximizeEvent::IsMaximized() const noexcept
    {
        return m_Maximized;
    }



    WindowFramebufferResizeEvent::WindowFramebufferResizeEvent(u32 framebufferWidth, u32 framebufferHeight) noexcept
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
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
        : Event(GetStaticType(), GetStaticCategories(), sizeof(*this))
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
}
