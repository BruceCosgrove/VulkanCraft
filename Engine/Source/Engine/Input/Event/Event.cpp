#include "Event.hpp"

namespace eng
{
    Event::Event(EventType type) noexcept
        : m_Type(type)
    {

    }

    EventType Event::GetType() const noexcept
    {
        return m_Type;
    }

    bool Event::IsHandled() const noexcept
    {
        return m_Handled;
    }

    void Event::Handle() noexcept
    {
        m_Handled = true;
    }
}
