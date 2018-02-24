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
  WindowGL / OpenGL Linux implementation
		
  revision history:
  ???/??/2001 - Edmond Cote - initial revision
  Dec/31/2003 - Edmond Cote - update
*/

#include <prcore/configure.hpp>
#include <prcore/array.hpp>
#include <prcore/opengl.hpp>
#include <prcore/exception.hpp>

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>


using namespace prcore;

#include <iostream>
using namespace std;

// =================================================
// modelist
// =================================================

static Array<ModeGL> ModeList;
static XF86VidModeModeInfo** ModeLines;
static bool mIsFullScreen;
static int curMode;

static void EnumModeList(WindowHandle mWindowHandle)
{
  
  int major, minor; // XF86 Version Numbers
  int event_base, error_base;
  int i,n;
  
  // check if extension is availible!
  
  if(!XF86VidModeQueryVersion(mWindowHandle.display,&minor, &major) ||
     !XF86VidModeQueryExtension(mWindowHandle.display,&event_base,&error_base))
    PRCORE_EXCEPTION("XF86VidMode Extension not availible");
      
  // query all modes

  XF86VidModeGetAllModeLines(mWindowHandle.display,
			     //mWindowHandle.visual.screen,
			     DefaultScreen(mWindowHandle.display),
			     &n, &ModeLines);
  
  // insert entry in ModeGL Array
  
  for(i=0;i<n;i++)
    {
      ModeGL mode;
      mode.width=ModeLines[i]->hdisplay;
      mode.height=ModeLines[i]->vdisplay;

      // wouldn't changing the display affect the X11 Visual

      // hack.. store index to modeline array
      mode.bits=i;

      // gee, shouldn't XFree calculate this for you!?
      mode.frequency=1000*ModeLines[i]->dotclock/(float)(ModeLines[i]->htotal*ModeLines[i]->vtotal);
 
      ModeList.PushBack(mode);

    }

  // set current mode ( 0 = default )
  // note that the 0 represents the size of the XFree desktop, and not
  // necessairily the size that the user started the program with
  // ie, via manual changes (ctrl, alt, +/-) of resolutions

  curMode = 0;

}


// =================================================
// context
// =================================================

struct ContextGLX
{
  GLXContext glXContext;
};

// =================================================
// WindowGL
// =================================================

WindowGL::WindowGL()
{
  mIsFullScreen = false;
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
  // check if index is valid

  assert( index>=0 && index<= ModeList.GetSize() );

  return ModeList[index];
}


// with respect to frequency, we are specifying a MINIMUM vertical refresh
// frequency
// ignore the bpp for XFree86 (who runs apps in 16bpp anymore anyways?)

bool WindowGL::SetMode(uint32 width, uint32 height, uint32 bits, uint32 frequency)
{
  
  // ensure that the video modes (modelines) are enumerated

  if ( !ModeList.GetSize() )
    {
      EnumModeList(mWindowHandle);
    }


  // find suitable video mode

  for(int i=0;i<GetModeCount();i++)
    {

      const ModeGL &mode = GetMode(i);
     
      if(mode.width == width &&
	 mode.height == height &&
	 mode.frequency >= frequency)
	{
	  return SetMode(mode);
	}
            
    }
  
  return false;
}
bool WindowGL::SetMode(const ModeGL& mode)
{
  // hack!!

  int modeLineIndex = mode.bits;

  // ensure that the mode lines are enumerated

    if ( !ModeList.GetSize() )
    {
      EnumModeList(mWindowHandle);
    }

    // place window in upper left corner
     
    XMoveWindow(mWindowHandle.display, mWindowHandle.window, 0,0);

    // resize window

    XResizeWindow(mWindowHandle.display, mWindowHandle.window,mode.width,
		  mode.height);
     
    XSetWindowBorderWidth(mWindowHandle.display, mWindowHandle.window,0);
      
    // Switch to Mode

    XF86VidModeSwitchToMode(mWindowHandle.display,
			    mWindowHandle.visual.screen,
			    ModeLines[modeLineIndex]);
    // Set Viewport
    XF86VidModeSetViewPort(mWindowHandle.display, 
			   mWindowHandle.visual.screen, 0, 0);
    // Lock Video Mode
    XF86VidModeLockModeSwitch(mWindowHandle.display,
			      mWindowHandle.visual.screen,
			      1);

    XGrabKeyboard(mWindowHandle.display,mWindowHandle.window,False,
		  GrabModeAsync,GrabModeAsync,CurrentTime);
    
    XGrabPointer(mWindowHandle.display,mWindowHandle.window,True,
		 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		 GrabModeAsync,GrabModeAsync,
		 mWindowHandle.window,None,CurrentTime);

    // Hide Cursor

    HideCursor();
    
    mIsFullScreen = true;
    curMode = modeLineIndex;
    
    return true;

}

bool WindowGL::RestoreMode()
{

  if( mIsFullScreen )
    {
      
      // Unlock Video Mode

      XF86VidModeLockModeSwitch(mWindowHandle.display, 
				mWindowHandle.visual.screen,0);
      
      // As noted above, modes[0] is not necessairly the initial mode

      XF86VidModeSwitchToMode(mWindowHandle.display,
			      mWindowHandle.visual.screen,
			      ModeLines[0]);  
      
      // Reset Viewport

      XF86VidModeSetViewPort(mWindowHandle.display,
			     mWindowHandle.visual.screen,0,0);
      
      // Show Cursor

      ShowCursor();
      
      mIsFullScreen = false;
    }

  return true;

}

ContextGL* WindowGL::CreateContext(uint32 color, uint32 depth, uint32 stencil, uint32 accum)
{

  if ( !ModeList.GetSize() )
    {
      EnumModeList(mWindowHandle);
    }

  // note that in this case all the parameters are ignored

  ContextGLX* context = new ContextGLX();
    
  // create render context
  context->glXContext = glXCreateContext(mWindowHandle.display,
					 (XVisualInfo *)&mWindowHandle.visual,
					 NULL,true);

  if ( context->glXContext == NULL )
    PRCORE_EXCEPTION("Could not create rendering context!");
  
  return (ContextGL *)context;
}

void WindowGL::DeleteContext(ContextGL* context)
{
  ContextGLX* ctx = (ContextGLX*)context;

  if(ctx)
    {
      glXDestroyContext(mWindowHandle.display,ctx->glXContext );
      delete ctx;
    }

  mCurrentContext = NULL;
}

void WindowGL::SetContext(ContextGL* context)
{
  ContextGLX* ctx = (ContextGLX*)context;

  if ( ctx )
    {

      glXMakeCurrent(mWindowHandle.display, mWindowHandle.window,
		     ctx->glXContext);

      // note: this could probably be a warning.. but 
      if(!glXIsDirect(mWindowHandle.display, ctx->glXContext))
	cerr << "Warning Direct Rendering NOT Availible!" << endl;

    }
  
  mCurrentContext = context;
}

void WindowGL::PageFlip(bool retrace)
{
  ContextGLX* ctx = (ContextGLX*)mCurrentContext;
  
  if ( ctx )
    glXSwapBuffers(mWindowHandle.display,mWindowHandle.window);
}


