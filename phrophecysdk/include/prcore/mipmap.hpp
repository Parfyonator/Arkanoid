/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_MIPMAP_HPP
#define PRCORE_MIPMAP_HPP


#include "surface.hpp"
#include "bitmap.hpp"


namespace prcore
{

	Bitmap CreateMipLevel(const Surface& surface, int sublevel, bool alphafilter = false);

} // namespace prcore


#endif