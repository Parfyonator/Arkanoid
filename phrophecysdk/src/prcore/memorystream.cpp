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
		memorystream implementation

	revision history:
		Mar/19/2001 - Jukka Liimatta - initial revision
		Sep/17/2002 - Jukka Liimatta & Mikko Nurmi - added assert to ::Seek() method
*/
#include <cstddef>
#include <prcore/filestream.hpp>
#include <prcore/memorystream.hpp>

namespace prcore
{

	MemoryStream::MemoryStream()
	: msoffset(0)
	{
	}

	MemoryStream::MemoryStream(int size)
	: Array<uint8>(size), msoffset(0)
	{
		ResetIndex();
	}

	MemoryStream::MemoryStream(const uint8* data, int size)
	: Array<uint8>(data,size), msoffset(0)
	{
	}

	MemoryStream::MemoryStream(const String& filename)
	: msoffset(0)
	{
		FileStream file(filename);

		int filesize = file.GetSize();
		SetSize(filesize,false);

		uint8* data = GetArray();
		file.Read(data,filesize);
	}

	MemoryStream::~MemoryStream()
	{
	}

	int MemoryStream::Seek(int delta, SeekMode mode)
	{
		int size = GetSize();

		switch ( mode )
		{
			case START:		msoffset = delta; break;
			case CURRENT:	msoffset += delta; break;
			case END:		msoffset = size - delta; break;
		}

		// assert offset range
		assert( msoffset >= 0 && msoffset <= size );

		return msoffset;
	}

	int MemoryStream::Read(void* target, int bytes)
	{
		int size = GetSize();

		int overflow = msoffset + bytes - size;
		if ( overflow > 0 )
		{
			bytes -= overflow;
			if ( bytes < 1 ) 
				return 0;
		}

		uint8* data = GetArray() + msoffset;
		memcpy(target,data,bytes);

		msoffset += bytes;

		return bytes;
	}

	int MemoryStream::Write(const void* source, int bytes)
	{
		if ( bytes < 1 )
			return 0;

		int size = GetSize();
		int left = size - msoffset;

		if ( left < bytes )
		{
			// allocate more memory
			SetIndex(msoffset);
			int right = msoffset + bytes - size;
			PushBlock(right);
		}

		uint8* data = GetArray() + msoffset;
		memcpy(data,source,bytes);
		msoffset += bytes;

		return bytes;
	}

} // prcore
