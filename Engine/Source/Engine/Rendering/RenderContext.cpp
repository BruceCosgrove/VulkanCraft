#include "RenderContext.hpp"
#include "Engine/Core/Attributes.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Rendering/Framebuffer.hpp"
#include "Engine/Rendering/Image.hpp"
#include "Engine/Rendering/ImageUtils.hpp"
#include "Engine/Rendering/RenderPass.hpp"
#include "Engine/Rendering/Shader.hpp"
#include <glfw/glfw3.h>
#include <array>
#include <optional>
#include <span>
#include <vector>

namespace eng
{
#if ENG_CONFIG_DEBUG
    static VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        static_cast<void>(messageTypes, pUserData);
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                ENG_LOG_TRACE("Vulkan {}: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                ENG_LOG_INFO("Vulkan {}: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                ENG_LOG_WARN("Vulkan {}: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                ENG_LOG_ERROR("Vulkan {}: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
                break;
        }
        return VK_FALSE;
    }
#endif

    VkCommandBuffer RenderContext::BeginOneTimeCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = m_CommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocateInfo, &commandBuffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate one-time command buffer.");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to begin one-time command buffer.");

        return commandBuffer;
    }

    void RenderContext::EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
    {
        VkResult result = vkEndCommandBuffer(commandBuffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to end one-time command buffer.");

        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffer;

        result = vkQueueSubmit(m_GraphicsQueue, 1, &info, VK_NULL_HANDLE);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to submit queue for one-time command buffer.");

        result = vkQueueWaitIdle(m_GraphicsQueue);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for queue for one-time command buffer.");

        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
    }

    void RenderContext::DeferFree(std::function<void()>&& freeFunction)
    {
        m_FrameFreeQueues[m_FrameIndex].emplace_back(std::move(freeFunction));
    }

    VkInstance RenderContext::GetInstance()
    {
        return s_Instance;
    }

    VkPhysicalDevice RenderContext::GetPhysicalDevice()
    {
        return s_PhysicalDevice;
    }

    VkPhysicalDeviceProperties const& RenderContext::GetPhysicalDeviceProperties()
    {
        return s_PhysicalDeviceProperties;
    }

    VkDevice RenderContext::GetDevice() const
    {
        return m_Device;
    }

    u32 RenderContext::GetGraphicsFamily() const
    {
        return m_GraphicsFamily;
    }

    u32 RenderContext::GetPresentFamily() const
    {
        return m_PresentFamily;
    }

    VkQueue RenderContext::GetGraphicsQueue() const
    {
        return m_GraphicsQueue;
    }

    VkQueue RenderContext::GetPresentQueue() const
    {
        return m_PresentQueue;
    }

    VkImageView RenderContext::GetSwapchainImageView(u32 index) const
    {
        return m_SwapchainImageViews[index];
    }

    u32 RenderContext::GetSwapchainImageCount() const
    {
        return m_SwapchainImageCount;
    }

    u32 RenderContext::GetSwapchainImageIndex() const
    {
        return m_FrameIndex;
    }

    VkExtent2D RenderContext::GetSwapchainExtent() const
    {
        return m_SwapchainExtent;
    }

    VkFormat RenderContext::GetSwapchainFormat() const
    {
        return m_SwapchainSurfaceFormat.format;
    }

    bool RenderContext::WasSwapchainRecreated() const
    {
        return m_WasSwapchainRecreated;
    }

    VkCommandBuffer RenderContext::GetActiveCommandBuffer() const
    {
        return m_FrameCommandBuffers[m_FrameIndex];
    }

    VkSurfaceKHR RenderContext::GetSurface() const
    {
        return m_Surface;
    }

    RenderContext::RenderContext(GLFWwindow* window)
        : m_Window(window)
    {
        // If there aren't any render contexts yet, initialize their shared context.
        if (s_RenderContextCount++ == 0)
        {
            CreateInstance();
#if ENG_CONFIG_DEBUG
            CreateDebugUtilsMessenger();
#endif
            SelectPhysicalDevice();
        }

        // The surface is needed to select queue families.
        CreateSurface();
        SelectQueueFamilies();
        CreateLogicalDevice();
        CreateOrRecreateSwapchain();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateFencesAndSemaphores();
    }

    RenderContext::~RenderContext()
    {
        m_FrameFreeQueues.clear();

        for (u32 i = 0; i < m_SwapchainImageCount; i++)
        {
            vkDestroySemaphore(m_Device, m_ImageAcquiredSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device, m_RenderCompleteSemaphores[i], nullptr);
            vkDestroyFence(m_Device, m_FrameInFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        for (u32 i = 0; i < m_SwapchainImageCount; i++)
            vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        vkDestroyDevice(m_Device, nullptr);
        vkDestroySurfaceKHR(s_Instance, m_Surface, nullptr);

        // If there aren't any render contexts any more, shutdown their shared context.
        if (--s_RenderContextCount == 0)
        {
#if ENG_CONFIG_DEBUG
            ENG_GET_FUNC_VK_EXT(vkDestroyDebugUtilsMessengerEXT);
            vkDestroyDebugUtilsMessengerEXT(s_Instance, s_DebugUtilsMessenger, nullptr);
#endif
            vkDestroyInstance(s_Instance, nullptr);
        }
    }

    bool RenderContext::BeginFrame()
    {
        // Recreate the swapchain if necessary.
        m_WasSwapchainRecreated = m_RecreateSwapchain;
        if (m_RecreateSwapchain)
        {
            m_RecreateSwapchain = false;
            CreateOrRecreateSwapchain();
        }

        VkSemaphore imageAcquiredSemaphore = m_ImageAcquiredSemaphores[m_SemaphoreIndex];

        // Get the next image to render the frame to.
        VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, std::numeric_limits<u64>::max(), imageAcquiredSemaphore, nullptr, &m_FrameIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR)
        {
            m_RecreateSwapchain = true;
            return false;
        }
        ENG_ASSERT(result == VK_SUCCESS, "Failed to acquire next swapchain image.");

        VkFence frameInFlightFence = m_FrameInFlightFences[m_FrameIndex];
        VkCommandBuffer frameCommandBuffer = m_FrameCommandBuffers[m_FrameIndex];

        // Wait for the previous frame using this image to finish rendering.
        result = vkWaitForFences(m_Device, 1, &frameInFlightFence, VK_TRUE, std::numeric_limits<u64>::max());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for frame fence.");

        result = vkResetFences(m_Device, 1, &frameInFlightFence);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to reset frame fence.");

        // Once the frame is finished, its resources can be free'd.
        m_FrameFreeQueues[m_FrameIndex].clear();

        // Reset the frame's command buffer.
        result = vkResetCommandBuffer(frameCommandBuffer, 0);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to reset frame command buffer.");

        // Begin the frame's command buffer.
        {
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            result = vkBeginCommandBuffer(frameCommandBuffer, &info);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer.");
        }

        return true;
    }

    void RenderContext::EndFrame()
    {
        ENG_ASSERT(not m_RecreateSwapchain);

        VkResult result = VK_SUCCESS;
        VkSemaphore imageAcquiredSemaphore = m_ImageAcquiredSemaphores[m_SemaphoreIndex];
        VkSemaphore renderCompleteSemaphore = m_RenderCompleteSemaphores[m_SemaphoreIndex];
        VkFence frameInFlightFence = m_FrameInFlightFences[m_FrameIndex];
        VkCommandBuffer frameCommandBuffer = m_FrameCommandBuffers[m_FrameIndex];

        // End the frame's command buffer and submit
        {
            VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &imageAcquiredSemaphore;
            info.pWaitDstStageMask = &waitMask;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &frameCommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &renderCompleteSemaphore;

            result = vkEndCommandBuffer(frameCommandBuffer);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to end command buffer.");
            result = vkQueueSubmit(m_GraphicsQueue, 1, &info, frameInFlightFence);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to submit render queue.");
        }

        // Present the most recently completed frame to the surface.
        {
            // TODO: in a scenario with multiple windows, it would be favorable
            // to present them all at once with one call to vkQueuePresentKHR,
            // giving info all their swapchains and frame indices, then checking
            // all their results via info.pResults.
            VkPresentInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &renderCompleteSemaphore;
            info.swapchainCount = 1;
            info.pSwapchains = &m_Swapchain;
            info.pImageIndices = &m_FrameIndex;

            result = vkQueuePresentKHR(m_PresentQueue, &info);
            if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR)
            {
                m_RecreateSwapchain = true;
                return;
            }
            ENG_ASSERT(result == VK_SUCCESS, "Failed to present frame.");

            // Use the next semaphore.
            m_SemaphoreIndex = (m_SemaphoreIndex + 1) % m_SwapchainImageCount;
        }
    }

    void RenderContext::CreateInstance()
    {
        std::vector<char const*> layers
        {
#if ENG_CONFIG_DEBUG
            "VK_LAYER_KHRONOS_validation",
#endif
        };

        std::vector<char const*> extensions
        {
#if ENG_CONFIG_DEBUG
            "VK_EXT_debug_utils",
#endif
        };

        u32 glfwExtensionCount;
        char const** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        extensions.append_range(std::span<char const*>(glfwExtensions, glfwExtensionCount));

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &applicationInfo;
        info.enabledLayerCount = static_cast<u32>(layers.size());
        info.ppEnabledLayerNames = layers.data();
        info.enabledExtensionCount = static_cast<u32>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();

        VkResult result = vkCreateInstance(&info, nullptr, &s_Instance);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan instance.");
    }

#if ENG_CONFIG_DEBUG
    void RenderContext::CreateDebugUtilsMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        info.pfnUserCallback = &DebugUtilsMessengerCallback;

        ENG_GET_FUNC_VK_EXT(vkCreateDebugUtilsMessengerEXT);
        VkResult result = vkCreateDebugUtilsMessengerEXT(s_Instance, &info, nullptr, &s_DebugUtilsMessenger);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create debug utils messenger.");
    }
#endif

    void RenderContext::SelectPhysicalDevice()
    {
        // Get all physical devices.
        u32 count;
        VkResult result = vkEnumeratePhysicalDevices(s_Instance, &count, nullptr);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get physical device count.");
        ENG_ASSERT(count > 0, "No physical devices available.");
        std::vector<VkPhysicalDevice> physicalDevices(count);
        result = vkEnumeratePhysicalDevices(s_Instance, &count, physicalDevices.data());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get physical devices.");

        // Select the most appropriate one.
        u32 fallbackPhysicalDeviceIndex = 0;
        u32 selectedPhysicalDeviceIndex = count;
        for (u32 i = 0; i < count; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                selectedPhysicalDeviceIndex = i;
                break;
            }
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                fallbackPhysicalDeviceIndex = i;
        }

        if (selectedPhysicalDeviceIndex == count)
        {
            selectedPhysicalDeviceIndex = fallbackPhysicalDeviceIndex;
            ENG_LOG_WARN("Warning: no dedicated GPU found, falling back to other physical device.");
        }
        s_PhysicalDevice = physicalDevices[selectedPhysicalDeviceIndex];
        // Cache the physical device properties for later querying.
        vkGetPhysicalDeviceProperties(s_PhysicalDevice, &s_PhysicalDeviceProperties);
    }

    void RenderContext::CreateSurface()
    {
        VkResult result = glfwCreateWindowSurface(s_Instance, m_Window, nullptr, &m_Surface);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create window surface.");
    }

    void RenderContext::SelectQueueFamilies()
    {
        // Get all queue family properties.
        u32 count;
        vkGetPhysicalDeviceQueueFamilyProperties(s_PhysicalDevice, &count, nullptr);
        ENG_ASSERT(count > 0, "No queue families available.");
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(s_PhysicalDevice, &count, queueFamilyProperties.data());

        std::optional<u32> graphicsFamily, presentFamily;
        for (u32 i = 0; i < count and !(graphicsFamily and presentFamily); i++)
        {
            if (!graphicsFamily and queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            if (!presentFamily)
            {
                VkBool32 presentSupported;
                VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(s_PhysicalDevice, i, m_Surface, &presentSupported);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to get surface presentation support.");
                if (presentSupported)
                    presentFamily = i;
            }
        }

        ENG_ASSERT(graphicsFamily.has_value() and presentFamily.has_value(), "Failed to find appropriate queue families.");
        m_GraphicsFamily = graphicsFamily.value();
        m_PresentFamily = presentFamily.value();
    }

    void RenderContext::CreateLogicalDevice()
    {
        auto extensions = std::to_array<char const*>
        ({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, // TODO
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME, // TODO
        });

        f32 priority = 1.0f;

        VkDeviceQueueCreateInfo deviceQueueCreateInfos[2]{};
        u32 queueCount = 1;
        deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfos[0].queueFamilyIndex = m_GraphicsFamily;
        deviceQueueCreateInfos[0].queueCount = 1;
        deviceQueueCreateInfos[0].pQueuePriorities = &priority;

        if (m_GraphicsFamily != m_PresentFamily)
        {
            queueCount = 2;
            deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfos[1].queueFamilyIndex = m_PresentFamily;
            deviceQueueCreateInfos[1].queueCount = 1;
            deviceQueueCreateInfos[1].pQueuePriorities = &priority;
        }

        VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{};
        dynamicState3Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
        dynamicState3Features.extendedDynamicState3PolygonMode = VK_TRUE; // TODO

        VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeatures{};
        shaderDrawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        shaderDrawParametersFeatures.pNext = &dynamicState3Features;
        shaderDrawParametersFeatures.shaderDrawParameters = VK_TRUE; // TODO

        VkPhysicalDeviceFeatures features{};
        features.multiDrawIndirect = VK_TRUE; // TODO
        features.drawIndirectFirstInstance = VK_TRUE; // TODO
        features.fillModeNonSolid = VK_TRUE; // TODO

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = &shaderDrawParametersFeatures;
        deviceCreateInfo.queueCreateInfoCount = queueCount;
        deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
        deviceCreateInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
        deviceCreateInfo.pEnabledFeatures = &features;

        VkResult result = vkCreateDevice(s_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create logical device.");

        vkGetDeviceQueue(m_Device, m_GraphicsFamily, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_PresentFamily, 0, &m_PresentQueue);
    }

    void RenderContext::CreateOrRecreateSwapchain()
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_PhysicalDevice, m_Surface, &surfaceCapabilities);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get surface capabilities.");

        // Create or recreate the swapchain.
        {
            std::unique_ptr<VkImageView[]> oldSwapchainImageViews = std::move(m_SwapchainImageViews);
            VkSwapchainKHR oldSwapchain = m_Swapchain;

            // Choose swapchain properties.
            u32 imageCount = SelectImageCount(surfaceCapabilities);
            VkExtent2D extent = SelectExtent(surfaceCapabilities);
            VkSurfaceFormatKHR surfaceFormat = SelectSurfaceFormat();
            VkPresentModeKHR presentMode = SelectPresentMode();

            VkSwapchainCreateInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            info.surface = m_Surface;
            info.minImageCount = imageCount;
            info.imageFormat = surfaceFormat.format;
            info.imageColorSpace = surfaceFormat.colorSpace;
            info.imageExtent = extent;
            info.imageArrayLayers = 1; // NOTE: Set to 2 for VR (one per eye).
            info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            if (m_GraphicsFamily != m_PresentFamily)
            {
                info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                info.queueFamilyIndexCount = 2;
            }
            auto queueFamilyIndices = std::to_array({m_GraphicsFamily, m_PresentFamily});
            info.pQueueFamilyIndices = queueFamilyIndices.data();

            info.preTransform = surfaceCapabilities.currentTransform;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = presentMode;
            info.clipped = VK_TRUE;
            info.oldSwapchain = oldSwapchain;

            // Create swapchain.
            result = vkCreateSwapchainKHR(m_Device, &info, nullptr, &m_Swapchain);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create swapchain.");

            // Get swapchain image count.
            result = vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to get swapchain image count.");

            // Get swapchain image count.
            m_SwapchainImages.reset(new VkImage[imageCount]);
            result = vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.get());
            ENG_ASSERT(result == VK_SUCCESS, "Failed to get swapchain images.");

            if (oldSwapchain)
            {
                // Wait for the old swapchain to be done being used before deleting it.
                // TODO: not a great solution. Would be ideal to wait for a fence.
                result = vkQueueWaitIdle(m_GraphicsQueue);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for queue to finish.");

                for (u32 i = 0; i < m_SwapchainImageCount; i++)
                    vkDestroyImageView(m_Device, oldSwapchainImageViews[i], nullptr);
                vkDestroySwapchainKHR(m_Device, oldSwapchain, nullptr);
            }

            // TODO: This should always be the case, right?
            ENG_ASSERT(m_SwapchainImageCount == 0 or m_SwapchainImageCount == imageCount);

            // Save these for later.
            m_SwapchainImageCount = imageCount;
            m_SwapchainExtent = extent;
            m_SwapchainSurfaceFormat = surfaceFormat;
        }

        // Create swapchain image views.
        {
            m_SwapchainImageViews.reset(new VkImageView[m_SwapchainImageCount]);

            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = m_SwapchainSurfaceFormat.format;
            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            for (u32 i = 0; i < m_SwapchainImageCount; i++)
            {
                info.image = m_SwapchainImages[i];
                VkResult result = vkCreateImageView(m_Device, &info, nullptr, &m_SwapchainImageViews[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create swapchain image view {}/{}.", i + 1, m_SwapchainImageCount);
            }
        }

        m_FrameFreeQueues.resize(m_SwapchainImageCount);
    }

    void RenderContext::CreateCommandPool()
    {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = m_GraphicsFamily;

        VkResult result = vkCreateCommandPool(m_Device, &info, nullptr, &m_CommandPool);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create command pool.");
    }

    void RenderContext::CreateCommandBuffers()
    {
        m_FrameCommandBuffers.reset(new VkCommandBuffer[m_SwapchainImageCount]);

        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = m_CommandPool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = m_SwapchainImageCount;

        VkResult result = vkAllocateCommandBuffers(m_Device, &info, m_FrameCommandBuffers.get());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers.");
    }

    void RenderContext::CreateFencesAndSemaphores()
    {
        VkResult result = VK_SUCCESS;

        // Create fences.
        {
            m_FrameInFlightFences.reset(new VkFence[m_SwapchainImageCount]);

            VkFenceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (u32 i = 0; i < m_SwapchainImageCount; i++)
            {
                result = vkCreateFence(m_Device, &info, nullptr, &m_FrameInFlightFences[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create fence.");
            }
        }

        // Create semaphores.
        {
            m_ImageAcquiredSemaphores.reset(new VkSemaphore[m_SwapchainImageCount]);
            m_RenderCompleteSemaphores.reset(new VkSemaphore[m_SwapchainImageCount]);

            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            for (u32 i = 0; i < m_SwapchainImageCount; i++)
            {
                result = vkCreateSemaphore(m_Device, &info, nullptr, &m_ImageAcquiredSemaphores[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create fence.");
                result = vkCreateSemaphore(m_Device, &info, nullptr, &m_RenderCompleteSemaphores[i]);
                ENG_ASSERT(result == VK_SUCCESS, "Failed to create fence.");
            }
        }
    }

    VkExtent2D RenderContext::SelectExtent(VkSurfaceCapabilitiesKHR const& surfaceCapabilities) const
    {
        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<u32>::max())
            return surfaceCapabilities.currentExtent;

        // TODO: figure out how to get framebuffer size without glfw.
        // I guess the swapchain has to be recreated every window resize?
        u32 width, height;
        glfwGetFramebufferSize(m_Window, (i32*)&width, (i32*)&height);

        return
        {
            // Clamp the extent to the min and max extents.
            std::clamp(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
            std::clamp(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
        };
    }

    u32 RenderContext::SelectImageCount(VkSurfaceCapabilitiesKHR const& surfaceCapabilities) const
    {
        u32 wantedImageCount = surfaceCapabilities.minImageCount + 1;

        // If unlimited max images, return the wanted image count.
        if (surfaceCapabilities.maxImageCount == 0)
            return wantedImageCount;

        // Otherwise, take the wanted image count if possible,
        // or the max image count if that's too much to ask.
        return std::min(wantedImageCount, surfaceCapabilities.maxImageCount);
    }

    VkSurfaceFormatKHR RenderContext::SelectSurfaceFormat() const
    {
        // Get surface format count.
        uint32_t surfaceFormatCount = 0;
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(s_PhysicalDevice, m_Surface, &surfaceFormatCount, nullptr);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get surface format count.");
        ENG_ASSERT(surfaceFormatCount > 0, "No surface formats available.");

        // Get surface formats.
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(s_PhysicalDevice, m_Surface, &surfaceFormatCount, surfaceFormats.data());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get surface formats.");

        // TODO: this is okay for now, but it should choose the
        // "best" format, not one hard coded RGBA sRGB format.
        for (auto& surfaceFormat : surfaceFormats)
            if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB and surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return surfaceFormat;

        // If no formats were our pick, just pick any and move on.
        return surfaceFormats.front();
    }

    VkPresentModeKHR RenderContext::SelectPresentMode() const
    {
        // Get present mode count.
        u32 presentModeCount = 0;
        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(s_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get present mode count.");
        ENG_ASSERT(presentModeCount > 0, "No present modes available.");

        // Get present modes.
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(s_PhysicalDevice, m_Surface, &presentModeCount, presentModes.data());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to get present modes.");

        // If VK_PRESENT_MODE_MAILBOX_KHR is available, use it.
        //for (VkPresentModeKHR presentMode : presentModes)
        //    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) // no vsync
        //        return presentMode;

        // Otherwise, fallback to VK_PRESENT_MODE_FIFO_KHR,
        // which is guaranteed to be available.
        return VK_PRESENT_MODE_FIFO_KHR; // vsync
    }
}
