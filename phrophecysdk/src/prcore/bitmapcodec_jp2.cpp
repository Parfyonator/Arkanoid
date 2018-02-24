/*
	Twilight Prophecy 3D/Multimedia SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Please read the file LICENSE.TXT for additional details.


	source: 
		BitmapCodec "JPEG2000"
		uses libjasper: http://www.ece.ubc.ca/~mdadams/jasper

	revision history:
		Dec/11/2001 - Daniel Kolakowski - initial revision
		Mar/23/2001 - Daniel Kolakowski - rewrote codec interface
		Oct/27/2002 - Jukka Liimatta - rewrote decoder (bounding box support, scaling,..)

	TODO:
	- need to refine the decoder so that all reference streams types work
	- optimize the YCrBr->RGB conversion
*/
#include <prcore/prcore.hpp>
#include "../extlib/jasper/jasper/jasper.h"

// libjasper defines it's own bool so we have to wipe it out here
#undef bool

using namespace prcore;


static bool JasperImage(Bitmap& bitmap, jas_image_t* jasimage)
{
	int pitch = bitmap.GetPitch();
	int bpp = bitmap.GetPixelFormat().GetBytes();
	uint8* image = bitmap.GetImage();

	int colormodel = jas_image_colorspace(jasimage);

	int numchan = jas_image_numcmpts(jasimage);
	for ( int i=0; i<numchan; ++i )
	{
		int xstep = jas_image_cmpthstep(jasimage,i);
		int ystep = jas_image_cmptvstep(jasimage,i);
		int xsize = jas_image_cmptwidth(jasimage,i);
		int ysize = jas_image_cmptheight(jasimage,i);
		int xorig = jas_image_cmpttlx(jasimage,i);
		int yorig = jas_image_cmpttly(jasimage,i);
		bool sign = jas_image_cmptsgnd(jasimage,i) != 0;

		// allocate decoding matrix
		jas_matrix_t* matrix = jas_matrix_create(1,xsize);
		if ( !matrix )
		{
			return false;
		}

		int shifter = 8 - jas_image_cmptprec(jasimage,i);
		if ( shifter < 0 )
		{
			jas_matrix_destroy(matrix);
			return false;
		}

		uint8* yimage = image + yorig*ystep*pitch + xorig*xstep*bpp;

		// color model
		int rawtype = jas_image_cmpttype(jasimage,i);
		int cltype = JAS_IMAGE_CT_COLOR(rawtype);

		// theoretically this should work.. in practise.. it doesn't ;-)
		// the mask should be 0x80000000, but it's 0x8000 in the header
		// .. I might just be idiot, so I will check into this later..
		// .. temporarily I assume "unknown" channel is alpha (see below)

		//bool isalpha = (rawtype & JAS_IMAGE_CT_OPACITY) != 0;

		switch ( colormodel )
		{
			case JAS_IMAGE_CS_UNKNOWN:
			case JAS_IMAGE_CS_GRAY:
				break;

			case JAS_IMAGE_CS_RGB:
				switch ( cltype )
				{
					case JAS_IMAGE_CT_RGB_R: yimage += 0; break;
					case JAS_IMAGE_CT_RGB_G: yimage += 1; break;
					case JAS_IMAGE_CT_RGB_B: yimage += 2; break;
					default: yimage += 3; break; // assume this is alpha channel ;-)
				}
				break;

			case JAS_IMAGE_CS_YCBCR:
				switch ( cltype )
				{
					case JAS_IMAGE_CT_YCBCR_Y: yimage += 2; break;
					case JAS_IMAGE_CT_YCBCR_CB: yimage += 1; break;
					case JAS_IMAGE_CT_YCBCR_CR: yimage += 0; break;
					default: yimage += 3; break; // assume this is alpha channel ;-)
				}
				break;
		}

		for ( int y=0; y<ysize; ++y )
		{
			if ( jas_image_readcmpt(jasimage,i,0,y,xsize,1,matrix) )
			{
				jas_matrix_destroy(matrix);
				return false;
			}

			uint8* buffer = yimage;
			for ( int x=0; x<xsize; ++x )
			{
				int v = jas_matrix_getv(matrix,x) << shifter;
				uint8 s = sign ? (uint8)(v + 128) : (uint8)v;

				for ( int u=0; u<xstep; ++u )
				{
					uint8* dest = buffer;
					for ( int v=0; v<ystep; ++v )
					{
						*dest = s;
						dest += pitch;
					}
					buffer += bpp;
				}
			}
			yimage += ystep*pitch;
		}

		// release decoding matrix
		jas_matrix_destroy(matrix);
	}

	if ( colormodel == JAS_IMAGE_CS_YCBCR )
	{
		// YCrBr to RGB conversion
		int xsize = bitmap.GetWidth();
		int ysize = bitmap.GetHeight();
		int count = xsize * ysize;
		while ( count-- )
		{
			float y = image[2];
			float u = image[1];
			float v = image[0];

			float b = 1.164f*(y - 16)                    + 2.018f*(u - 128);
			float g = 1.164f*(y - 16) - 0.813f*(v - 128) - 0.391f*(u - 128);
			float r = 1.164f*(y - 16) + 1.596f*(v - 128);

			if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
			if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
			if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;

			image[0] = (uint8)r;
			image[1] = (uint8)g;
			image[2] = (uint8)b;

			image += bpp;
		}
	}

	return true;
}


static bool jp2_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read stream
	int size = stream.GetSize();
	char* data = new char[size];
	stream.Read(data,size);

	// initialize jasper
	static bool jasper_initialized = false;
	if ( !jasper_initialized )
	{
		jas_init();
		jasper_initialized = true;
	}

	// open jasper memory stream
	jas_stream_t* in = jas_stream_memopen(data,size);
	if ( !in )
	{
		delete[] data;
		PRCORE_EXCEPTION("Cannot create input stream.");
	}

	int infmt = jas_image_getfmt(in);
	if ( infmt < 0 || infmt > 4 )
	{
		delete[] data;
		jas_stream_close(in);
		PRCORE_EXCEPTION("Unrecognized input stream.");
	}

	jas_image_t* jasimage = jas_image_decode(in,infmt,NULL);
	if ( !in )
	{
		delete[] data;
		jas_stream_close(in);
		jas_image_destroy(jasimage);
		PRCORE_EXCEPTION("Cannot load image data.");
	}

	// get component parameters
	int cmptno = jas_image_numcmpts(jasimage);
	if ( cmptno < 1 || cmptno > 4 )
	{
		delete[] data;
		jas_stream_close(in);
		jas_image_destroy(jasimage);
		PRCORE_EXCEPTION("Incorrect number of components.");
	}

	// compute image size
	int sx = 0;
	int sy = 0;
	for ( int i=0; i<cmptno; ++i )
	{
		int xstep = jas_image_cmpthstep(jasimage,i);
		int ystep = jas_image_cmptvstep(jasimage,i);
		int xsize = jas_image_cmptwidth(jasimage,i);
		int ysize = jas_image_cmptheight(jasimage,i);

		int dx = jas_image_cmpttlx(jasimage,i) + xsize*xstep;
		int dy = jas_image_cmpttly(jasimage,i) + ysize*ystep;
		if ( dx > sx ) sx = dx;
		if ( dy > sy ) sy = dy;
	}

	// select pixel format with number of components in mind
	PixelFormat pxf;
	switch ( cmptno )
	{
		case 1:	pxf = PixelFormat(8,0xff,0); break;
		case 2:	pxf = PixelFormat(16,0x00ff,0xff00); break;
		case 3:	pxf = PixelFormat(24,0x0000ff,0x00ff00,0xff0000,0); break;
		case 4:	pxf = PixelFormat(32,0x000000ff,0x0000ff00,0x00ff0000,0xff000000); break;
	}

	// create image object
	target = Bitmap(sx,sy,pxf);

	// compute color values
	JasperImage(target,jasimage);

	// cleanup
	delete[] data;
	jas_image_destroy(jasimage);
	jas_stream_close(in);

	// TODO: temporary fix..
	if ( sx < 64 || sy < 64 )
		target.ResizeImage(128,128,false);

	return true;
}


// =================================================
// CreateCodecJP2()
// =================================================

BitmapCodec CreateCodecJP2()
{
	static const String ext[] = { "jp2", "jpc", "j2k" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = jp2_decode;
	codec.encode    = NULL;

	return codec;
}
