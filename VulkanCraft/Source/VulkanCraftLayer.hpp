#pragma once

#include "CameraController.hpp"
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
    private:
        void CreateRenderPass();
        void CreateFramebuffer();
    private:
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

        // TODO: there really needs to be some kind of allocator/ref system so new isn't
        // called that frequently (also increases cache performance).
        // It would have a section for temp allocations too? like scope-wise temp allocations.
        // long-term storage vs temp storage

        std::shared_ptr<eng::VertexBuffer> m_VertexBuffer;
        std::shared_ptr<eng::UniformBuffer> m_UniformBuffer;
        std::shared_ptr<eng::Texture2D> m_Texture;
        std::shared_ptr<eng::Shader> m_Shader;

        CameraController m_CameraController;
    };
}
