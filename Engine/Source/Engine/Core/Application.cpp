#include "Application.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include <glfw/glfw3.h>

namespace eng
{
    void Application::Terminate()
    {
        m_Running = false;
    }

    Application::Application(ApplicationInfo const& info)
        : m_Window(info.WindowInfo)
    {
        // Send initial framebuffer resize event to initialize all Client systems.
        u32 width, height;
        m_Window.GetFramebufferSize(width, height);
        WindowFramebufferResizeEvent event(width, height);
        m_Window.OnEvent(event);
    }

    void Application::Run()
    {
        while (m_Running)
        {
            glfwPollEvents();

            m_Window.OnUpdate();
            m_Window.OnRender();
        }
    }
}
