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
		bitmap implementation

	revision history:
		Dec/18/1998 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Feb/21/2002 - Jukka Liimatta - created BitmapCodecManager
		Jul/15/2002 - Jukka Liimatta - greetings to stefan/dxm who tried to harass me while upgrading the code today
*/
#include <cassert>
#include <prcore/exception.hpp>
#include <prcore/filename.hpp>
#include <prcore/filestream.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// ========================================
// CopyImage()
// ========================================

static inline void CopyImage(uint8* d, uint8* s, int dpitch, int spitch, int count)
{
	if ( count > 0 )
	{
		while ( count-- )
		{
			memcpy(d,s,dpitch);
			d += dpitch;
			s += spitch;
		}
	}
}


// ========================================
// Bitmap
// ========================================

Bitmap::Bitmap()
{
}


Bitmap::Bitmap(const Bitmap& src)
{
	*this = src;
}


Bitmap::Bitmap(const Surface& surface)
{
	width  = surface.GetWidth();
	height = surface.GetHeight();
	format = surface.GetPixelFormat();
	pitch  = width * format.GetBytes();
	image  = new uint8[height*pitch];

	// copy image
	CopyImage(image,surface.GetImage(),pitch,surface.GetPitch(),height);
}


Bitmap::Bitmap(int xsize, int ysize, const PixelFormat& pxf)
{
	assert( xsize > 0 && ysize > 0 );

	width  = xsize;
	height = ysize;
	pitch  = width * pxf.GetBytes();
	format = pxf;
	image  = new uint8[height*pitch];
}


Bitmap::Bitmap(const String& filename)
{
	LoadImage(filename);
}


Bitmap::Bitmap(const String& filename, const PixelFormat& pxf)
{
	LoadImage(filename);
	ReformatImage(pxf);
}


Bitmap::~Bitmap()
{
	delete[] image;
}


void Bitmap::operator = (const Bitmap& src)
{
	assert( src.width > 0 && src.height > 0 );

	if ( src.image )
	{
		// reallocate image
		if ( pitch!=src.pitch || height!=src.height )
		{
			delete[] image;
			image = new uint8[src.pitch*src.height];
		}

		// copy image
		CopyImage(image,src.image,src.pitch,src.pitch,src.height);
	}
	else
	{
		delete[] image;
		image = NULL;
	}
	
	width  = src.width;
	height = src.height;
	pitch  = src.pitch;
	format = src.format;
}


Bitmap Bitmap::GetBitmap(const Rect& area) const
{
	// assert
	assert( image );

	// test area
	if ( area.width < 1 || 
		area.height < 1 || 
		area.x < 0 || 
		area.y < 0 || 
		(area.x+area.width) > width || 
		(area.y+area.height) > height ) 
		return *this;

	// clip
	Rect q;
	if ( !q.Intersect(GetRect(),area) )
		return *this;

	// new bitmap
	Bitmap p(q.width,q.height,format);

	// copy image
	CopyImage(p.image,
		image + q.y * pitch + q.x * format.GetBytes(),
		p.pitch,pitch,q.height);

	return p;
}


void Bitmap::LoadImage(Stream& stream, const String& filename)
{
	// find codec
	String ext = GetFilenameEXT(filename);
	const BitmapCodec* codec = FindBitmapCodec(ext);

	if ( !codec )
		PRCORE_EXCEPTION(String("Cannot find codec for extension: ") + ext);

	if ( !codec->decode )
		PRCORE_EXCEPTION("The codec is not decoder.");

	// decode
	codec->decode(*this,stream);
		
	// set interface name
	SetName(filename);
}


void Bitmap::LoadImage(Stream& stream, const String& filename, const PixelFormat& pxf)
{
	LoadImage(stream,filename);
	ReformatImage(pxf);
}


void Bitmap::LoadImage(const String& filename)
{
	// find codec
	String ext = GetFilenameEXT(filename);
	const BitmapCodec* codec = FindBitmapCodec(ext);

	if ( !codec )
		PRCORE_EXCEPTION(String("Cannot find codec for extension: ") + ext);

	if ( !codec->decode )
		PRCORE_EXCEPTION("The codec is not decoder.");

	// create stream
	FileStream stream(filename,Stream::READ);
		
	// decode
	codec->decode(*this,stream);
		
	// set interface name
	SetName(filename);
}


void Bitmap::LoadImage(const String& filename, const PixelFormat& pxf)
{
	LoadImage(filename);
	ReformatImage(pxf);
}


void Bitmap::SetImage(int xsize, int ysize, const PixelFormat& pxf, void* i)
{
	assert( xsize > 0 && ysize > 0 );

	delete[] image;	

	width  = xsize;
	height = ysize;
	pitch  = xsize * pxf.GetBytes();
	format = pxf;
	image  = reinterpret_cast<uint8*>(i);
}


void Bitmap::ResizeImage(int xsize, int ysize, bool filter)
{
	// assert
	assert( image );
	if ( width == xsize && height == ysize )
		return;

	// choose blitmode
	BlitMode mode = filter ? BLIT_BILINEAR_SCALE : BLIT_SCALE;

	// conversion
	Bitmap temp(xsize,ysize,format);
	temp.BlitImage(*this,mode);
	
	// swap pointers
	delete[] image;
	image = temp.image;
	temp.image = NULL;
	
	format = temp.format;
	width  = temp.width;
	height = temp.height;
	pitch  = temp.pitch;
}


void Bitmap::ReformatImage(const PixelFormat& pxf)
{
	assert( image );
	if ( format == pxf )
		return;

	// indexed reformat means color quantization
	if ( pxf.IsIndexed() )
	{
		QuantizeImage();
		return;
	}

	// conversion
	Bitmap temp(width,height,pxf);
	temp.BlitImage(*this,BLIT_COPY);
	
	// swap pointers
	delete[] image;
	image = temp.image;
	temp.image = NULL;
	
	pitch = temp.pitch;
	format = pxf;
}


// ========================================
// BitmapCodecManager
// ========================================

// the manager instance
BitmapCodecManager BitmapCodecManager::manager;

// forward declarations
BitmapCodec CreateCodecTGA();
BitmapCodec CreateCodecPCX();
BitmapCodec CreateCodecRGB();
BitmapCodec CreateCodecBMP();
BitmapCodec CreateCodecJPG();
BitmapCodec CreateCodecPNG();
BitmapCodec CreateCodecDDS();
BitmapCodec CreateCodecJP2();
BitmapCodec CreateCodecRAS();
BitmapCodec CreateCodecIFF();
BitmapCodec CreateCodecGIF();


BitmapCodecManager::BitmapCodecManager()
{
	manager.RegisterInterface(CreateCodecTGA());
	manager.RegisterInterface(CreateCodecPCX());
	manager.RegisterInterface(CreateCodecRGB());
	manager.RegisterInterface(CreateCodecBMP());
	manager.RegisterInterface(CreateCodecJPG());
	manager.RegisterInterface(CreateCodecPNG());
	manager.RegisterInterface(CreateCodecDDS());
	manager.RegisterInterface(CreateCodecJP2());
	manager.RegisterInterface(CreateCodecRAS());
	manager.RegisterInterface(CreateCodecIFF());
	manager.RegisterInterface(CreateCodecGIF());
}


int BitmapCodecManager::GetVersion() const
{
	return 1;
}
