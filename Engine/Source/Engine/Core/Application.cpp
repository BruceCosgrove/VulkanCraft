#include "Application.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include <glfw/glfw3.h>

namespace eng
{
    void Application::Terminate()
    {
        m_Running.store(false, std::memory_order_release);
    }

    bool Application::IsRunning() const
    {
        return m_Running.load(std::memory_order_acquire);
    }

    void Application::ExecuteAsync(std::function<void()>&& task)
    {
        m_ThreadPool.SubmitTask(std::move(task));
    }

    Application::Application(ApplicationInfo const& info)
        : m_ThreadPool(info.ThreadPoolSize)
        , m_Window(info.WindowInfo)
    {
        // Send initial framebuffer resize event to initialize all Client systems.
        u32 width, height;
        m_Window.GetFramebufferSize(width, height);
        WindowFramebufferResizeEvent event(width, height);
        m_Window.OnEvent(event);
    }

    void Application::Run()
    {
        std::jthread updateThread([this]
        {
            u32 tid = std::this_thread::get_id()._Get_underlying_id();
            ENG_LOG_TRACE("Starting update thread with tid={}.", tid);

            while (m_Running.load(std::memory_order_acquire))
            {
                m_Window.OnUpdate();
            }

            ENG_LOG_TRACE("Exiting update thread with tid={}.", tid);
        });

        std::jthread renderThread([this]
        {
            u32 tid = std::this_thread::get_id()._Get_underlying_id();
            ENG_LOG_TRACE("Starting render thread with tid={}.", tid);

            while (m_Running.load(std::memory_order_acquire))
            {
                m_Window.OnRender();
            }

            ENG_LOG_TRACE("Exiting render thread with tid={}.", tid);
        });

        ENG_LOG_TRACE("Starting event handling on main thread.");

        // Process all events on the main thread.
        while (m_Running.load(std::memory_order_acquire))
            glfwWaitEvents();

        ENG_LOG_TRACE("Exiting event handling on main thread.");
    }
}
