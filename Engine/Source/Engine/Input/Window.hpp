#pragma once

#include "Engine/Input/Event/Event.hpp"
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
        Window(WindowInfo const& info, std::function<void(Event&)>&& eventCallback);
        ~Window();
    private:
        std::function<void(Event&)> m_EventCallback;
        GLFWwindow* m_Window = nullptr;
    private:
        inline static std::uint32_t s_WindowCount = 0;
    };
}
