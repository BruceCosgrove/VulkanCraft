#include "LayerStack.hpp"
#include "Engine/Core/AssertOrVerify.hpp"

namespace eng::detail
{
    LayerStack::LayerStack(std::span<LayerProducer const> initialLayerProducers, Window& window)
    {
        m_Layers.reserve(initialLayerProducers.size());
        for (auto& layerProducer : initialLayerProducers)
        {
            m_Layers.push_back(layerProducer(window));
            m_OverlayInsertIndex++;
        }
    }

    void LayerStack::PushLayer(LayerProducer const& layerProducer, Window& window)
    {
        PushLayer(layerProducer(window));
    }

    void LayerStack::PushLayer(std::unique_ptr<Layer>&& layer)
    {
        ENG_ASSERT(layer != nullptr, "Tried to add a null layer.");
        ENG_ASSERT(std::find(m_Layers.begin(), m_Layers.end(), layer) == m_Layers.end(), "Tried to add the same layer twice.");

        m_Layers.insert(m_Layers.begin() + m_OverlayInsertIndex++, std::move(layer));
    }

    std::unique_ptr<Layer> LayerStack::PopLayer()
    {
        ENG_ASSERT(0 < m_OverlayInsertIndex, "Tried to pop a layer when none were present.");

        auto it = m_Layers.begin() + --m_OverlayInsertIndex;
        std::unique_ptr<Layer> layer = std::move(*it);
        m_Layers.erase(it);
        return layer;
    }

    void LayerStack::PushOverlay(LayerProducer const& overlayProducer, Window& window)
    {
        PushOverlay(overlayProducer(window));
    }

    void LayerStack::PushOverlay(std::unique_ptr<Layer>&& overlay)
    {
        ENG_ASSERT(overlay != nullptr, "Tried to add a null overlay.");
        ENG_ASSERT(std::find(m_Layers.begin(), m_Layers.end(), overlay) == m_Layers.end(), "Tried to add the same overlay twice.");

        m_Layers.push_back(std::move(overlay));
    }

    std::unique_ptr<Layer> LayerStack::PopOverlay()
    {
        ENG_ASSERT(m_OverlayInsertIndex < m_Layers.size(), "Tried to pop an overlay when none were present.");

        std::unique_ptr<Layer> overlay = std::move(m_Layers.back());
        m_Layers.pop_back();
        return overlay;
    }

    void LayerStack::OnEvent(Event& event)
    {
        for (auto it = m_Layers.rbegin(); it != m_Layers.rend() and !event.IsHandled(); ++it)
            (*it)->OnEvent(event);
    }

    void LayerStack::OnUpdate(Timestep timestep)
    {
        for (auto& layer : m_Layers)
            layer->OnUpdate(timestep);
    }

    void LayerStack::OnRender(Timestep timestep)
    {
        for (auto& layer : m_Layers)
            layer->OnRender(timestep);
    }

    void LayerStack::OnRenderThreadStarted()
    {
        for (auto& layer : m_Layers)
            layer->OnRenderThreadStarted();
    }

    void LayerStack::OnRenderThreadStopped()
    {
        for (auto& layer : m_Layers)
            layer->OnRenderThreadStopped();
    }
}
