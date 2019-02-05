// tVector3.h
//
// A 3D vector class with the expected member functions and overloads. Backs off of the tVec3 POD type and the
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


struct tVector3 : public tVec3
{
	tVector3()																											{ }
	tVector3(const tVec2& v, float z = 0.0f)																			{ tSet(*this, v, z); }
	tVector3(const tVec3& v)																							{ tSet(*this, v); }
	tVector3(const tVec4& v)								/* Discards 4th component. */								{ tSet(*this, v); }
	tVector3(float xyz)																									{ tSet(*this, xyz); }
	tVector3(float x, float y, float z)																					{ tSet(*this, x, y, z); }
	tVector3(const float* a)																							{ tSet(*this, a); }
	const tVec3& Pod() const																							{ return *this; }

	void Set(const tVec2& v, float z = 0.0f)																			{ tSet(*this, v, z); }
	void Set(const tVec3& v)																							{ tSet(*this, v); }
	void Set(const tVec4& v)																							{ tSet(*this, v); }
	void Set(float xyz)																									{ tSet(*this, xyz); }
	void Set(float x, float y, float z)																					{ tSet(*this, x, y, z); }
	void Set(const float* a)																							{ tSet(*this, a); }
	void Get(float& x, float& y, float& z) const																		{ tGet(x, y, z, *this); }
	void Get(float* a) const																							{ tGet(a, *this); }

	void Zero()																											{ tZero(*this); }
	void Zero(tComponents c)																							{ tZero(*this, c); }
	bool IsZero() const																									{ return tIsZero(*this); }
	bool IsZero(tComponents c) const																					{ return tIsZero(*this, c); }
	bool ApproxEqual(const tVec3& v, float e = Epsilon) const															{ return tApproxEqual(*this, v, e); }
	bool ApproxEqual(const tVec3& v, tComponents c, float e = Epsilon) const											{ return tApproxEqual(*this, v, c, e); }

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

	void Lerp(const tVec3& a, const tVec3& b, float t)		/* Also extrapolates. See tLerp comments. */				{ tLerp(*this, a, b, t); }
	const tMat4 MulByTranspose(const tVec3& a)																			{ tMat4 d; tMulByTranspose(d, *this, a); return d; }
	inline static int GetNumComponents()																				{ return 3; }

	// Friend is used for commutative binary operators where one of the operators is a built-in type. This essentially
	// puts these operators outside of the class where they belong. If the other operand type is user-defined, the
	// operator should be completely outside as there would be ambiguity as to which one owned the friend declaration.
	tVector3& operator=(const tVec2& v)																					{ tSet(*this, v); return *this; }
	tVector3& operator=(const tVec3& v)																					{ tSet(*this, v); return *this; }
	tVector3& operator=(const tVec4& v)																					{ tSet(*this, v); return *this; }

	// Addition is not defined for types other than the same type as the object itself, so no friend needed.
	tVector3& operator+=(const tVec3& a)																				{ tAdd(*this, a); return *this; }
	const tVector3 operator+(const tVec3& a) const																		{ tVector3 d; tAdd(d, *this, a); return d; }

	tVector3& operator-=(const tVec3& a)																				{ tSub(*this, a); return *this; }
	const tVector3 operator-(const tVec3& a) const																		{ tVector3 d; tSub(d, *this, a); return d; }
	const tVector3 operator-() const																					{ tVector3 d; tNeg(d, *this); return d; }

	tVector3& operator*=(float a)																						{ tMul(*this, a); return *this; }
	inline friend const tVector3 operator*(const tVec3& a, float b)														{ tVector3 d; tMul(d, a, b); return d; }
	inline friend const tVector3 operator*(float a, const tVec3& b)														{ tVector3 d; tMul(d, a, b); return d; }
	float operator*(const tVec3& a) const					/* Dot (inner) product. Same as Transpose(this) * a. */		{ return tDot(*this, a); }

	// Divide is not commutative so no need for friend.
	tVector3& operator/=(float a)																						{ tDiv(*this, a); return *this; }
	const tVector3 operator/(float a) const																				{ tVector3 d; tDiv(d, *this, a); return d; }

	tVector3& operator%=(const tVec3& v)					/* Cross product for 3D vectors. */							{ tCross(*this, *this, v); return *this; }
	tVector3 operator%(const tVec3& v) const				/* Cross product for 3D vectors. */							{ tVec3 c; tCross(c, *this, v); return c; }

	bool operator==(const tVec3& a) const																				{ return tEqual(*this, a); }
	bool operator!=(const tVec3& a) const																				{ return tNotEqual(*this, a); }
	operator const float*()																								{ return E; }
	operator const float*() const																						{ return E; }
	float& operator[](int i)																							{ return E[i]; }
	float operator[](int i) const																						{ return E[i]; }

	const static tVector3 zero;								// Zero vector  (0, 0, 0).
	const static tVector3 i;								// Basis vector (1, 0, 0).
	const static tVector3 j;								// Basis vector (0, 1, 0).
	const static tVector3 k;								// Basis vector (0, 0, 1).
};


}
