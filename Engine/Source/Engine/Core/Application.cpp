#include "Application.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/FunctionBindings.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

namespace eng
{
    void Application::Terminate()
    {
        m_Running = false;
    }

    LayerStack& Application::GetLayerStack()
    {
        return m_LayerStack;
    }

    Window& Application::GetWindow()
    {
        return m_Window;
    }

    Application::Application(ApplicationInfo const& info)
        : m_Window(
            info.WindowInfo,
            ENG_BIND_CLASS_FUNC(Application::OnEvent),
            ENG_BIND_MEMBER_FUNC(m_LayerStack, LayerStack::OnRender))
    {

    }

    Application::~Application()
    {
        VkResult result = vkDeviceWaitIdle(m_Window.GetRenderContext().GetDevice());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for the device to stop working. This really shouldn't happen.");

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

            // Do the Client's updating.
            m_LayerStack.OnUpdate(timestep);

            // TODO: if any non-imgui windows are visible,
            // do this, and only call OnRender and
            // OnImGuiRender for the visible windows.
            // 
            // Render a frame if it's visible.
            bool rendering = !m_Minimized && !m_ZeroSize;

            //ImGui_ImplVulkan_NewFrame();
            //ImGui_ImplGlfw_NewFrame();
            //ImGui::NewFrame();

            //// Do the Client's ImGui rendering.
            //if (rendering)
            //    m_LayerStack.OnImGuiRender();

            //ImGui::Render(); // Calls ImGui::EndFrame();
            //ImGui::UpdatePlatformWindows();
            //ImGui::RenderPlatformWindowsDefault();

            // Do the Client's rendering.
            if (rendering)
                m_Window.GetRenderContext().OnRender();
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
    }

    void Application::OnWindowMinimizeEvent(WindowMinimizeEvent& event)
    {
        m_Minimized = event.IsMinimized();
    }

    void Application::OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event)
    {
        m_ZeroSize = event.GetFramebufferWidth() == 0 || event.GetFramebufferHeight() == 0;

        // Block zero-size events from propagating to the Client.
        if (m_ZeroSize)
            event.Handle();
    }
}
