#include "VulkanCraftLayer.hpp"
#include "VulkanCraft/Rendering/BlockModel.hpp"
#include <array>

namespace vc
{
    VulkanCraftLayer::VulkanCraftLayer(Window& window)
        : Layer(window)
    {
        CreateRenderPass();
        CreateOrRecreateFramebuffers();

        m_ImGuiRenderContext = std::make_unique<ImGuiRenderContext>(window, m_RenderPass->GetRenderPass());
        m_ImGuiHelper = std::make_unique<ImGuiHelper>();

        m_WorldRenderer = std::make_unique<WorldRenderer>(window.GetRenderContext(), m_RenderPass->GetRenderPass(), 512); // TODO: render distance
        m_World = std::make_unique<World>();

        m_CameraController.SetPosition({0.0f, 64.0f, 0.0f});
        m_CameraController.SetRotation({glm::radians(-90.0f), glm::radians(180.0f), 0.0f});
        m_CameraController.SetFOV(glm::radians(90.0f));
        m_CameraController.SetNearPlane(0.001f);
        m_CameraController.SetFarPlane(1000.0f);
        m_CameraController.SetMovementSpeed(8.0f);
        m_CameraController.SetMouseSensitivity(2.0f);
    }

    void VulkanCraftLayer::OnEvent(Event& event)
    {
        event.Dispatch(m_ImGuiRenderContext.get(), &ImGuiRenderContext::OnEvent);
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

        auto clearValues = std::to_array<VkClearValue>
        ({
            {.color{0.2f, 0.3f, 0.8f, 1.0f}},
            {.depthStencil{1.0f, 0}},
        });

        // Begin the render pass.
        VkRenderPassBeginInfo info
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_RenderPass->GetRenderPass(),
            .framebuffer = m_Framebuffers[context.GetSwapchainImageIndex()]->GetFramebuffer(),
            .renderArea{.extent = context.GetSwapchainExtent()},
            .clearValueCount = u32(clearValues.size()),
            .pClearValues = clearValues.data(),
        };
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // Render world
        m_WorldRendererStatistics = m_WorldRenderer->Render(commandBuffer, *m_World, m_CameraController.GetViewProjection());

        // Render ImGui
        // TODO: SetWindowLongW, called from ImGui_ImplGlfw_NewFrame,
        // blocks execution if the application is terminated between
        // RenderContext::BeginFrame() and RenderContext::EndFrame().
        if (Application::Get().IsRunning())
        {
            ENG_GET_FUNC_VK_EXT(vkCmdSetPolygonModeEXT);
            vkCmdSetPolygonModeEXT(commandBuffer, VK_POLYGON_MODE_FILL);
            m_ImGuiRenderContext->BeginFrame();
            OnImGuiRender();
            m_ImGuiRenderContext->EndFrame(commandBuffer);
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
        if (event.IsPressed() and event.GetModifiers() == Modifiers::None)
        {
            switch (+event.GetKeycode())
            {
                case Keycode::F1: m_WorldRenderer->ReloadShaders(); break;
                case Keycode::F2: m_WorldRenderer->ToggleWireframe(); break;
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

        //ImGui::ShowDemoWindow();

        if (ImGui::Begin("World Renderer Statistics"))
        {
            constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("world renderer statistics table", 2, tableFlags))
            {
                auto tableEntry = [](small_string_view name, small_string_view value)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text(name.data());
                    ImGui::TableNextColumn();
                    ImGui::Text(value.data());
                };
                small_string indirectDrawCallCount = std::to_string(m_WorldRendererStatistics.IndirectDrawCallCount);
                small_string instanceCount = std::to_string(m_WorldRendererStatistics.InstanceCount);
                small_string chunkCount = std::to_string(m_WorldRendererStatistics.ChunkCount);
                small_string usedVertexBufferSize = std::to_string(m_WorldRendererStatistics.UsedVertexBufferSize);
                small_string usedUniformBufferSize = std::to_string(m_WorldRendererStatistics.UsedUniformBufferSize);
                small_string usedStorageBufferSize = std::to_string(m_WorldRendererStatistics.UsedStorageBufferSize);
                small_string usedIndirectBufferSize = std::to_string(m_WorldRendererStatistics.UsedIndirectBufferSize);
                tableEntry("Indirect Draw Call Count", indirectDrawCallCount);
                tableEntry("Instance Count", instanceCount);
                tableEntry("Chunk Count", chunkCount);
                tableEntry("Used Vertex Buffer Size", usedVertexBufferSize);
                tableEntry("Used Uniform Buffer Size", usedUniformBufferSize);
                tableEntry("Used Storage Buffer Size", usedStorageBufferSize);
                tableEntry("Used Indirect Buffer Size", usedIndirectBufferSize);
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void VulkanCraftLayer::CreateRenderPass()
    {
        auto& context = Layer::GetWindow().GetRenderContext();

        // For slightly more complicated subpass setups, use this for reference.
        // https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/

        auto attachments = std::to_array<VkAttachmentDescription>
        ({
            // Color attachment
            {
                .format = context.GetSwapchainFormat(),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
            // Depth attachment
            {
                .format = VK_FORMAT_D24_UNORM_S8_UINT,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
        });

        VkAttachmentReference colorAttachmentReference
        {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference depthAttachmentReference
        {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        auto subpasses = std::to_array<VkSubpassDescription>
        ({
            {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentReference,
                .pDepthStencilAttachment = &depthAttachmentReference,
            },
        });

        auto dependencies = std::to_array<VkSubpassDependency>
        ({
            {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            },
        });

        RenderPassInfo info
        {
            .RenderContext = &context,
            .Attachments = attachments,
            .Subpasses = subpasses,
            .SubpassDependencies = dependencies,
        };
        m_RenderPass = std::make_shared<RenderPass>(info);
    }

    void VulkanCraftLayer::CreateOrRecreateFramebuffers()
    {
        auto& context = Layer::GetWindow().GetRenderContext();
        u32 swapchainImageCount = context.GetSwapchainImageCount();

        // Delete the old framebuffer.
        if (not m_Framebuffers.empty())
        {
            context.DeferFree([
                oldFramebufferDepthAttachments = std::move(m_FramebufferDepthAttachments),
                oldFramebuffers = std::move(m_Framebuffers)
            ] {});
        }

        // Recreate the framebuffer depth attachments.
        {
            m_FramebufferDepthAttachments.reserve(swapchainImageCount);

            FramebufferAttachmentInfo info
            {
                .RenderContext = &context,
                .Extent = context.GetSwapchainExtent(),
                .Format = VK_FORMAT_D24_UNORM_S8_UINT,
                .Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .Aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
                .Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

            for (u32 i = 0; i < swapchainImageCount; i++)
                m_FramebufferDepthAttachments.push_back(std::make_shared<FramebufferAttachment>(info));
        }

        // Recreate the framebuffers.
        {
            m_Framebuffers.reserve(swapchainImageCount);

            FramebufferInfo info
            {
                .RenderContext = &context,
                .RenderPass = m_RenderPass->GetRenderPass(),
            };

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
}
