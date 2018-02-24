/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_KEYBOARD_HPP
#define PRCORE_KEYBOARD_HPP


#include "inputdevice.hpp"


namespace prcore
{

	enum KeyCode
	{
		KEYCODE_ESC          = 0x01,
		KEYCODE_1            = 0x02,
		KEYCODE_2            = 0x03,
		KEYCODE_3            = 0x04,
		KEYCODE_4            = 0x05,
		KEYCODE_5            = 0x06,
		KEYCODE_6            = 0x07,
		KEYCODE_7            = 0x08,
		KEYCODE_8            = 0x09,
		KEYCODE_9            = 0x0a,
		KEYCODE_0            = 0x0b,
		KEYCODE_A            = 0x10,
		KEYCODE_B            = 0x11,
		KEYCODE_C            = 0x12,
		KEYCODE_D            = 0x13,
		KEYCODE_E            = 0x14,
		KEYCODE_F            = 0x15,
		KEYCODE_G            = 0x16,
		KEYCODE_H            = 0x17,
		KEYCODE_I            = 0x18,
		KEYCODE_J            = 0x19,
		KEYCODE_K            = 0x1a,
		KEYCODE_L            = 0x1b,
		KEYCODE_M            = 0x1c,
		KEYCODE_N            = 0x1d,
		KEYCODE_O            = 0x1e,
		KEYCODE_P            = 0x1f,
		KEYCODE_Q            = 0x20,
		KEYCODE_R            = 0x21,
		KEYCODE_S            = 0x22,
		KEYCODE_T            = 0x23,
		KEYCODE_U            = 0x24,
		KEYCODE_V            = 0x25,
		KEYCODE_W            = 0x26,
		KEYCODE_X            = 0x27,
		KEYCODE_Y            = 0x28,
		KEYCODE_Z            = 0x29,
		KEYCODE_F1           = 0x30,
		KEYCODE_F2           = 0x31,
		KEYCODE_F3           = 0x32,
		KEYCODE_F4           = 0x33,
		KEYCODE_F5           = 0x34,
		KEYCODE_F6           = 0x35,
		KEYCODE_F7           = 0x36,
		KEYCODE_F8           = 0x37,
		KEYCODE_F9           = 0x38,
		KEYCODE_F10          = 0x39,
		KEYCODE_F11          = 0x3a,
		KEYCODE_F12          = 0x3b,
		KEYCODE_NUMPAD0      = 0x40,
		KEYCODE_NUMPAD1      = 0x41,
		KEYCODE_NUMPAD2      = 0x42,
		KEYCODE_NUMPAD3      = 0x43,
		KEYCODE_NUMPAD4      = 0x44,
		KEYCODE_NUMPAD5      = 0x45,
		KEYCODE_NUMPAD6      = 0x46,
		KEYCODE_NUMPAD7      = 0x47,
		KEYCODE_NUMPAD8      = 0x48,
		KEYCODE_NUMPAD9      = 0x49,
		KEYCODE_NUMLOCK      = 0x4a,
		KEYCODE_DIVIDE       = 0x4b,
		KEYCODE_MULTIPLY     = 0x4c,
		KEYCODE_SUBTRACT     = 0x4d,
		KEYCODE_ADDITION     = 0x4e,
		KEYCODE_ENTER        = 0x4f,
		KEYCODE_DECIMAL      = 0x50,
		KEYCODE_BACKSPACE    = 0x60,
		KEYCODE_TAB          = 0x61,
		KEYCODE_RETURN       = 0x62,
		KEYCODE_LEFT_CTRL    = 0x63,
		KEYCODE_RIGHT_CTRL   = 0x64,
		KEYCODE_LEFT_SHIFT   = 0x65,
		KEYCODE_RIGHT_SHIFT  = 0x66,
		KEYCODE_LEFT_ALT     = 0x67,
		KEYCODE_RIGHT_ALT    = 0x68,
		KEYCODE_SPACE        = 0x69,
		KEYCODE_CAPSLOCK     = 0x6a,
		KEYCODE_PRINT_SCREEN = 0x6b,
		KEYCODE_SCROLL_LOCK  = 0x6c,
		KEYCODE_PAGEUP       = 0x6d,
		KEYCODE_PAGEDOWN     = 0x6e,
		KEYCODE_INSERT       = 0x6f,
		KEYCODE_DELETE       = 0x70,
		KEYCODE_HOME         = 0x71,
		KEYCODE_END          = 0x72,
		KEYCODE_LEFT         = 0x73,
		KEYCODE_RIGHT        = 0x74,
		KEYCODE_UP			 = 0x75,
		KEYCODE_DOWN         = 0x76
	};

	class Keyboard : public InputBase
	{
		public:

		Keyboard();
		~Keyboard();

		int		GetButtonCount() const;
		int		GetButton(int keycode);
	};

} // namespace prcore


#endif