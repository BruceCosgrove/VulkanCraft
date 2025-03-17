#pragma once

#include "Engine/Core/Enums.hpp"
#include <spdlog/fmt/bundled/format.h>
#include <concepts>
#include <format>

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
        MouseMove,
        MouseEnter,
        MouseScroll,

        // Key Events
        KeyPress,
        KeyType,
    );

    ENG_DEFINE_MASKED_ENUM(
        EventCategory, u8,

        // Window Events
        Window,

        // Mouse Events
        Mouse,

        // Key Events
        Key,
    );

    class Event;

    namespace detail
    {
        // Interface for all event types to conform to.
        template <class EventT>
        concept EventI =
            std::derived_from<EventT, Event> &&
            requires(EventT& event)
            {
                { EventT::GetStaticType() } noexcept -> std::same_as<EventType>;
                { EventT::GetStaticCategories() } noexcept -> std::same_as<EventCategory>;
            };
    }

    class Event
    {
    public:
        Event(EventType type, EventCategory category) noexcept;

        EventType GetType() const noexcept;
        EventCategory GetCategories() const noexcept;
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

        template <detail::EventI EventT, typename... Args>
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

        template <class Class, detail::EventI EventT, typename... Args>
        void Dispatch(Class* object, void(Class::*callback)(EventT&, Args...), Args&&... args)
        noexcept(noexcept((object->*callback)(static_cast<EventT&>(*this), std::forward<Args>(args)...)))
        {
            if (!m_Handled and m_Type == EventT::GetStaticType())
                (object->*callback)(static_cast<EventT&>(*this), std::forward<Args>(args)...);
        }
    private:
        EventType m_Type;
        EventCategory m_Categories;
        bool m_Handled = false;
    };
}

#define _ENG_EVENT_GET_STATIC_FUNCS(type, categories) \
    static ::eng::EventType GetStaticType() noexcept { return type; } \
    static ::eng::EventCategory GetStaticCategories() noexcept { return categories; }

#define _ENG_ASSERT_EVENT_INTERFACE(event) \
    static_assert(::eng::detail::EventI<event>, #event " must satisfy the event interface concept.")

#define _ENG_EVENT_FORMAT(e, format_, ...) \
    template <> struct fmt::formatter<e> { \
        constexpr auto parse(format_parse_context& context) { return context.end(); } \
        auto format(e event, fmt::format_context& context) const { \
            return fmt::format_to(context.out(), format_ __VA_OPT__(,) __VA_ARGS__); \
        } \
    }

_ENG_EVENT_FORMAT(eng::Event,
    "Event(Type={}, Categories={}, Handled={})",
    event.GetType(), event.GetCategories(), event.IsHandled()
);
