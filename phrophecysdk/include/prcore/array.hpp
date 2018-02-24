/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_ARRAY_HPP
#define PRCORE_ARRAY_HPP


#include <cassert>
#include <cstddef>


namespace prcore
{

	template <typename T>
	class Array
	{
		public:

		Array()
		: mdata(NULL),mmaxsize(0),moffset(0),readonly(false)
		{
		}

		Array(int size)
		: mmaxsize(size),moffset(size),readonly(false)
		{
			mdata = new T[size];
		}

		Array(const T* data, int size)
		: mdata(const_cast<T*>(data)),mmaxsize(size),moffset(size),readonly(true)
		{
			// comment: creates a read-only reference to the constant data!
		}

		Array(const Array& v)
		: mdata(NULL),mmaxsize(v.GetSize()),moffset(v.GetSize()),readonly(false)
		{
			if ( mmaxsize )
			{
				mdata = new T[mmaxsize];

				for ( int i=0; i<mmaxsize; ++i )
					mdata[i] = v.mdata[i];
			}
		}

		~Array()
		{
			if ( !readonly )
				delete[] mdata;
		}

		void operator = (const Array& v)
		{
			assert( readonly == false );

			int size = v.GetSize();
			SetSize(size,false);

			for ( int i=0; i<size; ++i )
				mdata[i] = v.mdata[i];
		}

		T& operator [] (int index)
		{
			assert( index >= 0 && index < mmaxsize );
			return mdata[index];
		}

		const T& operator [] (int index) const
		{
			assert( index >= 0 && index < mmaxsize );
			return mdata[index];
		}

		bool IsEmpty() const
		{
			return moffset == 0;
		}

		int GetSize() const
		{
			return moffset;
		}

		const T* GetArray() const
		{
			return mdata;
		}

		T* GetArray()
		{
			return mdata;
		}

		void SetSize(int size, bool preserve = false)
		{
			assert( readonly == false );

			if ( size == mmaxsize )
			{
				moffset = size;
				return;
			}

			T* array = NULL;

			if ( size )
			{
				array = new T[size];

				if ( mdata && preserve )
				{
					int size0 = GetSize();
					int count = (size0 > size) ? size : size0;
					for ( int i=0; i<count; ++i )
						array[i] = mdata[i];
				}
			}

			delete[] mdata;
			mdata    = array;
			mmaxsize = size;
			moffset  = size;
		}

		void Reserve(int size)
		{
			assert( readonly == false );

			SetSize(size,false);
			ResetIndex();
		}

		T* PushBlock(int count)
		{
			assert( readonly == false );

			int curoff = moffset;
			int endoff = moffset + count;

			if ( endoff >= mmaxsize )
			{
				if ( !mmaxsize )
				{
					int size = count * 2;
					mdata = new T[size];
					mmaxsize = size;
				}
				else
				{
					int size = mmaxsize * 2 + count;
					SetSize(size,true);
				}
			}

			moffset = endoff;
			return mdata + curoff;
		}

		T& PushBack(const T& item)
		{
			assert( readonly == false );

			if ( moffset >= mmaxsize )
			{
				if ( !mmaxsize )
				{
					mdata = new T[4];
					mmaxsize = 4;
					moffset = 0;
				}
				else
				{
					int offset = moffset;
					int size = mmaxsize * 2;
					SetSize(size,true);
					moffset = offset;
				}
			}

			T& ref = mdata[moffset++];
			ref = item;
			return ref;
		}

		T& PushBack()
		{
			assert( readonly == false );

			if ( moffset >= mmaxsize )
			{
				if ( !mmaxsize )
				{
					mdata = new T[4];
					mmaxsize = 4;
					moffset = 0;
				}
				else
				{
					int offset = moffset;
					int size = mmaxsize * 2;
					SetSize(size,true);
					moffset = offset;
				}
			}
			return mdata[moffset++];
		}

		T& PopBack()
		{
			assert( readonly == false );
			assert( GetSize() > 0 );
			return mdata[--moffset];
		}

		void Remove(const T& item)
		{
			assert( readonly == false );

			int count = GetSize();
			T* src = mdata;
			T* dest = mdata;

			for ( int i=0; i<count; ++i )
			{
				if ( *src != item )
				{
					if ( src != dest )
						*dest = *src;

					++dest;
				}
				++src;
			}
			moffset = static_cast<int>(dest - mdata);
		}

		void RemoveBlock(int index, int count)
		{
			assert( readonly == false );

			T* dest = mdata + index;
			T* src = mdata + index + count;
			int left = static_cast<int>((mdata + moffset) - src);

			for ( int i=0; i<left; ++i )
				*dest++ = *src++;

			moffset = static_cast<int>(dest - mdata);
		}

		void SetIndex(int index)
		{
			assert( index >= 0 && index < mmaxsize );
			moffset = index;
		}

		void ResetIndex()
		{
			moffset = 0;
		}

		void TrimArray()
		{
			assert( readonly == false );

			int size = GetSize();
			SetSize(size,true);
		}

		void Clear()
		{
			assert( readonly == false );

			SetSize(0,false);
		}

		private:

		T*		mdata;
		int		mmaxsize;
		int		moffset;
		bool	readonly;
	};

} // namespace prcore


#endif