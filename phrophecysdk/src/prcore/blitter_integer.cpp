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
		surface blitter(integer) implementation

	revision history:
		Jun/30/1999 - Jukka Liimatta - initial revision
		Jul/06/1999 - Mikael Kalms - generic and c++ inners added
		Jul/14/1999 - Mikael Kalms - specialized x86 inners added
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Jun/14/2001 - Jukka Liimatta - blitter setup fixes (possible division by zero, etc. ;-)
		Dec/05/2001 - Jukka Liimatta - routine cleanup ;-)
		Dec/12/2001 - Francesco Montecuccoli - ::FlipYImage() method reference implementation
		Aug/17/2002 - Jukka Liimatta - custom C inner fixes (alpha component handling)
		Aug/19/2002 - Jukka Liimatta - optimized innerloops
		Jan/24/2003 - Jukka Liimatta - added support for floating-point pixelformats
		                               (the actual workload is delegated to blitter_float.cpp)
		Jan/31/2004 - Joe Ante - CodeWarrior / MacOS X related fixes in blitter templates
*/
/*
  - Indexed->Indexed [stretch]blit does not check palette at all.
	It should either fail or remap when blitting between pictures with
	different palettes
  - Generic Alpha->Alpha blits zero-extend the lower bits of alpha.
	It should rather repeat the alpha bits over and over again in the
	lower bits...? [Ideally this should be done for all components, but
	it is most important for alpha]
  - blitters[] table is never released, possible memory leak?
*/
/*
	Blitting is supported between all possible formats, except
	!Indexed->Indexed (that situation is handled by Bitmap::Quantize() instead).

    Combined format conversion + stretch operations perform point sampling.
	Pure stretching can perform point sampling on all formats, and
	  bilinear filtering on !Indexed formats.

	Indexed->!Indexed format conversion (+ possible stretch) will first
	convert the palette to	output format, and then remap (+ possibly
	stretch) the image.

	Current inners:

	* Generic C inners. These are quite slow, but will work with *any* Bitmaps.
	* Specialized C inners; stretch and/or format conversion between all
		formats listed in pixelformat.hpp are accelerated.

	MK, 09/07/1999
*/
/*
	Important compiler related information. Some of the template instantiations FAIL to
	compile into correct code with Microsoft Visual C++ 6.0 compiler (ServicePack 5, or
	ProcessorPack 1 do NOT fix this template problem). Compiling the library seems to work
	correctly with Visual Studio.NET, aka. Microsoft Visual C++ 7.0.

	JL, 12/05/2001
*/
/*
	The reason for the above mentioned VC++ 6.0 compiler bug is the way the compiler
	decorates template function names. Name Decoration uses the arguments and return type
	and doesn't use the explicitly specified template argument type. The result is that ONLY
	the last instantiated function is actually generated. The workaround is to add
	dummy parameters (with a default value) to the function declaration of the types
	specified for the template parameters.
	(see MSDN Knowledge Base Q240871 for reference)

	FM, 12/16/2001
*/
/*
	Loss of Accuracy issue on the generic innerloops. When a color component mask is being expanded
	it would result in better accuracy is the least significant bit of the (smaller) source mask would be
	replicated to the all least significant bits of the destination mask.
	
	ARGB4444 -> ARGB8888 conversion, using RED component as example:
	
	--------++++++++--------++++++++
	00000000000000000000RRRR00000000 4-bit source mask
	00000000RRRRrrrr0000000000000000 8-bit target mask
	
	The RRRR 4-bit mask is replicated as-is, the "rrrr" least significant bits
	should be replicated from least significant bit of RRRR 4-bit mask. This would increase
	accuracy to a significant degree. This error is highly visible on the example case, for instance.

	If such mask is used for translucency purposes, the HIGHEST possible translucency will be
	(128+64+32+16) / 255, or in otherwords, 94% of fully opaque value. This error is more than easy
	to see. Easy fix would be to fill the alphaor -mask with 1111's for least significant bits, but this would
	make completely transparent pixels visible with 6% intensity, again, wrong and visible error.
	
	This issue will be remedied in future versions, development resources permitting, as everyone can see,
	this file is rather large piece of code to maintain. ;-)

	Even More Precise Rounding:

	3-bit value converted to 8-bit value, like this:

	ABC       ; 3 bit value, each letter denotes single bit in the value
	ABCABCAB  ; 8 bit value, each letter denotes single bit in the value
	
	This is the so-called "perfect" rouding, the value bits are repeated until the destination value
	is filled with appropriate number of bits. This achieves rounding which matches, in above case,
	the equation:

	value * 255 / 7

	This is accurate to the least significant bit. The Prophecy SDK 2.0 implements this blitting system.
	
	Another Issue of additional pixel-/texelformats in DirectX 9 are support for 64-bit wide, and
	floating-point color. There are VERY difficult to handle efficiently through the current framework.
	
	Conversion from lower resolution to higher resolution will luckily be waste of memory since there won't be
	any extra accuracy, just waste of space in doing such conversions. For efficient support for such formats
	we would also require a custom bitmap codec to read/write to streams in such formats. Luckily these formats
	are mostly introduced to support the fact that 3D graphics chips will use these higher precision formats
	INTERNALLY in their rendering pipelines for better visual quality.

	I recommend writing custom routines to handle traffic between these new DirectX 9 specific formats and
	PRSDK supported formats. If we in future design new version of the SDK it will definitely support
	floating-point color and wider pixelformats, for now, you have to suffer from 32-bit color width limit. ;-)
	
	JL, 02/01/2004
*/
#include <algorithm>
#include <prcore/cpu.hpp>
#include <prcore/surface.hpp>

using namespace prcore;


// ===================================================
// code generation config
// ===================================================

// enable portable specialized innerloops
#define ENABLE_SPECIALIZED_C_INNERS

// enable x86 specific optimizations
#ifdef PRCORE_X86_SIMD
#define ENABLE_SPECIALIZED_X86_INNERS
#define ENABLE_SPECIALIZED_X86_MMX_INNERS
#endif


// ===================================================
// blitter
// ===================================================

enum
{
	BLIT_STRETCH			= 1,	// point-sampling stretch
	BLIT_REMAP				= 2,	// pixelformat conversion
	BLIT_STRETCH_REMAP		= 3,	// point-sampling stretch & format conversion
	BLIT_BILINEAR			= 4,	// bilinear flag
	BLIT_STRETCH_BILINEAR	= 5		// bilinear filtered stretch
};

struct BlitMask
{
	int32	left,right;
	uint32	mask;
};

struct InnerInfo
{
	uint8*	dest;
	uint8*	src;
	uint8*	src2;				// used by bilinear only
	int		width;
	uint32	ustart,ustep;		// used by point sampled + bilinear only
	uint32	vfrac;				// used by bilinear only
};


// information holder for a particular format conversion/stretch
struct Blitter
{
	typedef void (*BlitFunc)(Blitter*, const InnerInfo*);
	typedef bool (*BlitUpdatePalFunc)(Blitter& blitter, const PixelFormat& dest, const PixelFormat& src);

	BlitMask			red,green,blue,alpha,index,intens;
	uint32				alphaor;
	PixelFormat			dest,source;
	int					convtype;
	BlitFunc			func;				// "Perform blit on a row" -function
	BlitUpdatePalFunc	updatepal;			// "Update palette before blitting" -function
	char				palremap[1024];		// Indexed->!Indexed remapping table

	Blitter(const PixelFormat& destination, const PixelFormat& source, int type);
	~Blitter();

	void Blit(const InnerInfo* info) { func(this,info); }
};


static Color32		palmono[256];
static PixelFormat	palformat(32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);


// ===================================================
// external innerloops ( MMX )
// ===================================================

#ifdef ENABLE_SPECIALIZED_X86_INNERS

#ifdef ENABLE_SPECIALIZED_X86_MMX_INNERS
static bool mmxfound = false;
#endif // ENABLE_SPECIALIZED_X86_MMX_INNERS

extern "C"
{
	extern void* inner_stretch_smc_start;
	extern int inner_stretch_smc_size;

	void inner_stretch_rgb565_bilinear_x86(Blitter* blitter, const InnerInfo* info);
	void inner_stretch_argb1555_bilinear_x86(Blitter* blitter, const InnerInfo* info);
	void inner_stretch_argb4444_bilinear_x86(Blitter* blitter, const InnerInfo* info);

	#ifdef ENABLE_SPECIALIZED_X86_MMX_INNERS
	void inner_stretch_rgb888_bilinear_x86_mmx(Blitter* blitter, const InnerInfo* info);
	void inner_stretch_argb8888_bilinear_x86_mmx(Blitter* blitter, const InnerInfo* info);
	#endif // ENABLE_SPECIALIZED_X86_MMX_INNERS
};

#endif // ENABLE_SPECIALIZED_X86_INNERS


// ===================================================
// read/write(24) functions
// ===================================================

static inline uint32 read24(const uint8* v)
{
	#ifdef PRCORE_LITTLE_ENDIAN

		return v[0] | (v[1] << 8) | (v[2] << 16);

	#else

		return (v[0] << 16) | (v[1] << 8) | v[2];

	#endif
}

static inline void write24(uint8* v, uint32 data)
{
	#ifdef PRCORE_LITTLE_ENDIAN

		v[0] =  data        & 0xff;
		v[1] = (data >>  8) & 0xff;
		v[2] = (data >> 16) & 0xff;

	#else

		v[0] = (data >> 16) & 0xff;
		v[1] = (data >>  8) & 0xff;
		v[2] =  data        & 0xff;

	#endif
}


// ===================================================
// generic innerloops
// ===================================================

// remap_rgba_rgba template

template <typename S, typename D>

#if (_MSC_VER <= 1200)
inline void remap_rgba_rgba(Blitter& blitter, const InnerInfo& info, S vc6s = 0, D vc6d = 0)
#else
inline void remap_rgba_rgba(Blitter& blitter, const InnerInfo& info)
#endif

{
	S* src = reinterpret_cast<S*>(info.src);
	D* dest = reinterpret_cast<D*>(info.dest);
	int count = info.width;

	while ( count-- )
	{
		*dest = 
			(((*src >> blitter.red.right   ) << blitter.red.left  ) & blitter.red.mask  ) |
			(((*src >> blitter.green.right ) << blitter.green.left) & blitter.green.mask) |
			(((*src >> blitter.blue.right  ) << blitter.blue.left ) & blitter.blue.mask ) |
			(((*src >> blitter.alpha.right ) << blitter.alpha.left) & blitter.alpha.mask) |
			blitter.alphaor;
		++dest,++src;
	}
}

// remap_rgba_rgba inners

static void inner_remap_1rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint8,uint8>(*blitter,*info);
}

static void inner_remap_1rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint8,uint16>(*blitter,*info);
}

static void inner_remap_1rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = (((*src >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			| (((*src >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			| (((*src >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			| (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_1rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint8,uint32>(*blitter,*info);
}

static void inner_remap_2rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint16,uint8>(*blitter,*info);
}

static void inner_remap_2rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint16,uint16>(*blitter,*info);
}

static void inner_remap_2rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = (((*src >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			| (((*src >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			| (((*src >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			| (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			| blitter->alphaor;
		write24( dest, col );
		dest += 3;
	}
}

static void inner_remap_2rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint16,uint32>(*blitter,*info);
}

static void inner_remap_3rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		*dest++ = (((col >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			    | (((col >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			    | (((col >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			    | (((col >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			    | blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_3rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		*dest++ = (((col >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			    | (((col >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			    | (((col >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			    | (((col >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_3rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col1 = read24(src);
		uint32 col2 = (((col1 >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			 | (((col1 >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			 | (((col1 >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			 | (((col1 >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			 | blitter->alphaor;
		src += 3;
		write24(dest,col2);
		dest += 3;
	}
}

static void inner_remap_3rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		*dest++ = (((col >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			    | (((col >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			    | (((col >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			    | (((col >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_4rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint32,uint8>(*blitter,*info);
}

static void inner_remap_4rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint32,uint16>(*blitter,*info);
}

static void inner_remap_4rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = (((*src >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
			| (((*src >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
			| (((*src >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
			| (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_4rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_rgba<uint32,uint32>(*blitter,*info);
}


// remap_pa_ta template

template <typename S, typename D>

#if (_MSC_VER <= 1200)
inline void remap_pa_ta(Blitter& blitter, const InnerInfo& info, S vc6s = 0, D vc6d = 0)
#else
inline void remap_pa_ta(Blitter& blitter, const InnerInfo& info)
#endif

{
	S* src = reinterpret_cast<S*>(info.src);
	D* dest = reinterpret_cast<D*>(info.dest);
	D* pal = reinterpret_cast<D*>(blitter.palremap);
	int count = info.width;

	while ( count-- )
	{
		*dest =
			pal[(((*src >> blitter.index.right) << blitter.index.left) & blitter.index.mask)] | 
			(((*src >> blitter.alpha.right) << blitter.alpha.left) & blitter.alpha.mask) | 
			blitter.alphaor;
		++dest,++src;
	}
}

// remap_pa_ta inners

static void inner_remap_1pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint8,uint8>(*blitter,*info);
}

static void inner_remap_1pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint8,uint16>(*blitter,*info);
}

static void inner_remap_1pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		int offs = (((*src >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3;
		uint32 col = read24( &pal[offs] )
			| (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_1pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint8,uint32>(*blitter,*info);
}

static void inner_remap_2pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint16,uint8>(*blitter,*info);
}

static void inner_remap_2pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint16,uint16>(*blitter,*info);
}

static void inner_remap_2pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		int offs = (((*src >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3;
		uint32 col = read24(&pal[offs])
			| (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_2pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint16,uint32>(*blitter,*info);
}

static void inner_remap_3pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		*dest++ = pal[(((col >> blitter->index.right) << blitter->index.left) & blitter->index.mask)]
				| (((col >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_3pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	uint16* pal = reinterpret_cast<uint16*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		*dest++ = pal[(((col >> blitter->index.right) << blitter->index.left) & blitter->index.mask)]
				| (((col >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_3pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 col1 = read24( src );
		int offs = (((col1 >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3;
		uint32 col2 = read24( &pal[offs] )
			 | (((col1 >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			 | blitter->alphaor;
		src += 3;
		write24(dest,col2);
		dest += 3;
	}
}

static void inner_remap_3pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	uint32* pal = reinterpret_cast<uint32*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		*dest++ = pal[(((col >> blitter->index.right) << blitter->index.left) & blitter->index.mask)]
				| (((col >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_4pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint32,uint8>(*blitter,*info);
}

static void inner_remap_4pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint32,uint16>(*blitter,*info);
}

static void inner_remap_4pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;
	
	while ( count-- )
	{
		int offs = (((*src >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3;
		uint32 col = read24( &pal[offs] )
			| (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_4pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	remap_pa_ta<uint32,uint32>(*blitter,*info);
}


// remap_rgba_ia template

template <typename S, typename D>

#if (_MSC_VER <= 1200)
inline void remap_rgba_ia(Blitter& blitter, const InnerInfo& info, S vc6s = 0, D vc6d = 0)
#else
inline void remap_rgba_ia(Blitter& blitter, const InnerInfo& info)
#endif

{
	S* src = reinterpret_cast<S*>(info.src);
	D* dest = reinterpret_cast<D*>(info.dest);
	int count = info.width;

	while ( count-- )
	{
		uint32 col =
			palmono[((*src >> blitter.red.right  ) << blitter.red.left  ) & blitter.red.mask  ].r +
			palmono[((*src >> blitter.green.right) << blitter.green.left) & blitter.green.mask].g +
			palmono[((*src >> blitter.blue.right ) << blitter.blue.left ) & blitter.blue.mask ].b;
		*dest = 
			(((col >> blitter.intens.right) << blitter.intens.left) & blitter.intens.mask) | 
			(((*src >> blitter.alpha.right) << blitter.alpha.left) & blitter.alpha.mask) |
			blitter.alphaor;
		++dest,++src;
	}
}

// remap_rgba_ia inners

static void inner_remap_1rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint8,uint8>(*blitter,*info);
}

static void inner_remap_1rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint8,uint16>(*blitter,*info);
}

static void inner_remap_1rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = palmono[((*src >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((*src >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((*src >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		col = (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
		      | (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
			  | blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_1rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint8,uint32>(*blitter,*info);
}

static void inner_remap_2rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint16,uint8>(*blitter,*info);
}

static void inner_remap_2rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint16,uint16>(*blitter,*info);
}

static void inner_remap_2rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = palmono[((*src >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((*src >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((*src >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		col = (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
		        | (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_2rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint16,uint32>(*blitter,*info);
}

static void inner_remap_3rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col1 = read24(src);
		uint32 col2 = palmono[((col1 >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			   + palmono[((col1 >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			   + palmono[((col1 >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		*dest++ = (((col2 >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
			    | (((col1 >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_3rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col1 = read24(src);
		uint32 col2 = palmono[((col1 >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			   + palmono[((col1 >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			   + palmono[((col1 >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		*dest++ = (((col2 >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
			    | (((col1 >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_3rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col1 = read24(src);
		uint32 col2 = palmono[((col1 >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			   + palmono[((col1 >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			   + palmono[((col1 >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		col2 = (((col2 >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
		        | (((col1 >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
 				| blitter->alphaor;
		src += 3;
		write24(dest,col2);
		dest += 3;
	}
}

static void inner_remap_3rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col1 = read24(src);
		uint32 col2 = palmono[((col1 >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((col1 >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((col1 >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		*dest++ = (((col2 >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
			    | (((col1 >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		src += 3;
	}
}

static void inner_remap_4rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint32,uint8>(*blitter,*info);
}

static void inner_remap_4rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint32,uint16>(*blitter,*info);
}

static void inner_remap_4rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;
	
	while ( count-- )
	{
		uint32 col = palmono[((*src >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((*src >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((*src >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		col = (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
		        | (((*src++ >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_4rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	remap_rgba_ia<uint32,uint32>(*blitter,*info);
}


// ===================================================
// specialized innerloops
// ===================================================

#ifdef ENABLE_SPECIALIZED_C_INNERS

static void inner_remap_p8_1ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;
	
	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = pal[col] | alphaor;
	}
}

static void inner_remap_p8_2ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* pal = reinterpret_cast<uint16*>(blitter->palremap);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = pal[col] | alphaor;
	}
}

static void inner_remap_p8_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest,read24(&pal[col * 3]) | alphaor);
		dest += 3;
	}
}

static void inner_remap_p8_4ta(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* pal = reinterpret_cast<uint32*>(blitter->palremap);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = pal[col] | alphaor;
	}
}

static void inner_remap_i8_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = (col & 0xe0) | ((col & 0xe0) >> 3) | ((col & 0xc0) >> 6);
	}
}

static void inner_remap_i8_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = ((col & 0xf8) << 8) | ((col & 0xfc) << 3) | ((col & 0xf8) >> 3);
	}
}

static void inner_remap_i8_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest, (col << 16) | (col << 8) | col );
		dest += 3;
	}
}

static void inner_remap_i8_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		uint32 col2 = (col & 0xf8);
		*dest++ = (col2 << 7) | (col2 << 2) | (col2 >> 3) | 0x8000;
	}
}

static void inner_remap_i8_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		uint32 col2 = (col & 0xf0);
		*dest++ = (col2 << 4) | col2 | (col2 >> 4) | 0xf000;
	}
}

static void inner_remap_i8_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = (col << 16) | (col << 8) | col | 0xff000000;
	}
}

static void inner_remap_rgb332_i8(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			palmono[(col & 0xe0)     ].r +
			palmono[(col & 0x1c) << 3].g +
			palmono[(col & 0x03) << 6].b;
	}
}

static void inner_remap_rgb332_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = ((col & 0xe0) << 8) | ((col & 0x1c) << 6) | ((col & 0x03) << 3);
	}
}

static void inner_remap_rgb332_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest, ((col & 0xe0) << 16) | ((col & 0x1c) << 11) | ((col & 0x03) << 6) );
		dest += 3;
	}
}

static void inner_remap_rgb332_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = ((col & 0xe0) << 7) | ((col & 0x1c) << 5) | ((col & 0x03) << 3) | 0x8000;
	}
}

static void inner_remap_rgb332_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = ((col & 0xe0) << 4) | ((col & 0x1c) << 3) | ((col & 0x03) << 2) | 0xf000;
	}
}

static void inner_remap_rgb332_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ = ((col & 0xe0) << 16) | ((col & 0x1c) << 11) | ((col & 0x03) << 6) | 0xff000000;
	}
}

static void inner_remap_rgb565_i8(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			palmono[(col & 0xf800) >> 8].r +
			palmono[(col & 0x07e0) >> 3].g +
			palmono[(col & 0x001f) << 3].b;
	}
}

static void inner_remap_rgb565_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0xe000) >> 8) |
			((col & 0x0700) >> 6) |
			((col & 0x0018) >> 3);
	}
}

static void inner_remap_rgb565_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest,((col & 0xf800) << 8) | ((col & 0x07e0) << 5) | ((col & 0x001f) << 3));
		dest += 3;
	}
}

static void inner_remap_rgb565_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	if ( ((int)src ^ (int)dest) & 2 ) // misaligned?
	{
		while ( count-- )
		{
			uint32 col = *src++;
			*dest++ = ((col & 0xffc0) >> 1) | (col & 0x1f) | 0x8000;
		}
	}
	else
	{
		if ( (int)src & 2 ) // odd first uint16?
		{
			uint32 col = *src++;
			*dest++ = ((col & 0xffc0) >> 1) | (col & 0x1f) | 0x8000;
			--count;
		}

		int count2 = (count >> 1);
		if ( count2 ) // middle uint32s?
		{
			while ( count2-- )
			{
				uint32 col = *reinterpret_cast<uint32*>(src);
				src += 2;
				*reinterpret_cast<uint32*>(dest) = ((col & 0xffc0ffc0) >> 1) | (col & 0x001f001f) | 0x80008000;
				dest += 2;
			}
		}

		if ( count & 1 ) // odd last uint16?
		{
			uint32 col = *src++;
			*dest++ = ((col & 0xffc0) >> 1) | (col & 0x1f) | 0x8000;
		}
	}
}

static void inner_remap_rgb565_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	if ( ((int) src ^ (int) dest) & 2 ) // misaligned?
	{
		while ( count-- )
		{
			uint32 col = *src++;
			*dest++ = ((col & 0xf000) >> 4) | ((col & 0x0780) >> 3) | ((col & 0x001e) >> 1) | 0xf000;
		}
	}
	else
	{
		if ( (int) src & 2 ) // odd first uint16?
		{
			uint32 col = *src++;
			*dest++ = ((col & 0xf000) >> 4) | ((col & 0x0780) >> 3) | ((col & 0x001e) >> 1) | 0xf000;
			--count;
		}

		int count2 = (count >> 1);
		if ( count2 ) // middle uint32s?
		{
			while ( count2-- )
			{
				uint32 col = *((uint32*) src);
				src += 2;
				*((uint32*) dest) = ((col & 0xf000f000) >> 4) | ((col & 0x07800780) >> 3) 
									| ((col & 0x001e001e) >> 1) | 0xf000f000;
				dest += 2;
			}
		}

		if ( count & 1 ) // odd last uint16?
		{
			uint32 col = *src++;
			*dest++ = ((col & 0xf000) >> 4) | ((col & 0x780) >> 3) | ((col & 0x001e) >> 1) | 0xf000;
		}
	}
}

static void inner_remap_rgb565_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0xf800) << 8) |
			((col & 0x07e0) << 5) |
			((col & 0x001f) << 3) | 0xff000000;
	}
}

static void inner_remap_rgb888_i8(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		src += 3;
		*dest++ =
			palmono[(col & 0x00ff0000) >> 16].r +
			palmono[(col & 0x0000ff00) >> 8 ].g +
			palmono[(col & 0x000000ff)      ].b;
	}
}

static void inner_remap_rgb888_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		src += 3;
		*dest++ =
			((col & 0x00e00000) >> 16) |
			((col & 0x0000e000) >> 11) |
			((col & 0x000000c0) >> 6);
	}
}

static void inner_remap_rgb888_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		src += 3;
		*dest++ =
			((col & 0x00f80000) >> 8) |
			((col & 0x0000fc00) >> 5) |
			((col & 0x000000f8) >> 3);
	}
}

static void inner_remap_rgb888_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		src += 3;
		*dest++ =
			((col & 0x00f80000) >> 9) |
			((col & 0x0000f800) >> 6) |
			((col & 0x000000f8) >> 3) | 0x8000;
	}
}

static void inner_remap_rgb888_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(src);
		src += 3;
		*dest++ =
			((col & 0x00f00000) >> 12) |
			((col & 0x0000f000) >> 8 ) |
			((col & 0x000000f0) >> 4 ) | 0xf000;
	}
}

static void inner_remap_rgb888_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = read24(src) | 0xff000000;
		src += 3;
	}
}

static void inner_remap_argb1555_i8(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			palmono[(col & 0x7c00) >> 7].r +
			palmono[(col & 0x03e0) >> 2].g +
			palmono[(col & 0x001f) << 3].b;
	}
}

static void inner_remap_argb1555_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x7000) >> 7) |
			((col & 0x0380) >> 5) |
			((col & 0x0018) >> 3);
	}
}

static void inner_remap_argb1555_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	if ( ((int) src ^ (int) dest) & 2 ) // misaligned?
	{
		while ( count-- )
		{
			uint32 col = *src++ & 0x7fff;
			*dest++ = col + (col & 0xffe0);
		}
	}
	else
	{
		if ( (int) src & 2 ) // odd first uint16?
		{
			uint32 col = *src++ & 0x7fff;
			*dest++ = col + (col & 0xffe0);
			--count;
		}

		int count2 = (count >> 1);
		if ( count2 ) // middle uint32s?
		{
			while ( count2-- )
			{
				uint32 col = *((uint32*) src) & 0x7fff7fff;
				src += 2;
				*((uint32*) dest) = col + (col & 0xffe0ffe0);
				dest += 2;
			}
		}

		if ( count & 1 ) // odd last uint16?
		{
			uint32 col = *src++ & 0x7fff;
			*dest++ = col + (col & 0xffe0);
		}
	}
}

static void inner_remap_argb1555_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest, ((col & 0x7c00) << 9) | ((col & 0x03e0) << 6) | ((col & 0x001f) << 3) );
		dest += 3;
	}
}

static void inner_remap_argb1555_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	if ( ((int) src ^ (int) dest) & 2 ) // misaligned?
	{
		while ( count-- )
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x7800) >> 3) |
				((col & 0x03c0) >> 2) |
				((col & 0x001e) >> 1) |
				(0x10000 - ((col & 0x8000) >> 3));
		}
	}
	else
	{
		if ( (int) src & 2 ) // odd first uint16?
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x7800) >> 3) |
				((col & 0x03c0) >> 2) |
				((col & 0x001e) >> 1) |
				(0x10000 - ((col & 0x8000) >> 3));
			--count;
		}

		int count2 = (count >> 1);
		if ( count2 ) // middle uint32s?
		{
			while ( count2-- )
			{
				uint32 col = *((uint32*) src);
				src += 2;
				*((uint32*) dest) =
					((col & 0x78007800) >> 3) |
					((col & 0x03c003c0) >> 2) |
					((col & 0x001e001e) >> 1) |
					((-int((col & 0x8000) >> 4)) & 0xf000f000);
				dest += 2;
			}
		}

		if ( count & 1 ) // odd last uint16?
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x7800) >> 3) |
				((col & 0x03c0) >> 2) |
				((col & 0x001e) >> 1) |
				(0x10000 - ((col & 0x8000) >> 3));
		}
	}
}

static void inner_remap_argb1555_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x7c00) << 9) |
			((col & 0x03e0) << 6) |
			((col & 0x001f) << 3) |
			((-int(col & 0x8000)) & 0xff000000);
	}
}

static void inner_remap_argb4444_i8(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			palmono[(col & 0x0f00) >> 8].r +
			palmono[(col & 0x00f0)     ].g +
			palmono[(col & 0x000f) << 4].b;
	}
}

static void inner_remap_argb4444_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x0e00) >> 4) |
			((col & 0x00e0) >> 3) |
			((col & 0x000c) >> 2);
	}
}

static void inner_remap_argb4444_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	if ( ((int) src ^ (int) dest) & 2 ) // misaligned?
	{
		while ( count-- )
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x0f00) << 4) |
				((col & 0x00f0) << 3) |
				((col & 0x000f) << 1);
		}
	}
	else
	{
		if ( (int) src & 2 ) // odd first uint16?
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x0f00) << 4) |
				((col & 0x00f0) << 3) |
				((col & 0x000f) << 1);
			--count;
		}

		int count2 = (count >> 1);
		if ( count2 ) // middle uint32s?
		{
			while ( count2-- )
			{
				uint32 col = *((uint32*) src);
				src += 2;
				*((uint32*) dest) =
					((col & 0x0f000f00) << 4) |
					((col & 0x00f000f0) << 3) |
					((col & 0x000f000f) << 1);
				dest += 2;
			}
		}

		if ( count & 1 ) // odd last uint16?
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x0f00) << 4) |
				((col & 0x00f0) << 3) |
				((col & 0x000f) << 1);
		}
	}
}

static void inner_remap_argb4444_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest, ((col & 0x0f00) << 12) | ((col & 0x00f0) << 8) | ((col & 0x000f) << 4) );
		dest += 3;
	}
}

static void inner_remap_argb4444_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	if ( ((int) src ^ (int) dest) & 2 ) // misaligned?
	{
		while ( count-- )
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x0f00) << 3) |
				((col & 0x00f0) << 2) |
				((col & 0x000f) << 1) | (col & 0x8000);
		}
	}
	else
	{
		if ( (int) src & 2 ) // odd first uint16?
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x0f00) << 3) |
				((col & 0x00f0) << 2) |
				((col & 0x000f) << 1) | (col & 0x8000);
			--count;
		}

		int count2 = (count >> 1);
		if ( count2 ) // middle uint32s?
		{
			while ( count2-- )
			{
				uint32 col = *((uint32*) src);
				src += 2;
				*((uint32*) dest) =
					((col & 0x0f000f00) << 3) |
					((col & 0x00f000f0) << 2) |
					((col & 0x000f000f) << 1) | (col & 0x80008000);
				dest += 2;
			}
		}

		if ( count & 1 ) // odd last uint16?
		{
			uint32 col = *src++;
			*dest++ =
				((col & 0x0f00) << 3) |
				((col & 0x00f0) << 2) |
				((col & 0x000f) << 1) | (col & 0x8000);
		}
	}
}

static void inner_remap_argb4444_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		uint32 alpha = (col & 0xf000);
		*dest++ =
			((col & 0x0f00) << 12) |
			((col & 0x00f0) <<  8) |
			((col & 0x000f) <<  4) | (alpha << 16) | (alpha << 12);
	}
}

static void inner_remap_argb8888_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			palmono[(col & 0x00ff0000) >> 16].r +
			palmono[(col & 0x0000ff00) >> 8 ].g +
			palmono[(col & 0x000000ff)      ].b;
	}
}

static void inner_remap_argb8888_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x00e00000) >> 16) |
			((col & 0x0000e000) >> 11) |
			((col & 0x000000c0) >> 6);
	}
}

static void inner_remap_argb8888_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x00f80000) >> 8) |
			((col & 0x0000fc00) >> 5) |
			((col & 0x000000f8) >> 3);
	}
}

static void inner_remap_argb8888_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		write24(dest,col);
		dest += 3;
	}
}

static void inner_remap_argb8888_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x00f80000) >> 9) |
			((col & 0x0000f800) >> 6) |
			((col & 0x000000f8) >> 3) |
			((col & 0x80000000) >> 16);
	}
}

static void inner_remap_argb8888_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = *src++;
		*dest++ =
			((col & 0x00f00000) >> 12) |
			((col & 0x0000f000) >>  8) |
			((col & 0x000000f0) >>  4) |
			((col & 0xf0000000) >> 16);
	}
}

#endif // ENABLE_SPECIALIZED_C_INNERS


// ===================================================
// stretch/generic innerloops
// ===================================================

static void inner_stretch_1ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = src[u >> 16];
		u += info->ustep;
	}
}

static void inner_stretch_2ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = src[u >> 16];
		u += info->ustep;
	}
}

static void inner_stretch_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		write24(dest,read24(&src[(u >> 16) * 3]));
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_4ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = src[u >> 16];
		u += info->ustep;
	}
}

static void inner_stretch_1rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = src[u >> 16];
		u += info->ustep;
	}
}

static void inner_stretch_2rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = src[u >> 16];
		u += info->ustep;
	}
}

static void inner_stretch_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		write24(dest,read24( &src[(u >> 16) * 3]));
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_4rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = src[u >> 16];
		u += info->ustep;
	}
}


// ===================================================
// stretch+remap innerloops
// ===================================================

// stretch_remap_rgba_rgba template

template <typename S, typename D>

#if (_MSC_VER <= 1200)
inline void stretch_remap_rgba_rgba(Blitter& blitter, const InnerInfo& info, S vc6s = 0, D vc6d = 0)
#else
inline void stretch_remap_rgba_rgba(Blitter& blitter, const InnerInfo& info)
#endif

{
	uint32 u = info.ustart;
	S* src = reinterpret_cast<S*>(info.src);
	D* dest = reinterpret_cast<D*>(info.dest);
	int count = info.width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		*dest = (((origcol >> blitter.red.right  ) << blitter.red.left  ) & blitter.red.mask) |
			(((origcol >> blitter.green.right) << blitter.green.left) & blitter.green.mask) |
			(((origcol >> blitter.blue.right ) << blitter.blue.left ) & blitter.blue.mask) |
			(((origcol >> blitter.alpha.right) << blitter.alpha.left) & blitter.alpha.mask) |
			blitter.alphaor;
		u += info.ustep;
		++dest;
	}
}

// stretch_remap_rgba_rgba inners

static void inner_stretch_remap_1rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint8,uint8>(*blitter,*info);
}

static void inner_stretch_remap_1rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint8,uint16>(*blitter,*info);

}

static void inner_stretch_remap_1rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		write24(dest, (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
					   | (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
					   | (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_1rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint8,uint32>(*blitter,*info);

}

static void inner_stretch_remap_2rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint16,uint8>(*blitter,*info);
}

static void inner_stretch_remap_2rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint16,uint16>(*blitter,*info);
}

static void inner_stretch_remap_2rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		write24(dest, (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
					   | (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
					   | (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_2rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint16,uint32>(*blitter,*info);
}

static void inner_stretch_remap_3rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		*dest++ = (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
				| (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
				| (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
				| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		*dest++ = (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
				| (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
				| (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
				| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )	
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		write24(dest, (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
					   | (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
					   | (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		*dest++ = (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
				| (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
				| (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
				| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_4rgba_1rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint32,uint8>(*blitter,*info);
}

static void inner_stretch_remap_4rgba_2rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint32,uint16>(*blitter,*info);
}

static void inner_stretch_remap_4rgba_3rgba(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		write24(dest, (((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask)
					   | (((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask)
					   | (((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_4rgba_4rgba(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_rgba<uint32,uint32>(*blitter,*info);
}

// stretch_remap_pa_ta template

template <typename S, typename D>

#if (_MSC_VER <= 1200)
inline void stretch_remap_pa_ta(Blitter& blitter, const InnerInfo& info, S vc6s = 0, D vc6d = 0)
#else
inline void stretch_remap_pa_ta(Blitter& blitter, const InnerInfo& info)
#endif

{
	uint32 u = info.ustart;
	S* src = reinterpret_cast<S*>(info.src);
	D* dest = reinterpret_cast<D*>(info.dest);
	D* pal = reinterpret_cast<D*>(blitter.palremap);
	int count = info.width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		*dest = 
			pal[(((origcol >> blitter.index.right) << blitter.index.left) & blitter.index.mask)] |
				(((origcol >> blitter.alpha.right) << blitter.alpha.left) & blitter.alpha.mask) |
				blitter.alphaor;
		u += info.ustep;
		++dest;
	}
}

// stretch_remap_pa_ta inners

static void inner_stretch_remap_1pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint8,uint8>(*blitter,*info);
}

static void inner_stretch_remap_1pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint8,uint16>(*blitter,*info);
}

static void inner_stretch_remap_1pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		write24 (dest, read24( &pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3] )
						| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
						| blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_1pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint8,uint32>(*blitter,*info);
}

static void inner_stretch_remap_2pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint16,uint8>(*blitter,*info);
}

static void inner_stretch_remap_2pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint16,uint16>(*blitter,*info);
}

static void inner_stretch_remap_2pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		write24 (dest, read24( &pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3] )
						| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
						| blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_2pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint16,uint32>(*blitter,*info);
}

static void inner_stretch_remap_3pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		*dest++ = pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask)]
				| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	uint16* pal = reinterpret_cast<uint16*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		*dest++ = pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask)]
				| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		write24 (dest, read24( &pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3] )
						| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
						| blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	uint32* pal = reinterpret_cast<uint32*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		*dest++ = pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask)]
				| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_4pa_1ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint32,uint8>(*blitter,*info);
}

static void inner_stretch_remap_4pa_2ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint32,uint16>(*blitter,*info);
}

static void inner_stretch_remap_4pa_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		write24 (dest, read24( &pal[(((origcol >> blitter->index.right) << blitter->index.left) & blitter->index.mask) * 3] )
						| (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
						| blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_4pa_4ta(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_pa_ta<uint32,uint32>(*blitter,*info);
}

// stretch_remap_rgba_ia template

template <typename S, typename D>

#if (_MSC_VER <= 1200)
inline void stretch_remap_rgba_ia(Blitter& blitter, const InnerInfo& info, S vc6s = 0, D vc6d = 0)
#else
inline void stretch_remap_rgba_ia(Blitter& blitter, const InnerInfo& info)
#endif

{
	uint32 u = info.ustart;
	S* src = reinterpret_cast<S*>(info.src);
	D* dest = reinterpret_cast<D*>(info.dest);
	int count = info.width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		uint32 col =
			palmono[((origcol >> blitter.red.right  ) << blitter.red.left  ) & blitter.red.mask  ].r +
			palmono[((origcol >> blitter.green.right) << blitter.green.left) & blitter.green.mask].g +
			palmono[((origcol >> blitter.blue.right ) << blitter.blue.left ) & blitter.blue.mask ].b;
		*dest =
			(((col     >> blitter.intens.right) << blitter.intens.left) & blitter.intens.mask) |
			(((origcol >> blitter.alpha.right ) << blitter.alpha.left ) & blitter.alpha.mask) |
			blitter.alphaor;
		u += info.ustep;
		++dest;
	}
}

// stretch_remap_rgba_ia inners

static void inner_stretch_remap_1rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint8,uint8>(*blitter,*info);
}

static void inner_stretch_remap_1rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint8,uint16>(*blitter,*info);
}

static void inner_stretch_remap_1rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		uint32 col = palmono[((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		write24(dest, (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_1rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint8,uint32>(*blitter,*info);
}

static void inner_stretch_remap_2rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint16,uint8>(*blitter,*info);
}

static void inner_stretch_remap_2rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint16,uint16>(*blitter,*info);

}

static void inner_stretch_remap_2rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		uint32 col = palmono[((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		write24(dest, (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_2rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint16,uint32>(*blitter,*info);
}

static void inner_stretch_remap_3rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		uint32 col = palmono[((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		*dest++ = (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
			    | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		uint32 col = palmono[((origcol >> blitter->red.right) << blitter->red.left) & blitter->red.mask].r
			  + palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g
			  + palmono[((origcol >> blitter->blue.right) << blitter->blue.left) & blitter->blue.mask].b;
		*dest++ = (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
			    | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		uint32 col =
			palmono[((origcol >> blitter->red.right  ) << blitter->red.left  ) & blitter->red.mask].r +
			palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g +
			palmono[((origcol >> blitter->blue.right ) << blitter->blue.left ) & blitter->blue.mask].b;
		write24(dest, (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_3rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = read24(&src[(u >> 16) * 3]);
		uint32 col =
			palmono[((origcol >> blitter->red.right  ) << blitter->red.left  ) & blitter->red.mask].r +
			palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g +
			palmono[((origcol >> blitter->blue.right ) << blitter->blue.left ) & blitter->blue.mask].b;
		*dest++ = (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
			    | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
				| blitter->alphaor;
		u += info->ustep;
	}
}

static void inner_stretch_remap_4rgba_1ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint32,uint8>(*blitter,*info);
}

static void inner_stretch_remap_4rgba_2ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint32,uint16>(*blitter,*info);
}

static void inner_stretch_remap_4rgba_3ia(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 origcol = src[u >> 16];
		uint32 col =
			palmono[((origcol >> blitter->red.right  ) << blitter->red.left  ) & blitter->red.mask].r +
			palmono[((origcol >> blitter->green.right) << blitter->green.left) & blitter->green.mask].g +
			palmono[((origcol >> blitter->blue.right ) << blitter->blue.left ) & blitter->blue.mask].b;
		write24(dest, (((col >> blitter->intens.right) << blitter->intens.left) & blitter->intens.mask)
					   | (((origcol >> blitter->alpha.right) << blitter->alpha.left) & blitter->alpha.mask)
					   | blitter->alphaor );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_remap_4rgba_4ia(Blitter* blitter, const InnerInfo* info)
{
	stretch_remap_rgba_ia<uint32,uint32>(*blitter,*info);
}


// ===================================================
// stretch+remap specialized innerloops
// ===================================================

#ifdef ENABLE_SPECIALIZED_C_INNERS

static void inner_stretch_remap_p8_1ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ = pal[col] | alphaor;
		u += ustep;
	}
}

static void inner_stretch_remap_p8_2ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	uint16* pal = reinterpret_cast<uint16*>(blitter->palremap);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ = pal[col] | alphaor;
		u += ustep;
	}
}

static void inner_stretch_remap_p8_3ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	uint8* pal = reinterpret_cast<uint8*>(blitter->palremap);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest, read24( &pal[col * 3] ) | alphaor);
		dest += 3;
		u += ustep;
	}
}

static void inner_stretch_remap_p8_4ta(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	uint32* pal = reinterpret_cast<uint32*>(blitter->palremap);
	uint32 alphaor = blitter->alphaor;
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ = pal[col] | alphaor;
		u += ustep;
	}
}

static void inner_stretch_remap_i8_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			( col & 0xe0) |
			((col & 0xe0) >> 3) |
			((col & 0xc0) >> 6);
		u += ustep;
	}
}

static void inner_stretch_remap_i8_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xf8) << 8) |
			((col & 0xfc) << 3) |
			((col & 0xf8) >> 3);
		u += ustep;
	}
}

static void inner_stretch_remap_i8_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest, (col << 16) | (col << 8) | col);
		dest += 3;
		u += ustep;
	}
}

static void inner_stretch_remap_i8_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		uint32 col2 = (col & 0xf8);
		*dest++ = (col2 << 7) | (col2 << 2) | (col2 >> 3) | 0x8000;
		u += ustep;
	}
}

static void inner_stretch_remap_i8_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		uint32 col2 = (col & 0xf0);
		*dest++ = (col2 << 4) | col2 | (col2 >> 4) | 0xf000;
		u += ustep;
	}
}

static void inner_stretch_remap_i8_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ = (col << 16) | (col << 8) | col | 0xff000000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb332_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			palmono[ col & 0xe0      ].r +
			palmono[(col & 0x1c) << 3].g +
			palmono[(col & 0x03) << 6].b;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb332_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xe0) << 8) |
			((col & 0x1c) << 6) |
			((col & 0x03) << 3);
		u += ustep;
	}
}

static void inner_stretch_remap_rgb332_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest, ((col & 0xe0) << 16) | ((col & 0x1c) << 11) | ((col & 0x03) << 6));
		dest += 3;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb332_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xe0) << 7) |
			((col & 0x1c) << 5) |
			((col & 0x03) << 3) | 0x8000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb332_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xe0) << 4) |
			((col & 0x1c) << 3) |
			((col & 0x03) << 2) | 0xf000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb332_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xe0) << 16) |
			((col & 0x1c) << 11) |
			((col & 0x03) <<  6) | 0xff000000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb565_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			palmono[(col & 0xf800) >> 8].r +
			palmono[(col & 0x07e0) >> 3].g +
			palmono[(col & 0x001f) << 3].b;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb565_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xe000) >> 8) |
			((col & 0x0700) >> 6) |
			((col & 0x0018) >> 3);
		u += ustep;
	}
}

static void inner_stretch_remap_rgb565_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest, ((col & 0xf800) << 8) | ((col & 0x07e0) << 5) | ((col & 0x001f) << 3));
		u += ustep;
		dest += 3;
	}
}

static void inner_stretch_remap_rgb565_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ = ((col & 0xffc0) >> 1) | (col & 0x1f) | 0x8000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb565_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xf000) >> 4) |
			((col & 0x0780) >> 3) |
			((col & 0x001e) >> 1) | 0xf000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb565_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0xf800) << 8) |
			((col & 0x07e0) << 5) |
			((col & 0x001f) << 3) | 0xff000000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb888_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(&src[(u >> 16) * 3]);
		*dest++ =
			palmono[(col & 0x00ff0000) >> 16].r +
			palmono[(col & 0x0000ff00) >>  8].g +
			palmono[(col & 0x000000ff)      ].b;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb888_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(&src[(u >> 16) * 3]);
		*dest++ =
			((col & 0x00e00000) >> 16) |
			((col & 0x0000e000) >> 11) |
			((col & 0x000000c0) >>  6);
		u += ustep;
	}
}

static void inner_stretch_remap_rgb888_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(&src[(u >> 16) * 3]);
		*dest++ =
			((col & 0x00f80000) >> 8) |
			((col & 0x0000fc00) >> 5) |
			((col & 0x000000f8) >> 3);
		u += ustep;
	}
}

static void inner_stretch_remap_rgb888_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(&src[(u >> 16) * 3]);
		*dest++ =
			((col & 0x00f80000) >> 9) |
			((col & 0x0000f800) >> 6) |
			((col & 0x000000f8) >> 3) | 0x8000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb888_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = read24(&src[(u >> 16) * 3]);
		*dest++ =
			((col & 0x00f00000) >> 12) |
			((col & 0x0000f000) >>  8) |
			((col & 0x000000f0) >>  4) | 0xf000;
		u += ustep;
	}
}

static void inner_stretch_remap_rgb888_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint8* src = reinterpret_cast<uint8*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		*dest++ = read24(&src[(u >> 16) * 3]) | 0xff000000;
		u += ustep;
	}
}

static void inner_stretch_remap_argb1555_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			palmono[(col & 0x7c00) >> 7].r +
			palmono[(col & 0x03e0) >> 2].g +
			palmono[(col & 0x001f) << 3].b;
		u += ustep;
	}
}

static void inner_stretch_remap_argb1555_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x7000) >> 7) |
			((col & 0x0380) >> 5) |
			((col & 0x0018) >> 3);
		u += ustep;
	}
}

static void inner_stretch_remap_argb1555_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16] & 0x7fff;
		*dest++ = col + (col & 0xffe0);
		u += ustep;
	}
}

static void inner_stretch_remap_argb1555_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest, ((col & 0x7c00) << 9) | ((col & 0x03e0) << 6) | ((col & 0x001f) << 3));
		dest += 3;
		u += ustep;
	}
}

static void inner_stretch_remap_argb1555_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x7800) >> 3) |
			((col & 0x03c0) >> 2) |
			((col & 0x001e) >> 1) | (0x10000 - ((col & 0x8000) >> 3));
		u += ustep;
	}
}

static void inner_stretch_remap_argb1555_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x7c00) << 9) |
			((col & 0x03e0) << 6) |
			((col & 0x001f) << 3) | ((-int(col & 0x8000)) & 0xff000000);
		u += ustep;
	}
}

static void inner_stretch_remap_argb4444_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			palmono[(col & 0x0f00) >> 8].r +
			palmono[(col & 0x00f0)     ].g +
			palmono[(col & 0x000f) << 4].b;
		u += ustep;
	}
}

static void inner_stretch_remap_argb4444_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x0e00) >> 4) |
			((col & 0x00e0) >> 3) |
			((col & 0x000c) >> 2);
		u += ustep;
	}
}

static void inner_stretch_remap_argb4444_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x0f00) << 4) |
			((col & 0x00f0) << 3) |
			((col & 0x000f) << 1);
		u += ustep;
	}
}

static void inner_stretch_remap_argb4444_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest, ((col & 0x0f00) << 12) | ((col & 0x00f0) << 8) | ((col & 0x000f) << 4));
		dest += 3;
		u += ustep;
	}
}

static void inner_stretch_remap_argb4444_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x0f00) << 3) |
			((col & 0x00f0) << 2) |
			((col & 0x000f) << 1) | (col & 0x8000);
		u += ustep;
	}
}

static void inner_stretch_remap_argb4444_argb8888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint16* src = reinterpret_cast<uint16*>(info->src);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		uint32 alpha = (col & 0xf000);
		*dest++ =
			((col & 0x0f00) << 12) |
			((col & 0x00f0) <<  8) |
			((col & 0x000f) <<  4) | (alpha << 16) | (alpha << 12);
		u += ustep;
	}
}

static void inner_stretch_remap_argb8888_i8(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			palmono[(col & 0x00ff0000) >> 16].r +
			palmono[(col & 0x0000ff00) >>  8].g +
			palmono[(col & 0x000000ff)      ].b;
		u += ustep;
	}
}

static void inner_stretch_remap_argb8888_rgb332(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x00e00000) >> 16) |
			((col & 0x0000e000) >> 11) |
			((col & 0x000000c0) >>  6);
		u += ustep;
	}
}

static void inner_stretch_remap_argb8888_rgb565(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x00f80000) >> 8) |
			((col & 0x0000fc00) >> 5) |
			((col & 0x000000f8) >> 3);
		u += ustep;
	}
}

static void inner_stretch_remap_argb8888_rgb888(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		write24(dest,col);
		dest += 3;
		u += ustep;
	}
}

static void inner_stretch_remap_argb8888_argb1555(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x00f80000) >> 9) |
			((col & 0x0000f800) >> 6) |
			((col & 0x000000f8) >> 3) | 0x8000;
		u += ustep;
	}
}

static void inner_stretch_remap_argb8888_argb4444(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 ustep = info->ustep;
	uint32* src = reinterpret_cast<uint32*>(info->src);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 col = src[u >> 16];
		*dest++ =
			((col & 0x00f00000) >> 12) |
			((col & 0x0000f000) >>  8) |
			((col & 0x000000f0) >>  4) | 0xf000;
		u += ustep;
	}
}

#endif // ENABLE_SPECIALIZED_C_INNERS


// ===================================================
// stretch+bilinear generic innerloops
// ===================================================

static void inner_stretch_1ia_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src1 = reinterpret_cast<uint8*>(info->src);
	uint8* src2 = reinterpret_cast<uint8*>(info->src2);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;
	
	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		uint32 col1i = ((0x10000 - ufrac) * (((src1[uinte] >> blitter->intens.right) << blitter->intens.left) & 0xff)
			     + ufrac * (((src1[uinte + 1] >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col2i = ((0x10000 - ufrac) * (((src2[uinte] >> blitter->intens.right) << blitter->intens.left) & 0xff)
				 + ufrac * (((src2[uinte + 1] >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((src1[uinte] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((src1[uinte + 1] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((src2[uinte] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((src2[uinte + 1] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1i + info->vfrac * col2i) >> 16) >> blitter->intens.left) 
				<< blitter->intens.right) & blitter->intens.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		*dest++ = col;
		u += info->ustep;
	}
}

static void inner_stretch_2ia_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src1 = reinterpret_cast<uint16*>(info->src);
	uint16* src2 = reinterpret_cast<uint16*>(info->src2);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		uint32 col1i = ((0x10000 - ufrac) * (((src1[uinte] >> blitter->intens.right) << blitter->intens.left) & 0xff)
			     + ufrac * (((src1[uinte + 1] >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col2i = ((0x10000 - ufrac) * (((src2[uinte] >> blitter->intens.right) << blitter->intens.left) & 0xff)
				 + ufrac * (((src2[uinte + 1] >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((src1[uinte] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((src1[uinte + 1] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((src2[uinte] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((src2[uinte + 1] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1i + info->vfrac * col2i) >> 16) >> blitter->intens.left) 
				<< blitter->intens.right) & blitter->intens.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		*dest++ = col;
		u += info->ustep;
	}
}

static void inner_stretch_3ia_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src1 = reinterpret_cast<uint8*>(info->src);
	uint8* src2 = reinterpret_cast<uint8*>(info->src2);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		int offs = uinte * 3;
		uint32 pix11 = read24( &src1[offs] );
		uint32 pix12 = read24( &src1[offs + 3] );
		uint32 pix21 = read24( &src2[offs] );
		uint32 pix22 = read24( &src2[offs + 3] );
		uint32 col1i = ((0x10000 - ufrac) * (((pix11 >> blitter->intens.right) << blitter->intens.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col2i = ((0x10000 - ufrac) * (((pix21 >> blitter->intens.right) << blitter->intens.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((pix11 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((pix21 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix22 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1i + info->vfrac * col2i) >> 16) >> blitter->intens.left) 
				<< blitter->intens.right) & blitter->intens.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		write24(dest,col);
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_4ia_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src1 = reinterpret_cast<uint32*>(info->src);
	uint32* src2 = reinterpret_cast<uint32*>(info->src2);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )	
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		uint32 col1i = ((0x10000 - ufrac) * (((src1[uinte] >> blitter->intens.right) << blitter->intens.left) & 0xff)
			     + ufrac * (((src1[uinte + 1] >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col2i = ((0x10000 - ufrac) * (((src2[uinte] >> blitter->intens.right) << blitter->intens.left) & 0xff)
				 + ufrac * (((src2[uinte + 1] >> blitter->intens.right) << blitter->intens.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((src1[uinte] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((src1[uinte + 1] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((src2[uinte] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((src2[uinte + 1] >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1i + info->vfrac * col2i) >> 16) >> blitter->intens.left) 
				<< blitter->intens.right) & blitter->intens.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		*dest++ = col;
		u += info->ustep;
	}
}

static void inner_stretch_1rgba_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src1 = reinterpret_cast<uint8*>(info->src);
	uint8* src2 = reinterpret_cast<uint8*>(info->src2);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		uint32 pix11 = src1[uinte];
		uint32 pix12 = src1[uinte + 1];
		uint32 pix21 = src2[uinte];
		uint32 pix22 = src2[uinte + 1];
		uint32 col1r = ((0x10000 - ufrac) * (((pix11 >> blitter->red.right) << blitter->red.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col2r = ((0x10000 - ufrac) * (((pix21 >> blitter->red.right) << blitter->red.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col1g = ((0x10000 - ufrac) * (((pix11 >> blitter->green.right) << blitter->green.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col2g = ((0x10000 - ufrac) * (((pix21 >> blitter->green.right) << blitter->green.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col1b = ((0x10000 - ufrac) * (((pix11 >> blitter->blue.right) << blitter->blue.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col2b = ((0x10000 - ufrac) * (((pix21 >> blitter->blue.right) << blitter->blue.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((pix11 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((pix21 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix22 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1r + info->vfrac * col2r) >> 16) >> blitter->red.left) 
				<< blitter->red.right) & blitter->red.mask)
			  | ((((((0x10000 - info->vfrac) * col1g + info->vfrac * col2g) >> 16) >> blitter->green.left) 
				  << blitter->green.right) & blitter->green.mask)
			  | ((((((0x10000 - info->vfrac) * col1b + info->vfrac * col2b) >> 16) >> blitter->blue.left) 
				  << blitter->blue.right) & blitter->blue.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		*dest++ = col;
		u += info->ustep;
	}
}

static void inner_stretch_2rgba_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint16* src1 = reinterpret_cast<uint16*>(info->src);
	uint16* src2 = reinterpret_cast<uint16*>(info->src2);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		uint32 pix11 = src1[uinte];
		uint32 pix12 = src1[uinte + 1];
		uint32 pix21 = src2[uinte];
		uint32 pix22 = src2[uinte + 1];
		uint32 col1r = ((0x10000 - ufrac) * (((pix11 >> blitter->red.right) << blitter->red.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col2r = ((0x10000 - ufrac) * (((pix21 >> blitter->red.right) << blitter->red.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col1g = ((0x10000 - ufrac) * (((pix11 >> blitter->green.right) << blitter->green.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col2g = ((0x10000 - ufrac) * (((pix21 >> blitter->green.right) << blitter->green.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col1b = ((0x10000 - ufrac) * (((pix11 >> blitter->blue.right) << blitter->blue.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col2b = ((0x10000 - ufrac) * (((pix21 >> blitter->blue.right) << blitter->blue.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((pix11 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((pix21 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix22 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1r + info->vfrac * col2r) >> 16) >> blitter->red.left) 
				<< blitter->red.right) & blitter->red.mask)
			  | ((((((0x10000 - info->vfrac) * col1g + info->vfrac * col2g) >> 16) >> blitter->green.left) 
				  << blitter->green.right) & blitter->green.mask)
			  | ((((((0x10000 - info->vfrac) * col1b + info->vfrac * col2b) >> 16) >> blitter->blue.left) 
				  << blitter->blue.right) & blitter->blue.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		*dest++ = col;
		u += info->ustep;
	}
}

static void inner_stretch_3rgba_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint8* src1 = reinterpret_cast<uint8*>(info->src);
	uint8* src2 = reinterpret_cast<uint8*>(info->src2);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		int offs = uinte * 3;
		uint32 pix11 = read24( &src1[offs] );
		uint32 pix12 = read24( &src1[offs + 3] );
		uint32 pix21 = read24( &src2[offs] );
		uint32 pix22 = read24( &src2[offs + 3] );
		uint32 col1r = ((0x10000 - ufrac) * (((pix11 >> blitter->red.right) << blitter->red.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col2r = ((0x10000 - ufrac) * (((pix21 >> blitter->red.right) << blitter->red.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col1g = ((0x10000 - ufrac) * (((pix11 >> blitter->green.right) << blitter->green.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col2g = ((0x10000 - ufrac) * (((pix21 >> blitter->green.right) << blitter->green.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col1b = ((0x10000 - ufrac) * (((pix11 >> blitter->blue.right) << blitter->blue.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col2b = ((0x10000 - ufrac) * (((pix21 >> blitter->blue.right) << blitter->blue.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((pix11 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((pix21 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix22 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1r + info->vfrac * col2r) >> 16) >> blitter->red.left) 
				<< blitter->red.right) & blitter->red.mask)
			  | ((((((0x10000 - info->vfrac) * col1g + info->vfrac * col2g) >> 16) >> blitter->green.left) 
				  << blitter->green.right) & blitter->green.mask)
			  | ((((((0x10000 - info->vfrac) * col1b + info->vfrac * col2b) >> 16) >> blitter->blue.left) 
				  << blitter->blue.right) & blitter->blue.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		write24(dest,col);
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_4rgba_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32* src1 = reinterpret_cast<uint32*>(info->src);
	uint32* src2 = reinterpret_cast<uint32*>(info->src2);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		// Mikael Kalms wrote these,
		// and no small wonder he is called 'The Codenator'
		// He's the best and couldn't have done this without him. -JL
		uint32 uinte = (u >> 16);
		uint32 ufrac = u & 0xffff;
		uint32 pix11 = src1[uinte];
		uint32 pix12 = src1[uinte + 1];
		uint32 pix21 = src2[uinte];
		uint32 pix22 = src2[uinte + 1];
		uint32 col1r = ((0x10000 - ufrac) * (((pix11 >> blitter->red.right) << blitter->red.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col2r = ((0x10000 - ufrac) * (((pix21 >> blitter->red.right) << blitter->red.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->red.right) << blitter->red.left) & 0xff)) >> 16;
		uint32 col1g = ((0x10000 - ufrac) * (((pix11 >> blitter->green.right) << blitter->green.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col2g = ((0x10000 - ufrac) * (((pix21 >> blitter->green.right) << blitter->green.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->green.right) << blitter->green.left) & 0xff)) >> 16;
		uint32 col1b = ((0x10000 - ufrac) * (((pix11 >> blitter->blue.right) << blitter->blue.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col2b = ((0x10000 - ufrac) * (((pix21 >> blitter->blue.right) << blitter->blue.left) & 0xff)
				 + ufrac * (((pix22 >> blitter->blue.right) << blitter->blue.left) & 0xff)) >> 16;
		uint32 col1a = ((0x10000 - ufrac) * (((pix11 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix12 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col2a = ((0x10000 - ufrac) * (((pix21 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)
			     + ufrac * (((pix22 >> blitter->alpha.right) << blitter->alpha.left) & 0xff)) >> 16;
		uint32 col = ((((((0x10000 - info->vfrac) * col1r + info->vfrac * col2r) >> 16) >> blitter->red.left) 
				<< blitter->red.right) & blitter->red.mask)
			  | ((((((0x10000 - info->vfrac) * col1g + info->vfrac * col2g) >> 16) >> blitter->green.left) 
				  << blitter->green.right) & blitter->green.mask)
			  | ((((((0x10000 - info->vfrac) * col1b + info->vfrac * col2b) >> 16) >> blitter->blue.left) 
				  << blitter->blue.right) & blitter->blue.mask)
			  | ((((((0x10000 - info->vfrac) * col1a + info->vfrac * col2a) >> 16) >> blitter->alpha.left) 
				  << blitter->alpha.right) & blitter->alpha.mask);
		*dest++ = col;
		u += info->ustep;
	}
}


// ===================================================
// stretch+bilinear specialized innerloops
// ===================================================

#ifdef ENABLE_SPECIALIZED_C_INNERS

static void inner_stretch_i8_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 vfrac2 = info->vfrac >> 8;
	uint32 vfrac1 = 0x100 - vfrac2;
	uint8* src1 = reinterpret_cast<uint8*>(info->src);
	uint8* src2 = reinterpret_cast<uint8*>(info->src2);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac2 = (u & 0xff00) >> 8;
		uint32 ufrac1 = 0x100 - ufrac2;
		uint32 pix1 = (src1[uinte] << 16) | src2[uinte];
		uint32 pix2 = (src1[uinte + 1] << 16) | src2[uinte + 1];
		uint32 pix3 = ufrac1 * pix1 + ufrac2 * pix2;
		*dest++ = ((vfrac1 * ((pix3 & 0xff000000) >> 16)
					+ vfrac2 * (pix3 & 0x0000ff00)) & 0x00ff0000) >> 16;
		u += info->ustep;
	}
}

static void inner_stretch_rgb565_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 vfrac2 = info->vfrac >> 11;
	uint32 vfrac1 = 0x20 - vfrac2;
	uint16* src1 = reinterpret_cast<uint16*>(info->src);
	uint16* src2 = reinterpret_cast<uint16*>(info->src2);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac2 = (u & 0xf800) >> 11;
		uint32 ufrac1 = 0x20 - ufrac2;
		uint32 pix1a = (src1[uinte] << 16) | src2[uinte];
		uint32 pix2a = (src1[uinte + 1] << 16) | src2[uinte + 1];
		uint32 pix1b = pix1a & 0x07e0f81f;
		uint32 pix2b = pix2a & 0x07e0f81f;
		pix1a = (pix1a & 0xf81f07e0) >> 5;
		pix2a = (pix2a & 0xf81f07e0) >> 5;

		uint32 colrbg = ((ufrac1 * pix1a + ufrac2 * pix2a) & 0xf81f07e0);
		uint32 colgrb = ((ufrac1 * pix1b + ufrac2 * pix2b) & 0xfc1f03e0) >> 5;
		*dest++ = (((vfrac1 * ((colrbg & 0xf81f0000) >> 16)
					+ vfrac2 * (colgrb & 0x0000f81f)) & 0x001f03e0) >> 5)
			      | (((vfrac1 * ((colgrb & 0x07e00000) >> 16)
				      + vfrac2 * (colrbg & 0x000007e0)) & 0x0000fc00) >> 5);
		u += info->ustep;
	}
}

static void inner_stretch_rgb888_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 vfrac2 = info->vfrac >> 8;
	uint32 vfrac1 = 0x100 - vfrac2;
	uint8* src1 = reinterpret_cast<uint8*>(info->src);
	uint8* src2 = reinterpret_cast<uint8*>(info->src2);
	uint8* dest = reinterpret_cast<uint8*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac2 = (u & 0xff00) >> 8;
		uint32 ufrac1 = 0x100 - ufrac2;
		int offs = uinte * 3;
		uint32 pix11a = read24( &src1[offs] );
		uint32 pix12a = read24( &src1[offs + 3] );
		uint32 pix21a = read24( &src2[offs] );
		uint32 pix22a = read24( &src2[offs + 3] );
		uint32 pix11b = pix11a & 0x00ff00ff;
		uint32 pix12b = pix12a & 0x00ff00ff;
		uint32 pix21b = pix21a & 0x00ff00ff;
		uint32 pix22b = pix22a & 0x00ff00ff;
		pix11a = pix11a & 0x0000ff00;
		pix12a = pix12a & 0x0000ff00;
		pix21a = pix21a & 0x0000ff00;
		pix22a = pix22a & 0x0000ff00;

		uint32 col1g = (ufrac1 * pix11a + ufrac2 * pix12a) & 0x00ff0000;
		uint32 col2g = (ufrac1 * pix21a + ufrac2 * pix22a) & 0x00ff0000;
		uint32 col1rb = ((ufrac1 * pix11b + ufrac2 * pix12b) & 0xff00ff00) >> 8;
		uint32 col2rb = ((ufrac1 * pix21b + ufrac2 * pix22b) & 0xff00ff00) >> 8;
		write24(dest, (((vfrac1 * col1g + vfrac2 * col2g) & 0xff000000) >> 16)
					   | (((vfrac1 * col1rb + vfrac2 * col2rb) & 0xff00ff00) >> 8) );
		dest += 3;
		u += info->ustep;
	}
}

static void inner_stretch_argb1555_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 vfrac2 = info->vfrac >> 11;
	uint32 vfrac1 = 0x20 - vfrac2;
	uint16* src1 = reinterpret_cast<uint16*>(info->src);
	uint16* src2 = reinterpret_cast<uint16*>(info->src2);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac2 = (u & 0xf800) >> 11;
		uint32 ufrac1 = 0x20 - ufrac2;
		uint32 pix1a = (src1[uinte] << 16) | src2[uinte];
		uint32 pix2a = (src1[uinte + 1] << 16) | src2[uinte + 1];
		uint32 alpha = (pix1a & 0x80000000) >> 16;
		uint32 pix1b = pix1a & 0x03e07c1f;
		uint32 pix2b = pix2a & 0x03e07c1f;
		pix1a = (pix1a & 0x7c1f03e0) >> 5;
		pix2a = (pix2a & 0x7c1f03e0) >> 5;

		uint32 colrbg = ((ufrac1 * pix1a + ufrac2 * pix2a) & 0x7c1f03e0);
		uint32 colgrb = ((ufrac1 * pix1b + ufrac2 * pix2b) & 0x7c0f83e0) >> 5;
		*dest++ = (((vfrac1 * ((colrbg & 0x7c1f0000) >> 16) 
					+ vfrac2 * (colgrb & 0x00007c1f)) & 0x000f83e0) >> 5)
			      | (((vfrac1 * ((colgrb & 0x03e00000) >> 16)
				      + vfrac2 * (colrbg & 0x000003e0)) & 0x00007c00) >> 5)
				  | alpha;
		u += info->ustep;
	}
}

static void inner_stretch_argb4444_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 vfrac2 = info->vfrac >> 12;
	uint32 vfrac1 = 0x10 - vfrac2;
	uint16* src1 = reinterpret_cast<uint16*>(info->src);
	uint16* src2 = reinterpret_cast<uint16*>(info->src2);
	uint16* dest = reinterpret_cast<uint16*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac2 = (u & 0xf000) >> 12;
		uint32 ufrac1 = 0x10 - ufrac2;
		uint32 pix1a = (src1[uinte] << 16) | src2[uinte];
		uint32 pix2a = (src1[uinte + 1] << 16) | src2[uinte + 1];
		uint32 pix1b = pix1a & 0x0f0f0f0f;
		uint32 pix2b = pix2a & 0x0f0f0f0f;
		pix1a = (pix1a & 0xf0f0f0f0) >> 4;
		pix2a = (pix2a & 0xf0f0f0f0) >> 4;

		uint32 colag = ((ufrac1 * pix1a + ufrac2 * pix2a) & 0xf0f0f0f0) >> 4;
		uint32 colrb = ((ufrac1 * pix1b + ufrac2 * pix2b) & 0xf0f0f0f0) >> 4;
		*dest++ = ((vfrac1 * (colag >> 16) + vfrac2 * (colag & 0x0f0f)) & 0x0000f0f0)
			      | (((vfrac1 * (colrb >> 16) + vfrac2 * (colrb & 0x0f0f)) & 0x0000f0f0) >> 4);
		u += info->ustep;
	}
}

static void inner_stretch_argb8888_bilinear(Blitter* blitter, const InnerInfo* info)
{
	uint32 u = info->ustart;
	uint32 vfrac2 = info->vfrac >> 8;
	uint32 vfrac1 = 0x100 - vfrac2;
	uint32* src1 = reinterpret_cast<uint32*>(info->src);
	uint32* src2 = reinterpret_cast<uint32*>(info->src2);
	uint32* dest = reinterpret_cast<uint32*>(info->dest);
	int count = info->width;

	while ( count-- )
	{
		uint32 uinte = (u >> 16);
		uint32 ufrac2 = (u & 0xff00) >> 8;
		uint32 ufrac1 = 0x100 - ufrac2;
		uint32 pix11a = src1[uinte];
		uint32 pix12a = src1[uinte + 1];
		uint32 pix21a = src2[uinte];
		uint32 pix22a = src2[uinte + 1];
		uint32 pix11b = pix11a & 0x00ff00ff;
		uint32 pix12b = pix12a & 0x00ff00ff;
		uint32 pix21b = pix21a & 0x00ff00ff;
		uint32 pix22b = pix22a & 0x00ff00ff;
		pix11a = (pix11a & 0xff00ff00) >> 8;
		pix12a = (pix12a & 0xff00ff00) >> 8;
		pix21a = (pix21a & 0xff00ff00) >> 8;
		pix22a = (pix22a & 0xff00ff00) >> 8;

		uint32 col1ag = ((ufrac1 * pix11a + ufrac2 * pix12a) & 0xff00ff00) >> 8;
		uint32 col2ag = ((ufrac1 * pix21a + ufrac2 * pix22a) & 0xff00ff00) >> 8;
		uint32 col1rb = ((ufrac1 * pix11b + ufrac2 * pix12b) & 0xff00ff00) >> 8;
		uint32 col2rb = ((ufrac1 * pix21b + ufrac2 * pix22b) & 0xff00ff00) >> 8;
		*dest++ = (((vfrac1 * col1ag + vfrac2 * col2ag) & 0xff00ff00))
			      | (((vfrac1 * col1rb + vfrac2 * col2rb) & 0xff00ff00) >> 8);
		u += info->ustep;
	}
}

#endif // ENABLE_SPECIALIZED_C_INNERS


// ===================================================
// blitter implementation
// ===================================================

static const int BlitHashBits = 6; // must be 1 or larger
static const int BlitHashSize = (1 << BlitHashBits);
static Blitter* blitters[BlitHashSize];


static void BitmapBlitterInitialize()
{
	memset(blitters,0,sizeof(blitters));

	// initialize the RGB->greyscale multiply lookuptable
	for ( int i=0; i<256; ++i )
	{
		palmono[i].r = static_cast<uint8>(i * 0.299f);
		palmono[i].g = static_cast<uint8>(i * 0.587f);
		palmono[i].b = static_cast<uint8>(i * 0.114f);
		palmono[i].a = 0;
	}

	#ifdef ENABLE_SPECIALIZED_X86_INNERS
	#ifdef PRCORE_PLATFORM_WIN32
	unsigned long temp;
	VirtualProtect(&inner_stretch_smc_start,inner_stretch_smc_size,PAGE_EXECUTE_READWRITE,&temp);
	#endif
	#endif // ENABLE_SPECIALIZED_X86_INNERS

	#ifdef ENABLE_SPECIALIZED_X86_MMX_INNERS
	mmxfound = IsMMX();
	#endif // ENABLE_SPECIALIZED_X86_MMX_INNERS
}


static inline void MakeBlitMask(BlitMask& out, uint32 srcmask, uint32 dstmask )
{
	if ( !srcmask || !dstmask )
	{
		out.left = out.right = out.mask = 0;
	}
	else
	{
		int shift = HighestBit(dstmask) - HighestBit(srcmask);
		if ( shift > 0 )
		{
			out.left = shift;
			out.right = 0;
		}
		else
		{
			out.left = 0;
			out.right = -shift;
		}
		out.mask = ((srcmask >> out.right) << out.left) & dstmask;
	}
}


static inline void MakeBlitMaskBilinear(BlitMask& out, uint32 mask)
{
	if ( !mask )
	{
		out.left = 0;
		out.right = 32;
		out.mask = 0;
	}
	else
	{
		int shift = 7 - HighestBit( mask );
		if (shift > 0)
		{
			out.left = shift;
			out.right = 0;
		}
		else
		{
			out.left = 0;
			out.right = -shift;
		}
		out.mask = mask & ((0xff >> out.left) << out.right);
	}
}


static bool UpdatePalGenericRemap(Blitter& b, const PixelFormat& dest, const PixelFormat& source)
{
	if ( !source.IsIndexed() )
		return true;

	if ( dest.IsIndexed() ) // Indexed -> Indexed
	{
		// only partial support for Indexed -> Indexed so far
		return true;
	}
	else // Indexed -> !Indexed
	{
		Blitter blitter( dest, palformat, BLIT_REMAP );

		InnerInfo info;
		info.src = (uint8*) source.GetPalette();
		info.dest = (uint8*) b.palremap;
		info.width = 256;
		blitter.Blit( &info );
	}

	return true;
}


Blitter::Blitter(const PixelFormat& dest, const PixelFormat& source, int type)
{
	func = NULL;
	updatepal = NULL;

	static PixelFormat stdformats[8] = 
	{ 
		PIXELFORMAT_PALETTE8(NULL),
		PIXELFORMAT_INTENSITY8,
		PIXELFORMAT_RGB332,
		PIXELFORMAT_RGB565,
		PIXELFORMAT_RGB888,
		PIXELFORMAT_ARGB1555,
		PIXELFORMAT_ARGB4444,
		PIXELFORMAT_ARGB8888
	};

	if ( source.GetBytes() > 4 || dest.GetBytes() > 4 )
		return;

	if ( !source.IsIndexed() && dest.IsIndexed() )	// !Indexed -> Indexed is not supported
		return;


	convtype = type;
	this->source = source;
	this->dest = dest;

	int sourceidx, destidx;
	for ( sourceidx = 0; sourceidx < (int)sizeof(stdformats) / (int)sizeof(PixelFormat); sourceidx++ )
		if ( stdformats[sourceidx] == source )
			break;
	if ( sourceidx == sizeof(stdformats) / sizeof(PixelFormat) )
		sourceidx = -1;

	for ( destidx = 0; destidx < (int)sizeof(stdformats) / (int)sizeof(PixelFormat); destidx++ )
		if ( stdformats[destidx] == dest )
			break;
	if ( destidx == sizeof(stdformats) / sizeof(PixelFormat) )
		destidx = -1;

	switch ( convtype )
	{
		case 0:
			break;

		case BLIT_REMAP:
			if ( source.IsIndexed() && dest.IsIndexed() ) // Indexed -> Indexed is fully supported,
			{											  // except for remapping between different
				MakeBlitMask( red, source.GetIntensityMask(), dest.GetIntensityMask() ); // palettes
				MakeBlitMask( green, 0, 0 );
				MakeBlitMask( blue, 0, 0 );
				MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
				alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();

				static BlitFunc funcptrs[4][4] =
				{
					{ inner_remap_1rgba_1rgba, inner_remap_1rgba_2rgba, inner_remap_1rgba_3rgba, inner_remap_1rgba_4rgba},
					{ inner_remap_2rgba_1rgba, inner_remap_2rgba_2rgba, inner_remap_2rgba_3rgba, inner_remap_2rgba_4rgba},
					{ inner_remap_3rgba_1rgba, inner_remap_3rgba_2rgba, inner_remap_3rgba_3rgba, inner_remap_3rgba_4rgba},
					{ inner_remap_4rgba_1rgba, inner_remap_4rgba_2rgba, inner_remap_4rgba_3rgba, inner_remap_4rgba_4rgba}
				};

				func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
			}
			else if ( source.IsIndexed() ) // Indexed -> !Indexed is fully supported
			{
				if ( dest.IsIntensity() )
				{
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					MakeBlitMask( index, source.GetIntensityMask(), 0x000000ff );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();
				}
				else
				{
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					MakeBlitMask( index, source.GetIntensityMask(), 0x000000ff );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();
				}

				/* // Feature il Debugga Maxima
				
				char buffy[256];
				sprintf( buffy, "r: %d %d %08x\ng: %d %d %08x\nb: %d %d %08x\n"
								"a: %d %d %08x\ni: %d %d %08x\n"
								"source: %08x %08x %08x %08x\n"
								"dest: %08x %08x %08x %08x\n",
					red.left, red.right, red.mask,
					green.left, green.right, green.mask,
					blue.left, blue.right, blue.mask,
					alpha.left, alpha.right, alpha.mask,
					index.left, index.right, index.mask,
					source.GetRedMask(), source.GetGreenMask(), source.GetBlueMask(), source.GetAlphaMask(),
					dest.GetRedMask(), dest.GetGreenMask(), dest.GetBlueMask(), dest.GetAlphaMask() );
				MessageBox( NULL, buffy, "P2?", MB_OK );
				*/

				#ifdef ENABLE_SPECIALIZED_C_INNERS

				if (sourceidx == 0 && destidx != -1)
				{
					static BlitFunc funcptrs[4] =
					{ inner_remap_p8_1ta, inner_remap_p8_2ta, inner_remap_p8_3ta, 
					  inner_remap_p8_4ta };

					updatepal = &UpdatePalGenericRemap;

					func = funcptrs[dest.GetBytes() - 1];
					return;
				}

				#endif // ENABLE_SPECIALIZED_C_INNERS

				static BlitFunc funcptrs[4][4] =
				{
					{ inner_remap_1pa_1ta, inner_remap_1pa_2ta, inner_remap_1pa_3ta, inner_remap_1pa_4ta},
					{ inner_remap_2pa_1ta, inner_remap_2pa_2ta, inner_remap_2pa_3ta, inner_remap_2pa_4ta},
					{ inner_remap_3pa_1ta, inner_remap_3pa_2ta, inner_remap_3pa_3ta, inner_remap_3pa_4ta},
					{ inner_remap_4pa_1ta, inner_remap_4pa_2ta, inner_remap_4pa_3ta, inner_remap_4pa_4ta}
				};

				updatepal = &UpdatePalGenericRemap;

				func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
			}
			else // !Indexed -> !Indexed is fully supported
			{

				#ifdef ENABLE_SPECIALIZED_C_INNERS

				if (sourceidx != -1 && destidx != -1)
				{
					static BlitFunc funcptrs[8][8] =
					{
						{ 0, 0, 0, 0, 0, 0, 0, 0 },
						{ 0, 0, inner_remap_i8_rgb332, inner_remap_i8_rgb565,
						  inner_remap_i8_rgb888, inner_remap_i8_argb1555,
						  inner_remap_i8_argb4444, inner_remap_i8_argb8888 },
						{ 0, inner_remap_rgb332_i8, 0, inner_remap_rgb332_rgb565,
						  inner_remap_rgb332_rgb888, inner_remap_rgb332_argb1555,
						  inner_remap_rgb332_argb4444, inner_remap_rgb332_argb8888 },
						{ 0, inner_remap_rgb565_i8, inner_remap_rgb565_rgb332, 0,
						  inner_remap_rgb565_rgb888, inner_remap_rgb565_argb1555,
						  inner_remap_rgb565_argb4444, inner_remap_rgb565_argb8888 },
						{ 0, inner_remap_rgb888_i8, inner_remap_rgb888_rgb332,
						  inner_remap_rgb888_rgb565, 0, inner_remap_rgb888_argb1555,
						  inner_remap_rgb888_argb4444, inner_remap_rgb888_argb8888 },
						{ 0, inner_remap_argb1555_i8, inner_remap_argb1555_rgb332,
						  inner_remap_argb1555_rgb565, inner_remap_argb1555_rgb888, 0,
						  inner_remap_argb1555_argb4444, inner_remap_argb1555_argb8888 },
						{ 0, inner_remap_argb4444_i8, inner_remap_argb4444_rgb332,
						  inner_remap_argb4444_rgb565, inner_remap_argb4444_rgb888,
						  inner_remap_argb4444_argb1555, 0, inner_remap_argb4444_argb8888 },
						{ 0, inner_remap_argb8888_i8, inner_remap_argb8888_rgb332,
						  inner_remap_argb8888_rgb565, inner_remap_argb8888_rgb888,
						  inner_remap_argb8888_argb1555, inner_remap_argb8888_argb4444, 0 },
					};

					func = funcptrs[sourceidx][destidx];
					if ( func )
						return;
				}

				#endif // ENABLE_SPECIALIZED_C_INNERS

				if ( dest.IsIntensity() )
				{
					MakeBlitMask( red,	(source.IsIntensity() ? source.GetIntensityMask() : source.GetRedMask()), 0x000000ff );
					MakeBlitMask( green,(source.IsIntensity() ? source.GetIntensityMask() : source.GetGreenMask()), 0x000000ff );
					MakeBlitMask( blue,	(source.IsIntensity() ? source.GetIntensityMask() : source.GetBlueMask()), 0x000000ff );
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					MakeBlitMask( intens, 0x000000ff, dest.GetIntensityMask() );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();

					static BlitFunc funcptrs[4][4] =
					{
						{ inner_remap_1rgba_1ia, inner_remap_1rgba_2ia, inner_remap_1rgba_3ia, inner_remap_1rgba_4ia},
						{ inner_remap_2rgba_1ia, inner_remap_2rgba_2ia, inner_remap_2rgba_3ia, inner_remap_2rgba_4ia},
						{ inner_remap_3rgba_1ia, inner_remap_3rgba_2ia, inner_remap_3rgba_3ia, inner_remap_3rgba_4ia},
						{ inner_remap_4rgba_1ia, inner_remap_4rgba_2ia, inner_remap_4rgba_3ia, inner_remap_4rgba_4ia}
					};

					func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
				}
				else
				{
					MakeBlitMask( red, (source.IsIntensity() ? source.GetIntensityMask() : source.GetRedMask()), dest.GetRedMask() );
					MakeBlitMask( green, (source.IsIntensity() ? source.GetIntensityMask() : source.GetGreenMask()), dest.GetGreenMask() );
					MakeBlitMask( blue, (source.IsIntensity() ? source.GetIntensityMask() : source.GetBlueMask()), dest.GetBlueMask() );
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();

					/* // Feature il Debugga Maxima

					char buffy[256];
					sprintf( buffy, "r: %d %d %08x\ng: %d %d %08x\nb: %d %d %08x\n"
									"a: %d %d %08x %08x\ni: %d %d %08x\n"
									"source: %08x %08x %08x %08x\n"
									"dest: %08x %08x %08x %08x\n",
						red.left, red.right, red.mask,
						green.left, green.right, green.mask,
						blue.left, blue.right, blue.mask,
						alpha.left, alpha.right, alpha.mask, alphaor,
						index.left, index.right, index.mask,
						source.GetRedMask(), source.GetGreenMask(), source.GetBlueMask(), source.GetAlphaMask(),
						dest.GetRedMask(), dest.GetGreenMask(), dest.GetBlueMask(), dest.GetAlphaMask() );
					MessageBox(NULL,buffy,"Direct2Direct",MB_OK);
					*/
					static BlitFunc funcptrs[4][4] =
					{
						{ inner_remap_1rgba_1rgba, inner_remap_1rgba_2rgba, inner_remap_1rgba_3rgba, inner_remap_1rgba_4rgba},
						{ inner_remap_2rgba_1rgba, inner_remap_2rgba_2rgba, inner_remap_2rgba_3rgba, inner_remap_2rgba_4rgba},
						{ inner_remap_3rgba_1rgba, inner_remap_3rgba_2rgba, inner_remap_3rgba_3rgba, inner_remap_3rgba_4rgba},
						{ inner_remap_4rgba_1rgba, inner_remap_4rgba_2rgba, inner_remap_4rgba_3rgba, inner_remap_4rgba_4rgba}
					};

					func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
				}
			}
			break;

		case BLIT_STRETCH:
			if ( source.IsIndexed() || source.IsIntensity() )
			{
				static BlitFunc funcptrs[4] =
				{ inner_stretch_1ia, inner_stretch_2ia, inner_stretch_3ia, inner_stretch_4ia };

				func = funcptrs[source.GetBytes() - 1];
			}
			else // Direct
			{
				static BlitFunc funcptrs[4] =
				{ inner_stretch_1rgba, inner_stretch_2rgba, inner_stretch_3rgba, inner_stretch_4rgba };

				func = funcptrs[source.GetBytes() - 1];
			}

			break;

		case BLIT_STRETCH_REMAP:

			if ( source.IsIndexed() && dest.IsIndexed() ) // Indexed -> Indexed is fully supported,
			{											  // except for remapping between different
				MakeBlitMask( red, source.GetIntensityMask(), dest.GetIntensityMask() ); // palettes
				MakeBlitMask( green, 0, 0 );
				MakeBlitMask( blue, 0, 0 );
				MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
				alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();

				static BlitFunc funcptrs[4][4] =
				{
					{ inner_remap_1rgba_1rgba, inner_remap_1rgba_2rgba, inner_remap_1rgba_3rgba, inner_remap_1rgba_4rgba},
					{ inner_remap_2rgba_1rgba, inner_remap_2rgba_2rgba, inner_remap_2rgba_3rgba, inner_remap_2rgba_4rgba},
					{ inner_remap_3rgba_1rgba, inner_remap_3rgba_2rgba, inner_remap_3rgba_3rgba, inner_remap_3rgba_4rgba},
					{ inner_remap_4rgba_1rgba, inner_remap_4rgba_2rgba, inner_remap_4rgba_3rgba, inner_remap_4rgba_4rgba}
				};

				func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
			}
			else if ( source.IsIndexed() ) // Indexed -> !Indexed is fully supported
			{
				if ( dest.IsIntensity() )
				{
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					MakeBlitMask( index, source.GetIntensityMask(), 0x000000ff );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();
				}
				else
				{
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					MakeBlitMask( index, source.GetIntensityMask(), 0x000000ff );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();
				}

				#ifdef ENABLE_SPECIALIZED_C_INNERS

				if (sourceidx == 0 && destidx != -1)
				{
					static BlitFunc funcptrs[4] =
					{ inner_stretch_remap_p8_1ta, inner_stretch_remap_p8_2ta,
					  inner_stretch_remap_p8_3ta, inner_stretch_remap_p8_4ta };

					updatepal = &UpdatePalGenericRemap;

					func = funcptrs[dest.GetBytes() - 1];
					return;
				}

				#endif // ENABLE_SPECIALIZED_C_INNERS

				static BlitFunc funcptrs[4][4] =
				{
					{ inner_stretch_remap_1pa_1ta, inner_stretch_remap_1pa_2ta,
					  inner_stretch_remap_1pa_3ta, inner_stretch_remap_1pa_4ta },
					{ inner_stretch_remap_2pa_1ta, inner_stretch_remap_2pa_2ta,
					  inner_stretch_remap_2pa_3ta, inner_stretch_remap_2pa_4ta },
					{ inner_stretch_remap_3pa_1ta, inner_stretch_remap_3pa_2ta,
					  inner_stretch_remap_3pa_3ta, inner_stretch_remap_3pa_4ta },
					{ inner_stretch_remap_4pa_1ta, inner_stretch_remap_4pa_2ta,
					  inner_stretch_remap_4pa_3ta, inner_stretch_remap_4pa_4ta }
				};

				updatepal = &UpdatePalGenericRemap;

				func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
			}
			else // !Indexed -> !Indexed is fully supported
			{

				#ifdef ENABLE_SPECIALIZED_C_INNERS

				if (sourceidx != -1 && destidx != -1)
				{
					static BlitFunc funcptrs[8][8] =
					{
						{ 0, 0, 0, 0, 0, 0, 0, 0 },
						{ 0, 0, inner_stretch_remap_i8_rgb332, inner_stretch_remap_i8_rgb565,
						  inner_stretch_remap_i8_rgb888, inner_stretch_remap_i8_argb1555,
						  inner_stretch_remap_i8_argb4444, inner_stretch_remap_i8_argb8888 },
						{ 0, inner_stretch_remap_rgb332_i8, 0, inner_stretch_remap_rgb332_rgb565,
						  inner_stretch_remap_rgb332_rgb888, inner_stretch_remap_rgb332_argb1555,
						  inner_stretch_remap_rgb332_argb4444, inner_stretch_remap_rgb332_argb8888 },
						{ 0, inner_stretch_remap_rgb565_i8, inner_stretch_remap_rgb565_rgb332, 0,
						  inner_stretch_remap_rgb565_rgb888, inner_stretch_remap_rgb565_argb1555,
						  inner_stretch_remap_rgb565_argb4444, inner_stretch_remap_rgb565_argb8888 },
						{ 0, inner_stretch_remap_rgb888_i8, inner_stretch_remap_rgb888_rgb332,
						  inner_stretch_remap_rgb888_rgb565, 0, inner_stretch_remap_rgb888_argb1555,
						  inner_stretch_remap_rgb888_argb4444, inner_stretch_remap_rgb888_argb8888 },
						{ 0, inner_stretch_remap_argb1555_i8, inner_stretch_remap_argb1555_rgb332,
						  inner_stretch_remap_argb1555_rgb565, inner_stretch_remap_argb1555_rgb888, 0,
						  inner_stretch_remap_argb1555_argb4444, inner_stretch_remap_argb1555_argb8888 },
						{ 0, inner_stretch_remap_argb4444_i8, inner_stretch_remap_argb4444_rgb332,
						  inner_stretch_remap_argb4444_rgb565, inner_stretch_remap_argb4444_rgb888,
						  inner_stretch_remap_argb4444_argb1555, 0, inner_stretch_remap_argb4444_argb8888 },
						{ 0, inner_stretch_remap_argb8888_i8, inner_stretch_remap_argb8888_rgb332,
						  inner_stretch_remap_argb8888_rgb565, inner_stretch_remap_argb8888_rgb888,
						  inner_stretch_remap_argb8888_argb1555, inner_stretch_remap_argb8888_argb4444, 0 },
					};

					func = funcptrs[sourceidx][destidx];
					if ( func )
						return;
				}

				#endif // ENABLE_SPECIALIZED_C_INNERS

				if ( dest.IsIntensity() )
				{
					MakeBlitMask( red, (source.IsIntensity() ? source.GetIntensityMask() : source.GetGreenMask()), 0x000000ff );
					MakeBlitMask( green, (source.IsIntensity() ? source.GetIntensityMask() : source.GetGreenMask()), 0x000000ff );
					MakeBlitMask( blue, (source.IsIntensity() ? source.GetIntensityMask() : source.GetBlueMask()), 0x000000ff );
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					MakeBlitMask( intens, 0x000000ff, dest.GetIntensityMask() );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();

					static BlitFunc funcptrs[4][4] =
					{
						{ inner_stretch_remap_1rgba_1ia, inner_stretch_remap_1rgba_2ia,
						  inner_stretch_remap_1rgba_3ia, inner_stretch_remap_1rgba_4ia },
						{ inner_stretch_remap_2rgba_1ia, inner_stretch_remap_2rgba_2ia,
						  inner_stretch_remap_2rgba_3ia, inner_stretch_remap_2rgba_4ia },
						{ inner_stretch_remap_3rgba_1ia, inner_stretch_remap_3rgba_2ia,
						  inner_stretch_remap_3rgba_3ia, inner_stretch_remap_3rgba_4ia },
						{ inner_stretch_remap_4rgba_1ia, inner_stretch_remap_4rgba_2ia,
						  inner_stretch_remap_4rgba_3ia, inner_stretch_remap_4rgba_4ia }
					};

					func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
				}
				else
				{

					MakeBlitMask( red, (source.IsIntensity() ? source.GetIntensityMask() : source.GetRedMask()), dest.GetRedMask() );
					MakeBlitMask( green, (source.IsIntensity() ? source.GetIntensityMask() : source.GetGreenMask()), dest.GetGreenMask() );
					MakeBlitMask( blue, (source.IsIntensity() ? source.GetIntensityMask() : source.GetBlueMask()), dest.GetBlueMask() );
					MakeBlitMask( alpha, source.GetAlphaMask(), dest.GetAlphaMask() );
					alphaor = (source.GetAlphaMask() ? 0 : 0xffffffff) & dest.GetAlphaMask();

					static BlitFunc funcptrs[4][4] =
					{
						{ inner_stretch_remap_1rgba_1rgba, inner_stretch_remap_1rgba_2rgba,
						  inner_stretch_remap_1rgba_3rgba, inner_stretch_remap_1rgba_4rgba },
						{ inner_stretch_remap_2rgba_1rgba, inner_stretch_remap_2rgba_2rgba,
						  inner_stretch_remap_2rgba_3rgba, inner_stretch_remap_2rgba_4rgba },
						{ inner_stretch_remap_3rgba_1rgba, inner_stretch_remap_3rgba_2rgba,
						  inner_stretch_remap_3rgba_3rgba, inner_stretch_remap_3rgba_4rgba },
						{ inner_stretch_remap_4rgba_1rgba, inner_stretch_remap_4rgba_2rgba,
						  inner_stretch_remap_4rgba_3rgba, inner_stretch_remap_4rgba_4rgba }
					};

					func = funcptrs[source.GetBytes() - 1][dest.GetBytes() - 1];
				}
			}
			break;

		case BLIT_STRETCH_BILINEAR:

			#ifdef ENABLE_SPECIALIZED_X86_INNERS

			if ( sourceidx != -1 )
			{

				#ifdef ENABLE_SPECIALIZED_X86_MMX_INNERS

				static BlitFunc funcptrs[2][8] =
					{
						{ 0, 0, 0, inner_stretch_rgb565_bilinear_x86,
						  0, inner_stretch_argb1555_bilinear_x86,
						  inner_stretch_argb4444_bilinear_x86, 0 },
						{ 0, 0, 0, inner_stretch_rgb565_bilinear_x86,
						  inner_stretch_rgb888_bilinear_x86_mmx, inner_stretch_argb1555_bilinear_x86,
						  inner_stretch_argb4444_bilinear_x86, inner_stretch_argb8888_bilinear_x86_mmx }
					};
					func = funcptrs[mmxfound][sourceidx];

				#else // ENABLE_SPECIALIZED_X86_MMX_INNERS

				static BlitFunc funcptrs[8] =
					{ 0, 0, 0, inner_stretch_rgb565_bilinear_x86,
					  0, inner_stretch_argb1555_bilinear_x86,
					  inner_stretch_argb4444_bilinear_x86, 0 };
					func = funcptrs[sourceidx];

				#endif // ENABLE_SPECIALIZED_X86_MMX_INNERS

					if ( func )
						return;
			}

			#endif // ENABLE_SPECIALIZED_X86_INNERS

			#ifdef ENABLE_SPECIALIZED_C_INNERS

			if ( sourceidx != -1 )
			{
					static BlitFunc funcptrs[8] =
					{ 
						0, inner_stretch_i8_bilinear, 0, inner_stretch_rgb565_bilinear,
						inner_stretch_rgb888_bilinear, inner_stretch_argb1555_bilinear,
						inner_stretch_argb4444_bilinear, inner_stretch_argb8888_bilinear 
					};

					func = funcptrs[sourceidx];

					if ( func )
						return;
			}

			#endif // ENABLE_SPECIALIZED_C_INNERS

			if ( source.IsIndexed() )
			{
				convtype &= ~BLIT_BILINEAR;
				static BlitFunc funcptrs[4] =
				{ 
					inner_stretch_1ia, inner_stretch_2ia, 
					inner_stretch_3ia, inner_stretch_4ia 
				};

				func = funcptrs[source.GetBytes() - 1];
			}
			else if ( source.IsIntensity() )
			{
				MakeBlitMaskBilinear( intens, source.GetIntensityMask() );
				MakeBlitMaskBilinear( alpha,  source.GetAlphaMask() );

				static BlitFunc funcptrs[4] =
				{ inner_stretch_1ia_bilinear, inner_stretch_2ia_bilinear, 
				  inner_stretch_3ia_bilinear, inner_stretch_4ia_bilinear };

				func = funcptrs[source.GetBytes() - 1];
			}
			else // Direct
			{
				MakeBlitMaskBilinear( red,   source.GetRedMask() );
				MakeBlitMaskBilinear( green, source.GetGreenMask() );
				MakeBlitMaskBilinear( blue,  source.GetBlueMask() );
				MakeBlitMaskBilinear( alpha, source.GetAlphaMask() );
				static BlitFunc funcptrs[4] =
				{ 
					inner_stretch_1rgba_bilinear, inner_stretch_2rgba_bilinear, 
					inner_stretch_3rgba_bilinear, inner_stretch_4rgba_bilinear 
				};

				func = funcptrs[source.GetBytes() - 1];
			}

			break;
	}

}

Blitter::~Blitter()
{
}


static inline int32 HashValue(const PixelFormat& pxf)
{
	return   HighestBit( pxf.GetRedMask() ) 
		   ^ HighestBit( pxf.GetGreenMask() )
		   ^ HighestBit( pxf.GetBlueMask() )
		   ^ HighestBit( pxf.GetAlphaMask() )
		   ^ pxf.GetBytes();
}


static Blitter* GetBlitter(const PixelFormat& dest, const PixelFormat& src, int type)
{
	//  bilinear resampling is not available when remapping
	if ( type & BLIT_REMAP )
		type &= ~BLIT_BILINEAR;
	
	int idx = ((((HashValue( src ) << 1) ^ HashValue( dest )) & ((BlitHashSize >> 2) - 1)) 
			  ^ ((type & ~BLIT_BILINEAR) << (BlitHashBits - 2))) & (BlitHashSize - 1);

	// is suitable blitter already created?
	if ( blitters[idx] && blitters[idx]->source == src && blitters[idx]->dest == dest
		 && blitters[idx]->convtype == type )
	{
		// update Indexed -> Direct remapping table if necessary
		if ( blitters[idx]->updatepal && !blitters[idx]->updatepal( *blitters[idx], dest, src ) )
			return NULL;

		return blitters[idx];
	}

	// create a new blitter
	Blitter* blitter = new Blitter( dest, src, type );
	if ( !blitter )
		return NULL;

	// update Indexed -> !Indexed remapping table if necessary
	if ( blitter->updatepal && !blitter->updatepal(*blitter,dest,src) )
	{
		delete blitter;
		return NULL;
	}

	// insert blitter into table
	if ( blitters[idx] )
		delete blitters[idx];

	blitters[idx] = blitter;
	return blitter;
}


// ===================================================
// clean innerloops
// ===================================================

typedef void (*ClearFunc)(void* src, int count, uint32 orv, uint32 mask);


template <typename T>
inline void clear(T* src, int count, uint32 orv, uint32 mask)
{
	while ( count-- )
	{
		*src++ = (*src & mask) | orv;
	}
}


static void inner_clear1(void* src, int count, uint32 orv, uint32 mask)
{
	clear<uint8>(reinterpret_cast<uint8*>(src),count,orv,mask);
}

static void inner_clear2(void* src, int count, uint32 orv, uint32 mask)
{
	clear<uint16>(reinterpret_cast<uint16*>(src),count,orv,mask);
}

static void inner_clear3(void* src, int count, uint32 orv, uint32 mask)
{
	uint8* src2 = reinterpret_cast<uint8*>(src);
	while ( count-- )
	{
		write24(src2, (read24(src2) & mask) | orv );
		src2 += 3;
	}
}

static void inner_clear4(void* src, int count, uint32 orv, uint32 mask)
{
	clear<uint32>(reinterpret_cast<uint32*>(src),count,orv,mask);
}


// ===================================================
// flip innerloops
// ===================================================

typedef void (*FlipFunc)(void* line0, void* line1, int count);


template <typename T>
inline void flip(T* rawline0, T* rawline1, int count)
{
	while ( count-- )
	{
		T v = *rawline0;
		*rawline0++ = *rawline1;
		*rawline1++ = v;
	}
}


static void inner_flip1(void* line0, void* line1, int count)
{
	flip<uint8>(reinterpret_cast<uint8*>(line0),reinterpret_cast<uint8*>(line1),count);
}


static void inner_flip2(void* line0, void* line1, int count)
{
	flip<uint16>(reinterpret_cast<uint16*>(line0),reinterpret_cast<uint16*>(line1),count);
}


static void inner_flip3(void* line0, void* line1, int count)
{
	flip<uint8>(reinterpret_cast<uint8*>(line0),reinterpret_cast<uint8*>(line1),count*3);
}


static void inner_flip4(void* line0, void* line1, int count)
{
	flip<uint32>(reinterpret_cast<uint32*>(line0),reinterpret_cast<uint32*>(line1),count);
}


// ===================================================
// surface interface
// ===================================================

void Surface::ClearImage(const color32& color, ClearMode mode)
{

	// delegate the workload for floating-point blitting module
	// if pixelformat is floating-point
	if ( format.IsFloat() )
	{
		ClearImageFloat(color,mode);
		return;
	}


	ClearFunc clearfuncs[4] =
	{
		inner_clear1,
		inner_clear2,
		inner_clear3,
		inner_clear4
	};

	// choose clear function
	ClearFunc func = clearfuncs[format.GetBytes() - 1];

	uint32 mask,orv;

	if ( format.IsIndexed() )
	{
		int bestdist = 256 * 256 * 3;
		int bestidx = 0;
		Color32* palette = format.GetPalette();

		for ( int i=0; i<256; ++i )
		{
			int dr = palette[i].r - color.r;
			int dg = palette[i].g - color.g;
			int db = palette[i].b - color.b;
			int newdist = dr * dr + dg * dg + db * db;
			if ( newdist < bestdist )
			{
				bestdist = newdist;
				bestidx = i;
			}
		}

		mask = ((mode & CLEAR_COLOR) ? format.GetIntensityMask() : 0) |
			   ((mode & CLEAR_ALPHA) ? format.GetAlphaMask() : 0);

		BlitMask maski,maska;
		MakeBlitMask(maski, 0x000000ff, format.GetIntensityMask());
		MakeBlitMask(maska, 0xff000000, format.GetAlphaMask());
		orv = ((((bestidx >> maski.right) << maski.left) & maski.mask)
			  | (((((color & 0xff000000) >> 24) >> maska.right) << maska.left)
			       & maska.mask)) & mask;

	}
	else if ( format.IsIntensity() )
	{
		mask = ((mode & CLEAR_COLOR) ? format.GetIntensityMask() : 0) |
			   ((mode & CLEAR_ALPHA) ? format.GetAlphaMask() : 0);
		BlitMask maski, maska;
		MakeBlitMask(maski, 0x000000ff, format.GetIntensityMask());
		MakeBlitMask(maska, 0xff000000, format.GetAlphaMask());
		uint32 c = palmono[(color & 0xff0000) >> 16].r
						 + palmono[(color & 0xff00) >> 8].g
						 + palmono[color & 0xff].b;
		orv = ((((c >> maski.right) << maski.left) & maski.mask)
			  | (((c >> maska.right) << maska.left) & maska.mask)) & mask;
	}
	else // Direct
	{
		mask = ((mode & CLEAR_COLOR) ? (format.GetRedMask() | format.GetGreenMask() | format.GetBlueMask()) : 0) |
			   ((mode & CLEAR_ALPHA) ? (format.GetAlphaMask()) : 0);
		BlitMask maskr, maskg, maskb, maska;
		MakeBlitMask(maskr, palformat.GetRedMask(),   format.GetRedMask());
		MakeBlitMask(maskg, palformat.GetGreenMask(), format.GetGreenMask());
		MakeBlitMask(maskb, palformat.GetBlueMask(),  format.GetBlueMask());
		MakeBlitMask(maska, 0xff000000,               format.GetAlphaMask());
		uint32 c = color;
		orv = ((((c >> maskr.right) << maskr.left) & maskr.mask)
			  | (((c >> maskg.right) << maskg.left) & maskg.mask)
			  | (((c >> maskb.right) << maskb.left) & maskb.mask)
			  | (((c >> maska.right) << maska.left) & maska.mask)) & mask;
	}

	// clear
	uint8* yimage = image;
	mask = ~mask;
	for ( int y=0; y<height; ++y )
	{
		func(yimage,width,orv,mask);
		yimage += pitch;
	}
}


void Surface::BlitImage(const Surface& source, BlitMode mode)
{

	static bool init = false;
	if ( !init )
	{
		BitmapBlitterInitialize();
		init = true;
	}
	
	// validate surfaces
	if ( !(width && height && image && source.width && source.height && source.image) )
		return;

	bool resize = width != source.width || height != source.height;

	// choose blitter mode
	int type = 0;
	if ( source.format != format )				type |= BLIT_REMAP;
	if ( mode==BLIT_SCALE && resize )			type |= BLIT_STRETCH;
	if ( mode==BLIT_BILINEAR_SCALE && resize )	type |= BLIT_BILINEAR | BLIT_STRETCH;

	// plain row-by-row copying if !conversion && !stretching
	if ( !type )
	{
		int minwidth  = std::min(width,source.width);
		int minheight = std::min(height,source.height);

		int length = minwidth * format.GetBytes();
		uint8* s = source.image;
		uint8* d = image;
		for ( int y=0; y<minheight; ++y )
		{
			memcpy(d,s,length);
			s += source.pitch;
			d += pitch;
		}
	}
	else
	{

		// delegate the workload for floating-point blitting module
		// if either source or destination pixelformat is floating-point
		if ( format.IsFloat() || source.format.IsFloat() )
		{
			BlitImageFloat(source,mode);
			return;
		}


		// get a suitable blitter
		Blitter* blitter = GetBlitter(format,source.format,type);
		if ( !blitter )
			return;

		InnerInfo innerinfo;
		innerinfo.dest = reinterpret_cast<uint8*>(image);

		// stretching
		if ( blitter->convtype & BLIT_STRETCH )
		{
			uint32 vstep,v;

			// bilinear stretch has special UV calculations
			if ( blitter->convtype & BLIT_BILINEAR )
			{
				if ( width < source.width )
				{
					innerinfo.ustep  = (source.width << 16) / width;
					innerinfo.ustart = (innerinfo.ustep >> 1) - 0x8000;
				}
				else
				{
					int div = width - 1;
					if ( !div ) div = 1;

					innerinfo.ustep  = ((source.width - 1) << 16) / div;
					innerinfo.ustart = 0;
				}

				if ( height < source.height )
				{
					vstep = (source.height << 16) / height;
					v = (vstep >> 1) - 0x8000;
				}
				else
				{
					int div = height - 1;
					if ( !div ) div = 1;

					vstep = ((source.height - 1) << 16) / div;
					v = 0;
				}
			}
			else // !bilinear
			{
				innerinfo.ustep = (source.width << 16) / width;
				innerinfo.ustart = (innerinfo.ustep >> 1);
				vstep = (source.height << 16) / height;
				v = vstep >> 1;
			}

			innerinfo.width = width;
			
			for ( int y=0; y<height; ++y )
			{
				innerinfo.src = reinterpret_cast<uint8*>(source.image) + ((v >> 16) * source.pitch);
				innerinfo.src2 = innerinfo.src + source.pitch;

				// clamp lower scanline - just in case
				// the delta calculation above should ensure this is not required,
				// but, rgb888_bilinear innerloops pagefault -- fix this performance/quality
				// caveat sometime. ;-)
				if ( y == (height - 1) )
					innerinfo.src2 = innerinfo.src;

				innerinfo.vfrac = v & 0xffff;
				blitter->Blit(&innerinfo);
				v += vstep;
				innerinfo.dest += pitch;
			}
		}
		else // conversion && !stretching
		{
			int minwidth  = std::min(width,source.width);
			int minheight = std::min(height,source.height);

			innerinfo.width = minwidth;

			innerinfo.src = reinterpret_cast<uint8*>(source.image);
			for ( int y=0; y<minheight; ++y )
			{
				blitter->Blit(&innerinfo);
				innerinfo.dest += pitch;
				innerinfo.src += source.pitch;
			}
		}
	}
}


void Surface::FlipImageY()
{
	if ( !width || !height || !image )
		return;

	uint8* top = image;
	uint8* bottom = image + (height - 1) * pitch;

	// setup innerloop
	FlipFunc func;
	int samples;

	if ( format.IsFloat16() )
	{
		func = inner_flip2;
		samples = width * format.GetBytes() / sizeof(uint16);
	}
	else
	if ( format.IsFloat32() )
	{
		func = inner_flip4;
		samples = width * format.GetBytes() / sizeof(uint32);
	}
	else
	if ( format.IsInteger() )
	{
		switch ( format.GetBytes() )
		{
			case 1: func = inner_flip1; break;
			case 2: func = inner_flip2; break;
			case 3: func = inner_flip3; break;
			case 4: func = inner_flip4; break;
			default: return;
		}
		samples = width;
	}
	else
	{
		// throw as bizarre exception we can think of ; this situation is "impossible"
		// (why are we then even bothering to test for it?)
		// (because it would amuse Jukka propably, when he ever gets this one and checks the sourcecode.. ;-)
		PRCORE_EXCEPTION("MegaTrap2000. Just kidding, unknown PixelFormat datatype.");
	}

	// call innerloop	
	while ( top < bottom )
	{
		func(top,bottom,samples);
		
		top += pitch;
		bottom -= pitch;
	}
}
