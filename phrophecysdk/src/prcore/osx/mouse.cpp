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
	Mouse OSX implementation
		
	revision history:
	December/25/2003 - Stefan Karlsson
 */

#include <prcore/prcore.hpp>
#include <Carbon/Carbon.h>
namespace prcore
{
        

    // ========================================
    // globals
    // ========================================

    // needed in another module, do not declare static
    static int mouseX = 0;
    static int mouseY = 0;

    Mouse::Mouse()
    { 

    }


    Mouse::~Mouse()
    {
    }


    void Mouse::SetArea(const Rect& rect)
    {
    }

    void Mouse::SetXY(int x, int y)
    {
    }


    int Mouse::GetX()
    {
        ::Point mousePos; 
        GetMouse(&mousePos);
        mouseX = mousePos.h;
        mouseY = mousePos.v;
        
        return mouseX;
    }


    int Mouse::GetY()
    {
        ::Point mousePos;
        GetMouse(&mousePos);
        mouseX = mousePos.h;
        mouseY = mousePos.v;
     
        return mouseY;
    }


    int Mouse::GetAxisCount() const
    {
        return 0;
    }


    int Mouse::GetButtonCount() const
    {
        return 1;
    }


    float Mouse::GetAxis(int index)
    {
        return 0;
    }


    int Mouse::GetButton(int index)
    {
        return Button()?1:0;
    }
}
