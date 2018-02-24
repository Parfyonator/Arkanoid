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
		cpu capabilities

	revision history:
		May/10/1998 - Jukka Liimatta - initial revision
		Jan/01/2002 - Daniel Kolakowski - added 3DNow!+ & MMX+ detection
		Jan/12/2003 - Jukka Liimatta - rewrote the initialization
*/
#include <cstring>
#include <prcore/configure.hpp>
#include <prcore/cpu.hpp>

using namespace prcore;

// =================================================
// initialize cpu module
// =================================================

#ifdef PRCORE_X86_SIMD

	extern "C" uint32 x86_cpu_caps();
	extern "C" void*  x86_amd_memcpy(void* dest, const void* src, size_t count);

#endif

void* (*prcore::simd_memcpy)(void* dest, const void* src, size_t count);

namespace {

	enum {
		CPU_MMX       = 0x00000001,
		CPU_MMX_EXT   = 0x00000002,
		CPU_3DNOW     = 0x00000004,
		CPU_3DNOW_EXT = 0x00000008,
		CPU_SSE       = 0x00000010,
		CPU_SSE2      = 0x00000020,
	};

	class initcpu
	{
		public:

		uint32 caps;
	
		initcpu()
		: caps(0)
		{
			simd_memcpy = memcpy;

			#ifdef PRCORE_X86_SIMD

			caps = x86_cpu_caps();
			if ( caps & CPU_3DNOW_EXT )
				simd_memcpy = x86_amd_memcpy;
			
			#endif
		}
	
		~initcpu()
		{
		}
	};

	// cpu object
	initcpu thecpu;

} // namespace

// =================================================
// API
// =================================================

bool prcore::IsMMX()
{
	return (thecpu.caps & CPU_MMX) != 0;
}

bool prcore::IsMMX_EXT()
{
	return (thecpu.caps & CPU_MMX_EXT) != 0;
}

bool prcore::Is3DNOW()
{
	return (thecpu.caps & CPU_3DNOW) != 0;
}

bool prcore::Is3DNOW_EXT()
{
	return (thecpu.caps & CPU_3DNOW_EXT) != 0;
}

bool prcore::IsSSE()
{
	return (thecpu.caps & CPU_SSE) != 0;
}

bool prcore::IsSSE2()
{
	return (thecpu.caps & CPU_SSE2) != 0;
}
