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
  Mouse Linux implementation
		
  revision history:
  ???/??/2001 - Edmond Cote - initial revision
  Apr/13/2003 - Jukka Liimatta - update
  Dec/30/2003 - Edmond Cote - update
	
*/
#include <X11/Xlib.h>
#include <prcore/prcore.hpp>

using namespace prcore;


// ========================================
// globals
// ========================================

// needed in another module, do not declare static
void mouseMovementStaticHook(XButtonEvent* event);
void mouseButtonStaticHook(XButtonEvent* event, bool state);

static int mouseFlags = 0;
static int mouseX = 0;
static int mouseY = 0;
static long mouseStaticInitCount = 0;


Mouse::Mouse()
{ 
  // install hook
  ++mouseStaticInitCount;
}


Mouse::~Mouse()
{
  if ( mouseStaticInitCount > 0 )
    --mouseStaticInitCount;

  //unhook
}


void Mouse::SetArea(const Rect& rect)
{
  //RECT clip = { rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom() };
  //ClipCursor(&clip);
}

void Mouse::SetXY(int x, int y)
{
  //SetCursorPos(x,y);
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
    case MOUSE_LEFT:	return mouseFlags & MOUSE_LEFT;
    case MOUSE_RIGHT:	return mouseFlags & MOUSE_RIGHT;
    case MOUSE_MIDDLE:	return mouseFlags & MOUSE_MIDDLE;
    default:			return 0;
    }
  /*
    switch ( index )
    {
    case MOUSE_LEFT:	return (mouseFlags & MOUSE_LEFT) != 0;
    case MOUSE_RIGHT:	return (mouseFlags & MOUSE_RIGHT) != 0;
    case MOUSE_MIDDLE:	return (mouseFlags & MOUSE_MIDDLE) != 0;
    default:			return false;
    }
  */
}


void mouseMovementStaticHook(XButtonEvent* event)
{
  mouseX = static_cast<int>(event->x);
  mouseY = static_cast<int>(event->y);
}


void mouseButtonStaticHook(XButtonEvent* event, bool state)
{
  // note: we are reading the mouse position from the X11 message loop
  // is it possible to retrive the information directly?
  
  // 
  mouseX = static_cast<int>(event->x);
  mouseY = static_cast<int>(event->y);

  // note: button numbers 4 and 5 represeent mouse wheel up and down

  int button = event->button;
  switch ( button )
    {
    case 1:
      mouseFlags |= MOUSE_LEFT;
      mouseFlags &= (0x0ffffff - MOUSE_MIDDLE);
      mouseFlags &= (0x0ffffff - MOUSE_RIGHT);
      break;

    case 2:
      mouseFlags |= MOUSE_MIDDLE;
      mouseFlags &= (0x0ffffff - MOUSE_LEFT);
      mouseFlags &= (0x0ffffff - MOUSE_RIGHT);
      break;

    case 3:
      mouseFlags |= MOUSE_RIGHT;
      mouseFlags &= (0x0ffffff - MOUSE_LEFT);
      mouseFlags &= (0x0ffffff - MOUSE_MIDDLE);
      break;

    default:
      mouseFlags &= (0x0ffffff - MOUSE_LEFT);
      mouseFlags &= (0x0ffffff - MOUSE_MIDDLE);
      mouseFlags &= (0x0ffffff - MOUSE_RIGHT);
      break;
    }
}


