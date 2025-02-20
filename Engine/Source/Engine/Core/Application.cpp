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

    RenderContext& Application::GetRenderContext() noexcept
    {
        return m_RenderContext;
    }

    Application::Application(ApplicationInfo const& info)
        : m_Window(
            info.WindowInfo,
            ENG_BIND_CLASS_FUNC(Application::OnEvent))
        , m_RenderContext(
            m_Window,
            ENG_BIND_MEMBER_FUNC(m_LayerStack, LayerStack::OnRender),
            ENG_BIND_MEMBER_FUNC(m_LayerStack, LayerStack::OnImGuiRender))
    {

    }

    Application::~Application()
    {
        // Must destroy Client first.
        m_LayerStack.Clear();
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

            m_RenderContext.DoFrame(!m_Minimized and !m_ZeroSize);
        }
    }

    void Application::OnEvent(Event& event)
    {
        event.Dispatch(this, &Application::OnWindowMinimizeEvent);
        event.Dispatch(this, &Application::OnWindowFramebufferResizeEvent);

        event.Dispatch(&m_LayerStack, &LayerStack::OnEvent);

        // Allow the Client to handle the window close event,
        // thus not terminating the application.
        event.Dispatch(this, &Application::OnWindowCloseEvent);
    }

    void Application::OnWindowCloseEvent(WindowCloseEvent& event)
    {
        Terminate();
        event.Handle();
    }

    void Application::OnWindowMinimizeEvent(WindowMinimizeEvent& event)
    {
        m_Minimized = event.IsMinimized();
    }

    void Application::OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event)
    {
        m_ZeroSize = event.GetFramebufferWidth() == 0 or event.GetFramebufferHeight() == 0;
        if (m_ZeroSize)
            event.Handle();
    }
}
