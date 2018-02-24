/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_JOYSTICK_HPP
#define PRCORE_JOYSTICK_HPP


#include "configure.hpp"
#include "inputdevice.hpp"


namespace prcore
{

	class Joystick : public InputBase
	{
		public:

		Joystick(WindowHandle handle, int index = 0);
		~Joystick();

		int			GetAxisCount() const;
		int			GetButtonCount() const;
		int			GetPOVCount() const;
		float		GetAxis(int);
		int			GetButton(int code);
		int			GetPOV(int);

		static int			GetJoystickCount();
		static const char*	GetJoystickName(int index);

		private:

		class JoystickRep*	mpRep;
	};

} // namespace prcore


#endif