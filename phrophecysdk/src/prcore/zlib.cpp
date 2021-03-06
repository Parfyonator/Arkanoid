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
		zlib entrypoint

	revision history:
		Mar/05/2003 - Jukka Liimatta - initial revision
*/
#include "../../src/extlib/zlib/zlib.h"
#include <prcore/exception.hpp>
#include <prcore/zlib.hpp>

using namespace prcore;


uint32 prcore::zlib_encode(uint8* dest, uint32 destlen, const uint8* source, uint32 sourcelen)
{
	uLong zd = static_cast<uLong>(destlen);

	int zv = ::compress(reinterpret_cast<Bytef*>(dest),&zd,
		reinterpret_cast<const Bytef*>(source),static_cast<uLong>(sourcelen));

	if ( zv != Z_OK )
		PRCORE_EXCEPTION("zlib compression failed.");

	return static_cast<uint32>(zd);
}


uint32 prcore::zlib_decode(uint8* dest, uint32 destlen, const uint8* source, uint32 sourcelen)
{
	uLong zd = static_cast<uLong>(destlen);

	int zv = ::uncompress(reinterpret_cast<Bytef*>(dest),&zd,
		reinterpret_cast<const Bytef*>(source),static_cast<uLong>(sourcelen));

	if ( zv != Z_OK )
		PRCORE_EXCEPTION("zlib uncompression failed.");

	return static_cast<uint32>(zd);
}
