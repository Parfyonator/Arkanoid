/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_PLUGIN_HPP
#define PRCORE_PLUGIN_HPP


#include "refcount.hpp"


namespace prcore
{

	class Plugin : public RefCount
	{
		public:

		Plugin(const char* filename, bool rawmode = true);
		~Plugin();

		void GetAddress(void** address, const char* symbolname);

		private:

		void* handle;
	};

} // namespace prcore


#endif