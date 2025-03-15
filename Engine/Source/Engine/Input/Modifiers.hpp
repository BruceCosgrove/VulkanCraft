#pragma once

#include "Engine/Core/Enums.hpp"

namespace eng
{
    ENG_DEFINE_MASKED_ENUM(
        Modifiers, u8,

        Shift   = 1 << 0,
        Control = 1 << 1,
        Alt     = 1 << 2,
        Super   = 1 << 3,
    );

    constexpr bool HasAnyModifiers(Modifiers modifiers, Modifiers any) noexcept { return static_cast<bool>(modifiers & any); }
    constexpr bool HasAllModifiers(Modifiers modifiers, Modifiers all) noexcept { return (modifiers & all) == all; }
}
