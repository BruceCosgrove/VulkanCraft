#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/Timestep.hpp"
#include "Engine/Input/Event/Event.hpp"

namespace eng
{
    class Window;

    class Layer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Layer);
    public:
        Layer(Window& window);
        virtual ~Layer() = default;

        virtual void OnEvent(Event& event);
        virtual void OnUpdate(Timestep timestep);
        virtual void OnRender();

        Window& GetWindow();
    private:
        Window& m_Window; // non-owning
    };
}
