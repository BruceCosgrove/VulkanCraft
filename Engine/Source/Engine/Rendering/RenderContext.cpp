#include "RenderContext.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Log.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <span>

#define _ENG_GET_FUNC_VK_EXT(name) \
    auto name = (PFN_##name)vkGetInstanceProcAddr(m_Instance, #name); \
    ENG_ASSERT(name != nullptr, "Failed to load Vulkan extension: \"" #name "\".")

namespace eng
{
    // TODO: remove
    static ImGui_ImplVulkanH_Window s_MainWindow{};

#if ENG_CONFIG_DEBUG
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, char const* pLayerPrefix, char const* pMessage, void* pUserData)
    {
        static_cast<void>(flags, objectType, object, location, messageCode, pLayerPrefix, pUserData);
        ENG_LOG_DEBUG("Vulkan debug report: {}", pMessage);
        return VK_FALSE;
    }
#endif

    VkInstance RenderContext::GetInstance() const
    {
        return m_Instance;
    }

    VkPhysicalDevice RenderContext::GetPhysicalDevice() const
    {
        return m_PhysicalDevice;
    }

    VkDevice RenderContext::GetDevice() const
    {
        return m_Device;
    }

    VkCommandBuffer RenderContext::GetCommandBuffer()
    {
        VkResult result = VK_SUCCESS;
        VkCommandBuffer& commandBuffer = m_AllocatedCommandBuffers[s_MainWindow.FrameIndex].emplace_back();

        // Allocate a command buffer.
        {
            VkCommandBufferAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = s_MainWindow.Frames[s_MainWindow.FrameIndex].CommandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;

            result = vkAllocateCommandBuffers(m_Device, &info, &commandBuffer);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffer.");
        }

        // Begin the command buffer.
        {
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            result = vkBeginCommandBuffer(commandBuffer, &info);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer.");
        }

        return commandBuffer;
    }

    void RenderContext::FlushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        VkResult result = VK_SUCCESS;

        // End the command buffer.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        result = vkEndCommandBuffer(commandBuffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to end command buffer.");

        // Create a fence to ensure the command buffer has finished executing.
        // TODO: reuse fences, seriously, creating and destroying them on the fly is so dumb.
        // ESPECIALLY when they were literally designed to be REUSED.
        VkFence fence = VK_NULL_HANDLE;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        result = vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &fence);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create fence.");

        result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to submit queue.");

        result = vkWaitForFences(m_Device, 1, &fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for fence.");

        vkDestroyFence(m_Device, fence, nullptr);
    }

    void RenderContext::SubmitResourceFree(std::function<void()>&& freeFunc)
    {
        m_ResourceFreeQueue[m_CurrentFrameIndex].emplace_back(std::move(freeFunc));
    }

    RenderContext::RenderContext(Window& window, std::function<void()>&& renderCallback, std::function<void()>&& imguiRenderCallback)
        : m_ContextWindow(window.m_Window)
        , m_RenderCallback(std::move(renderCallback))
        , m_ImGuiRenderCallback(std::move(imguiRenderCallback))
    {
        VkResult result = VK_SUCCESS;

        // Create the instance.
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
                "VK_EXT_debug_report",
#endif
            };

            std::uint32_t glfwExtensionCount;
            char const** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            extensions.append_range(std::span<char const*>(glfwExtensions, glfwExtensionCount));

            VkInstanceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            info.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
            info.ppEnabledLayerNames = layers.data();
            info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
            info.ppEnabledExtensionNames = extensions.data();

            result = vkCreateInstance(&info, nullptr, &m_Instance);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan instance.");
        }

#if ENG_CONFIG_DEBUG
        // Create the debug report callback.
        {
            VkDebugReportCallbackCreateInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            info.flags =
                VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_ERROR_BIT_EXT |
                VK_DEBUG_REPORT_DEBUG_BIT_EXT;
            info.pfnCallback = &DebugReport;

            _ENG_GET_FUNC_VK_EXT(vkCreateDebugReportCallbackEXT);
            result = vkCreateDebugReportCallbackEXT(m_Instance, &info, nullptr, &m_DebugReportCallback);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create debug report callback.");
        }
#endif

        // Select physical device.
        {
            // Get all physical devices.
            std::uint32_t count;
            result = vkEnumeratePhysicalDevices(m_Instance, &count, nullptr);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to get physical device count.");
            ENG_ASSERT(count > 0, "No physical devices available.");
            std::vector<VkPhysicalDevice> physicalDevices(count);
            result = vkEnumeratePhysicalDevices(m_Instance, &count, physicalDevices.data());
            ENG_ASSERT(result == VK_SUCCESS, "Failed to get physical devices.");

            // Select the most appropriate one.
            std::uint32_t fallbackPhysicalDeviceIndex = 0;
            std::uint32_t selectedPhysicalDeviceIndex = count;
            for (std::uint32_t i = 0; i < count; i++)
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
            m_PhysicalDevice = physicalDevices[selectedPhysicalDeviceIndex];
        }

        // Select queue families.
        {
            // Get all queue family properties.
            std::uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &count, nullptr);
            ENG_ASSERT(count > 0, "No queue families available.");
            std::vector<VkQueueFamilyProperties> queueFamilyProperties(count);
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &count, queueFamilyProperties.data());

            for (std::uint32_t i = 0; i < count; i++)
            {
                if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    m_GraphicsFamilyIndex = i;
                    break;
                }
            }

            ENG_ASSERT(m_GraphicsFamilyIndex != VK_QUEUE_FAMILY_IGNORED, "Failed to find appropriate queue family.");
        }

        // Create logical device.
        {
            std::vector<char const*> extensions
            {
                "VK_KHR_swapchain",
            };

            float priority = 1.0f;

            VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
            deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo.queueFamilyIndex = m_GraphicsFamilyIndex;
            deviceQueueCreateInfo.queueCount = 1;
            deviceQueueCreateInfo.pQueuePriorities = &priority;

            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.queueCreateInfoCount = 1;
            deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
            //deviceCreateInfo.enabledLayerCount
            //deviceCreateInfo.ppEnabledLayerNames
            deviceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

            result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create logical device.");

            vkGetDeviceQueue(m_Device, m_GraphicsFamilyIndex, 0, &m_GraphicsQueue);
        }

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

            result = vkCreateDescriptorPool(m_Device, &info, nullptr, &m_DescriptorPool);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool.");
        }

        // Create surface.
        result = glfwCreateWindowSurface(m_Instance, m_ContextWindow, nullptr, &m_Surface);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create window surface.");

        // Setup vulkan for the given window.
        {
            std::int32_t width, height;
            glfwGetFramebufferSize(m_ContextWindow, &width, &height);

            s_MainWindow.Surface = m_Surface;

            // TODO: this is supposed to use the presentation queue family index
            VkBool32 supported;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, m_GraphicsFamilyIndex, m_Surface, &supported);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to get surface support.");
            ENG_ASSERT(supported, "No surface support available.");

            // These ImGui helper functions for choosing settings are close to,
            // if not exactly, what I would've done, so they stay for now.

            // TODO: don't use imgui's helper function.
            VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
            VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            s_MainWindow.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_PhysicalDevice, m_Surface, &format, 1, colorSpace);

            // TODO: don't use imgui's helper function.
            VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR; // TODO: enable/disable vsync related
            s_MainWindow.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(m_PhysicalDevice, m_Surface, &presentMode, 1);

            // Use an extra image if vsync is on to buffer an extra frame when possible.
            m_MinImageCount = presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? 3u : 2u;

            // TODO: this is supposed to use the presentation queue family index
            ImGui_ImplVulkanH_CreateOrResizeWindow(m_Instance, m_PhysicalDevice, m_Device, &s_MainWindow, m_GraphicsFamilyIndex, nullptr, width, height, m_MinImageCount);

            m_AllocatedCommandBuffers.resize(s_MainWindow.ImageCount);
            m_ResourceFreeQueue.resize(s_MainWindow.ImageCount);
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
            info.Instance = m_Instance;
            info.PhysicalDevice = m_PhysicalDevice;
            info.Device = m_Device;
            // TODO: imgui seemingly assumes that the graphics queue and the presentation queue will be the same,
            // as they have only included one queue and queue family in their init info struct.
            // I will have to eventually replace the ImGui backend with my own implementation.
            info.QueueFamily = m_GraphicsFamilyIndex;
            info.Queue = m_GraphicsQueue;
            info.DescriptorPool = m_DescriptorPool;
            info.RenderPass = s_MainWindow.RenderPass;
            info.MinImageCount = m_MinImageCount;
            info.ImageCount = s_MainWindow.ImageCount;
            info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

            ENG_VERIFY(ImGui_ImplGlfw_InitForVulkan(m_ContextWindow, true), "Failed to initialize GLFW for ImGui.");
            ENG_VERIFY(ImGui_ImplVulkan_Init(&info), "Failed to initialize Vulkan for ImGui.");

            // TODO: load and upload font
        }
    }

    RenderContext::~RenderContext()
    {
        VkResult result = vkDeviceWaitIdle(m_Device);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for the device to stop working. This really shouldn't happen.");

        // Shutdown ImGui.
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        for (auto& queue : m_ResourceFreeQueue)
            for (auto& freeFunc : queue)
                freeFunc();
        m_ResourceFreeQueue.clear();

        ImGui_ImplVulkanH_DestroyWindow(m_Instance, m_Device, &s_MainWindow, nullptr);

        // Destroy vulkan objects.
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        vkDestroyDevice(m_Device, nullptr);
#if ENG_CONFIG_DEBUG
        _ENG_GET_FUNC_VK_EXT(vkDestroyDebugReportCallbackEXT);
        vkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReportCallback, nullptr);
#endif
        vkDestroyInstance(m_Instance, nullptr);
    }

    void RenderContext::DoFrame(bool render)
    {
        // Recreate the swapchain if necessary.
        if (m_RecreateSwapChain && render)
        {
            std::int32_t width, height;
            glfwGetFramebufferSize(m_ContextWindow, &width, &height);

            ImGui_ImplVulkanH_CreateOrResizeWindow(m_Instance, m_PhysicalDevice, m_Device, &s_MainWindow, m_GraphicsFamilyIndex, nullptr, width, height, m_MinImageCount);
            s_MainWindow.FrameIndex = 0;

            m_AllocatedCommandBuffers.clear();
            m_AllocatedCommandBuffers.resize(s_MainWindow.ImageCount);

            m_RecreateSwapChain = false;
        }

        // Update ImGui state and get its draw data for this frame.
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Do the Client's ImGui rendering.
            m_ImGuiRenderCallback();

            ImGui::Render();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // If not rendering, don't.
        if (!render)
            return;

        VkResult result = VK_SUCCESS;

        VkSemaphore imageAcquiredSemaphore = s_MainWindow.FrameSemaphores[s_MainWindow.SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore renderCompleteSemaphore = s_MainWindow.FrameSemaphores[s_MainWindow.SemaphoreIndex].RenderCompleteSemaphore;

        // Get the next image to render the frame to.
        result = vkAcquireNextImageKHR(m_Device, s_MainWindow.Swapchain, std::numeric_limits<std::uint64_t>::max(), imageAcquiredSemaphore, nullptr, &s_MainWindow.FrameIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            m_RecreateSwapChain = true;
            return;
        }
        ENG_ASSERT(result == VK_SUCCESS, "Failed to acquire next swapchain image.");

        ImGui_ImplVulkanH_Frame& frame = s_MainWindow.Frames[s_MainWindow.FrameIndex];

        // Wait for the image to be available.
        result = vkWaitForFences(m_Device, 1, &frame.Fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for frame fence.");

        result = vkResetFences(m_Device, 1, &frame.Fence);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to reset frame fence.");

        // Clear resources in the free queue.
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % s_MainWindow.ImageCount;
        for (auto& freeFunc : m_ResourceFreeQueue[m_CurrentFrameIndex])
            freeFunc();
        m_ResourceFreeQueue[m_CurrentFrameIndex].clear();

        // Reset the command buffers and pool.
        {
            auto& commandBuffers = m_AllocatedCommandBuffers[s_MainWindow.FrameIndex];
            if (!commandBuffers.empty())
            {
                vkFreeCommandBuffers(m_Device, frame.CommandPool, static_cast<std::uint32_t>(commandBuffers.size()), commandBuffers.data());
                commandBuffers.clear();
            }

            result = vkResetCommandPool(m_Device, frame.CommandPool, 0);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to reset frame command pool.");
        }

        // Begin the frame's command buffer.
        {
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            result = vkBeginCommandBuffer(frame.CommandBuffer, &info);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer.");
        }

        // Do the render pass.
        {
            VkRenderPassBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = s_MainWindow.RenderPass;
            info.framebuffer = frame.Framebuffer;
            info.renderArea.extent.width = static_cast<std::uint32_t>(s_MainWindow.Width);
            info.renderArea.extent.height = static_cast<std::uint32_t>(s_MainWindow.Height);
            info.clearValueCount = 1;
            info.pClearValues = &s_MainWindow.ClearValue;

            // Begin the render pass.
            vkCmdBeginRenderPass(frame.CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
            // Do all Client rendering.
            m_RenderCallback();
            // Do all ImGui rendering.
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frame.CommandBuffer);
            // End the render pass.
            vkCmdEndRenderPass(frame.CommandBuffer);
        }

        // End the frame's command buffer and submit
        {
            VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &imageAcquiredSemaphore;
            info.pWaitDstStageMask = &waitMask;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &frame.CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &renderCompleteSemaphore;

            result = vkEndCommandBuffer(frame.CommandBuffer);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to end command buffer.");
            result = vkQueueSubmit(m_GraphicsQueue, 1, &info, frame.Fence);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to submit render queue.");
        }
        
        // Present the most recently completed frame to the surface.
        {
            VkPresentInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &renderCompleteSemaphore;
            info.swapchainCount = 1;
            info.pSwapchains = &s_MainWindow.Swapchain;
            info.pImageIndices = &s_MainWindow.FrameIndex;
        
            VkResult result = vkQueuePresentKHR(m_GraphicsQueue, &info);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                m_RecreateSwapChain = true;
                return;
            }
            ENG_ASSERT(result == VK_SUCCESS, "Failed to present frame.");

            // Use the next semaphore.
            s_MainWindow.SemaphoreIndex = (s_MainWindow.SemaphoreIndex + 1) % s_MainWindow.ImageCount;
        }
    }
}
