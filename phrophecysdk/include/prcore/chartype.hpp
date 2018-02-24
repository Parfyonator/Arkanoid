/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_CHARTYPE_HPP
#define PRCORE_CHARTYPE_HPP

#if defined(_MSC_VER) && (_MSC_VER <= 1200) && !defined(_STLPORT_VERSION)
#define PRCORE_CHARTYPE_DISABLE1
#endif

#include <ctype.h>
#include <cwctype>

namespace prcore
{

	// case conversion templates
	// NOTE: obsolete, but kept for backward compatibility with
	//       older SDK versions

	template <typename S>
	inline S tolower(S v)
	{
		return v;
	}

	template <typename S>
	inline S toupper(S v)
	{
		return v;
	}

	template <>
	inline char tolower(char v)
	{
		return ::tolower(v);
	}

	template <>
	inline char toupper(char v)
	{
		return ::toupper(v);
	}

	#ifndef PRCORE_CHARTYPE_DISABLE1

	template <>
	inline uchar16 tolower(uchar16 v)
	{
		return ::towlower(v);
	}

	template <>
	inline uchar16 toupper(uchar16 v)
	{
		return ::towupper(v);
	}

	#endif

} // namespace prcore

#ifdef PRCORE_CHARTYPE_DISABLE1
#undef PRCORE_CHARTYPE_DISABLE1
#endif

#endif