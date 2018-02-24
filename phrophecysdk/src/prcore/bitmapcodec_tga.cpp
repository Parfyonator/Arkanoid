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
		BitmapCodec "tga" (TrueVision Targa)

	revision history:
		Nov/14/2000 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Oct/27/2001 - Jukka Liimatta - fixed the decoder
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
		Dec/28/2002 - Jukka Liimatta - improved grayscale encoding
*/
#include <algorithm>
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// tga header
// =================================================

struct HeaderTGA
{
	uint8		idfield_length;
	uint8		colormap_type;
	uint8		data_type;
	uint16		colormap_origin;
	uint16		colormap_length;
	uint8		colormap_bitsize;
	uint16		image_origin_x;
	uint16		image_origin_y;
	uint16		image_width;
	uint16		image_height;
	uint8		pixelsize;
	uint8		descriptor;

	bool ReadHeader(Stream& stream)
	{
		// read header
		idfield_length   = ReadLittleEndian<uint8>(stream);
		colormap_type    = ReadLittleEndian<uint8>(stream);
		data_type        = ReadLittleEndian<uint8>(stream);
		colormap_origin  = ReadLittleEndian<uint16>(stream);
		colormap_length  = ReadLittleEndian<uint16>(stream);
		colormap_bitsize = ReadLittleEndian<uint8>(stream);
		image_origin_x   = ReadLittleEndian<uint16>(stream);
		image_origin_y   = ReadLittleEndian<uint16>(stream);
		image_width      = ReadLittleEndian<uint16>(stream);
		image_height     = ReadLittleEndian<uint16>(stream);
		pixelsize        = ReadLittleEndian<uint8>(stream);
		descriptor       = ReadLittleEndian<uint8>(stream);

		// validate header
		switch ( data_type )
		{
			case 1:
			case 2:
			case 9:
			case 10: break;
			default: return false;
		}

		switch ( pixelsize )
		{
			case 8:
			case 16:
			case 24:
			case 32: break;
			default: return false;
		}

		if ( colormap_type > 1 )
			return false;

		if ( (data_type == 1 || data_type == 9) && (colormap_bitsize != 24 || colormap_length > 256) )
			return false;

		// everything seems to be in order
		return true;
	}

	void WriteHeader(Stream& stream) const
	{
		WriteLittleEndian<uint8>(stream,idfield_length);
		WriteLittleEndian<uint8>(stream,colormap_type);
		WriteLittleEndian<uint8>(stream,data_type);
		WriteLittleEndian<uint16>(stream,colormap_origin);
		WriteLittleEndian<uint16>(stream,colormap_length);
		WriteLittleEndian<uint8>(stream,colormap_bitsize);
		WriteLittleEndian<uint16>(stream,image_origin_x);
		WriteLittleEndian<uint16>(stream,image_origin_y);
		WriteLittleEndian<uint16>(stream,image_width);
		WriteLittleEndian<uint16>(stream,image_height);
		WriteLittleEndian<uint8>(stream,pixelsize);
		WriteLittleEndian<uint8>(stream,descriptor);
	}
};


// =================================================
// codec
// =================================================

static void UnpackLINEAR(Stream& stream, char* scan, int nextscan, int width, int height, int depth)
{
	int pitch = width * depth;
	for ( int y=0; y<height; ++y, scan+=nextscan )
	{
		// decode scanline
		stream.Read(scan,pitch);
	}
}


static void UnpackRLE(Stream& stream, char* scan, int nextscan, int width, int height, int depth)
{
	int x = 0;
	int y = 0;
	char* buffer = scan;

	for ( ;; )
	{
		uint8 sample = ReadLittleEndian<uint8>(stream);
		int count = (sample & 0x7f) + 1;

		if ( sample & 0x80 )
		{
			uint8 color[4];
			stream.Read(color,depth);

			for ( ; count; )
			{
				int left = width - x;
				int size = std::min(count,left);
				
				count -= size;
				x += size;
				
				for ( ; size; --size )
					for ( int j=0; j<depth; ++j )
						*buffer++ = color[j];
					
				if ( x >= width )
				{
					if ( ++y >= height )
						return;

					x = 0;
					scan += nextscan;
					buffer = scan;
				}
			}
		}
		else
		{
			for ( ; count; )
			{
				int left = width - x;
				int size = std::min(count,left);
				
				stream.Read(buffer,size*depth);
				buffer += size*depth;
				count -= size;

				x += size;
				if ( x >= width )
				{
					if ( ++y >= height )
						return;

					x = 0;
					scan += nextscan;
					buffer = scan;
				}
			}
		}
	}
}


static bool tga_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read header
	HeaderTGA header;
	if ( !header.ReadHeader(stream) )
		return false;

	// skip idfield
	stream.Seek(header.idfield_length,Stream::CURRENT);

	// information
	PixelFormat pxf;
	switch ( header.pixelsize )
	{
		case 8:		pxf = PIXELFORMAT_PALETTE8(NULL); break;
		case 16:	pxf = PIXELFORMAT_ARGB1555; break;
		case 24:	pxf = PIXELFORMAT_RGB888; break;
		case 32:	pxf = PIXELFORMAT_ARGB8888; break;
	}

	// read palette
	switch ( header.data_type )
	{
		case 2:
		case 10:
		{
			int delta = header.colormap_length * ((header.colormap_bitsize + 1) >> 3);
			stream.Seek(delta,Stream::CURRENT);
			break;
		}

		case 1:
		case 9:
		{
			Color32* palette = pxf.GetPalette();
			for ( int i=0; i<static_cast<int>(header.colormap_length); ++i )
			{
				palette[i].b = ReadLittleEndian<uint8>(stream);
				palette[i].g = ReadLittleEndian<uint8>(stream);
				palette[i].r = ReadLittleEndian<uint8>(stream);
				palette[i].a = 0xff;
			}
			break;
		}
	}

	// setup info
	int width  = header.image_width;
	int height = header.image_height;
	int depth  = pxf.GetBytes();
	int pitch  = width * depth;

	// set image
	char* image = new char[height * pitch];
	target.SetImage(width,height,pxf,image);

	// image orientation
	char* scan   = (header.descriptor & 32) ? image : image + (height - 1) * pitch;
	int nextscan = (header.descriptor & 32) ? pitch : -pitch;

	// decode image
	switch ( header.data_type )
	{
		case 1: UnpackLINEAR(stream,scan,nextscan,width,height,depth); break;	// indexed 8bit (linear)
		case 2:	UnpackLINEAR(stream,scan,nextscan,width,height,depth); break;	// rgb 16,24,32bit (linear)
		case 9: UnpackRLE(stream,scan,nextscan,width,height,depth); break;		// indexed 8bit (rle)
		case 10: UnpackRLE(stream,scan,nextscan,width,height,depth); break;		// rgb 16,24,32bit (rle)
	}

	return true;
}


static bool tga_encode(Stream& target, Surface& surface, float)
{
	// choose pixelformat
	PixelFormat pxf = surface.GetPixelFormat();
	if ( !pxf.IsIndexed() )
	{
		if ( pxf.IsIntensity() )
		{
			// waste, but preserves accuracy
			// TODO: encode into indexed with grayscale palette
			pxf = PIXELFORMAT_RGB888;
		}
		else
		{
			// supported DirectColor formats
			switch ( pxf.GetBits() )
			{
				case 8: // flow to 16bit format
				case 16: pxf = PIXELFORMAT_ARGB1555; break;
				case 24: pxf = PIXELFORMAT_RGB888; break;
				case 32: pxf = PIXELFORMAT_ARGB8888; break;
			}
		}
	}

	// information
	uint16 width  = surface.GetWidth();
	uint16 height = surface.GetHeight();
	uint8 bits    = pxf.GetBits();
	uint8 alpha   = (bits == 32) ? 8 : 0;
	Color32* palette = pxf.IsIndexed() ? pxf.GetPalette() : NULL;

	// create encode bitmap
	Bitmap tga(width,height,pxf);
	tga.BlitImage(surface,Surface::BLIT_COPY);

	// setup header
	HeaderTGA header;
	if ( palette )
	{
		header.colormap_type    = 1;
		header.data_type        = 1;
		header.colormap_origin  = 0;
		header.colormap_length  = 256;
		header.colormap_bitsize = 24;
	}
	else
	{
		header.colormap_type    = 0;
		header.data_type        = 2;
		header.colormap_origin  = 0;
		header.colormap_length  = 0;
		header.colormap_bitsize = 0;
	}
	header.idfield_length = 0;
	header.image_origin_x = 0;
	header.image_origin_y = 0;
	header.image_width    = width;
	header.image_height   = height;
	header.pixelsize      = bits;
	header.descriptor     = 0x20 | alpha;

	// write header
	header.WriteHeader(target);

	// write palette
	if ( palette )
	{
		for ( int i=0; i<256; ++i )
		{
			WriteLittleEndian<uint8>(target,palette->b);
			WriteLittleEndian<uint8>(target,palette->g);
			WriteLittleEndian<uint8>(target,palette->r);
			++palette;
		}
	}

	// write image
	target.Write(tga.GetImage(),width*height*pxf.GetBytes());

	return true;
}


// =================================================
// CreateCodecTGA()
// =================================================

BitmapCodec CreateCodecTGA()
{
	static const String ext[] = { "tga" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = tga_decode;
	codec.encode    = tga_encode;

	return codec;
}
