#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/Timestep.hpp"
#include "Engine/Input/Event/Event.hpp"

namespace eng
{
    class Window;

    class Layer
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_INHERITABLE_CLASS(Layer);

        virtual void OnAttach();
        virtual void OnDetach();
        virtual void OnEvent(Event& event);
        virtual void OnUpdate(Timestep timestep);
        virtual void OnRender();

        Window& GetWindow();
    private:
        friend class LayerStack;
        Window* m_Window = nullptr; // non-owning
    };
}
