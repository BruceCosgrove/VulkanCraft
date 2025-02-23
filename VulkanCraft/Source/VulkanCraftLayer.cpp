#include "VulkanCraftLayer.hpp"
#include <imgui.h>

namespace vc
{
    void VulkanCraftLayer::OnAttach()
    {
        CreateRenderPass();
        CreateFramebuffer();
    }

    void VulkanCraftLayer::OnDetach()
    {
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkDevice device = context.GetDevice();

        vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
        vkDestroyRenderPass(device, m_RenderPass, nullptr);
    }

    void VulkanCraftLayer::OnEvent(eng::Event& event)
    {
        // TODO: remove
        ENG_LOG_DEBUG("VulkanCraftLayer::OnEvent(TODO: event logging)");
        event.Dispatch(this, &VulkanCraftLayer::OnWindowCloseEvent);
    }

    static float angle = 0.0f;
    void VulkanCraftLayer::OnUpdate(eng::Timestep timestep)
    {
        angle += timestep;
    }

    void VulkanCraftLayer::OnRender()
    {
        // TODO: my plan
        // 1) render client stuff to framebuffer
        // 2) clear depth buffer
        // 3) keep framebuffer unchanged between render passes
        // 4) render imgui on top of that same framebuffer
        // 5) present that framebuffer to the swapchain

        // TODO: Each window should own its own layer stack.
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkDevice device = context.GetDevice();
        VkExtent2D extent = context.GetSwapchainExtent();
        VkImageView imageView = context.GetActiveSwapchainImageView();
        VkCommandBuffer commandBuffer = context.GetActiveCommandBuffer();

        // Recreate the framebuffer.
        if (context.WasSwapchainRecreated())
        {
            vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
            CreateFramebuffer();
        };

        VkRenderPassAttachmentBeginInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
        attachmentInfo.attachmentCount = 1;
        attachmentInfo.pAttachments = &imageView;

        VkClearValue clearValue{};
        clearValue.color = {(std::cosf(angle) + 1.0f) * 0.5f, 0.0f, (std::sinf(angle) + 1.0f) * 0.5f, 1.0f};

        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.pNext = &attachmentInfo;
        info.renderPass = m_RenderPass;
        info.framebuffer = m_Framebuffer;
        info.renderArea.extent = extent;
        info.clearValueCount = 1;
        info.pClearValues = &clearValue;

        // Begin the render pass.
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // DO THE BIG RENDER

        // End the render pass.
        vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanCraftLayer::OnImGuiRender()
    {
        ImGui::ShowDemoWindow();

        ImGui::Begin("test window");
        ImGui::Button("test button");
        ImGui::End();
    }

    void VulkanCraftLayer::OnWindowCloseEvent(eng::WindowCloseEvent& event)
    {
        eng::Application::Get().Terminate();
    }

    void VulkanCraftLayer::CreateRenderPass()
    {
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkFormat format = context.GetSwapchainFormat();
        VkDevice device = context.GetDevice();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // TODO: relevant
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: relevant
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: relevant
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO: relevant

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0; // Index into info.pAttachments; referring to colorAttachment
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentReference;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &colorAttachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;

        VkResult result = vkCreateRenderPass(device, &info, nullptr, &m_RenderPass);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create render pass.");
    }

    void VulkanCraftLayer::CreateFramebuffer()
    {
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkFormat format = context.GetSwapchainFormat();
        VkDevice device = context.GetDevice();
        VkExtent2D extent = context.GetSwapchainExtent();

        // Defer giving the framebuffer an image until the render pass is began.

        VkFramebufferAttachmentImageInfo attachmentImageInfo{};
        attachmentImageInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
        attachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        attachmentImageInfo.width = extent.width;
        attachmentImageInfo.height = extent.height;
        attachmentImageInfo.layerCount = 1;

        // TODO: this MIGHT be required to render this as a texture in imgui
        // https://registry.khronos.org/vulkan/specs/latest/man/html/VkFramebufferAttachmentImageInfo.html
        // TODO: is this required???
        attachmentImageInfo.viewFormatCount = 1;
        attachmentImageInfo.pViewFormats = &format;

        VkFramebufferAttachmentsCreateInfo attachmentsInfo{};
        attachmentsInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
        attachmentsInfo.attachmentImageInfoCount = 1;
        attachmentsInfo.pAttachmentImageInfos = &attachmentImageInfo;

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = &attachmentsInfo;
        framebufferInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        // TODO: keep this framebuffer in sync with the window resizes.
        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer);
        ENG_ASSERT(result == VK_SUCCESS, "Failed to create swapchain framebuffer.");
    }
}
