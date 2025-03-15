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
    private:
        friend class Window;

        LayerStack(std::vector<std::unique_ptr<Layer>(*)()> const& initialLayerProducers, Window* window);
        ~LayerStack();

        void OnEvent(Event& event);
        void OnUpdate(Timestep timestep);
        void OnRender();

        void PushLayer(std::unique_ptr<Layer>&& layer, Window* window);
        std::unique_ptr<Layer> PopLayer();

        void PushOverlay(std::unique_ptr<Layer>&& overlay, Window* window);
        std::unique_ptr<Layer> PopOverlay();
    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        u64 m_OverlayInsertIndex = 0;
    };
}
