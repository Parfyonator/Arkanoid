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
		win32 mouse

	revision history:
		Jan/07/2000 - Thomas Mannfred Carlsson - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Sep/22/2002 - Timo Saarinen - mouse focus "refinement" ;-)

	TODO:
	- Mouse velocity/axis implemenation
	- Change behaviour to work "correctly" in windowed mode [DONE by TS sep/22/2002] ?
*/

// kludge to enable mouse wheel in win32 api
#define _WIN32_WINNT 0x0500
#include <windows.h>

#include <prcore/configure.hpp>
#include <prcore/mouse.hpp>

using namespace prcore;


// =================================================
// statics
// =================================================

static LRESULT CALLBACK mouseStaticHook(int ncode, WPARAM wparam, LPARAM lparam);

static int mouseCounts[6] = { 0,0,0,0,0,0 };
static int mouseX = 0;
static int mouseY = 0;
static HHOOK mouseStaticHookId = 0;
static long mouseStaticInitCount = 0;


// =================================================
// compatibility
// =================================================

	// these are defined in <winuser.h> in Visual C++ .NET 2002 and 2003,
	// and a good reason to believe that in the future aswell. ;)

	#ifndef WM_XBUTTONUP
	#define WM_XBUTTONUP		0x020C
	#endif

	#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN		0x020B
	#endif

	#ifndef WM_XBUTTONDBLCLK
	#define WM_XBUTTONDBLCLK	0x020D
	#endif

	#ifndef XBUTTON1
	#define XBUTTON1			0x0001
	#endif

	#ifndef XBUTTON2
	#define XBUTTON2			0x0002
	#endif

	#ifndef GET_XBUTTON_WPARAM
	#define GET_XBUTTON_WPARAM(w)	(HIWORD(w))
	#endif

	#ifndef GET_WHEEL_DELTA_WPARAM
	#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))

	typedef struct tagMOUSEHOOKSTRUCTEX {
		POINT     pt; 
		HWND      hwnd; 
		UINT      wHitTestCode; 
		DWORD     dwExtraInfo; 
		DWORD     mouseData;
	} MOUSEHOOKSTRUCTEX;
	#endif

// =================================================
// Mouse
// =================================================

Mouse::Mouse()
{
	mouseStaticInitCount++;

	if ( ( !mouseStaticHookId ) && ( mouseStaticInitCount <= 1 ) )
	{
		mouseStaticHookId = SetWindowsHookEx( WH_MOUSE,
			(HOOKPROC) mouseStaticHook, 0, GetCurrentThreadId() );

		if ( !mouseStaticHookId ) 
			return;
	}
}


Mouse::~Mouse()
{
	if ( mouseStaticInitCount > 0 ) mouseStaticInitCount--;

	if ( ( mouseStaticHookId ) && ( mouseStaticInitCount == 0 ) )
	{
		UnhookWindowsHookEx( mouseStaticHookId );
		mouseStaticHookId = 0;

		ClipCursor( NULL );	// restore when done
	}
}


void Mouse::SetArea(const Rect& rect)
{
	RECT clip = 
	{ 
		rect.x,
		rect.y,
		rect.x+rect.width-1,
		rect.y+rect.height-1
	};
	ClipCursor( &clip );
}


void Mouse::SetXY(int x, int y)
{
	SetCursorPos( x, y );
}


int Mouse::GetX()
{
	return mouseX;
}


int Mouse::GetY()
{
	return mouseY;
}


int Mouse::GetAxisCount() const
{
	return 0;
}


int Mouse::GetButtonCount() const
{
	return 3;
}


float Mouse::GetAxis(int index)
{
	return 0;
}


int Mouse::GetButton(int index)
{
	switch ( index )
	{
		case MOUSE_LEFT:	return mouseCounts[MOUSE_LEFT];
		case MOUSE_RIGHT:	return mouseCounts[MOUSE_RIGHT];
		case MOUSE_MIDDLE:	return mouseCounts[MOUSE_MIDDLE];
		case MOUSE_XBUTTON1:return mouseCounts[MOUSE_XBUTTON1];
		case MOUSE_XBUTTON2:return mouseCounts[MOUSE_XBUTTON2];
		case MOUSE_WHEEL:	return mouseCounts[MOUSE_WHEEL];
		default:			return 0;
	}
}


static LRESULT CALLBACK mouseStaticHook(int ncode, WPARAM wparam, LPARAM lparam)
{
	if ( ncode < 0 )
	{
		return (int)CallNextHookEx(mouseStaticHookId,ncode,wparam,lparam);
	}

	MOUSEHOOKSTRUCTEX& mhs = *(MOUSEHOOKSTRUCTEX*)lparam;
	mouseX = mhs.pt.x;
	mouseY = mhs.pt.y;

	switch ( wparam )
	{
		case WM_LBUTTONDOWN:
			mouseCounts[MOUSE_LEFT] = 1;
			break;
		case WM_LBUTTONUP:
			mouseCounts[MOUSE_LEFT] = 0;
			break;
		case WM_LBUTTONDBLCLK:
			mouseCounts[MOUSE_LEFT] = 2;
			break;


		case WM_MBUTTONDOWN:
			mouseCounts[MOUSE_MIDDLE] = 1;
			break;
		case WM_MBUTTONUP:
			mouseCounts[MOUSE_MIDDLE] = 0;
			break;
		case WM_MBUTTONDBLCLK:
			mouseCounts[MOUSE_MIDDLE] = 2;
			break;

		case WM_RBUTTONDOWN:
			mouseCounts[MOUSE_RIGHT] = 1;
			break;
		case WM_RBUTTONUP:
			mouseCounts[MOUSE_RIGHT] = 0;
			break;
		case WM_RBUTTONDBLCLK:
			mouseCounts[MOUSE_RIGHT] = 2;
			break;

		case WM_XBUTTONDOWN:
			switch ( GET_XBUTTON_WPARAM(mhs.mouseData) )
			{
				case XBUTTON1: mouseCounts[MOUSE_XBUTTON1] = 1; break;
				case XBUTTON2: mouseCounts[MOUSE_XBUTTON2] = 1; break;
				default: break;
			}
			break;
		case WM_XBUTTONUP:
			switch ( GET_XBUTTON_WPARAM(mhs.mouseData) )
			{
				case XBUTTON1: mouseCounts[MOUSE_XBUTTON1] = 0; break;
				case XBUTTON2: mouseCounts[MOUSE_XBUTTON2] = 0; break;
				default: break;
			}
			break;
		case WM_XBUTTONDBLCLK:
			switch ( GET_XBUTTON_WPARAM(mhs.mouseData) )
			{
				case XBUTTON1: mouseCounts[MOUSE_XBUTTON1] = 2; break;
				case XBUTTON2: mouseCounts[MOUSE_XBUTTON2] = 2; break;
				default: break;
			}
			break;

		case WM_MOUSEWHEEL:
			mouseCounts[MOUSE_WHEEL] += GET_WHEEL_DELTA_WPARAM(mhs.mouseData);
			break;

		default:
			break;
	}

	return CallNextHookEx(mouseStaticHookId,ncode,wparam,lparam);
}
