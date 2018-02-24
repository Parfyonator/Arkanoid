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
	Keyboard OSX implementation
		
	revision history:
	December/25/2003 - Stefan Karlsson (just unix ripoff this time.. nothing i think ;)
*/
#include <prcore/prcore.hpp>

using namespace prcore;


static char kbuffer[256];
static uint8 pbuffer[256];
static const int KCHARMAX = 256;

static long kbStaticInitCount = 0;
static uint32 pkbfunc = 0;

// needed in another module, do not declare static
void kbStaticHook(unsigned char key, char ascii, bool state);


Keyboard::Keyboard()
{
	++kbStaticInitCount;
	pkbfunc = 0;
	memset(kbuffer,0,sizeof(kbuffer));
	memset(kbuffer,0,sizeof(pbuffer));
}


Keyboard::~Keyboard()
{
	if ( kbStaticInitCount > 0 )
		--kbStaticInitCount;
}


int Keyboard::GetButtonCount() const
{
	return KCHARMAX;
}


int Keyboard::GetButton(int code)
{
	return kbuffer[code];
//	return !( kbuffer[code] == 0 );
}


void kbStaticHook(unsigned char key, char ascii, bool state)
{
	kbuffer[key] = state;
}
