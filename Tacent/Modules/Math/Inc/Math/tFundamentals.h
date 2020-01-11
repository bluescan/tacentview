// tFundamentals.h
//
// Core math functions needed by the rest or of the module as well as for external use. Functions include trigonometric
// functions, intervals, angle manipulation, power functions, and other analytic functions.
//
// Copyright (c) 2004, 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <math.h>
#include <functional>
#include "Foundation/tPlatform.h"
#include "Math/tConstants.h"
namespace tMath
{


enum class tAngleMode
{
	Radians,	// Circle divided into 2Pi angle units.
	Degrees,	// Circle divided into 360 angle units.
	Norm256,	// Circle divided into 256 angle units.
	NormOne,	// Circle divided into one angle unit.
	Default		= Radians,
};


// Interval Notation. When you see [a,b] or (a,b) the square bracket means include the endpoint and the rounded brackets
// mean exclude. See http://www.coolmath.com/algebra/07-solving-inequalities/03-interval-notation-01. When a function
// takes a bias argument, a Low bias will cause the return value to include the lower extent of the interval and exclude
// the high extent. A High bias will exclude the low end and include the high end. As a notational convenience When a
// function takes a bias argument, we'll write the interval as [(a,b)] and the table below shows how to interpret it:
//
//		Bias			Interval
//		Full/Outer		[a,b]
//		Low/Left		[a,b)
//		High/Right		(a,b]
//		Center/Inner	(a,b)
enum class tIntervalBias
{
	Full,				Outer = Full,
	Low,				Left = Low,
	High,				Right = High,
	Center,				Inner = Center
};

// These are handy functions that return appropriate comparison operators when computing biased intervals.
std::function<bool(float,float)> tBiasLess(tIntervalBias);
std::function<bool(float,float)> tBiasGreater(tIntervalBias);

template<typename T> inline T tMin(const T& a, const T& b)																{ return a < b ? a : b; }
template<typename T> inline T tMax(const T& a, const T& b)																{ return a > b ? a : b; }
template<typename T> inline T tMin(const T& a, const T& b, const T& c)													{ T ab = a < b ? a : b; return ab < c ? ab : c; }
template<typename T> inline T tMax(const T& a, const T& b, const T& c)													{ T ab = a > b ? a : b; return ab > c ? ab : c; }
template<typename T> inline T tMin(const T& a, const T& b, const T& c, const T& d)										{ T ab = a < b ? a : b; T cd = c < d ? c : d; return ab < cd ? ab : cd; }
template<typename T> inline T tMax(const T& a, const T& b, const T& c, const T& d)										{ T ab = a > b ? a : b; T cd = c > d ? c : d; return ab > cd ? ab : cd; }
template<typename T> inline T tClamp(T val, T min, T max)																{ return (val < min) ? min : ((val > max) ? max : val); }
template<typename T> inline T tClampMin(T val, T min)																	{ return (val < min) ? min : val; }
template<typename T> inline T tClampMax(T val, T max)																	{ return (val > max) ? max : val; }
template<typename T> inline T tSaturate(T val)																			{ return (val < T(0)) ? T(0) : ((val > T(1)) ? T(1) : val); }
template<typename T> inline bool tInRange(const T val, const T min, const T max)		/* Returns val E [min, max]	*/	{ return tInIntervalII(val, min, max); }
template<typename T> inline bool tInInterval(const T val, const T min, const T max)		/* Returns val E [min, max]	*/	{ return tInIntervalII(val, min, max); }
template<typename T> inline bool tInIntervalII(const T val, const T min, const T max)									{ if ((val >= min) && (val <= max)) return true; return false; }	// Implements val E [min, max]
template<typename T> inline bool tInIntervalIE(const T val, const T min, const T max)									{ if ((val >= min) && (val < max)) return true; return false; }		// Implements val E [min, max)
template<typename T> inline bool tInIntervalEI(const T val, const T min, const T max)									{ if ((val > min) && (val <= max)) return true; return false; }		// Implements val E (min, max]
template<typename T> inline bool tInIntervalEE(const T val, const T min, const T max)									{ if ((val > min) && (val < max)) return true; return false; }		// Implements val E (min, max)
template<typename T> inline T tSign(T val)																				{ return val < T(0) ? T(-1) : val > T(0) ? T(1) : T(0); }
template<typename T> inline T tBinarySign(T val)																		{ return val < T(0) ? T(-1) : T(1); }	// Same as Sign but does not return 0 ever.  Two return values only.
template<typename T> inline bool tIsZero(T a)																			{ return (a == 0); }
template<typename T> inline bool tApproxEqual(T a, T b, float e = Epsilon)												{ return (tAbs(a-b) < e); }
template<typename T> inline bool tEquals(T a, T b)																		{ return a == b; }
template<typename T> inline bool tNotEqual(T a, T b)																	{ return a != b; }

// For the 'ti' versions of the below functions, the 'i' means 'in-place' (ref var) rather than returning the value.
template<typename T> inline void tiClamp(T& val, T min, T max)															{ val = (val < min) ? min : ((val > max) ? max : val); }
template<typename T> inline void tiClampMin(T& val, T min)																{ val = (val < min) ? min : val; }
template<typename T> inline void tiClampMax(T& val, T max)																{ val = (val > max) ? max : val; }
template<typename T> inline void tiSaturate(T& val)																		{ val = (val < T(0)) ? T(0) : ((val > T(1)) ? T(1) : val); }

// Use this instead of casting to int. The only difference is it rounds instead of truncating and is way faster -- The
// FPU stays in rounding mode so pipeline not flushed.
inline int tFloatToInt(float val)																						{ return int(val + 0.5f); }

// tCeiling and tFloor both need to change the FPU from round mode to truncate. Could have performance hit.
inline float tCeiling(float v)																							{ return ceilf(v); }
inline float tFloor(float v)																							{ return floorf(v); }
inline float tRound(float v)																							{ return floorf(v + 0.5f); }
inline float tMod(float n, float d)																						{ return fmodf(n,d); }
inline int tAbs(int val)																								{ return (val < 0) ? -val : val; }
inline float tAbs(float val)																							{ return (val < 0.0f) ? -val : val; }
inline double tAbs(double val)																							{ return (val < 0.0) ? -val : val; }

// For the 'ti' versions of the below functions, the 'i' means 'in-place' (ref var) rather than returning the value.
inline void tiCeiling(float& v)																							{ v = ceilf(v); }
inline void tiFloor(float& v)																							{ v = floorf(v); }
inline void tiRound(float& v)																							{ v = floorf(v + 0.5f); }
inline void tiAbs(int& val)																								{ val = (val < 0) ? -val : val; }
inline void tiAbs(float& val)																							{ val = (val < 0.0f) ? -val : val; }
inline void tiAbs(double& val)																							{ val = (val < 0.0) ? -val : val; }

// The following Abs function deserves a little explanation. Some linear algebra texts use the term absolute value and
// norm interchangeably. Others suggest that the absolute value of a matrix is the matrix with each component
// being the absolute value of the original. This is what the following template function returns -- not the L2 norm
// (scalar length).
template <typename T> inline T tAbs(T v)																				{ T result; for (int c = 0; c < T::GetNumComponents(); c++) result[c] = tAbs(v[c]); return result; }
inline float tFrac(float val)																							{ return tAbs(val - float(int(val))); }
inline float tSqrt(float x)																								{ return sqrtf(x); }
float tSqrtFast(float x);
inline float tRecipSqrt(float x)																						{ return 1.0f/sqrtf(x); }
float tRecipSqrtFast(float x);

inline float tDegToRad(float deg)																						{ return deg * Pi / 180.0f; }
inline float tRadToDeg(float rad)																						{ return rad * 180.0f / Pi; }
inline float tSin(float x)																								{ return sinf(x); }
inline float tSinFast(float x);								// For x E [0, Pi/2].
inline float tCos(float x)																								{ return cosf(x); }
inline float tCosFast(float x)								/* For x E [0, Pi/2]. */									{ float s = tSinFast(x); return tSqrtFast(1.0f - s*s); }
inline void tCosSin(float& cos, float& sin, float x);
inline void tCosSinFast(float& cos, float& sin, float x);	// For x E [0, Pi/2].
inline float tTan(float x)																								{ return tanf(x); }
inline float tArcSin(float x)																							{ return asinf(x); }
inline float tArcCos(float x)																							{ return acosf(x); }
inline float tArcTan(float y, float x)						/* Order is y, x. Returns angle of a slope (rise/run). */	{ return atan2f(y, x); }
inline float tArcTan(float m)																							{ return atanf(m); }
inline float tExp(float x)																								{ return expf(x); }
inline float tLog(float x)									/* Natural logarithm. */									{ return logf(x); }

// For the 'ti' versions of the below functions, the 'i' means 'in-place' (ref var) rather than returning the value.
inline void tiDegToRad(float& ang)																						{ ang = ang * Pi / 180.0f; }
inline void tiRadToDeg(float& ang)																						{ ang = ang * 180.0f / Pi; }

// Returns integral base 2 logarithm. If v is <= 0 returns MinInt32. If v is a power of 2 you will get an exact
// result. If v is not a power of two it will return the logarithm of the next lowest power of 2. For example,
// Log2(2) = 1, Log2(3) = 1, and Log2(4) = 2.
inline int tLog2(int v);
inline float tPow(float a, float b)																						{ return powf(a, b); }
inline double tPow(double a, double b)																					{ return pow(a, b); }

// For the 'ti' versions of the functions, the 'i' means 'in-place' (ref var) rather than returning the value.
inline bool tIsPower2(int v)																							{ if (v < 1) return false; return (v & (v-1)) ? false : true; }
inline void tiNextLowerPower2(uint& v)																					{ uint pow2 = 1; while (pow2 < v) pow2 <<= 1; pow2 >>= 1; v = pow2 ? pow2 : 1; }
inline uint tNextLowerPower2(uint v)																					{ uint pow2 = 1; while (pow2 < v) pow2 <<= 1; pow2 >>= 1; return pow2 ? pow2 : 1; }
inline void tiNextHigherPower2(uint& v)																					{ uint pow2 = 1; while (pow2 <= v) pow2 <<= 1; v = pow2; }
inline uint tNextHigherPower2(uint v)																					{ uint pow2 = 1; while (pow2 <= v) pow2 <<= 1; return pow2; }
inline void tiClosestPower2(uint& v)																					{ if (tIsPower2(v)) return; int h = tNextHigherPower2(v); int l = tNextLowerPower2(v); v = ((h - v) < (v - l)) ? h : l; }
inline uint tClosestPower2(uint v)																						{ if (tIsPower2(v)) return v; int h = tNextHigherPower2(v); int l = tNextLowerPower2(v); return ((h - v) < (v - l)) ? h : l; }
void tiNormalizeAngle(float& angle, tIntervalBias = tIntervalBias::Low);		// Results in angle E [(-Pi,Pi)].
inline float tNormalizedAngle(float angle, tIntervalBias bias = tIntervalBias::Low)										{ tiNormalizeAngle(angle, bias); return angle; }
void tiNormalizeAngle2Pi(float& angle, tIntervalBias = tIntervalBias::Low);		// Results in angle E [(0,2Pi)].
inline float tNormalizedAngle2Pi(float angle, tIntervalBias bias = tIntervalBias::Low)									{ tiNormalizeAngle2Pi(angle, bias); return angle; }

// Gets the range (y) value of a normal distribution with mean = 0, and given variance. Pass in the domain (x) value.
inline float tNormalDist(float variance, float x)																		{ return tPow(2*Pi*variance, -0.5f) * tExp(-tPow(x, 2.0f) / (2.0f*variance)); }

// These functions compute f(x) with x E [0, 1]. When not flipped f(0) = 0 and f(1) = 1. Furthermore, 0 <= f(x) <= 1.
// Curve shape may be controlled by one or more constant (c) arguments. These functions always exist in the unit square,
// even when flipped. Essentially flipping in x flips around the x=1/2 line, and y about the y=1/2 line.
enum tUnitFlip
{
	tUnitFlip_None																										= 0x00000000,
	tUnitFlip_X																											= 0x00000001,
	tUnitFlip_Y																											= 0x00000002,
	tUnitFlip_XY																										= tUnitFlip_X | tUnitFlip_Y
};

// Plot: http://www.wolframalpha.com/input/?i=Plot%5B%28Sin%28x*pi-pi%2F2%29%2B1%29%2F2%2C+%7Bx%2C0%2C1%7D%5D
float tUnitSin(float x, uint32 = tUnitFlip_None);

// Plot: http://www.wolframalpha.com/input/?i=Plot%5BSin%28x*pi%2F2%29%2C+%7Bx%2C0%2C1%7D%5D
float tUnitSinHalf(float x, uint32 = tUnitFlip_None);

// Plot: http://www.wolframalpha.com/input/?i=Plot%5Bpow%28x%2C+c%29%2C+%7Bx%2C0%2C1%7D%2C+%7Bc%2C0.1%2C3%7D%5D
// c E (0, inf). c < 1 pulls towards top left. c > 1 pulls towards bottom right.
float tUnitPow(float x, float c = 1.0f, uint32 = tUnitFlip_None);

// Plot: http://www.wolframalpha.com/input/?i=Plot+Piecewise%5B%7B%7Bpow%282x%2C+3%29%2F2%2C+x%3C0.5%7D%2C+%7B1+-+%28pow%281-2%28x-1%2F2%29%2C+3%29%29%2F2%2C+x%3E0.5%7D%7D%5D%2C+%7Bx%2C0%2C1%7D
float tUnitPowPlateau(float x, float c = 1.0f, uint32 = tUnitFlip_None);

// Plot: http://www.wolframalpha.com/input/?i=Plot+sqrt%281+-+%281-x%29%5E2%29%2C+%7Bx%2C0%2C1%7D
float tUnitArc(float x, uint32 = tUnitFlip_None);																		


}


// Implementation below this line.


inline std::function<bool(float,float)> tMath::tBiasLess(tIntervalBias bias)
{
	switch (bias)
	{
		case tIntervalBias::Right:
		case tIntervalBias::Outer:
			return [](float a, float b) { return a <= b; };

		case tIntervalBias::Left:
		case tIntervalBias::Inner:
		default:
			return [](float a, float b) { return a < b; };
	}
}


inline std::function<bool(float,float)> tMath::tBiasGreater(tIntervalBias bias)
{
	switch (bias)
	{
		case tIntervalBias::Left:
		case tIntervalBias::Outer:
			return [](float a, float b) { return a >= b; };

		case tIntervalBias::Right:
		case tIntervalBias::Inner:
		default:
			return [](float a, float b) { return a > b; };
	}
}


inline float tMath::tSqrtFast(float x)
{
	#ifdef PLATFORM_WIN
	__m128 in = _mm_set_ss(x);
	__m128 out = _mm_sqrt_ss(in);
	return *(float*)(&out);
	#else
	return tSqrt(x);
	#endif
}


inline float tMath::tRecipSqrtFast(float x)
{
	#ifdef PLATFORM_WIN
	__m128 in = _mm_set_ss(x);
	__m128 out = _mm_rsqrt_ss(in);
	return *(float*)(&out);
	#else
	return (1.0f / tSqrt(x));
	#endif
}


inline float tMath::tSinFast(float x)
{
	float x2 = x*x;
	float r = 7.61e-03f;
	r *= x2;
	r -= 1.6605e-01f;
	r *= x2;
	r += 1.0f;
	r *= x;

	return r;
}


inline void tMath::tCosSin(float& c, float& s, float x)
{
	// We can make a faster version by using pythagoras: cos^2 + sin^2 = 1. However, I don't feel like dealing with the negative roots right
	// now, so it's implemented naively.
	c = tCos(x);
	s = tSin(x);
}


inline void tMath::tCosSinFast(float& c, float& s, float x)
{
	// The fast versions are domain limited so we can use pythagoras without worrying about the negative roots.
	s = tSinFast(x);
	c = tSqrtFast(1.0f - s*s);
}


inline int tMath::tLog2(int x)
{
	if (x <= 0)
		return MinInt32;

	float f = float(x);
	return ((( *(uint32*)((void*)&f) ) & 0x7f800000) >> 23) - 127;
}


inline void tMath::tiNormalizeAngle(float& a, tIntervalBias bias)
{
	std::function<bool(float,float)> less = tBiasLess(bias);
	std::function<bool(float,float)> greater = tBiasGreater(bias);
	while (less(a, -Pi)) a += TwoPi;
	while (greater(a, Pi)) a -= TwoPi;
}


inline void tMath::tiNormalizeAngle2Pi(float& a, tIntervalBias bias)
{
	std::function<bool(float,float)> less = tBiasLess(bias);
	std::function<bool(float,float)> greater = tBiasGreater(bias);
	while (less(a, 0.0f)) a += TwoPi;
	while (greater(a, TwoPi)) a -= TwoPi;
}


inline float tMath::tUnitSin(float x, uint32 flip)
{
	tiClamp(x, 0.0f, 1.0f);
	x = (flip & tUnitFlip_X) ? 1.0f-x : x;
	float y = (tSin(x*Pi - PiOver2) + 1.0f)/2.0f;
	y = (flip & tUnitFlip_Y) ? 1.0f-y : y;
	return y;
}


inline float tMath::tUnitSinHalf(float x, uint32 flip)
{
	tiClamp(x, 0.0f, 1.0f);
	x = (flip & tUnitFlip_X) ? 1.0f-x : x;
	float y = tSin(x*PiOver2);
	y = (flip & tUnitFlip_Y) ? 1.0f-y : y;
	return y;
}


inline float tMath::tUnitPow(float x, float c, uint32 flip)
{
	tiClamp(x, 0.0f, 1.0f);
	x = (flip & tUnitFlip_X) ? 1.0f-x : x;
	float y = tPow(x, c);
	y = (flip & tUnitFlip_Y) ? 1.0f-y : y;
	return y;
}


inline float tMath::tUnitPowPlateau(float x, float c, uint32 flip)
{
	tiClamp(x, 0.0f, 1.0f);
	x = (flip & tUnitFlip_X) ? 1.0f-x : x;

	float y = 0.0f;
	if (x < 0.5f)
		y = 0.5f*tUnitPow(2.0f*x, c);
	else
		y = 0.5f + 0.5f*tUnitPow(2.0f*(x-0.5f), c, tUnitFlip_XY);

	y = (flip & tUnitFlip_Y) ? 1.0f-y : y;
	return y;
}


inline float tMath::tUnitArc(float x, uint32 flip)
{
	tiClamp(x, 0.0f, 1.0f);
	x = (flip & tUnitFlip_X) ? 1.0f-x : x;
	float y = tSqrt(1.0f - (1.0f-x)*(1.0f-x));
	y = (flip & tUnitFlip_Y) ? 1.0f-y : y;
	return y;
}
