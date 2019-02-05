// tQuaternion.h
//
// A quaternion class for representing rotations along with expected functionality such as spherical and
// normalized linear interpolation. Ability to construct unit quaternions (rotations) from axis-angle as
// well as from rotation matrices.
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


struct tQuaternion : public tQuat
{
	tQuaternion()																										{ }
	tQuaternion(const tQuat& s)																							{ tSet(*this, s); }
	tQuaternion(float x, float y, float z, float w)																		{ tSet(*this, x, y, z, w); }
	tQuaternion(const float* a)																							{ tSet(*this, a); }
	tQuaternion(const tMat4& orientation)					/* Unit quaternion from a 4x4 rotation matrix. */			{ tSet(*this, orientation); }
	tQuaternion(float r, tVec3 v)							/* r,v is used in textbooks. This is not axis-angle. */		{ tSet(*this, r, v); }
	tQuaternion(const tVec3& axis, float angle)				/* Assumes axis vector is normalized. */					{ tSet(*this, axis, angle); }
	tQuaternion(const tVec3& v)								/* r (w) gets set to 0.0f. */								{ tSet(*this, v); }
	const tQuat& Pod() const																							{ return *this; }

	void Set(const tQuat& s)																							{ tSet(*this, s); }
	void Set(float x, float y, float z, float w)																		{ tSet(*this, x, y, z, w); }
	void Set(const float* a)																							{ tSet(*this, a); }
	void Set(const tMat4& orientation)																					{ tSet(*this, orientation); }
	void Set(float r, tVec3 v)																							{ tSet(*this, r, v); }
	void Set(const tVec3& axis, float angle)																			{ tSet(*this, axis, angle); }
	void Set(const tVec3& v)																							{ tSet(*this, v); }
	void Get(float& x, float& y, float& z, float& w) const																{ tGet(x, y, z, w, *this); }
	void Get(float* a) const																							{ tGet(a, *this); }
	void Get(tVec3& axis, float& angle) const																			{ tGet(axis, angle, *this); }

	void Zero()																											{ tZero(*this); }
	void Zero(tComponents c)																							{ tZero(*this, c); }
	bool IsZero() const																									{ return tIsZero(*this); }
	bool IsZero(tComponents c) const																					{ return tIsZero(*this, c); }
	bool ApproxEqual(const tQuat& q, float e = Epsilon) const															{ return tApproxEqual(*this, q, e); }
	bool ApproxEqual(const tQuat& q, tComponents c, float e = Epsilon) const											{ return tApproxEqual(*this, q, c, e); }

	float LengthSq() const																								{ return tLengthSq(*this); }
	float Length() const																								{ return tLength(*this); }
	float LengthFast() const																							{ return tLengthFast(*this); }
	float NormSq() const									/* Returns the norm (length) squared. */					{ return tLengthSq(*this); }
	float Norm() const										/* Returns the norm (length). */							{ return tLength(*this); }
	float NormFast() const									/* Less accurate but faster. */								{ return tLengthFast(*this); }

	void Normalize()																									{ tNormalize(*this); }
	float NormalizeGetLength()								/* Returns pre-normalized length. */						{ return tNormalizeGetLength(*this); }
	bool NormalizeSafe()									/* Returns success. */										{ return tNormalizeSafe(*this); }
	float NormalizeSafeGetLength()							/* Returns pre-normalized length. 0.0f if no go. */			{ return tNormalizeSafeGetLength(*this); }
	void NormalizeFast()																								{ tNormalizeFast(*this); }
	bool NormalizeSafeFast()																							{ return tNormalizeSafeFast(*this); }

	// The lack of invert functions is intentional. For unit quaternion just use the conjugate functions.
	tQuat Conjugate() const																								{ tQuat d; tConjugate(d, *this); return d; }
	tQuat& MakeConjugate()																								{ tConjugate(*this); return *this; }
	void Slerp(const tQuat& a, const tQuat& b, float t)		/* Spherical linear interpolation between a and b. */		{ tSlerp(*this, a, b, t); }
	void Nlerp(const tQuat& a, const tQuat& b, float t)		/* Normalized linear interpolation between a and b. */		{ tNlerp(*this, a, b, t); }
	float RotationAngle() const																							{ return tRotationAngle(*this); }
	void Rotate(tVec3& v) const								/* Rotates the vector. */									{ tRotate(v, *this); }
	void Rotate(tVec4& v) const								/* Rotates the vector. Needs testing. */					{ tRotate(v, *this); }
	inline static int GetNumComponents()																				{ return 4; }

	// Don't add extra operators without first checking the math.
	tQuaternion& operator=(const tQuat& q)																				{ tSet(*this, q); return *this; }

	tQuaternion& operator+=(const tQuat& a)																				{ tAdd(*this, a); return *this; }
	const tQuaternion operator+(const tQuat& a) const																	{ tQuaternion d; tAdd(d, *this, a); return d; }

	tQuaternion& operator-=(const tQuat& a)																				{ tSub(*this, a); return *this; }
	const tQuaternion operator-(const tQuat& a) const																	{ tQuaternion d; tSub(d, *this, a); return d; }
	const tQuaternion operator-() const																					{ tQuaternion d; tNeg(d, *this); return d; }

	tQuaternion& operator*=(float a)																					{ tMul(*this, a); return *this; }
	tQuaternion& operator*=(const tQuat& a)																				{ tMul(*this, a); return *this; }
	tQuaternion& operator*=(const tVec3& a)					/* Treats 'a' as an augmented (pure) quaternion. w = 0. */	{ tMul(*this, a); return *this; }
	const tQuaternion operator*(const tQuat& a) const																	{ tQuaternion d; tMul(d, *this, a); return d; }
	const tQuaternion operator*(const tVec3& a) const		/* Treats 'a' as an augmented (pure) quaternion. w = 0. */	{ tQuaternion d; tMul(d, *this, a); return d; }
	friend const tQuaternion operator*(const tQuat& a, float b)															{ tQuaternion d; tMul(d, a, b); return d; }
	friend const tQuaternion operator*(float a, const tQuat& b)															{ tQuaternion d; tMul(d, a, b); return d; }

	tQuaternion& operator/=(float a)																					{ tDiv(*this, a); return *this; }
	const tQuaternion operator/(float a) const																			{ tQuaternion d; tDiv(d, *this, a); return d; }

	bool operator==(const tQuat& a) const																				{ return tEqual(*this, a); }
	bool operator!=(const tQuat& a) const																				{ return tNotEqual(*this, a); }
	operator const float*()																								{ return E; }
	operator const float*() const																						{ return E; }
	float& operator[](int i)																							{ return E[i]; }
	float operator[](int i) const																						{ return E[i]; }

	const static tQuaternion zero;							// Zero quaternion (0, 0, 0, 0).
	const static tQuaternion unit;							// Unit quaternion (0, 0, 0, 1).
};


}
