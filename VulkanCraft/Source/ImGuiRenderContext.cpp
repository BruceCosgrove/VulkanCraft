#include "ImGuiRenderContext.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include "Engine/Input/Window.hpp"
#include <glfw/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace vc
{
    // TODO: if storing the imgui render context in the base render context,
    // then upon render context creation, it would either need the client render pass,
    // or it would need to create its own, which requires a framebuffer at render time.
    ImGuiRenderContext::ImGuiRenderContext(Window& window, VkRenderPass renderPass)
    {
        RenderContext& context = window.GetRenderContext();
        u32 imageCount = context.GetSwapchainImageCount();

        // Excerpt from imgui_impl_vulkan.cpp:1544:
        // "info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;           // Assume that graphics family == present family"
        ENG_ASSERT(context.GetGraphicsFamily() == context.GetPresentFamily(), "ImGui requires the graphics queue to be the same as the present queue.");

        ImGui::CreateContext();

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = RenderContext::GetInstance();
        info.PhysicalDevice = RenderContext::GetPhysicalDevice();
        info.Device = context.GetDevice();
        info.QueueFamily = context.GetGraphicsFamily();
        info.Queue = context.GetGraphicsQueue();
        info.RenderPass = renderPass;
        info.MinImageCount = imageCount;
        info.ImageCount = imageCount;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.DescriptorPoolSize = imageCount;

        ENG_VERIFY(ImGui_ImplGlfw_InitForVulkan(window.GetNativeWindow(), true), "Failed to initialize GLFW for ImGui.");
        ENG_VERIFY(ImGui_ImplVulkan_Init(&info), "Failed to initialize Vulkan for ImGui.");
    }

    ImGuiRenderContext::~ImGuiRenderContext()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiRenderContext::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiRenderContext::EndFrame(VkCommandBuffer commandBuffer)
    {
        ImGui::Render(); // Calls ImGui::EndFrame();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        //ImGui::UpdatePlatformWindows();
        //ImGui::RenderPlatformWindowsDefault();
    }

    void ImGuiRenderContext::OnEvent(Event& event)
    {
        ImGuiIO& io = ImGui::GetIO();

        if ((io.WantCaptureMouse and event.GetCategories().HasAll(EventCategory::Mouse)) or
            (io.WantCaptureKeyboard and event.GetCategories().HasAll(EventCategory::Key)))
        {
            event.Handle();
        }
    }
}
