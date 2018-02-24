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
		filename implementation

	revision history:
		Jan/24/2001 - Jukka Liimatta - initial revision/renaissance build
		Feb/21/2003 - Jukka Liimatta - rewrote functions to use prcore::String2<> instead of const char*
*/
#include <cstring>
#include <prcore/filename.hpp>

using namespace prcore;


String prcore::GetFilenameEXT(const String& filename)
{
	int length = filename.GetLength();
	const char* text = filename + length;

	while ( length-- )
	{
		if ( *--text == '.' )
			return String(text + 1);
	}

	return String();
}


String prcore::GetFilenamePATH(const String& filename)
{
	int length = filename.GetLength();
	const char* text = filename + length;

	while ( length-- )
	{
		char v = *--text;
		if ( v == '\\' || v == '/' || v == ':' )
		{
			int size = text - static_cast<const char*>(filename) + 1;
			return filename.SubString(0,size);
		}
	}

	return String();
}


String prcore::GetFilenameNOEXT(const String& filename)
{
	int length = filename.GetLength();
	const char* text = filename + length;

	while ( length-- )
	{
		if ( *--text == '.' )
		{
			int size = text - static_cast<const char*>(filename);
			return filename.SubString(0,size);
		}
	}

	return filename;
}


String prcore::GetFilenameNOPATH(const String& filename)
{
	int length = filename.GetLength();
	const char* text = filename + length;

	while ( length-- )
	{
		char v = *--text;
		if ( v == '\\' || v == '/' || v == ':' )
			return String(text + 1);
	}

	return filename;
}
