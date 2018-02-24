/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_ZLIB_HPP
#define PRCORE_ZLIB_HPP


#include <prcore/configure.hpp>


namespace prcore
{

	// zlib compression
	// ==========================================================================
	// dest      - pointer to destination buffer
	// destlen   - must be atleast 0.1% larger than sourcelen plus 12 bytes
	// source    - pointer to source buffer
	// sourcelen - size of source buffer in bytes
	//
	// zlib_encode() returns number of bytes encoded in the destination buffer

	uint32 zlib_encode(uint8* dest, uint32 destlen, const uint8* source, uint32 sourcelen);


	// zlib decompression
	// ==========================================================================
	// dest      - pointer to destination buffer
	// destlen   - size of destination buffer ( sourcelen parameter of zlib_encode() )
	// source    - pointer to source buffer
	// sourcelen - size of source buffer in bytes ( return value of zlib_encode() )
	//
	// zlib_decode() returns number of bytes decoded in the destination buffer

	uint32 zlib_decode(uint8* dest, uint32 destlen, const uint8* source, uint32 sourcelen);

} // namespace prcore


#endif