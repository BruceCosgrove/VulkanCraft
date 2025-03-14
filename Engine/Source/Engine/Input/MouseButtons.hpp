#pragma once

#include "Engine/Core/Enums.hpp"

namespace eng
{
    ENG_DEFINE_BOUNDED_ENUM(
		MouseButton, u8,

		One   = 1, Left     = One,
		Two   = 2, Right    = Two,
		Three = 3, Middle   = Three,
		Four  = 4, Backward = Four,
		Five  = 5, Forward  = Five,
		Six   = 6,
		Seven = 7,
		Eight = 8,
	);
}
