/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_ENDIAN_HPP
#define PRCORE_ENDIAN_HPP


#include "configure.hpp"


namespace prcore
{

	// byte swaps

	inline void ByteSwap(uint8&)
	{
		// satisfy macros defined later
	}

	inline void ByteSwap(int8&)
	{
		// satisfy macros defined later
	}

	inline void ByteSwap(uint16& i)
	{
		i = static_cast<uint16>((i << 8) | (i >> 8));
	}

	inline void ByteSwap(int16& i)
	{
		ByteSwap(reinterpret_cast<uint16&>(i));
	}

	inline void ByteSwap(uint32& i)
	{
		i = (i >> 24) | (i >> 8) & 0x0000ff00 | (i << 8) & 0x00ff0000 | (i << 24);
	}

	inline void ByteSwap(int32& i)
	{
		ByteSwap(reinterpret_cast<uint32&>(i));
	}

	inline void ByteSwap(float32& v)
	{
		ByteSwap(reinterpret_cast<uint32&>(v));
	}

	inline void ByteSwap(uint64& i)
	{
		uint32* p = reinterpret_cast<uint32*>(&i);
		uint32 u = (p[0] >> 24) | (p[0] << 24) | ((p[0] & 0x00ff0000) >> 8) | ((p[0] & 0x0000ff00) << 8);
		p[0] = (p[1] >> 24) | (p[1] << 24) | ((p[1] & 0x00ff0000) >> 8) | ((p[1] & 0x0000ff00) << 8);
		p[1] = u;
	}

	inline void ByteSwap(int64& i)
	{
		ByteSwap(reinterpret_cast<uint64&>(i));
	}

	inline void ByteSwap(float64& v)
	{
		ByteSwap(reinterpret_cast<uint64&>(v));
	}

	// remap templates

	template <class T>
	inline T LittleEndianRemap(T v)
	{
		#ifdef PRCORE_BIG_ENDIAN
		ByteSwap(v);
		#endif
		return v;
	}

	template <class T>
	inline T BigEndianRemap(T v)
	{
		#ifdef PRCORE_LITTLE_ENDIAN
		ByteSwap(v);
		#endif
		return v;
	}

	// implementation note:
	//
	// we must do the following conversion char-by-char so that
	// machines with strict alignment rules will not generate
	// unaligned memory access exception.

	template <class T, class R>
	inline T ReadLittleEndianBuffer(R& buffer)
	{
		assert( sizeof(buffer[0]) == 1 );

		T v;
		char*& ps = reinterpret_cast<char*&>(buffer);

		#ifdef PRCORE_LITTLE_ENDIAN

		// little-to-little endian conversion:
		char* pv = reinterpret_cast<char*>(&v);
		for ( int i=0; i<sizeof(T); ++i ) {
			*pv++ = *ps++; }

		#else

		// little-to-big endian conversion:
		char* pv = reinterpret_cast<char*>(&v) + sizeof(T);
		for ( int i=0; i<sizeof(T); ++i ) {
			*--pv = *ps++; }

		#endif

		return v;
	}

	template <class T, class R>
	inline T ReadBigEndianBuffer(R& buffer)
	{
		assert( sizeof(buffer[0]) == 1 );

		T v;
		char*& ps = reinterpret_cast<char*&>(buffer);

		#ifdef PRCORE_LITTLE_ENDIAN

		// little-to-big endian conversion:
		char* pv = reinterpret_cast<char*>(&v) + sizeof(T);
		for ( int i=0; i<sizeof(T); ++i ) {
			*--pv = *ps++; }

		#else

		// big-to-big endian conversion:
		char* pv = reinterpret_cast<char*>(&v);
		for ( int i=0; i<sizeof(T); ++i ) {
			*pv++ = *ps++; }

		#endif

		return v;
	}

	// runtime endianess check

	inline bool IsLittleEndian()
	{
		uint32 v = 1;
		return *reinterpret_cast<uint8*>(&v) != 0;
	}

	inline bool IsBigEndian()
	{
		uint32 v = 1;
		return *reinterpret_cast<uint8*>(&v) == 0;
	}

} // namespace prcore


#endif