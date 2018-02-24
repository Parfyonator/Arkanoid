///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

// Primary authors:
//     Florian Kainz <kainz@ilm.com>
//     Rod Bogart <rgb@ilm.com>


//---------------------------------------------------------------------------
//
//	class half --
//	implementation of non-inline members
//
//---------------------------------------------------------------------------

#include <cassert>
#include "half.h"


namespace
{

	half::uif      table0[1 << 16];
	unsigned short table1[1 << 9];

	//---------------------------------------------------
	// Interpret an unsigned short bit pattern as a half,
	// and convert that half to the corresponding float's
	// bit pattern.
	//---------------------------------------------------

	unsigned int
	halfToFloat (unsigned short y)
	{

		int s = (y >> 15) & 0x00000001;
		int e = (y >> 10) & 0x0000001f;
		int m =  y        & 0x000003ff;

		if (e == 0)
		{
		if (m == 0)
		{
			//
			// Plus or minus zero
			//

			return s << 31;
		}
		else
		{
			//
			// Denormalized number -- renormalize it
			//

			while (!(m & 0x00000400))
			{
			m <<= 1;
			e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
		}
		else if (e == 31)
		{
		if (m == 0)
		{
			//
			// Positive or negative infinity
			//

			return (s << 31) | 0x7f800000;
		}
		else
		{
			//
			// Nan -- preserve sign and significand bits
			//

			return (s << 31) | 0x7f800000 | (m << 13);
		}
		}

		//
		// Normalized number
		//

		e = e + (127 - 15);
		m = m << 13;

		//
		// Assemble s, e and m.
		//

		return (s << 31) | (e << 23) | m;
	}

	//-----------------------------------------------------
	// Compute a lookup table for float-to-half conversion.
	//
	// When indexed with the combined sign and exponent of
	// a float, the table either returns the combined sign
	// and exponent of the corresponding half, or zero if
	// the corresponding half may not be normalized (zero,
	// denormalized, overflow).
	//-----------------------------------------------------

	void
	initELut (unsigned short eLut[])
	{
		for (int i = 0; i < 0x100; i++)
		{
		int e = (i & 0x0ff) - (127 - 15);

		if (e <= 0 || e >= 30)
		{
			//
			// Special case
			//

			eLut[i]         = 0;
			eLut[i | 0x100] = 0;
		}
		else
		{
			//
			// Common case - normalized half, no exponent overflow possible
			//

			eLut[i]         =  (e << 10);
			eLut[i | 0x100] = ((e << 10) | 0x8000);
		}
		}
	}

	//---------------------------------------------------
	// Initialize static lookup-tables, initialized by selftst()
	//---------------------------------------------------

	void initLookup()
	{
		const int iMax = (1 << 16);
		for (int i = 0; i < iMax; i++)
		{
			table0[i].i = halfToFloat(i);
		}

		initELut(table1);
	}
}


//-------------------------------------------------------------
// Lookup tables for half-to-float and float-to-half conversion
//-------------------------------------------------------------

const half::uif* half::_toFloat   = table0;
const unsigned short* half::_eLut = table1;


//--------------------------------------
// Dummy flag, initialized by selftest()
//--------------------------------------

const bool half::_itWorks =
      selftest(); 


//-----------------------------------------------
// Overflow handler for float-to-half conversion;
// generates a hardware floating-point overflow,
// which may be trapped by the operating system.
//-----------------------------------------------

float
half::overflow ()
{
    volatile float f = 1e10;

    for (int i = 0; i < 10; i++)	
	f *= f;				// this will overflow before
					// the for­loop terminates
    return f;
}


//-----------------------------------------------------
// Float-to-half conversion -- general case, including
// zeroes, denormalized numbers and exponent overflows.
//-----------------------------------------------------

short
half::convert (int i)
{
    //
    // Our floating point number, f, is represented by the bit
    // pattern in integer i.  Disassemble that bit pattern into
    // the sign, s, the exponent, e, and the significand, m.
    // Shift s into the position where it will go in in the
    // resulting half number.
    // Adjust e, accounting for the different exponent bias
    // of float and half (127 versus 15).
    //

    register int s =  (i >> 16) & 0x00008000;
    register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
    register int m =   i        & 0x007fffff;

    //
    // Now reassemble s, e and m into a half:
    //

    if (e <= 0)
    {
	if (e < -10)
	{
	    //
	    // E is less than -10.  The absolute value of f is
	    // less than HALF_MIN (f may be a small normalized
	    // float, a denormalized float or a zero).
	    //
	    // We convert f to a half zero.
	    //

	    return 0;
	}

	//
	// E is between -10 and 0.  F is a normalized float,
	// whose magnitude is less than HALF_NRM_MIN.
	//
	// We convert f to a denormalized half.
	// 

	m = (m | 0x00800000) >> (1 - e);

	//
	// Round to nearest, round "0.5" up.
	//
	// Rounding may cause the significand to overflow and make
	// our number normalized.  Because of the way a half's bits
	// are laid out, we don't have to treat this case separately;
	// the code below will handle it correctly.
	// 

	if (m &  0x00001000)
	    m += 0x00002000;

	//
	// Assemble the half from s, e (zero) and m.
	//

	return s | (m >> 13);
    }
    else if (e == 0xff - (127 - 15))
    {
	if (m == 0)
	{
	    //
	    // F is an infinity; convert f to a half
	    // infinity with the same sign as f.
	    //

	    return s | 0x7c00;
	}
	else
	{
	    //
	    // F is a NAN; produce a half NAN that preserves
	    // the sign bit and the 10 leftmost bits of the
	    // significand of f.
	    //

	    return s | 0x7c00 | (m >> 13);
	}
    }
    else
    {
	//
	// E is greater than zero.  F is a normalized float.
	// We try to convert f to a normalized half.
	//

	//
	// Round to nearest, round "0.5" up
	//

	if (m &  0x00001000)
	{
	    m += 0x00002000;

	    if (m & 0x00800000)
	    {
		m =  0;		// overflow in significand,
		e += 1;		// adjust exponent
	    }
	}

	//
	// Handle exponent overflow
	//

	if (e > 30)
	{
	    overflow ();	// Cause a hardware floating point overflow;
	    return s | 0x7c00;	// if this returns, the half becomes an
	}   			// infinity with the same sign as f.

	//
	// Assemble the half from s, e and m.
	//

	return s | (e << 10) | (m >> 13);
    }
}


//--------------------------------------
// Simple selftest, triggered by static
// initialization of half::_itWorks flag
//--------------------------------------

namespace
{
    void
    testNormalized (float f)
    {
	half  h (f);
	float e (1 - h / f);

	if (e < 0)
	    e = -e;

	if (e > HALF_EPSILON * 0.5)
	{
//	    cerr << "Internal error: float/half conversion does not work.";
	    assert (false);
	}
    }


    void
    testDenormalized (float f)
    {
	half  h (f);
	float e (h - f);

	if (e < 0)
	    e = -e;

	if (e > HALF_MIN * 0.5)
	{
//	    cerr << "Internal error: float/half conversion does not work.";
	    assert (false);
	}
    }
}


bool
half::selftest ()
{
	initLookup();
    testNormalized   ( HALF_MAX);
    testNormalized   (-HALF_MAX);
    testNormalized   ( 0.1f);
    testNormalized   (-0.1f);
    testNormalized   ( 0.5f);
    testNormalized   (-0.5f);
    testNormalized   ( 1.0f);
    testNormalized   (-1.0f);
    testNormalized   ( 2.0f);
    testNormalized   (-2.0f);
    testNormalized   ( 3.0f);
    testNormalized   (-3.0f);
    testNormalized   ( 17.0f);
    testNormalized   (-17.0f);
    testNormalized   ( HALF_NRM_MIN);
    testNormalized   (-HALF_NRM_MIN);
    testDenormalized ( HALF_MIN);
    testDenormalized (-HALF_MIN);
    testDenormalized ( 0.0f);
    testDenormalized (-0.0f);

    return true;
}
