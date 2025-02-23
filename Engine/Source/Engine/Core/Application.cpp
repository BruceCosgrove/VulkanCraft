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

    Window& Application::GetWindow()
    {
        return m_Window;
    }

    Application::Application(ApplicationInfo const& info)
        : m_Window(info.WindowInfo)
    {

    }

    Application::~Application()
    {
        VkResult result = vkDeviceWaitIdle(m_Window.GetRenderContext().GetDevice());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for the device to stop working. This really shouldn't happen.");
    }

    void Application::Run()
    {
        while (m_Running)
        {
            glfwPollEvents();

            m_Window.OnUpdate();
            m_Window.OnRender();

            // TODO: if any non-imgui windows are visible,
            // do this, and only call OnRender and
            // OnImGuiRender for the visible windows.
            // 
            // Render a frame if it's visible.

            //ImGui_ImplVulkan_NewFrame();
            //ImGui_ImplGlfw_NewFrame();
            //ImGui::NewFrame();

            //// Do the Client's ImGui rendering.
            //if (rendering)
            //    m_Window.GetLayerStack().OnImGuiRender();

            //ImGui::Render(); // Calls ImGui::EndFrame();
            //ImGui::UpdatePlatformWindows();
            //ImGui::RenderPlatformWindowsDefault();
        }
    }
}
