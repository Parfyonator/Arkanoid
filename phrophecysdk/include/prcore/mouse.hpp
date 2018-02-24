/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_MOUSE_HPP
#define PRCORE_MOUSE_HPP


#include "rect.hpp"
#include "inputdevice.hpp"


namespace prcore
{

	enum MouseAxis
	{
		MOUSE_X,
		MOUSE_Y,
	};

	enum MouseButton
	{
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		MOUSE_XBUTTON1,
		MOUSE_XBUTTON2,
		MOUSE_WHEEL
	};

	class Mouse : public InputBase
	{
		public:

		Mouse();
		~Mouse();

		int			GetAxisCount() const;
		int			GetButtonCount() const;
		float		GetAxis(int axis);
		int			GetButton(int button);

		void		SetArea(const Rect& rect);
		void		SetXY(int x, int y);
		int			GetX();
		int			GetY();
	};

} // namespace prcore


#endif