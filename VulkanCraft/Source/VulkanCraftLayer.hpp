#pragma once

#include <Engine.hpp>

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
        virtual void OnImGuiRender() override;
    private:
        void UI_Dockspace();
    };
}
