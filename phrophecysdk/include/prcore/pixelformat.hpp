/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_PIXELFORMAT_HPP
#define PRCORE_PIXELFORMAT_HPP


#include <cassert>
#include <cstddef>
#include "configure.hpp"
#include "exception.hpp"
#include "integer.hpp"
#include "color32.hpp"


namespace prcore
{

	/*

	PixelFormat symmetry restrictions:

	---------------------------
	component format: integer
	number of color bits supported: 8,16,24,32
	masktype: bitmask
	---------------------------
	component format: float16
	number of color bits supported: 16,32,48,64,80,96,112,128
	masktype: float16 component
	---------------------------
	component format: float32
	number of color bits supported: 32,64,96,128
	masktype: float32 component
	---------------------------

	Abstract:
	
	The masktype for each component format is determined by the smallest "atom" size,
	for integer pixelformats smallest atom is a single bit, for floating-point pixelformats
	the smallest atom is single floating-point value.

	There is also practical limit of using 32 bit masks, which limits the integer pixelformats
	to 32 bits per pixel. It would be possible to extend the system to use bytemask for pixelformats
	which are wider than 32 bits per pixel, however, this would break symmetry and floating-point formats
	satisfy the range and precision for wider pixelformats. This unfortunately means some DirectX specific
	64 bits per pixel formats are not possible to support with this system. We feel that this is reasonable
	price to pay for consistency.

	Practical Notes:
	
	This system understands only grayscale, indexed and rgb color with optional alpha channel. YUV and other
	color schemes are not directly supported, however, such formats are possible to store using our system.
	Simply choose grayscale format and set bitmask accordingly, this allows our internal blitters to transfer
	pixels unchanged when there are no pixelformat conversions taking place. This has been adequate for
	practical purposes. The reasoning for dropping some normal vector, YUV and other more exotic formats
	is that we want to handle only COLOR formats. YUV is a color format, but there is no real standard *1) yet
	in existence how precisely the bits translate into RGB color, so supporting this is quite a bit difficult
	in practise. Therefore, we leave handling such formats as exercise to the reader- and- hardware, as such
	functionality is most desirable in hardware accelerated environment. Single most widespread exception to
	this rule is the JPEG compression standard, but this alone does not satisfy requirement for support.

	*1) If environment and/or API is known this statement does not necessarily hold true, but there is too
	    much diversity in different hardware, operating systems, API's and so on to develop generic rules,
	    or symmetry we could take advantage of in pixelformat definition. In otherwords, the only practical
	    choise at the moment would be to enumerate YUV format and keep adding support for more formats
	    as they come along, for this we respectfully decline.

	*/


	enum
	{
		PIXELFORMAT_INTEGER		= 0x80000000,
		PIXELFORMAT_FLOAT16		= 0x40000000,
		PIXELFORMAT_FLOAT32		= 0x20000000,
	};


	class PixelFormat
	{
		private:

		enum
		{
			PIXELFORMAT_DIRECTCOLOR	= 0x10000000,
			PIXELFORMAT_INTENSITY	= 0x08000000,
			PIXELFORMAT_INDEX8		= 0x04000000,
			PIXELFORMAT_VALUEMASK	= 0x0000ffff,
		};

		uint32		mbits;		// type flags & color bits
		uint32		rmask;		// red mask
		uint32		gmask;		// green mask
		uint32		bmask;		// blue/intensity mask
		uint32		amask;		// alpha mask
		color32		mcount;		// component bitcount
		color32		moffset;	// component offset
		color32*	mpalette;	// palette pointer

		void ComputeMask(uint32 mask, uint8& count, uint8& shift) const
		{
			uint8 ulow = static_cast<uint8>(LowestBit(mask));
			shift      = static_cast<uint8>(HighestBit(mask));
			count      = static_cast<uint8>(mask ? (shift - ulow + 1) : 0);
		}

		public:

		PixelFormat()
		: mbits(0),rmask(0),gmask(0),bmask(0),amask(0),mcount(0),moffset(0),mpalette(NULL)
		{
		}

		PixelFormat(uint32 bits, uint32 r, uint32 g, uint32 b, uint32 a, uint32 type = PIXELFORMAT_INTEGER)
		: mbits(bits|type|PIXELFORMAT_DIRECTCOLOR),rmask(r),gmask(g),bmask(b),amask(a),mpalette(NULL)
		{
			assert( (bits % 8) == 0 );

			if ( type & PIXELFORMAT_FLOAT16 && (bits % 16) != 0 )
				PRCORE_EXCEPTION("Incorrect floating-point PixelFormat declaration.");

			if ( type & PIXELFORMAT_FLOAT32 && (bits % 32) != 0 )
				PRCORE_EXCEPTION("Incorrect floating-point PixelFormat declaration.");

			ComputeMask(r,mcount.r,moffset.r);
			ComputeMask(g,mcount.g,moffset.g);
			ComputeMask(b,mcount.b,moffset.b);
			ComputeMask(a,mcount.a,moffset.a);
		}

		PixelFormat(uint32 bits, uint32 i, uint32 a, uint32 type = PIXELFORMAT_INTEGER)
		: mbits(bits|type|PIXELFORMAT_INTENSITY),rmask(0),gmask(0),bmask(i),amask(a),mcount(0),moffset(0),mpalette(NULL)
		{
			assert( (bits % 8) == 0 );

			if ( type & PIXELFORMAT_FLOAT16 && (bits % 16) != 0 )
				PRCORE_EXCEPTION("Incorrect floating-point PixelFormat declaration.");

			if ( type & PIXELFORMAT_FLOAT32 && (bits % 32) != 0 )
				PRCORE_EXCEPTION("Incorrect floating-point PixelFormat declaration.");

			ComputeMask(i,mcount.b,moffset.b);
			ComputeMask(a,mcount.a,moffset.a);
		}

		PixelFormat(uint32 bits, uint32 i, uint32 a, const color32 palette[])
		: mbits(bits|PIXELFORMAT_INTEGER|PIXELFORMAT_INDEX8),rmask(0),gmask(0),bmask(i),amask(a),mcount(0),moffset(0),mpalette(NULL)
		{
			assert( (bits % 8) == 0 );

			mpalette = new color32[256];
			if ( palette )
			{
				for ( int i=0; i<256; ++i )
					mpalette[i] = palette[i];
			}

			ComputeMask(i,mcount.b,moffset.b);
			ComputeMask(a,mcount.a,moffset.a);
		}

		PixelFormat(const PixelFormat& pxf)
		: mpalette(NULL)
		{
			*this = pxf;
		}

		~PixelFormat()
		{
			delete[] mpalette;
		}

		PixelFormat& operator = (const PixelFormat& pxf)
		{
			mbits   = pxf.mbits;
			rmask   = pxf.rmask;
			gmask   = pxf.gmask;
			bmask   = pxf.bmask;
			amask   = pxf.amask;
			mcount  = pxf.mcount;
			moffset = pxf.moffset;

			if ( pxf.mpalette )
			{
				if ( !mpalette ) 
					mpalette = new color32[256];

				for ( int i=0; i<256; ++i )
					mpalette[i] = pxf.mpalette[i];
			}
			else
			{
				delete[] mpalette;
				mpalette = NULL;
			}

			return *this;
		}

		bool operator == (const PixelFormat& format) const
		{
			if ( (rmask != format.rmask) || (gmask != format.gmask) ||
				(bmask != format.bmask) || (amask != format.amask) )
				return false;

			if ( mbits != format.mbits ) 
				return false;

			if ( (mpalette==NULL) != (format.mpalette==NULL) ) 
				return false;

			return true;
		}

		bool operator != (const PixelFormat& format) const
		{
			return !(*this==format);
		}

		bool IsIntensity() const
		{
			return (mbits & PIXELFORMAT_INTENSITY) != 0;
		}

		bool IsIndexed() const
		{
			return (mbits & PIXELFORMAT_INDEX8) != 0;
		}

		bool IsDirectColor() const
		{
			return (mbits & PIXELFORMAT_DIRECTCOLOR) != 0;
		}

		bool IsInteger() const
		{
			return (mbits & PIXELFORMAT_INTEGER) != 0;
		}

		bool IsFloat() const
		{
			return (mbits & PIXELFORMAT_INTEGER) == 0;
		}

		bool IsFloat16() const
		{
			return (mbits & PIXELFORMAT_FLOAT16) != 0;
		}

		bool IsFloat32() const
		{
			return (mbits & PIXELFORMAT_FLOAT32) != 0;
		}

		uint32 GetBits() const
		{
			return mbits & PIXELFORMAT_VALUEMASK;
		}

		uint32 GetBytes() const
		{
			return (mbits & PIXELFORMAT_VALUEMASK) >> 3;
		}

		color32* GetPalette() const
		{
			return mpalette;
		}

		void SetPalette(const color32 palette[])
		{
			if ( !palette )
				return;

			if ( !IsIndexed() )
				PRCORE_EXCEPTION("PixelFormat is not indexed.");
		
			for ( int i=0; i<256; ++i )
				mpalette[i] = palette[i];
		}

		uint32 GetRedMask() const
		{
			return rmask;
		}

		uint32 GetGreenMask() const
		{
			return gmask;
		}

		uint32 GetBlueMask() const
		{
			return bmask;
		}

		uint32 GetAlphaMask() const
		{
			return amask;
		}

		uint32 GetIntensityMask() const
		{
			return bmask;
		}

		uint32 GetIndexMask() const
		{
			return bmask;
		}

		uint8 GetRedBits() const
		{
			return mcount.r;
		}

		uint8 GetGreenBits() const
		{
			return mcount.g;
		}

		uint8 GetBlueBits() const
		{
			return mcount.b;
		}

		uint8 GetAlphaBits() const
		{
			return mcount.a;
		}

		uint8 GetIntensityBits() const
		{
			return mcount.b;
		}

		uint8 GetIndexBits() const
		{
			return mcount.b;
		}

		uint8 GetRedOffset() const
		{
			return moffset.r;
		}

		uint8 GetGreenOffset() const
		{
			return moffset.g;
		}

		uint8 GetBlueOffset() const
		{
			return moffset.b;
		}

		uint8 GetAlphaOffset() const
		{
			return moffset.a;
		}

		uint8 GetIntensityOffset() const
		{
			return moffset.b;
		}

		uint8 GetIndexOffset() const
		{
			return moffset.b;
		}
	};

	// predefined formats

	#define PIXELFORMAT_PALETTE8(p)     PixelFormat(8,0xff,0x00,reinterpret_cast<color32*>(p))
	#define PIXELFORMAT_INTENSITY8      PixelFormat(8,0xff,0x00)
	#define PIXELFORMAT_RGB332			PixelFormat(8,0xe0,0x1c,0x03,0x00)
	#define PIXELFORMAT_RGB565          PixelFormat(16,0x0000f800,0x000007e0,0x0000001f,0x00000000)
	#define PIXELFORMAT_RGB888          PixelFormat(24,0x00ff0000,0x0000ff00,0x000000ff,0x00000000)
	#define PIXELFORMAT_ARGB1555        PixelFormat(16,0x00007c00,0x000003e0,0x0000001f,0x00008000)
	#define PIXELFORMAT_ARGB4444        PixelFormat(16,0x00000f00,0x000000f0,0x0000000f,0x0000f000)
	#define PIXELFORMAT_ARGB8888        PixelFormat(32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000)

} // namespace prcore


#endif