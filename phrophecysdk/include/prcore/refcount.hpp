/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_REFCOUNT_HPP
#define PRCORE_REFCOUNT_HPP


#include "string.hpp"


namespace prcore
{

	class RefCount
	{
		public:
		
		virtual void SetName(const String& s)
		{
			name = s;
		}

		const String& GetName() const
		{
			return name;
		}

		void AddRef()
		{
			++refcount;
		}

		int GetRefCount() const
		{
			return refcount;
		}

		void Release()
		{
			if ( !--refcount )
				delete this;
		}

		protected:

		RefCount()
		: refcount(1)
		{
		}

		RefCount(const RefCount& v)
		: refcount(v.refcount),name(v.name)
		{
		}

		virtual	~RefCount()
		{
		}

		void operator = (const RefCount& v)
		{
			refcount = v.refcount;
			name = v.name;
		}

		int refcount;
		String name;
	};

} // namespace prcore


#endif