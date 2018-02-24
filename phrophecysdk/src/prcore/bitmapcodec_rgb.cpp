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
		BitmapCodec "rgb"

	revision history:
		Sep/22/2000 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
*/
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// codec
// =================================================

static bool rgb_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read header
	uint16 magic = ReadBigEndian<uint16>(stream);
	if ( magic != 474 )
		return false;

	uint8 format     = ReadBigEndian<uint8>(stream);		// 0 - raw, 1 - rle
	uint8 bpc        = ReadBigEndian<uint8>(stream);		// bytes per pixel channel ( 1,2 )
	uint16 dimension = ReadBigEndian<uint16>(stream);		// number of dimensions ( 1,2,3 )
	uint16 xsize     = ReadBigEndian<uint16>(stream);		// width
	uint16 ysize     = ReadBigEndian<uint16>(stream);		// height
	uint16 zsize     = ReadBigEndian<uint16>(stream);		// number of channels
	                   ReadBigEndian<uint32>(stream);		// minimum pixel value
	                   ReadBigEndian<uint32>(stream);		// maximum pixel value
	                   stream.Seek(4,Stream::CURRENT);		// ignored
	                   stream.Seek(80,Stream::CURRENT);		// image name
	uint32 colormap  = ReadBigEndian<uint32>(stream);		// colormap, 0 - normal, 1 - dithered, 2 - screen, 3 - colormap
	                   stream.Seek(404,Stream::CURRENT);	// ignored

	if ( bpc != 1 || colormap != 0 )
		return false;

	// choose pixelformat
	PixelFormat pxf;
	switch ( dimension )
	{
		case 1:
			pxf = PixelFormat(8,0xff,0x00);
			ysize = 1;
			zsize = 1;
			break;

		case 2:
			pxf = PixelFormat(8,0xff,0x00);
			zsize = 1;
			break;

		case 3:
		{
			if ( zsize == 3 )
			{
				pxf = PixelFormat(24,0x0000ff,0x00ff00,0xff0000,0x000000);
				break;
			}

			if ( zsize == 4 )
			{
				pxf = PixelFormat(32,0x000000ff,0x0000ff00,0x00ff0000,0xff000000);
				break;
			}

			return false;
		}

		default:
			return false;
	}


	// allocate memory
	int pitch = xsize * pxf.GetBytes();
	char* buffer = new char[ysize * pitch];

	// decode RAW
	if ( format == 0 )
	{
		for ( int plane=0; plane<zsize; ++plane )
		{
			for ( int y=0; y<ysize; ++y )
			{
				int scanline = ysize - y - 1;
				char* ptr = buffer + (pitch * scanline) + plane;

				int offset = (y + plane*ysize) * xsize;
				stream.Seek(offset,Stream::START);

				// raw unpack
				for ( int x=0; x<xsize; ++x )
				{
					*ptr = ReadBigEndian<uint8>(stream);
					ptr += zsize;
				}
			}
		}
	}

	// decore RLE
	if ( format == 1 )
	{
		// rle offset table
		int count = ysize * zsize;
		uint32* scanoffset = new uint32[count];

		for ( int i=0; i<count; ++i )
			scanoffset[i] = ReadBigEndian<uint32>(stream);

		for ( int plane=0; plane<zsize; ++plane )
		{
			for ( int y=0; y<ysize; ++y )
			{
				int scanline = ysize - y - 1;
				char* ptr = buffer + (pitch * scanline) + plane;

				int offset = scanoffset[y + plane*ysize];
				stream.Seek(offset,Stream::START);

				// rle unpack
				for ( ;; )
				{
					uint8 pixel = ReadBigEndian<uint8>(stream);
					uint8 count = pixel & 0x7f; 

					if ( !count )
						break;

					if( pixel & 0x80 ) 
					{
						while( count-- ) 
						{
							*ptr = ReadBigEndian<uint8>(stream);
							ptr += zsize;
 						}
 					} 
					else 
					{
 						pixel = ReadBigEndian<uint8>(stream);
 						while( count-- )
						{
							*ptr = pixel;
							ptr += zsize;
						}
					}
				}
			}
		}

		delete[] scanoffset;
	}

	// copy image
	target.SetImage( xsize, ysize, pxf, buffer );

	return true;
}


// =================================================
// CreateCodecRGB()
// =================================================

BitmapCodec CreateCodecRGB()
{
	static const String ext[] = { "rgb", "rgba", "bw", "sgi" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = rgb_decode;
	codec.encode    = NULL;

	return codec;
}
