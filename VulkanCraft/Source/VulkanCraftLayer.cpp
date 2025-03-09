#include "VulkanCraftLayer.hpp"
#include <imgui.h>
#include <array>

namespace vc
{
    struct LocalUniformBuffer
    {
        alignas(16) glm::mat4 ViewProjection;
    };

    void VulkanCraftLayer::OnAttach()
    {
        // TODO: context retrieval needs to be reworked with multiple windows.
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();

        {
            // TODO: update pipeline to add depth

            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = context.GetSwapchainFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

            eng::RenderPassInfo info;
            info.RenderContext = &context;
            info.Attachments = std::span(&colorAttachment, 1);
            info.Subpasses = std::span(&subpass, 1);
            info.SubpassDependencies = std::span(&dependency, 1);
            m_RenderPass = std::make_shared<eng::RenderPass>(info);
        }

        CreateOrRecreateFramebuffers();

        {
            eng::VertexBufferInfo info;
            info.RenderContext = &context;
            info.Size = 1024; // 1 KiB for now, nothing fancy
            m_VertexBuffer = std::make_shared<eng::VertexBuffer>(info);

            constexpr auto data = std::to_array
            ({
                -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // 0
                +0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // 1
                +0.5f, +0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // 2
                +0.5f, +0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // 2
                -0.5f, +0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, // 3
                -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // 0
            });
            m_VertexBuffer->SetData(data);
        }

        {
            eng::UniformBufferInfo info;
            info.RenderContext = &context;
            info.Size = sizeof(LocalUniformBuffer);
            m_UniformBuffer = std::make_shared<eng::UniformBuffer>(info);
        }

        {
            eng::ShaderInfo info;
            info.RenderContext = &context;
            info.Filepath = "Assets/Shaders/Basic";
            info.VertexBufferBindings =
            {
                {0, VK_VERTEX_INPUT_RATE_VERTEX, {0, 1, 2}},
            };
            info.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            info.RenderPass = m_RenderPass->GetRenderPass();
            m_Shader = std::make_shared<eng::Shader>(info);
        }

        {
            VkSamplerCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            info.magFilter = VK_FILTER_NEAREST;
            info.minFilter = VK_FILTER_LINEAR;
            info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            info.anisotropyEnable = VK_TRUE; // TODO: slider for performance, 0 (off) -> max
            info.maxAnisotropy = eng::RenderContext::GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
            info.compareEnable = VK_FALSE;
            info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            info.unnormalizedCoordinates = VK_FALSE;

            VkResult result = vkCreateSampler(context.GetDevice(), &info, nullptr, &m_Sampler);
            ENG_ASSERT(result == VK_SUCCESS, "Failed to create sampler.");
        }

        {
#define VC_TEXTURE(x) R"(D:\Dorkspace\Programming\Archive\VanillaDefault-Resource-Pack-16x-1.21\assets\minecraft\textures\)" x
            eng::LocalTexture2D texture(VC_TEXTURE("block/ancient_debris_side.png"));

            eng::Texture2DInfo info;
            info.RenderContext = &context;
            info.LocalTexture = &texture;
            m_Texture = std::make_shared<eng::Texture2D>(info);
        }

        m_CameraController.SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
        m_CameraController.SetFOV(glm::radians(90.0f));
        m_CameraController.SetNearPlane(0.001f);
        m_CameraController.SetFarPlane(1000.0f);
        m_CameraController.SetMovementSpeed(1.0f);
        m_CameraController.SetMouseSensitivity(1.0f);
    }

    void VulkanCraftLayer::OnDetach()
    {
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkDevice device = context.GetDevice();

        m_Texture.reset();
        vkDestroySampler(device, m_Sampler, nullptr);

        m_Shader.reset();
        m_UniformBuffer.reset();
        m_VertexBuffer.reset();
        m_Framebuffers.clear();
        m_FramebufferColorAttachments.clear();
        m_RenderPass.reset();
    }

    void VulkanCraftLayer::OnEvent(eng::Event& event)
    {
        // TODO: remove
        //ENG_LOG_DEBUG("VulkanCraftLayer::OnEvent(TODO: event logging)");
        event.Dispatch(this, &VulkanCraftLayer::OnWindowCloseEvent);
        event.Dispatch(&m_CameraController, &CameraController::OnEvent);
    }

    static float angle = 0.0f;
    void VulkanCraftLayer::OnUpdate(eng::Timestep timestep)
    {
        angle += timestep;
        m_CameraController.OnUpdate(timestep);
    }

    void VulkanCraftLayer::OnRender()
    {
        // TODO: Store a window's render context as a pointer in each
        // layer with a "RenderContext& Layer::GetRenderContext()"?
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkExtent2D extent = context.GetSwapchainExtent();
        VkCommandBuffer commandBuffer = context.GetActiveCommandBuffer();
        std::uint32_t swapchainImageIndex = context.GetSwapchainImageIndex();

        // Recreate the framebuffer if necessary.
        if (context.WasSwapchainRecreated())
            CreateOrRecreateFramebuffers();

        VkClearValue clearValue{};
        clearValue.color = {(std::cosf(angle) + 1.0f) * 0.5f, 0.0f, (std::sinf(angle) + 1.0f) * 0.5f, 1.0f};

        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = m_RenderPass->GetRenderPass();
        info.framebuffer = m_Framebuffers[swapchainImageIndex]->GetFramebuffer();
        info.renderArea.extent = extent;
        info.clearValueCount = 1;
        info.pClearValues = &clearValue;

        // Begin the render pass.
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // Set all the necessary data.
        {
            LocalUniformBuffer localUniformBuffer;
            localUniformBuffer.ViewProjection = m_CameraController.GetViewProjection();
            m_UniformBuffer->SetData(localUniformBuffer);

            eng::ShaderUniformBufferBinding binding0(0, m_UniformBuffer);
            eng::ShaderSamplerBinding binding1(1, m_Sampler, m_Texture->GetImageView());

            auto uniformBuffers = std::to_array({binding0});
            auto samplers = std::to_array({binding1});

            m_Shader->UpdateDescriptorSet({uniformBuffers, samplers});
        }

        // Bind everything.
        m_Shader->Bind(commandBuffer);
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

        vkCmdDraw(commandBuffer, 6, 1, 0, 0);

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

    void VulkanCraftLayer::CreateOrRecreateFramebuffers()
    {
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();
        VkExtent2D swapchainExtent = context.GetSwapchainExtent();
        VkFormat swapchainFormat = context.GetSwapchainFormat();
        std::uint32_t swapchainImageCount = context.GetSwapchainImageCount();

        // Recreate the framebuffer color attachments.
        //{
        //    m_FramebufferColorAttachments.clear();
        //    m_FramebufferColorAttachments.reserve(swapchainImageCount);

        //    eng::FramebufferAttachmentInfo framebufferAttachmentInfo;
        //    framebufferAttachmentInfo.RenderContext = &context;
        //    framebufferAttachmentInfo.Extent = swapchainExtent;
        //    framebufferAttachmentInfo.Format = swapchainFormat;
        //    framebufferAttachmentInfo.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        //    framebufferAttachmentInfo.Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        //    framebufferAttachmentInfo.Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        //    for (std::uint32_t i = 0; i < swapchainImageCount; i++)
        //        m_FramebufferColorAttachments.push_back(std::make_shared<eng::FramebufferAttachment>(framebufferAttachmentInfo));
        //}

        // Recreate the framebuffers.
        {
            m_Framebuffers.clear();
            m_Framebuffers.reserve(swapchainImageCount);

            eng::FramebufferInfo info;
            info.RenderContext = &context;
            info.RenderPass = m_RenderPass->GetRenderPass();
            for (std::uint32_t i = 0; i < swapchainImageCount; i++)
            {
                //VkImageView colorAttachment = m_FramebufferColorAttachments[i]->GetImageView();
                VkImageView colorAttachment = context.GetSwapchainImageView(i);
                info.Attachments = std::span(&colorAttachment, 1);
                m_Framebuffers.push_back(std::make_shared<eng::Framebuffer>(info));
            }
        }
    }
}
