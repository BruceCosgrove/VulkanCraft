#include "ImGuiRenderContext.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include "Engine/Input/Window.hpp"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <array>

namespace eng
{
    ImGuiRenderContext::ImGuiRenderContext(Window& window)
        : m_Window(window)
    {
        VkResult result = VK_SUCCESS;

        RenderContext& context = window.GetRenderContext();
        VkInstance instance = RenderContext::GetInstance();
        VkPhysicalDevice physicalDevice = RenderContext::GetPhysicalDevice();
        VkDevice device = context.GetDevice();
        std::uint32_t graphicsFamily = context.GetGraphicsFamily();
        VkQueue graphicsQueue = context.GetGraphicsQueue();
        std::uint32_t minImageCount = 2; // TODO

        // Create descriptor pool.
        // https://vkguide.dev/docs/chapter-4/descriptors/
        {
            auto descriptorPoolSizes = std::to_array<VkDescriptorPoolSize>
            ({
                {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000},
            });

            VkDescriptorPoolCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            info.maxSets = static_cast<std::uint32_t>(1000 * descriptorPoolSizes.size());
            info.poolSizeCount = static_cast<std::uint32_t>(descriptorPoolSizes.size());
            info.pPoolSizes = descriptorPoolSizes.data();

            result = vkCreateDescriptorPool(device, &info, nullptr, &m_DescriptorPool);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool.");
        }

        // Initialize ImGui.
        {
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;

            ImGui_ImplVulkan_InitInfo info{};
            info.Instance = instance;
            info.PhysicalDevice = physicalDevice;
            info.Device = device;
            // TODO: imgui seemingly assumes that the graphics queue and the presentation queue will be the same,
            // as they have only included one queue and queue family in their init info struct.
            // I will have to eventually replace the ImGui backend with my own implementation.
            // NOTE: Yup! I've confirmed they assume they're the same queue.
            // Excerpt from imgui_impl_vulkan.cpp:1544:
            // "info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;           // Assume that graphics family == present family"
            info.QueueFamily = graphicsFamily;
            info.Queue = graphicsQueue;
            info.DescriptorPool = m_DescriptorPool;
            info.RenderPass = m_MainWindow.RenderPass;
            info.MinImageCount = minImageCount;
            info.ImageCount = m_MainWindow.ImageCount;
            info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

            ENG_VERIFY(ImGui_ImplGlfw_InitForVulkan(window.GetNativeWindow(), true), "Failed to initialize GLFW for ImGui.");
            ENG_VERIFY(ImGui_ImplVulkan_Init(&info), "Failed to initialize Vulkan for ImGui.");

            // TODO: load and upload font
        }

        // TODO: don't call glfw here, just get it from the window or something.
        std::int32_t width, height;
        glfwGetFramebufferSize(window.GetNativeWindow(), &width, &height);

        // TODO: this is supposed to use the presentation queue family index
        ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, &m_MainWindow, graphicsFamily, nullptr, width, height, minImageCount);



        // TODO: render
        //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frame.CommandBuffer);
    }

    ImGuiRenderContext::~ImGuiRenderContext()
    {
        RenderContext& context = m_Window.GetRenderContext();
        VkInstance instance = RenderContext::GetInstance();
        VkDevice device = context.GetDevice();

        ImGui_ImplVulkanH_DestroyWindow(instance, device, &m_MainWindow, nullptr);

        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

        // Shutdown ImGui.
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
