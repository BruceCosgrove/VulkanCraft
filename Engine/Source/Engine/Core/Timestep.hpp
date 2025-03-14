#pragma once

#include "Engine/Core/DataTypes.hpp"

namespace eng
{
    struct Timestep
    {
    public:
        Timestep(f32 seconds = 0.0f) noexcept;
        operator f32() const noexcept;

        Timestep  operator+ (Timestep timestep) const noexcept;
        Timestep& operator+=(Timestep timestep) noexcept;
        Timestep  operator- (Timestep timestep) const noexcept;
        Timestep& operator-=(Timestep timestep) noexcept;
        Timestep  operator* (Timestep timestep) const noexcept;
        Timestep& operator*=(Timestep timestep) noexcept;
        Timestep  operator/ (Timestep timestep) const noexcept;
        Timestep& operator/=(Timestep timestep) noexcept;
    private:
        f32 m_Seconds;
    };
}
