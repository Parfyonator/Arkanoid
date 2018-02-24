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
		surface implementation

	revision history:
		Jan/24/2001 - Jukka Liimatta - initial revision/renaissance build
		Apr/16/2002 - Jukka Liimatta - added ::SaveImage() method
		Jan/20/2003 - Jukka Liimatta - added quality parameter for ::SaveImage()
		May/16/2003 - Jukka Liimatta - fixed ::SaveImage() error message
*/
#include <cassert>
#include <prcore/exception.hpp>
#include <prcore/filename.hpp>
#include <prcore/filestream.hpp>
#include <prcore/bitmap.hpp>

namespace prcore
{
        

    Surface::Surface()
    : width(0),height(0),pitch(0),image(NULL)
    {
    }


    Surface::Surface(int w, int h, int p, const PixelFormat& pxf, void* i)
    : width(w),height(h),pitch(p),format(pxf),image(reinterpret_cast<uint8*>(i))
    {
        assert( width > 0 && height > 0 );
    }


    Surface::~Surface()
    {
    }


    Surface Surface::GetSurface(const Rect& rect) const
    {
        // clip
        Rect q;
        if ( !q.Intersect(GetRect(),rect) )
            return *this;
            
        Surface surface;

        surface.width  = q.width;
        surface.height = q.height;
        surface.pitch  = pitch;
        surface.format = format;
        surface.image  = image + q.y * pitch + q.x * format.GetBytes();
        
        return surface;
    }


    void Surface::SaveImage(const String& filename, float quality)
    {
        // assert
        assert( quality >= 0.0f && quality <= 1.0f );

        // find codec
        String ext = GetFilenameEXT(filename);
        const BitmapCodec* codec = BitmapCodecManager::manager.FindInterface(ext);

        if ( !codec )
            PRCORE_EXCEPTION(String("Cannot find codec for extension: ") + ext);

        if ( !codec->encode )
            PRCORE_EXCEPTION("The codec is not encoder.");
                
        // create stream
        FileStream stream(filename,Stream::WRITE);
        if ( !stream.IsOpen() )
            return;
                
        // encode
        codec->encode(stream,*this,quality);
    }


    void Surface::BlitImage(int x, int y, const Surface& source)
    {
        // assert
        assert( image && source.image );

        // clip
        Rect rect;
        if ( !rect.Intersect(
            Rect(0,0,width,height),
            Rect(x,y,source.width,source.height) ))
            return;

        int sx = rect.x - x;
        int sy = rect.y - y;

        // setup blit surfaces
        int bpp1 = format.GetBytes();
        int bpp2 = source.format.GetBytes();
        uint8* image1 = image + rect.y * pitch + rect.x * bpp1;
        uint8* image2 = source.image + sy * source.pitch + sx * bpp2;

        Surface s1(rect.width,rect.height,pitch,format,image1);
        Surface s2(rect.width,rect.height,source.pitch,source.format,image2);

        // blit
        s1.BlitImage(s2,BLIT_COPY);
    }    
}
