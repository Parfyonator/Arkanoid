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
		InputDevice

	revision history:
		Jun/01/2000 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Dec/08/2002 - Jukka Liimatta - christmas cleanup
*/
#include <cstddef>
#include <prcore/configure.hpp>
#include <prcore/inputdevice.hpp>

using namespace prcore;


// =================================================
// InputBase
// =================================================

int InputBase::GetAxisCount() const
{
	return 0;
}


int InputBase::GetButtonCount() const
{
	return 0;
}


float InputBase::GetAxis(int index)
{
	return 0.0f;
}


int InputBase::GetButton(int index)
{
	return 0;
}


// =================================================
// InputDevice
// =================================================

InputDevice::InputDevice()
{
}


InputDevice::~InputDevice()
{
}


InputDevice::BindIO* InputDevice::FindBindIO(Array<InputDevice::BindIO>& array, int index)
{
	BindIO* node = array.GetArray();
	BindIO* end = node + array.GetSize();

	for ( ; node<end; ++node )
	{
		if ( node->dest == index )
			return node;
	}

	return NULL;
}


void InputDevice::BindAxis(int dest, int src, InputBase& input)
{
	BindIO* bind = FindBindIO(axisbinds,dest);
	if ( bind )
	{
		// previous bind found, attach new control
		bind->dest  = dest;
		bind->src   = src;
		bind->input = &input;
	}
	else
	{
		// create a new bind and attach control
		BindIO& bindrf = axisbinds.PushBack();

		bindrf.dest  = dest;
		bindrf.src   = src;
		bindrf.input = &input;
	}
}


void InputDevice::BindButton(int dest, int src, InputBase& input)
{
	BindIO* bind = FindBindIO(buttonbinds,dest);
	if ( bind )
	{
		// previous bind found, attach new control
		bind->dest  = dest;
		bind->src   = src;
		bind->input = &input;
	}
	else
	{
		// create a new bind and attach control
		BindIO& bindrf = buttonbinds.PushBack();

		bindrf.dest  = dest;
		bindrf.src   = src;
		bindrf.input = &input;
	}
}


int InputDevice::GetAxisCount() const
{
	return axisbinds.GetSize();
}


int InputDevice::GetButtonCount() const
{
	return buttonbinds.GetSize();
}


float InputDevice::GetAxis(int index)
{
	BindIO* bind = FindBindIO(axisbinds,index);
	return bind ? bind->input->GetAxis(bind->src) : 0.0f;
}


int InputDevice::GetButton(int index)
{
	BindIO* bind = FindBindIO(buttonbinds,index);
	return bind ? bind->input->GetButton(bind->src) : 0;
}
