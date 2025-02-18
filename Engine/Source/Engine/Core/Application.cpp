#include "Application.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/FunctionBindings.hpp"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

namespace eng
{
    Application::Application(ApplicationInfo const& info)
        : m_Window(info.WindowInfo, ENG_BIND_CLASS_FUNC(OnEvent))
    {

    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        ENG_ASSERT(m_Running == false, "Tried to rerun the application.");
        m_Running = true;

        while (m_Running)
        {
            glfwPollEvents();
        }
    }

    void Application::OnEvent(Event& event)
    {
        event.Dispatch(this, &Application::OnWindowCloseEvent);
    }

    void Application::OnWindowCloseEvent(WindowCloseEvent& event)
    {
        m_Running = false;
        event.Handle();
    }
}
