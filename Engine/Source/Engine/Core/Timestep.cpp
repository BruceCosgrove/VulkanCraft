#include "Timestep.hpp"

namespace eng
{
    Timestep::Timestep(f32 seconds) noexcept
        : m_Seconds(seconds)
    {

    }

    Timestep::operator f32() const noexcept
    {
        return m_Seconds;
    }

    f32 Timestep::Seconds() const noexcept
    {
        return m_Seconds;
    }

    f32 Timestep::Millis() const noexcept
    {
        return m_Seconds * 1'000.0f;
    }

    f32 Timestep::Micros() const noexcept
    {
        return m_Seconds * 1'000'000.0f;
    }

    f32 Timestep::Nanos() const noexcept
    {
        return m_Seconds * 1'000'000'000.0f;
    }

    Timestep Timestep::operator+(Timestep timestep) const noexcept
    {
        return m_Seconds + timestep.m_Seconds;
    }

    Timestep& Timestep::operator+=(Timestep timestep) noexcept
    {
        m_Seconds += timestep.m_Seconds; return *this;
    }

    Timestep Timestep::operator-(Timestep timestep) const noexcept
    {
        return m_Seconds - timestep.m_Seconds;
    }

    Timestep& Timestep::operator-=(Timestep timestep) noexcept
    {
        m_Seconds -= timestep.m_Seconds; return *this;
    }

    Timestep Timestep::operator*(Timestep timestep) const noexcept
    {
        return m_Seconds * timestep.m_Seconds;
    }

    Timestep& Timestep::operator*=(Timestep timestep) noexcept
    {
        m_Seconds *= timestep.m_Seconds; return *this;
    }

    Timestep Timestep::operator/(Timestep timestep) const noexcept
    {
        return m_Seconds / timestep.m_Seconds;
    }

    Timestep& Timestep::operator/=(Timestep timestep) noexcept
    {
        m_Seconds /= timestep.m_Seconds; return *this;
    }
}
