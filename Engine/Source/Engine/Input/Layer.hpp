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

        // Called on the main thread when an event occurs.
        virtual void OnEvent(Event& event);
        // Called on the update thread once per update.
        virtual void OnUpdate(Timestep timestep);
        // Called on the render thread once per frame.
        virtual void OnRender(Timestep timestep);
        // Called on the render thread right after it starts.
        virtual void OnRenderThreadStarted();
        // Called on the render thread right before it stops.
        virtual void OnRenderThreadStopped();

        Window& GetWindow();
    private:
        Window& m_Window; // non-owning
    };
}
