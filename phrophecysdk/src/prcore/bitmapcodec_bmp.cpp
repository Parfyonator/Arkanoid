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
		BitmapCodec "bmp"

	revision history:
		Dec/02/2000 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Feb/13/2002 - Jukka Liimatta - added encoding support
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
		Jul/02/2002 - Jukka Liimatta - added support for extended pixelformats
		Jul/12/2002 - Jukka Liimatta - added BitStream abstraction (incomplete)
		Jul/13/2002 - Jukka Liimatta - added RLE4 support
		Aug/17/2003 - Jukka Liimatta - cleaning up
*/
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// BitStream
// =================================================

struct BitStream
{
	Stream& stream;
	uint32 data;
	int bits;

	BitStream(Stream& s)
	: stream(s),data(0),bits(0)
	{
	}

	uint8 GetSample(int size)
	{
		bits -= size;
		if ( bits <= 0 )
		{
			// TODO: concenate with bits still in "data" register
			// - need two 32bit registers for this
			// - since extract is always 4 or 1 bit for whole stream, this is not a problem
			// - want to fix this later on for more generic use & expose in the prcore api
			data = ReadBigEndian<uint32>(stream);
			bits = 32;
		}

		uint32 mask = (1 << size) - 1;
		return (data >> (bits - size)) & mask;
	}
};


// =================================================
// codec
// =================================================

static bool bmp_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read header
	uint16 signature = ReadLittleEndian<uint16>(stream);
	if ( signature != PRCORE_CODE16('B','M') )
		return false;

	ReadLittleEndian<uint32>(stream); // filesize
	ReadLittleEndian<uint32>(stream); // reserved
	uint32 dataoffset = ReadLittleEndian<uint32>(stream);

	uint32 size = ReadLittleEndian<uint32>(stream);
	int32 width = 0;
	int32 height = 0;
	//uint16 planes = 0;
	uint16 bitcount = 0;

	// compression
	enum bi_type
	{
		bi_rgb = 0,
		bi_rle4 = 1,
		bi_rle8 = 2,
		bi_bitfields = 3
	};
	uint32 compression = bi_rgb;

	bool OS2_BMP = false;
	bool EXTENDED_BMP = false;

	if ( (size == 40) || (size == 56) )
	{
		// win32 bmp
		width        = ReadLittleEndian<int32>(stream);
		height       = ReadLittleEndian<int32>(stream);
		/*planes = */  ReadLittleEndian<uint16>(stream);
		bitcount     = ReadLittleEndian<uint16>(stream);
		compression  = ReadLittleEndian<uint32>(stream);

		ReadLittleEndian<uint32>(stream); // imagesize
		ReadLittleEndian<uint32>(stream); // xres
		ReadLittleEndian<uint32>(stream); // yres
		ReadLittleEndian<uint32>(stream); // colorused
		ReadLittleEndian<uint32>(stream); // colorimportant

		if ( size == 56 )
		{
			EXTENDED_BMP = true;
		}
	}
	else if ( (size == 12) || (size == 240) )
	{
		// OS/2 bmp
		OS2_BMP = true;

		width       = ReadLittleEndian<int16>(stream);
		height      = ReadLittleEndian<int16>(stream);
		/*planes = */ ReadLittleEndian<uint16>(stream);
		bitcount    = ReadLittleEndian<uint16>(stream);
	}
	else
	{
		return false;
	}

	// image dimensions
	if ( width < 0 || height == 0 )
		return false;

	// pixelformat

	int palsize = 0;
	int pitch = 0;

	PixelFormat pxf;

	switch ( bitcount )
	{
		case 1:
			pxf = PIXELFORMAT_PALETTE8(NULL);
			palsize = 2;
			pitch = (width+7) >> 3;
			break;

		case 4:
			pxf = PIXELFORMAT_PALETTE8(NULL);
			palsize = 16;
			pitch = (width+3) >> 2;
			break;

		case 8:
			pxf = PIXELFORMAT_PALETTE8(NULL);
			palsize = 256;
			pitch = width;
			break;

		case 16:
			pxf = PixelFormat(16,0x7c00,0x03e0,0x001f,0);
			palsize = 0;
			pitch = width * 2;
			break;

		case 24:
			pxf = PIXELFORMAT_RGB888;
			palsize = 0;
			pitch = width * 3;
			break;

		case 32:
			pxf = PIXELFORMAT_ARGB8888;
			palsize = 0;
			pitch = width * 4;
			break;

		default:
			return false;
	}

	// read colormap

	Color32* palette = pxf.GetPalette();
	for ( int i=0; i<palsize; ++i )
	{
		Color32 color;
		color.b = ReadLittleEndian<uint8>(stream);
		color.g = ReadLittleEndian<uint8>(stream);
		color.r = ReadLittleEndian<uint8>(stream);
		color.a = OS2_BMP ? 0xff : ReadLittleEndian<uint8>(stream);

		if ( palette )
			*palette++ = color;
	}

	// extended pixelformat colormasks

	if ( EXTENDED_BMP )
	{
		uint32 rmask = ReadLittleEndian<uint32>(stream);
		uint32 gmask = ReadLittleEndian<uint32>(stream);
		uint32 bmask = ReadLittleEndian<uint32>(stream);
		uint32 amask = ReadLittleEndian<uint32>(stream);

		pxf = PixelFormat(bitcount,rmask,gmask,bmask,amask);
	}
	else if ( compression == bi_bitfields )
	{
		uint32 rmask = ReadLittleEndian<uint32>(stream);
		uint32 gmask = ReadLittleEndian<uint32>(stream);
		uint32 bmask = ReadLittleEndian<uint32>(stream);
		uint32 amask = ~(rmask|gmask|bmask);

		pxf = PixelFormat(bitcount,rmask,gmask,bmask,amask);
	}
	
	// handle negative height

	bool yflip = true;
	if ( height < 0 )
	{
		height = -height;
		yflip = false;
	}

	// initialize decode target
	target = Bitmap(width,height,pxf);

	// 32bit alignment adjust
	int align = ((pitch + 3) & 0xfffffffc) - pitch;

	// seek to image
	stream.Seek(dataoffset,Stream::START);

	// read image
	uint8* image = target.GetImage();

	if ( compression == bi_rgb || compression == bi_bitfields )
	{
		// ================================
		// compression: LINEAR
		// ================================
		switch ( bitcount )
		{
			case 1:
			case 4:
			{
				if ( yflip )
				{
					image += (height - 1) * width;
					pitch = -width;
				}
				else
				{
					pitch = width;
				}

				int ycount = static_cast<int>(height);
				while ( ycount-- )
				{
					uint8* buffer = image;

					BitStream bitstream(stream);

					int xcount = static_cast<int>(width);
					while ( xcount-- )
					{
						*buffer++ = bitstream.GetSample(bitcount);
					}

					// scanline alignment to 32-bit boundary
					// - not required, GetSample() does alignment automatically
					// - critical that BitStream is not moved from above position

					image += pitch;
				}

				break;
			}

			case 8:
			case 16:
			case 24:
			case 32:
			{
				int xcount = pitch;
				if ( yflip )
				{
					image += (height - 1) * pitch;
					pitch = -pitch;
				}

				int ycount = static_cast<int>(height);
				while ( ycount-- )
				{
					stream.Read(image,xcount);

					// scanline alignment to 32-bit boundary
					if ( align )
						stream.Seek(align,Stream::CURRENT);

					image += pitch;
				}

				break;
			}

			default:
				return false;
		}
	}
	else
	{
		// ================================
		// compression: RLE
		// ================================
		switch ( bitcount )
		{
			case 4:
			{
				int y = 0;
				int ymax = static_cast<int>(height);
				while ( y < ymax )
				{
					int offset = stream.GetOffset();

					int x = 0;
					int xmax = static_cast<int>(width);
					while ( x < xmax )
					{
						// read RLE sample
						uint8 n = ReadLittleEndian<uint8>(stream);
						uint8 c = ReadLittleEndian<uint8>(stream);

						// buffer address
						int scan = yflip ? height - 1 - y : y;
						uint8* buffer = image + scan * width + x;

						if ( n )
						{
							x += n;

							int ad = 4;
							while ( n-- )
							{
								*buffer++ = (c >> ad) & 0xf;
								ad = 4 - ad;
							}
						}
						else
						{
							switch ( c )
							{
								case 0: // end of scanline
									x = 0;
									break;
		
								case 1: // end of image
									return true;

								case 2: // position delta
								{
									x += ReadLittleEndian<uint8>(stream);
									y += ReadLittleEndian<uint8>(stream);
									break;
								}

								default: // linear imagedata
								{
									x += c;

									int sum = 0;
									int count = c >> 1;
									if ( count )
									{
										while ( count-- )
										{
											uint8 s = ReadLittleEndian<uint8>(stream);
											++sum;
											*buffer++ = s >> 4;
											*buffer++ = s & 0xf;
										}
									}

									if ( c & 1 )
									{
										uint8 s = ReadLittleEndian<uint8>(stream);
										++sum;
										*buffer = s >> 4;
									}

									if ( sum & 1 )
										ReadLittleEndian<uint8>(stream);

									break;
								}
							}
						}
					}
					
					// scanline alignment to 32-bit boundary
					int count = (stream.GetOffset() - offset) & 3;
					if ( count )
					{
						stream.Seek(4 - count,Stream::CURRENT);
					}

					// next scanline
					++y;
				}

				// ------------------------------------------------

				break;
			}

			case 8:
			{
				int y = 0;
				int ymax = static_cast<int>(height);
				while ( y < ymax )
				{
					int offset = stream.GetOffset();

					int x = 0;
					int xmax = static_cast<int>(width);
					while ( x < xmax )
					{
						// read RLE sample
						uint8 n = ReadLittleEndian<uint8>(stream);
						uint8 c = ReadLittleEndian<uint8>(stream);

						// buffer address
						int scan = yflip ? height - 1 - y : y;
						uint8* buffer = image + scan * pitch + x;

						if ( n )
						{
							x += n;
							while ( n-- )
								*buffer++ = c;
						}
						else
						{
							switch ( c )
							{
								case 0: // end of scanline
									x = 0;
									break;
		
								case 1: // end of image
									return true;

								case 2: // position delta
								{
									x += ReadLittleEndian<uint8>(stream);
									y += ReadLittleEndian<uint8>(stream);
									break;
								}

								default: // linear imagedata
								{
									x += c;
									stream.Read(buffer,c);

									if ( c & 1 )
										stream.Seek(1,Stream::CURRENT);

									break;
								}
							}
						}
					}
					
					// scanline alignment to 32-bit boundary
					int count = (stream.GetOffset() - offset) & 3;
					if ( count )
					{
						stream.Seek(4 - count,Stream::CURRENT);
					}

					// next scanline
					++y;
				}

				break;
			}

			default:
				return false;
		}
	}

	return true;
}


static bool bmp_encode(Stream& stream, Surface& surface, float)
{

	// info
	uint32 width       = surface.GetWidth();
	uint32 height      = surface.GetHeight();
	uint16 planes      = 1;
	uint16 bitcount    = 8;
	uint32 compression = 0;
	uint32 dataoffset  = 18 + 16 + 20;

	// choose pixelformat
	PixelFormat pxf = surface.GetPixelFormat();
	if ( pxf.IsIndexed() )
	{
		dataoffset += 1024;
	}
	else
	{
		switch ( pxf.GetBits() )
		{
			case 8: // flow to 16bit format
			case 16: pxf = PIXELFORMAT_RGB565;   planes = 3; bitcount = 16; break;
			case 24: pxf = PIXELFORMAT_RGB888;   planes = 3; bitcount = 24; break;
			case 32: pxf = PIXELFORMAT_ARGB8888; planes = 4; bitcount = 32; break;
		}
	}

	int pitch = width * pxf.GetBytes();
	int align = ((pitch + 3) & 0xfffffffc) - pitch;
	uint32 filesize = dataoffset + (height * (pitch+align));

	// write header
	// 18 bytes
	WriteLittleEndian<uint16>(stream,PRCORE_CODE16('B','M')); // id
	WriteLittleEndian<uint32>(stream,filesize);               // filesize
	WriteLittleEndian<uint32>(stream,0);                      // reserved
	WriteLittleEndian<uint32>(stream,dataoffset);             // data offset
	WriteLittleEndian<uint32>(stream,40);                     // header size

	// 16 bytes
	WriteLittleEndian<uint32>(stream,width);
	WriteLittleEndian<uint32>(stream,height);
	WriteLittleEndian<uint16>(stream,planes);
	WriteLittleEndian<uint16>(stream,bitcount);
	WriteLittleEndian<uint32>(stream,compression);

	// 20 bytes
	WriteLittleEndian<uint32>(stream,0); // imagesize
	WriteLittleEndian<uint32>(stream,0); // xres
	WriteLittleEndian<uint32>(stream,0); // yres
	WriteLittleEndian<uint32>(stream,0); // colorused
	WriteLittleEndian<uint32>(stream,0); // colorimportant
	
	// write colormap
	if ( pxf.IsIndexed() )
	{
		Color32* palette = pxf.GetPalette();
		for ( int i=0; i<256; ++i )
		{
			WriteLittleEndian<uint8>(stream,palette[i].b);
			WriteLittleEndian<uint8>(stream,palette[i].g);
			WriteLittleEndian<uint8>(stream,palette[i].r);
			WriteLittleEndian<uint8>(stream,palette[i].a);
		}
	}

	// write image
	Bitmap encobj(width,height,pxf);
	encobj.BlitImage(surface);

	uint8* image = encobj.GetImage();

	int ymax = static_cast<int>(height);
	for ( int y=0; y<ymax; ++y )
	{
		uint8* buffer = image + (height - y - 1) * pitch;
		stream.Write(buffer,pitch);

		for ( int j=0; j<align; ++j )
			WriteLittleEndian<uint8>(stream,0);
	}

	return true;
}


// =================================================
// CreateCodecBMP()
// =================================================

BitmapCodec CreateCodecBMP()
{
	static const String ext[] = { "bmp" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = bmp_decode;
	codec.encode    = bmp_encode;

	return codec;
}
