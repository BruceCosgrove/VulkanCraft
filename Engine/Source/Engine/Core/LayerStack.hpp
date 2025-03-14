#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Core/Layer.hpp"
#include <memory>
#include <vector>

namespace eng
{
    class LayerStack
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_DEFAULTABLE_CLASS(LayerStack);

        void PushLayer(std::unique_ptr<Layer>&& layer);
        std::unique_ptr<Layer> PopLayer();

        void PushOverlay(std::unique_ptr<Layer>&& overlay);
        std::unique_ptr<Layer> PopOverlay();
    private:
        friend class Window;

        ~LayerStack();

        void OnEvent(Event& event);
        void OnUpdate(Timestep timestep);
        void OnRender();
    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        u64 m_OverlayInsertIndex = 0;
    };
}
