#pragma once

namespace eng
{
    struct Timestep
    {
    public:
        Timestep(float seconds = 0.0f) noexcept;
        operator float() const noexcept;

        Timestep  operator+ (Timestep timestep) const noexcept;
        Timestep& operator+=(Timestep timestep) noexcept;
        Timestep  operator- (Timestep timestep) const noexcept;
        Timestep& operator-=(Timestep timestep) noexcept;
        Timestep  operator* (Timestep timestep) const noexcept;
        Timestep& operator*=(Timestep timestep) noexcept;
        Timestep  operator/ (Timestep timestep) const noexcept;
        Timestep& operator/=(Timestep timestep) noexcept;
    private:
        float m_Seconds;
    };
}
