// tMatrix2.h
//
// A 2x2 matrix class with the expected member functions and overloads. Backs off of the tMat2 POD type and the
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
#include "Math/tVector2.h"
namespace tMath
{


struct tMatrix2 : public tMat2
{
	tMatrix2()																											{ }
	tMatrix2(const tMat2& m)																							{ tSet(*this, m); }
	tMatrix2(const tVec2& c1, const tVec2& c2)																			{ tSet(*this, c1, c2); }
	tMatrix2(const float* a)																							{ tSet(*this, a); }
	tMatrix2(float a11, float a21, float a12, float a22)																{ tSet(*this, a11, a21, a12, a22); }
	const tMat2& Pod() const																							{ return *this; }

	void Set(const tMat2& m)																							{ tSet(*this, m); }
	void Set(const tVec2& c1, const tVec2& c2)																			{ tSet(*this, c1, c2); }
	void Set(float a11, float a21, float a12, float a22)																{ tSet(*this, a11, a21, a12, a22); }
	void Set(const float* a)																							{ tSet(*this, a); }
	void Get(float* a) const																							{ tGet(a, *this); }
	tVector2& Col1()																									{ return *((tVector2*)&C1); }
	tVector2& Col2()																									{ return *((tVector2*)&C2); }

	void Zero()																											{ tZero(*this); }
	void Zero(tComponent c)																								{ tZero(*this, c); }
	bool IsZero() const																									{ return tIsZero(*this); }
	bool IsZero(tComponent c) const																						{ return tIsZero(*this, c); }
	bool ApproxEqual(const tMat2& m, float e = Epsilon) const															{ return tApproxEqual(*this, m, e); }
	bool ApproxEqual(const tMat2& m, tComponent c, float e = Epsilon) const												{ return tApproxEqual(*this, m, c, e); }

	void Identity()																										{ tIdentity(*this); }
	void Transpose()																									{ tTranspose(*this); }
	float Determinant() const																							{ return tDeterminant(*this); }
	float Trace() const																									{ return tTrace(*this); }
	void Invert()																										{ tInvert(*this); }

	tMatrix2& operator=(const tMat2& m)																					{ tSet(*this, m); return *this; }
	tMatrix2& operator+=(const tMat2& a)																				{ tAdd(*this, a); return *this; }
	const tMatrix2 operator+(const tMat2& a) const																		{ tMatrix2 d; tAdd(d, *this, a); return d; }
	tMatrix2& operator-=(const tMat2& a)																				{ tSub(*this, a); return *this; }
	const tMatrix2 operator-(const tMat2& a) const																		{ tMatrix2 d; tSub(d, *this, a); return d; }
	const tMatrix2 operator-() const																					{ tMatrix2 d; tNeg(d, *this); return d; }

	// The multiplication by a vector is not declared as a friend since we treat vectors as column vectors only,
	// therefore the vector multiplication is not commutable.
	tMatrix2& operator*=(float a)																						{ tMul(*this, a); return *this; }
	tMatrix2& operator*=(const tMat2& a)																				{ tMul(*this, a); return *this; }
	const tMatrix2 operator*(const tMat2& a) const																		{ tMatrix2 d; tMul(d, *this, a); return d; }
	const tVec2 operator*(const tVec2& a) const																			{ tVec2 d; tMul(d, *this, a); return d; }
	inline friend const tMatrix2 operator*(float a, const tMat2& b)														{ tMatrix2 d; tMul(d, a, b); return d; }
	inline friend const tMatrix2 operator*(const tMat2& a, float b)														{ tMatrix2 d; tMul(d, a, b); return d; }
	tMatrix2& operator/=(float a)																						{ tDiv(*this, a); return *this; }
	const tMatrix2 operator/(float a) const																				{ tMatrix2 d; tDiv(d, *this, a); return d; }

	bool operator==(const tMat2& a) const																				{ return tEqual(*this, a); }
	bool operator!=(const tMat2& a) const																				{ return tNotEqual(*this, a); }
	operator const float*()																								{ return E; }
	operator const float*() const																						{ return E; }
	float& operator[](int i)																							{ return E[i]; }
	float operator[](int i) const																						{ return E[i]; }

	const static tMatrix2 zero;								// Zero matrix.
	const static tMatrix2 identity;							// Identity matrix.
};


}
