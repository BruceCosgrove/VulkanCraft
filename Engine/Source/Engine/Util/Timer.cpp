#include "Timer.hpp"
#include "Engine/Core/Log.hpp"

namespace eng
{
    Timer::Timer(small_string_view name)
        : m_Name(name)
        , m_StartTime(std::chrono::high_resolution_clock::now())
    {
        ENG_LOG_INFO("Starting timer \"{}\".", m_Name);
    }

    Timer::~Timer()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        f32 seconds = f32(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_StartTime).count()) * 1e-9f;

        ENG_LOG_INFO("Stopping timer \"{}\" after {}s.", m_Name, seconds);
    }
}
