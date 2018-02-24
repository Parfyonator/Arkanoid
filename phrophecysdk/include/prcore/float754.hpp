/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_FLOAT754_HPP
#define PRCORE_FLOAT754_HPP


#include <cmath>
#include "configure.hpp"
#include "endian.hpp"


namespace prcore
{

	// type conversions

	// NOTE!
	// do not use as replacement for static_cast<int>, these are not fully compatible
	// with the C++ rounding rules (and values must be "reasonably" close to zero)
	// however, in a environment and use such as software rasterizers these can be very usefull.

	// - you been warned! ;-)

	#ifdef PRCORE_LITTLE_ENDIAN
	#define PRCORE_FLTBIT(v) ((int32*)&v)[0]
	#else
	#define PRCORE_FLTBIT(v) ((int32*)&v)[1]
	#endif

	inline int32 DoubleToInt(double v)
	{
		v += (68719476736.0 * 1.5);
		return PRCORE_FLTBIT(v) >> 16;
	}

	inline int32 DoubleToFixed(double v)
	{
		v += (68719476736.0 * 1.5);
		return PRCORE_FLTBIT(v);
	}
    
	inline int32 FloatToInt(float v)
	{
		double magic = 68719476736.0 * 1.5 + v;
		return PRCORE_FLTBIT(magic) >> 16;
	}
    
    // This has a nameconflict or something! Shouldnt need the "2" here...
    inline int32 FloatToFixed2(float v)
    {
        double magic = 68719476736.0 * 1.5 + v;
        return PRCORE_FLTBIT(magic);
	}
    
	#undef PRCORE_FLTBIT

	// type conversions complying to the ieee754 rounding rules
	// from: Ville "wili" Miettinen @ www.flipcode.com code-of-day column

	// NOTE!
	// it has been brought to our attention that wili's functions have more up-to-date versions,
	// with major bugfixes - so use with care until this warning has been removed!

	inline int FloatToIntChop(const float& v)
	{
		const int32& u = *(const int32*)&v;
		int32 s = u >> 31;
		int32 e = ((u & 0x7fffffff) >> 23) - 127;
		return (((((uint32)((u & ((1<<23) - 1)) | (1<<23)) << 8) >> (31-e)) ^ (s)) - s ) &~ (e>>31);
	}

	inline int FloatToIntFloor(const float& v)
	{
		const int32& u = *(const int32*)&v;
		int32 sign = u >> 31;
		int32 exponent = ((u & 0x7fffffff)>>23) - 127;
		int32 expsign = ~(exponent >> 31);
		int32 mantissa = (u & ((1<<23) - 1));
		return (((((uint32)(mantissa | (1<<23)) << 8) >> (31-exponent)) & expsign) ^ (sign)) + ((!((mantissa<<8) & (((1<<(31-(exponent & expsign)))) - 1))) & sign);
	}

	inline int FloatToIntCeil(const float& v)
	{
		int32 u = (*(const int32*)&v) ^ 0x80000000;
		int32 sign = u >> 31;
		int32 exponent = ((u & 0x7fffffff)>>23) - 127;
		int32 expsign = ~(exponent>>31);
		int32 mantissa = (u & ((1<<23) - 1));
		return -(int)((((((uint32)(mantissa | (1<<23)) << 8) >> (31-exponent)) & expsign) ^ (sign)) + ((!((mantissa<<8) & (((1<<(31-(exponent & expsign)))) - 1))) & sign & expsign));
	}

	inline float lerpf(float a, float b, float time)
	{
		return a + (b - a) * time;
	}

	inline float snapf(float v, float grid)
	{
		return grid ? static_cast<float>(floor((v + grid * 0.5f) / grid) * grid) : v;
	}

	// reciprocal square root
	
	inline float rsqrtf(float v)
	{
		// store original value of v for the first iteration step
		float h = v * 0.5f;

		// initial guess: v^-0.5
		int32& i = reinterpret_cast<int32&>(v);
		//i = 0x5f3759df - (i >> 1);
		i = 0x5f375a86 - (i >> 1);

		// newton-rhapsod iteration
		v *= 1.5f - (h * v * v);
		v *= 1.5f - (h * v * v);
		return v;
	}

	// power function, evaluates: v^p

	inline float powf(float v, float p)
	{
		// approximation: scale the exponent by the power
		int32 i = static_cast<int32>(p * (reinterpret_cast<int32&>(v) - 0x3f800000)) + 0x3f800000;
		return reinterpret_cast<float&>(i);
	}

	inline float sqrtf(float v)
	{
		// evaluates to powf(v,0.5)
		// specialized to constant power: 0.5
		int32 i = (reinterpret_cast<int32&>(v) >> 1) + (0x3f800000 >> 1);
		float y = reinterpret_cast<float&>(i);

		// iterate
		y = (y * y + v) / (2 * y);
		y = (y * y + v) / (2 * y);
		return y;
	}

	// two's logarithm functions

	inline int log2f(float v)
	{
		// logarithm is encoded in the exponent bits as biased integer!
		return static_cast<int>(((reinterpret_cast<uint32&>(v) >> 23) & 0xff) - 127);
	}

	inline int log2i(uint16 v)
	{
		// logarithm is encoded in the exponent bits as biased integer!
		float fv = static_cast<float>(v);
		return (reinterpret_cast<int&>(fv) >> 23) - 127;
	}

	inline int sqrtlog2f(float v)
	{
		// sqrt(v) equals to v^0.5, this is dony by multiplying the exponent by 0.5 (bitshift left ;-)
		// the next step is pretty much a carbon copy of log2f() ..
		return ((((reinterpret_cast<int32&>(v) - 0x3f800000) >> 1) + 0x3f800000) >> 23) - 127;
	}

	// normalized random value: [0.0,1.0]

	inline float randf(uint32 seed = 0, bool reseed = false)
	{
		static uint32 prev = 0x12345678;

		prev = reseed ?	seed : prev;
		prev = prev * 1664525 + 1013904223;

		// exponent = 0 ( 0x3f800000 )
		// mantissa = full precision (1+23 bits)
		// value    = 2^exponent * 1.mantissa
		// range    = [1.0,2.0]
		uint32 v = 0x3f800000 | (prev & 0x007fffff);

		// reinterpret the bits as floating-point value,
		// normalize the value to range [0.0,1.0] with floating-point subtraction
		return reinterpret_cast<float&>(v) - 1.0f;
	}

	// trigonometry
	
	template <typename T>
	inline float sinf(const T& v)
	{
		return static_cast<float>(sin(v));
	}

	template <typename T>
	inline float cosf(const T& v)
	{
		return static_cast<float>(cos(v));
	}

} // namespace prcore


#endif
