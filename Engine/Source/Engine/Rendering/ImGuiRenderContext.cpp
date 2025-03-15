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
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
        io.ConfigViewportsNoTaskBarIcon = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
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

        // TODO: event categories
        if ((io.WantCaptureMouse and (
                event.GetType() == EventType::MouseButtonPress or
                event.GetType() == EventType::MouseButtonRelease or
                event.GetType() == EventType::MouseMove or
                event.GetType() == EventType::MouseEnter or
                event.GetType() == EventType::MouseScroll)) or (
            io.WantCaptureKeyboard and (
                event.GetType() == EventType::KeyPress or
                event.GetType() == EventType::KeyType)))
        {
            event.Handle();
        }
    }
}
