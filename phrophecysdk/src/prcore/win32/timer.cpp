/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


	source: 
		Timer ( win32 )

	revision history:
		Jan/27/1999 - Jukka Liimatta - initial revision
		Jan/01/2003 - Daniel Kolakowski - added ::GetTick() method
		Jun/20/2003 - Jukka Liimatta - upgraded implementation
*/
#include <prcore/timer.hpp>

using namespace prcore;


// =================================================
// Timer
// =================================================

Timer::Timer()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	mFreq = static_cast<int64>(freq.QuadPart);

	LARGE_INTEGER time0;
	QueryPerformanceCounter(&time0);
	mTime0 = static_cast<int64>(time0.QuadPart);
}


Timer::~Timer()
{
}


float Timer::GetTime() const
{
	LARGE_INTEGER time1;
	QueryPerformanceCounter(&time1);
	int64 delta = static_cast<int64>(time1.QuadPart) - mTime0;

	return static_cast<float>(static_cast<double>(delta) / static_cast<double>(mFreq));
}


uint32 Timer::GetTick() const
{
	LARGE_INTEGER time1;
	QueryPerformanceCounter(&time1);
	int64 delta = static_cast<int64>(time1.QuadPart) - mTime0;

	return static_cast<uint32>(delta / (mFreq / 1000));
}


void Timer::Reset()
{
	LARGE_INTEGER time0;
	QueryPerformanceCounter(&time0);
	mTime0 = static_cast<int64>(time0.QuadPart);
}


// =================================================
// GetTimeInfo()
// =================================================

TimeInfo prcore::GetTimeInfo()
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);

	TimeInfo time;

	time.year         = systime.wYear;
	time.month        = systime.wMonth;
	time.day          = systime.wDay;
	time.hour         = systime.wHour;
	time.minute       = systime.wMinute;
	time.second       = systime.wSecond;
	time.milliseconds = systime.wMilliseconds;

	return time;
}
