#include "Event.hpp"

namespace eng
{
    Event::Event(EventType type, EventCategory category) noexcept
        : m_Type(type)
        , m_Categories(category)
    {

    }

    EventType Event::GetType() const noexcept
    {
        return m_Type;
    }

    EventCategory Event::GetCategories() const noexcept
    {
        return m_Categories;
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
