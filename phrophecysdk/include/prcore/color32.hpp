/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_COLOR32_HPP
#define PRCORE_COLOR32_HPP


#include "configure.hpp"


namespace prcore
{

	struct color32
	{
		// members

		#ifdef PRCORE_LITTLE_ENDIAN
		uint8 b,g,r,a;
		#endif

		#ifdef PRCORE_BIG_ENDIAN
		uint8 a,r,g,b;
		#endif

		// constructors

		color32()
		{
		}

		color32(uint32 color)
		{
			*this = color;
		}

		color32(const color32& color)
		{
			*this = color;
		}

		color32(uint8 red, uint8 green, uint8 blue, uint8 alpha = 0xff)
		#ifdef PRCORE_LITTLE_ENDIAN
		: b(blue),g(green),r(red),a(alpha)
		#endif
		#ifdef PRCORE_BIG_ENDIAN
		: a(alpha),r(red),g(green),b(blue)
		#endif
		{
		}

		// operators

		color32 operator + () const
		{
			return *this;
		}

		color32 operator - () const
		{
			const uint32& u = *this;
			return color32(u ^ 0xffffffff);
		}

		color32 operator + (const color32& color) const
		{
			// rgb components are saturated, a wraps around
			const uint32& u1 = *this;
			const uint32& u2 = color;
			uint32 x = (u1 & 0xfefefefe) + (u2 & 0xfefefefe);
			return color32(x | ((x & 0x01010101) - ((x & 0x01010101) >> 8)));
		}

		color32 operator * (const color32& color) const
		{
			return color32(
				static_cast<uint8>((r * color.r) >> 8),
				static_cast<uint8>((g * color.g) >> 8),
				static_cast<uint8>((b * color.b) >> 8),
				static_cast<uint8>((a * color.a) >> 8) );
		}

		color32 operator * (const uint8 scale) const
		{
			const uint32& u = *this;
			uint32 lsb = (((u & 0x00ff00ff) * scale) >> 8) & 0x00ff00ff;
			uint32 msb = (((u & 0xff00ff00) >> 8) * scale) & 0xff00ff00;
			return color32(lsb|msb);
		}

		color32& operator = (const uint32 color)
		{
			reinterpret_cast<uint32*>(this)[0] = color;
			return *this;
		}

		color32& operator = (const color32& color)
		{
			reinterpret_cast<uint32*>(this)[0] =
				reinterpret_cast<const uint32&>(color);
			return *this;
		}

		bool operator == (const color32& color) const
		{
			return reinterpret_cast<const uint32&>(*this) ==
				reinterpret_cast<const uint32&>(color);
		}

		bool operator != (const color32& color) const
		{
			return reinterpret_cast<const uint32&>(*this) !=
				reinterpret_cast<const uint32&>(color);
		}

		operator uint32 () const
		{
			return reinterpret_cast<const uint32&>(*this);
		}
	};

	// inline function(s)

	inline color32 Lerp(const color32& c0, const color32& c1, uint32 scale)
	{
		const uint32& u0 = c0;
		const uint32& u1 = c1;
		uint32 vx = u0 & 0x00ff00ff;
		uint32 rb = vx + ((((u1 & 0x00ff00ff) - vx) * scale) >> 8) & 0x00ff00ff;
		vx = u0 & 0xff00ff00;
		return color32(rb | (vx + ((((u1 >> 8) & 0x00ff00ff) - (vx >> 8)) * scale) & 0xff00ff00));
	}

	// backward compatibility

	typedef prcore::color32 Color32;

} // namespace prcore


#endif