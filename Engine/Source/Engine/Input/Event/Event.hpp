#pragma once

#include "Engine/Core/Enums.hpp"
#include <concepts>

namespace eng
{
    ENG_DEFINE_BOUNDED_ENUM(
        EventType, u8,

        // Window Events
        WindowMove,
        WindowResize,
        WindowClose,
        WindowRefresh,
        WindowFocus,
        WindowMinimize,
        WindowMaximize,
        WindowFramebufferResize,
        WindowContentScale,
        WindowPathDrop,

        // Mouse Events
        MouseButtonPress,
        MouseButtonRelease,
        MouseMove,
        MouseEnter,
        MouseScroll,

        // Key Events
        KeyPress,
        KeyRelease,
        KeyCharType,
    );

    class Event;

    // Interface for all event types to conform to.
    template <class EventT>
    concept EventI =
        std::is_base_of_v<Event, EventT> &&
        requires(EventT& event)
        {
            { EventT::GetStaticType() } noexcept -> std::same_as<EventType>;
        };

    class Event
    {
    public:
        Event(EventType type) noexcept;

        EventType GetType() const noexcept;
        bool IsHandled() const noexcept;
        void Handle() noexcept;
    public:
        template <typename... Args>
        void Dispatch(void(*callback)(Event&, Args...), Args&&... args)
        noexcept(noexcept(callback(*this, std::forward<Args>(args)...)))
        {
            if (!m_Handled)
                callback(*this, std::forward<Args>(args)...);
        }

        template <EventI EventT, typename... Args>
        void Dispatch(void(*callback)(EventT&, Args...), Args&&... args)
        noexcept(noexcept(callback(static_cast<EventT&>(*this), std::forward<Args>(args)...)))
        {
            if (!m_Handled and m_Type == EventT::GetStaticType())
                callback(static_cast<EventT&>(*this), std::forward<Args>(args)...);
        }

        template <class Class, typename... Args>
        void Dispatch(Class* object, void(Class::*callback)(Event&, Args...), Args&&... args)
        noexcept(noexcept((object->*callback)(*this, std::forward<Args>(args)...)))
        {
            if (!m_Handled)
                (object->*callback)(*this, std::forward<Args>(args)...);
        }

        template <class Class, EventI EventT, typename... Args>
        void Dispatch(Class* object, void(Class::*callback)(EventT&, Args...), Args&&... args)
        noexcept(noexcept((object->*callback)(static_cast<EventT&>(*this), std::forward<Args>(args)...)))
        {
            if (!m_Handled and m_Type == EventT::GetStaticType())
                (object->*callback)(static_cast<EventT&>(*this), std::forward<Args>(args)...);
        }
    private:
        EventType m_Type;
        bool m_Handled = false;
    };
}

#define _ENG_EVENT_GET_STATIC_TYPE(type) \
    static ::eng::EventType GetStaticType() noexcept { return type; }

#define _ENG_ASSERT_EVENT_INTERFACE(type) \
    static_assert(::eng::EventI<type>, #type " must satisfy the event interface concept.")
