#include "EventThread.hpp"

namespace eng
{
    void EventThread::EnqueueEvent(Event& event)
    {
        std::span<u32> data((u32*)&event, event.GetSize() / sizeof(u32));
        std::scoped_lock lock(m_EventDataMutex);
        m_EventData.append_range(data);
    }

    void EventThread::ProcessEvents(void(*callback)(Event&))
    {
        std::vector<u32> eventData;
        {
            std::scoped_lock lock(m_EventDataMutex);
            eventData = std::move(m_EventData);
        }

        std::span data((u8*)eventData.data(), eventData.size() * sizeof(u32));
        for (u64 i = 0; i < data.size_bytes(); )
        {
            Event& event = *(Event*)&data[i];
            callback(event);
            i += event.GetSize();
        }
    }
}
