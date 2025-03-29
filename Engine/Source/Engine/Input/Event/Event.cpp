#include "Event.hpp"

namespace eng
{
    EventType Event::GetType() const noexcept
    {
        return m_Type;
    }

    EventCategory Event::GetCategories() const noexcept
    {
        return m_Categories;
    }

    u8 Event::GetSize() const noexcept
    {
        return m_Size;
    }

    bool Event::IsHandled() const noexcept
    {
        return m_Handled;
    }

    void Event::Handle() noexcept
    {
        m_Handled = true;
    }

    Event::Event(EventType type, EventCategory category, u8 size) noexcept
        : m_Type(type)
        , m_Categories(category)
        , m_Size(size)
    {

    }
}
