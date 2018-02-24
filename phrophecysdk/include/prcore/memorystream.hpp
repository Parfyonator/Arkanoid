/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_MEMORYSTREAM_HPP
#define PRCORE_MEMORYSTREAM_HPP

#include "configure.hpp"
#include "stream.hpp"
#include "array.hpp"

namespace prcore
{

	class MemoryStream : public Stream, public Array<uint8>
	{
		public:

		MemoryStream();
		MemoryStream(int size);
		MemoryStream(const uint8* data, int size);
		MemoryStream(const String& filename);
		~MemoryStream();

		int Seek(int delta, SeekMode mode);
		int Read(void* target, int bytes);
		int Write(const void* source, int bytes);

		int GetOffset() const
		{
			return msoffset;
		}

		int GetSize() const
		{
			return Array<uint8>::GetSize();
		}

		bool IsOpen() const
		{
			return true;
		}

		bool IsEOS() const
		{
			return msoffset >= GetSize();
		}

		uint8* GetData()
		{
			return GetArray();
		}

		const uint8* GetData() const
		{
			return GetArray();
		}

		private:

		int msoffset;
	};

} // namespace prcore

#endif
