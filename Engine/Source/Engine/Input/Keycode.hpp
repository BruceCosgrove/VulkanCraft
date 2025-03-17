#pragma once

#include "Engine/Core/Enums.hpp"

namespace eng
{
	// Taken from glfw3.h and modified.

	ENG_DEFINE_BOUNDED_ENUM(
		Keycode, u16,

		/* Printable keys */
		Space,
		Apostrophe,      /* ' */
		Comma,           /* , */
		Minus,           /* - */
		Period,          /* . */
		Slash,           /* / */

		D0,              /* 0 */
		D1,              /* 1 */
		D2,              /* 2 */
		D3,              /* 3 */
		D4,              /* 4 */
		D5,              /* 5 */
		D6,              /* 6 */
		D7,              /* 7 */
		D8,              /* 8 */
		D9,              /* 9 */

		Semicolon,       /* ; */
		Equal,           /* = */

		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		LeftBracket,     /* [ */
		BackSlash,       /* \ */
		RightBracket,    /* ] */
		GraveAccent,     /* ` */
		World1,          /* non-US #1 */
		World2,          /* non-US #2 */

		/* Function keys */
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		Delete,
		Right,
		Left,
		Down,
		Up,
		PageUp,
		PageDown,
		Home,
		End,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		F25,

		/* Keypad */
		KeyPad0,
		KeyPad1,
		KeyPad2,
		KeyPad3,
		KeyPad4,
		KeyPad5,
		KeyPad6,
		KeyPad7,
		KeyPad8,
		KeyPad9,

		KeyPadDecimal,
		KeyPadDivide,
		KeyPadMultiply,
		KeyPadSubtract,
		KeyPadAdd,
		KeyPadEnter,
		KeyPadEqual,

		LeftShift,
		LeftControl,
		LeftAlt,
		LeftSuper,
		RightShift,
		RightControl,
		RightAlt,
		RightSuper,
		Menu,
	);
}
