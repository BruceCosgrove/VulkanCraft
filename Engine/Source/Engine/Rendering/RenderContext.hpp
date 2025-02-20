#pragma once

#include "Engine/Input/Window.hpp"
#include <vulkan/vulkan.h>
#include <vector>

struct ImDrawData;

namespace eng
{
    // TODO: Refactor; this is getting pretty spaghetti.
    class RenderContext
    {
    public:
        RenderContext(Window& window, std::function<void()>&& renderCallback, std::function<void()>&& imguiRenderCallback);
        ~RenderContext();
    public:
        VkInstance GetInstance();
        VkPhysicalDevice GetPhysicalDevice();
        VkDevice GetDevice();
        VkCommandBuffer GetCommandBuffer();
        void FlushCommandBuffer(VkCommandBuffer commandBuffer);
        void SubmitResourceFree(std::function<void()>&& freeFunc);
    private:
        friend class Application;
        void DoFrame(bool render);
    private:
        void FrameRender(ImDrawData* drawData);
        void FramePresent();
    private:
        GLFWwindow* m_ContextWindow = nullptr;
        std::function<void()> m_RenderCallback;
        std::function<void()> m_ImGuiRenderCallback;

        VkInstance m_Instance = VK_NULL_HANDLE;
#if ENG_CONFIG_DEBUG
        VkDebugReportCallbackEXT m_DebugReportCallback = VK_NULL_HANDLE;
#endif
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        std::uint32_t m_GraphicsFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        std::vector<std::vector<VkCommandBuffer>> m_AllocatedCommandBuffers;
        std::vector<std::vector<std::function<void()>>> m_ResourceFreeQueue;
        std::uint32_t m_CurrentFrameIndex = 0;
        bool m_RebuildSwapChain = false;
    };
}
