#include "VulkanCraftLayer.hpp"
#include <imgui.h>
#include <array>

namespace vc
{
    struct LocalUniformBuffer
    {
        alignas(16) glm::mat4 ViewProjection;

        struct alignas(16)
        {
            alignas(8) glm::uvec2 TextureCount;
            alignas(8) glm::vec2 TextureScale;         // 1.0f / TextureCount
            alignas(4) std::uint32_t TexturesPerLayer; // TextureCount.x * TextureCount.y
            alignas(4) float TextureThreshold;         // 0.5f / (size of a single texture in pixels)
        } BlockTextureAtlas;
    };

    void VulkanCraftLayer::OnAttach()
    {
        // TODO: context retrieval needs to be reworked with multiple windows.
        auto& context = eng::Application::Get().GetWindow().GetRenderContext();

        {
            std::array<VkAttachmentDescription, 2> attachments{};

            auto& colorAttachment = attachments[0];
            colorAttachment.format = context.GetSwapchainFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            auto& depthAttachment = attachments[1];
            depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentReference{};
            colorAttachmentReference.attachment = 0; // Index into info.pAttachments; referring to colorAttachment
            colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentReference{};
            depthAttachmentReference.attachment = 1; // Index into info.pAttachments; referring to colorAttachment
            depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentReference;
            subpass.pDepthStencilAttachment = &depthAttachmentReference;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            eng::RenderPassInfo info;
            info.RenderContext = &context;
            info.Attachments = attachments;
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
        }

        {
            eng::UniformBufferInfo info;
            info.RenderContext = &context;
            info.Size = sizeof(LocalUniformBuffer);
            m_UniformBuffer = std::make_shared<eng::UniformBuffer>(info);
        }

        {
            eng::StorageBufferInfo info;
            info.RenderContext = &context;
            info.Size = 1024; // 1 KiB for now, nothing fancy
            m_StorageBuffer = std::make_shared<eng::StorageBuffer>(info);
        }

        {
            eng::ShaderInfo info;
            info.RenderContext = &context;
            info.Filepath = "Assets/Shaders/Chunk";
            info.VertexBufferBindings =
            {
                {0, VK_VERTEX_INPUT_RATE_VERTEX, {0}},
            };
            info.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            info.RenderPass = m_RenderPass->GetRenderPass();
            m_Shader = std::make_shared<eng::Shader>(info);
        }

        {
#define VC_TEXTURE(x) R"(D:\Dorkspace\Programming\Archive\VanillaDefault-Resource-Pack-16x-1.21\assets\minecraft\textures\)" x
            std::vector<eng::LocalTexture> textures;
            textures.reserve(4);
            textures.emplace_back(VC_TEXTURE("block/amethyst_block.png"));
            textures.emplace_back(VC_TEXTURE("block/ancient_debris_side.png"));
            textures.emplace_back(VC_TEXTURE("block/ancient_debris_top.png"));
            textures.emplace_back(VC_TEXTURE("block/andesite.png"));
            m_BlockTextureAtlas = std::make_unique<TextureAtlas>(context, 16, textures);
        }

        m_CameraController.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
        m_CameraController.SetRotation(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
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

        m_BlockTextureAtlas.reset();

        m_Shader.reset();
        m_StorageBuffer.reset();
        m_UniformBuffer.reset();
        m_VertexBuffer.reset();
        m_Framebuffers.clear();
        m_FramebufferDepthAttachments.clear();
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

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {(std::cosf(angle) + 1.0f) * 0.5f, 0.0f, (std::sinf(angle) + 1.0f) * 0.5f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = m_RenderPass->GetRenderPass();
        info.framebuffer = m_Framebuffers[swapchainImageIndex]->GetFramebuffer();
        info.renderArea.extent = extent;
        info.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
        info.pClearValues = clearValues.data();

        // Begin the render pass.
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // Set all the necessary data.
        {
            m_VertexBuffer->SetData(std::to_array<glm::uvec2>
            ({
                {0, 0},
            }));

            LocalUniformBuffer localUniformBuffer;
            localUniformBuffer.ViewProjection = m_CameraController.GetViewProjection();
            localUniformBuffer.BlockTextureAtlas.TextureCount = m_BlockTextureAtlas->GetTextureCount();
            localUniformBuffer.BlockTextureAtlas.TextureScale = m_BlockTextureAtlas->GetTextureScale();
            localUniformBuffer.BlockTextureAtlas.TexturesPerLayer = m_BlockTextureAtlas->GetTexturesPerLayer();
            localUniformBuffer.BlockTextureAtlas.TextureThreshold = m_BlockTextureAtlas->GetTextureThreshold();
            m_UniformBuffer->SetData(localUniformBuffer);

            m_StorageBuffer->SetData(std::to_array<glm::uvec2>
            ({
                {0, 4 << 16}, // back face
            }));

            auto uniformBuffers = std::to_array<eng::ShaderUniformBufferBinding>
            ({
                {0, m_UniformBuffer},
            });
            auto storageBuffers = std::to_array<eng::ShaderStorageBufferBinding>
            ({
                {1, m_StorageBuffer},
            });
            auto samplers = std::to_array<eng::ShaderSamplerBinding>
            ({
                {2, m_BlockTextureAtlas->GetSampler(), m_BlockTextureAtlas->GetTexture()->GetImageView()},
            });

            m_Shader->UpdateDescriptorSet({uniformBuffers, storageBuffers, samplers});
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

        vkCmdDraw(commandBuffer, 12, 1, 0, 0);

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
        {
            m_FramebufferDepthAttachments.clear();
            m_FramebufferDepthAttachments.reserve(swapchainImageCount);

            eng::FramebufferAttachmentInfo framebufferAttachmentInfo;
            framebufferAttachmentInfo.RenderContext = &context;
            framebufferAttachmentInfo.Extent = swapchainExtent;
            framebufferAttachmentInfo.Format = VK_FORMAT_D24_UNORM_S8_UINT;
            framebufferAttachmentInfo.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            framebufferAttachmentInfo.Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            framebufferAttachmentInfo.Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            for (std::uint32_t i = 0; i < swapchainImageCount; i++)
                m_FramebufferDepthAttachments.push_back(std::make_shared<eng::FramebufferAttachment>(framebufferAttachmentInfo));
        }

        // Recreate the framebuffers.
        {
            m_Framebuffers.clear();
            m_Framebuffers.reserve(swapchainImageCount);

            eng::FramebufferInfo info;
            info.RenderContext = &context;
            info.RenderPass = m_RenderPass->GetRenderPass();
            for (std::uint32_t i = 0; i < swapchainImageCount; i++)
            {
                auto attachments = std::to_array
                ({
                    context.GetSwapchainImageView(i),
                    m_FramebufferDepthAttachments[i]->GetImageView(),
                });
                info.Attachments = attachments;
                m_Framebuffers.push_back(std::make_shared<eng::Framebuffer>(info));
            }
        }
    }
}
