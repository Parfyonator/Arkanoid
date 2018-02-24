/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
/*
	source:
		Expression Template string class

	revision history:
		Jun/10/2000 - Thomas Mannfred Carlsson - initial revision
		Nov/16/2001 - Mats Byggmastar - reported a memory leak in "+ operator"(s)
		Dec/01/2001 - Jukka Liimatta - rewrote the string class
		Feb/06/2002 - Francesco Montecuccoli - added SubString() method
		Nov/12/2002 - Jukka Liimatta - rewrote the string to use prcore::Array<char> as container
		Dec/26/2002 - Jukka Liimatta - rewrote the + operator to support expressions (christmas fun..)
		Jan/12/2003 - Nietje - fixed SubString() method
		Jan/12/2003 - Jukka Liimatta - added MetaSubString() method, and rewrote SubString() to use it
		Jan/13/2003 - Jukka Liimatta - fixed operator == for case when both strings are empty
		Jan/14/2003 - Jukka Liimatta - case conversion templates, made prcore::StringCompare() a template
*/
#ifndef PRCORE_STRING_HPP
#define PRCORE_STRING_HPP


#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include "configure.hpp"
#include "chartype.hpp"
#include "array.hpp"


namespace prcore
{

	// string expression

	template <typename S, typename L, typename R>
	class StringExp
	{
		const L left;
		const R right;
		public:

		StringExp(const L& leftarg, const R& rightarg)
		: left(leftarg),right(rightarg)
		{
		}

		int size() const
		{
			return left.size() + right.size();
		}

		void append(S*& dest) const
		{
			left.append(dest);
			right.append(dest);
		}
	};


	// meta string (used as low-overhead string references)
	// string expression use this frequently

	template <typename S>
	class MetaString
	{
		const S* mdata;
		int msize;
		public:

		MetaString(const S* data)
		: mdata(data)
		{
			const S* s = data;
			for ( ; *s; ++s ) ;
			msize = static_cast<int>(s - data);
		}

		MetaString(const S* data, int size)
		: mdata(data),msize(size)
		{
		}

		int size() const
		{
			return msize;
		}

		void append(S*& dest) const
		{
			for ( int i=0; i<msize; ++i )
				dest[i] = mdata[i];

			dest += msize;
		}
	};


	// string container

	template <typename S>
	class String2
	{
		public:

		String2()
		{
			mdata.PushBack(0);
		}

		String2(int size)
		: mdata(size+1)
		{
			mdata[size] = 0;
		}

		String2(const String2& s)
		: mdata(s.mdata)
		{
		}

		String2(const S* text)
		{
			*this = text;
		}

		~String2()
		{
		}

		template <typename L, typename R>
		String2(const StringExp<S,L,R>& exp)
		: mdata(exp.size() + 1)
		{
			S* dest = mdata.GetArray();
			exp.append(dest);
			*dest = 0;
		}

		template <typename L, typename R>
		String2& operator = (const StringExp<S,L,R>& exp)
		{
			// hotfix: inefficient, but works :)
			String2 temp = exp;
			*this = temp;
			return *this;
		}

		String2& operator = (const String2& s)
		{
			mdata = s.mdata;
			return *this;
		}

		template <typename S2>
		String2& operator = (const S2* text)
		{
			assert( text != NULL );

			mdata.ResetIndex();
			for ( ; *text; ++text )
				mdata.PushBack(*text);

			mdata.PushBack(0);
			return *this;
		}

		String2& operator += (const String2& s)
		{
			int count = s.GetLength();
			S* dest = mdata.PushBlock(count) - 1;
			const S* text = s;

			for ( int i=0; i<=count; ++i )
				*dest++ = *text++;

			return *this;
		}

		template <typename S2>
		String2& operator += (const S2* text)
		{
			assert( text != NULL );

			mdata.PopBack();
			for ( ; *text; ++text )
				mdata.PushBack(*text);

			mdata.PushBack(0);
			return *this;
		}

		bool operator == (const String2& s) const
		{
			if ( GetLength() != s.GetLength() )
				return false;

			const S* s1 = *this;
			const S* s2 = s;
			int count = GetLength();

			while ( count-- )
			{
				if ( *s1++ != *s2++ )
					return false;
			}

			return true;
		}

		bool operator == (const S* text) const
		{
			assert( text != NULL );

			const S* s = *this;
			while ( *s == *text++ )
			{
				if ( *s++ == 0 )
					return true;
			}

			return false;
		}

		bool operator != (const String2& s) const
		{
			return !(*this == s);
		}

		bool operator != (const S* text) const
		{
			return !(*this == text);
		}

		S& operator [] (int index)
		{
			return mdata[index];
		}

		const S& operator [] (int index) const
		{
			return mdata[index];
		}

		operator S* ()
		{
			return mdata.GetArray();
		}

		operator const S* () const
		{
			return mdata.GetArray();
		}

		bool IsEmpty() const
		{
			return mdata.GetSize() <= 1;
		}

		int GetLength() const
		{
			return mdata.GetSize() - 1;
		}

		S* SetLength(int length)
		{
			mdata.SetSize(length + 1);
			mdata[length] = 0;
			return mdata.GetArray();
		}

		String2& operator << (const String2& s)
		{
			return operator += (s);
		}

		template <typename S2>
		String2& operator << (const S2* text)
		{
			assert( text != NULL );
			return operator += (text);
		}

		String2& operator << (const unsigned char v)
		{
			const int offset = mdata.GetSize() - 1;
			mdata[offset] = v;
			mdata.PushBack(0);
			return *this;
		}

		String2& operator << (const char v)
		{ 
			return operator << (static_cast<const unsigned char>(v));
		}

		String2& operator << (const signed char v)
		{ 
			return operator << (static_cast<const unsigned char>(v));
		}

		String2& operator << (const unsigned char* v)
		{ 
			return operator << (reinterpret_cast<const char*>(v));
		}

		String2& operator << (const signed char* v)
		{ 
			return operator << (reinterpret_cast<const char*>(v));
		}

		String2& operator << (const short v)
		{
			char text[8];
			sprintf(text,"%d",static_cast<int>(v));
			return operator += (text);
		}

		String2& operator << (const unsigned short v)
		{
			char text[8];
			sprintf(text,"%u",static_cast<unsigned int>(v));
			return operator += (text);
		}

		String2& operator << (const int v)
		{
			char text[16];
			sprintf(text,"%d",static_cast<int>(v));
			return operator += (text);
		}

		String2& operator << (const unsigned int v)
		{
			char text[16];
			sprintf(text,"%u",static_cast<unsigned int>(v));
			return operator += (text);
		}

		String2& operator << (const long v)
		{
			char text[16];
			sprintf(text,"%d",static_cast<int>(v));
			return operator += (text);
		}

		String2& operator << (const unsigned long v)
		{
			char text[16];
			sprintf(text,"%u",static_cast<unsigned int>(v));
			return operator += (text);
		}

		String2& operator << (const float v)
		{
			char text[16];
			sprintf(text,"%.3f",v);
			return operator += (text);
		}

		void PrintText(const char* text, ...)
		{
			assert( text != NULL );

			va_list args;
			va_start(args,text);

			// warning:
			// Do not use this method in security-critical environment for blind
			// processing of user input as the buffer size is fixed to 256 characters.
			char buffer[256];
			vsprintf(buffer,text,args);
			va_end(args);

			*this = buffer;
		}

		StringExp<S,MetaString<S>,MetaString<S> >
		MetaSubString(int offset, int length) const
		{
			int size = GetLength();
			if ( offset < 0 || offset >= size || length <= 0 )
				return StringExp<S,MetaString<S>,MetaString<S> >(MetaString<S>(NULL,0),MetaString<S>(NULL,0));

			int count = (offset + length > size ? size - offset : length);
			const S* text = *this + offset;

			// warning:
			// This method might be tempting one to use.
			return StringExp<S,MetaString<S>,MetaString<S> >(MetaString<S>(text,count),MetaString<S>(NULL,0));
		}

		String2 SubString(int offset, int length) const
		{
			return String2(MetaSubString(offset,length));
		}

		void ToLower()
		{
			int size = GetLength();
			for ( int i=0; i<size; ++i )
				mdata[i] = prcore::tolower(mdata[i]);
		}

		void ToUpper()
		{
			int size = GetLength();
			for ( int i=0; i<size; ++i )
				mdata[i] = prcore::toupper(mdata[i]);
		}

		private:

		Array<S> mdata;
	};


	// std::ostream operators

	template <typename S>
	inline std::ostream& operator << (std::ostream& out, String2<S>& s)
	{
		out << static_cast<S*>(s);
		return out;
	}

	template <typename S>
	inline std::ostream& operator << (std::ostream& out, const String2<S>& s)
	{
		out << static_cast<const S*>(s);
		return out;
	}

	template <typename S, typename L, typename R>
	inline std::ostream& operator << (std::ostream& out, const StringExp<S,L,R> exp)
	{
		String2<S> s = exp;
		out << static_cast<const S*>(s);
		return out;
	}


	// expression template + operator(s)

	// exp + exp
	template <typename S, typename L0, typename R0, typename L1, typename R1>
	inline const StringExp<S,StringExp<S,L0,R0>,StringExp<S,L1,R1> > operator + (const StringExp<S,L0,R0>& exp0, const StringExp<S,L1,R1>& exp1)
	{
		return StringExp<S,StringExp<S,L0,R0>,StringExp<S,L1,R1> >(exp0,exp1);
	}

	// exp + string
	template <typename S, typename L, typename R>
	inline const StringExp<S,StringExp<S,L,R>,MetaString<S> > operator + (const StringExp<S,L,R>& exp, const String2<S>& s)
	{
		return StringExp<S,StringExp<S,L,R>,MetaString<S> >(exp,MetaString<S>(s,s.GetLength()));
	}

	// exp + ansi
	template <typename S, typename L, typename R>
	inline const StringExp<S,StringExp<S,L,R>,MetaString<S> > operator + (const StringExp<S,L,R>& exp, const S* text)
	{
		return StringExp<S,StringExp<S,L,R>,MetaString<S> >(exp,MetaString<S>(text));
	}

	// string + exp
	template <typename S, typename L, typename R>
	inline const StringExp<S,MetaString<S>,StringExp<S,L,R> > operator + (const String2<S>& s, const StringExp<S,L,R>& exp)
	{
		return StringExp<S,MetaString<S>,StringExp<S,L,R> >(MetaString<S>(s,s.GetLength()),exp);
	}

	// string + string
	template <typename S>
	inline const StringExp<S,MetaString<S>,MetaString<S> > operator + (const String2<S>& s0, const String2<S>& s1)
	{
		return StringExp<S,MetaString<S>,MetaString<S> >(MetaString<S>(s0,s0.GetLength()),MetaString<S>(s1,s1.GetLength()));
	}

	// string + ansi
	template <typename S>
	inline const StringExp<S,MetaString<S>,MetaString<S> > operator + (const String2<S>& s, const S* text)
	{
		return StringExp<S,MetaString<S>,MetaString<S> >(MetaString<S>(s,s.GetLength()),MetaString<S>(text));
	}

	// ansi + exp
	template <typename S, typename L, typename R>
	inline const StringExp<S,MetaString<S>,StringExp<S,L,R> > operator + (const S* text, const StringExp<S,L,R>& exp)
	{
		return StringExp<S,MetaString<S>,StringExp<S,L,R> >(MetaString<S>(text),exp);
	}

	// ansi + string
	template <typename S>
	inline const StringExp<S,MetaString<S>,MetaString<S> > operator + (const S* text, const String2<S>& s)
	{
		return StringExp<S,MetaString<S>,MetaString<S> >(MetaString<S>(text),MetaString<S>(s,s.GetLength()));
	}


	// global case sensitive string compare

	template <typename S>
	inline bool StringCompare(const String2<S>& s0, const String2<S>& s1, bool case_sensitive = true)
	{
		// first test for equal length
		int size = s0.GetLength();
		if ( size != s1.GetLength() )
			return false;

		const S* v0 = s0;
		const S* v1 = s1;

		if ( case_sensitive )
		{
			for ( int i=0; i<size; ++i )
				if ( v0[i] != v1[i] )
					return false;
		}
		else
		{
			for ( int i=0; i<size; ++i )
				if ( prcore::toupper(v0[i]) != prcore::toupper(v1[i]) )
					return false;
		}
		
		return true;
	}


	// backward compatibility

	typedef prcore::String2<char>		String;
	typedef prcore::String2<char>		StringANSI;
	typedef prcore::String2<uchar16>	StringUNICODE;

} // namespace prcore


#endif