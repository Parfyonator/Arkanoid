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
		rect implementation

	revision history:
		Feb/04/2001 - Jukka Liimatta - initial revision/renaissance build
        Dec/26/2003 - Stefan Karlsson - added prcore{} around code shit ;)
*/
#include <algorithm>
#include <prcore/configure.hpp>
#include <prcore/rect.hpp>

namespace prcore
{

    Rect::Rect(int x0, int y0, int w, int h)
    : x(x0), y(y0), width(w), height(h)
    {
    }

    Rect::Rect(const Rect& rect)
    : x(rect.x), y(rect.y), width(rect.width), height(rect.height)
    {
    }

    float Rect::GetAspect() const
    {
        return (float)width / float(height);
    }

    bool Rect::Intersect(const Rect& a, const Rect& b)
    {
        int ax1 = a.x + a.width - 1;
        int ay1 = a.y + a.height - 1;
        int bx1 = b.x + b.width - 1;
        int by1 = b.y + b.height - 1;

        // trivial reject
        if ( ax1 < b.x ) return false; // a completely left of b
        if ( ay1 < b.y ) return false; // a completely above of b
        if ( a.x > bx1 ) return false; // a completely right of b
        if ( a.y > by1 ) return false; // a completely below of b 

        // intersecting area
        x = std::max(a.x,b.x);
        y = std::max(a.y,b.y);
        int x1 = std::min(ax1,bx1);
        int y1 = std::min(ay1,by1);

        if ( x > x1 ) return false;
        if ( y > y1 ) return false;

        width = x1 - x + 1;
        height = y1 - y + 1;

        return true;
    }

    bool Rect::PointInside(int px, int py)
    {
        if ( px < x || px >= (x+width) ) return false;
        if ( py < y || py >= (y+height) ) return false;
        return true;
    }

}
