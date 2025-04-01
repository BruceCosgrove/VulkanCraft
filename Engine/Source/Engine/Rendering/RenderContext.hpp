#pragma once

#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>
#include <vector>

#define ENG_GET_FUNC_VK_EXT(name) \
    PFN_##name name = ::eng::RenderContext::GetExtensionFunction<PFN_##name>(#name)

struct GLFWwindow;

namespace eng
{
    class Framebuffer;
    class Image;
    class RenderPass;
    class Shader;

    class RenderContext
    {
    private:
        struct FreeRAII
        {
            ~FreeRAII() { FreeFunction(); }
            std::function<void()> FreeFunction;
        };
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(RenderContext);

        VkCommandBuffer BeginOneTimeCommandBuffer();
        void EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);

        void DeferFree(std::function<void()>&& freeFunction);

        static VkInstance GetInstance();
        static VkPhysicalDevice GetPhysicalDevice();
        static VkPhysicalDeviceProperties const& GetPhysicalDeviceProperties();

        template <typename T>
        static T GetExtensionFunction(small_string_view name)
        {
            if (auto it = s_ExtensionFunctions.find(name); it != s_ExtensionFunctions.end())
                return (T)it->second;

            PFN_vkVoidFunction func = vkGetInstanceProcAddr(s_Instance, name.data());
            ENG_ASSERT(func != nullptr, "Failed to load Vulkan extension: \"{}\".", name);
            s_ExtensionFunctions[name] = func;
            return (T)func;
        }

        VkSurfaceKHR GetSurface() const;
        VkDevice GetDevice() const;
        u32 GetGraphicsFamily() const;
        u32 GetPresentFamily() const;
        VkQueue GetGraphicsQueue() const;
        VkQueue GetPresentQueue() const;
        VkImageView GetSwapchainImageView(u32 index) const;
        u32 GetSwapchainImageCount() const;
        u32 GetSwapchainImageIndex() const;
        VkExtent2D GetSwapchainExtent() const;
        VkFormat GetSwapchainFormat() const;
        bool WasSwapchainRecreated() const;
        VkCommandBuffer GetActiveCommandBuffer() const;
    private:
        friend class Window;

        RenderContext(GLFWwindow* window);
        ~RenderContext();

        bool BeginFrame();
        void EndFrame();
    private:
        static void CreateInstance();
#if ENG_CONFIG_DEBUG
        static void CreateDebugUtilsMessenger();
#endif
        static void SelectPhysicalDevice();

        void CreateSurface();
        void SelectQueueFamilies();
        void CreateLogicalDevice();
        void CreateOrRecreateSwapchain();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateFencesAndSemaphores();

        VkExtent2D SelectExtent(VkSurfaceCapabilitiesKHR const& surfaceCapabilities) const;
        u32 SelectImageCount(VkSurfaceCapabilitiesKHR const& surfaceCapabilities) const;
        VkSurfaceFormatKHR SelectSurfaceFormat() const;
        VkPresentModeKHR SelectPresentMode() const;
    private:
        // Per-application context.

        inline static u32 s_RenderContextCount = 0;
        inline static VkInstance s_Instance = VK_NULL_HANDLE;
        inline static std::unordered_map<small_string_view, PFN_vkVoidFunction> s_ExtensionFunctions;
#if ENG_CONFIG_DEBUG
        inline static VkDebugUtilsMessengerEXT s_DebugUtilsMessenger = VK_NULL_HANDLE;
#endif
        inline static VkPhysicalDevice s_PhysicalDevice = VK_NULL_HANDLE;
        inline static VkPhysicalDeviceProperties s_PhysicalDeviceProperties{};

        // TODO: Per-window context

        GLFWwindow* m_Window = nullptr; // Non-owning

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;
        u32 m_GraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        u32 m_PresentFamily = VK_QUEUE_FAMILY_IGNORED;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;

        // Swapchain

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        std::unique_ptr<VkImage[]> m_SwapchainImages;
        std::unique_ptr<VkImageView[]> m_SwapchainImageViews;
        u32 m_SwapchainImageCount = 0;
        VkExtent2D m_SwapchainExtent{0, 0};
        VkSurfaceFormatKHR m_SwapchainSurfaceFormat{};
        bool m_RecreateSwapchain = false;
        bool m_WasSwapchainRecreated = false;

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        u32 m_FrameIndex = 0;
        u32 m_SemaphoreIndex = 0;
        std::unique_ptr<VkCommandBuffer[]> m_FrameCommandBuffers;
        std::unique_ptr<VkFence[]> m_FrameInFlightFences;
        std::unique_ptr<VkSemaphore[]> m_ImageAcquiredSemaphores;
        std::unique_ptr<VkSemaphore[]> m_RenderCompleteSemaphores;
        std::vector<std::vector<FreeRAII>> m_FrameFreeQueues;
    };
}
