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
		alpha blending

	revision history:
		Feb/02/1999 - Jukka Liimatta - initial revision
		Feb/05/2001 - Jukka Liimatta - renaissance build
		Sep/08/2001 - Jukka Liimatta - improved c++ innerloop framework
		Sep/20/2003 - Jukka Liimatta - fixed extern "C" to work with broader range of compilers

*/
#include <prcore/cpu.hpp>
#include <prcore/alphablend.hpp>

using namespace prcore;

namespace
{

	// =================================================
	// innerloop macros
	// =================================================

	// constant alpha	
	uint32 alpha0, invalpha0;
	uint32 alpha1, invalpha1;


	inline uint32 AddColor(uint32 u, uint32 v)
	{
		// rgb components are saturated, alpha wraps around
		uint32 x = (u & 0xfefefefe) + (v & 0xfefefefe);
		return x | ((x & 0x01010101) - ((x & 0x01010101) >> 8));
	}

	inline uint32 InvColor(uint32 u)
	{
		return u ^ 0xffffffff;
	}

	inline uint32 InvAlpha(uint32 u)
	{
		return u ^ 0x000000ff;
	}

	inline uint32 SquareColor(uint32 u)
	{
		uint32 s = u & 0xff;
		uint32 lsb = u & 0xff00;
		lsb = (((lsb * lsb) >> 16) & 0xff00) | ((s * s) >> 8);
		u >>= 16;
		s = u & 0xff;
		uint32 msb = u & 0xff00;
		msb = ((msb * msb) & 0xff000000) | (((s * s) << 8) & 0x00ff0000);
		return msb | lsb;
	}
	
	inline uint32 ScaleColor(uint32 u, uint32 s)
	{
		uint32 lsb = (((u & 0xff) * (s & 0xff)) >> 8) | ((((u & 0xff00) * (s & 0xff00)) >> 16) & 0xff00);
		u >>= 16;
		s >>= 16;
		uint32 msb = ((((u & 0xff) * (s & 0xff)) << 8) & 0x00ff0000) | (((u & 0xff00) * (s & 0xff00)) & 0xff000000);
		return msb | lsb;
	}

	inline uint32 ScaleInvColor(uint32 u, uint32 s)
	{
		return ScaleColor(u,s^0xffffffff);
	}
	
	inline uint32 ScaleMult(uint32 u, uint32 s)
	{
		uint32 lsb = (((u & 0x00ff00ff) * s) >> 8) & 0x00ff00ff;
		uint32 msb = (((u & 0xff00ff00) >> 8) * s) & 0xff00ff00;
		return msb | lsb;
	}

	inline uint32 GetAlpha0(uint32 u)
	{
		return ((u >> 24) * alpha0) >> 8;
	}

	inline uint32 GetAlpha1(uint32 u)
	{
		return ((u >> 24) * alpha1) >> 8;
	}

	inline uint32 GetInvAlpha0(uint32 u)
	{
		return ((u >> 24) * invalpha0) >> 8;
	}

	inline uint32 GetInvAlpha1(uint32 u)
	{
		return ((u >> 24) * invalpha1) >> 8;
	}
	
	inline uint32 ScaleAlpha0(uint32 u, uint32 s)
	{
		return ScaleMult(u,GetAlpha0(s));
	}

	inline uint32 ScaleAlpha1(uint32 u, uint32 s)
	{
		return ScaleMult(u,GetAlpha1(s));
	}

	inline uint32 ScaleInvAlpha0(uint32 u, uint32 s)
	{
		return ScaleMult(u,GetInvAlpha0(s));
	}

	inline uint32 ScaleInvAlpha1(uint32 u, uint32 s)
	{
		return ScaleMult(u,GetInvAlpha1(s));
	}


	// =================================================
	// innerloops
	// =================================================

	void Blend_Zero_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		memset(dest,0,sizeof(uint32)*count);
	}

	void Blend_Zero_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		memcpy(dest,sr1,sizeof(uint32)*count);
	}

	void Blend_Zero_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleColor(*sr1++,*sr0++);
		}
		while ( --count );
	}

	void Blend_Zero_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleInvColor(*sr1++,*sr0++);
		}
		 while ( --count );
	}

	void Blend_Zero_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleAlpha0(*sr1++,*sr0++);
		}
		while ( --count );
	}

	void Blend_Zero_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleInvAlpha0(*sr1++,*sr0++);
		}
		while ( --count );
	}

	void Blend_Zero_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = SquareColor(*sr1++);
		}
		while ( --count );
	}

	void Blend_Zero_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = ScaleInvColor(c1,c1);
		}
		while ( --count );
	}

	void Blend_Zero_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = ScaleAlpha1(c1,c1);
		}
		while ( --count );
	}

	void Blend_Zero_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = ScaleInvAlpha1(c1,c1);
		}
		while ( --count );
	}

	void Blend_One_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		memcpy(dest,sr0,sizeof(uint32)*count);
	}

	void Blend_One_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = AddColor(*sr0++,*sr1++);
		}
		while ( --count );
	}

	void Blend_One_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(c0,ScaleColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_One_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(c0,ScaleInvColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_One_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(c0,ScaleAlpha0(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_One_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(c0,ScaleInvAlpha0(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_One_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = AddColor(*sr0++,SquareColor(*sr1++));
		}
		while ( --count );
	}

	void Blend_One_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(*sr0++,ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_One_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(*sr0++,ScaleAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_One_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(*sr0++,ScaleInvAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcColor_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = SquareColor(*sr0++);
		}
		while ( --count );
	}

	void Blend_SrcColor_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = AddColor(SquareColor(*sr0++),*sr1++);
		}
		while ( --count );
	}

	void Blend_SrcColor_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(SquareColor(c0),ScaleColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcColor_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(SquareColor(c0),ScaleInvColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcColor_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(SquareColor(c0),ScaleAlpha0(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcColor_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(SquareColor(c0),ScaleInvAlpha0(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcColor_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = AddColor(SquareColor(*sr0++),SquareColor(*sr1++));
		}
		while ( --count );
	}

	void Blend_SrcColor_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(SquareColor(*sr0++),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcColor_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(SquareColor(*sr0++),ScaleAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcColor_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(SquareColor(*sr0++),ScaleInvAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = ScaleInvColor(c0,c0);
		}
		while ( --count );
	}

	void Blend_SrcInvColor_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			AddColor(ScaleInvColor(c0,c0),*sr1++);
		}
		while ( --count );
	}

	void Blend_SrcInvColor_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleInvColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleAlpha0(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleInvAlpha0(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),SquareColor(*sr1++));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcInvColor_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c0),ScaleInvAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = ScaleAlpha0(c0,c0);
		}
		while ( --count );
	}

	void Blend_SrcAlpha_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleAlpha0(c0,c0),*sr1++);
		}
		while ( --count );
	}

	void Blend_SrcAlpha_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleAlpha0(c0,c0),ScaleColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleAlpha0(c0,c0),ScaleInvColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 a0 = GetAlpha0(c0);
			*dest++ = AddColor(ScaleMult(c0,a0),ScaleMult(*sr1++,a0));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 a0 = GetAlpha0(c0);
			*dest++ = AddColor(ScaleMult(c0,a0),ScaleMult(*sr1++,InvAlpha(a0)));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleMult(c0,GetAlpha0(c0)),SquareColor(*sr1++));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleMult(c0,GetAlpha0(c0)),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleMult(c0,GetAlpha0(c0)),ScaleMult(c1,GetAlpha1(c1)));
		}
		while ( --count );
	}

	void Blend_SrcAlpha_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleMult(c0,GetAlpha0(c0)),ScaleMult(c1,GetInvAlpha1(c1)));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = ScaleMult(c0,GetInvAlpha0(c0));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			AddColor(ScaleMult(c0,GetInvAlpha0(c0)),*sr1++);
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleMult(c0,GetInvAlpha0(c0)),ScaleColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleMult(c0,GetInvAlpha0(c0)),ScaleInvColor(*sr1++,c0));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 a0 = GetAlpha0(c0);
			*dest++ = AddColor(ScaleMult(c0,InvAlpha(a0)),ScaleMult(*sr1++,a0));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 ia0 = GetInvAlpha0(c0);
			*dest++ = AddColor(ScaleMult(c0,ia0),ScaleMult(*sr1++,ia0));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			*dest++ = AddColor(ScaleInvAlpha0(c0,c0),SquareColor(*sr1++));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha0(c0,c0),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha0(c0,c0),ScaleAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_SrcInvAlpha_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha0(c0,c0),ScaleInvAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestColor_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleColor(*sr0++,*sr1++);
		}
		while ( --count );
	}

	void Blend_DestColor_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(*sr0++,c1),c1);
		}
		while ( --count );
	}

	void Blend_DestColor_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(c0,c1),ScaleColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestColor_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(c0,c1),ScaleInvColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestColor_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(c0,c1),ScaleAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestColor_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(c0,c1),ScaleInvAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestColor_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(*sr0++,c1),SquareColor(c1));
		}
		while ( --count );
	}

	void Blend_DestColor_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(*sr0++,c1),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestColor_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(*sr0++,c1),ScaleAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestColor_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleColor(*sr0++,c1),ScaleInvAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestInvColor_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleInvColor(*sr0++,*sr1++);
		}
		while ( --count );
	}

	void Blend_DestInvColor_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(*sr0++,c1),c1);
		}
		while ( --count );
	}

	void Blend_DestInvColor_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c1),ScaleColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvColor_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c1),ScaleInvColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvColor_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c1),ScaleAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvColor_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(c0,c1),ScaleInvAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvColor_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(*sr0++,c1),SquareColor(c1));
		}
		while ( --count );
	}

	void Blend_DestInvColor_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			uint32 i1 = InvColor(c1);
			*dest++ = AddColor(ScaleColor(*sr0++,i1),ScaleColor(c1,i1));
		}
		while ( --count );
	}

	void Blend_DestInvColor_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(*sr0++,c1),ScaleAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestInvColor_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvColor(*sr0++,c1),ScaleInvAlpha1(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestAlpha_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleAlpha1(*sr0++,*sr1++);
		}
		while ( --count );
	}

	void Blend_DestAlpha_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(*sr0++,c1),c1);
		}
		while ( --count );
	}

	void Blend_DestAlpha_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(c0,c1),ScaleColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestAlpha_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(c0,c1),ScaleInvColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestAlpha_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(c0,c1),ScaleAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestAlpha_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(c0,c1),ScaleInvAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestAlpha_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(*sr0++,c1),SquareColor(c1));
		}
		while ( --count );
	}

	void Blend_DestAlpha_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleAlpha1(*sr0++,c1),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestAlpha_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			uint32 a1 = GetAlpha1(c1);
			*dest++ = AddColor(ScaleMult(*sr0++,a1),ScaleMult(c1,a1));
		}
		while ( --count );
	}

	void Blend_DestAlpha_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			uint32 a1 = GetAlpha1(c1);
			*dest++ = AddColor(ScaleMult(*sr0++,a1),ScaleMult(c1,InvAlpha(a1)));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_Zero(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			*dest++ = ScaleInvAlpha1(*sr0++,*sr1++);
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_One(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(*sr0++,c1),c1);
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_SrcColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(c0,c1),ScaleColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_SrcInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(c0,c1),ScaleInvColor(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_SrcAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(c0,c1),ScaleAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_SrcInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c0 = *sr0++;
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(c0,c1),ScaleInvAlpha0(c1,c0));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_DestColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(*sr0++,c1),SquareColor(c1));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_DestInvColor(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			*dest++ = AddColor(ScaleInvAlpha1(*sr0++,c1),ScaleInvColor(c1,c1));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_DestAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			uint32 a1 = GetAlpha1(c1);
			*dest++ = AddColor(ScaleMult(*sr0++,InvAlpha(a1)),ScaleMult(c1,a1));
		}
		while ( --count );
	}

	void Blend_DestInvAlpha_DestInvAlpha(uint32* dest, const uint32* sr0, const uint32* sr1, int count)
	{
		do
		{
			uint32 c1 = *sr1++;
			uint32 ia1 = GetInvAlpha1(c1);
			*dest++ = AddColor(ScaleMult(*sr0++,ia1),ScaleMult(c1,ia1));
		}
		while ( --count );
	}


	// =================================================
	// C++ implementation table
	// =================================================

	typedef void (*BlendFunc)(uint32*, const uint32*, const uint32*, int);


	BlendFunc FuncTableCPP[] =
	{
		Blend_Zero_Zero,
		Blend_Zero_One,
		Blend_Zero_SrcColor,
		Blend_Zero_SrcInvColor,
		Blend_Zero_SrcAlpha,
		Blend_Zero_SrcInvAlpha,
		Blend_Zero_DestColor,
		Blend_Zero_DestInvColor,
		Blend_Zero_DestAlpha,
		Blend_Zero_DestInvAlpha,
		Blend_One_Zero,
		Blend_One_One,
		Blend_One_SrcColor,
		Blend_One_SrcInvColor,
		Blend_One_SrcAlpha,
		Blend_One_SrcInvAlpha,
		Blend_One_DestColor,
		Blend_One_DestInvColor,
		Blend_One_DestAlpha,
		Blend_One_DestInvAlpha,
		Blend_SrcColor_Zero,
		Blend_SrcColor_One,
		Blend_SrcColor_SrcColor,
		Blend_SrcColor_SrcInvColor,
		Blend_SrcColor_SrcAlpha,
		Blend_SrcColor_SrcInvAlpha,
		Blend_SrcColor_DestColor,
		Blend_SrcColor_DestInvColor,
		Blend_SrcColor_DestAlpha,
		Blend_SrcColor_DestInvAlpha,
		Blend_SrcInvColor_Zero,
		Blend_SrcInvColor_One,
		Blend_SrcInvColor_SrcColor,
		Blend_SrcInvColor_SrcInvColor,
		Blend_SrcInvColor_SrcAlpha,
		Blend_SrcInvColor_SrcInvAlpha,
		Blend_SrcInvColor_DestColor,
		Blend_SrcInvColor_DestInvColor,
		Blend_SrcInvColor_DestAlpha,
		Blend_SrcInvColor_DestInvAlpha,
		Blend_SrcAlpha_Zero,
		Blend_SrcAlpha_One,
		Blend_SrcAlpha_SrcColor,
		Blend_SrcAlpha_SrcInvColor,
		Blend_SrcAlpha_SrcAlpha,
		Blend_SrcAlpha_SrcInvAlpha,
		Blend_SrcAlpha_DestColor,
		Blend_SrcAlpha_DestInvColor,
		Blend_SrcAlpha_DestAlpha,
		Blend_SrcAlpha_DestInvAlpha,
		Blend_SrcInvAlpha_Zero,
		Blend_SrcInvAlpha_One,
		Blend_SrcInvAlpha_SrcColor,
		Blend_SrcInvAlpha_SrcInvColor,
		Blend_SrcInvAlpha_SrcAlpha,
		Blend_SrcInvAlpha_SrcInvAlpha,
		Blend_SrcInvAlpha_DestColor,
		Blend_SrcInvAlpha_DestInvColor,
		Blend_SrcInvAlpha_DestAlpha,
		Blend_SrcInvAlpha_DestInvAlpha,
		Blend_DestColor_Zero,
		Blend_DestColor_One,
		Blend_DestColor_SrcColor,
		Blend_DestColor_SrcInvColor,
		Blend_DestColor_SrcAlpha,
		Blend_DestColor_SrcInvAlpha,
		Blend_DestColor_DestColor,
		Blend_DestColor_DestInvColor,
		Blend_DestColor_DestAlpha,
		Blend_DestColor_DestInvAlpha,
		Blend_DestInvColor_Zero,
		Blend_DestInvColor_One,
		Blend_DestInvColor_SrcColor,
		Blend_DestInvColor_SrcInvColor,
		Blend_DestInvColor_SrcAlpha,
		Blend_DestInvColor_SrcInvAlpha,
		Blend_DestInvColor_DestColor,
		Blend_DestInvColor_DestInvColor,
		Blend_DestInvColor_DestAlpha,
		Blend_DestInvColor_DestInvAlpha,
		Blend_DestAlpha_Zero,
		Blend_DestAlpha_One,
		Blend_DestAlpha_SrcColor,
		Blend_DestAlpha_SrcInvColor,
		Blend_DestAlpha_SrcAlpha,
		Blend_DestAlpha_SrcInvAlpha,
		Blend_DestAlpha_DestColor,
		Blend_DestAlpha_DestInvColor,
		Blend_DestAlpha_DestAlpha,
		Blend_DestAlpha_DestInvAlpha,
		Blend_DestInvAlpha_Zero,
		Blend_DestInvAlpha_One,
		Blend_DestInvAlpha_SrcColor,
		Blend_DestInvAlpha_SrcInvColor,
		Blend_DestInvAlpha_SrcAlpha,
		Blend_DestInvAlpha_SrcInvAlpha,
		Blend_DestInvAlpha_DestColor,
		Blend_DestInvAlpha_DestInvColor,
		Blend_DestInvAlpha_DestAlpha,
		Blend_DestInvAlpha_DestInvAlpha,
	};


	// =================================================
	// X86-MMX implementation table
	// =================================================

	#ifdef PRCORE_X86_SIMD

		extern "C"
		{
			#if defined(__INTEL_COMPILER)

			uint32			mmx_mult0;
			uint32			mmx_mult1;
			int*			mmx_alpha;

			#else

			extern uint32	mmx_mult0;
			extern uint32	mmx_mult1;
			extern int*		mmx_alpha;

			#endif
		};

		BlendFunc* FuncTableMMX = reinterpret_cast<BlendFunc*>(&mmx_alpha);

	#endif // PRCORE_X86_SIMD

	// =================================================
	// setup
	// =================================================

	BlendFunc* InnerTable = NULL;
	BlendFunc BlendFuncPtr = NULL;


	inline int AlphaFuncOffset(AlphaFunc af0, AlphaFunc af1)
	{
		return af0 * 10 + af1;
	}


	void InitializeInner()
	{
		// already initialized?
		if ( InnerTable )
			return;

		// default: generic c++ innerloops
		InnerTable = FuncTableCPP;

		#ifdef PRCORE_X86_SIMD

			// use MMX innerloops if possible
			if ( IsMMX() ) 
				InnerTable = FuncTableMMX;

		#endif // PRCORE_X86_SIMD

		// default alphafunc
		BlendFuncPtr = InnerTable[ AlphaFuncOffset(ONE,ZERO) ];
		
		// default constant alpha
		SetConstantAlpha(0xff,0xff);
	}
	
} // namespace

// =================================================
// AlphaBlend API
// =================================================

void prcore::SetConstantAlpha(uint8 mult0, uint8 mult1)
{
	if ( !InnerTable )
		InitializeInner();

	if ( InnerTable == FuncTableCPP )
	{
		alpha0 = mult0;
		alpha1 = mult1;
		invalpha0 = mult0 ^ 0xff;
		invalpha1 = mult1 ^ 0xff;
	}
	else
	{
		#ifdef PRCORE_X86_SIMD
		
			uint32 m0 = (mult0 << 16) | mult0;
			uint32 m1 = (mult1 << 16) | mult1;
			uint32* a = &mmx_mult0;
			uint32* b = &mmx_mult1;
			a[0] = m0; a[1] = m0;
			b[0] = m1; b[1] = m1;

		#endif // PRCORE_X86_SIMD
	}
}

void prcore::SetAlphaFunc(AlphaFunc func0, AlphaFunc func1)
{
	if ( !InnerTable )
		InitializeInner();

	BlendFuncPtr = InnerTable[ AlphaFuncOffset(func0,func1) ];
}

void prcore::AlphaBlend(uint32* dest, uint32* source0, uint32* source1, int count)
{
	if ( !InnerTable )
		InitializeInner();

	BlendFuncPtr(
		reinterpret_cast<uint32*>(dest),
		reinterpret_cast<uint32*>(source0),
		reinterpret_cast<uint32*>(source1),
		count);
}

void prcore::AlphaBlend(int x, int y, Surface& dest, const Surface& source)
{
	// assert

	assert( dest.GetImage() != NULL );
	assert( source.GetImage() != NULL );

	// verify pixelformat (as number of bits, atleast)

	if ( dest.GetPixelFormat().GetBits() != 32 ) return;
	if ( source.GetPixelFormat().GetBits() != 32 ) return;

	if ( !InnerTable )
		InitializeInner();

	// clip

	Rect rect;
	if ( !rect.Intersect(
		dest.GetRect(),
		Rect(x,y,source.GetWidth(),source.GetHeight()) ))
		return;

	int sx = rect.x - x;
	int sy = rect.y - y;

	// setup surfaces

	int bpp0 = dest.GetPixelFormat().GetBytes();
	int bpp1 = source.GetPixelFormat().GetBytes();
	int pitch0 = dest.GetPitch();
	int pitch1 = source.GetPitch();
	uint8* image0 = dest.GetImage() + rect.y * pitch0 + rect.x * bpp0;
	uint8* image1 = source.GetImage() + sy * pitch1 + sx * bpp1;

	// alphablend

	for ( int i=0; i<rect.height; ++i )
	{
		BlendFuncPtr(
			reinterpret_cast<uint32*>(image0),
			reinterpret_cast<uint32*>(image1),
			reinterpret_cast<uint32*>(image0),
			rect.width );

		image0 += pitch0;
		image1 += pitch1;
	}
}

void prcore::AlphaBlendVolatile(Surface& dest, const Surface& source0, const Surface& source1)
{
	// ======================================
	// note! this routine does NOT clip, 
	// implemented for convenience reasons
	// ======================================

	if ( !InnerTable )
		InitializeInner();

	// get surface pointers	

	uint8* imaged = dest.GetImage();
	uint8* image0 = source0.GetImage();
	uint8* image1 = source1.GetImage();

	// get pitch

	int pitchd = dest.GetPitch();
	int pitch0 = source0.GetPitch();
	int pitch1 = source1.GetPitch();
	
	// dimensions

	int width  = dest.GetWidth();
	int height = dest.GetHeight();

	// alphablend

	for ( int i=0; i<height; ++i )
	{
		BlendFuncPtr(
			reinterpret_cast<uint32*>(imaged),
			reinterpret_cast<uint32*>(image0),
			reinterpret_cast<uint32*>(image1),
			width);
		imaged += pitchd;
		image0 += pitch0;
		image1 += pitch1;
	}
}
