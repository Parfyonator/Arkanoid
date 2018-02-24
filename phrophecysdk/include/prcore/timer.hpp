/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_TIMER_HPP
#define PRCORE_TIMER_HPP


#include "configure.hpp"


namespace prcore
{

	class Timer
	{
		public:

		Timer();						// create high resolution timer
		~Timer();						// delete high resolution timer

		void		Reset();			// reset timer age to current time
		float		GetTime() const;	// timer age in seconds
		uint32		GetTick() const;	// timer age in milliseconds

		private:

		int64		mFreq;
		int64		mTime0;
	};


	struct TimeInfo
	{
		uint16		year;
		uint16		month;
		uint16		day;
		uint16		hour;
		uint16		minute;
		uint16		second;
		uint16		milliseconds;
	};

	TimeInfo GetTimeInfo();

} // namespace prcore


#endif