// tLinearAlgebra.cpp
//
// POD types for Vectors, Matrices and a function library to manipulate them. Includes geometrical transformations,
// cross/dot products, inversion functions, projections, normalization etc. These POD types are used as superclasses
// for the more object-oriented and complete derived types. eg. tVector3 derives from the POD type tVec2 found here.
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

#include "Math/tLinearAlgebra.h"
#include "Math/tVector2.h"
#include "Math/tVector3.h"
#include "Math/tVector4.h"
#include "Math/tQuaternion.h"
#include "Math/tMatrix2.h"
#include "Math/tMatrix4.h"


const tMath::tVector2 tMath::tVector2::zero			= { 0.0f, 0.0f };
const tMath::tVector2 tMath::tVector2::i			= { 1.0f, 0.0f };
const tMath::tVector2 tMath::tVector2::j			= { 0.0f, 1.0f };
const tMath::tVector3 tMath::tVector3::zero			= { 0.0f, 0.0f, 0.0f };
const tMath::tVector3 tMath::tVector3::i			= { 1.0f, 0.0f, 0.0f };
const tMath::tVector3 tMath::tVector3::j			= { 0.0f, 1.0f, 0.0f };
const tMath::tVector3 tMath::tVector3::k			= { 0.0f, 0.0f, 1.0f };
const tMath::tVector4 tMath::tVector4::zero			= { 0.0f, 0.0f, 0.0f, 0.0f };
const tMath::tVector4 tMath::tVector4::i			= { 1.0f, 0.0f, 0.0f, 1.0f };
const tMath::tVector4 tMath::tVector4::j			= { 0.0f, 1.0f, 0.0f, 1.0f };
const tMath::tVector4 tMath::tVector4::k			= { 0.0f, 0.0f, 1.0f, 1.0f };
const tMath::tVector4 tMath::tVector4::origin		= { 0.0f, 0.0f, 0.0f, 1.0f };
const tMath::tQuaternion tMath::tQuaternion::zero	= { 0.0f, 0.0f, 0.0f, 0.0f };
const tMath::tQuaternion tMath::tQuaternion::unit	= { 0.0f, 0.0f, 0.0f, 1.0f };
const tMath::tMatrix2 tMath::tMatrix2::zero			= { 0.0f, 0.0f, 0.0f, 0.0f };
const tMath::tMatrix2 tMath::tMatrix2::identity		= { 1.0f, 0.0f, 0.0f, 1.0f };
const tMath::tMatrix4 tMath::tMatrix4::zero			= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
const tMath::tMatrix4 tMath::tMatrix4::identity		= { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };


void tMath::tSet(tQuat& d, const tMat4& m)
{
	float trace = m.E[0] + m.E[5] + m.E[10];

	// The 0.0 here implies a decision that when w < 0.5 we consider rounding errors to be a potential issue.
	if (trace > 0.0f)
	{
		// w >= 0.5. Negligible error.
		float s = tSqrt(trace + 1.0f);
		d.w = 0.5f * s;
		s = 0.5f / s;
		d.x = (m.E[6] - m.E[9]) * s;
		d.y = (m.E[8] - m.E[2]) * s;
		d.z = (m.E[1] - m.E[4]) * s;
	}
	else
	{
		// w < 0.5. Find the largest of the x, y, z components. The remaining components get computed from this
		// to avoid rounding errors.
		if ((m.E[0] > m.E[5]) && (m.E[0] > m.E[10]))
		{
			float s = tSqrt((m.E[0] - (m.E[5] + m.E[10])) + 1.0f);
			d.x = 0.5f * s;
			s = 0.5f / s;
			d.y = (m.E[4] + m.E[1]) * s;
			d.z = (m.E[2] + m.E[8]) * s;
			d.w = (m.E[6] - m.E[9]) * s;
		}
		else if (m.E[5] > m.E[10])
		{
			float s = tSqrt((m.E[5] - (m.E[10] + m.E[0])) + 1.0f);
			d.y = 0.5f * s;
			s = 0.5f / s;
			d.z = (m.E[9] + m.E[6]) * s;
			d.x = (m.E[4] + m.E[1]) * s;
			d.w = (m.E[8] - m.E[2]) * s;
		}
		else
		{
			float s = tSqrt((m.E[10] - (m.E[0] + m.E[5])) + 1.0f);
			d.z = 0.5f * s;
			s = 0.5f / s;
			d.x = (m.E[2] + m.E[8]) * s;
			d.y = (m.E[9] + m.E[6]) * s;
			d.w = (m.E[1] - m.E[4]) * s;
		}
	}
}


void tMath::tSet(tQuat& d, const tVec3& axis, float angle)
{
	d.r = tCos(angle / 2.0f);
	float s = tSin(angle / 2.0f);
	tSet(d.v, axis.x * s, axis.y * s, axis.z * s);
}


void tMath::tSet(tMat4& d, const tQuat& s)
{
	float yy = s.y * s.y;
	float xx = s.x * s.x;
	float zz = s.z * s.z;
	float xy = s.x * s.y;
	float yz = s.y * s.z;
	float wz = s.w * s.z;
	float xz = s.x * s.z;
	float wx = s.w * s.x;
	float wy = s.w * s.y;

	d.E[0] = 1.0f - 2.0f*(yy + zz);
	d.E[1] = 2.0f*(xy + wz);
	d.E[2] = 2.0f*(xz - wy);
	d.E[3] = 0.0f;

	d.E[4] = 2.0f*(xy - wz);
	d.E[5] = 1.0f - 2.0f*(xx + zz);
	d.E[6] = 2.0f*(yz + wx);
	d.E[7] = 0.0f;

	d.E[8] = 2.0f*(xz + wy);
	d.E[9] = 2.0f*(yz - wx);
	d.E[10] = 1.0f - 2.0f*(xx + yy);
	d.E[11] = 0.0f;

	d.E[12] = 0.0f;
	d.E[13] = 0.0f;
	d.E[14] = 0.0f;
	d.E[15] = 1.0f;
}


void tMath::tGet(tVec3& axis, float& angle, const tQuat& q)
{
	angle = 2.0f * tArcCos(q.w);			// Angle E [0, 2Pi]
	float s = tSqrt(1.0f - q.w*q.w);		// Assumes normalised so Abs(w) < 1, w*w E [0,1], and 1-w*w > 0.
	if (s > 0.00001f)						// Avoid infinity. 
		tSet(axis, q.x/s, q.y/s, q.z/s);
	else
		tSet(axis, 1.0f, 0.0f, 0.0f);		// The axis direction doesn't matter so (1,0,0) is fine.

	if (angle > Pi)
	{
		// To return an angle E [0,Pi] we may have to reverse the axis direction.
		tNeg(axis);
		angle = TwoPi - angle;
	}
}


void tMath::tMul(tVec3& d, const tMat4& a, const tVec3& b)
{
	// It would be nice to have an intrinsics-based implementation instead of this inline assembly.
	// Unfortunately inline asm is not supported on architectures like x64.
	float x = b.x*a.a11 + b.y*a.a12 + b.z*a.a13 + a.a14;
	float y = b.x*a.a21 + b.y*a.a22 + b.z*a.a23 + a.a24;
	float z = b.x*a.a31 + b.y*a.a32 + b.z*a.a33 + a.a34;
	tSet(d, x, y, z);
}


void tMath::tMul(tVec4& d, const tMat4& a, const tVec4& b)
{
	// @todo Optimize this using intrinsics or SSE.
	float x = b.x*a.a11 + b.y*a.a12 + b.z*a.a13 + b.w*a.a14;
	float y = b.x*a.a21 + b.y*a.a22 + b.z*a.a23 + b.w*a.a24;
	float z = b.x*a.a31 + b.y*a.a32 + b.z*a.a33 + b.w*a.a34;
	float w = b.x*a.a41 + b.y*a.a42 + b.z*a.a43 + b.w*a.a44;

	tSet(d, x, y, z, w);
}


void tMath::tMul(tMat4& d, const tMat4& a, const tMat4& b)
{
	for (int i = 0; i < 4; ++i)
	{
		float a0i = a.A[0][i];
		float a1i = a.A[1][i];
		float a2i = a.A[2][i];
		float a3i = a.A[3][i];

		d.A[0][i] = (a0i * b.a11) + (a1i * b.a21) + (a2i * b.a31) + (a3i * b.a41);
		d.A[1][i] = (a0i * b.a12) + (a1i * b.a22) + (a2i * b.a32) + (a3i * b.a42);
		d.A[2][i] = (a0i * b.a13) + (a1i * b.a23) + (a2i * b.a33) + (a3i * b.a43);
		d.A[3][i] = (a0i * b.a14) + (a1i * b.a24) + (a2i * b.a34) + (a3i * b.a44);
	}
}


void tMath::tSlerp(tQuat& d, const tQuat& a, const tQuat& b, float t)
{
	if (t >= 1.0f)
	{
		d = b;
		return;
	}

	if (t <= 0.0f)
	{
		d = a;
		return;
	}

	float cosTheta = tDot(a, b);
	if (cosTheta < 0.0f)
	{
		cosTheta = -cosTheta;
		tNeg(d, b);
	}
	else
	{
		d = b;
	}

	const float delta = 0.98f;
	float facA = 1.0f - t;
	float facB = t;
	if (cosTheta < delta)
	{
		float theta = tArcCos(cosTheta);
		float sinTheta = tSin(theta);
		float den = 1.0f / sinTheta;

		facA = den * tSin((1.0f - t) * theta);
		facB = den * tSin(t * theta);
	}

	d.x = facA*a.x + facB*d.x;
	d.y = facA*a.y + facB*d.y;
	d.z = facA*a.z + facB*d.z;
	d.w = facA*a.w + facB*d.w;
}


void tMath::tSlerp(tQuat& d, const tQuat& a, const tQuat& bb, float t, tOrientationPath path)
{
	if (t >= 1.0f)
	{
		d = bb;
		return;
	}

	if (t <= 0.0f)
	{
		d = a;
		return;
	}

	tQuat b;
	tSet(b, bb);
	float cosTheta = tDot(a, b);

	if
	(
		((path == tOrientationPath::Shortest) && (cosTheta < 0.0f)) ||		// Shortest spherical path.
		((path == tOrientationPath::Longest) && (cosTheta > 0.0f)) ||		// Longest spherical path.
		((path == tOrientationPath::Clockwise) && (a.w > b.w)) ||			// Clockwise spherical path.
		((path == tOrientationPath::CounterClockwise) && (a.w < b.w))		// Counterclockwise spherical path.
	)
	{
		cosTheta = -cosTheta;
		tNeg(b);
	}

	// We need to separate out the top of the cos curve (where it's flat and the sin goes to 0).
	// In the same way that sin(x) ~= x for small x, cos(x) ~= 1 for small x.
	float thresh = 0.98f;
	float facA = 1.0f - t;
	float facB = t;
	if (cosTheta < thresh)
	{
		float theta = tArcCos(cosTheta);
		float sinTheta = tSin(theta);
		float inv = 1.0f / sinTheta;
		facA = inv * tSin((1.0f - t) * theta);
		facB = inv * tSin(t * theta);
	}

	d.x = facA*a.x + facB*b.x;
	d.y = facA*a.y + facB*b.y;
	d.z = facA*a.z + facB*b.z;
	d.w = facA*a.w + facB*b.w;
}


void tMath::tNlerp(tQuat& d, const tQuat& a, const tQuat& bb, float t, tOrientationPath path)
{
	if (t >= 1.0f)
	{
		d = bb;
		return;
	}

	if (t <= 0.0f)
	{
		d = a;
		return;
	}

	tQuat b;
	tSet(b, bb);
	float cosTheta = tDot(a, b);

	if
	(
		((path == tOrientationPath::Shortest) && (cosTheta < 0.0f)) ||			// Shortest linear path.
		((path == tOrientationPath::Longest) && (cosTheta > 0.0f)) ||			// Longest linear path.
		((path == tOrientationPath::Clockwise) && (a.w > b.w)) ||				// Clockwise path.
		((path == tOrientationPath::CounterClockwise) && (a.w < b.w))			// Counterclockwise path.
	)
	{
		cosTheta = -cosTheta;
		tNeg(b);
	}

	float facA = 1.0f - t;
	float facB = t;
	d.x = facA*a.x + facB*b.x;
	d.y = facA*a.y + facB*b.y;
	d.z = facA*a.z + facB*b.z;
	d.w = facA*a.w + facB*b.w;
	tNormalize(d);
}


float tMath::tDeterminant(const tMat4& m)
{
	return
		m.A[0][0] *
		(
			m.A[1][1] * (m.A[2][2] * m.A[3][3] - m.A[2][3] * m.A[3][2]) +
			m.A[2][1] * (m.A[1][3] * m.A[3][2] - m.A[1][2] * m.A[3][3]) +
			m.A[3][1] * (m.A[1][2] * m.A[2][3] - m.A[1][3] * m.A[2][2])
		) -
		m.A[1][0] *
		(
			m.A[0][1] * (m.A[2][2] * m.A[3][3] - m.A[2][3] * m.A[3][2]) +
			m.A[2][1] * (m.A[0][3] * m.A[3][2] - m.A[0][2] * m.A[3][3]) +
			m.A[3][1] * (m.A[0][2] * m.A[2][3] - m.A[0][3] * m.A[2][2])
		) +
		m.A[2][0] *
		(
			m.A[0][1] * (m.A[1][2] * m.A[3][3] - m.A[1][3] * m.A[3][2]) +
			m.A[1][1] * (m.A[0][3] * m.A[3][2] - m.A[0][2] * m.A[3][3]) +
			m.A[3][1] * (m.A[0][2] * m.A[1][3] - m.A[0][3] * m.A[1][2])
		) -
		m.A[3][0] *
		(
			m.A[0][1] * (m.A[1][2] * m.A[2][3] - m.A[1][3] * m.A[2][2]) +
			m.A[1][1] * (m.A[0][3] * m.A[2][2] - m.A[0][2] * m.A[2][3]) +
			m.A[2][1] * (m.A[0][2] * m.A[1][3] - m.A[0][3] * m.A[1][2])
		);
}


bool tMath::tInvert(tMat4& m)
{
	#if defined(PLATFORM_WINDOWS)
	if (tDeterminant(m) == 0.0f)
		return false;

	// Naming: m for minor. de for determinant. t0 for temporary. r for row.
	__m128 m1, m2, m3, m4, r1, r3, de;
	__m128 r2 = _mm_setzero_ps();
	__m128 r4 = _mm_setzero_ps();
	__m128 t0 = _mm_setzero_ps();

	// Cramer's rule used here (not Gaussian Elim).
	t0 = _mm_loadh_pi(_mm_loadl_pi(t0, (__m64*)(m.E+ 0)), (__m64*)(m.E+ 4));
	r2 = _mm_loadh_pi(_mm_loadl_pi(r2, (__m64*)(m.E+ 8)), (__m64*)(m.E+12));
	r1 = _mm_shuffle_ps(t0, r2, 0x88);
	r2 = _mm_shuffle_ps(r2, t0, 0xDD);
	t0 = _mm_loadh_pi(_mm_loadl_pi(t0, (__m64*)(m.E+ 2)), (__m64*)(m.E+ 6));
	r4 = _mm_loadh_pi(_mm_loadl_pi(r4, (__m64*)(m.E+10)), (__m64*)(m.E+14));
	r3 = _mm_shuffle_ps(t0, r4, 0x88);
	r4 = _mm_shuffle_ps(r4, t0, 0xDD);
	t0 = _mm_mul_ps(r3, r4);
	t0 = _mm_shuffle_ps(t0, t0, 0xB1);
	m1 = _mm_mul_ps(r2, t0);
	m2 = _mm_mul_ps(r1, t0);
	t0 = _mm_shuffle_ps(t0, t0, 0x4E);
	m1 = _mm_sub_ps(_mm_mul_ps(r2, t0), m1);
	m2 = _mm_sub_ps(_mm_mul_ps(r1, t0), m2);
	m2 = _mm_shuffle_ps(m2, m2, 0x4E);
	t0 = _mm_mul_ps(r2, r3);
	t0 = _mm_shuffle_ps(t0, t0, 0xB1);
	m1 = _mm_add_ps(_mm_mul_ps(r4, t0), m1);
	m4 = _mm_mul_ps(r1, t0);
	t0 = _mm_shuffle_ps(t0, t0, 0x4E);
	m1 = _mm_sub_ps(m1, _mm_mul_ps(r4, t0));
	m4 = _mm_sub_ps(_mm_mul_ps(r1, t0), m4);
	m4 = _mm_shuffle_ps(m4, m4, 0x4E);
	t0 = _mm_mul_ps(_mm_shuffle_ps(r2, r2, 0x4E), r4);
	t0 = _mm_shuffle_ps(t0, t0, 0xB1);
	r3 = _mm_shuffle_ps(r3, r3, 0x4E);
	m1 = _mm_add_ps(_mm_mul_ps(r3, t0), m1);
	m3 = _mm_mul_ps(r1, t0);
	t0 = _mm_shuffle_ps(t0, t0, 0x4E);
	m1 = _mm_sub_ps(m1, _mm_mul_ps(r3, t0));
	m3 = _mm_sub_ps(_mm_mul_ps(r1, t0), m3);
	m3 = _mm_shuffle_ps(m3, m3, 0x4E);
	t0 = _mm_mul_ps(r1, r2);
	t0 = _mm_shuffle_ps(t0, t0, 0xB1);
	m3 = _mm_add_ps(_mm_mul_ps(r4, t0), m3);
	m4 = _mm_sub_ps(_mm_mul_ps(r3, t0), m4);
	t0 = _mm_shuffle_ps(t0, t0, 0x4E);
	m3 = _mm_sub_ps(_mm_mul_ps(r4, t0), m3);
	m4 = _mm_sub_ps(m4, _mm_mul_ps(r3, t0));
	t0 = _mm_mul_ps(r1, r4);
	t0 = _mm_shuffle_ps(t0, t0, 0xB1);
	m2 = _mm_sub_ps(m2, _mm_mul_ps(r3, t0));
	m3 = _mm_add_ps(_mm_mul_ps(r2, t0), m3);
	t0 = _mm_shuffle_ps(t0, t0, 0x4E);
	m2 = _mm_add_ps(_mm_mul_ps(r3, t0), m2);
	m3 = _mm_sub_ps(m3, _mm_mul_ps(r2, t0));
	t0 = _mm_mul_ps(r1, r3);
	t0 = _mm_shuffle_ps(t0, t0, 0xB1);
	m2 = _mm_add_ps(_mm_mul_ps(r4, t0), m2);
	m4 = _mm_sub_ps(m4, _mm_mul_ps(r2, t0));
	t0 = _mm_shuffle_ps(t0, t0, 0x4E);
	m2 = _mm_sub_ps(m2, _mm_mul_ps(r4, t0));
	m4 = _mm_add_ps(_mm_mul_ps(r2, t0), m4);
	de = _mm_mul_ps(r1, m1);
	de = _mm_add_ps(_mm_shuffle_ps(de, de, 0x4E), de);
	de = _mm_add_ss(_mm_shuffle_ps(de, de, 0xB1), de);
	t0 = _mm_rcp_ss(de);
	de = _mm_sub_ss(_mm_add_ss(t0, t0), _mm_mul_ss(de, _mm_mul_ss(t0, t0)));
	de = _mm_shuffle_ps(de, de, 0x00);
	m1 = _mm_mul_ps(de, m1);

	// Assign the result.
	_mm_storel_pi((__m64*)(m.E+ 0), m1);
	_mm_storeh_pi((__m64*)(m.E+ 2), m1);
	m2 = _mm_mul_ps(de, m2);
	_mm_storel_pi((__m64*)(m.E+ 4), m2);
	_mm_storeh_pi((__m64*)(m.E+ 6), m2);
	m3 = _mm_mul_ps(de, m3);
	_mm_storel_pi((__m64*)(m.E+ 8), m3);
	_mm_storeh_pi((__m64*)(m.E+10), m3);
	m4 = _mm_mul_ps(de, m4);
	_mm_storel_pi((__m64*)(m.E+12), m4);
	_mm_storeh_pi((__m64*)(m.E+14), m4);
	return true;

	#else
	tMat4 s;
	tSet(s, m);
	return tInvert(m, s);

	#endif
}


bool tMath::tInvert(tMat4& d, const tMat4& s)
{
	#if defined(PLATFORM_WINDOWS)
	tSet(d, s);
	return tInvert(d);
	
	#else
	tMat4 t;
	tTranspose(t, s);
	float prd[12];

	// Calculate intermediate products for the first set of 8 cofactors.
	prd[ 0] = t.E[10]*t.E[15];	prd[ 1] = t.E[11]*t.E[14];	prd[ 2] = t.E[ 9]*t.E[15];	prd[ 3] = t.E[11]*t.E[13];
	prd[ 4] = t.E[ 9]*t.E[14];	prd[ 5] = t.E[10]*t.E[13];	prd[ 6] = t.E[ 8]*t.E[15];	prd[ 7] = t.E[11]*t.E[12];
	prd[ 8] = t.E[ 8]*t.E[14];	prd[ 9] = t.E[10]*t.E[12];	prd[10] = t.E[ 8]*t.E[13];	prd[11] = t.E[ 9]*t.E[12];

	// Calculate first set of 8 cofactors.
	d.E[ 0] = (prd[ 0]*t.E[ 5] + prd[ 3]*t.E[ 6] + prd[ 4]*t.E[ 7]) - (prd[ 1]*t.E[ 5] + prd[ 2]*t.E[ 6] + prd[ 5]*t.E[ 7]);
	d.E[ 1] = (prd[ 1]*t.E[ 4] + prd[ 6]*t.E[ 6] + prd[ 9]*t.E[ 7]) - (prd[ 0]*t.E[ 4] + prd[ 7]*t.E[ 6] + prd[ 8]*t.E[ 7]);
	d.E[ 2] = (prd[ 2]*t.E[ 4] + prd[ 7]*t.E[ 5] + prd[10]*t.E[ 7]) - (prd[ 3]*t.E[ 4] + prd[ 6]*t.E[ 5] + prd[11]*t.E[ 7]);
	d.E[ 3] = (prd[ 5]*t.E[ 4] + prd[ 8]*t.E[ 5] + prd[11]*t.E[ 6]) - (prd[ 4]*t.E[ 4] + prd[ 9]*t.E[ 5] + prd[10]*t.E[ 6]);
	d.E[ 4] = (prd[ 1]*t.E[ 1] + prd[ 2]*t.E[ 2] + prd[ 5]*t.E[ 3]) - (prd[ 0]*t.E[ 1] + prd[ 3]*t.E[ 2] + prd[ 4]*t.E[ 3]);
	d.E[ 5] = (prd[ 0]*t.E[ 0] + prd[ 7]*t.E[ 2] + prd[ 8]*t.E[ 3]) - (prd[ 1]*t.E[ 0] + prd[ 6]*t.E[ 2] + prd[ 9]*t.E[ 3]);
	d.E[ 6] = (prd[ 3]*t.E[ 0] + prd[ 6]*t.E[ 1] + prd[11]*t.E[ 3]) - (prd[ 2]*t.E[ 0] + prd[ 7]*t.E[ 1] + prd[10]*t.E[ 3]);
	d.E[ 7] = (prd[ 4]*t.E[ 0] + prd[ 9]*t.E[ 1] + prd[10]*t.E[ 2]) - (prd[ 5]*t.E[ 0] + prd[ 8]*t.E[ 1] + prd[11]*t.E[ 2]);

	// Calculate intermediate products for the second set of 8 cofactors.
	prd[ 0] = t.E[ 2]*t.E[ 7];	prd[ 1] = t.E[ 3]*t.E[ 6];	prd[ 2] = t.E[ 1]*t.E[ 7];	prd[ 3] = t.E[ 3]*t.E[ 5];
	prd[ 4] = t.E[ 1]*t.E[ 6];	prd[ 5] = t.E[ 2]*t.E[ 5];	prd[ 6] = t.E[ 0]*t.E[ 7];	prd[ 7] = t.E[ 3]*t.E[ 4];
	prd[ 8] = t.E[ 0]*t.E[ 6];	prd[ 9] = t.E[ 2]*t.E[ 4];	prd[10] = t.E[ 0]*t.E[ 5];	prd[11] = t.E[ 1]*t.E[ 4];

	// Calculate second set of 8 cofactors.
	d.E[ 8] = (prd[ 0]*t.E[13] + prd[ 3]*t.E[14] + prd[ 4]*t.E[15]) - (prd[ 1]*t.E[13] + prd[ 2]*t.E[14] + prd[ 5]*t.E[15]);
	d.E[ 9] = (prd[ 1]*t.E[12] + prd[ 6]*t.E[14] + prd[ 9]*t.E[15]) - (prd[ 0]*t.E[12] + prd[ 7]*t.E[14] + prd[ 8]*t.E[15]);
	d.E[10] = (prd[ 2]*t.E[12] + prd[ 7]*t.E[13] + prd[10]*t.E[15]) - (prd[ 3]*t.E[12] + prd[ 6]*t.E[13] + prd[11]*t.E[15]);
	d.E[11] = (prd[ 5]*t.E[12] + prd[ 8]*t.E[13] + prd[11]*t.E[14]) - (prd[ 4]*t.E[12] + prd[ 9]*t.E[13] + prd[10]*t.E[14]);
	d.E[12] = (prd[ 2]*t.E[10] + prd[ 5]*t.E[11] + prd[ 1]*t.E[ 9]) - (prd[ 4]*t.E[11] + prd[ 0]*t.E[ 9] + prd[ 3]*t.E[10]);
	d.E[13] = (prd[ 8]*t.E[11] + prd[ 0]*t.E[ 8] + prd[ 7]*t.E[10]) - (prd[ 6]*t.E[10] + prd[ 9]*t.E[11] + prd[ 1]*t.E[ 8]);
	d.E[14] = (prd[ 6]*t.E[ 9] + prd[11]*t.E[11] + prd[ 3]*t.E[ 8]) - (prd[10]*t.E[11] + prd[ 2]*t.E[ 8] + prd[ 7]*t.E[ 9]);
	d.E[15] = (prd[10]*t.E[10] + prd[ 4]*t.E[ 8] + prd[ 9]*t.E[ 9]) - (prd[ 8]*t.E[ 9] + prd[11]*t.E[10] + prd[ 5]*t.E[ 8]);

	// Calculate determinant.
	float det = t.E[0]*d.E[0] + t.E[1]*d.E[1] + t.E[2]*d.E[2] + t.E[3]*d.E[3];
	if (det == 0.0f)
		return false;

	// Calculate inverse by multiplying by reciprocal of determinant.
	tDiv(d, det);
	#endif

	return true;
}


void tMath::tInvertAffine(tMat4& d, const tMat4& m)
{
	// Let A be a 4x4 affine matrix where R is a 3x3 orthonormal matrix (the rotation part), T is the 3x1 translation,
	// and the row 4 of A is U, where U = [ 0 0 0 1 ].
	// A =		[ R T ]
	//			[ U   ]
	//
	// then the inverse is given by
	// Inv(A) =	[ Inv(R)   -Inv(R)*T ]
	//			[ U                  ]
	d.a11 = m.a11; d.a21 = m.a12; d.a31 = m.a13;
	d.a12 = m.a21; d.a22 = m.a22; d.a32 = m.a23;
	d.a13 = m.a31; d.a23 = m.a32; d.a33 = m.a33;

	d.C4.x = -m.C1.x*m.C4.x + -m.C1.y*m.C4.y + -m.C1.z*m.C4.z;
	d.C4.y = -m.C2.x*m.C4.x + -m.C2.y*m.C4.y + -m.C2.z*m.C4.z;
	d.C4.z = -m.C3.x*m.C4.x + -m.C3.y*m.C4.y + -m.C3.z*m.C4.z;
}


void tMath::tMakeLookAt(tMat4& d, const tVec3& eye, const tVec3& look, const tVec3& up)
{
	tVec3 z;
	tSub(z, eye, look);
	tNormalize(z);

	tVec3 y(up);
	tVec3 x;
	tCross(x, y, z);
	tCross(y, z, x);

	tNormalize(x);
	tNormalize(y);

	tSet(d.C1, x.x, y.x, z.x, 0.0f);
	tSet(d.C2, x.y, y.y, z.y, 0.0f);
	tSet(d.C3, x.z, y.z, z.z, 0.0f);
	tSet(d.C4, 0.0f, 0.0f, 0.0f, 1.0f);

	tVec3 neg;
	tNeg(neg, eye);
	d.E[12] = d.E[0]*neg.x + d.E[4]*neg.y + d.E[ 8]*neg.z;
	d.E[13] = d.E[1]*neg.x + d.E[5]*neg.y + d.E[ 9]*neg.z;
	d.E[14] = d.E[2]*neg.x + d.E[6]*neg.y + d.E[10]*neg.z;
}


void tMath::tMakeRotate(tMat4& d, const tVec3& axis, float angle)
{
	float t = tLengthSq(axis);
	if (t < 0.00001f)
	{
		tIdentity(d);
		return;
	}

	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	float s = tSin(angle);
	float c = tCos(angle);

	t = tRecipSqrtFast(t);
	x *= t; y *= t; z *= t;
	float xx = x*x;		float yy = y*y;		float zz = z*z;
	float xy = x*y;		float yz = y*z;		float zx = z*x;
	float xs = x*s;		float ys = y*s;		float zs = z*s;
	float rc = 1.0f - c;

	// Again, this is correct, even though at first it looks like the transpose.
	// It is being filled out in column major order for efficiency.
	d.a11 = rc*xx + c;		d.a21 = rc*xy + zs;		d.a31 = rc*zx - ys;		d.a41 = 0.0f;
	d.a12 = rc*xy - zs;		d.a22 = rc*yy + c;		d.a32 = rc*yz + xs;		d.a42 = 0.0f;
	d.a13 = rc*zx + ys;		d.a23 = rc*yz - xs;		d.a33 = rc*zz + c;		d.a43 = 0.0f;
	d.a14 = 0.0f;			d.a24 = 0.0f;			d.a34 = 0.0f;			d.a44 = 1.0f;
}


void tMath::tMakeRotate(tMat4& d, const tVec3& a, const tVec3& b)
{
	// Based on "Efficiently building a matrix to rotate one vector to another vector", by Thomas Möller and John F.
	// Hughes, Journal of Graphics Tools, Vol 4, No 4, 1999. The method is also described in "Real-Time Rendering", by
	// Akenine-Möller and Haines, 2nd Edition, Section 3.3.2.
	float e = tDot(a, b);
	tVec3 v; tCross(v, a, b);

	if (tLength(v) < Epsilon)		// Parallel?
	{
		if (e > 0.0f)				// Aligned?
		{
			tIdentity(d);
			return;
		}
		else if (e < 0.0f)
		{
			// Rotate by Pi radians around an axis perpendicular to a.
			float smallest = tMin(a.x, a.y, a.z);

			// Make vector u perpendicular to a.
			tVec3 u;
			if (a.x == smallest)
				tSet(u,  0.0f, -a.z, a.y);
			else if (a.y == smallest)
				tSet(u, -a.z,  0.0f, a.x);
			else
				tSet(u, -a.y,  a.z,  0.0f);

			tMakeRotate(d, u, Pi);
		}
	}

	// Not parallel to each other.
	float h = (1.0f - e) / tDot(v, v);
	d.a11 = e + h*v.x*v.x;		d.a21 = h * v.x*v.y + v.z;	d.a31 = h*v.x*v.z - v.y;	d.a41 = 0.0f;
	d.a12 = h*v.x*v.y - v.z;	d.a22 = e + h*v.y*v.y;		d.a32 = h*v.y*v.z + v.x;	d.a42 = 0.0f;
	d.a13 = h*v.x*v.z + v.y;	d.a23 = h*v.y*v.z - v.x;	d.a33 = e + h*v.z*v.z;		d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeRotateXYZ(tMat4& d, float a, float b, float c)
{
	// An excellent reference can be found here: http://www.songho.ca/opengl/gl_anglestoaxes.html
	float ca = tCos(a);			float sa = tSin(a);
	float cb = tCos(b);			float sb = tSin(b);
	float cc = tCos(c);			float sc = tSin(c);

	// For all these, cos is written before sin, and secondarily, a before b before c.
	d.a11 = cb*cc;				d.a21 = cb*sc;				d.a31 = -sb;				d.a41 = 0.0f;
	d.a12 = cc*sa*sb - sc*ca;	d.a22 = ca*cc + sa*sb*sc;	d.a32 = cb*sa;				d.a42 = 0.0f;
	d.a13 = sa*sc + ca*cc*sb;	d.a23 = ca*sb*sc - cc*sa;	d.a33 = ca*cb;				d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeRotateYZX(tMat4& d, float a, float b, float c)
{
	float ca = tCos(a);			float sa = tSin(a);
	float cb = tCos(b);			float sb = tSin(b);
	float cc = tCos(c);			float sc = tSin(c);

	d.a11 = cb*cc;				d.a21 = ca*cb*sc + sa*sb;	d.a31 = cb*sa*sc - ca*sb;	d.a41 = 0.0f;
	d.a12 = -sc;				d.a22 = ca*cc;				d.a32 = cc*sa;				d.a42 = 0.0f;
	d.a13 = cc*sb;				d.a23 = ca*sb*sc - cb*sa;	d.a33 = sa*sb*sc + ca*cb;	d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeRotateZXY(tMat4& d, float a, float b, float c)
{
	float ca = tCos(a);			float sa = tSin(a);
	float cb = tCos(b);			float sb = tSin(b);
	float cc = tCos(c);			float sc = tSin(c);

	d.a11 = cb*cc + sa*sb*sc;	d.a21 = ca*sc;				d.a31 = cb*sa*sc - cc*sb;	d.a41 = 0.0f;
	d.a12 = cc*sa*sb - cb*sc;	d.a22 = ca*cc;				d.a32 = sb*sc + cb*cc*sa;	d.a42 = 0.0f;
	d.a13 = ca*sb;				d.a23 = -sa;				d.a33 = ca*cb;				d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeRotateZYX(tMat4& d, float a, float b, float c)
{
	float ca = tCos(a);			float sa = tSin(a);
	float cb = tCos(b);			float sb = tSin(b);
	float cc = tCos(c);			float sc = tSin(c);

	d.a11 = cb*cc;				d.a21 = cc*sa*sb + ca*sc;	d.a31 = sa*sc - ca*cc*sb;	d.a41 = 0.0f;
	d.a12 = -cb*sc;				d.a22 = ca*cc - sa*sb*sc;	d.a32 = ca*sb*sc + cc*sa;	d.a42 = 0.0f;
	d.a13 = sb;					d.a23 = -cb*sa;				d.a33 = ca*cb;				d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeRotateXZY(tMat4& d, float a, float b, float c)
{
	float ca = tCos(a);			float sa = tSin(a);
	float cb = tCos(b);			float sb = tSin(b);
	float cc = tCos(c);			float sc = tSin(c);

	// Note there is (was?) a typo for a12 at the site: http://www.songho.ca/opengl/gl_anglestoaxes.html
	// I posted a correction. It may or may not have been updated. In any case, a12 is correct below.
	d.a11 = cb*cc;				d.a21 = sc;					d.a31 = -cc*sb;				d.a41 = 0.0f;
	d.a12 = sa*sb - ca*cb*sc;	d.a22 = ca*cc;				d.a32 = ca*sb*sc + cb*sa;	d.a42 = 0.0f;
	d.a13 = cb*sa*sc + ca*sb;	d.a23 = -cc*sa;				d.a33 = ca*cb - sa*sb*sc;	d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeRotateYXZ(tMat4& d, float a, float b, float c)
{
	float ca = tCos(a);			float sa = tSin(a);
	float cb = tCos(b);			float sb = tSin(b);
	float cc = tCos(c);			float sc = tSin(c);

	d.a11 = cb*cc - sa*sb*sc;	d.a21 = cb*sc + cc*sa*sb;	d.a31 = -ca*sb;				d.a41 = 0.0f;
	d.a12 = -ca*sc;				d.a22 = ca*cc;				d.a32 = sa;					d.a42 = 0.0f;
	d.a13 = cc*sb + cb*sa*sc;	d.a23 = sb*sc - cb*cc*sa;	d.a33 = ca*cb;				d.a43 = 0.0f;
	d.a14 = 0.0f;				d.a24 = 0.0f;				d.a34 = 0.0f;				d.a44 = 1.0f;
}


void tMath::tMakeProjPersp(tMat4& d, float left, float right, float bottom, float top, float near, float far)
{
	// Generates:	2n/(r-l)	0			(r+l)/(r-l)		0	
	//				0			2n/(t-b)	(t+b)/(t-b)		0
	//				0			0			-(f+n)/(f-n)	-2fn/(f-n)
	//				0			0			-1				0
	tZero(d);
	float lr = right-left;		float bt = top-bottom;
	float nf = far-near;		float n2 = 2.0f*near;

	d.a11 = n2 / lr;
	d.a22 = n2 / bt;
	d.a13 = (right+left) / lr;
	d.a23 = (top+bottom) / bt;
	d.a33 = -(far+near) / nf;
	d.a43 = -1.0f;
	d.a34 = -n2*far / nf;
}


void tMath::tMakeProjPersp(tMat4& d, const tVec3& boxMin, const tVec3& boxMax)
{
	float left = boxMin.x;
	float right = boxMax.x;
	float bottom = boxMin.y;
	float top = boxMax.y;
	float nearPlane = -boxMax.z;
	float farPlane = -boxMin.z;
	tMakeProjPersp(d, left, right, bottom, top, nearPlane, farPlane);
}


void tMath::tMakeProjPerspSym(tMat4& d, float right, float top, float nearPlane, float farPlane)
{
	// Symmetry allows a faster implementation since left = -right and bottom = -top.
	// Generates: 	n/r		0		0				0	
	//				0		n/t		0				0
	//				0		0		-(f+n)/(f-n)	-2fn/(f-n)
	//				0		0		-1				0
	tZero(d);
	d.a11 = nearPlane/right;
	d.a22 = nearPlane/top;
	d.a33 = -(farPlane+nearPlane)/(farPlane-nearPlane);
	d.a34 = -2.0f*farPlane*nearPlane/(farPlane-nearPlane);
	d.a43 = -1.0f;
}


void tMath::tMakeProjPerspSymFovV(tMat4& d, float fovX, float aspect, float nearPlane, float farPlane)
{
	tAssert(aspect > 0.0f);
	float top = nearPlane*tTan(fovX/2.0f);
	float right = top*aspect;
	tMakeProjPerspSym(d, right, top, nearPlane, farPlane);
}


void tMath::tMakeProjPerspSymFovH(tMat4& d, float fovY, float aspect, float nearPlane, float farPlane)
{
	tAssert(aspect > 0.0f);
	float right = nearPlane*tTan(fovY/2.0f);
	float top = right/aspect;
	tMakeProjPerspSym(d, right, top, nearPlane, farPlane);
}


void tMath::tMakeProjPerspOffset(tMat4& d, float right, float top, float nearPlane, float farPlane, float offsetX, float offsetY)
{
	float offsetH = -offsetX*right;
	float offsetV = -offsetY*top;
	tMakeProjPersp(d, offsetH-right, offsetH+right, offsetV-top, offsetV+top, nearPlane, farPlane);
}


void tMath::tMakeProjPerspFovVOffset(tMat4& d, float fovX, float aspect, float nearPlane, float farPlane, float offsetX, float offsetY)
{
	tAssert(aspect > 0.0f);
	float top = nearPlane*tTan(fovX/2.0f);
	float right = top*aspect;

	float offsetH = -offsetX*right;
	float offsetV = -offsetY*top;
	tMakeProjPersp(d, offsetH-right, offsetH+right, offsetV-top, offsetV+top, nearPlane, farPlane);
}


void tMath::tMakeProjPerspFovHOffset(tMat4& d, float fovY, float aspect, float nearPlane, float farPlane, float offsetX, float offsetY)
{
	tAssert(aspect > 0.0f);
	float right = nearPlane*tTan(fovY/2.0f);
	float top = right/aspect;

	float offsetH = -offsetX*right;
	float offsetV = -offsetY*top;
	tMakeProjPersp(d, offsetH-right, offsetH+right, offsetV-top, offsetV+top, nearPlane, farPlane);
}


void tMath::tMakeProjParallel(tMat4& d, const tVec3& boxMin, const tVec3& boxMax)
{
	tZero(d);
	d.a11 = 2.0f / (boxMax.x-boxMin.x);
	d.a14 = -(boxMax.x+boxMin.x) / (boxMax.x-boxMin.x);
	d.a22 = 2.0f / (boxMax.y-boxMin.y);
	d.a24 = -(boxMax.y+boxMin.y) / (boxMax.y-boxMin.y);
	d.a33 = -2.0f / (boxMax.z-boxMin.z);
	d.a34 = -(boxMax.z+boxMin.z) / (boxMax.z-boxMin.z);
	d.a44 = 1.0f;
}


void tMath::tMakeOrthoNormal(tMat4& m)
{
	tVec3 c1;
	tSet(c1, m.C1);
	tNormalize(c1);

	tVec3 t2;
	tSet(t2, m.C2);

	tVec3 c3;
	tCross(c3, c1, t2);
	tNormalize(c3);

	tVec3 c2;
	tCross(c2, c3, c1);

	tSet(m.C1, c1);
	tSet(m.C2, c2);
	tSet(m.C3, c3);

	m.C1.w = m.C2.w = m.C3.w = 0.0f; m.C4.w = 1.0f;
}


void tMath::tMakeOrthoUniform(tMat4& m)
{
	tVec3 c1;
	tSet(c1, m.C1);
	float scale = tLength(c1);		// We use the length of the first column for all columns.

	tVec3 c2;
	tSet(c2, m.C2);

	tVec3 c3;
	tCross(c3, c1, c2);
	tNormalizeScale(c3, scale);
	tCross(c2, c3, c1);
	tNormalizeScale(c2, scale);

	tSet(m.C2, c2);
	tSet(m.C3, c3);
	m.C1.w = m.C2.w = m.C3.w = 0.0f; m.C4.w = 1.0f;
}


void tMath::tMakeOrthoNonUniform(tMat4& m)
{
	tVec3 c1;
	tSet(c1, m.C1);

	tVec3 c2;
	tSet(c2, m.C2);
	float scale2 = tLength(c2);

	m.C3.w = 0.0f;
	float scale3 = tLength(m.C3);

	tVec3 c3;
	tCross(c3, c1, c2);
	tNormalizeScale(c3, scale3);

	tCross(c2, c3, c1);
	tNormalizeScale(c2, scale2);

	tSet(m.C2, c2);
	tSet(m.C3, c3);
	m.C1.w = m.C2.w = 0.0f; m.C4.w = 1.0f;
}


void tMath::tExtractProjectionPlanes(tVec4 planes[6], const tMat4& m, bool outwardNormals, bool normalizePlanes)
{
	// Order is: Right, Left, Top, Bottom, Near, Far.
	tSet(planes[0], m.C1.d - m.C1.a, m.C2.d - m.C2.a, m.C3.d - m.C3.a, m.C4.d - m.C4.a );
	tSet(planes[1], m.C1.d + m.C1.a, m.C2.d + m.C2.a, m.C3.d + m.C3.a, m.C4.d + m.C4.a );
	tSet(planes[2], m.C1.d - m.C1.b, m.C2.d - m.C2.b, m.C3.d - m.C3.b, m.C4.d - m.C4.b );
	tSet(planes[3], m.C1.d + m.C1.b, m.C2.d + m.C2.b, m.C3.d + m.C3.b, m.C4.d + m.C4.b );
	tSet(planes[4], m.C1.d + m.C1.c, m.C2.d + m.C2.c, m.C3.d + m.C3.c, m.C4.d + m.C4.c );
	tSet(planes[5], m.C1.d - m.C1.c, m.C2.d - m.C2.c, m.C3.d - m.C3.c, m.C4.d - m.C4.c );

	if (outwardNormals)
	{
		for (int p = 0; p < 6; p++)
			tNeg(planes[p]);
	}

	if (normalizePlanes)
	{
		for (int p = 0; p < 6; p++)
		{
			tVec3 point;
			tSet(point, planes[p]);
			float length = tNormalizeGetLength(point);
			tDiv(planes[p], length);
		}
	}
}


bool tMath::tExtractRotationEulerXYZ
(
	tVec3& sol1, tVec3& sol2, const tMat4& rot,
	float gimbalZ, tIntervalBias bias
)
{
	// Extraction based on http://staff.city.ac.uk/~sbbh653/publications/euler.pdf
	float gimbalEpsilon = 0.000001f;
	bool gimbalNeg = tApproxEqual(rot.a31, -1.0f, gimbalEpsilon);
	bool gimbalPos = tApproxEqual(rot.a31,  1.0f, gimbalEpsilon);
	if (!gimbalNeg && !gimbalPos)
	{
		sol1.y = -tArcSin(rot.a31);							// [-Pi/2, Pi/2]
		sol2.y = tNormalizedAngle( Pi - sol1.y, bias );

		float ct1 = tCos(sol1.y);							// Cos(theta1)
		float ct2 = tCos(sol2.y);							// Cos(theta2)
		sol1.x = tNormalizedAngle( tArcTan(rot.a32/ct1, rot.a33/ct1), bias );
		sol2.x = tNormalizedAngle( tArcTan(rot.a32/ct2, rot.a33/ct2), bias );
		sol1.z = tNormalizedAngle( tArcTan(rot.a21/ct1, rot.a11/ct1), bias );
		sol2.z = tNormalizedAngle( tArcTan(rot.a21/ct2, rot.a11/ct2), bias );
	}
	else
	{
		// User choice of particular solution returned.
		sol1.z = gimbalZ;
		if (gimbalNeg)
		{
			sol1.y = PiOver2;
			sol1.x = tNormalizedAngle(sol1.z + tArcTan(rot.a12, rot.a13), bias);
		}
		else
		{
			sol1.y = -PiOver2;
			sol1.x = tNormalizedAngle(-sol1.z + tArcTan(-rot.a12, -rot.a13), bias);
		}
		sol2 = sol1;
	}

	return gimbalNeg || gimbalPos;
}


bool tMath::tExtractAffineEulerXYZ(tVec3& sol1, tVec3& sol2, const tMat4& affine, float gimbalZ, tIntervalBias bias)
{
	tMat4 r; tSet(r, affine);
	tNormalize(r.C1);
	tNormalize(r.C2);
	tNormalize(r.C3);
	return tExtractRotationEulerXYZ(sol1, sol2, r, gimbalZ, bias);
}


bool tMath::tApproachAngle(float& angle, float dest, float rate, float dt)
{
	float diff = dest - angle;
	while (diff > Pi)
		diff -= TwoPi;

	while (diff < -Pi)
		diff += TwoPi;

	float delta = rate*dt;
	if (tAbs(diff) <= delta)
	{
		angle = dest;
		return true;
	}

	angle += tSign(diff)*delta;
	return false;
}


bool tMath::tApproachOrientation(tQuat& curr, const tQuat& dest, float rate, float dt)
{
	// Here is the quaternion that will rotate from curr to dest.  q' = q1^-1 * q2.
	tQuat rot;
	tConjugate(rot, curr);
	tMul(rot, dest);

	// Angle is in [0,Pi]. It will be unit length.
	float angle;
	tVec3 axis;
	tGet(axis, angle, rot);
	if (angle <= (rate*dt))
	{
		curr = dest;
		return true;
	}

	tQuat diff;
	tSet(diff, axis, rate*dt);
	tMul(curr, diff);
	tNormalize(curr);
	return false;
}
