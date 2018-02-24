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
		BitmapCodec "ras" - SUN Raster File

	revision history:
		???/??/1998 - Ilkka Pelkonen - initial revision
		Oct/15/2002 - Jukka Liimatta - rewrite to use current stream/codec system
		
	TODO:
		- find RLE compressed .ras files and actually test the RLE code ;-)
		- when find the test files:
		  - write support for 1-bit RLE files
		  - write support for ARGB,ABGR,RGB,BGR RLE files
*/
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// rasrle()
// =================================================

static void rasrle(uint8* image, uint8* buffer, int numsample)
{
	while ( numsample > 0 )
	{
		uint8 v = *buffer++;
		if ( v == 0x80 )
		{
			uint8 s = *buffer++;
			if ( !s )
			{
				// single value of 0x80
				*image++ = 0x80;
				--numsample;
			}
			else
			{
				// rle run
				v = *buffer++;
				int xcount = static_cast<int>(s) + 1;
				numsample -= xcount;
				while ( xcount-- )
				{
					*image++ = v;
				}
			}
		}
		else
		{
			// single value
			*image++ = v;
			--numsample;
		}
	}
}


// =================================================
// codec
// =================================================

static bool ras_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// streamsize - headersize
	int size = stream.GetSize() - 32;
	if ( size < 1 )
		return false;

	// read header
	uint32 magic		= ReadBigEndian<uint32>(stream);
	uint32 width		= ReadBigEndian<uint32>(stream);
	uint32 height		= ReadBigEndian<uint32>(stream);
	uint32 bits			= ReadBigEndian<uint32>(stream);
/*	uint32 length = */	  ReadBigEndian<uint32>(stream);
	uint32 type			= ReadBigEndian<uint32>(stream);
	uint32 maptype		= ReadBigEndian<uint32>(stream);
	uint32 maplength	= ReadBigEndian<uint32>(stream);

	// verify header
	if ( magic != 0x59a66a95 )
		return false;

	if ( bits != 1 && bits != 8 && bits != 24 && bits != 32 )
		return false;

	if ( type != 0 && type != 1 && type != 2 && type != 3 )
		return false;

	// create bitmap
	switch ( bits )
	{
		case 1:  target = Bitmap(width,height,PIXELFORMAT_INTENSITY8); break;
		case 8:  target = Bitmap(width,height,PIXELFORMAT_PALETTE8(NULL)); break;
		case 24: target = Bitmap(width,height,PIXELFORMAT_RGB888); break;
		case 32: target = Bitmap(width,height,PIXELFORMAT_ARGB8888); break;
		default: return false;
	}

	// read colormap
	switch ( maptype )
	{
		// "none" - no palette
		case 0:
		{
			stream.Seek(maplength,Stream::CURRENT);
			break;
		}

		// "equal rgb" -mode palette
		case 1:
		{
			if ( bits != 8 || maplength > 768 )
				return false;

			const PixelFormat& pxf = target.GetPixelFormat();
			Color32* palette = pxf.GetPalette();
			int count = static_cast<int>(maplength / 3);

			int i;
			for ( i=0; i<count; ++i ) palette[i].r = ReadBigEndian<uint8>(stream);
			for ( i=0; i<count; ++i ) palette[i].g = ReadBigEndian<uint8>(stream);
			for ( i=0; i<count; ++i ) palette[i].b = ReadBigEndian<uint8>(stream);
			for ( i=0; i<count; ++i ) palette[i].a = 0xff;

			break;
		}

		// "raw" -mode palette
		// TODO: this code has not been tested
		//       (couldn't find files to test with or specification)
		//       this is my best guess how the "raw" mode works!
		case 2:
		{
			if ( bits != 8 || maplength > 768 )
				return false;

			const PixelFormat& pxf = target.GetPixelFormat();
			Color32* palette = pxf.GetPalette();
			int count = static_cast<int>(maplength / 3);

			for ( int i=0; i<count; ++i )
			{
				palette[i].r = ReadBigEndian<uint8>(stream);
				palette[i].g = ReadBigEndian<uint8>(stream);
				palette[i].b = ReadBigEndian<uint8>(stream);
				palette[i].a = 0xff;
			}
		}

		// unknown fileformat
		default: return false;
	}

	// decode buffer
	int buffersize = stream.GetSize() - stream.GetOffset();
	uint8* buffer = new uint8[buffersize];
	stream.Read(buffer,buffersize);

	// read image
	uint8* image = target.GetImage();

	int numpixel = width * height;
	if ( numpixel < 1 )
		return false;

	switch ( bits )
	{
		case 1:
		{
			switch ( type )
			{
				case 0:
				case 1:
				case 3:
				{
					int ycount = static_cast<int>(height);
					while ( ycount-- )
					{
						int bitcount = -1;
						uint8 v = 0;
						int xcount = static_cast<int>(width);
						while ( xcount-- )
						{
							if ( bitcount < 0 )
							{
								bitcount = 7;
								v = *buffer++;
							}
							*image++ = (v >> (bitcount--)) & 1 ? 0 : 255;
						}
					}
					break;
				}
				case 2:
				{
					// TODO: RLE decoder
					// TODO: special handling of 1 bit stream
					break;
				}
			}
			break;
		}

		case 8:
		{
			switch ( type )
			{
				case 0:
				case 1:
				case 3:
				{
					memcpy(image,buffer,numpixel);
					break;
				}
				case 2:
				{
					// TODO: RLE decoder
					// TODO: RGB/BGR fix
					// TODO: test rasrle()
					rasrle(image,buffer,numpixel);
					break;
				}
			}
			break;
		}

		case 24:
		{
			switch ( type )
			{
				case 0:
				case 1:
				{
					memcpy(image,buffer,numpixel*3);
					break;
				}
				case 2:
				{
					// TODO: RLE decoder
					// TODO: RGB/BGR fix
					// TODO: test rasrle()
					rasrle(image,buffer,numpixel*3);
					break;
				}
				case 3:
				{
					while ( numpixel-- )
					{
						image[0] = buffer[2];
						image[1] = buffer[1];
						image[2] = buffer[0];
						buffer += 3;
						image += 3;
					}
					break;
				}
			}
			break;
		}

		case 32:
		{
			switch ( type )
			{
				case 0:
				case 1:
				{
					while ( numpixel-- )
					{
						image[0] = buffer[1];
						image[1] = buffer[2]; 
						image[2] = buffer[3];
						image[3] = buffer[0];
						buffer += 4;
						image += 4;
					}
					break;
				}
				case 2:
				{
					// TODO: RLE decoder
					// TODO: RGB/BGR fix
					// TODO: test rasrle()
					rasrle(image,buffer,numpixel*4);
					break;
				}
				case 3:
				{
					while ( numpixel-- )
					{
						image[0] = buffer[3];
						image[1] = buffer[2];
						image[2] = buffer[1];
						image[3] = buffer[0];
						buffer += 4;
						image += 4;
					}
					break;
				}
			}
			break;
		}
	}

	// free decode buffer
	delete[] buffer;

	return true;
}


// =================================================
// CreateCodecRAS()
// =================================================

BitmapCodec CreateCodecRAS()
{
	static const String ext[] = { "ras", "sun" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = ras_decode;
	codec.encode    = NULL;

	return codec;
}
