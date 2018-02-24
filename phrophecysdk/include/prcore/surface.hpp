/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_SURFACE_HPP
#define PRCORE_SURFACE_HPP


#include "rect.hpp"
#include "color32.hpp"
#include "pixelformat.hpp"
#include "refcount.hpp"
#include "string.hpp"


namespace prcore
{

	class Surface : public RefCount
	{
		public:
		
		enum ClearMode
		{
			CLEAR_COLOR			= 1,
			CLEAR_ALPHA			= 2,
			CLEAR_COLOR_ALPHA	= CLEAR_COLOR | CLEAR_ALPHA
		};

		enum BlitMode
		{
			BLIT_COPY,
			BLIT_SCALE,
			BLIT_BILINEAR_SCALE
		};
	
		Surface();
		Surface(int width, int height, int pitch, const PixelFormat& format, void* image);
		~Surface();

		int GetWidth() const
		{
			return width;
		}

		int GetHeight() const
		{
			return height;
		}

		int GetPitch() const
		{
			return pitch;
		}

		const PixelFormat& GetPixelFormat() const
		{
			return format;
		}

		uint8* GetImage() const
		{
			return image;
		}

		Rect GetRect() const
		{
			return Rect(0,0,width,height);
		}

		Surface		GetSurface(const Rect& rect) const;
		void		SaveImage(const String& filename, float quality = 1.0f);
		void		ClearImage(const color32& color, ClearMode mode = CLEAR_COLOR_ALPHA);
		void		BlitImage(const Surface& source, BlitMode mode = BLIT_COPY);
		void		BlitImage(int x, int y, const Surface& source);
		void		FlipImageY();

		protected:

		int			width;		// width in pixels
		int			height;		// height in pixels
		int			pitch;		// width in bytes
		PixelFormat	format;		// pixelformat
		uint8*		image;		// image pointer

		private:

		void		ClearImageFloat(const color32& color, ClearMode mode);
		void		BlitImageFloat(const Surface& source, BlitMode mode);
	};

} // namespace prcore


#endif