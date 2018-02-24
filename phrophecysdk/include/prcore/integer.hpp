/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_INTEGER_HPP
#define PRCORE_INTEGER_HPP


#include "configure.hpp"


namespace prcore
{

	// macros to create 16 bit codes

	#define PRCORE_CODE16(n0,n1)       ((uint32)(uint8)(n0) | ((uint32)(uint8)(n1) << 8))
	#define PRCORE_LITTLECODE16(n0,n1) ((uint32)(uint8)(n0) | ((uint32)(uint8)(n1) << 8))
	#define PRCORE_BIGCODE16(n1,n0)    ((uint32)(uint8)(n0) | ((uint32)(uint8)(n1) << 8))

	// macros to create 32 bit codes

	#define PRCORE_CODE32(n0,n1,n2,n3)       ((uint32)(uint8)(n0) | ((uint32)(uint8)(n1) << 8) | ((uint32)(uint8)(n2) << 16) | ((uint32)(uint8)(n3) << 24 ))
	#define PRCORE_LITTLECODE32(n0,n1,n2,n3) ((uint32)(uint8)(n0) | ((uint32)(uint8)(n1) << 8) | ((uint32)(uint8)(n2) << 16) | ((uint32)(uint8)(n3) << 24 ))
	#define PRCORE_BIGCODE32(n3,n2,n1,n0)    ((uint32)(uint8)(n0) | ((uint32)(uint8)(n1) << 8) | ((uint32)(uint8)(n2) << 16) | ((uint32)(uint8)(n3) << 24 ))

	// index of the most significant bit in the mask

	inline int HighestBit(uint32 mask)
	{
		int base;

		if ( mask & 0xffff0000 )
		{
			if ( mask & 0xff000000 )
			{
				base = 24;
				mask >>= 24;
			}
			else
			{
				base = 16;
				mask >>= 16;
			}
		}
		else
		{
			if ( mask & 0x0000ff00 )
			{
				base = 8;
				mask >>= 8;
			}
			else
			{
				base = 0;
			}
		}

		if ( mask & 0x0000ff00 )
		{
			base += 8;
			mask >>= 8;
		}

		if ( mask & 0x000000f0 )
		{
			base += 4;
			mask >>= 4;
		}

		if ( mask & 0x0000000c )
		{
			base += 2;
			mask >>= 2;
		}

		if ( mask & 0x00000002 )
		{
			++base;
			mask >>= 1;
		}

		return base - 1 + (mask & 1);
	}

	// index of the least significant bit in the mask

	inline int LowestBit(uint32 mask)
	{
		if ( !mask ) 
			return -1;

		int base;

		if ( !(mask & 0xffff) )
		{
			if ( !(mask & 0x00ff0000) )
			{
				base = 24;
				mask >>= 24;
			}
			else
			{
				base = 16;
				mask >>= 16;
			}
		}
		else
		{
			if ( !(mask & 0x00ff) )
			{
				base = 8;
				mask >>= 8;
			}
			else
			{
				base = 0;
			}
		}

		if ( !(mask & 0x000f) )
		{
			base += 4;
			mask >>= 4;
		}

		if ( !(mask & 0x0003) )
		{
			base += 2;
			mask >>= 2;
		}

		return base + 1 - (mask & 1);
	}

	// from Steve Baker's "Cute Code" collection
	// http://web2.airmail.net/sjbaker1/software/cute_code.html

	// number of set bits in the mask

	inline int BitsInMask(uint32 mask)
	{
		mask = (mask & 0x55555555) + ((mask & 0xaaaaaaaa) >> 1);
		mask = (mask & 0x33333333) + ((mask & 0xcccccccc) >> 2);
		mask = (mask & 0x0f0f0f0f) + ((mask & 0xf0f0f0f0) >> 4);
		mask = (mask & 0x00ff00ff) + ((mask & 0xff00ff00) >> 8);
		mask = (mask & 0x0000ffff) + (mask >> 16);
		return mask;
	}

	// reverse bit order

	inline void ReverseBits(uint32& mask)
	{
		mask = ((mask >>  1) & 0x55555555) | ((mask <<  1) & 0xaaaaaaaa);
		mask = ((mask >>  2) & 0x33333333) | ((mask <<  2) & 0xcccccccc);
		mask = ((mask >>  4) & 0x0f0f0f0f) | ((mask <<  4) & 0xf0f0f0f0);
		mask = ((mask >>  8) & 0x00ff00ff) | ((mask <<  8) & 0xff00ff00);
		mask = (mask >> 16) | (mask << 16);
	}

	// is value a power-of-two?

	inline bool IsPowerOfTwo(uint32 mask)
	{
		return (mask & (mask - 1)) == 0;
	}

	// return the next power-of-two of a 32bit number

	inline uint32 NextPowerOfTwo(uint32 v)
	{
		v -= 1;
		v |= v >> 16;
		v |= v >> 8;
		v |= v >> 4;
		v |= v >> 2;
		v |= v >> 1;
		return v + 1;
	}

} // namespace prcore


#endif