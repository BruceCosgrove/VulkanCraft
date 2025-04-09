#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include <thread>

namespace eng
{
    class ThreadTracer
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(ThreadTracer);
    public:
        ThreadTracer(small_string_view name);
        ~ThreadTracer();

        std::thread::id GetTID() const;
    private:
        small_string m_Name;
        std::thread::id m_TID;
    };
}
