#pragma once

#include "VulkanCraft/Input/CameraController.hpp"
#include "VulkanCraft/Rendering/TextureAtlas.hpp"
#include "VulkanCraft/UI/ImGuiRenderContext.hpp"
#include "VulkanCraft/UI/ImGuiHelper.hpp"
#include "VulkanCraft/World/BlockRegistry.hpp"
#include "VulkanCraft/World/World.hpp"
#include <Engine.hpp>
#include <atomic>
#include <memory>

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

        std::shared_ptr<RenderPass> CreateRenderPass();

        void CreateOrRecreateFramebuffers();
        std::shared_ptr<Shader> LoadShader();

        void SetDefaultViewportAndScissor();
    private:
        // TODO: there really needs to be some kind of allocator/ref system so new isn't
        // called that frequently (also increases cache performance).
        // It would have a section for temp allocations too? like scope-wise temp allocations.
        // long-term storage vs temp storage

        std::shared_ptr<RenderPass> m_RenderPass;
        std::vector<std::shared_ptr<Image>> m_FramebufferDepthAttachments;
        std::vector<std::shared_ptr<Framebuffer>> m_Framebuffers;

        DynamicResource<std::shared_ptr<Shader>> m_Shader;

        std::shared_ptr<UniformBuffer> m_UniformBuffer;
        std::shared_ptr<StorageBuffer> m_StorageBuffer;
        std::unique_ptr<TextureAtlas> m_BlockTextureAtlas;

        ImGuiRenderContext m_ImGuiRenderContext;
        ImGuiHelper m_ImGuiHelper;

        std::unique_ptr<BlockRegistry> m_BlockRegistry;
        //std::unique_ptr<World> m_World;
        std::shared_ptr<Chunk> m_Chunk;
        CameraController m_CameraController;
    };
}
