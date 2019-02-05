// tMatrix4.h
//
// A 4x4 matrix class with the expected member functions and overloads. Backs off of the tMat4 POD type and the
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
#include "Math/tVector4.h"
namespace tMath
{


struct tMatrix4 : public tMat4
{
	tMatrix4()																											{ }
	tMatrix4(const tMat4& m)																							{ tSet(*this, m); }
	tMatrix4(const tVec4& c1, const tVec4& c2, const tVec4& c3, const tVec4& c4)										{ tSet(*this, c1, c2, c3, c4); }
	tMatrix4(const tVec3& c1, const tVec3& c2, const tVec3& c3, const tVec3& c4)										{ tSet(*this, c1, c2, c3, c4); }
	tMatrix4(const float* a)																							{ tSet(*this, a); }
	tMatrix4(const tQuat& q)								/* Creates a rotation matrix from the quaternion. */		{ tSet(*this, q); }
	tMatrix4												/* Subscript is row# followed by col# for each element. */
	(
		float a11, float a21, float a31, float a41,
		float a12, float a22, float a32, float a42,
		float a13, float a23, float a33, float a43,
		float a14, float a24, float a34, float a44
	)																													{ tSet(*this, a11, a21, a31, a41, a12, a22, a32, a42, a13, a23, a33, a43, a14, a24, a34, a44); }
	const tMat4& Pod() const																							{ return *this; }

	void Set(const tMat4& m)																							{ tSet(*this, m); }
	void Set(const tVec4& c1, const tVec4& c2, const tVec4& c3, const tVec4& c4)										{ tSet(*this, c1, c2, c3, c4); }
	void Set(const tVec3& c1, const tVec3& c2, const tVec3& c3, const tVec3& c4)										{ tSet(*this, c1, c2, c3, c4); }
	void Set(const float* a)																							{ tSet(*this, a); }
	void Set(const tQuat& q)								/* Creates a rotation matrix from the quaternion. */		{ tSet(*this, q); }
	void Set
	(
		float a11, float a21, float a31, float a41,
		float a12, float a22, float a32, float a42,
		float a13, float a23, float a33, float a43,
		float a14, float a24, float a34, float a44
	)																													{ tSet(*this, a11, a21, a31, a41, a12, a22, a32, a42, a13, a23, a33, a43, a14, a24, a34, a44); }
	void Get(float* a) const																							{ tStd::tMemcpy(a, this, 64); }
	tVector4& Col1()																									{ return *((tVector4*)&C1); }
	tVector4& Col2()																									{ return *((tVector4*)&C2); }
	tVector4& Col3()																									{ return *((tVector4*)&C3); }
	tVector4& Col4()																									{ return *((tVector4*)&C4); }

	void Zero()																											{ tZero(*this); }
	void Zero(tComponents c)																							{ tZero(*this, c); }
	bool IsZero() const																									{ return tIsZero(*this); }
	bool IsZero(tComponents c) const																					{ return tIsZero(*this, c); }
	bool ApproxEqual(const tMat4& m, float e = Epsilon) const															{ return tApproxEqual(*this, m, e); }
	bool ApproxEqual(const tMat4& m, tComponents c, float e = Epsilon) const											{ return tApproxEqual(*this, m, c, e); }

	void Identity()																										{ tIdentity(*this); }
	void Transpose()										/* Will invert a purely orthogonal matrix. */				{ tTranspose(*this); }
	float Determinant() const																							{ return tDeterminant(*this); }
	float Trace() const																									{ return tTrace(*this); }
	void Invert()																										{ tInvert(*this); }
	void InvertAffine()																									{ tInvertAffine(*this); }

	// Look in LinearAlgebra for function descriptions of all the composition functions below.
	void MakeTranslate(float x, float y, float z)																		{ tMakeTranslate(*this, x, y, z); }
	void MakeTranslate(const tVec3& t)																					{ tMakeTranslate(*this, t); }
	void MakeRotateX(float angle)																						{ tMakeRotateX(*this, angle); }
	void MakeRotateY(float angle)																						{ tMakeRotateY(*this, angle); }
	void MakeRotateZ(float angle)																						{ tMakeRotateZ(*this, angle); }
	void MakeRotate(const tVec3& axis, float angle)																		{ tMakeRotate(*this, axis, angle); }
	void MakeRotate(const tVec3& a, const tVec3& b)																		{ tMakeRotate(*this, a, b); }
	void MakeRotateXYZ(float eulerX, float eulerY, float eulerZ)														{ tMakeRotateXYZ(*this, eulerX, eulerY, eulerZ); }
	void MakeScale(float scale)																							{ tMakeScale(*this, scale); }
	void MakeScale(float scaleX, float scaleY, float scaleZ)															{ tMakeScale(*this, scaleX, scaleY, scaleZ); }
	void MakeScale(const tVec3& scale)																					{ tMakeScale(*this, scale); }
	void MakeReflectXY()																								{ tMakeReflectXY(*this); }
	void MakeReflectXZ()																								{ tMakeReflectXZ(*this); }
	void MakeReflectYZ()																								{ tMakeReflectYZ(*this); }
	void MakeLookAt(const tVec3& eye, const tVec3& look, const tVec3& up)												{ tMakeLookAt(*this, eye, look, up); }
	void MakeProjPersp(float left, float right, float bottom, float top, float nearPlane, float farPlane)				{ tMakeProjPersp(*this, left, right, bottom, top, nearPlane, farPlane); }
	void MakeProjPersp(const tVec3& boxMin, const tVec3& boxMax)														{ tMakeProjPersp(*this, boxMin, boxMax); }
	void MakeProjPerspSym(float right, float top, float nearPlane, float farPlane)										{ tMakeProjPerspSym(*this, right, top, nearPlane, farPlane); }
	void MakeProjPerspSymFovV(float fovX, float aspect, float nearPlane, float farPlane)								{ tMakeProjPerspSymFovV(*this, fovX, aspect, nearPlane, farPlane); }
	void MakeProjPerspSymFovH(float fovY, float aspect, float nearPlane, float farPlane)								{ tMakeProjPerspSymFovH(*this, fovY, aspect, nearPlane, farPlane); }
	void MakeProjPerspOffset(float right, float top, float nearPlane, float farPlane, float offX, float offY)			{ tMakeProjPerspOffset(*this, right, top, nearPlane, farPlane, offX, offY); }
	void MakeProjPerspFovVOffset(float fovX, float aspect, float nearPlane, float farPlane, float offX, float offY)		{ tMakeProjPerspFovVOffset(*this, fovX, aspect, nearPlane, farPlane, offX, offY); }
	void MakeProjPerspFovHOffset(float fovY, float aspect, float nearPlane, float farPlane, float offX, float offY)		{ tMakeProjPerspFovHOffset(*this, fovY, aspect, nearPlane, farPlane, offX, offY); }
	void MakeProjParallel(const tVec3& boxMin, const tVec3& boxMax)														{ tMakeProjParallel(*this, boxMin, boxMax); }
	void MakeOrthoNormal()																								{ tMakeOrthoNormal(*this); }
	void MakeOrthoUniform()																								{ tMakeOrthoUniform(*this); }
	void MakeOrthoNonUniform()																							{ tMakeOrthoNonUniform(*this); }
	void MakeOrtho()																									{ tMakeOrthoNonUniform(*this); }

	// Look in LinearAlgebra for function descriptions of all the decomposition functions below.
	void ExtractAffineScale(tVec3& scale) const																			{ tExtractAffineScale(scale, *this); }
	void ExtractProjectionPlanes(tVec4 planes[6], bool outwardNormals = false, bool normalizePlanes = true) const		{ tExtractProjectionPlanes(planes, *this, outwardNormals, normalizePlanes); }
	float ExtractProjectionNear() const																					{ return tExtractProjectionNear(*this); }
	float ExtractProjectionFar() const																					{ return tExtractProjectionFar(*this); }
	float ExtractProjectionAspect()	const																				{ return tExtractProjectionAspect(*this); }
	float ExtractProjectionOffsetX() const																				{ return tExtractProjectionOffsetX(*this); }
	float ExtractProjectionOffsetY() const																				{ return tExtractProjectionOffsetY(*this); }
	float ExtractPerspectiveFovV() const																				{ return tExtractPerspectiveFovV(*this); }
	float ExtractPerspectiveFovH() const																				{ return tExtractPerspectiveFovH(*this); }
	void ExtractPerspective(float& fovV, float& fovH, float& aspect, float& nearPlane, float& farPlane) const			{ tExtractPerspective(fovV, fovH, aspect, nearPlane, farPlane, *this); }
	void ExtractPerspective
	(
		float& fovV, float& fovH, float& aspect,
		float& nearPlane, float& farPlane,
		float& offsetX, float& offsetY
	) const																												{ return tExtractPerspective(fovV, fovH, aspect, nearPlane, farPlane, offsetX, offsetY, *this); }

	bool ExtractRotationEulerXYZ
	(
		tVec3& sol1, tVec3& sol2,
		float gimbalZValue = 0.0f, tIntervalBias bias = tIntervalBias::Low
	) const																												{ return tExtractRotationEulerXYZ(sol1, sol2, *this, gimbalZValue, bias); }

	bool ExtractAffineEulerXYZ
	(
		tVec3& sol1, tVec3& sol2,
		float gimbalZValue = 0.0f, tIntervalBias bias = tIntervalBias::Low
	) const																												{ return tExtractAffineEulerXYZ(sol1, sol2, *this, gimbalZValue, bias); } 

	tMatrix4& operator=(const tMat4& m)																					{ tSet(*this, m); return *this; }
	tMatrix4& operator+=(const tMat4& a)																				{ tAdd(*this, a); return *this; }
	const tMatrix4 operator+(const tMat4& a) const																		{ tMatrix4 d; tAdd(d, *this, a); return d; }
	tMatrix4& operator-=(const tMat4& a)																				{ tSub(*this, a); return *this; }
	const tMatrix4 operator-(const tMat4& a) const																		{ tMatrix4 d; tSub(d, *this, a); return d; }
	const tMatrix4 operator-() const																					{ tMatrix4 d; tNeg(d, *this); return d; }

	// The multiplication by a vector is not declared as a friend since we treat vectors as column vectors only,
	// therefore the vector multiplication is not commutable.
	tMatrix4& operator*=(float a)																						{ tMul(*this, a); return *this; }
	tMatrix4& operator*=(const tMat4& a)																				{ tMul(*this, a); return *this; }
	const tMatrix4 operator*(const tMat4& a) const																		{ tMatrix4 d; tMul(d, *this, a); return d; }
	const tVector3 operator*(const tVec3& a) const																		{ tVec3 d; tMul(d, *this, a); return d; }
	const tVector4 operator*(const tVec4& a) const																		{ tVec4 d; tMul(d, *this, a); return d; }
	inline friend const tMatrix4 operator*(float a, const tMat4& b)														{ tMatrix4 d; tMul(d, a, b); return d; }
	inline friend const tMatrix4 operator*(const tMat4& a, float b)														{ tMatrix4 d; tMul(d, a, b); return d; }
	tMatrix4& operator/=(float a)																						{ tDiv(*this, a); return *this; }
	const tMatrix4 operator/(float a) const																				{ tMatrix4 d; tDiv(d, *this, a); return d; }

	bool operator==(const tMat4& a) const																				{ return tEqual(*this, a); }
	bool operator!=(const tMat4& a) const																				{ return tNotEqual(*this, a); }
	operator const float*()																								{ return E; }
	operator const float*() const																						{ return E; }
	float& operator[](int i)																							{ return E[i]; }
	float operator[](int i) const																						{ return E[i]; }

	const static tMatrix4 zero;								// Zero matrix.
	const static tMatrix4 identity;							// Identity matrix.
};


}
