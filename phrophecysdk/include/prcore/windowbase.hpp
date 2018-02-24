/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_WINDOWBASE_HPP
#define PRCORE_WINDOWBASE_HPP

#include "configure.hpp"
#include "refcount.hpp"
#include "rect.hpp"
#include "string.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"

namespace prcore {

	enum {

		// window style flags

		WINDOW_SIZE			= 0x001, // window has resize button
		WINDOW_CLOSE		= 0x002, // window has close button
		WINDOW_MINIMIZE		= 0x004, // window has minimize button
		WINDOW_FRAME		= 0x008, // window has frame
		WINDOW_TITLE		= 0x010, // window has title
		WINDOW_QUITONCLOSE	= 0x080, // terminate application on close
		WINDOW_USES3D		= 0x100, // window supports 3d acceleration
		WINDOW_ALWAYSONTOP	= 0x200, // always on top
		WINDOW_CHILD		= 0x500, // child window

		// pre-defined styles

		WINDOW_FULLSCREEN	= WINDOW_QUITONCLOSE | WINDOW_USES3D,
		WINDOW_NORMAL		= 0xffffffff & ~WINDOW_ALWAYSONTOP

	};


	class WindowBase : public RefCount
	{
		public:

		WindowBase();
		WindowBase(WindowHandle parent);
		~WindowBase();

		bool		OpenWindow(int width, int height, const String& name, uint32 windowstyle, bool show = true);
	 	bool		OpenWindow(WindowHandle parent, int width, int height);
		bool		CloseWindow(bool mainbreak = true);
		bool		BindWindow(WindowHandle handle);
		bool		UnbindWindow();

		void		RenameWindow(const String& name);
		void		ResizeWindow(int width, int height);
		void		MoveWindow(int x, int y);
		void		ShowCursor();
		void		HideCursor();
		void		ShowWindow();
		void		HideWindow();
		void		EnableEvent();
		void		DisableEvent();

		void		SetMainFrequency(float freq);
		float		GetMainFrequency() const;

		bool		IsWindowOpen() const;
		bool		IsWindowBind() const;
		bool		IsEventEnable() const;
		uint32		GetWindowStyle() const;
	WindowHandle	GetWindowHandle() const;
		Rect		GetWindowRect(bool frame = false) const;
		float		GetWindowAspect() const;
		bool		IsDown(int keycode) const;

		void		DispatchEventKeyboard(int keycode, char charcode, bool press);
		void*		GetPrevProc() const;

		void		MainBreak();
		void		MainLoop();

		// virtual events

		virtual	bool EventMain()
		{
			// default handler: keep running (return false)
			return true;
		}

		virtual	void EventDraw()
		{
			// default handler: do nothing
		}

		virtual	void EventSize(int width, int height)
		{
			// default handler: do nothing
		}

		virtual	void EventKeyboard(int keycode, bool press)
		{
			// default handler: do nothing
		}

		virtual	void EventKeyboard(int keycode, char charcode, bool press)
		{
			// default handler: chain to simpler
			EventKeyboard(keycode,press);
		}

		virtual	void EventFocus(bool enter)
		{
			// default handler: do nothing
		}

		virtual	void EventMouseMove(int x, int y)
		{
			// default handler: do nothing
		}

		virtual void EventMouseButton(int x, int y, MouseButton button, int count)
		{
			// default handler: do nothing

			// count 0, button release
			// count 1, button press
			// count 2, button double-click

			// count has a special meaning for MOUSE_WHEEL:
			// - count > 0, wheel is rotated forward
			// - count < 0, wheel is rotated backward
		}

		virtual	void EventDropFile(const String& filename)
		{
			// default handler: do nothing
		}

		virtual	void EventClose()
		{
			// default handler: do nothing
		}

		protected:

	WindowHandle	mWindowParent;
	WindowHandle	mWindowHandle;
		uint32		mWindowStyle;

		private:

		float		mMainFreq;
		void*		mpPrevProc;
		bool		mIsOpen;
		bool		mIsBind;
		bool		mIsEventEnable;
		bool		mMainBreak;
		bool		mKeyDown[256];
	};

} // namespace prcore

#endif
