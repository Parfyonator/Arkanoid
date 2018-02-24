/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_INPUTDEVICE_HPP
#define PRCORE_INPUTDEVICE_HPP


#include "array.hpp"
#include "refcount.hpp"


namespace prcore
{

	class InputBase : public RefCount
	{
		public:

virtual	int		GetAxisCount() const;		// get number of axis
virtual	int		GetButtonCount() const;		// get number of buttons
virtual	float	GetAxis(int index);			// get state [-1.0,1.0]
virtual	int		GetButton(int index);		// is button pressed?

		bool IsDown(int index)
		{
			return GetButton(index) != 0;
		}
	};


	class InputDevice : public InputBase
	{
		public:

		InputDevice();
		~InputDevice();

		int		GetAxisCount() const;
		int		GetButtonCount() const;
		float	GetAxis(int);
		int		GetButton(int code);

		void	BindAxis(int dest, int src, InputBase&);
		void	BindButton(int dest, int src, InputBase&);

		private:

		struct BindIO
		{
			InputBase*	input;
			int			dest;
			int			src;
		};

		typedef prcore::Array<BindIO> BindIOArray;

		BindIOArray		axisbinds;
		BindIOArray		buttonbinds;

		BindIO*			FindBindIO(BindIOArray&, int index);
	};

} // namespace prcore


#endif