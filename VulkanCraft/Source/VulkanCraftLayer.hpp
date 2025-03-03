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
        // TODO: remove this from eng::Layer and make imgui a client decision.
        virtual void OnImGuiRender() override;
    private:
        void OnWindowCloseEvent(eng::WindowCloseEvent& event);
    private:
        void CreateRenderPass();
        void CreateFramebuffer();
    private:
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
        std::unique_ptr<eng::Shader> m_Shader;
        std::unique_ptr<eng::VertexBuffer> m_VertexBuffer;

        CameraController m_CameraController;
    };
}
