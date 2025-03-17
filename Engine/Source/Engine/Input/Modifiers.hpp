#pragma once

#include "Engine/Core/Enums.hpp"

namespace eng
{
    ENG_DEFINE_MASKED_ENUM(
        Modifiers, u8,

        Shift,
        Control,
        Alt,
        Super,
    );
}
