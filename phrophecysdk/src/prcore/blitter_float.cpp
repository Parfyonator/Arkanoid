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
		surface blitter(float) implementation

	revision history:
		Jan/24/2003 - Jukka Liimatta - initial revision (experimental)
*/
#include <prcore/surface.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// ===================================================
// misc
// ===================================================

namespace
{

	inline float gray(float r, float g, float b)
	{
		return b * 0.114f + g * 0.587f + r * 0.299f;
	}

	inline float clampf(float v)
	{
		return (v < 0) ? 0 : (v > 1) ? 1 : v;
	}

}


// ===================================================
// ClearImageFloat()
// ===================================================

template <typename I>
static void ClearLine(I* dest, int count, I v[], uint32 writemask, int stride)
{
	for ( int bit=0; bit<4; ++bit )
	{
		if ( writemask & (1 << bit) )
		{
			I iv = v[bit];
			I* xp = dest + bit;

			int xcount = count;
			while ( xcount-- )
			{
				*xp = iv;
				xp += stride;
			}
		}
	}
}


template <typename F, typename I>
static void ClearBlockFloat(Surface& surface, Surface::ClearMode mode, const color32& color)
{
	int width  = surface.GetWidth();
	int height = surface.GetHeight();
	int pitch  = surface.GetPitch() / sizeof(I);
	I* buffer  = reinterpret_cast<I*>(surface.GetImage());

	const PixelFormat& pxf = surface.GetPixelFormat();

	// color component offsets
	uint32 roffset = pxf.GetRedOffset();
	uint32 goffset = pxf.GetGreenOffset();
	uint32 boffset = pxf.GetBlueOffset();
	uint32 aoffset = pxf.GetAlphaOffset();

	// clear color in floating point
	F r = color.r / 255.0f;
	F g = color.g / 255.0f;
	F b = color.b / 255.0f;
	F a = color.a / 255.0f;

	if ( pxf.IsIntensity() )
	{
		b = gray(r,g,b);
	}

	// clear color as integer bitpattern,
	// ordered according to pixelformat color masks
	I v[4];
	v[roffset] = reinterpret_cast<I&>(r);
	v[goffset] = reinterpret_cast<I&>(g);
	v[boffset] = reinterpret_cast<I&>(b);
	v[aoffset] = reinterpret_cast<I&>(a);

	// calculate write mask
	uint32 writemask = 0;
	if ( pxf.IsDirectColor() )
	{
		// rgb mode
		switch ( mode )
		{
			case Surface::CLEAR_COLOR:
				writemask = (1 << roffset) | (1 << goffset) | (1 << boffset);
				break;

			case Surface::CLEAR_ALPHA:
				writemask = (1 << aoffset);
				break;

			case Surface::CLEAR_COLOR_ALPHA:
				writemask = (1 << roffset) | (1 << goffset) | (1 << boffset) | (1 << aoffset);
				break;
		}
	}
	else
	{
		// grayscale mode
		switch ( mode )
		{
			case Surface::CLEAR_COLOR:
				writemask = (1 << boffset);
				break;

			case Surface::CLEAR_ALPHA:
				writemask = (1 << aoffset);
				break;

			case Surface::CLEAR_COLOR_ALPHA:
				writemask = (1 << boffset) | (1 << aoffset);
				break;
		}
	}

	// calculate xstride
	int xstride = pitch / width;

	// innerloop
	for ( int y=0; y<height; ++y )
	{
		ClearLine<I>(buffer,width,v,writemask,xstride);
		buffer += pitch;
	}
}


void Surface::ClearImageFloat(const color32& color, ClearMode mode)
{
	if ( format.IsFloat16() )
	{
		ClearBlockFloat<float16,uint16>(*this,mode,color);
		return;
	}

	if ( format.IsFloat32() )
	{
		ClearBlockFloat<float32,uint32>(*this,mode,color);
		return;
	}
}


// ===================================================
// BlitImageFloat()
// ===================================================

template <typename D, typename S>
static void COPY_FLOAT_FLOAT(int xcount, int pitch0, int pitch1, void* i0, const void* i1)
{
	D* d = reinterpret_cast<D*>(i0);
	if ( d )
	{
		if ( i1 )
		{
			const S* s = reinterpret_cast<const S*>(i1);
			while ( xcount-- )
			{
				*d = static_cast<D>(*s);
				d += pitch0;
				s += pitch1;
			}
		}
		else
		{
			while ( xcount-- )
			{
				*d = 1;
				d += pitch0;
			}
		}
	}
}


template <typename D, typename S>
static void COPY_FI_FRGB(int count, int pitch0, int pitch1,
	void* i0_, void* a0_,
	const void* r1_, const void* g1_, const void* b1_, const void* a1_)
{
	D* i0 = reinterpret_cast<D*>(i0_);
	D* a0 = reinterpret_cast<D*>(a0_);
	const S* r1 = reinterpret_cast<const S*>(r1_);
	const S* g1 = reinterpret_cast<const S*>(g1_);
	const S* b1 = reinterpret_cast<const S*>(b1_);
	const S* a1 = reinterpret_cast<const S*>(a1_);

	while ( count-- )
	{
		float r;
		float g;
		float b;
		float a;
		if ( r1 ) { r = *r1; r1 += pitch1; } else { r = 1; }
		if ( g1 ) { g = *g1; g1 += pitch1; } else { g = 1; }
		if ( b1 ) { b = *b1; b1 += pitch1; } else { b = 1; }
		if ( a1 ) { a = *a1; a1 += pitch1; } else { a = 1; }

		if ( i0 ) { *i0 = gray(r,g,b); i0 += pitch0; }
		if ( a0 ) { *a0 = a; a0 += pitch0; }
	}
}


template <typename D, typename S>
static void COPY_FRGB_FI(int count, int pitch0, int pitch1,
	void* r0_, void* g0_, void* b0_, void* a0_,
	const void* i1_, const void* a1_)
{
	D* r0 = reinterpret_cast<D*>(r0_);
	D* g0 = reinterpret_cast<D*>(g0_);
	D* b0 = reinterpret_cast<D*>(b0_);
	D* a0 = reinterpret_cast<D*>(a0_);
	const S* i1 = reinterpret_cast<const S*>(i1_);
	const S* a1 = reinterpret_cast<const S*>(a1_);

	while ( count-- )
	{
		float i;
		float a;
		if ( i1 ) { i = *i1; i1 += pitch1; } else { i = 1; }
		if ( a1 ) { a = *a1; a1 += pitch1; } else { a = 1; }

		if ( r0 ) { *r0 = i; r0 += pitch0; }
		if ( g0 ) { *g0 = i; g0 += pitch0; }
		if ( b0 ) { *b0 = i; b0 += pitch0; }
		if ( a0 ) { *a0 = a; a0 += pitch0; }
	}
}


template <typename D, typename S>
static void BlitCopyFloat(Surface& dest, const Surface& source)
{
	uint8* d = dest.GetImage();
	const uint8* s = source.GetImage();

	int width  = dest.GetWidth();
	int height = dest.GetHeight();
	int pitch0 = dest.GetPitch();
	int pitch1 = source.GetPitch();

	const PixelFormat& pxf0 = dest.GetPixelFormat();
	const PixelFormat& pxf1 = source.GetPixelFormat();

	int xstride0 = pitch0 / width / sizeof(D);
	int xstride1 = pitch1 / source.GetWidth() / sizeof(S);

	int inner = 0;
	if ( pxf0.IsDirectColor() ) inner |= 2;
	if ( pxf1.IsDirectColor() ) inner |= 1;

	for ( int y=0; y<height; ++y )
	{
		D* dp = reinterpret_cast<D*>(d);
		D* r0 = dp + pxf0.GetRedOffset();
		D* g0 = dp + pxf0.GetGreenOffset();
		D* b0 = dp + pxf0.GetBlueOffset();
		D* a0 = dp + pxf0.GetAlphaOffset();

		if ( pxf0.GetRedMask() == 0 )   r0 = NULL;
		if ( pxf0.GetGreenMask() == 0 ) g0 = NULL;
		if ( pxf0.GetBlueMask() == 0 )  b0 = NULL;
		if ( pxf0.GetAlphaMask() == 0 ) a0 = NULL;

		const S* sp = reinterpret_cast<const S*>(s);
		const S* r1 = sp + pxf1.GetRedOffset();
		const S* g1 = sp + pxf1.GetGreenOffset();
		const S* b1 = sp + pxf1.GetBlueOffset();
		const S* a1 = sp + pxf1.GetAlphaOffset();

		if ( pxf1.GetRedMask() == 0 )   r1 = NULL;
		if ( pxf1.GetGreenMask() == 0 ) g1 = NULL;
		if ( pxf1.GetBlueMask() == 0 )  b1 = NULL;
		if ( pxf1.GetAlphaMask() == 0 ) a1 = NULL;

		if ( pxf0.IsFloat16() )
		{
			if ( pxf1.IsFloat16() )
			{
				switch ( inner )
				{
					case 0:
						COPY_FLOAT_FLOAT<float16,float16>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float16,float16>(width,xstride0,xstride1,a0,a1);
						break;

					case 1:
						COPY_FI_FRGB<float16,float16>(width,xstride0,xstride1,b0,a0,r1,g1,b1,a1);
						break;

					case 2:
						COPY_FRGB_FI<float16,float16>(width,xstride0,xstride1,r0,g0,b0,a0,b1,a1);
						break;

					case 3:
						COPY_FLOAT_FLOAT<float16,float16>(width,xstride0,xstride1,r0,r1);
						COPY_FLOAT_FLOAT<float16,float16>(width,xstride0,xstride1,g0,g1);
						COPY_FLOAT_FLOAT<float16,float16>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float16,float16>(width,xstride0,xstride1,a0,a1);
						break;
				}
			}

			if ( pxf1.IsFloat32() )
			{
				switch ( inner )
				{
					case 0:
						COPY_FLOAT_FLOAT<float16,float32>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float16,float32>(width,xstride0,xstride1,a0,a1);
						break;

					case 1:
						COPY_FI_FRGB<float16,float32>(width,xstride0,xstride1,b0,a0,r1,g1,b1,a1);
						break;

					case 2:
						COPY_FRGB_FI<float16,float32>(width,xstride0,xstride1,r0,g0,b0,a0,b1,a1);
						break;

					case 3:
						COPY_FLOAT_FLOAT<float16,float32>(width,xstride0,xstride1,r0,r1);
						COPY_FLOAT_FLOAT<float16,float32>(width,xstride0,xstride1,g0,g1);
						COPY_FLOAT_FLOAT<float16,float32>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float16,float32>(width,xstride0,xstride1,a0,a1);
						break;
				}
			}
		}

		if ( pxf0.IsFloat32() )
		{
			if ( pxf1.IsFloat16() )
			{
				switch ( inner )
				{
					case 0:
						COPY_FLOAT_FLOAT<float32,float16>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float32,float16>(width,xstride0,xstride1,a0,a1);
						break;

					case 1:
						COPY_FI_FRGB<float32,float16>(width,xstride0,xstride1,b0,a0,r1,g1,b1,a1);
						break;

					case 2:
						COPY_FRGB_FI<float32,float16>(width,xstride0,xstride1,r0,g0,b0,a0,b1,a1);
						break;

					case 3:
						COPY_FLOAT_FLOAT<float32,float16>(width,xstride0,xstride1,r0,r1);
						COPY_FLOAT_FLOAT<float32,float16>(width,xstride0,xstride1,g0,g1);
						COPY_FLOAT_FLOAT<float32,float16>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float32,float16>(width,xstride0,xstride1,a0,a1);
						break;
				}
			}

			if ( pxf1.IsFloat32() )
			{
				switch ( inner )
				{
					case 0:
						COPY_FLOAT_FLOAT<float32,float32>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float32,float32>(width,xstride0,xstride1,a0,a1);
						break;

					case 1:
						COPY_FI_FRGB<float32,float32>(width,xstride0,xstride1,b0,a0,r1,g1,b1,a1);
						break;

					case 2:
						COPY_FRGB_FI<float32,float32>(width,xstride0,xstride1,r0,g0,b0,a0,b1,a1);
						break;

					case 3:
						COPY_FLOAT_FLOAT<float32,float32>(width,xstride0,xstride1,r0,r1);
						COPY_FLOAT_FLOAT<float32,float32>(width,xstride0,xstride1,g0,g1);
						COPY_FLOAT_FLOAT<float32,float32>(width,xstride0,xstride1,b0,b1);
						COPY_FLOAT_FLOAT<float32,float32>(width,xstride0,xstride1,a0,a1);
						break;
				}
			}
		}

		d += pitch0;
		s += pitch1;
	}
}


template <typename D, typename S>
static void BlitBilinearFloat(Surface& dest, const Surface& source)
{
	// TODO..
	PRCORE_EXCEPTION("Bilinear floating-point blitter is not yet supported.");
	return;
}


template <typename D>
static void BlitIntegerToFloat(Surface& dest, const Surface& source)
{
	uint8* d = dest.GetImage();
	const uint8* s = source.GetImage();

	int width  = dest.GetWidth();
	int height = dest.GetHeight();
	int pitch0 = dest.GetPitch();
	int pitch1 = source.GetPitch();

	int xstride0 = pitch0 / width / sizeof(D);
	const PixelFormat& pxf0 = dest.GetPixelFormat();

	if ( pxf0.IsDirectColor() )
	{
		for ( int y=0; y<height; ++y )
		{
			const color32* sp = reinterpret_cast<const color32*>(s);

			D* dp = reinterpret_cast<D*>(d);
			D* r0 = dp + pxf0.GetRedOffset();
			D* g0 = dp + pxf0.GetGreenOffset();
			D* b0 = dp + pxf0.GetBlueOffset();
			D* a0 = dp + pxf0.GetAlphaOffset();

			if ( pxf0.GetRedMask() == 0 )   r0 = NULL;
			if ( pxf0.GetGreenMask() == 0 ) g0 = NULL;
			if ( pxf0.GetBlueMask() == 0 )  b0 = NULL;
			if ( pxf0.GetAlphaMask() == 0 ) a0 = NULL;

			for ( int x=0; x<width; ++x )
			{
				color32 v = *sp++;
				if ( r0 ) { *r0 = v.r / 255.0f; r0 += xstride0; }
				if ( g0 ) { *g0 = v.g / 255.0f; g0 += xstride0; }
				if ( b0 ) { *b0 = v.b / 255.0f; b0 += xstride0; }
				if ( a0 ) { *a0 = v.a / 255.0f; a0 += xstride0; }
			}

			d += pitch0;
			s += pitch1;
		}
		return;
	}

	if ( pxf0.IsIntensity() )
	{
		for ( int y=0; y<height; ++y )
		{
			const color32* sp = reinterpret_cast<const color32*>(s);

			D* dp = reinterpret_cast<D*>(d);
			D* b0 = dp + pxf0.GetBlueOffset();
			D* a0 = dp + pxf0.GetAlphaOffset();

			if ( pxf0.GetBlueMask() == 0 )  b0 = NULL;
			if ( pxf0.GetAlphaMask() == 0 ) a0 = NULL;

			for ( int x=0; x<width; ++x )
			{
				color32 v = *sp++;
				if ( b0 ) { *b0 = gray(v.r/255.0f,v.g/255.0f,v.b/255.0f); b0 += xstride0; }
				if ( a0 ) { *a0 = v.a / 255.0f; a0 += xstride0; }
			}

			d += pitch0;
			s += pitch1;
		}
		return;
	}
}


template <typename S>
static void BlitFloatToInteger(Surface& dest, const Surface& source)
{
	uint8* d = dest.GetImage();
	const uint8* s = source.GetImage();

	int width  = dest.GetWidth();
	int height = dest.GetHeight();
	int pitch0 = dest.GetPitch();
	int pitch1 = source.GetPitch();

	int xstride1 = pitch1 / source.GetWidth() / sizeof(S);
	const PixelFormat& pxf1 = source.GetPixelFormat();

	if ( pxf1.IsDirectColor() )
	{
		for ( int y=0; y<height; ++y )
		{
			color32* dp = reinterpret_cast<color32*>(d);
			const S* sp = reinterpret_cast<const S*>(s);
			const S* r1 = sp + pxf1.GetRedOffset();
			const S* g1 = sp + pxf1.GetGreenOffset();
			const S* b1 = sp + pxf1.GetBlueOffset();
			const S* a1 = sp + pxf1.GetAlphaOffset();

			if ( pxf1.GetRedMask() == 0 )   r1 = NULL;
			if ( pxf1.GetGreenMask() == 0 ) g1 = NULL;
			if ( pxf1.GetBlueMask() == 0 )  b1 = NULL;
			if ( pxf1.GetAlphaMask() == 0 ) a1 = NULL;

			for ( int x=0; x<width; ++x )
			{
				color32 v = 0xffffffff;
				if ( r1 ) { v.r = static_cast<uint8>(clampf(*r1) * 255.0f); r1 += xstride1;	}
				if ( g1 ) { v.g = static_cast<uint8>(clampf(*g1) * 255.0f); g1 += xstride1;	}
				if ( b1 ) {	v.b = static_cast<uint8>(clampf(*b1) * 255.0f); b1 += xstride1;	}
				if ( a1 ) { v.a = static_cast<uint8>(clampf(*a1) * 255.0f); a1 += xstride1; }
				*dp++ = v;
			}

			d += pitch0;
			s += pitch1;
		}
		return;
	}

	if ( pxf1.IsIntensity() )
	{
		for ( int y=0; y<height; ++y )
		{
			color32* dp = reinterpret_cast<color32*>(d);
			const S* sp = reinterpret_cast<const S*>(s);
			const S* b1 = sp + pxf1.GetBlueOffset();
			const S* a1 = sp + pxf1.GetAlphaOffset();

			if ( pxf1.GetBlueMask() == 0 )  b1 = NULL;
			if ( pxf1.GetAlphaMask() == 0 ) a1 = NULL;

			for ( int x=0; x<width; ++x )
			{
				color32 v = 0xffffffff;
				if ( b1 )
				{
					uint8 i = static_cast<uint8>(*b1 * 255.0f); b1 += xstride1;
					v.r = i;
					v.g = i;
					v.b = i;
				}
				if ( a1 ) { v.a = static_cast<uint8>(*a1 * 255.0f); a1 += xstride1; }
				*dp++ = v;
			}

			d += pitch0;
			s += pitch1;
		}
		return;
	}
}


void Surface::BlitImageFloat(const Surface& source, BlitMode mode)
{

	bool resize = (width != source.width || height != source.height) &&
		(mode == BLIT_SCALE || mode == BLIT_BILINEAR_SCALE);

	// --------------------------------
	// integer-to-float conversion
	// --------------------------------

	if ( source.format.IsInteger() )
	{
		int width2  = source.GetWidth();
		int height2 = source.GetHeight();

		Bitmap source2(width2,height2,PIXELFORMAT_ARGB8888);
		Bitmap source3(width2,height2,format);

		source2.BlitImage(source,BLIT_COPY);

		if ( format.IsFloat16() )
		{
			BlitIntegerToFloat<float16>(source3,source2);
			if ( resize )
			{
				BlitBilinearFloat<float16,float16>(*this,source3);
			}
			else
			{
				BlitCopyFloat<float16,float16>(*this,source3);
			}
		}

		if ( format.IsFloat32() )
		{
			BlitIntegerToFloat<float32>(source3,source2);
			if ( resize )
			{
				BlitBilinearFloat<float32,float32>(*this,source3);
			}
			else
			{
				BlitCopyFloat<float32,float32>(*this,source3);
			}
		}

		return;
	}

	// --------------------------------
	// float-to-integer
	// --------------------------------

	if ( format.IsInteger() )
	{
		if ( resize )
		{
			Bitmap source2(width,height,source.format);

			if ( source.format.IsFloat16() ) BlitBilinearFloat<float16,float16>(source2,source);
			if ( source.format.IsFloat32() ) BlitBilinearFloat<float32,float32>(source2,source);

			Bitmap source3(width,height,PIXELFORMAT_ARGB8888);

			if ( source.format.IsFloat16() ) BlitFloatToInteger<float16>(source3,source2);
			if ( source.format.IsFloat32() ) BlitFloatToInteger<float32>(source3,source2);

			BlitImage(source3);

			return;
		}

		Bitmap source3(width,height,PIXELFORMAT_ARGB8888);

		if ( source.format.IsFloat16() ) BlitFloatToInteger<float16>(source3,source);
		if ( source.format.IsFloat32() ) BlitFloatToInteger<float32>(source3,source);

		BlitImage(source3);

		return;
	}

	// --------------------------------
	// float-to-float
	// --------------------------------

	if ( resize )
	{
		if ( source.format.IsFloat16() )
		{
			if ( format.IsFloat16() ) BlitBilinearFloat<float16,float16>(*this,source);
			if ( format.IsFloat32() ) BlitBilinearFloat<float32,float16>(*this,source);
		}

		if ( source.format.IsFloat32() )
		{
			if ( format.IsFloat16() ) BlitBilinearFloat<float16,float32>(*this,source);
			if ( format.IsFloat32() ) BlitBilinearFloat<float32,float32>(*this,source);
		}
	}
	else
	{
		if ( source.format.IsFloat16() )
		{
			if ( format.IsFloat16() ) BlitCopyFloat<float16,float16>(*this,source);
			if ( format.IsFloat32() ) BlitCopyFloat<float32,float16>(*this,source);
		}

		if ( source.format.IsFloat32() )
		{
			if ( format.IsFloat16() ) BlitCopyFloat<float16,float32>(*this,source);
			if ( format.IsFloat32() ) BlitCopyFloat<float32,float32>(*this,source);
		}
	}
}
