#include "Timestep.hpp"

namespace eng
{
    Timestep::Timestep(float seconds) noexcept
        : m_Seconds(seconds)
    {

    }

    Timestep::operator float() const noexcept
    {
        return m_Seconds;
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
