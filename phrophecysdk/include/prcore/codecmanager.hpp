/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_CODECMANAGER_HPP
#define PRCORE_CODECMANAGER_HPP


#include <cstddef>
#include "array.hpp"
#include "string.hpp"


namespace prcore
{

	struct CodecInterface
	{
		int				count;
		const String*	extension;
	};


	template <typename T>
	class CodecManager
	{
		public:

		int GetInterfaceCount() const
		{
			return mCodecArray.GetSize();
		}

		const T* GetInterface(int index) const
		{
			return &mCodecArray[index];
		}

		const T* FindInterface(const String& ext) const
		{
			// iterate through all codecs
			for ( int i=0; i<mCodecArray.GetSize(); ++i )
			{
				const T& codec = mCodecArray[i];

				// search all extensions in currect codec
				for ( int j=0; j<codec.count; ++j )
					if ( StringCompare(codec.extension[j],ext,false) )
						return &codec;
			}

			return NULL;
		}

		void RegisterInterface(const T& codec)
		{
			mCodecArray.PushBack(codec);
		}

		private:

		Array<T> mCodecArray;
	};

} // namespace prcore


#endif