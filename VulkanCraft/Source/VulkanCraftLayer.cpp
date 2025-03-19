#include "VulkanCraftLayer.hpp"
#include <imgui.h>
#include <array>

namespace vc
{
    struct LocalUniformBuffer
    {
        alignas(16) mat4 ViewProjection;

        struct alignas(16)
        {
            alignas(8) uvec3 TextureCount;
            alignas(8) vec2 TextureScale;    // 1.0f / TextureCount
            alignas(4) u32 TexturesPerLayer; // TextureCount.x * TextureCount.y
        } BlockTextureAtlas;
    };

    VulkanCraftLayer::VulkanCraftLayer(Window& window)
        : Layer(window)
        , m_RenderPass(CreateRenderPass())
        , m_Shader(LoadShader())
        , m_ImGuiRenderContext(window, m_RenderPass->GetRenderPass())
    {
        auto& context = window.GetRenderContext();

        CreateOrRecreateFramebuffers();

        {
            VertexBufferInfo info;
            info.RenderContext = &context;
            info.Size = 1024; // 1 KiB for now, nothing fancy
            m_VertexBuffer = std::make_shared<VertexBuffer>(info);
        }

        {
            UniformBufferInfo info;
            info.RenderContext = &context;
            info.Size = sizeof(LocalUniformBuffer);
            m_UniformBuffer = std::make_shared<UniformBuffer>(info);
        }

        {
            StorageBufferInfo info;
            info.RenderContext = &context;
            info.Size = 1024; // 1 KiB for now, nothing fancy
            m_StorageBuffer = std::make_shared<StorageBuffer>(info);
        }

        {
#define VC_TEXTURE(x) R"(D:\Dorkspace\Programming\Archive\VanillaDefault-Resource-Pack-16x-1.21\assets\minecraft\textures\)" x
            std::vector<LocalTexture> textures;
            textures.reserve(4);
            textures.emplace_back(VC_TEXTURE("block/amethyst_block.png"));
            textures.emplace_back(VC_TEXTURE("block/ancient_debris_side.png"));
            textures.emplace_back(VC_TEXTURE("block/ancient_debris_top.png"));
            textures.emplace_back(VC_TEXTURE("block/andesite.png"));
            textures.emplace_back(VC_TEXTURE("block/bedrock.png"));
            m_BlockTextureAtlas = std::make_unique<TextureAtlas>(context, 16, textures);
        }

        m_CameraController.SetPosition(vec3(0.0f, 0.0f, -1.0f));
        m_CameraController.SetRotation(vec3(0.0f, glm::radians(180.0f), 0.0f));
        m_CameraController.SetFOV(glm::radians(90.0f));
        m_CameraController.SetNearPlane(0.001f);
        m_CameraController.SetFarPlane(1000.0f);
        m_CameraController.SetMovementSpeed(2.0f);
        m_CameraController.SetMouseSensitivity(1.0f);
    }

    void VulkanCraftLayer::OnEvent(Event& event)
    {
        event.Dispatch(&m_ImGuiRenderContext, &ImGuiRenderContext::OnEvent);
        event.Dispatch(this, &VulkanCraftLayer::OnWindowCloseEvent);
        event.Dispatch(this, &VulkanCraftLayer::OnKeyPressEvent);
        event.Dispatch(&m_CameraController, &CameraController::OnEvent);
    }

    void VulkanCraftLayer::OnUpdate(Timestep timestep)
    {
        m_CameraController.OnUpdate(timestep);
    }

    void VulkanCraftLayer::OnRender(Timestep timestep)
    {
        auto& context = Layer::GetWindow().GetRenderContext();
        VkCommandBuffer commandBuffer = context.GetActiveCommandBuffer();

        // Recreate the framebuffer if necessary.
        if (context.WasSwapchainRecreated())
            CreateOrRecreateFramebuffers();

        auto shader = m_Shader.Get();
        if (shader.Old) // Defer old shader deletion until all previous frames have finished using it.
            context.DeferFree([shader = std::move(shader.Old)] {});

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.2f, 0.3f, 0.8f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = m_RenderPass->GetRenderPass();
        info.framebuffer = m_Framebuffers[context.GetSwapchainImageIndex()]->GetFramebuffer();
        info.renderArea.extent = context.GetSwapchainExtent();
        info.clearValueCount = static_cast<u32>(clearValues.size());
        info.pClearValues = clearValues.data();

        // Begin the render pass.
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // Set all the necessary data.
        {
            m_VertexBuffer->SetData(std::to_array<uvec2>
            ({
                {0 << 16 | 1, 17 << 12 | 0}, // left
                {1 << 16 | 1, 17 << 12 | 1 << 0}, // right
                {2 << 16 | 2, 17 << 12 | 0}, // bottom
                {3 << 16 | 2, 17 << 12 | 1 << 4}, // top
                {4 << 16 | 1, 17 << 12 | 0}, // back
                {5 << 16 | 1, 17 << 12 | 1 << 8}, // front
            }));

            LocalUniformBuffer localUniformBuffer;
            localUniformBuffer.ViewProjection = m_CameraController.GetViewProjection();
            localUniformBuffer.BlockTextureAtlas.TextureCount = m_BlockTextureAtlas->GetTextureCount();
            localUniformBuffer.BlockTextureAtlas.TextureScale = m_BlockTextureAtlas->GetTextureScale();
            localUniformBuffer.BlockTextureAtlas.TexturesPerLayer = m_BlockTextureAtlas->GetTexturesPerLayer();
            m_UniformBuffer->SetData(localUniformBuffer);

            m_StorageBuffer->SetData(std::to_array<uvec2>
            ({
                {0, 0 << 16}, // left
                {0, 1 << 16}, // right
                {0, 2 << 16}, // bottom
                {0, 3 << 16}, // top
                {0, 4 << 16}, // back
                {0, 5 << 16}, // front
            }));

            auto uniformBuffers = std::to_array<ShaderUniformBufferBinding>
            ({
                {0, m_UniformBuffer},
            });
            auto storageBuffers = std::to_array<ShaderStorageBufferBinding>
            ({
                {1, m_StorageBuffer},
            });
            auto samplers = std::to_array<ShaderSamplerBinding>
            ({
                {2, m_BlockTextureAtlas->GetSampler(), m_BlockTextureAtlas->GetTexture()->GetImageView()},
            });

            shader.Current->UpdateDescriptorSet({uniformBuffers, storageBuffers, samplers});

            // Bind everything.
            shader.Current->Bind(commandBuffer);
            m_VertexBuffer->Bind(commandBuffer, 0);

            SetDefaultViewportAndScissor();
            vkCmdDraw(commandBuffer, 6, 6, 0, 0);
        }

        // Render ImGui
        {
            m_ImGuiRenderContext.BeginFrame(timestep);
            OnImGuiRender();
            m_ImGuiRenderContext.EndFrame(commandBuffer);
        }

        // End the render pass.
        vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanCraftLayer::OnWindowCloseEvent(WindowCloseEvent& event)
    {
        Application::Get().Terminate();
    }

    void VulkanCraftLayer::OnKeyPressEvent(KeyPressEvent& event)
    {
        if (event.IsPressed() and event.GetKeycode() == Keycode::R and event.GetModifiers().HasOnly(Modifiers::Control))
        {
            if (not m_Shader.Loading())
            {
                Application::Get().ExecuteAsync([this]
                {
                    m_Shader.Load([this] { return LoadShader(); });
                });
            }
        }
    }

    void VulkanCraftLayer::OnImGuiRender()
    {
        // TODO: multiple threads broke imgui extra viewports.
        // THATS PROBABLY RELATED TO WHY SetWindowLongW BLOCKS in ImGui_ImplGlfw_NewFrame.
        // I forgot ImGui wasn't thread safe.
        // I need to initialize and shutdown its context on the render thread, cause it's
        // only ever called from there, so a single context should do.

        ImGui::ShowDemoWindow();

        ImGui::Begin("test window");
        ImGui::Button("test button");
        ImGui::End();
    }

    std::shared_ptr<RenderPass> VulkanCraftLayer::CreateRenderPass()
    {
        auto& context = Layer::GetWindow().GetRenderContext();

        // For slightly more complicated subpass setups, use this for reference.
        // https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/

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
        depthAttachmentReference.attachment = 1; // Index into info.pAttachments; referring to depthAttachment
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpasses{};

        auto& subpass0 = subpasses[0];
        subpass0.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass0.colorAttachmentCount = 1;
        subpass0.pColorAttachments = &colorAttachmentReference;
        subpass0.pDepthStencilAttachment = &depthAttachmentReference;

        std::array<VkSubpassDependency, 1> dependencies{};

        auto& dependency0 = dependencies[0];
        dependency0.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency0.dstSubpass = 0;
        dependency0.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency0.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency0.srcAccessMask = 0;
        dependency0.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        RenderPassInfo info;
        info.RenderContext = &context;
        info.Attachments = attachments;
        info.Subpasses = subpasses;
        info.SubpassDependencies = dependencies;
        return std::make_shared<RenderPass>(info);
    }

    void VulkanCraftLayer::CreateOrRecreateFramebuffers()
    {
        auto& context = Layer::GetWindow().GetRenderContext();
        VkExtent2D swapchainExtent = context.GetSwapchainExtent();
        u32 swapchainImageCount = context.GetSwapchainImageCount();

        // Recreate the framebuffer depth attachments.
        {
            m_FramebufferDepthAttachments.clear();
            m_FramebufferDepthAttachments.reserve(swapchainImageCount);

            FramebufferAttachmentInfo info;
            info.RenderContext = &context;
            info.Extent = swapchainExtent;
            info.Format = VK_FORMAT_D24_UNORM_S8_UINT;
            info.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            info.Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            info.Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            for (u32 i = 0; i < swapchainImageCount; i++)
                m_FramebufferDepthAttachments.push_back(std::make_shared<FramebufferAttachment>(info));
        }

        // Recreate the framebuffers.
        {
            m_Framebuffers.clear();
            m_Framebuffers.reserve(swapchainImageCount);

            FramebufferInfo info;
            info.RenderContext = &context;
            info.RenderPass = m_RenderPass->GetRenderPass();

            for (u32 i = 0; i < swapchainImageCount; i++)
            {
                auto attachments = std::to_array
                ({
                    context.GetSwapchainImageView(i),
                    m_FramebufferDepthAttachments[i]->GetImageView(),
                });
                info.Attachments = attachments;
                m_Framebuffers.push_back(std::make_shared<Framebuffer>(info));
            }
        }
    }

    std::shared_ptr<Shader> VulkanCraftLayer::LoadShader()
    {
        auto& context = Layer::GetWindow().GetRenderContext();

        auto bindings = std::to_array<ShaderVertexBufferBinding>
        ({
            {0, VK_VERTEX_INPUT_RATE_INSTANCE, {0}},
        });

        ShaderInfo info;
        info.RenderContext = &context;
        info.Filepath = "Assets/Shaders/Chunk";
        info.VertexBufferBindings = bindings;
        info.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        info.RenderPass = m_RenderPass->GetRenderPass();

        ENG_LOG_INFO("Loading shader \"{}\"...", info.Filepath.string());
        return std::make_shared<Shader>(info);
    }

    void VulkanCraftLayer::SetDefaultViewportAndScissor()
    {
        auto& context = Layer::GetWindow().GetRenderContext();
        VkExtent2D extent = context.GetSwapchainExtent();
        VkCommandBuffer commandBuffer = context.GetActiveCommandBuffer();

        // Vulkan's +y direction is down, this fixes that.
        // https://stackoverflow.com/questions/45570326/flipping-the-viewport-in-vulkan
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<f32>(extent.height);
        viewport.width = static_cast<f32>(extent.width);
        viewport.height = -static_cast<f32>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
}
