#pragma once

#include "CameraController.hpp"
#include "TextureAtlas.hpp"
#include <Engine.hpp>
#include <memory>

namespace vc
{
    class VulkanCraftLayer : public eng::Layer
    {
    public:
        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(eng::Event& event) override;
        virtual void OnUpdate(eng::Timestep timestep) override;
        virtual void OnRender() override;
        void OnImGuiRender();
    private:
        void OnWindowCloseEvent(eng::WindowCloseEvent& event);

        void CreateOrRecreateFramebuffers();
    private:
        // TODO: there really needs to be some kind of allocator/ref system so new isn't
        // called that frequently (also increases cache performance).
        // It would have a section for temp allocations too? like scope-wise temp allocations.
        // long-term storage vs temp storage

        std::shared_ptr<eng::RenderPass> m_RenderPass;
        std::vector<std::shared_ptr<eng::Image>> m_FramebufferDepthAttachments;
        std::vector<std::shared_ptr<eng::Framebuffer>> m_Framebuffers;
        std::shared_ptr<eng::VertexBuffer> m_VertexBuffer;
        std::shared_ptr<eng::UniformBuffer> m_UniformBuffer;
        std::shared_ptr<eng::StorageBuffer> m_StorageBuffer;
        std::shared_ptr<eng::Shader> m_Shader;

        CameraController m_CameraController;
        std::unique_ptr<TextureAtlas> m_BlockTextureAtlas;
    };
}
