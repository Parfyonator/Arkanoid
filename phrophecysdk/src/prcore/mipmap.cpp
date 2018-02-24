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
		mipmap generation

	revision history:
		Sep/15/1999 - Jukka Liimatta - initial revision
		Oct/26/2000 - Jukka Liimatta - HighQuality 8bit indexed mipping
		Dec/12/2002 - Jukka Liimatta - revised the code, template alphafilter implementation
		              Timo Saarinen - alpha filtering prototype

	TODO:
	- alphafilter to indexed and intensity pixelformats
*/
#include <prcore/configure.hpp>
#include <prcore/float754.hpp>
#include <prcore/mipmap.hpp>
#include <prcore/exception.hpp>

using namespace prcore;


// =================================================
// mipinfo
// =================================================

typedef void (*MipFunc)(const struct MipInfo&);


struct MipInfo
{
	uint8*		dest;
	uint8*		src;
	int			width,height,pitch;
	uint32		rmask,gmask,bmask,amask,imask;
	float		rcp;
	Color32*	palette;
	MipFunc		func;

	bool		alphafilter;
	uint32		ashift,abits;
	float		aweight;
	float		arcp;

	float GetAlphaWeight(uint32 color) const
	{
		return static_cast<float>(color & amask) * aweight;
	}

	void UpdateAlphaFilterARGB(float& a, float& r, float& g, float& b, uint32 color) const
	{
		if ( alphafilter )
		{
			float aw = GetAlphaWeight(color);
			a += aw;
			r += static_cast<float>(color & rmask) * aw;
			g += static_cast<float>(color & gmask) * aw;
			b += static_cast<float>(color & bmask) * aw;
		}
		else
		{
			a += static_cast<float>(color & amask);
			r += static_cast<float>(color & rmask);
			g += static_cast<float>(color & gmask);
			b += static_cast<float>(color & bmask);
		}
	}

	uint32 ComputeAlphaFilterARGB(float a, float r, float g, float b) const
	{
		float m0;
		float m1;
		if ( alphafilter )
		{
			if ( a < 0.0001f )
			{
				m0 = 0;
				m1 = 0;
			}
			else
			{
				m0 = arcp;
				m1 = 1.0f / a;
			}
		}
		else
		{
			m0 = rcp;
			m1 = rcp;
		}

		uint32 alpha = static_cast<uint32>(a * m0) & amask;
		uint32 red   = static_cast<uint32>(r * m1) & rmask;
		uint32 green = static_cast<uint32>(g * m1) & gmask;
		uint32 blue  = static_cast<uint32>(b * m1) & bmask;

		return alpha | red | green | blue;
	}

	int FindPaletteIndex(int red, int green, int blue) const
	{
		// brute force search

		int best = 0xfffffff;
		int index = 0;
		for ( int i=0; i<256; ++i )
		{
			int b = palette[i].b - blue;
			int g = palette[i].g - green;
			int r = palette[i].r - red;
			int dist = b*b + g*g + r*r;

			if ( dist < best )
			{
				best = dist;
				index = i;
			}
		}
		return index;
	}
};


// =================================================
// innerloops
// =================================================

static void MipIndexed8(const MipInfo& info)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	uint8* src = info.src;
	int adder = info.pitch - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			Color32& color = info.palette[*src++];
			b += color.b;
			g += color.g;
			r += color.r;
		}
		src += adder;
	}

	int red   = static_cast<int>(r * info.rcp);
	int green = static_cast<int>(g * info.rcp);
	int blue  = static_cast<int>(b * info.rcp);
	int index = info.FindPaletteIndex(red,green,blue);

	info.dest[0] = index;
}


static void MipIndexed16(const MipInfo& info)
{
	info.dest[0] = info.src[0];
	info.dest[1] = info.src[1];
}


static void MipIndexed24(const MipInfo& info)
{
	info.dest[0] = info.src[0];
	info.dest[1] = info.src[1];
	info.dest[2] = info.src[2];
}


static void MipIndexed32(const MipInfo& info)
{
	info.dest[0] = info.src[0];
	info.dest[1] = info.src[1];
	info.dest[2] = info.src[2];
	info.dest[3] = info.src[3];
}


static void MipIntensity8(const MipInfo& info)
{
	float i = 0.0f;
	float a = 0.0f;

	uint8* src = info.src;
	int adder = info.pitch - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			uint32 color = *src++;
			i += static_cast<float>(color & info.imask);
			a += static_cast<float>(color & info.amask);
		}
		src += adder;
	}

	uint32 luma  = static_cast<uint32>(i * info.rcp) & info.imask;
	uint32 alpha = static_cast<uint32>(a * info.rcp) & info.amask;

	info.dest[0] = static_cast<uint8>(alpha | luma);
}


static void MipIntensity16(const MipInfo& info)
{
	float i = 0.0f;
	float a = 0.0f;

	uint16* src = reinterpret_cast<uint16*>(info.src);
	int adder = info.pitch / 2 - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			uint32 color = *src++;
			i += static_cast<float>(color & info.imask);
			a += static_cast<float>(color & info.amask);
		}
		src += adder;
	}

	uint32 luma  = static_cast<uint32>(i * info.rcp) & info.imask;
	uint32 alpha = static_cast<uint32>(a * info.rcp) & info.amask;

	uint16* dest = reinterpret_cast<uint16*>(info.dest);
	dest[0] = alpha | luma;
}


static void MipIntensity24(const MipInfo& info)
{
	float i = 0.0f;
	float a = 0.0f;

	uint8* src = info.src;
	int adder = info.pitch - info.width * 3;

	uint32 color;
	uint8* pc = reinterpret_cast<uint8*>(&color);

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			pc[0] = src[0];
			pc[1] = src[1];
			pc[2] = src[2];
			src += 3;

			i += static_cast<float>(color & info.imask);
			a += static_cast<float>(color & info.amask);
		}
		src += adder;
	}

	uint32 luma  = static_cast<uint32>(i * info.rcp) & info.imask;
	uint32 alpha = static_cast<uint32>(a * info.rcp) & info.amask;

	color = alpha | luma;
	info.dest[0] = pc[0];
	info.dest[1] = pc[1];
	info.dest[2] = pc[2];
}


static void MipIntensity32(const MipInfo& info)
{
	float i = 0.0f;
	float a = 0.0f;

	uint32* src = reinterpret_cast<uint32*>(info.src);
	int adder = info.pitch / 4 - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			uint32 color = *src++;
			i += static_cast<float>(color & info.imask);
			a += static_cast<float>(color & info.amask);
		}
		src += adder;
	}

	uint32 luma  = static_cast<uint32>(i * info.rcp) & info.imask;
	uint32 alpha = static_cast<uint32>(a * info.rcp) & info.amask;

	uint32* dest = reinterpret_cast<uint32*>(info.dest);
	dest[0] = alpha | luma;
}


static void MipDirect8(const MipInfo& info)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;

	uint8* src = info.src;
	int adder = info.pitch - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			uint32 color = *src++;
			info.UpdateAlphaFilterARGB(a,r,g,b,color);
		}
		src += adder;
	}
	
	info.dest[0] = static_cast<uint8>(info.ComputeAlphaFilterARGB(a,r,g,b));
}


static void MipDirect16(const MipInfo& info)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;

	uint16* src = reinterpret_cast<uint16*>(info.src);
	int adder = info.pitch / 2 - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			uint32 color = *src++;
			info.UpdateAlphaFilterARGB(a,r,g,b,color);
		}
		src += adder;
	}

	uint16* dest = reinterpret_cast<uint16*>(info.dest);
	dest[0] = static_cast<uint16>(info.ComputeAlphaFilterARGB(a,r,g,b));
}


static void MipDirect24(const MipInfo& info)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;

	uint8* src = info.src;
	int adder = info.pitch - info.width * 3;

	uint32 color;
	uint8* pc = reinterpret_cast<uint8*>(&color);

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			pc[0] = src[0];
			pc[1] = src[1];
			pc[2] = src[2];
			src += 3;
			info.UpdateAlphaFilterARGB(a,r,g,b,color);
		}
		src += adder;
	}

	color = static_cast<uint32>(info.ComputeAlphaFilterARGB(a,r,g,b));
	info.dest[0] = pc[0];
	info.dest[1] = pc[1];
	info.dest[2] = pc[2];
}


static void MipDirect32(const MipInfo& info)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;

	uint32* src = reinterpret_cast<uint32*>(info.src);
	int adder = info.pitch / 4 - info.width;

	for ( int y=0; y<info.height; ++y )
	{
		int count = info.width;
		while ( count-- )
		{
			uint32 color = *src++;
			info.UpdateAlphaFilterARGB(a,r,g,b,color);
		}
		src += adder;
	}

	uint32* dest = reinterpret_cast<uint32*>(info.dest);
	dest[0] = static_cast<uint32>(info.ComputeAlphaFilterARGB(a,r,g,b));
}


// =================================================
// utils
// =================================================

static inline int SubLevel(int lod, int sublevel)
{
	int level = lod - sublevel;
	return level < 0 ? 0 : level;
}


static MipFunc SelectInnerloop(const PixelFormat& format)
{
	// argb
	if ( format.IsDirectColor() )
	{
		switch ( format.GetBits() )
		{
			case 8:  return MipDirect8;
			case 16: return MipDirect16;
			case 24: return MipDirect24;
			case 32: return MipDirect32;
		}
	}
	
	// indexed
	if ( format.IsIndexed() )
	{
		switch ( format.GetBits() )
		{
			case 8:  return MipIndexed8;
			case 16: return MipIndexed16;
			case 24: return MipIndexed24;
			case 32: return MipIndexed32;
		}
	}

	// grayscale
	switch ( format.GetBits() )
	{
		case 8:  return MipIntensity8;
		case 16: return MipIntensity16;
		case 24: return MipIntensity24;
		case 32: return MipIntensity32;
	}

	PRCORE_EXCEPTION("Unknown pixelformat.");
	return NULL;
}


static void DrawMipLevel(Bitmap& mipmap, const Surface& surface, int tilex, int tiley, int sublevel, MipInfo& info)
{

	// setup mipinfo
	info.width  = surface.GetWidth() / tilex;
	info.height = surface.GetHeight() / tiley;
	info.pitch  = surface.GetPitch();
	info.rcp    = 1.0f / static_cast<float>(info.width * info.height);

	if ( info.alphafilter )
	{
		float n = static_cast<float>(1 << (info.ashift - info.abits));
		info.arcp = info.rcp * static_cast<float>((1 << info.abits) - 1) * n;
	}

	int vbpp = surface.GetPixelFormat().GetBytes();
	int vblk = vbpp * info.width;

	for ( int y=0; y<tiley; ++y )
	{
		info.dest = mipmap.GetImage() + y * mipmap.GetPitch();
		info.src  = surface.GetImage() + y * info.height * surface.GetPitch();

		for ( int x=0; x<tilex; ++x )
		{
			info.func(info);
			info.dest += vbpp;
			info.src  += vblk;
		}
	}
}


// =================================================
// Bitmap::CreateMipLevel()
// =================================================

Bitmap prcore::CreateMipLevel(const Surface& surface, int sublevel, bool alphafilter)
{

	// assert
	assert( sublevel >= 0 );
	assert( surface.GetImage() != NULL );

	// get pixelformat
	const PixelFormat& pxf = surface.GetPixelFormat();

	// setup mipinfo
	MipInfo info;

	info.rmask   = pxf.GetRedMask();
	info.gmask   = pxf.GetGreenMask();
	info.bmask   = pxf.GetBlueMask();
	info.amask   = pxf.GetAlphaMask();
	info.imask   = pxf.GetIntensityMask();
	info.palette = pxf.GetPalette();
	info.func    = SelectInnerloop(pxf);

	// setup alphafilter
	info.alphafilter = alphafilter;
	if ( alphafilter )
	{
		info.ashift = pxf.GetAlphaOffset();
		info.abits	= pxf.GetAlphaBits();

		float as = static_cast<float>(1 << (info.ashift - info.abits));
		info.aweight = 1.0f / static_cast<float>((1 << info.abits) - 1) / as;
	}

	// compute levels
	uint16 width = surface.GetWidth();
	uint16 height = surface.GetHeight();
	int lodx = log2i(width);
	int lody = log2i(height);

	// compute sublevels
	int slodx = SubLevel(lodx,sublevel);
	int slody = SubLevel(lody,sublevel);

	// compute log2 size
	uint16 w = 1 << lodx;
	uint16 h = 1 << lody;
	int tilex = (1 << slodx);
	int tiley = (1 << slody);

	// create mipmap
	Bitmap mipmap(tilex,tiley,pxf);

	// make source surface match dimensions required for our "beautiful" innerloop(s)
	if ( (width != w) || (height != h) )
	{
		// this could be optimized out if innerloops support bilinear boxfilter
		Bitmap temp(w,h,surface.GetPixelFormat());
		temp.BlitImage(surface,Surface::BLIT_BILINEAR_SCALE);

		// return toplevel, no need for (further) processing
		if ( !sublevel )
			return temp;

		DrawMipLevel(mipmap,temp,tilex,tiley,sublevel,info);
		return mipmap;
	}

	// source surface dimensions match our requirements

	// return toplevel, no need for processing
	if ( !sublevel )
		return Bitmap(surface);

	DrawMipLevel(mipmap,surface,tilex,tiley,sublevel,info);
	return mipmap;
}
