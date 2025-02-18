#include "WindowEvents.hpp"

namespace eng
{
    WindowMoveEvent::WindowMoveEvent(std::int32_t x, std::int32_t y) noexcept
        : Event(GetStaticType())
        , m_X(x)
        , m_Y(y)
    {

    }

    std::int32_t WindowMoveEvent::GetX() const noexcept
    {
        return m_X;
    }

    std::int32_t WindowMoveEvent::GetY() const noexcept
    {
        return m_Y;
    }



    WindowResizeEvent::WindowResizeEvent(std::uint32_t width, std::uint32_t height) noexcept
        : Event(GetStaticType())
        , m_Width(width)
        , m_Height(height)
    {

    }

    std::uint32_t WindowResizeEvent::GetWidth() const noexcept
    {
        return m_Width;
    }

    std::uint32_t WindowResizeEvent::GetHeight() const noexcept
    {
        return m_Height;
    }



    WindowCloseEvent::WindowCloseEvent() noexcept
        : Event(GetStaticType())
    {

    }



    WindowRefreshEvent::WindowRefreshEvent() noexcept
        : Event(GetStaticType())
    {

    }



    WindowFocusEvent::WindowFocusEvent(bool focused) noexcept
        : Event(GetStaticType())
        , m_Focused(focused)
    {

    }

    bool WindowFocusEvent::IsFocused() const noexcept
    {
        return m_Focused;
    }



    WindowMinimizeEvent::WindowMinimizeEvent(bool minimized) noexcept
        : Event(GetStaticType())
        , m_Minimized(minimized)
    {

    }

    bool WindowMinimizeEvent::IsMinimized() const noexcept
    {
        return m_Minimized;
    }



    WindowMaximizeEvent::WindowMaximizeEvent(bool maximized) noexcept
        : Event(GetStaticType())
        , m_Maximized(maximized)
    {

    }

    bool WindowMaximizeEvent::IsMaximized() const noexcept
    {
        return m_Maximized;
    }



    WindowFramebufferResizeEvent::WindowFramebufferResizeEvent(std::uint32_t framebufferWidth, std::uint32_t framebufferHeight) noexcept
        : Event(GetStaticType())
        , m_FramebufferWidth(framebufferWidth)
        , m_FramebufferHeight(framebufferHeight)
    {

    }

    std::uint32_t WindowFramebufferResizeEvent::GetFramebufferWidth() const noexcept
    {
        return m_FramebufferWidth;
    }

    std::uint32_t WindowFramebufferResizeEvent::GetFramebufferHeight() const noexcept
    {
        return m_FramebufferHeight;
    }



    WindowContentScaleEvent::WindowContentScaleEvent(float contentScaleX, float contentScaleY) noexcept
        : Event(GetStaticType())
        , m_ContentScaleX(contentScaleX)
        , m_ContentScaleY(contentScaleY)
    {

    }

    float WindowContentScaleEvent::GetContentScaleX() const noexcept
    {
        return m_ContentScaleX;
    }

    float WindowContentScaleEvent::GetContentScaleY() const noexcept
    {
        return m_ContentScaleY;
    }



    WindowPathDropEvent::WindowPathDropEvent(std::span<char const*> paths) noexcept
        : Event(GetStaticType())
        , m_Paths(paths)
    {

    }

    std::span<char const* const> WindowPathDropEvent::GetPaths() const noexcept
    {
        return m_Paths;
    }
}
