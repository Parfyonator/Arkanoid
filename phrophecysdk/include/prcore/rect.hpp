/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_RECT_HPP
#define PRCORE_RECT_HPP

namespace prcore
{

	struct Rect
	{
		int x;
		int y;
		int width;
		int height;

		Rect() {}
		Rect(int x, int y, int width, int height);
		Rect(const Rect& rect);

		float GetAspect() const;
		bool Intersect(const Rect& a, const Rect& b);
		bool PointInside(int px, int py);
	};

} // namespace prcore

#endif
