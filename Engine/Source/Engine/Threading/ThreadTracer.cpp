#include "ThreadTracer.hpp"
#include "Engine/Core/Log.hpp"

namespace eng
{
    ThreadTracer::ThreadTracer(small_string_view name)
        : m_Name(name)
        , m_TID(std::this_thread::get_id())
    {
        ENG_LOG_TRACE("Starting {} thread with tid={}.", m_Name, m_TID._Get_underlying_id());
    }

    ThreadTracer::~ThreadTracer()
    {
        ENG_LOG_TRACE("Exiting {} thread with tid={}.", m_Name, m_TID._Get_underlying_id());
    }

    std::thread::id ThreadTracer::GetTID() const
    {
        return m_TID;
    }
}
