#pragma once

#include "Engine/Core/Enums.hpp"

namespace eng
{
    ENG_DEFINE_MASKED_ENUM(
        Modifiers, std::uint8_t,

        Shift   = 1 << 0,
        Control = 1 << 1,
        Alt     = 1 << 2,
        Super   = 1 << 3,
    );

    constexpr bool HasModifiers(Modifiers modifiers, Modifiers has) noexcept { return static_cast<bool>(modifiers & has); }
    constexpr bool AreModifiers(Modifiers modifiers, Modifiers are) noexcept { return (modifiers & are) == are; }
}
