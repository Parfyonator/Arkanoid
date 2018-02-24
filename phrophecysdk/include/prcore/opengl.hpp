/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_OPENGL_HPP
#define PRCORE_OPENGL_HPP


#include "windowbase.hpp"


namespace prcore
{

	typedef void ContextGL;


	struct ModeGL
	{
		uint32 width;		// width of videomode in pixels
		uint32 height;		// height of videomode in pixels
		uint32 bits;		// color depth of videomode in bytes
		uint32 frequency;	// frequency of videomode in hz (0 = default)
	};


	class WindowGL : public WindowBase
	{
		public:

		WindowGL();
		~WindowGL();

		// mode
		int				GetModeCount() const;
		const ModeGL&	GetMode(int index) const;
		bool			SetMode(const ModeGL& mode);
		bool			SetMode(uint32 width, uint32 height, uint32 bits, uint32 frequency = 0);
		bool			RestoreMode();

		// context
		ContextGL*		CreateContext(uint32 color = 32, uint32 depth = 24, uint32 stencil = 8, uint32 accum = 0);
		void			DeleteContext(ContextGL* context);
		void			SetContext(ContextGL* context);

		void			PageFlip(bool retrace = true);

		private:

		ContextGL*		mCurrentContext;
	};

} // namespace prcore


#endif