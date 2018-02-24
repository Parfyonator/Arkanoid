/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_CONFIGURE_HPP
#define PRCORE_CONFIGURE_HPP


	// =============================================
	// common preprocessor macros
	// =============================================

	#ifndef PRCORE_FILENAME
	#ifndef __FILE__
	#define PRCORE_FILENAME		"Unknown File"
	#else
	#define PRCORE_FILENAME		__FILE__
	#endif
	#endif

	#ifndef PRCORE_FUNCTION
	#ifndef __FUNCTION__
	#define PRCORE_FUNCTION		"Unknown function"
	#else
	#define PRCORE_FUNCTION		__FUNCTION__
	#endif
	#endif

	#ifndef PRCORE_LINE
	#ifndef __LINE__
	#define PRCORE_LINE			-1
	#else
	#define PRCORE_LINE			__LINE__
	#endif
	#endif


	// =============================================
	// platform specific configuration
	// =============================================

	#ifndef PRCORE_CONFIGURED
	#if defined(_WIN32) || defined(_WINDOWS_)

		#define PRCORE_PLATFORM_WIN32
		#define PRCORE_LITTLE_ENDIAN
		#define PRCORE_X86_SIMD

		#include "windows.hpp"

	#endif
	#endif

	#ifndef PRCORE_CONFIGURED
	#if defined(__linux__)

		#define PRCORE_PLATFORM_LINUX
		#define PRCORE_LITTLE_ENDIAN
		#define PRCORE_X86_SIMD

		#include "unix.hpp"

	#endif
	#endif

	#ifndef PRCORE_CONFIGURED
	#if defined(__sgi__)

		#define PRCORE_PLATFORM_IRIX
		#define PRCORE_BIG_ENDIAN

		#include "unix.hpp"

	#endif
	#endif

	#ifndef PRCORE_CONFIGURED
	#if defined(__BEOS__)

		#define PRCORE_PLATFORM_BEOS
		#define PRCORE_LITTLE_ENDIAN
		#define PRCORE_X86_SIMD

		#include "beos.hpp"

	#endif
	#endif

    #ifndef PRCORE_CONFIGURED
    #if defined(__APPLE__) && defined(__ppc__)
        #define PRCORE_PLATORM_OSX
        #define PRCORE_BIG_ENDIAN
        #include "osx.hpp"
    #endif
    #endif

	#ifndef PRCORE_CONFIGURED
	#error "platform not supported, e-mail: support@twilight3d.com"
	#endif


#endif