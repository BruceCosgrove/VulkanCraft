#pragma once

#include "VulkanCraft/Input/CameraController.hpp"
#include "VulkanCraft/UI/ImGuiRenderContext.hpp"
#include "VulkanCraft/UI/ImGuiHelper.hpp"
#include "VulkanCraft/World/World.hpp"
#include "VulkanCraft/World/BlockRegistry.hpp"
#include "VulkanCraft/World/ChunkGenerator.hpp"
#include "VulkanCraft/Rendering/WorldRenderer.hpp"
#include <Engine.hpp>
#include <memory>
#include <vector>

using namespace eng;

namespace vc
{
    class VulkanCraftLayer : public Layer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(VulkanCraftLayer);
    public:
        VulkanCraftLayer(Window& window);

        virtual void OnEvent(Event& event) override;
        virtual void OnUpdate(Timestep timestep) override;
        virtual void OnRender(Timestep timestep) override;
    private:
        void OnWindowCloseEvent(WindowCloseEvent& event);
        void OnKeyPressEvent(KeyPressEvent& event);

        void OnImGuiRender();

        void CreateRenderPass();
        void CreateOrRecreateFramebuffers();
    private:
        // TODO: there really needs to be some kind of allocator/ref system so new isn't
        // called that frequently (also increases cache performance).
        // It would have a section for temp allocations too? like scope-wise temp allocations.
        // long-term storage vs temp storage

        std::shared_ptr<RenderPass> m_RenderPass;
        std::vector<std::shared_ptr<Image>> m_FramebufferDepthAttachments;
        std::vector<std::shared_ptr<Framebuffer>> m_Framebuffers;

        std::unique_ptr<ImGuiRenderContext> m_ImGuiRenderContext;
        std::unique_ptr<ImGuiHelper> m_ImGuiHelper;

        std::unique_ptr<BlockRegistry> m_Blocks;
        std::unique_ptr<World> m_World;
        std::unique_ptr<WorldRenderer> m_WorldRenderer;
        std::unique_ptr<ChunkGenerator> m_ChunkGenerator;
        WorldRenderer::Statistics m_WorldRendererStatistics;

        CameraController m_CameraController;
    };
}
