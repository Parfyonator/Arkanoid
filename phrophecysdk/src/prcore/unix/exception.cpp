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
	Exception Linux implementation
		
	revision history:
	??/??/2001 - Edmond Cote - initial implementation
	09/06/2002 - Edmond Cote - upgraded implementation
   	Apr/08/2003 - Jukka Liimatta - upgraded UNIX implementation
*/
#include <iostream>
#include <prcore/exception.hpp>

using namespace prcore;


Exception::Exception()
: mMethod("Unknown."),mMessage("N/A"),mFilename("Unknown."),mLinenr(-1)
{
}


Exception::Exception(const String& method, const String& message, const String& filename, int linenr)
: mMethod(method),mMessage(message),mFilename(filename),mLinenr(linenr)
{
}


Exception::Exception(const Exception& e)
{
	*this = e;
}


void Exception::operator = (const Exception& e)
{
	mFilename = e.mFilename;
	mMethod   = e.mMethod;
	mMessage  = e.mMessage;
	mLinenr   = e.mLinenr;
}


Exception::~Exception()
{
}


bool Exception::SetLogo(const Bitmap& logo)
{
	// TODO: not supported in ascii mode ;-)
	return true;
}


void Exception::DrawDialog()
{
	std::cout <<
	"Prophecy / Multimedia SDK Exception Handler\n" <<
	"-------------------------------------------\n" <<
	"Filename: " << mFilename << "\n" <<
	"Method:   " << mMethod   << "\n" <<
	"Linenr:   " << mLinenr   << "\n" <<
	"Message:  " << mMessage  << std::endl;
}


