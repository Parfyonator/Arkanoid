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
		BitmapCodec "png"

	revision history:
		May/10/1997 - Thomas Carlsson - initian revision
		Dec/25/1999 - Jukka Liimatta - upgraded libpng to version 1.0.5
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
		May/23/2002 - Jukka Liimatta - fixed grayscale PNGs with libpng 1.2.2
		May/23/2002 - Jukka Liimatta - better pixelformat chosen for decoded PNG streams
*/
#include "../../src/extlib/pnglib/png.h"
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// libpng
// =================================================

namespace
{

	struct APNG_DECODE
	{
		int		size;
		float	gamma;
		float	r;
		float	g;
		float	b;
		int		warnings;
	};

	PixelFormat	pformat;
	int			pwidth;
	int			pheight;
	char*		pimage;
	char*		cbuf;

	void user_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		// need overflow check?
		memcpy(data,cbuf,length); 
		cbuf += length;
	}

	void user_warning_fn(png_struct* png_ptr, png_const_charp warning_msg)
	{
	}

	bool png_decodebuf(APNG_DECODE* png, char* mfbuf, int fsize)
	{
		if ( !png || !mfbuf || !fsize )
			return false;

		if ( png->size != sizeof(APNG_DECODE) )
			return false;

		cbuf = mfbuf;

		// sanity check
		if ( !png_check_sig((unsigned char*)mfbuf,8) )
			return false;

		png_structp		png_ptr;
		png_infop		info_ptr;
		png_uint_32		width, height;
		int				bit_depth, color_type, interlace_type;
		png_color_16	my_background;

		double			screen_gamma  = 0;
		double			image_gamma   = 0.45;
		int				number_passes = 0;

		if ( ( png->r != 0.0 ) || ( png->g != 0.0 ) || ( png->b != 0.0 ) )
		{
			if ( png->r > 1.0f ) png->r = 1.0f;
			if ( png->g > 1.0f ) png->g = 1.0f;
			if ( png->b > 1.0f ) png->b = 1.0f;
			if ( png->r < 0.0f ) png->r = 0.0f;
			if ( png->g < 0.0f ) png->g = 0.0f;
			if ( png->b < 0.0f ) png->b = 0.0f;

			my_background.red   = (png_uint_16)(png->r * 65535.0);
			my_background.green = (png_uint_16)(png->g * 65535.0);
			my_background.blue  = (png_uint_16)(png->b * 65535.0);
		}
		else
		{
			my_background.red = my_background.green = my_background.blue = 0;
		}

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,&user_warning_fn);
		if ( png_ptr == NULL )
			return false;

		info_ptr = png_create_info_struct(png_ptr);
		if ( info_ptr == NULL )
		{
			png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
			return false;
		}

		if ( setjmp(png_ptr->jmpbuf) )
		{
			png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
			return false;
		}

		png_set_read_fn(png_ptr,(void*)NULL,&user_read_fn);
		png_read_info(png_ptr,info_ptr);

		png_get_IHDR(png_ptr,info_ptr,&width,&height,&bit_depth,&color_type,&interlace_type,NULL,NULL);

		png_set_strip_16(png_ptr); // strip 16 bit channels to 8 bit
		png_set_packing(png_ptr);  // separate palettized channels

		// setup surface info

		int pdepth;
		pwidth  = width;
		pheight = height;

		if ( color_type & PNG_COLOR_MASK_COLOR )
		{
			if ( color_type & PNG_COLOR_MASK_ALPHA )
			{
				pformat = PixelFormat(32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
				pdepth = 4;
			}
			else
			{
				pformat = PixelFormat(24,0xff0000,0x00ff00,0x0000ff,0);
				pdepth = 3;
			}
		}
		else
		{
			if ( color_type & PNG_COLOR_MASK_ALPHA )
			{
				pformat = PixelFormat(16,0x00ff,0xff00);
				pdepth = 2;
			}
			else
			{
				pformat = PixelFormat(8,0xff,0);
				pdepth = 1;
			}
		}

		// palette -> rgb
		if ( color_type == PNG_COLOR_TYPE_PALETTE )
			png_set_expand(png_ptr);

		// grayscale -> 8 bits
		if ( !(color_type & PNG_COLOR_MASK_COLOR) && bit_depth < 8 )
			png_set_expand(png_ptr);

		// if exists, expand tRNS to alpha channel
		if ( png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS) )
			png_set_expand(png_ptr);

		if ( png->r || png->g || png->b )
			png_set_background(png_ptr,&my_background,PNG_BACKGROUND_GAMMA_SCREEN,0,1.0);

		screen_gamma = png->gamma ? png->gamma : 1.7;

		image_gamma = 0.0;
		if ( png_get_gAMA(png_ptr,info_ptr,&image_gamma) )
		{
			png_set_gamma(png_ptr,screen_gamma,image_gamma);
		}
		else
		{
			png_set_gamma(png_ptr,screen_gamma,0.45);
		}

		png_set_bgr(png_ptr);         // flip RGBA to BGRA
		//png_set_swap_alpha(png_ptr);  // swap BGRA to ABGR
		//png_set_swap(png_ptr);        // swap 16 bit order to lsb first

		// apply when necessary..
		//png_set_filler(png_ptr,0xff,PNG_FILLER_AFTER); // force alpha byte

		number_passes = png_set_interlace_handling(png_ptr);
		png_read_update_info(png_ptr,info_ptr); // update gamma, etc.

		png_bytep* row_pointers = new png_bytep[height];

		// allocate image

		int numbyte = pwidth * pheight * pdepth;
		char* tbuffer = new char[numbyte];
		if ( !tbuffer )
		{
			png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
			delete[] row_pointers;
			row_pointers = NULL;
			return false;
		}

		memset(tbuffer,0,numbyte);

		int stride = pwidth * pdepth;
		for ( png_uint_32 row=0; row<height; ++row )
			row_pointers[row] = (unsigned char*)(tbuffer + row * stride);

		int number_of_rows = height;
		for ( int pass=0; pass<number_passes; pass++ )
		{
			for ( png_uint_32 y=0; y<height; y += number_of_rows )
				png_read_rows(png_ptr,row_pointers,NULL,number_of_rows);
		}

		// cleanup

		png_read_end(png_ptr,info_ptr);
		png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
		delete[] row_pointers;

		pimage = tbuffer;

		return true;
	}

} // namespace


// =================================================
// codec
// =================================================

static bool png_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read stream, allocate buffer
	int size = stream.GetSize();
	char* data = new char[size];
	stream.Read(data,size);

	// parameters for decoder
	APNG_DECODE upng;
	upng.size     = sizeof(upng);
	upng.gamma    = 2.0f;
	upng.r        = 0;
	upng.g        = 0;
	upng.b        = 0;
	upng.warnings = 0;

	// decode png
	pimage = NULL;
	if ( !png_decodebuf(&upng,data,size) )
	{
		delete[] pimage;
		delete[] data;
		return false;
	}

	// setup image
	target.SetImage(pwidth,pheight,pformat,pimage);

	// release buffer
	delete[] data;

	return true;
}


// =================================================
// CreateCodecPNG()
// =================================================

BitmapCodec CreateCodecPNG()
{
	static const String ext[] = { "png" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = png_decode;
	codec.encode    = NULL;

	return codec;
}
