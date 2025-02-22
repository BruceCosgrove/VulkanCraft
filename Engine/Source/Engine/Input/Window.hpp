#pragma once

#include "Engine/Input/Event/Event.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include <functional>
#include <string>

struct GLFWwindow;

namespace eng
{
    struct WindowInfo
    {
        std::string Title = "TODO: Window Title";
        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;
        bool Resizable = true;
    };

    class Window
    {
    public:
        GLFWwindow* GetNativeWindow();
        RenderContext& GetRenderContext();
    private:
        friend class Application;
        Window(WindowInfo const& info, std::function<void(Event&)>&& eventCallback, std::function<void()>&& renderCallback);
        ~Window();
    private:
        GLFWwindow* m_Window = nullptr;
        std::function<void(Event&)> m_EventCallback;
        std::unique_ptr<RenderContext> m_RenderContext;
    private:
        inline static std::uint32_t s_WindowCount = 0;
    };
}
