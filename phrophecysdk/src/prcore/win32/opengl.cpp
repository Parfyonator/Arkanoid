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
		OpenGL windows implementation

	revision history:
		Apr/22/2001 - Jukka Liimatta - initial revision
		Dec/14/2002 - Francesco Montecuccoli - added ZeroMemory() for WGL structures
*/
#include <prcore/configure.hpp>
#include <prcore/array.hpp>
#include <prcore/opengl.hpp>
#include <GL/gl.h>

using namespace prcore;


// =================================================
// modelist
// =================================================

static Array<ModeGL> ModeList;


static void EnumModeList()
{
	DEVMODE devmode;
	ZeroMemory(&devmode, sizeof(DEVMODE));

	devmode.dmSize = sizeof(DEVMODE);

	for ( int index=0; EnumDisplaySettings(NULL,index,&devmode); ++index )
	{
		// reject indexed pixelformats
		if ( devmode.dmBitsPerPel > 8 )
		{
			ModeGL mode;

			mode.width     = devmode.dmPelsWidth;
			mode.height    = devmode.dmPelsHeight;
			mode.bits      = devmode.dmBitsPerPel;
			mode.frequency = devmode.dmDisplayFrequency;

			ModeList.PushBack(mode);
		}
	}
}


// =================================================
// context
// =================================================

struct ContextWGL
{
	HDC		hdc;
	HGLRC	hrc;
};


// =================================================
// WindowGL
// =================================================

WindowGL::WindowGL()
{
	if ( !ModeList.GetSize() )
	{
		EnumModeList();
	}
}


WindowGL::~WindowGL()
{
	RestoreMode();
}


int WindowGL::GetModeCount() const
{
	return ModeList.GetSize();
}


const ModeGL& WindowGL::GetMode(int index) const
{
	assert( index>=0 && index<= ModeList.GetSize() );
	return ModeList[index];
}


bool WindowGL::SetMode(uint32 width, uint32 height, uint32 bits, uint32 frequency)
{
	DEVMODE devmode;
	ZeroMemory(&devmode, sizeof(DEVMODE));

	devmode.dmSize        = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	devmode.dmBitsPerPel  = bits;
	devmode.dmPelsWidth   = width;
	devmode.dmPelsHeight  = height;
	devmode.dmFields      = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if ( frequency )
	{
		devmode.dmDisplayFrequency = frequency;
		devmode.dmFields |= DM_DISPLAYFREQUENCY;
	}

	return ChangeDisplaySettings(&devmode,CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
}


bool WindowGL::SetMode(const ModeGL& mode)
{
	return SetMode(mode.width,mode.height,mode.bits,mode.frequency);
}


bool WindowGL::RestoreMode()
{
	return ChangeDisplaySettings(NULL,0) == DISP_CHANGE_SUCCESSFUL;
}


ContextGL* WindowGL::CreateContext(uint32 color, uint32 depth, uint32 stencil, uint32 accum)
{
	// do we have a rendering window?
	HWND hwnd = mWindowHandle.hwnd;
	if ( !hwnd )
		return NULL;

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType   = PFD_TYPE_RGBA;
	pfd.cColorBits   = color;
	pfd.cAccumBits   = accum;
	pfd.cDepthBits   = depth;
	pfd.cStencilBits = stencil;
	pfd.dwLayerMask  = PFD_MAIN_PLANE;

	// get device context
	HDC hdc = GetDC(hwnd);

	// set pixelformat
	int index = ChoosePixelFormat(hdc,&pfd);
	SetPixelFormat(hdc,index,&pfd);

	// create gl render context
	HGLRC hrc = wglCreateContext(hdc);

	ContextWGL* context = new ContextWGL();
	context->hdc = hdc;
	context->hrc = hrc;

	return (ContextGL*)context;
}


void WindowGL::DeleteContext(ContextGL* context)
{
	ContextWGL* ctx = (ContextWGL*)context;
	
	if ( ctx )
	{
		wglDeleteContext(ctx->hrc);
		delete ctx;
	}

	if ( mCurrentContext == context )
	{
		wglMakeCurrent(NULL,NULL);
		mCurrentContext = NULL;
	}
}


void WindowGL::SetContext(ContextGL* context)
{
	ContextWGL* ctx = (ContextWGL*)context;
	if ( ctx )
		wglMakeCurrent(ctx->hdc,ctx->hrc);
		
	mCurrentContext = context;
}


void WindowGL::PageFlip(bool retrace)
{
	ContextWGL* ctx = (ContextWGL*)mCurrentContext;
	if ( ctx )
		SwapBuffers(ctx->hdc);
}
