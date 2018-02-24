/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


	source:
	Plugin Linux implementation
		
	revision history:
	???/??/2001 - Edmond Cote - initial revision
	Feb/28/2004 - Andre Reschke - rewrite for the newer version of the SDK
*/
#include <prcore/configure.hpp>
#include <prcore/plugin.hpp>

#include <dlfcn.h>

using namespace prcore;

// ================================================
// extension
// ================================================
	#ifndef NDEBUG
	#define DLL_EXTENSION "Debug.so"
	#else
	#define DLL_EXTENSION ".so"
	#endif
	
// ================================================
// Plugin
// ================================================

Plugin::Plugin( const char* filename, bool rawmode )
{
	int length = (int)strlen(filename) + (int)strlen(DLL_EXTENSION) + 1;
	char *name = new char[length];
	strcpy(name, filename);
	if ( !rawmode )
		strcat(name, DLL_EXTENSION);
		
	handle = dlopen(name, RTLD_LAZY);
	
	delete[] name;
	
	SetName(filename);
}

Plugin::~Plugin()
{
	if (handle)
		dlclose(handle);
}

void Plugin::GetAddress( void** address, const char* symbol)
{
	assert(symbol);
	assert(address);
	*address = handle ? (void*)dlsym(handle, symbol) : NULL;
}


