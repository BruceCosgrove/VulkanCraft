#pragma once

// TODO: don't include these here, just in the cpp.
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace eng
{
    class Window;

    class ImGuiRenderContext
    {
    public:

    private:
        friend class Application;
        ImGuiRenderContext(Window& window);
        ~ImGuiRenderContext();
    private:
        Window& m_Window; // non-owning
        // TODO: replace with own implementation, sooner rather than later.
        ImGui_ImplVulkanH_Window m_MainWindow{};

        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    };
}
