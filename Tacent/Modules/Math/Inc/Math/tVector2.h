// tVector2.h
//
// A 2D vector class with the expected member functions and overloads. Backs off of the tVec2 POD type and the
// tLinearAlgebra library functions.
//
// Copyright (c) 2004-2006, 2015, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include "Math/tLinearAlgebra.h"


namespace tMath
{


// This is the type you should use most of the time for 2D vectors. Extends the POD type with operators etc. Note the
// intentional empty default constructor.
struct tVector2 : public tVec2
{
	tVector2()																											{ }
	tVector2(const tVec2& v)																							{ tSet(*this, v); }
	tVector2(const tVec3& v)								/* Discards v.z. */											{ tSet(*this, v); }
	tVector2(const tVec4& v)								/* Discards v.z and v.w. */									{ tSet(*this, v); }
	tVector2(float xy)										/* Sets both x and y to xy. */								{ tSet(*this, xy); }
	tVector2(float x, float y)																							{ tSet(*this, x, y); }
	tVector2(const float* a)																							{ tSet(*this, a); }
	const tVec2& Pod() const																							{ return *this; }

	void Set(const tVec2& v)																							{ tSet(*this, v); }
	void Set(const tVec3& v)																							{ tSet(*this, v); }
	void Set(const tVec4& v)																							{ tSet(*this, v); }
	void Set(float xy)																									{ tSet(*this, xy); }
	void Set(float x, float y)																							{ tSet(*this, x, y); }
	void Set(const float* a)																							{ tSet(*this, a); }
	void Get(float& x, float& y) const																					{ tGet(x, y, *this); }
	void Get(float* a) const																							{ tGet(a, *this); }

	void Zero()																											{ tZero(*this); }
	void Zero(tComponents c)																							{ tZero(*this, c); }
	bool IsZero() const																									{ return tIsZero(*this); }
	bool IsZero(tComponents c) const																					{ return tIsZero(*this, c); }
	bool ApproxEqual(const tVec2& v, float e = Epsilon) const															{ return tApproxEqual(*this, v, e); }
	bool ApproxEqual(const tVec2& v, tComponents c, float e = Epsilon) const											{ return tApproxEqual(*this, v, c, e); }

	float LengthSq() const																								{ return tLengthSq(*this); }
	float Length() const																								{ return tLength(*this); }
	float LengthFast() const																							{ return tLengthFast(*this); }

	void Normalize()																									{ tNormalize(*this); }
	float NormalizeGetLength()								/* Returns pre-normalized length. */						{ return tNormalizeGetLength(*this); }
	bool NormalizeSafe()									/* Returns success. */										{ return tNormalizeSafe(*this); }
	float NormalizeSafeGetLength()							/* Returns pre-normalized length. 0.0f if no go. */			{ return tNormalizeSafeGetLength(*this); }
	void NormalizeScale(float s)							/* Normalize then scale. Resizes the vector. */				{ tNormalizeScale(*this, s); }
	bool NormalizeScaleSafe(float s)																					{ return tNormalizeScaleSafe(*this, s); }
	void NormalizeFast()																								{ tNormalizeFast(*this); }
	bool NormalizeSafeFast()																							{ return tNormalizeSafeFast(*this); }

	void Lerp(const tVec2& a, const tVec2& b, float t)		/* Also extrapolates. See tLerp comments. */				{ tLerp(*this, a, b, t); }
	inline static int GetNumComponents()																				{ return 2; }

	// Friend is used for commutative binary operators where one of the operators is a built-in type. This essentially
	// puts these operators outside of the class where they belong. If the other operand type is user-defined, the
	// operator should be completely outside as there would be ambiguity as to which one owned the friend declaration.
	tVector2& operator=(const tVec2& v)																					{ tSet(*this, v); return *this; }
	tVector2& operator=(const tVec3& v)																					{ tSet(*this, v); return *this; }
	tVector2& operator=(const tVec4& v)																					{ tSet(*this, v); return *this; }

	// Addition is not defined for types other than the same type as the object itself, so no friend needed.
	tVector2& operator+=(const tVec2& a)																				{ tAdd(*this, a); return *this; }
	const tVector2 operator+(const tVec2& a) const																		{ tVector2 d; tAdd(d, *this, a); return d; }

	tVector2& operator-=(const tVec2& a)																				{ tSub(*this, a); return *this; }
	const tVector2 operator-(const tVec2& a) const																		{ tVector2 d; tSub(d, *this, a); return d; }
	const tVector2 operator-() const																					{ tVector2 d; tNeg(d, *this); return d; }

	tVector2& operator*=(float a)																						{ tMul(*this, a); return *this; }
	inline friend const tVector2 operator*(const tVec2& a, float b)														{ tVector2 d; tMul(d, a, b); return d; }
	inline friend const tVector2 operator*(float a, const tVec2& b)														{ tVector2 d; tMul(d, a, b); return d; }
	float operator*(const tVec2& a) const					/* Dot (inner) product. Same as Transpose(this) * a. */		{ return tDot(*this, a); }

	// Divide is not commutative so no need for friend.
	tVector2& operator/=(float a)																						{ tDiv(*this, a); return *this; }
	const tVector2 operator/(float a) const																				{ tVector2 d; tDiv(d, *this, a); return d; }

	bool operator==(const tVec2& a) const																				{ return tEqual(*this, a); }
	bool operator!=(const tVec2& a) const																				{ return tNotEqual(*this, a); }
	operator const float*()																								{ return E; }
	operator const float*() const																						{ return E; }
	float& operator[](int i)																							{ return E[i]; }
	float operator[](int i) const																						{ return E[i]; }

	const static tVector2 zero;								// Zero vector  (0, 0).
	const static tVector2 i;								// Basis vector (1, 0).
	const static tVector2 j;								// Basis vector (0, 1).
};


}
