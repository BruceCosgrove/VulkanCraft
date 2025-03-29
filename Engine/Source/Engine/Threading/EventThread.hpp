#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Input/Event/Event.hpp"
#include <mutex>
#include <span>
#include <vector>

namespace eng
{
    class EventThread
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(EventThread);
    public:
        void EnqueueEvent(Event& event);
        void ProcessEvents(void(*callback)(Event&));
    private:
    };
}
