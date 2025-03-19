#include "Application.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>

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

    }

    void Application::Run()
    {
        std::jthread updateThread([this]
        {
            ENG_LOG_TRACE("Starting update thread.");

            while (m_Running.load(std::memory_order_acquire))
            {
                m_Window.OnUpdate();
            }

            ENG_LOG_TRACE("Stopping update thread.");
        });

        std::jthread renderThread([this]
        {
            ENG_LOG_TRACE("Starting render thread.");
            m_Window.OnRenderThreadStarted();

            while (m_Running.load(std::memory_order_acquire))
            {
                m_Window.OnRender();
            }

            VkResult result = vkDeviceWaitIdle(m_Window.GetRenderContext().GetDevice());
            ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for the device to stop working. This really shouldn't happen.");

            m_Window.OnRenderThreadStopped();
            ENG_LOG_TRACE("Stopping render thread.");
        });

        ENG_LOG_TRACE("Starting event handling on main thread.");

        // Process all events on the main thread.
        while (m_Running.load(std::memory_order_acquire))
            glfwWaitEvents();

        ENG_LOG_TRACE("Stopping event handling on main thread.");
    }
}
