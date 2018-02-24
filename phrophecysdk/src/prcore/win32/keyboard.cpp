/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


	source:
		win32 keyboard

	revision history:
		???/??/1997 - Thomas Mannfred Carlsson - initial revision
		Feb/06/2001 - Jukka Liimatta - renaissance build
		Nov/24/2001 - Francesco Montecuccoli - keyboard hook calls next handler before returning
*/
#include <prcore/configure.hpp>
#include <prcore/keyboard.hpp>

using namespace prcore;


// =================================================
// statics
// =================================================

static LRESULT CALLBACK kbStaticHook(int ncode, WPARAM wparam, LPARAM lparam);
static LRESULT CALLBACK sysStaticHook(int ncode, WPARAM wparam, LPARAM lparam);

static char		kbuffer[256];
static BYTE		pbuffer[256];
static char		kcharbuf[256];
static int		kcharsize = 0;

static HHOOK	kbStaticHookId = 0;
static HHOOK	sysStaticHookId = 0;
static long		kbStaticInitCount = 0;
static uint32	pkbfunc = NULL;

static char remap[] =
{
	0x00,	//
	0x01,	// KEYCODE_ESC
	0x02,	// KEYCODE_1
	0x03,	// KEYCODE_2
	0x04,	// KEYCODE_3
	0x05,	// KEYCODE_4
	0x06,	// KEYCODE_5
	0x07,	// KEYCODE_6
	0x08,	// KEYCODE_7
	0x09,	// KEYCODE_8
	0x0a,	// KEYCODE_9
	0x0b,	// KEYCODE_0
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x1e,	// KEYCODE_A
	0x30,	// KEYCODE_B
	0x2e,	// KEYCODE_C
	0x20,	// KEYCODE_D
	0x12,	// KEYCODE_E
	0x21,	// KEYCODE_F
	0x22,	// KEYCODE_G
	0x23,	// KEYCODE_H
	0x17,	// KEYCODE_I
	0x24,	// KEYCODE_J
	0x25,	// KEYCODE_K
	0x26,	// KEYCODE_L
	0x32,	// KEYCODE_M
	0x31,	// KEYCODE_N
	0x18,	// KEYCODE_O
	0x19,	// KEYCODE_P
	0x10,	// KEYCODE_Q
	0x13,	// KEYCODE_R
	0x1f,	// KEYCODE_S
	0x14,	// KEYCODE_T
	0x16,	// KEYCODE_U
	0x2f,	// KEYCODE_V
	0x11,	// KEYCODE_W
	0x2d,	// KEYCODE_X
	0x15,	// KEYCODE_Y
	0x2c,	// KEYCODE_Z
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x3b,	// KEYCODE_F1
	0x3c,	// KEYCODE_F2
	0x3d,	// KEYCODE_F3
	0x3e,	// KEYCODE_F4
	0x3f,	// KEYCODE_F5
	0x40,	// KEYCODE_F6
	0x41,	// KEYCODE_F7
	0x42,	// KEYCODE_F8
	0x43,	// KEYCODE_F9
	0x44,	// KEYCODE_F10
	0x57,	// KEYCODE_F11
	0x58,	// KEYCODE_F12
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x52,	// KEYCODE_NUMPAD0
	0x4f,	// KEYCODE_NUMPAD1
	0x50,	// KEYCODE_NUMPAD2
	0x51,	// KEYCODE_NUMPAD3
	0x4b,	// KEYCODE_NUMPAD4
	0x4c,	// KEYCODE_NUMPAD5
	0x4d,	// KEYCODE_NUMPAD6
	0x47,	// KEYCODE_NUMPAD7
	0x48,	// KEYCODE_NUMPAD8
	0x49,	// KEYCODE_NUMPAD9
	0x45,	// KEYCODE_NUMLOCK
	0x35,	// KEYCODE_DIVIDE
	0x37,	// KEYCODE_MULTIPLY
	0x4a,	// KEYCODE_SUBTRACT
	0x4e,	// KEYCODE_ADDITION
	0x1c,	// KEYCODE_ENTER
	0x53,	// KEYCODE_DECIMAL
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x0e,	// KEYCODE_BACKSPACE
	0x0f,	// KEYCODE_TAB
	0x1c,	// KEYCODE_RETURN
	0x1d,	// KEYCODE_L_CTRL
	0x5a,	// KEYCODE_R_CTRL
	0x2a,	// KEYCODE_L_SHIFT
	0x36,	// KEYCODE_R_SHIFT
	0x38,	// KEYCODE_L_ALT
	0x5c,	// KEYCODE_R_ALT
	0x39,	// KEYCODE_SPACE
	0x3a,	// KEYCODE_CAPSLOCK
	0x47,	// KEYCODE_PRINT_SCREEN
	0x57,	// KEYCODE_SCROLL_LOCK
	0x49,	// KEYCODE_PAGEUP
	0x51,	// KEYCODE_PAGEDOWN
	0x52,	// KEYCODE_INSERT
	0x53,	// KEYCODE_DELETE
	0x47,	// KEYCODE_HOME
	0x4f,	// KEYCODE_END
	0x4b,	// KEYCODE_LEFT
	0x4d,	// KEYCODE_RIGHT
	0x48,	// KEYCODE_UP
	0x50,	// KEYCODE_DOWN
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
	0x00,	//
};


// =================================================
// Keyboard
// =================================================

Keyboard::Keyboard()
{
	++kbStaticInitCount;

	if ( ( !kbStaticHookId ) && ( kbStaticInitCount <= 1 ) )
	{
		pkbfunc = 0;

		memset(kbuffer,0,sizeof(kbuffer));
		if ( !GetKeyboardState((PBYTE)pbuffer) )
			memset(pbuffer,0,sizeof(pbuffer));

		kbStaticHookId = SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)kbStaticHook,0,GetCurrentThreadId());
		if ( !kbStaticHookId ) 
		{ 
			return; 
		}

		sysStaticHookId = SetWindowsHookEx(WH_CBT,(HOOKPROC)sysStaticHook,0,GetCurrentThreadId());
		if ( !sysStaticHookId ) 
		{ 
			return; 
		}
	}
}


Keyboard::~Keyboard()
{
	if ( kbStaticInitCount > 0 )
	{
		--kbStaticInitCount;
	}

	if ( ( kbStaticHookId ) && ( kbStaticInitCount == 0 ) )
	{
		UnhookWindowsHookEx(kbStaticHookId);
		kbStaticHookId = 0;

		if ( sysStaticHookId )
		{
			UnhookWindowsHookEx(sysStaticHookId);
			sysStaticHookId = 0;
		}

		pkbfunc = 0;
	}
}


int Keyboard::GetButtonCount() const
{
	return 128;
}


int Keyboard::GetButton(int code)
{
	return kbuffer[remap[code]];
}


static uint16 tkbuf[4] = {0,0,0,0};


static LRESULT CALLBACK kbStaticHook(int ncode, WPARAM wparam, LPARAM lparam)
{
	if ( ncode < 0 )
	{
		return (int)CallNextHookEx(kbStaticHookId,ncode,wparam,lparam);
	}

	if ( lparam & 0x80000000 )
	{
		pbuffer[wparam & 0xff] &= ~0x80;
		kbuffer[(char)((lparam >> 16) & 127)] = 0;

		if ( pkbfunc )
		{
			int ascres = ToAscii((UINT)wparam,((UINT)lparam >> 16) & 0x7f,(PBYTE)pbuffer,(LPWORD)tkbuf,0);
			if ( ascres > 0 )
			{
				uint16 ascii;
				if ( ascres == 2 ) 	ascii = tkbuf[1];
				else				ascii = tkbuf[0];
			}
			else
			{
			}
		}

		return (int)CallNextHookEx(kbStaticHookId,ncode,wparam,lparam);
	}
	else
	{
		BYTE& sample = pbuffer[wparam & 0xff];
		sample ^= 0x01;
		sample |= 0x80;
		
		kbuffer[(char)((lparam >> 16) & 127)] = 1;

		if ( pkbfunc )
		{
			int ascres = ToAscii((UINT)wparam,((UINT)lparam >> 16) & 0x7f,(PBYTE)pbuffer,(LPWORD)tkbuf,0);
			if ( ascres > 0 )
			{
				uint16 ascii;
				if ( ascres == 2 ) 	ascii = tkbuf[1];
				else				ascii = tkbuf[0];
			}
			else
			{
			}
		}
		return (int)CallNextHookEx(kbStaticHookId,ncode,wparam,lparam);
	}

	// if we passed over control, these should be filtered out..
	if ( ( WORD(lparam >> 16 ) & KF_ALTDOWN ) )
	{
		return (int)CallNextHookEx(kbStaticHookId,ncode,wparam,lparam);
	}

	if ( GetAsyncKeyState(VK_CONTROL) & 0x80000000 )
	{
		return (int)CallNextHookEx(kbStaticHookId,ncode,wparam,lparam);
	}
}


static LRESULT CALLBACK sysStaticHook(int ncode, WPARAM wparam, LPARAM lparam)
{
	return (int)CallNextHookEx(sysStaticHookId,ncode,wparam,lparam);
}
