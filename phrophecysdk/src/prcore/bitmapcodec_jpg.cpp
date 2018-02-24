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
		BitmapCodec "jpg"

	revision history:
		Jul/01/1999 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
		Jun/27/2002 - Jukka Liimatta - added support for 8-bit grayscale, cleared up code
		Nov/22/2002 - Thomas M. Carlsson - fixed encoding buffer size
*/
#include "../../src/extlib/jpglib/cdjpeg.h"
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// libjpg
// =================================================

enum OptimizeMode
{
	JPG_NONE		= 0,
	JPG_DOWNSCALE	= 1,
	JPG_PREVIEW		= 2
};

struct AJPG_DECODE
{
	int size;		// input: sizeof(AJPG_DECODE)
	int optimize;	// input: optimize mode
	int maxkb;		// input: maximum outbuf size
	int scale;		// output: scale
	int warnings;	// output: warnings
};

struct AJPG_ENCODE
{
	int size;		// input: sizeof(AJPG_DECODE)
	int quality;	// quality, 0 - 100 (0 = lowest, 100 = highest)
};


extern "C" int outbytes;


static int jpg_decode2(AJPG_DECODE* jpg, char* mfbuf, int fsize, int& jwidth, int& jheight, char*& jbuffer, PixelFormat& jformat)
{
	if ( !mfbuf || !fsize ) 
		return 0;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int pitch;
	int i;
	int maxkb;

	int jmpret = setjmp(*jerror_env());

	if ( !jmpret )
	{
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);

		jpeg_stdio_src( &cinfo, mfbuf, fsize );
		(void) jpeg_read_header(&cinfo, TRUE);

		jpeg_calc_output_dimensions(&cinfo);

		if ( ( cinfo.output_components != 3 ) && ( cinfo.out_color_space != JCS_GRAYSCALE ) )
		{
			jpeg_destroy_decompress(&cinfo);
			return 0;
		}

		if ( jpg->optimize == JPG_DOWNSCALE )
		{
			maxkb = jpg->maxkb;

			if ( maxkb < 24 ) maxkb = 24;

			int jscale = 1 << ((cinfo.output_width * cinfo.output_height * 3) / (1024 * maxkb));
			if ( jscale > 8 ) jscale = 8;

			cinfo.scale_num = 1;
			cinfo.scale_denom = jscale;

			jpeg_calc_output_dimensions(&cinfo);
		}
		else if ( jpg->optimize == JPG_PREVIEW )
		{
			cinfo.scale_num = 1;
			cinfo.scale_denom = 8;

			cinfo.dct_method = JDCT_IFAST;
			cinfo.do_fancy_upsampling = FALSE;

			jpeg_calc_output_dimensions(&cinfo);
		}

		pitch = cinfo.output_width * cinfo.output_components;
		jwidth = cinfo.output_width;
		jheight = cinfo.output_height;

		if ( cinfo.out_color_space == JCS_GRAYSCALE )
		{
			jformat = PixelFormat(8,0xff,0);
		}
		else
		{
			jformat = PixelFormat(24,0xff0000,0x00ff00,0x0000ff,0x000000);
		}

		int jdepth = cinfo.output_components;
		char* tbuffer = new char[jwidth * jheight * jdepth];
		jbuffer = tbuffer;

		if ( !tbuffer )
		{
			jpeg_destroy_decompress(&cinfo);
			return 0;
		}

		buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo,JPOOL_IMAGE,pitch,1);
		jpeg_start_decompress(&cinfo);

		while ( cinfo.output_scanline < cinfo.output_height )
		{
			jpeg_read_scanlines(&cinfo,buffer,1);

			if ( cinfo.out_color_space == JCS_GRAYSCALE )
			{
				for ( i=0; i<pitch; ++i )
				{
	 				*tbuffer++ = *(buffer[0] + i);
				}
			}
			else
			{
				memcpy(tbuffer,buffer[0],pitch);
				tbuffer += pitch;
			}
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		return 1;
	}

	return 0;
}


static int jpg_encode(char* outbuf, int bufsize, int quality, char* image, int width, int height)
{
	int jmpret = setjmp( *jerror_env() );

	if ( !jmpret )
	{
		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		JSAMPROW row_pointer[1];
		JSAMPLE* image_buffer = (JSAMPLE*)image;
		
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);

		if ( !outbuf || !bufsize ) 
			return 0;

		jpeg_stdio_dest(&cinfo,outbuf,bufsize);

		cinfo.image_width = width;
		cinfo.image_height = height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);

		jpeg_set_quality(&cinfo,quality,TRUE);
		jpeg_start_compress(&cinfo,TRUE);

		while ( cinfo.next_scanline < cinfo.image_height )
		{
			row_pointer[0] = &image_buffer[cinfo.next_scanline * width * 3];
			jpeg_write_scanlines(&cinfo,row_pointer,1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		return outbytes;
	}
	else
	{
		return 0;
	}
}


// =================================================
// codec
// =================================================

static bool jpg_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read stream
	int size = stream.GetSize();
	char* data = new char[size];
	stream.Read(data,size);

	// setup decode struct
	AJPG_DECODE ujpg;
	ujpg.size = sizeof(ujpg);
	ujpg.optimize = JPG_NONE;
	ujpg.scale = 1;
	ujpg.warnings = 0;

	// decode
	PixelFormat jformat;
	char* jbuffer = NULL;
	int jwidth = 0;
	int jheight = 0;

	if ( !jpg_decode2(&ujpg,data,size,jwidth,jheight,jbuffer,jformat) )
	{
		delete[] jbuffer;
		delete[] data;

		return false;
	}

	// setup image
	target.SetImage(jwidth,jheight,jformat,jbuffer);

	// release
	delete[] data;

	return true;
}


static bool jpg_encode(Stream& target, Surface& surface, float quality)
{
	// assert
	int width = surface.GetWidth();
	int height = surface.GetHeight();
	if ( width < 1 || height < 1 || !surface.GetImage() )
		return false;

	// temp
	Bitmap temp(width,height,PixelFormat(24,0xff0000,0x00ff00,0x0000ff,0x000000));
	temp.BlitImage(surface,Surface::BLIT_COPY);

	// allocate workbuffer
	int maxsize = width * height * 4 + 4096;
	char* buffer = new char[maxsize];

	// encode image to jpeg stream
	int jq = static_cast<int>(100 * quality);
	char* image = reinterpret_cast<char*>(temp.GetImage());
	int bytes = jpg_encode(buffer,maxsize,jq,image,width,height);

	// write jpeg stream
	if ( bytes > 0 )
	{
		target.Write(buffer,bytes);
	}

	// release workbuffer
	delete[] buffer;

	return bytes != 0;
}


// =================================================
// CreateCodecJPG()
// =================================================

BitmapCodec CreateCodecJPG()
{
	static const String ext[] = { "jpg", "jpeg", "jfif" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = jpg_decode;
	codec.encode    = jpg_encode;

	return codec;
}
