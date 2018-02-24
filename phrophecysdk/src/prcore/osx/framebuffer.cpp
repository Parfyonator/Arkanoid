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
		FrameBuffer OSX implementation

    Maybee this can be placed upon OpenGL somehow, who cares about framebuffers nowadays
    anyways? ;)

	revision history:
		December/25/2003 - Stefan Karlsson
*/
#include <prcore/framebuffer.hpp>

using namespace prcore;


FrameBuffer::FrameBuffer()
{
	// TODO
}


FrameBuffer::~FrameBuffer()
{
	// TODO
}


int FrameBuffer::GetModeCount() const
{
	// TODO
	return 0;
}


VideoMode FrameBuffer::GetMode(int index) const
{
	// TODO
	VideoMode mode;
	return mode;
}


bool FrameBuffer::OpenBuffer(WindowHandle handle)
{
	// TODO
	return false;
}


bool FrameBuffer::OpenBuffer(int width, int height, const String& name, uint32 windowstyle)
{
	// TODO
	return false;
}


bool FrameBuffer::OpenBuffer(int width, int height, int bits, const String& name, uint32 windowstyle)
{
	// TODO
	return false;
}


bool FrameBuffer::CloseBuffer(bool mainbreak)
{
	// TODO
	return false;
}


void FrameBuffer::SetPalette(const color32 palette[])
{
	// TODO
}


void FrameBuffer::BlitWrite(int x, int y, const Surface& source)
{
	// TODO
}


bool FrameBuffer::ResizeBuffer(int width, int height)
{
	// TODO
	return false;
}


void FrameBuffer::ClearBuffer(color32 color)
{
	// TODO
}


Surface* FrameBuffer::LockBuffer()
{
	// TODO
	return NULL;
}


void FrameBuffer::UnlockBuffer()
{
	// TODO
}


void FrameBuffer::PageFlip(bool retrace)
{
	// TODO
}
