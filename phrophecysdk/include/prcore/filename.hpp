/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_FILENAME_HPP
#define PRCORE_FILENAME_HPP


#include "string.hpp"


namespace prcore
{

	// filename manipulation functions

	String GetFilenameEXT(const String& filename);		// get extension of given filename
	String GetFilenamePATH(const String& filename);		// get path of given filename
	String GetFilenameNOEXT(const String& filename);	// get filename without extension
	String GetFilenameNOPATH(const String& filename);	// get filename without path

} // namespace prcore


#endif