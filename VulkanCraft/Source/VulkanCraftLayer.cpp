#include "VulkanCraftLayer.hpp"
#include <imgui.h>
#include <array>

namespace vc
{
    void VulkanCraftLayer::OnAttach()
    {
        // TODO: context retrieval needs to be reworked with multiple windows.
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        std::uint32_t swapchainImageCount = context.GetSwapchainImageCount();

        CreateRenderPass();
        CreateFramebuffer();

        {
            eng::ShaderInfo info;
            info.RenderContext = &context;
            info.Filepath = "Assets/Shaders/Basic";
            info.VertexBindings =
            {
                {0, VK_VERTEX_INPUT_RATE_VERTEX, {0, 1}},
            };
            info.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            info.RenderPass = m_RenderPass;
            m_Shader = std::make_unique<eng::Shader>(info);
        }

        {
            eng::VertexBufferInfo info;
            info.RenderContext = &context;
            info.Size = 1024; // 1 KiB for now, nothing fancy
            m_VertexBuffer = std::make_unique<eng::VertexBuffer>(info);

            constexpr auto data = std::to_array
            ({
                -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f,
                +1.0f, -1.0f, 0.0f, 0.5f, 1.0f, 0.5f,
                 0.0f, +1.0f, 0.0f, 0.5f, 0.5f, 1.0f,
            });
            m_VertexBuffer->SetData(data);
        }
    }

    void VulkanCraftLayer::OnDetach()
    {
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkDevice device = context.GetDevice();

        m_VertexBuffer.reset();
        m_Shader.reset();
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

        m_ViewProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 1000.0f)
            * glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(cosf(angle) * 0.5f, sinf(angle) * 0.5f, 1.0f)));
    }

    void VulkanCraftLayer::OnRender()
    {
        // TODO: my plan
        // RenderContext will have its own pipeline, renderpass, and framebuffers.
        // It will simply render a client framebuffer to a fullscreen quad. Since
        // it's using a shader, client framebuffer resizes can be called from OnEvent,
        // since it will be stretched, via a sampler2D, to fit the swapchain extent.
        // This will also allow for a modular Framebuffer.hpp/cpp in Engine/Rendering/
        // that the client can use one of, without having to rely on it being imageless.
        // Same thing for RenderPass.hpp/cpp.
        // 
        // Rendering order:
        //  1) RenderContext::BeginFrame
        //  2) begin first client renderpass
        //  3) render client stuff to client framebuffer
        //  4) end last client renderpass (likely same as first)
        //  5) OnImGuiRender
        //  6) execute imgui draw list (imgui's own renderpass, pipeline, vertex/index buffers)
        //  7) RenderContext::EndFrame
        // 
        // NOTE: ImGui "rendering" code (OnImGuiRender) doesn't actually execute
        // any graphics commands, it just collates all its textured quads into a
        // draw list. This draw list will then be used to actually render it.

        // TODO: Each window should own its own layer stack.
        // Store a window's render context as a pointer in each
        // layer with a "RenderContext& Layer::GetRenderContext()"?
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

        m_Shader->Bind(commandBuffer);
        m_Shader->GetUniformBuffer(0)->SetData(m_ViewProjection);
        m_Shader->UpdateDescriptorSet();

        m_VertexBuffer->Bind(commandBuffer, 0);

        // Vulkan's +y direction is down, this fixes that.
        // https://stackoverflow.com/questions/45570326/flipping-the-viewport-in-vulkan
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(extent.height);
        viewport.width = static_cast<float>(extent.width);
        viewport.height = -static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

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

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &colorAttachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;

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
