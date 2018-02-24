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
	Timer Linux implementation
		
	revision history:
	???/??/2001 - Edmond Cote - initial revision
	Jan/13/2003 - Daniel Kolakowski - added ::GetTick() method
*/
#include <prcore/prcore.hpp>
#include <sys/time.h>
#include <unistd.h>

using namespace prcore;


// NOTE: this is a very basic timer

static struct timeval start;
static struct timeval now;


Timer::Timer()
{
	gettimeofday(&start,NULL);
}


Timer::~Timer()
{
}


void Timer::Reset()
{
}


float Timer::GetTime() const
{
	gettimeofday(&now,NULL);
	return (float)((now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000.0f)/1000.f;
}


uint32 Timer::GetTick() const
{
	gettimeofday(&now,NULL);
	return ((now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000);
}


TimeInfo GetTimeInfo()
{
	TimeInfo time;

	// TODO
	time.year         = 0;
	time.month        = 0;
	time.day          = 0;
	time.hour         = 0;
	time.minute       = 0;
	time.second       = 0;
	time.milliseconds = 0;

	return time;
}


