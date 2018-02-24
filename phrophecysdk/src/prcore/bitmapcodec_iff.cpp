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
		BitmapCodec "iff"

	revision history:
		Jul/01/1999 - Ilkka Pelkonen - initial revision
		Oct/15/2002 - Jukka Liimatta - rewrite to use current stream/codec system
*/
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// codec
// =================================================

static void rle(uint8* image, uint8* buffer, int size)
{
	while ( size > 0 )
	{
		int8 n = *buffer++;
		if ( n >= 0 )
		{
			int count = n + 1; size -= count;
			while ( count-- )
			{
				*image++ = *buffer++;
			}
		}
		else
		{
			uint8 v = *buffer++;
			int count = 1 - n; size -= count;
			while ( count-- )
			{
				*image++ = v;
			}
		}
	}
}


static void p2c_raw(uint8* dest, uint8* workptr, int width, int height, int nplanes)
{
	int bpp = (nplanes + 7) >> 3;
	int lineskip = ((width + 15) >> 4) << 1;

	for ( int y=0; y<height; ++y )
	{
		uint32 bitmask = 0x80;
		uint8* workptr2 = workptr;
		for ( int x=0; x<width; ++x )
		{
			uint32 v = 0;
			uint32 colorbit = 1;
			uint8* workptr3 = workptr2;
			for ( int plane=0; plane<nplanes; ++plane )
			{
				if ( *workptr3 & bitmask )
				{
					v |= colorbit;
				}
				workptr3 += lineskip;
				colorbit += colorbit;
			}

			switch ( bpp )
			{
				case 4:
					*dest++ = (v >> 16) & 0xff;
					*dest++ = (v >> 8)& 0xff;
					*dest++ = v & 0xff;
					*dest++ = v >> 24;
					break;

				case 3: *dest++ = (v >> 16) & 0xff;
				case 2: *dest++ = (v >> 8)& 0xff;
				case 1: *dest++ = v & 0xff;
			}

			bitmask >>= 1;
			if ( !bitmask )
			{
				bitmask = 0x80;
				++workptr2;
			}
		}
		workptr += lineskip * nplanes;
	}
}


static void p2c_ham(uint8* dest, uint8* workptr, int width, int height, int nplanes, Color32* palette)
{
	bool hamcode2b = (nplanes == 6 || nplanes == 8);
	int hamscale = 8 - (nplanes - (hamcode2b ? 2 : 1));

	int lineskip = ((width + 15) >> 4) << 1;

	for ( int y=0; y<height; ++y )
	{
		uint32 r = 0;
		uint32 g = 0;
		uint32 b = 0;

		uint32 bitmask = 0x80;
		uint8* workptr2 = workptr;
		for ( int x=0; x<width; ++x )
		{
			uint8* workptr3 = workptr2;

			// read value
			uint32 v = 0;
			uint32 colorbit = 1;

			for ( int plane=2; plane<nplanes; ++plane )
			{
				if ( *workptr3 & bitmask )
				{
					v |= colorbit;
				}
				workptr3 += lineskip;
				colorbit += colorbit;
			}

			// read hamcode
			uint32 hamcode = 0;

			if ( *workptr3 & bitmask ) hamcode = 1;
			workptr3 += lineskip;
			if ( hamcode2b )
			{
				if ( *workptr3 & bitmask ) hamcode |= 2;
				workptr3 += lineskip;
			}

			// hold-and-modify
			switch ( hamcode )
			{
				case 0:
					r = palette[v].r;
					g = palette[v].g;
					b = palette[v].b;
					break;

				case 1:	b = v << hamscale; break;
				case 2:	r = v << hamscale; break;
				case 3: g = v << hamscale; break;
			}

			*dest++ = b;
			*dest++ = g;
			*dest++ = r;

			bitmask >>= 1;
			if ( !bitmask )
			{
				bitmask = 0x80;
				++workptr2;
			}
		}
		workptr += lineskip * nplanes;
	}
}


static bool iff_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read header
	uint32 v0 = ReadBigEndian<uint32>(stream);
	            ReadBigEndian<uint32>(stream);
	uint32 v1 = ReadBigEndian<uint32>(stream);

	// is stream iff?
	if ( v0 != PRCORE_BIGCODE32('F','O','R','M') )
		return false;

	// is iff "ILBM" format?
	if ( v1 != PRCORE_BIGCODE32('I','L','B','M') &&
		 v1 != PRCORE_BIGCODE32('P','B','M',' ') )
		return false;

	// header
	Color32* palette = NULL;
	int palettesize = 0;
	uint8* image = NULL;
	int imagesize = 0;

	bool ham = false;
	bool ehb = false;
	uint8 nplanes = 0;
	uint8 compression = 0;
	int width = 0;
	int height = 0;

	// read subchunks
	while ( !stream.IsEOS() )
	{
		// read chunk header
		uint32 id = ReadBigEndian<uint32>(stream);
		uint32 size = (ReadBigEndian<uint32>(stream) + 1) & 0xfffffffe;

		// select chunk reader
		switch ( id )
		{
			case PRCORE_BIGCODE32('B','M','H','D'):
			{
				// read bitmap header
				width = static_cast<int>(ReadBigEndian<uint16>(stream));
				height = static_cast<int>(ReadBigEndian<uint16>(stream));
								ReadBigEndian<int16>(stream);	// x
								ReadBigEndian<int16>(stream);	// y
				nplanes =		ReadBigEndian<uint8>(stream);	// nplanes
								ReadBigEndian<uint8>(stream);	// masking
				compression	=	ReadBigEndian<uint8>(stream);	// compression
								ReadBigEndian<uint8>(stream);	// padding
								ReadBigEndian<uint16>(stream);	// transcol
								ReadBigEndian<uint8>(stream);	// x aspect
								ReadBigEndian<uint8>(stream);	// y aspect
								ReadBigEndian<int16>(stream);	// page width
								ReadBigEndian<int16>(stream);	// page height

				break;
			}

			case PRCORE_BIGCODE32('C','M','A','P'):
			{
				palettesize = size / 3;
				palette = new Color32[palettesize];

				for ( int i=0; i<palettesize; ++i )
				{
					palette[i].r = ReadBigEndian<uint8>(stream);
					palette[i].g = ReadBigEndian<uint8>(stream);
					palette[i].b = ReadBigEndian<uint8>(stream);
					palette[i].a = 0xff;
				}

				break;
			}

			case PRCORE_BIGCODE32('C','A','M','G'):
			{
				uint32 v = ReadBigEndian<uint32>(stream);

				ham = (v & 0x800) != 0;	// holf and mofidy
				ehb = (v & 0x80) != 0;	// extra halfbrite

				break;
			}

			case PRCORE_BIGCODE32('B','O','D','Y'):
			{
				// allocate decode buffer
				imagesize = (((width + 15) >> 4) << 1) * height * nplanes;
				image = new uint8[imagesize];

				if ( compression )
				{
					uint8* buffer = new uint8[size];
					stream.Read(buffer,size);

					rle(image,buffer,imagesize);

					delete[] buffer;
				}
				else
				{
					stream.Read(image,size);
				}

				break;
			}

			default:
			{
				stream.Seek(size,Stream::CURRENT);
				break;
			}
		}
	}

	// choose pixelformat
	PixelFormat pxf;
	if ( ham )
	{
		// always decode Hold And Modify modes into 24bpp
		pxf = PIXELFORMAT_RGB888;
	}
	else
	{
		int bpp = (nplanes + 7) >> 3;
		switch ( bpp )
		{
			// TODO: 
			// to be honest, we should have mechanism to determine
			// which bitplanes are grouped how to form red,green and blue
			// color masks for the pixelformat
			case 1: pxf = PIXELFORMAT_PALETTE8(NULL); break;
			case 2: pxf = PIXELFORMAT_RGB565; break;
			case 3: pxf = PIXELFORMAT_RGB888; break;
			case 4: pxf = PIXELFORMAT_ARGB8888; break;
		}
	}

	// assign palette
	if ( pxf.IsIndexed() && palettesize <= 256 )
	{
		Color32* pal = pxf.GetPalette();
		memcpy(pal,palette,palettesize*sizeof(uint32));

		// "correct" .iff files should have all 64 palette entries
		// written into the file for extra half brite mode,
		// but this takes care of the "broken" files aswell
		if ( ehb && palettesize == 32 )
		{
			for ( int i=0; i<32; ++i )
			{
				Color32& color = pal[i];
				Color32& fixed = pal[i+32];

				fixed.r = color.r >> 1;
				fixed.g = color.g >> 1;
				fixed.b = color.b >> 1;
				fixed.a = 0xff;
			}
		}
	}

	// create target bitmap
	target = Bitmap(width,height,pxf);

	// planar-to-chunky conversion
	if ( ham )
	{
		p2c_ham(target.GetImage(),image,width,height,nplanes,palette);
	}
	else
	{
		// TODO: this is a bit shady.. but it appears that "PBM " forms
		// are chunky, and "ILBM" forms are planar..
		if ( v1 == PRCORE_BIGCODE32('P','B','M',' ') )
		{
			memcpy(target.GetImage(),image,width*height);
		}
		else
		{
			p2c_raw(target.GetImage(),image,width,height,nplanes);
		}
	}

	delete[] palette;
	delete[] image;

	return true;
}


// =================================================
// CreateCodecIFF()
// =================================================

BitmapCodec CreateCodecIFF()
{
	static const String ext[] = { "iff", "lbm", "ham", "ham8", "ilbm", "ehb" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = iff_decode;
	codec.encode    = NULL;

	return codec;
}
