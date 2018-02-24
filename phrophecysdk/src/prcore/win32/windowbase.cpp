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
		WindowBase Windows implementation

	revision history:
		May/23/2000 - Jukka Liimatta - initial revision
		Feb/04/2001 - Jukka Liimatta - renaissance build
		Oct/17/2001 - Jukka Liimatta - added drag'n'drop support
		Nov/24/2001 - Francesco Montecuccoli - added charcode to ::EventKeyboard()
		Jul/30/2002 - Thomas Mannfred Carlsson - added mouse support (only mouse downs registered, as they're the only reliable events)
		Jul/30/2002 - Mikko Nurmi - added window style WINDOW_ALWAYSONTOP
		Aug/16/2002 - Jukka Liimatta & Henri Tuhkanen - added frame to GetRect() method
		Oct/29/2002 - Stefan/DXM - fixed cursor hiding bug (you don't want to know what DXM means)
		Dec/14/2002 - Jukka Liimatta - added ::BindWindow() and ::UnbindWindow() methods
		                               added ::IsEventEnable()
		                               fixed ::GetWindowStyle()
									   fixed RegisterWindowClass() and OpenWindow()
		Jan/22/2003 - Jukka Liimatta - added ::SetMainFrequency() and ::GetMainFrequency()
		Apr/13/2003 - Jukka Liimatta - fixed ::MainLoop() to setup "mainbreak" to false before message loop
		Apr/24/2003 - Jukka Liimatta - added support for mousewheel, xbuttons, double-clicking
		Jun/26/2003 - Jukka Liimatta - fixed GET_XBUTTON_WPARAM macro
		Jun/30/2003 - Franceso Montecuccoli - fixed GET_WHEEL_DELTA_WPARAM macro
*/

// kludge to enable mouse wheel in win32 api
#define _WIN32_WINNT 0x0500
#include <windows.h>

#include <cstddef>
#include <cassert>
#include <prcore/timer.hpp>
#include <prcore/keyboard.hpp>
#include <prcore/mouse.hpp>
#include <prcore/windowbase.hpp>
#include <prcore/exception.hpp>

// =================================================
// compatibility
// =================================================

// these are defined in <winuser.h> in Visual C++ .NET 2002 and 2003,
// and a good reason to believe that in the future aswell. ;)

#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP				0x020C
#endif

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN				0x020B
#endif

#ifndef WM_XBUTTONDBLCLK
#define WM_XBUTTONDBLCLK			0x020D
#endif

#ifndef XBUTTON1
#define XBUTTON1					0x0001
#endif

#ifndef XBUTTON2
#define XBUTTON2					0x0002
#endif

#ifndef GET_XBUTTON_WPARAM
#define GET_XBUTTON_WPARAM(w)		(HIWORD(w))
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(w)	((short)HIWORD(w)) 
#endif

namespace prcore
{

	// =================================================
	// WIN32 API
	// =================================================

	bool gb_destroy_window = true;
	bool gb_call_close     = true;
	bool gb_mainbreak      = true;

	BYTE pbuffer[256];
	uint16 tkbuf[4] = { 0,0,0,0 };

	int remap(int code)
	{
		switch ( code )
		{
			case VK_ESCAPE:		return KEYCODE_ESC;
			case '0':			return KEYCODE_0;
			case '1':			return KEYCODE_1;
			case '2':			return KEYCODE_2;
			case '3':			return KEYCODE_3;
			case '4':			return KEYCODE_4;
			case '5':			return KEYCODE_5;
			case '6':			return KEYCODE_6;
			case '7':			return KEYCODE_7;
			case '8':			return KEYCODE_8;
			case '9':			return KEYCODE_9;
			case 'A':			return KEYCODE_A;
			case 'B':			return KEYCODE_B;
			case 'C':			return KEYCODE_C;
			case 'D':			return KEYCODE_D;
			case 'E':			return KEYCODE_E;
			case 'F':			return KEYCODE_F;
			case 'G':			return KEYCODE_G;
			case 'H':			return KEYCODE_H;
			case 'I':			return KEYCODE_I;
			case 'J':			return KEYCODE_J;
			case 'K':			return KEYCODE_K;
			case 'L':			return KEYCODE_L;
			case 'M':			return KEYCODE_M;
			case 'N':			return KEYCODE_N;
			case 'O':			return KEYCODE_O;
			case 'P':			return KEYCODE_P;
			case 'Q':			return KEYCODE_Q;
			case 'R':			return KEYCODE_R;
			case 'S':			return KEYCODE_S;
			case 'T':			return KEYCODE_T;
			case 'U':			return KEYCODE_U;
			case 'V':			return KEYCODE_V;
			case 'W':			return KEYCODE_W;
			case 'X':			return KEYCODE_X;
			case 'Y':			return KEYCODE_Y;
			case 'Z':			return KEYCODE_Z;
			case VK_F1:			return KEYCODE_F1;
			case VK_F2:			return KEYCODE_F2;
			case VK_F3:			return KEYCODE_F3;
			case VK_F4:			return KEYCODE_F4;
			case VK_F5:			return KEYCODE_F5;
			case VK_F6:			return KEYCODE_F6;
			case VK_F7:			return KEYCODE_F7;
			case VK_F8:			return KEYCODE_F8;
			case VK_F9:			return KEYCODE_F9;
			case VK_F10:		return KEYCODE_F10;
			case VK_F11:		return KEYCODE_F11;
			case VK_F12:		return KEYCODE_F12;
			case VK_NUMPAD0:	return KEYCODE_NUMPAD0;
			case VK_NUMPAD1:	return KEYCODE_NUMPAD1;
			case VK_NUMPAD2:	return KEYCODE_NUMPAD2;
			case VK_NUMPAD3:	return KEYCODE_NUMPAD3;
			case VK_NUMPAD4:	return KEYCODE_NUMPAD4;
			case VK_NUMPAD5:	return KEYCODE_NUMPAD5;
			case VK_NUMPAD6:	return KEYCODE_NUMPAD6;
			case VK_NUMPAD7:	return KEYCODE_NUMPAD7;
			case VK_NUMPAD8:	return KEYCODE_NUMPAD8;
			case VK_NUMPAD9:	return KEYCODE_NUMPAD9;
			case VK_NUMLOCK:	return KEYCODE_NUMLOCK;
			case VK_DIVIDE:		return KEYCODE_DIVIDE;
			case VK_MULTIPLY:	return KEYCODE_MULTIPLY;
			case VK_SUBTRACT:	return KEYCODE_SUBTRACT;
			case VK_ADD:		return KEYCODE_ADDITION;
			case VK_DECIMAL:	return KEYCODE_DECIMAL;
			case VK_BACK:		return KEYCODE_BACKSPACE;
			case VK_TAB:		return KEYCODE_TAB;
			case VK_RETURN:		return KEYCODE_RETURN;
			case VK_LCONTROL:	return KEYCODE_LEFT_CTRL;
			case VK_RCONTROL:	return KEYCODE_RIGHT_CTRL;
			case VK_LSHIFT:		return KEYCODE_LEFT_SHIFT;
			case VK_RSHIFT:		return KEYCODE_RIGHT_SHIFT;
			case VK_SPACE:		return KEYCODE_SPACE;
			case VK_PRINT:		return KEYCODE_PRINT_SCREEN;
			case VK_SCROLL:		return KEYCODE_SCROLL_LOCK;
			case VK_PRIOR:		return KEYCODE_PAGEUP;
			case VK_NEXT:		return KEYCODE_PAGEDOWN;
			case VK_INSERT:		return KEYCODE_INSERT;
			case VK_DELETE:		return KEYCODE_DELETE;
			case VK_HOME:		return KEYCODE_HOME;
			case VK_END:		return KEYCODE_END;
			case VK_LEFT:		return KEYCODE_LEFT;
			case VK_RIGHT:		return KEYCODE_RIGHT;
			case VK_UP:			return KEYCODE_UP;
			case VK_DOWN:		return KEYCODE_DOWN;
			default:			return 0;
		}

		// the following cannot be handled by the EventKeyboard()
		// ----------------------------------------------------------------
		// KEYCODE_ENTER;
		// KEYCODE_LEFT_ALT;
		// KEYCODE_RIGHT_ALT;
		// KEYCODE_CAPSLOCK;
	}

	// window callback type
	#ifdef STRICT
	typedef WNDPROC WindowCB;
	#else
	typedef FARPROC WindowCB;
	#endif

	// window callback
	LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wparam, LPARAM lparam)
	{
		// get window object
		WindowBase* window = reinterpret_cast<WindowBase*>(GetWindowLong(hwnd,GWL_USERDATA));

		// chain window proc
		if ( window )
		{
			WindowCB prevproc = reinterpret_cast<WindowCB>(window->GetPrevProc());
			if ( prevproc )
			{
				//TODO: enable windowproc chaining..
				//CallWindowProc(prevproc,hwnd,imsg,wparam,lparam);
			}
		}

		switch ( imsg )
		{
			//case WM_CREATE:
				//break;

			case WM_TIMER:
				break;

			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				int v = remap(static_cast<int>(wparam));

				GetKeyboardState((PBYTE)pbuffer);
				int ascres = ToAscii(static_cast<UINT>(wparam),(static_cast<UINT>(lparam) >> 16) & 0x07f,
					(PBYTE)pbuffer,(LPWORD)tkbuf,0);

				char ascii;
				switch ( ascres )
				{
					case 1:  ascii = static_cast<char>(tkbuf[0] & 0xff); break;
					case 2:  ascii = static_cast<char>(tkbuf[1] & 0xff); break;
					default: ascii = 0; break;
				}

				if ( window && window->IsEventEnable() )
					window->DispatchEventKeyboard(v,ascii,imsg == WM_KEYDOWN);

				return 0;
			}

			case WM_PAINT:
			{
				PAINTSTRUCT paint;
				BeginPaint(hwnd,&paint);

				if ( window && window->IsEventEnable() )
					window->EventDraw();

				EndPaint(hwnd,&paint);
				ValidateRect(hwnd,NULL);

				return 0;
			}

			case WM_MOUSEMOVE:
			{
				if ( window && window->IsEventEnable() )
				{
					int x = LOWORD(lparam);
					int y = HIWORD(lparam);
					window->EventMouseMove(x,y);
				}
				return 0;
			}

			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			case WM_XBUTTONUP:
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_XBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
			case WM_MOUSEWHEEL:
			{
				if ( window && window->IsEventEnable() )
				{
					MouseButton button;
	            	int count;

	            	switch ( imsg )
	            	{
						case WM_LBUTTONUP:
							button = MOUSE_LEFT;
							count = 0;
							break;

						case WM_MBUTTONUP:
							button = MOUSE_MIDDLE;
							count = 0;
							break;

						case WM_RBUTTONUP:
							button = MOUSE_RIGHT;
							count = 0;
							break;

						case WM_XBUTTONUP:
							switch ( GET_XBUTTON_WPARAM(wparam) )
							{
								case XBUTTON1: button = MOUSE_XBUTTON1; break;
								case XBUTTON2: button = MOUSE_XBUTTON2; break;
								default: return 0;
							}
							count = 0;
							break;

						case WM_LBUTTONDOWN:
							button = MOUSE_LEFT;
							count = 1;
							break;

						case WM_MBUTTONDOWN:
							button = MOUSE_MIDDLE;
							count = 1;
							break;

						case WM_RBUTTONDOWN:
							button = MOUSE_RIGHT;
							count = 1;
							break;

						case WM_XBUTTONDOWN:
							switch ( GET_XBUTTON_WPARAM(wparam) )
							{
								case XBUTTON1: button = MOUSE_XBUTTON1; break;
								case XBUTTON2: button = MOUSE_XBUTTON2; break;
								default: return 0;
							}
							count = 1;
							break;

						case WM_LBUTTONDBLCLK:
							button = MOUSE_LEFT;
							count = 2;
							break;

						case WM_MBUTTONDBLCLK:
							button = MOUSE_MIDDLE;
							count = 2;
							break;

						case WM_RBUTTONDBLCLK:
							button = MOUSE_RIGHT;
							count = 2;
							break;

						case WM_XBUTTONDBLCLK:
							switch ( GET_XBUTTON_WPARAM(wparam) )
							{
								case XBUTTON1: button = MOUSE_XBUTTON1; break;
								case XBUTTON2: button = MOUSE_XBUTTON2; break;
								default: return 0;
							}
							count = 2;
							break;

						case WM_MOUSEWHEEL:
							button = MOUSE_WHEEL;
							count = GET_WHEEL_DELTA_WPARAM(wparam);
							break;

						default:
							return 0;
	            	}

					int x = LOWORD(lparam);
					int y = HIWORD(lparam);
					window->EventMouseButton(x,y,button,count);
				}
				return 0;
			}

			case WM_SIZE:
			{
				if ( window && window->IsEventEnable() )
				{
					int width  = LOWORD(lparam);
					int height = HIWORD(lparam);
					window->EventSize(width,height);
				}
				return 0;
			}

			case WM_ACTIVATE:
			{
				if ( window && window->IsEventEnable() )
				{
					bool enter = LOWORD(wparam) != WA_INACTIVE;
					window->EventFocus(enter);
				}
				return 0;
			}
			
			case WM_DROPFILES:
			{
				char filename[512];
				if ( DragQueryFile((HDROP)wparam,0,filename,sizeof(filename)) > 0 )
				{
					if ( window && window->IsEventEnable() )
						window->EventDropFile(filename);
				}
				return 0;
			}

			case WM_DESTROY:
			{
				if ( window )
				{
					if ( gb_call_close )
					{
						gb_destroy_window = false;
						window->CloseWindow();
						gb_destroy_window = true;
					}

					if ( gb_mainbreak )
					{
						window->MainBreak();
					}
				}

				::SetWindowLong(hwnd,GWL_USERDATA,(LONG)NULL);
				return 0;
			}
		}

		return DefWindowProc(hwnd,imsg,wparam,lparam);
	}

	String RegisterWindowClass(HINSTANCE hInstance, bool close)
	{

		// NOTE:
		// - in practise we only require two registered classes
		//   1. one with the close button
		//   2. one without the close button
		//   .. and register them only once per application

		WNDCLASSEX wndclass;
		::ZeroMemory(&wndclass,sizeof(wndclass));

		// generate unique class name
		static int index = 0;
		String classname = "twilight3d" + index;
		++index;

		// fill wndclass
		wndclass.cbSize        = sizeof(wndclass);
		wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wndclass.lpfnWndProc   = WndProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInstance;
		wndclass.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
		wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = classname;
		wndclass.hIconSm       = LoadIcon(NULL,IDI_APPLICATION);

		if ( close == false )
			wndclass.style |= CS_NOCLOSE;

		::RegisterClassEx(&wndclass);

		return classname;
	}

	void AdjustSize(uint32 mask, int& width, int& height)
	{
		RECT r;

		r.left 	 = 0;
		r.top 	 = 0;
		r.right  = width - 1;
		r.bottom = height - 1;

		AdjustWindowRect(&r,mask,FALSE);
	
		width  = r.right - r.left + 1;
		height = r.bottom - r.top + 1;
	}

// =================================================
// WindowBase
// =================================================

WindowBase::WindowBase()
: mIsOpen(false),mIsBind(false),mIsEventEnable(false),
  mpPrevProc(NULL),mMainFreq(0)
{
	mWindowParent.hwnd = NULL;
	mWindowHandle.hwnd = NULL;

	mMainBreak = false;

	for ( int i=0; i<256; ++i )
		mKeyDown[i] = false;
}


WindowBase::WindowBase(WindowHandle parent)
: mIsOpen(false),mIsBind(false),mIsEventEnable(false),
  mpPrevProc(NULL),mMainFreq(0)
{
	mWindowParent.hwnd = parent.hwnd;
	mWindowHandle.hwnd = NULL;

	mMainBreak = false;

	for ( int i=0; i<256; ++i )
		mKeyDown[i] = false;
}

WindowBase::~WindowBase()
{
	CloseWindow();
	UnbindWindow();
}

bool WindowBase::BindWindow(WindowHandle handle)
{
	if ( IsWindowBind() )
		UnbindWindow();

	if ( IsWindowOpen() )
		CloseWindow();

	// get window handle
	HWND hwnd = handle.hwnd;
	if ( !hwnd )
		return false;

	// get parent window handle
	HWND parent = ::GetParent(hwnd);

	// set current window members
	mWindowHandle.hwnd = hwnd;
	mWindowParent.hwnd = parent;
	mWindowStyle = 0;

	// store previous wndproc
	mpPrevProc = reinterpret_cast<void*>(::GetWindowLong(hwnd,GWL_WNDPROC));

	// connect the hwnd to winproc chain
	::SetWindowLong(hwnd,GWL_WNDPROC,(LONG)WndProc);

	// set win32 hwnd userdata to point to the window object
	::SetWindowLong(hwnd,GWL_USERDATA,(LONG)this);
	::UpdateWindow(hwnd);

	// set bind
	mIsBind = true;

	return true;
}

bool WindowBase::UnbindWindow()
{
	if ( mIsBind )
	{
		// restore previous wndproc
		HWND hwnd = mWindowHandle.hwnd;
		::SetWindowLong(hwnd,GWL_WNDPROC,(LONG)mpPrevProc);
		mpPrevProc = NULL;

		MainBreak();
		mWindowHandle.hwnd = NULL;

		return true;
	}
	return false;
}

bool WindowBase::OpenWindow(int width, int height, const String& name, uint32 windowstyle, bool show)
{
	if ( IsWindowOpen() )
		CloseWindow();

	if ( IsWindowBind() )
		UnbindWindow();

	//TODO: move
	uint32 mask = (windowstyle & WINDOW_FRAME) ? WS_OVERLAPPEDWINDOW : WS_POPUP;
	if ( !(windowstyle & WINDOW_SIZE) )		mask &= ~WS_SIZEBOX;
	if ( !(windowstyle & WINDOW_MINIMIZE) )	mask &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	if ( !(windowstyle & WINDOW_TITLE) )	mask &= ~WS_CAPTION;

	// register window class
	HINSTANCE hInstance = NULL;
	String classname = RegisterWindowClass(hInstance,(windowstyle & WINDOW_CLOSE) != 0);

	// adjust rectangle
	AdjustSize(mask,width,height);

	// create window
	if ( windowstyle & WINDOW_ALWAYSONTOP )
	{
		const char* text = name;
		mWindowHandle.hwnd = ::CreateWindowEx(WS_EX_TOPMOST,classname,
			text,mask,0,0,width,height,mWindowParent.hwnd,NULL,hInstance,NULL);
	}
	else
	{
		const char* text = name;
		mWindowHandle.hwnd = ::CreateWindow(classname,
			text,mask,0,0,width,height,mWindowParent.hwnd,NULL,hInstance,NULL);
	}

	// parent window
	if ( mWindowParent.hwnd )
	{
		::SetParent(mWindowHandle.hwnd,mWindowParent.hwnd);
	}

	// set win32 hwnd userdata to point to the window object
	::SetWindowLong(mWindowHandle.hwnd,GWL_USERDATA,(LONG)this);
	::UpdateWindow(mWindowHandle.hwnd);

	// store windowstyle
	mWindowStyle = windowstyle;

	// default behaviour
	if ( show )
		ShowWindow();

	ShowCursor();
	DragAcceptFiles(mWindowHandle.hwnd,TRUE);

	// set state
	mIsOpen = true;
	EnableEvent();

	return true;
}

// Aktion
// Would be nice if i can pass hInstance here as well.
// and perhaps you want clean up it with using WINDOW_CHILD constant.

bool WindowBase::OpenWindow(WindowHandle parent, int width, int height)
{
	if ( IsWindowOpen() )
		CloseWindow();

	if ( IsWindowBind() )
		UnbindWindow();

	//TODO: move
	uint32 mask = WS_CHILD;

	// register window class
	HINSTANCE hInstance = NULL;
	String classname = RegisterWindowClass(hInstance,false);

	// adjust rectangle
	AdjustSize(mask,width,height);

	{
		const char* text = "noname";
		mWindowHandle.hwnd = ::CreateWindow(classname,text,mask,0,0,width,height,parent.hwnd,NULL,hInstance,NULL);
	}

	// parent window
	if ( parent.hwnd )
	{
		::SetParent(mWindowHandle.hwnd,parent.hwnd);
	}

	// set win32 hwnd userdata to point to the window object
	::SetWindowLong(mWindowHandle.hwnd,GWL_USERDATA,(LONG)this);
	::UpdateWindow(mWindowHandle.hwnd);

	// store windowstyle
	mWindowStyle = WINDOW_CHILD;

	// default behaviour
	ShowWindow();
	ShowCursor();

	// set state
	mIsOpen = true;
	EnableEvent();

	return true;
}


bool WindowBase::CloseWindow(bool mainbreak)
{
	if ( mIsOpen )
	{
		// close event
		EventClose();

		// set state
		mIsOpen = false;
		DisableEvent();

		if ( gb_destroy_window )
		{
			gb_mainbreak = mainbreak;
			gb_call_close = false;
			::DestroyWindow(mWindowHandle.hwnd);
			gb_call_close = true;
		}

		gb_destroy_window = true;
		mWindowHandle.hwnd = NULL;

		return true;
	}
	return false;
}

void WindowBase::RenameWindow(const String& name)
{
	const char* text = name;
	::SetWindowText(mWindowHandle.hwnd,text);
}

void WindowBase::ResizeWindow(int width, int height)
{
	LONG mask = GetWindowLong(mWindowHandle.hwnd,GWL_STYLE);
	AdjustSize(mask,width,height);

	::SetWindowPos(mWindowHandle.hwnd,HWND_TOP,
		0,0,width,height,SWP_NOMOVE|SWP_NOZORDER);
}

void WindowBase::MoveWindow(int x, int y)
{
	::SetWindowPos(mWindowHandle.hwnd,HWND_TOP,
		x,y,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

void WindowBase::ShowCursor()
{
	while ( ::ShowCursor(TRUE) < 0 )
	{
	}
}

void WindowBase::HideCursor()
{
	while ( ::ShowCursor(FALSE) >= 0 )
	{
	}

	// work around a Windows bug:
	// cursor won't be hidden until it gets a move message
	::SetCursorPos(0,0);
	::SetCursorPos(1,0);
}

void WindowBase::ShowWindow()
{
	::ShowWindow(mWindowHandle.hwnd,SW_SHOW);
}

void WindowBase::HideWindow()
{
	::ShowWindow(mWindowHandle.hwnd,SW_HIDE);
}

void WindowBase::EnableEvent()
{
	mIsEventEnable = true;
}

void WindowBase::DisableEvent()
{
	mIsEventEnable = false;
}

void WindowBase::SetMainFrequency(float freq)
{
	assert( freq >= 0 );
	mMainFreq = freq;
}

float WindowBase::GetMainFrequency() const
{
	return mMainFreq;
}

void WindowBase::MainBreak()
{
	mMainBreak = true;
}

void WindowBase::MainLoop()
{

	// enable callback events
	//EnableEvent();

	// update window before we enter the mainloop
	// this is for applications which do not generate animation
	// .. so that the EventDraw() is called atleast once!
	// .. and anything else we might think to add in the future!
	::UpdateWindow(mWindowHandle.hwnd);

	// enter the message loop
	MSG msg;
	::ZeroMemory(&msg,sizeof(msg));

	// timer for frequency control
	Timer timer;
	float prevtime = timer.GetTime();

	// mainbreak:false
	mMainBreak = false;

	// message loop
	while ( msg.message != WM_QUIT && mMainBreak == false )
	{
		if ( ::PeekMessage(&msg,NULL,0U,0U,PM_REMOVE) )
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			float freq = mMainFreq;
			if ( freq )
			{
				// frequency controlled EventMain() dispatching
				float curtime = timer.GetTime();
				if ( (curtime - prevtime) >= (1 / freq) )
				{
					prevtime = curtime;
					if ( !EventMain() )
						break;
				}
			}
			else
			{
				// frequency control free EventMain() dispatching
				if ( !EventMain() )
					break;
			}
		}
	}

	// disable callback events
	//DisableEvent();
}

bool WindowBase::IsWindowOpen() const
{
	return mIsOpen;
}

bool WindowBase::IsWindowBind() const
{
	return mIsBind;
}

bool WindowBase::IsEventEnable() const
{
	return mIsEventEnable;
}

uint32 WindowBase::GetWindowStyle() const
{
	return mWindowStyle;
}

WindowHandle WindowBase::GetWindowHandle() const
{
	return mWindowHandle;
}

Rect WindowBase::GetWindowRect(bool frame) const
{
	RECT rect;

	if ( frame )
	{
		::GetWindowRect(mWindowHandle.hwnd,&rect);
	}
	else
	{
		::GetClientRect(mWindowHandle.hwnd,&rect);
	}
	
	return Rect(
		rect.left,rect.top,
		(rect.right - rect.left),
		(rect.bottom - rect.top));
}

float WindowBase::GetWindowAspect() const
{
	Rect rect = GetWindowRect();
	return rect.GetAspect();
}

bool WindowBase::IsDown(int keycode) const
{
	return mKeyDown[keycode & 0xff];
}

void WindowBase::DispatchEventKeyboard(int keycode, char charcode, bool press)
{
	// update the keymap
	mKeyDown[keycode & 0xff] = press;
	
	// tell the client key was pressed or released
	EventKeyboard(keycode,charcode,press);
}

void* WindowBase::GetPrevProc() const
{
	return mpPrevProc;
}

} // prcore
