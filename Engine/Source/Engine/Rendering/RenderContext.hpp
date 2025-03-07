#pragma once

#include <vulkan/vulkan.h>
#include <functional>
#include <memory>
#include <vector>

struct GLFWwindow;

namespace eng
{
    class Framebuffer;
    class Image;
    class RenderPass;
    class Shader;

    class RenderContext
    {
    public:
        // TODO: how to avoid this
        void SetFrameImage(std::shared_ptr<Image> imageView);

        VkCommandBuffer BeginOneTimeCommandBuffer();
        void EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);

        static VkInstance GetInstance();
        static VkPhysicalDevice GetPhysicalDevice();
        static VkPhysicalDeviceProperties const& GetPhysicalDeviceProperties();

        VkSurfaceKHR GetSurface() const;
        VkDevice GetDevice() const;
        std::uint32_t GetGraphicsFamily() const;
        std::uint32_t GetPresentFamily() const;
        VkQueue GetGraphicsQueue() const;
        VkQueue GetPresentQueue() const;
        VkImageView GetActiveSwapchainImageView() const;
        std::uint32_t GetSwapchainImageCount() const;
        std::uint32_t GetSwapchainImageIndex() const;
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
        void RenderFullscreenQuad();
    private:
        static void CreateInstance();
#if ENG_CONFIG_DEBUG
        static void CreateDebugUtilsMessenger();
#endif
        static void SelectPhysicalDevice();

        void CreateSurface();
        void SelectQueueFamilies();
        void CreateLogicalDevice();
        void CreateRenderPass();
        void CreateOrRecreateSwapchain();
        void CreateShader();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateFencesAndSemaphores();

        VkExtent2D SelectExtent(VkSurfaceCapabilitiesKHR const& surfaceCapabilities) const;
        std::uint32_t SelectImageCount(VkSurfaceCapabilitiesKHR const& surfaceCapabilities) const;
        VkSurfaceFormatKHR SelectSurfaceFormat() const;
        VkPresentModeKHR SelectPresentMode() const;
    private:
        // Per-application context.

        inline static std::uint32_t s_RenderContextCount = 0;
        inline static VkInstance s_Instance = VK_NULL_HANDLE;
#if ENG_CONFIG_DEBUG
        inline static VkDebugUtilsMessengerEXT s_DebugUtilsMessenger = VK_NULL_HANDLE;
#endif
        inline static VkPhysicalDevice s_PhysicalDevice = VK_NULL_HANDLE;
        inline static VkPhysicalDeviceProperties s_PhysicalDeviceProperties{};

        // TODO: Per-window context

        GLFWwindow* m_Window = nullptr; // Non-owning

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;
        std::uint32_t m_GraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        std::uint32_t m_PresentFamily = VK_QUEUE_FAMILY_IGNORED;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;

        // Swapchain

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        std::unique_ptr<VkImage[]> m_SwapchainImages;
        std::unique_ptr<VkImageView[]> m_SwapchainImageViews;
        std::uint32_t m_SwapchainImageCount = 0;
        VkExtent2D m_SwapchainExtent{0, 0};
        VkSurfaceFormatKHR m_SwapchainSurfaceFormat{};
        bool m_RecreateSwapchain = false;
        bool m_WasSwapchainRecreated = false;

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        std::uint32_t m_FrameIndex = 0;
        std::uint32_t m_SemaphoreIndex = 0;
        std::unique_ptr<VkCommandBuffer[]> m_FrameCommandBuffers;
        std::unique_ptr<VkFence[]> m_FrameInFlightFences;
        std::unique_ptr<VkSemaphore[]> m_ImageAcquiredSemaphores;
        std::unique_ptr<VkSemaphore[]> m_RenderCompleteSemaphores;

        // TODO: rename this section, idk what to call it.
        // My rendering solution.

        VkSampler m_Sampler = VK_NULL_HANDLE;
        std::shared_ptr<RenderPass> m_RenderPass;
        std::shared_ptr<Shader> m_Shader;
        std::vector<std::shared_ptr<Framebuffer>> m_Framebuffers;
        std::vector<std::shared_ptr<Image>> m_FrameImages; // co-owning
    };
}
