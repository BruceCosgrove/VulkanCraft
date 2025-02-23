#pragma once

#include "Engine/Core/Layer.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Attributes.hpp"
#include "Engine/Core/ClassTypes.hpp"
#include <memory>
#include <vector>

namespace eng
{
    class LayerStack
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_DEFAULTABLE_CLASS(LayerStack);

        void PushLayer(std::unique_ptr<Layer>&& layer)
        {
            ENG_ASSERT(layer != nullptr, "Tried to add a null layer.");
            ENG_ASSERT(std::find(m_Layers.begin(), m_Layers.end(), layer) == m_Layers.end(), "Tried to add the same layer twice.");

            (*m_Layers.emplace(m_Layers.begin() + m_OverlayInsertIndex++, std::move(layer)))->OnAttach();
        }

        std::unique_ptr<Layer> PopLayer()
        {
            ENG_ASSERT(0 < m_OverlayInsertIndex, "Tried to pop a layer when none were present.");

            auto it = m_Layers.begin() + --m_OverlayInsertIndex;
            std::unique_ptr<Layer> layer = std::move(*it);
            m_Layers.erase(it);
            layer->OnDetach();
            return layer;
        }

        void PushOverlay(std::unique_ptr<Layer>&& overlay)
        {
            ENG_ASSERT(overlay, "Tried to add a null overlay.");
            ENG_ASSERT(std::find(m_Layers.begin(), m_Layers.end(), overlay) == m_Layers.end(), "Tried to add the same overlay twice.");

            m_Layers.emplace_back(std::move(overlay))->OnAttach();
        }

        std::unique_ptr<Layer> PopOverlay()
        {
            ENG_ASSERT(m_OverlayInsertIndex < m_Layers.size(), "Tried to pop an overlay when none were present.");

            std::unique_ptr<Layer> overlay = std::move(m_Layers.back());
            m_Layers.pop_back();
            overlay->OnDetach();
            return overlay;
        }
    private:
        friend class Window;

        ~LayerStack()
        {
            for (auto& layer : m_Layers)
                layer->OnDetach();
            m_Layers.clear();
            m_OverlayInsertIndex = 0;
        }

        void OnEvent(Event& event)
        {
            for (auto it = m_Layers.rbegin(); it != m_Layers.rend() and !event.IsHandled(); ++it)
                (*it)->OnEvent(event);
        }

        void OnUpdate(Timestep timestep)
        {
            for (auto& layer : m_Layers)
                layer->OnUpdate(timestep);
        }

        void OnRender()
        {
            for (auto& layer : m_Layers)
                layer->OnRender();
        }

        void OnImGuiRender()
        {
            for (auto& layer : m_Layers)
                layer->OnImGuiRender();
        }
    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        std::size_t m_OverlayInsertIndex = 0;
    };
}
