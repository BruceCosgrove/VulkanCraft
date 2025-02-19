#include "Application.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/FunctionBindings.hpp"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

namespace eng
{
    void Application::Terminate() noexcept
    {
        m_Running = false;
    }

    LayerStack& Application::GetLayerStack() noexcept
    {
        return m_LayerStack;
    }

    Application::Application(ApplicationInfo const& info)
        : m_Window(info.WindowInfo, ENG_BIND_CLASS_FUNC(OnEvent))
    {

    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        while (m_Running)
        {
            glfwPollEvents();

            // Calculate the time since the last update/frame combo.
            // TODO: separate update/render threads, and only pass the time to the update thread.
            // TODO: think about implementing FixedUpdate.
            double currentTime = glfwGetTime();
            Timestep timestep = static_cast<float>(currentTime - m_LastTime);
            m_LastTime = currentTime;

            m_LayerStack.OnUpdate(timestep);
            m_LayerStack.OnRender();
            // TODO: begin/end imgui around this.
            //m_LayerStack.OnImGuiRender();
        }
    }

    void Application::OnEvent(Event& event)
    {
        m_LayerStack.OnEvent(event);
        // Allow the Client to handle the window close event,
        // thus not terminating the application.
        event.Dispatch(this, &Application::OnWindowCloseEvent);
    }

    void Application::OnWindowCloseEvent(WindowCloseEvent& event)
    {
        m_Running = false;
        event.Handle();
    }
}
