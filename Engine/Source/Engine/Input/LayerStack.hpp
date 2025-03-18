#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Layer.hpp"
#include <memory>
#include <span>
#include <vector>

namespace eng
{
    class Window;
    using LayerProducer = std::unique_ptr<Layer>(*)(Window&);
}

namespace eng::detail
{
    class LayerStack
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(LayerStack);
    public:
        LayerStack(std::span<LayerProducer const> initialLayerProducers, Window& window);

        void OnEvent(Event& event);
        void OnUpdate(Timestep timestep);
        void OnRender();

        void PushLayer(LayerProducer const& layerProducer, Window& window);
        void PushLayer(std::unique_ptr<Layer>&& layer);
        std::unique_ptr<Layer> PopLayer();

        void PushOverlay(LayerProducer const& overlayProducer, Window& window);
        void PushOverlay(std::unique_ptr<Layer>&& overlay);
        std::unique_ptr<Layer> PopOverlay();
    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        u64 m_OverlayInsertIndex = 0;
    };
}
