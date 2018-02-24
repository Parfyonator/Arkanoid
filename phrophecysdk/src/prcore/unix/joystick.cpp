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
	Joystick Linux implementation
		
	revision history:
	???/??/2001 - Edmond Cote - Initial Revision
	Dec/30/2003 - Edmond Cote - Patch in order to compile

	todo:
	- everything ;-)

	notes:
	- see /usr/src/linux/Documentation/joystick-api.txt for implementation information
*/
#include <prcore/prcore.hpp>

using namespace prcore;


Joystick::Joystick(WindowHandle handle, int index)
{
  //  int fd = open("/dev/js0", O_RDONLY);
}


Joystick::~Joystick()
{
}


int Joystick::GetAxisCount() const
{
	return 0;
}


int Joystick::GetButtonCount() const
{
	return 0;
}


int Joystick::GetPOVCount() const
{
	return 0;
}


float Joystick::GetAxis(int i)
{
	return 0;
}


int Joystick::GetButton(int code)
{
	return false;
}


int Joystick::GetPOV(int index)
{
	return -1;
}


int Joystick::GetJoystickCount()
{
	return 0;
}


const char* Joystick::GetJoystickName(int index)
{
	return NULL;
}


