#include "ImGuiRenderContext.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include "Engine/Input/Window.hpp"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace eng
{
    ImGuiRenderContext::ImGuiRenderContext(Window& window, VkRenderPass renderPass)
    {
        RenderContext& context = window.GetRenderContext();
        VkInstance instance = RenderContext::GetInstance();
        VkPhysicalDevice physicalDevice = RenderContext::GetPhysicalDevice();
        VkDevice device = context.GetDevice();
        u32 graphicsFamily = context.GetGraphicsFamily();
        VkQueue graphicsQueue = context.GetGraphicsQueue();
        u32 imageCount = context.GetSwapchainImageCount();

        // Excerpt from imgui_impl_vulkan.cpp:1544:
        // "info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;           // Assume that graphics family == present family"
        ENG_ASSERT(graphicsFamily == context.GetPresentFamily(), "ImGui requires the graphics queue to be the same as the present queue.");

        // Initialize ImGui.

        ImGui::CreateContext();

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = instance;
        info.PhysicalDevice = physicalDevice;
        info.Device = device;
        info.QueueFamily = graphicsFamily;
        info.Queue = graphicsQueue;
        info.RenderPass = renderPass;
        info.MinImageCount = imageCount;
        info.ImageCount = imageCount;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.DescriptorPoolSize = imageCount;

        ENG_VERIFY(ImGui_ImplGlfw_InitForVulkan(window.GetNativeWindow(), true), "Failed to initialize GLFW for ImGui.");
        ENG_VERIFY(ImGui_ImplVulkan_Init(&info), "Failed to initialize Vulkan for ImGui.");

        // TODO: load and upload font

        // Configuration.

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_IsSRGB;
        io.ConfigViewportsNoTaskBarIcon = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        // NOTE: There is a PR for this on ImGui's repository, but it needn't be that complicated.
        // https://github.com/ocornut/imgui/pull/7826
        // Information I used to understand the issue.
        // https://www.reddit.com/r/vulkan/comments/wbh3ul/vulkan_color_space_is_different_when_using_rgb/
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        // Convert to sRGB.
        for (u32 i = 0; i < ImGuiCol_COUNT; i++)
        {
            style.Colors[i].x = std::powf(style.Colors[i].x, 2.2f);
            style.Colors[i].y = std::powf(style.Colors[i].y, 2.2f);
            style.Colors[i].z = std::powf(style.Colors[i].z, 2.2f);
            style.Colors[i].w = std::powf(style.Colors[i].w, 2.2f);
        }
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
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    void ImGuiRenderContext::OnEvent(Event& event)
    {
        ImGuiIO& io = ImGui::GetIO();

        if ((io.WantCaptureMouse and +(event.GetCategories() & EventCategory::Mouse)) or 
            (io.WantCaptureKeyboard and +(event.GetCategories() & EventCategory::Key)))
        {
            event.Handle();
        }
    }
}
