#include "Application.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include "Engine/Threading/ThreadTracer.hpp"
#include <glfw/glfw3.h>

namespace eng
{
    void Application::Terminate()
    {
        m_Running.store(false, std::memory_order_relaxed);
    }

    bool Application::IsRunning() const
    {
        return m_Running.load(std::memory_order_relaxed);
    }

    void Application::UpdateThread()
    {
        ThreadTracer tracer("update");
        while (m_Running.load(std::memory_order_relaxed))
            m_Window.OnUpdate();
    }

    void Application::RenderThread()
    {
        ThreadTracer tracer("render");
        while (m_Running.load(std::memory_order_relaxed))
            m_Window.OnRender();
    }

    void Application::EventThread()
    {
        ThreadTracer tracer("event");
        while (m_Running.load(std::memory_order_relaxed))
            glfwWaitEvents();
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
        // Run the update thread on its own thread.
        std::jthread updateThread([this] { UpdateThread(); });
        // Run the render thread on its own thread.
        std::jthread renderThread([this] { RenderThread(); });
        // Run the event thread on the main thread.
        EventThread();
    }
}
