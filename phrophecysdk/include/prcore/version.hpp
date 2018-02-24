/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_VERSION_HPP
#define PRCORE_VERSION_HPP


namespace prcore
{

	// version information

	int				GetLibraryBuild();
	float			GetLibraryVersion();
	const char*		GetLibraryDate();
	const char*		GetLibraryTime();

} // namespace prcore


#endif