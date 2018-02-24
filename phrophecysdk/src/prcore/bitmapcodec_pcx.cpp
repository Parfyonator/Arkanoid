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
		BitmapCodec "pcx"

	revision history:
		Jan/24/2001 - Jukka Liimatta - initial revision/renaissance build
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
		Apr/09/2002 - Jukka Liimatta - fixed decoder (correct handling of odd image dimensions)
*/
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// codec
// =================================================

static void decode_rlescan(uint8* scan, uint8* end, Stream& stream)
{
	while ( scan < end )
	{
		uint8 sample = ReadLittleEndian<uint8>(stream);
		if ( sample < 0xc0 )
		{
			*scan++ = sample;
		}
		else
		{
			int count = sample & 0x3f;
			sample = ReadLittleEndian<uint8>(stream);
			memset(scan,sample,count);
			scan += count;
		}
	}
}


static bool pcx_decode(Bitmap& target, Stream& stream)
{
	
	struct HeaderPCX
	{
		uint8		Manufacturer;
		uint8		Version;
		uint8		Encoding;
		uint8		BitsPerPixel;
		uint16		Xmin,Ymin,Xmax,Ymax;
		uint16		HDpi,VDpi;
		uint8		Colormap[48];
		uint8		Reserved;
		uint8		NPlanes;
		uint16		BytesPerLine;
		uint16		PaletteInfo;
		uint16		HscreenSize;
		uint16		VscreenSize;
		uint8		Filler[54];
	};

	// reset stream
	stream.Seek(0,Stream::START);

	// read header
	HeaderPCX header;

	header.Manufacturer = ReadLittleEndian<int8>(stream);
	header.Version      = ReadLittleEndian<int8>(stream);
	header.Encoding     = ReadLittleEndian<int8>(stream);
	header.BitsPerPixel = ReadLittleEndian<int8>(stream);
	header.Xmin         = ReadLittleEndian<int16>(stream);
	header.Ymin         = ReadLittleEndian<int16>(stream);
	header.Xmax         = ReadLittleEndian<int16>(stream);
	header.Ymax         = ReadLittleEndian<int16>(stream);
	header.HDpi         = ReadLittleEndian<int16>(stream);
	header.VDpi         = ReadLittleEndian<int16>(stream);

	stream.Read(header.Colormap,48);
	stream.Seek(1,Stream::CURRENT);

	header.NPlanes      = ReadLittleEndian<int8>(stream);
	header.BytesPerLine = ReadLittleEndian<int16>(stream);
	header.PaletteInfo  = ReadLittleEndian<int16>(stream);
	header.HscreenSize  = ReadLittleEndian<int16>(stream);
	header.VscreenSize  = ReadLittleEndian<int16>(stream);

	stream.Seek(54,Stream::CURRENT);

	// verify header
	if ( (header.Manufacturer!=10) || (header.NPlanes!=1 && header.NPlanes!=3) )
		return false;

	// dimensions
	int width  = header.Xmax - header.Xmin + 1;
	int height = header.Ymax - header.Ymin + 1;

	// used to patch the last pixel of scanline
	// when image dimensions are not even.
	int readwidth = (width & 1) ? width + 1 : width;


	// decode 8bit

	if ( header.NPlanes == 1 )
	{
		target = Bitmap(width,height,PIXELFORMAT_PALETTE8(NULL));
		uint8* dest = target.GetImage();

		// read image
		for ( int y=0; y<height; ++y )
		{
			// decode rle scanline
			decode_rlescan(dest,dest+readwidth,stream);
			dest += width;
		}

		// read palette
		ReadLittleEndian<uint8>(stream);
		Color32* palette = target.GetPixelFormat().GetPalette();
		for ( int i=0; i<256; ++i )
		{
			palette[i].r = ReadLittleEndian<uint8>(stream);
			palette[i].g = ReadLittleEndian<uint8>(stream);
			palette[i].b = ReadLittleEndian<uint8>(stream);
			palette[i].a = 0xff;
		}
	}


	// decode 24bit

	if ( header.NPlanes == 3 )
	{
		// allocate scanbuffer
		uint8* scanbuffer = new uint8[readwidth * header.NPlanes];

		target = Bitmap(width,height,PIXELFORMAT_RGB888);
		uint8* dest = target.GetImage();

		// read image
		for ( int y=0; y<height; ++y )
		{
			// decode interleaved rle scanline
			decode_rlescan(scanbuffer,scanbuffer + readwidth * 3,stream);

			// interleaved -> packed conversion
			uint8* src = scanbuffer;
			int count = width;
			while ( count-- )
			{
				*dest++ = src[readwidth * 2];
				*dest++ = src[readwidth];
				*dest++ = *src++;
			}
		}

		// free scanbuffer
		delete[] scanbuffer;
	}

	return true;
}


// =================================================
// CreateCodecPCX()
// =================================================

BitmapCodec CreateCodecPCX()
{
	static const String ext[] = { "pcx" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = pcx_decode;
	codec.encode    = NULL;

	return codec;
}
