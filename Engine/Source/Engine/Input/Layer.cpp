#include "Layer.hpp"

namespace eng
{
    Layer::Layer(Window& window)
        : m_Window(window)
    {

    }

    void Layer::OnEvent(Event& event)
    {

    }

    void Layer::OnUpdate(Timestep timestep)
    {

    }

    void Layer::OnRender(Timestep timestep)
    {

    }

    Window& Layer::GetWindow()
    {
        return m_Window;
    }
}
