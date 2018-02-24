/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_CPU_HPP
#define PRCORE_CPU_HPP


#include <cstddef>


namespace prcore
{

	// cpu capabilities

	bool IsMMX();
	bool IsMMX_EXT();
	bool Is3DNOW();
	bool Is3DNOW_EXT();
	bool IsSSE();
	bool IsSSE2();

	// SIMD optimized memcpy function

	extern void* (*simd_memcpy)(void* dest, const void* src, size_t count);

} // namespace prcore


#endif