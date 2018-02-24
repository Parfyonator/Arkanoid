/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_FRAMEBUFFER_HPP
#define PRCORE_FRAMEBUFFER_HPP

#include "surface.hpp"
#include "windowbase.hpp"

namespace prcore {

	struct VideoMode
	{
		int width;
		int height;
		int bits;
	};

	class FrameBuffer : public WindowBase
	{
		public:

		FrameBuffer();
		~FrameBuffer();

		int			GetModeCount() const;
		VideoMode	GetMode(int index) const;

		bool		OpenBuffer(WindowHandle handle);
		bool		OpenBuffer(WindowHandle parent, int width, int height);
		bool		OpenBuffer(int width, int height, const String& name, uint32 windowstyle = WINDOW_NORMAL);
		bool		OpenBuffer(int width, int height, int bits, const String& name, uint32 windowstyle = WINDOW_FULLSCREEN);
		bool		CloseBuffer(bool mainbreak = true);

		void		SetPalette(const color32 palette[]);
		void		BlitWrite(int x, int y, const Surface& source);

		bool		ResizeBuffer(int width, int height);
		void		ClearBuffer(color32 color);
		Surface*	LockBuffer();
		void		UnlockBuffer();

		void		PageFlip(bool retrace = true);

		private:

		void*		mpContext;
	};

} // namespace prcore

#endif
