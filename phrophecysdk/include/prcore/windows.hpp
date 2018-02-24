
	#ifndef PRCORE_CONFIGURE_HPP
	#error "NEVER manually include this header."
	#endif

	#define PRCORE_CONFIGURED

	// compiler-specific settings

	#if (_MSC_VER >= 1000) || defined(__VISUALC__)
	#pragma warning(disable : 4251 4512 4786)
	#pragma inline_depth(255)
	#pragma inline_recursion(on)
	#pragma auto_inline(on)
	#endif

	// <windows.h> and related "issues"
#define _CRT_SECURE_NO_WARNINGS

	// don't define min/max macros
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif

	// "safe" to include <windows.h> now
	#include <windows.h>

	// fix standard header <algorithm>
	#include "../prmath/minmax.hpp"

	// interface

	#define PRCORE_EXPORT __declspec(dllexport)
	#define PRCORE_IMPORT __declspec(dllimport)

	#ifdef PRCORE_COMPILE_RUNTIME
	#define PRCORE_API PRCORE_EXPORT
	#else
	#define PRCORE_API PRCORE_IMPORT
	#endif

	// floating-point types

	#include "half.h"

	typedef half				float16;
	typedef float				float32;
	typedef double				float64;

	// integer types

	typedef	signed char			int8;
	typedef	unsigned char		uint8;
	typedef	signed short		int16;
	typedef	unsigned short		uint16;
	typedef signed int			int32;
	typedef unsigned int		uint32;
	typedef signed __int64		int64;
	typedef unsigned __int64	uint64;

	// unicode chartype

	typedef wchar_t				uchar16;

	// window handle

	struct WindowHandle
	{
		HWND hwnd;

		operator HWND () const
		{
			return hwnd;
		}
	};

	// runtime main() declaration

	#ifdef PRCORE_IMPLEMENT_MAIN

	#include "../prcore/string.hpp"
	#include "../prcore/exception.hpp"

	// user entrypoint
	extern int prmain(int argc, char** argv);

	// system entrypoint
	int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR commandline, int)
	{
		// -----------------------------------------------------------
		// original implementation:
		// www.flipcode.com - "Code Of The Day" - by Max McGuire
		// -----------------------------------------------------------

		// count the arguments
		int argc = 1;
		char* arg  = commandline;

		while ( arg[0] != 0 )
		{
			while ( *arg != 0 && *arg == ' ' )
				++arg;

			if ( *arg != 0 )
			{
				++argc;
				while ( *arg != 0 && *arg != ' ' )
					++arg;
			}
		}    

		// tokenize the arguments
		char** argv = new char*[argc];

		arg = commandline;
		int index = 1;

		while ( *arg != 0 )
		{
			while ( *arg != 0 && *arg == ' ' )
				++arg;

			if ( *arg != 0 )
			{
				argv[index] = arg;
				++index;

				while ( *arg != 0 && *arg != ' ' )
					++arg;

				if ( *arg != 0 )
					*arg++ = 0;
			}
		}    

		// put the program name into argv[0]
		char filename[_MAX_PATH];

		GetModuleFileName(NULL,filename,_MAX_PATH);
		argv[0] = filename;

		int result = 0;

		// exception handling
		try
		{
			// call the user specified main function
			result = prmain(argc,argv);
		}
		catch (prcore::Exception& e)
		{
			e.DrawDialog();
		}

		delete[] argv;
		return result;
	}

	#endif

	// runtime module declaration

	#ifdef PRCORE_COMPILE_RUNTIME

	// user entrypoint

	extern void prattach();
	extern void prdetach();

	// system entrypoint

	BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
	{
		switch( reason )
		{
			case DLL_PROCESS_ATTACH:
			case DLL_THREAD_ATTACH:
				prattach();
				break;

			case DLL_PROCESS_DETACH:
			case DLL_THREAD_DETACH:
				prdetach();
				break;

			default:
				break;
		}
		return TRUE;
	}

	#endif
