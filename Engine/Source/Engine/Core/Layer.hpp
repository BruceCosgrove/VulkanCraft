#pragma once

#include "Engine/Core/Timestep.hpp"
#include "Engine/Input/Event/Event.hpp"

namespace eng
{
    class Layer
    {
    public:
        virtual ~Layer() = default;
        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnEvent(Event& event) {}
        virtual void OnUpdate(Timestep timestep) {}
        virtual void OnRender() {}
        virtual void OnImGuiRender() {}
    };
}
