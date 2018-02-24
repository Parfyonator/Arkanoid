/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_STREAM_HPP
#define PRCORE_STREAM_HPP


#include "refcount.hpp"
#include "endian.hpp"


namespace prcore
{
	
	class Stream : public RefCount
	{
		public:

		enum AccessMode
		{
			READ		= 1,
			WRITE		= 2,
			READWRITE	= READ | WRITE
		};

		enum SeekMode
		{
			START,		// seek from the start of the stream
			CURRENT,	// seek from the current stream position
			END			// seek from the end of the stream
		};

		virtual	int		Seek(int delta, SeekMode mode) = 0;
		virtual	int		Read(void* target, int bytes) = 0;
		virtual	int		Write(const void* source, int bytes) = 0;
		virtual	int		GetOffset() const = 0;
		virtual	int		GetSize() const = 0;
		virtual	bool	IsOpen() const = 0;
		virtual	bool	IsEOS() const = 0;
	};


	// endian templates

	template <typename T> 
	T ReadLittleEndian(Stream& stream)
	{
		T v;
		stream.Read(&v,sizeof(v));
		return LittleEndianRemap(v);
	}

	template <typename T>
	T ReadBigEndian(Stream& stream)
	{
		T v;
		stream.Read(&v,sizeof(v));
		return BigEndianRemap(v);
	}

	template <typename T>
	void WriteLittleEndian(Stream& stream, T v)
	{
		v = LittleEndianRemap(v);
		stream.Write(&v,sizeof(v));
	}

	template <typename T>
	void WriteBigEndian(Stream& stream, T v)
	{
		v = BigEndianRemap(v);
		stream.Write(&v,sizeof(v));
	}

} // namespace prcore


#endif