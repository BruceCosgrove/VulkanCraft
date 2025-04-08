#pragma once

#include "Engine/Core/DataTypes.hpp"
#include <chrono>
#include <string>

namespace eng
{
    class Timer
    {
    public:
        Timer(small_string_view name);
        ~Timer();
    private:
        small_string m_Name;
        std::chrono::high_resolution_clock::time_point m_StartTime;
    };
}
