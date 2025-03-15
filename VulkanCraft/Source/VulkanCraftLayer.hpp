#pragma once

#include "CameraController.hpp"
#include "TextureAtlas.hpp"
#include <Engine.hpp>
#include <memory>

using namespace eng;

namespace vc
{
    class VulkanCraftLayer : public Layer
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_DEFAULTABLE_CLASS(VulkanCraftLayer);

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& event) override;
        virtual void OnUpdate(Timestep timestep) override;
        virtual void OnRender() override;
        void OnImGuiRender();
    private:
        void OnWindowCloseEvent(WindowCloseEvent& event);
        void OnKeyPressEvent(KeyPressEvent& event);

        void CreateOrRecreateFramebuffers();
        void LoadShader();
    private:
        // TODO: there really needs to be some kind of allocator/ref system so new isn't
        // called that frequently (also increases cache performance).
        // It would have a section for temp allocations too? like scope-wise temp allocations.
        // long-term storage vs temp storage

        std::shared_ptr<RenderPass> m_RenderPass;
        std::vector<std::shared_ptr<Image>> m_FramebufferDepthAttachments;
        std::vector<std::shared_ptr<Framebuffer>> m_Framebuffers;

        std::unique_ptr<ImGuiRenderContext> m_ImGuiRenderContext;

        std::shared_ptr<VertexBuffer> m_VertexBuffer;
        std::shared_ptr<UniformBuffer> m_UniformBuffer;
        std::shared_ptr<StorageBuffer> m_StorageBuffer;
        std::shared_ptr<Shader> m_Shader;

        CameraController m_CameraController;
        std::unique_ptr<TextureAtlas> m_BlockTextureAtlas;

        bool m_ReloadShader = false;
    };
}
