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
  WindowBase UNIX implementation

  revision history:
  ???/??/2001 - Edmond Cote - initial revision
  May/12/2002 - Edmond Cote - update
  Apr/13/2003 - Jukka Liimatta - update (* still a lot of work to do..)
  Dec/30/2003 - Edmond Cote - update (* less work to do)
*/

#include <prcore/timer.hpp>
#include <prcore/keyboard.hpp>
#include <prcore/mouse.hpp>
#include <prcore/windowbase.hpp>
#include <prcore/exception.hpp>

#include <X11/keysym.h>
#include <GL/glx.h>

using namespace prcore;

// ======================================
// globals
// ======================================

static bool gDestroyWindowOnClose = true;

extern void kbStaticHook(unsigned char key, char ascii, bool state);
extern void mouseButtonStaticHook(XButtonEvent* event, bool state);
extern void mouseMovementStaticHook(XButtonEvent* event);


static int remap(int code)
{
  switch ( code )
    {
    case XK_Escape:			return KEYCODE_ESC;
    case XK_0:				return KEYCODE_0;
    case XK_1:				return KEYCODE_1;
    case XK_2:				return KEYCODE_2;
    case XK_3:				return KEYCODE_3;
    case XK_4:				return KEYCODE_4;
    case XK_5:				return KEYCODE_5;
    case XK_6:				return KEYCODE_6;
    case XK_7:				return KEYCODE_7;
    case XK_8:				return KEYCODE_8;
    case XK_9:				return KEYCODE_9;
    case XK_a:				return KEYCODE_A;
    case XK_b:				return KEYCODE_B;
    case XK_c:				return KEYCODE_C;
    case XK_d:				return KEYCODE_D;
    case XK_e:				return KEYCODE_E;
    case XK_f:				return KEYCODE_F;
    case XK_g:				return KEYCODE_G;
    case XK_h:				return KEYCODE_H;
    case XK_i:				return KEYCODE_I;
    case XK_j:				return KEYCODE_J;
    case XK_k:				return KEYCODE_K;
    case XK_l:				return KEYCODE_L;
    case XK_m:				return KEYCODE_M;
    case XK_n:				return KEYCODE_N;
    case XK_o:				return KEYCODE_O;
    case XK_p:				return KEYCODE_P;
    case XK_q:				return KEYCODE_Q;
    case XK_r:				return KEYCODE_R;
    case XK_s:				return KEYCODE_S;
    case XK_t:				return KEYCODE_T;
    case XK_u:				return KEYCODE_U;
    case XK_v:				return KEYCODE_V;
    case XK_w:				return KEYCODE_W;
    case XK_x:				return KEYCODE_X;
    case XK_y:				return KEYCODE_Y;
    case XK_z:				return KEYCODE_Z;
    case XK_F1:				return KEYCODE_F1;
    case XK_F2:				return KEYCODE_F2;
    case XK_F3:				return KEYCODE_F3;
    case XK_F4:				return KEYCODE_F4;
    case XK_F5:				return KEYCODE_F5;
    case XK_F6:				return KEYCODE_F6;
    case XK_F7:				return KEYCODE_F7;
    case XK_F8:				return KEYCODE_F8;
    case XK_F9:				return KEYCODE_F9;
    case XK_F10:			return KEYCODE_F10;
    case XK_F11:			return KEYCODE_F11;
    case XK_F12:			return KEYCODE_F12;
    case XK_KP_0:			return KEYCODE_NUMPAD0;
    case XK_KP_1:			return KEYCODE_NUMPAD1;
    case XK_KP_2:			return KEYCODE_NUMPAD2;
    case XK_KP_3:			return KEYCODE_NUMPAD3;
    case XK_KP_4:			return KEYCODE_NUMPAD4;
    case XK_KP_5:			return KEYCODE_NUMPAD5;
    case XK_KP_6:			return KEYCODE_NUMPAD6;
    case XK_KP_7:			return KEYCODE_NUMPAD7;
    case XK_KP_8:			return KEYCODE_NUMPAD8;
    case XK_KP_9:			return KEYCODE_NUMPAD9;
    case XK_Num_Lock:		return KEYCODE_NUMLOCK;
    case XK_KP_Divide:		return KEYCODE_DIVIDE;
    case XK_KP_Multiply:	return KEYCODE_MULTIPLY;
    case XK_KP_Subtract:	return KEYCODE_SUBTRACT;
    case XK_KP_Add:			return KEYCODE_ADDITION;
    case XK_KP_Decimal:		return KEYCODE_DECIMAL;
    case XK_BackSpace:		return KEYCODE_BACKSPACE;
    case XK_Tab:			return KEYCODE_TAB;
    case XK_Return:			return KEYCODE_RETURN;
    case XK_Control_L:  	return KEYCODE_LEFT_CTRL;
    case XK_Control_R:		return KEYCODE_RIGHT_CTRL;
    case XK_Shift_L:		return KEYCODE_LEFT_SHIFT;
    case XK_Shift_R:		return KEYCODE_RIGHT_SHIFT;
    case XK_space:			return KEYCODE_SPACE;
    case XK_Sys_Req:		return KEYCODE_PRINT_SCREEN;
    case XK_Scroll_Lock:	return KEYCODE_SCROLL_LOCK;
    case XK_Page_Up:		return KEYCODE_PAGEUP;
    case XK_Page_Down:		return KEYCODE_PAGEDOWN;
    case XK_Insert:			return KEYCODE_INSERT;
    case XK_Delete:			return KEYCODE_DELETE;
    case XK_Home:			return KEYCODE_HOME;
    case XK_End:			return KEYCODE_END;
    case XK_Left:			return KEYCODE_LEFT;
    case XK_Right:			return KEYCODE_RIGHT;
    case XK_Up:				return KEYCODE_UP;
    case XK_Down:			return KEYCODE_DOWN;
    case XK_Alt_L:      	return KEYCODE_LEFT_ALT;
    case XK_Alt_R:      	return KEYCODE_RIGHT_ALT;
    case XK_Caps_Lock:  	return KEYCODE_CAPSLOCK;
    case XK_KP_Enter:   	return KEYCODE_ENTER;
    default:				return 0;
    }

  // the following cannot be handled by the EventKeyboard()
  // ----------------------------------------------------------------
  // KEYCODE_PRNT_SCREEN
}


// ======================================
// WindowBase
// ======================================

WindowBase::WindowBase()
  : mMainFreq(0),mpPrevProc(NULL),mIsOpen(false),mIsBind(false),
    mIsEventEnable(false)
{
  mWindowHandle.display = NULL;
  mWindowHandle.window  = 0;
  mWindowHandle.cursor  = 0;
  mWindowHandle.pixmap  = 0;

  mMainBreak = false;

  for ( int i=0; i<256; ++i )
    mKeyDown[i] = false;
}


// this constructor is ignored, same as above
WindowBase::WindowBase(WindowHandle parent)
  : mMainFreq(0),mpPrevProc(NULL),mIsOpen(false),mIsBind(false),
    mIsEventEnable(false)
{
  mWindowHandle.display = NULL;
  mWindowHandle.window  = 0;
  mWindowHandle.cursor  = 0;
  mWindowHandle.pixmap  = 0;

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
  // TODO

  PRCORE_EXCEPTION("Function unsupported!");

  mIsBind = false;
  return false;
}


bool WindowBase::UnbindWindow()
{

  mIsBind = false;
  return false;
}


bool WindowBase::OpenWindow(int width, int height, const String& name, uint32 windowstyle, bool show)
{
  if ( IsWindowOpen() )
    CloseWindow();

  // open display

  Display* display = XOpenDisplay(NULL);
  if ( !display )
    PRCORE_EXCEPTION("Unable to establish connection to X Server.");

  int screen = DefaultScreen(display);
  Window rootwindow = RootWindow(display,screen);

  Window window;

  // choose visual

  XVisualInfo visual;

  if(windowstyle & WINDOW_USES3D)
    {
      // query GLX extension

      int dummy;

      if(!glXQueryExtension(display, &dummy, &dummy)) 
	PRCORE_EXCEPTION("X server has no OpenGL GLX extension");

      // locat an OpenGL compatible visual (w/ DOUBLE_BUFFERING) with 
      // largest possible depth and stencil buffers

      int dbAttribList[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8,
			     GLX_DOUBLEBUFFER, None };

      int sbAttribList[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8,
			     None };
     
      XVisualInfo *v = glXChooseVisual(display,screen,dbAttribList);

      if(!v)
	{
	  v = glXChooseVisual(display,screen,sbAttribList);
	  if(!v)
	    PRCORE_EXCEPTION("no RGB visual with depth and stencil buffer availible");
	  // todo: place warning such that no double buffered visuals were found
	}
      
      if(v->c_class != TrueColor)
	PRCORE_EXCEPTION("TrueColor visual required");
      
      visual = *v;

    }
  else // no 3d support
    {

      // get an appropriate visual

      XVisualInfo* v;
      int numvis;
      int i;
      
      visual.depth  = 0;
      visual.screen = screen;
      
      v = XGetVisualInfo(display,VisualScreenMask,&visual,&numvis);
      
      if ( !v )
	PRCORE_EXCEPTION("No suitable visual available.");
      
      for ( i=0; i<numvis; ++i )
	{
	  
	  if ( v[i].depth > visual.depth && v[i].c_class == TrueColor)
	    visual = v[i];
	}
      
      XFree(v);
		
    }

  // create colormap

  Colormap colormap = XCreateColormap(display,rootwindow,visual.visual,
				      AllocNone);

  // set window attributes

  XSetWindowAttributes attributes;

  attributes.background_pixmap = None;
  attributes.background_pixel  = WhitePixel(display,screen);
  attributes.colormap          = colormap;
  attributes.border_pixel      = 0;
  attributes.event_mask        = PointerMotionMask | ButtonPressMask |
    ButtonReleaseMask | ExposureMask | KeyPressMask | KeyReleaseMask |
    StructureNotifyMask | EnterWindowMask | LeaveWindowMask;

  if (( windowstyle & WINDOW_FULLSCREEN)  == WINDOW_FULLSCREEN )
    {
      attributes.override_redirect = True;
    }
  else
    {
      attributes.override_redirect = False;
    }

  // create the window

  window = XCreateWindow(display,rootwindow,
			 0,0,width,height,0,
			 visual.depth,InputOutput,visual.visual,
			 CWBackPixmap | CWBackPixel | CWColormap |
			 CWBorderPixel | CWEventMask | CWOverrideRedirect,
			 &attributes);

  // set window name

  XSetStandardProperties(display,window,
			 name,(char*)"ProphecySDK",None,
			 NULL,0,(XSizeHints*)NULL);

  mWindowHandle.deleteatom = XInternAtom(display,"WM_DELETE_WINDOW",False);

  XSetWMProtocols(display,window,
		  &mWindowHandle.deleteatom,1);
  
  // create cursor

  char zero = 0;
  Pixmap pixmap = XCreatePixmapFromBitmapData(display,window,&zero,1,1,0,0,1);

  // create color ( black )

  XColor black;
  memset(&black,0,sizeof(black));

  // create pixmap cursor
      
  mWindowHandle.cursor = XCreatePixmapCursor(display,pixmap,pixmap,&black,
					     &black,0,0);

  // create graphics context

  GC gc=XCreateGC(display, window, 0, NULL);

  // set auto repeat to off 

  XAutoRepeatOff(display);

  // Requests that the window be shown on the scren

  XMapRaised(display,window);
    
  // grab exclusive keyboard and mouse access

  if (( windowstyle & WINDOW_FULLSCREEN )  == WINDOW_FULLSCREEN)
    {
      XGrabKeyboard(display,window,False,
	 	    GrabModeAsync,GrabModeAsync,CurrentTime);
       
      XGrabPointer(display,window,True,
		   PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		   GrabModeAsync,GrabModeAsync,
		   window,None,CurrentTime);
      
      // TODO: we may want to shut off the the screensaver
    }
  
  // save window handle
            
  mWindowHandle.display = display;
  mWindowHandle.window  = window;
  mWindowHandle.visual  = visual;
  mWindowHandle.pixmap  = pixmap;
  mWindowHandle.gc = gc;

  // store windowstyle

  mWindowStyle = windowstyle;

  // show window

  ShowWindow();

  // show cursor

  ShowCursor();

  // set state

  mIsOpen = true;

  EnableEvent();

  return true;

}


bool WindowBase::OpenWindow(WindowHandle parent, int width, int height)
{

	// TODO: implement linux/x11 version

	/*
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
	*/

	return false;
}


bool WindowBase::CloseWindow(bool mainbreak)
{
  if ( IsWindowOpen() )
    {
      EventClose();
      XAutoRepeatOn(mWindowHandle.display);

      if ( gDestroyWindowOnClose )
	{
	  XFreeCursor(mWindowHandle.display,
		      mWindowHandle.cursor);
	  mWindowHandle.cursor = 0;

	  XFreePixmap(mWindowHandle.display,
		      mWindowHandle.pixmap);
	  mWindowHandle.pixmap = 0;

	  XFreeGC(mWindowHandle.display, mWindowHandle.gc);

	  XCloseDisplay(mWindowHandle.display);
	}
      gDestroyWindowOnClose = true;

      mWindowHandle.window  = 0;
      mWindowHandle.display = NULL;

      mIsOpen = false;

      return true;
    }
  return false;
}


void WindowBase::RenameWindow(const String& name)
{
  XStoreName(mWindowHandle.display,mWindowHandle.window,name);
}


void WindowBase::ResizeWindow(int width, int height)
{
  XResizeWindow(mWindowHandle.display,
		mWindowHandle.window,width,height);
}


void WindowBase::MoveWindow(int x, int y)
{
  XMoveWindow(mWindowHandle.display,
	      mWindowHandle.window,x,y);
}


void WindowBase::ShowCursor()
{
  XUndefineCursor(mWindowHandle.display,
		  mWindowHandle.window);
}


void WindowBase::HideCursor()
{
  XDefineCursor(mWindowHandle.display,
		mWindowHandle.window,
		mWindowHandle.cursor);
}


void WindowBase::ShowWindow()
{  
  // note, this doesn't function like win32 ::ShowWindow function
  // it raises the window, so that the entire window is visble

  // only valid if window is in fullscreen

  // todo: test this
  // XMapRaised();

  // XRaiseWindow

  XMapRaised(mWindowHandle.display,
	     mWindowHandle.window);
}


void WindowBase::HideWindow()
{

  // this doesn't make hte window disapear, only lowers the window
  // could technically use iconify window, that will be tried in full screen mode
    
  /*XLowerWindow(mWindowHandle.display,
    mWindowHandle.window);*/

  XIconifyWindow(mWindowHandle.display,
		 mWindowHandle.window,
		 mWindowHandle.visual.screen);
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

  // todo: update window before entering the mainloop
  // this would be for applications which do not generate
  // animation, eventdraw() would need to be called at least
  // once
  // in X11 terms I believe we'd want an Expose event to be posted

  Timer timer;
  float prevtime = timer.GetTime();

  mMainBreak = false;

  // message loop

  while(mMainBreak==false)
    {

      if ( XPending(mWindowHandle.display) > 0 )
	{
	  XEvent event;
	  XNextEvent(mWindowHandle.display,&event);

	  switch ( event.type )
	    {
	      XSizeHints sizehints;
	      KeySym keysym;
	      
	    case Expose:

	      if ( mWindowHandle.window && IsEventEnable() )
		EventDraw();

	      break;

	      
	    case KeyPress:

	      // generate static hook for keyboard driver

	      keysym = XLookupKeysym((XKeyEvent*)&event,0);
	      kbStaticHook(remap(keysym),0,true);

	      if( mWindowHandle.window && IsEventEnable() )
		DispatchEventKeyboard(remap(keysym),keysym,true);

	      break;

	    case KeyRelease:

	      // generate static hook for keyboard driver

	      keysym = XLookupKeysym((XKeyEvent*)&event,0);
	      kbStaticHook(remap(keysym),0,false);

	      if( mWindowHandle.window && IsEventEnable() )
		EventKeyboard(remap(keysym),keysym,false);

	      break;

	    case MotionNotify:

	      // generate static hook for mouse driver

	      mouseMovementStaticHook((XButtonEvent*)&event);

	      if( mWindowHandle.window && IsEventEnable() )
		{

		  XButtonEvent *xbe=(XButtonEvent *)&event;
		  EventMouseMove(xbe->x,xbe->y);
		}

	      break;

		case ButtonPress:

			// generate static hook for mouse driver

			mouseButtonStaticHook((XButtonEvent*)&event,true);

			if ( mWindowHandle.window && IsEventEnable() )
			{

				// todo: this does not handle the mouse wheel at all
				// in the same way that win32 does
	  
				XButtonEvent* xbe = reinterpret_cast<XButtonEvent*>(&event);

				switch ( xbe->button )
				{
					case 1:
						EventMouseButton(xbe->x,xbe->y,MOUSE_LEFT,1);
						break;

					case 2:
						EventMouseButton(xbe->x,xbe->y,MOUSE_RIGHT,1);
						break;

					case 3:
						EventMouseButton(xbe->x,xbe->y,MOUSE_MIDDLE,1);
						break;

					case 4:
						EventMouseButton(xbe->x,xbe->y,MOUSE_WHEEL,1);
						break;

					case 5:
						EventMouseButton(xbe->x,xbe->y,MOUSE_WHEEL,1);
						break;
				}
			}

			break;

	    case ButtonRelease:

	      // generate static hook for mouse drivers

	      mouseButtonStaticHook((XButtonEvent*)&event,false);

	      if( mWindowHandle.window && IsEventEnable())
		{

		  // todo: this does not handle the mouse wheel at all 
		  // in the same way that win32 does

				XButtonEvent* xbe = reinterpret_cast<XButtonEvent*>(&event);

				switch ( xbe->button )
				{
					case 1:
						EventMouseButton(xbe->x,xbe->y,MOUSE_LEFT,0);
						break;

					case 2:
						EventMouseButton(xbe->x,xbe->y,MOUSE_RIGHT,0);
						break;

					case 3:
						EventMouseButton(xbe->x,xbe->y,MOUSE_MIDDLE,0);
						break;

					case 4:
						EventMouseButton(xbe->x,xbe->y,MOUSE_WHEEL,0);
						break;

					case 5:
						EventMouseButton(xbe->x,xbe->y,MOUSE_WHEEL,0);
						break;
				}
		}
	      
	      break;

	    case ConfigureNotify:

	      if ( mWindowHandle.window && IsEventEnable())
		{
		  sizehints.x           = event.xconfigure.x;
		  sizehints.y           = event.xconfigure.y;
		  sizehints.width       = event.xconfigure.width;
		  sizehints.height      = event.xconfigure.height;
		  sizehints.base_width  = event.xconfigure.width;
		  sizehints.base_height = event.xconfigure.height;

		  EventSize(event.xconfigure.width,event.xconfigure.height);
		}

	      break;

	    case EnterNotify:

	      if ( mWindowHandle.window && IsEventEnable())
		EventFocus(true);

	      break;

	    case LeaveNotify:

	      if ( mWindowHandle.window && IsEventEnable())
		EventFocus(false);

	      break;

	    case ClientMessage:

	      if ( event.xclient.data.l[0] == 
		   static_cast<int>(mWindowHandle.deleteatom) )
		mMainBreak = true;

	      break;

	    default:
	      break;
	    }
	} // end X11 Message Loop
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
  // TODO: support frame parameter
  // Q?: what does frame do

  Rect rect;

  if(frame)
    {
      PRCORE_EXCEPTION("Frame parameter not supported");
    }
  else
    {
      
      XWindowAttributes attrib;

      XGetWindowAttributes(mWindowHandle.display,
			   mWindowHandle.window,
			   &attrib);

      rect.x      = attrib.x;
      rect.y      = attrib.y;
      rect.width  = attrib.width;
      rect.height = attrib.height;
    }

  return rect;
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


