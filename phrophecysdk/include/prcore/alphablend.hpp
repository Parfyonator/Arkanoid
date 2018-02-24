/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_ALPHABLEND_HPP
#define PRCORE_ALPHABLEND_HPP

#include "surface.hpp"

namespace prcore
{

	enum AlphaFunc
	{
		ZERO,			// 0
		ONE,			// 1
		SRCCOLOR,		// src.rgb
		SRCINVCOLOR,	// 1 - src.rgb
		SRCALPHA,		// src.alpha
		SRCINVALPHA,	// 1 - src.alpha
		DESTCOLOR,		// dest.rgb
		DESTINVCOLOR,	// 1 - dest.rgb
		DESTALPHA,		// dest.alpha
		DESTINVALPHA,	// 1 - dest.alpha
	};

	void SetConstantAlpha(uint8 mult0, uint8 mult1);
	void SetAlphaFunc(AlphaFunc func0, AlphaFunc func1);

	void AlphaBlend(uint32* dest, uint32* source0, uint32* source1, int count);
	void AlphaBlend(int x, int y, Surface& dest, const Surface& source);
	void AlphaBlendVolatile(Surface& dest, const Surface& source0, const Surface& source1);

} // prcore

#endif
