// tLinearAlgebra.h
//
// POD types for Vectors, Matrices and a function library to manipulate them. Includes geometrical transformations,
// cross/dot products, inversion functions, projections, normalization etc. These POD types are used as superclasses
// for the more object-oriented and complete derived types. eg. tVector3 derives from the POD type tVec2 found here.
//
// Copyright (c) 2004-2006, 2015, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tStandard.h>
#include "Math/tFundamentals.h"
namespace tMath
{


// These can be ORed together if necessary and are generally passed around using the tComponents type.
enum tComponent
{
	tComponent_X																										= 0x00000001,
	tComponent_Y																										= 0x00000002,
	tComponent_Z																										= 0x00000004,
	tComponent_W																										= 0x00000008,
	tComponent_XY																										= tComponent_X | tComponent_Y,
	tComponent_YZ																										= tComponent_Y | tComponent_Z,
	tComponent_ZX																										= tComponent_Z | tComponent_X,
	tComponent_XYZ																										= tComponent_X | tComponent_Y | tComponent_Z,
	tComponent_XYZW																										= tComponent_X | tComponent_Y | tComponent_Z | tComponent_W,

	// For matrices the notation Amn means the component at row m and column n (like in linear algebra).
	tComponent_A11																										= 0x00000010,
	tComponent_A21																										= 0x00000020,
	tComponent_A31																										= 0x00000040,
	tComponent_A41																										= 0x00000080,
	tComponent_A12																										= 0x00000100,
	tComponent_A22																										= 0x00000200,
	tComponent_A32																										= 0x00000400,
	tComponent_A42																										= 0x00000800,
	tComponent_A13																										= 0x00001000,
	tComponent_A23																										= 0x00002000,
	tComponent_A33																										= 0x00004000,
	tComponent_A43																										= 0x00008000,
	tComponent_A14																										= 0x00010000,
	tComponent_A24																										= 0x00020000,
	tComponent_A34																										= 0x00040000,
	tComponent_A44																										= 0x00080000,
	tComponent_C1																										= tComponent_A11 | tComponent_A21 | tComponent_A31 | tComponent_A41,
	tComponent_C2																										= tComponent_A12 | tComponent_A22 | tComponent_A32 | tComponent_A42,
	tComponent_C3																										= tComponent_A13 | tComponent_A23 | tComponent_A33 | tComponent_A43,
	tComponent_C4																										= tComponent_A14 | tComponent_A24 | tComponent_A34 | tComponent_A44,
	tComponent_R1																										= tComponent_A11 | tComponent_A12 | tComponent_A13 | tComponent_A14,
	tComponent_R2																										= tComponent_A21 | tComponent_A22 | tComponent_A23 | tComponent_A24,
	tComponent_R3																										= tComponent_A31 | tComponent_A32 | tComponent_A33 | tComponent_A34,
	tComponent_R4																										= tComponent_A41 | tComponent_A42 | tComponent_A43 | tComponent_A44,

	tComponent_All																										= 0xFFFFFFFF
};


// Since it is awkward to do bitwise operations on the enum and enum class types we simply use a uint32 for components.
typedef uint32 tComponents;


// Note that points are not implemented separately since in linear algebra "A vector is a point is a vector". At the
// very least they have the same behaviour, even if they are conceptually different. All vectors are column vectors.
struct tVec2
{
	union
	{
		struct { float x, y; };
		struct { float a, b; };
		float E[2];											// E for elements.
		uint64 B;											// B for bits.
	};
};


struct tVec3
{
#pragma pack(push, 4)
	union
	{
		struct { float x, y, z; };
		struct { float a, b, c; };
		float E[3];
		struct { uint64 B0; uint32 B1; };
	};
#pragma pack(pop)
};


struct tVec4
{
	union
	{
		struct { float x, y, z, w; };
		struct { float a, b, c, d; };
		float E[4];
		struct { uint64 B0, B1; };
	};
};


struct tQuat
{
	union
	{
		struct { float x, y, z, w; };						// W comes last unlike white-papers on quaternions.
		struct { tVec3 v; float r; };
		float E[4];
		struct { uint64 B0, B1; };
	};
};


struct tMat2
{
	union
	{
		struct
		{
			float a11, a21;									// Convention is A_rc where r = row and c = col.
			float a12, a22;
		};
		struct { tVec2 C1, C2; };							// C for column.
		float A[2][2];										// A for array.
		float E[4];
		struct { uint64 B0, B1; };
	};
};


// tMat4 is a 4x4 (16 dimensional) homogeneous matrix plain-old-data type. Components are stored inmemory column major.
//
// For example, the translation matrix:
// 1 0 0 tx
// 0 1 0 ty
// 0 0 1 tz
// 0 0 0 1
// is stored in memory in memory as 1 0 0 0 0 1 0 0 0 0 1 0 tx ty tz 1 with lower addresses on the left. Column 4 (C4)
// is therefore the column vector (tx, ty, tz, 1).
struct tMat4
{
	union
	{
		struct
		{
			float a11, a21, a31, a41;
			float a12, a22, a32, a42;
			float a13, a23, a33, a43;
			float a14, a24, a34, a44;
		};

		struct { tVec4 C1, C2, C3, C4; };
		tVec4 C[4];
		float A[4][4];
		float E[16];
		struct { uint64 B0, B1, B2, B3, B4, B5, B6, B7; };
	};
};


// For the functions below we generally use d for destination and s for source. When a non-const v, m, or q is used, the
// parameter is both input and output. s should be different to d for overloads that use them.
inline void tSet(tVec2& d, const tVec2& s)																				{ d.B = s.B; }
inline void tSet(tVec2& d, const tVec3& s)																				{ d.x = s.x; d.y = s.y; }
inline void tSet(tVec2& d, const tVec4& s)																				{ d.x = s.x; d.y = s.y; }
inline void tSet(tVec2& d, float xy)																					{ d.x = xy; d.y = xy; }
inline void tSet(tVec2& d, float x, float y)																			{ d.x = x; d.y = y; }
inline void tSet(tVec2& d, const float* a)																				{ d.x = a[0]; d.y = a[1]; }
inline void tSet(tVec3& d, const tVec2& s, float z = 0.0f)																{ d.x = s.x; d.y = s.y; d.z = z; }
inline void tSet(tVec3& d, const tVec3& s)																				{ d.B0 = s.B0; d.B1 = s.B1; }
inline void tSet(tVec3& d, const tVec4& s)																				{ d.x = s.x; d.y = s.y; d.z = s.z; }
inline void tSet(tVec3& d, float xyz)																					{ d.x = xyz; d.y = xyz; d.z = xyz; }
inline void tSet(tVec3& d, float x, float y, float z)																	{ d.x = x; d.y = y; d.z = z; }
inline void tSet(tVec3& d, const float* a)																				{ d.x = a[0]; d.y = a[1]; d.z = a[2]; }
inline void tSet(tVec4& d, const tVec2& s, float z = 0.0f, float w = 0.0f)												{ d.B0 = s.B; d.z = z; d.w = w; }
inline void tSet(tVec4& d, const tVec3& s, float w = 0.0f)																{ d.B0 = s.B0; d.z = s.z; d.w = w; }
inline void tSet(tVec4& d, const tVec4& s)																				{ d.B0 = s.B0; d.B1 = s.B1; }
inline void tSet(tVec4& d, float xyzw)																					{ d.x = xyzw; d.y = xyzw; d.z = xyzw; d.w = xyzw; }
inline void tSet(tVec4& d, float x, float y, float z, float w)															{ d.x = x; d.y = y; d.z = z; d.w = w; }
inline void tSet(tVec4& d, const float* a)																				{ d.x = a[0]; d.y = a[1]; d.z = a[2]; d.w = a[3]; }
inline void tSet(tQuat& d, const tQuat& s)																				{ d.B0 = s.B0; d.B1 = s.B1; }
inline void tSet(tQuat& d, float x, float y, float z, float w)															{ d.x = x; d.y = y; d.z = z; d.w = w; }
inline void tSet(tQuat& d, const float* a)																				{ d.x = a[0]; d.y = a[1]; d.z = a[2]; d.w = a[3]; }
       void tSet(tQuat& d, const tMat4&);							// Creates a unit quaternion from a rotation matrix.
inline void tSet(tQuat& d, float r, const tVec3& v)			/* Not axis-angle. */										{ d.v = v; d.r = r; }
       void tSet(tQuat& d, const tVec3& axis, float angle);	// Assumes axis is normalized.
inline void tSet(tQuat& d, const tVec3& v)																				{ d.v = v; d.r = 0.0f; }
inline void tSet(tMat2& d, const tMat2& s)																				{ d.B0 = s.B0; d.B1 = s.B1; }
inline void tSet(tMat2& d, const tVec2& c1, const tVec2& c2)															{ d.C1 = c1; d.C2 = c2; }
inline void tSet(tMat2& d, float a11, float a21, float a12, float a22)													{ d.a11 = a11; d.a21 = a21; d.a12 = a12; d.a22 = a22; }
inline void tSet(tMat2& d, const float* a)																				{ d.a11 = a[0]; d.a21 = a[1]; d.a12 = a[2]; d.a22 = a[3]; }
inline void tSet(tMat4& d, const tMat4& s)																				{ for (int e = 0; e < 16; e++) d.E[e] = s.E[e]; }
inline void tSet(tMat4& d, const tVec4& c1, const tVec4& c2, const tVec4& c3, const tVec4& c4)							{ d.C1 = c1; d.C2 = c2; d.C3 = c3; d.C4 = c4; }
inline void tSet(tMat4& d, const tVec3& c1, const tVec3& c2, const tVec3& c3, const tVec3& c4)							{ tSet(d.C1, c1, 0.0f); tSet(d.C2, c2, 0.0f); tSet(d.C3, c3, 0.0f); tSet(d.C4, c4, 1.0f); }
inline void tSet
(
	tMat4& d,
	float a11, float a21, float a31, float a41,
	float a12, float a22, float a32, float a42,
	float a13, float a23, float a33, float a43,
	float a14, float a24, float a34, float a44
);
inline void tSet(tMat4& d, const float* a)																				{ for (int e = 0; e < 16; e++) d.E[e] = a[e]; }
       void tSet(tMat4& d, const tQuat& s);

// Similar to the tSet functions except generally used only for decomposing the pod type rather than conversions.
inline void tGet(float& x, float& y, const tVec2& s)																	{ x = s.x; y = s.y; }
inline void tGet(float* a, const tVec2& s)																				{ a[0] = s.x; a[1] = s.y; }
inline void tGet(float& x, float& y, float& z, const tVec3& s)															{ x = s.x; y = s.y; z = s.z; }
inline void tGet(float* a, const tVec3& s)																				{ a[0] = s.x; a[1] = s.y; a[2] = s.z; }
inline void tGet(float& x, float& y, float& z, float& w, const tVec4& s)												{ x = s.x; y = s.y; z = s.z; w = s.w; }
inline void tGet(float* a, const tVec4& s)																				{ a[0] = s.x; a[1] = s.y; a[2] = s.z; a[3] = s.w; }
inline void tGet(float& x, float& y, float& z, float& w, const tQuat& s)												{ x = s.x; y = s.y; z = s.z; w = s.w; }
inline void tGet(float* a, const tQuat& s)																				{ a[0] = s.x; a[1] = s.y; a[2] = s.z; a[3] = s.w; }
       void tGet(tVec3& axis, float& angle, const tQuat&);			// Angle always returned E [0,Pi].
inline void tGet(float& a11, float& a21, float& a12, float& a22, const tMat2& s)										{ a11 = s.a11; a21 = s.a21; a12 = s.a12; a22 = s.a22; }
inline void tGet(float* a, const tMat2& m)																				{ a[0] = m.E[0]; a[1] = m.E[1]; a[2] = m.E[2]; a[3] = m.E[3]; }
inline void tGet
(
	float& a11, float& a21, float& a31, float& a41,
	float& a12, float& a22, float& a32, float& a42,
	float& a13, float& a23, float& a33, float& a43,
	float& a14, float& a24, float& a34, float& a44,
	const tMat4& s
);
inline void tGet(float* a, const tMat4& m)																				{ for (int e = 0; e < 16; e++) a[e] = m.E[e]; }

inline void tZero(tVec2& d)																								{ d.B = 0; }
inline void tZero(tVec2& d, tComponents c)																				{ if (c & tComponent_X) d.x = 0.0f; if (c & tComponent_Y) d.y = 0.0f; }
inline void tZero(tVec3& d)																								{ d.B0 = 0; d.B1 = 0; }
inline void tZero(tVec3& d, tComponents c)																				{ if (c & tComponent_X) d.x = 0.0f; if (c & tComponent_Y) d.y = 0.0f; if (c & tComponent_Z) d.z = 0.0f; }
inline void tZero(tVec4& d)																								{ d.B0 = 0; d.B1 = 0; }
inline void tZero(tVec4& d, tComponents c)																				{ if (c & tComponent_X) d.x = 0.0f; if (c & tComponent_Y) d.y = 0.0f; if (c & tComponent_Z) d.z = 0.0f; if (c & tComponent_W) d.w = 0.0f; }
inline void tZero(tQuat& d)																								{ d.B0 = 0; d.B1 = 0; }
inline void tZero(tQuat& d, tComponents c)																				{ if (c & tComponent_X) d.x = 0.0f; if (c & tComponent_Y) d.y = 0.0f; if (c & tComponent_Z) d.z = 0.0f; if (c & tComponent_W) d.w = 0.0f; }
inline void tZero(tMat2& d)																								{ d.B0 = 0; d.B1 = 0; }
inline void tZero(tMat2& d, tComponents c)																				{ if (c & tComponent_A11) d.a11 = 0.0f; if (c & tComponent_A21) d.a21 = 0.0f; if (c & tComponent_A12) d.a12 = 0.0f; if (c & tComponent_A22) d.a22 = 0.0f; }
inline void tZero(tMat4& d)																								{ for (int e = 0; e < 16; e++) d.E[e] = 0.0f; }
inline void tZero(tMat4& d, tComponents c);

inline bool tIsZero(const tVec2& v)																						{ return v.B == 0; }
inline bool tIsZero(const tVec2& v, tComponents c)																		{ return (!(c & tComponent_X) || (v.x == 0.0f)) && (!(c & tComponent_Y) || (v.y == 0.0f)); }
inline bool tIsZero(const tVec3& v)																						{ return (v.B0 == 0) && (v.B1 == 0); }
inline bool tIsZero(const tVec3& v, tComponents c)																		{ return (!(c & tComponent_X) || (v.x == 0.0f)) && (!(c & tComponent_Y) || (v.y == 0.0f)) && (!(c & tComponent_Z) || (v.z == 0.0f)); }
inline bool tIsZero(const tVec4& v)																						{ return (v.B0 == 0) && (v.B1 == 0); }
inline bool tIsZero(const tVec4& v, tComponents c)																		{ return (!(c & tComponent_X) || (v.x == 0.0f)) && (!(c & tComponent_Y) || (v.y == 0.0f)) && (!(c & tComponent_Z) || (v.z == 0.0f)) && (!(c & tComponent_W) || (v.w == 0.0f)); }
inline bool tIsZero(const tQuat& q)																						{ return (q.B0 == 0) && (q.B1 == 0); }
inline bool tIsZero(const tQuat& q, tComponents c)																		{ return (!(c & tComponent_X) || (q.x == 0.0f)) && (!(c & tComponent_Y) || (q.y == 0.0f)) && (!(c & tComponent_Z) || (q.z == 0.0f)) && (!(c & tComponent_W) || (q.w == 0.0f)); }
inline bool tIsZero(const tMat2& m)																						{ return (m.B0 == 0) && (m.B1 == 0); }
inline bool tIsZero(const tMat2& m, tComponents c)																		{ return (!(c & tComponent_A11) || (m.a11 == 0.0f)) && (!(c & tComponent_A21) || (m.a21 == 0.0f)) && (!(c & tComponent_A12) || (m.a12 == 0.0f)) && (!(c & tComponent_A22) || (m.a22 == 0.0f)); }
inline bool tIsZero(const tMat4& m)																						{ return (m.B0 == 0) && (m.B1 == 0) && (m.B2 == 0) && (m.B3 == 0) && (m.B4 == 0) && (m.B5 == 0) && (m.B6 == 0) && (m.B7 == 0); }
inline bool tIsZero(const tMat4& m, tComponents c);

// Test for equality within epsilon. Each basis component is tested independently. A relative error metric would behave
// better: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
inline bool tApproxEqual(const tVec2& a, const tVec2& b, float e = Epsilon)												{ return (tAbs(a.x-b.x) < e) && (tAbs(a.y-b.y) < e); }
inline bool tApproxEqual(const tVec2& a, const tVec2& b, tComponents c, float e = Epsilon)								{ return (!(c & tComponent_X) || (tAbs(a.x-b.x) < e)) && (!(c & tComponent_Y) || (tAbs(a.y-b.y) < e)); }
inline bool tApproxEqual(const tVec3& a, const tVec3& b, float e = Epsilon)												{ return (tAbs(a.x-b.x) < e) && (tAbs(a.y-b.y) < e) && (tAbs(a.z-b.z) < e); }
inline bool tApproxEqual(const tVec3& a, const tVec3& b, tComponents c, float e = Epsilon)								{ return (!(c & tComponent_X) || (tAbs(a.x-b.x) < e)) && (!(c & tComponent_Y) || (tAbs(a.y-b.y) < e)) && (!(c & tComponent_Z) || (tAbs(a.z-b.z) < e)); }
inline bool tApproxEqual(const tVec4& a, const tVec4& b, float e = Epsilon)												{ return (tAbs(a.x-b.x) < e) && (tAbs(a.y-b.y) < e) && (tAbs(a.z-b.z) < e) && (tAbs(a.w-b.w) < e); }
inline bool tApproxEqual(const tVec4& a, const tVec4& b, tComponents c, float e = Epsilon)								{ return (!(c & tComponent_X) || (tAbs(a.x-b.x) < e)) && (!(c & tComponent_Y) || (tAbs(a.y-b.y) < e)) && (!(c & tComponent_Z) || (tAbs(a.z-b.z) < e)) && (!(c & tComponent_W) || (tAbs(a.w-b.w) < e)); }
inline bool tApproxEqual(const tQuat& a, const tQuat& b, float e = Epsilon)												{ return (tAbs(a.x-b.x) < e) && (tAbs(a.y-b.y) < e) && (tAbs(a.z-b.z) < e) && (tAbs(a.w-b.w) < e); }
inline bool tApproxEqual(const tQuat& a, const tQuat& b, tComponents c, float e = Epsilon)								{ return (!(c & tComponent_X) || (tAbs(a.x-b.x) < e)) && (!(c & tComponent_Y) || (tAbs(a.y-b.y) < e)) && (!(c & tComponent_Z) || (tAbs(a.z-b.z) < e)) && (!(c & tComponent_W) || (tAbs(a.w-b.w) < e)); }
inline bool tApproxEqual(const tMat2& a, const tMat2& b, float e = Epsilon)												{ return tApproxEqual(a.C1, b.C1, e) && tApproxEqual(a.C2, b.C2, e); }
inline bool tApproxEqual(const tMat2& a, const tMat2& b, tComponents c, float e = Epsilon);
inline bool tApproxEqual(const tMat4& a, const tMat4& b, float e = Epsilon)												{ return tApproxEqual(a.C1, b.C1, e) && tApproxEqual(a.C2, b.C2, e) && tApproxEqual(a.C3, b.C3, e) && tApproxEqual(a.C4, b.C4, e); }
inline bool tApproxEqual(const tMat4& a, const tMat4& b, tComponents c, float e = Epsilon);

inline bool tEqual(const tVec2& a, const tVec2& b)																		{ return a.B == b.B; }
inline bool tEqual(const tVec2& a, const tVec2& b, tComponents c)														{ return (!(c & tComponent_X) || (a.x == b.x)) && (!(c & tComponent_Y) || (a.y == b.y)); }
inline bool tEqual(const tVec3& a, const tVec3& b)																		{ return (a.B0 == b.B0) && (a.B1 == b.B1); }
inline bool tEqual(const tVec3& a, const tVec3& b, tComponents c)														{ return (!(c & tComponent_X) || (a.x == b.x)) && (!(c & tComponent_Y) || (a.y == b.y)) && (!(c & tComponent_Z) || (a.z == b.z)); }
inline bool tEqual(const tVec4& a, const tVec4& b)																		{ return (a.B0 == b.B0) && (a.B1 == b.B1); }
inline bool tEqual(const tVec4& a, const tVec4& b, tComponents c)														{ return (!(c & tComponent_X) || (a.x == b.x)) && (!(c & tComponent_Y) || (a.y == b.y)) && (!(c & tComponent_Z) || (a.z == b.z)) && (!(c & tComponent_W) || (a.w == b.w)); }
inline bool tEqual(const tQuat& a, const tQuat& b)																		{ return (a.B0 == b.B0) && (a.B1 == b.B1); }
inline bool tEqual(const tQuat& a, const tQuat& b, tComponents c)														{ return (!(c & tComponent_X) || (a.x == b.x)) && (!(c & tComponent_Y) || (a.y == b.y)) && (!(c & tComponent_Z) || (a.z == b.z)) && (!(c & tComponent_W) || (a.w == b.w)); }
inline bool tEqual(const tMat2& a, const tMat2& b)																		{ return (a.B0 == b.B0) && (a.B1 == b.B1); }
inline bool tEqual(const tMat2& a, const tMat2& b, tComponents c)														{ return (!(c & tComponent_A11) || (a.a11 == b.a11)) && (!(c & tComponent_A21) || (a.a21 == b.a21)) && (!(c & tComponent_A12) || (a.a12 == b.a12)) && (!(c & tComponent_A22) || (a.a22 == b.a22)); }
inline bool tEqual(const tMat4& a, const tMat4& b)																		{ return (a.B0 == b.B0) && (a.B1 == b.B1) && (a.B2 == b.B2) && (a.B3 == b.B3) && (a.B4 == b.B4) && (a.B5 == b.B5) && (a.B6 == b.B6) && (a.B7 == b.B7); }
inline bool tEqual(const tMat4& a, const tMat4& b, tComponents c);

inline bool tNotEqual(const tVec2& a, const tVec2& b)																	{ return a.B != b.B; }
inline bool tNotEqual(const tVec2& a, const tVec2& b, tComponents c)													{ return ((c & tComponent_X) && (a.x != b.x)) || ((c & tComponent_Y) && (a.y != b.y)); }
inline bool tNotEqual(const tVec3& a, const tVec3& b)																	{ return (a.B0 != b.B0) || (a.B1 != b.B1); }
inline bool tNotEqual(const tVec3& a, const tVec3& b, tComponents c)													{ return ((c & tComponent_X) && (a.x != b.x)) || ((c & tComponent_Y) && (a.y != b.y)) || ((c & tComponent_Z) && (a.z != b.z)); }
inline bool tNotEqual(const tVec4& a, const tVec4& b)																	{ return (a.B0 != b.B0) || (a.B1 != b.B1); }
inline bool tNotEqual(const tVec4& a, const tVec4& b, tComponents c)													{ return ((c & tComponent_X) && (a.x != b.x)) || ((c & tComponent_Y) && (a.y != b.y)) || ((c & tComponent_Z) && (a.z != b.z)) || ((c & tComponent_W) && (a.w != b.w)); }
inline bool tNotEqual(const tQuat& a, const tQuat& b)																	{ return (a.B0 != b.B0) || (a.B1 != b.B1); }
inline bool tNotEqual(const tQuat& a, const tQuat& b, tComponents c)													{ return ((c & tComponent_X) && (a.x != b.x)) || ((c & tComponent_Y) && (a.y != b.y)) || ((c & tComponent_Z) && (a.z != b.z)) || ((c & tComponent_W) && (a.w != b.w)); }
inline bool tNotEqual(const tMat2& a, const tMat2& b)																	{ return (a.B0 != b.B0) || (a.B1 != b.B1); }
inline bool tNotEqual(const tMat2& a, const tMat2& b, tComponents c)													{ return ((c & tComponent_A11) && (a.a11 != b.a11)) || ((c & tComponent_A21) && (a.a21 != b.a21)) || ((c & tComponent_A12) && (a.a12 != b.a12)) || ((c & tComponent_A22) && (a.a22 != b.a22)); }
inline bool tNotEqual(const tMat4& a, const tMat4& b)																	{ return (a.B0 != b.B0) || (a.B1 != b.B1) || (a.B2 != b.B2) || (a.B3 != b.B3) || (a.B4 != b.B4) || (a.B5 != b.B5) || (a.B6 != b.B6) || (a.B7 != b.B7); }
inline bool tNotEqual(const tMat4& a, const tMat4& b, tComponents c);

inline float tLengthSq(const tVec2& v)																					{ return v.x*v.x + v.y*v.y; }
inline float tLengthSq(const tVec3& v)																					{ return v.x*v.x + v.y*v.y + v.z*v.z; }
inline float tLengthSq(const tVec4& v)																					{ return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; }
inline float tLengthSq(const tQuat& q)																					{ return q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; }

inline float tLength(const tVec2& v)																					{ return tSqrt(tLengthSq(v)); }
inline float tLength(const tVec3& v)																					{ return tSqrt(tLengthSq(v)); }
inline float tLength(const tVec4& v)																					{ return tSqrt(tLengthSq(v)); }
inline float tLength(const tQuat& q)																					{ return tSqrt(tLengthSq(q)); }

inline float tLengthFast(const tVec2& v)																				{ return tSqrtFast(tLengthSq(v)); }
inline float tLengthFast(const tVec3& v)																				{ return tSqrtFast(tLengthSq(v)); }
inline float tLengthFast(const tVec4& v)																				{ return tSqrtFast(tLengthSq(v)); }
inline float tLengthFast(const tQuat& q)																				{ return tSqrtFast(tLengthSq(q)); }

inline void tNormalize(tVec2& v)																						{ float l = tLength(v); v.x /= l; v.y /= l; }
inline void tNormalize(tVec3& v)																						{ float l = tLength(v); v.x /= l; v.y /= l; v.z /= l; }
inline void tNormalize(tVec4& v)																						{ float l = tLength(v); v.x /= l; v.y /= l; v.z /= l; v.w /= l; }
inline void tNormalize(tQuat& q)																						{ float l = tLength(q); q.x /= l; q.y /= l; q.z /= l; q.w /= l; }

inline float tNormalizeGetLength(tVec2& v)					/* Returns pre-normalized length. */						{ float l = tLength(v); v.x /= l; v.y /= l; return l; }
inline float tNormalizeGetLength(tVec3& v)					/* Returns pre-normalized length. */						{ float l = tLength(v); v.x /= l; v.y /= l; v.z /= l; return l; }
inline float tNormalizeGetLength(tVec4& v)					/* Returns pre-normalized length. */						{ float l = tLength(v); v.x /= l; v.y /= l; v.z /= l; v.w /= l; return l; }
inline float tNormalizeGetLength(tQuat& q)					/* Returns pre-normalized length. */						{ float l = tLength(q); q.x /= l; q.y /= l; q.z /= l; q.w /= l; return l; }

inline bool tNormalizeSafe(tVec2& v)						/* Returns success. */										{ float l = tLength(v); if (l == 0.0f) return false; v.x /= l; v.y /= l; return true; }
inline bool tNormalizeSafe(tVec3& v)						/* Returns success. */										{ float l = tLength(v); if (l == 0.0f) return false; v.x /= l; v.y /= l; v.z /= l; return true; }
inline bool tNormalizeSafe(tVec4& v)						/* Returns success. */										{ float l = tLength(v); if (l == 0.0f) return false; v.x /= l; v.y /= l; v.z /= l; v.w /= l; return true; }
inline bool tNormalizeSafe(tQuat& q)						/* Returns success. */										{ float l = tLength(q); if (l == 0.0f) return false; q.x /= l; q.y /= l; q.z /= l; q.w /= l; return true; }

inline float tNormalizeSafeGetLength(tVec2& v)				/* Returns pre-normalized length. 0.0f if no go. */			{ float l = tLength(v); if (l == 0.0f) return l; v.x /= l; v.y /= l; return l; }
inline float tNormalizeSafeGetLength(tVec3& v)				/* Returns pre-normalized length. 0.0f if no go. */			{ float l = tLength(v); if (l == 0.0f) return l; v.x /= l; v.y /= l; v.z /= l; return l; }
inline float tNormalizeSafeGetLength(tVec4& v)				/* Returns pre-normalized length. 0.0f if no go. */			{ float l = tLength(v); if (l == 0.0f) return l; v.x /= l; v.y /= l; v.z /= l; v.w /= l; return l; }
inline float tNormalizeSafeGetLength(tQuat& q)				/* Returns pre-normalized length. 0.0f if no go. */			{ float l = tLength(q); if (l == 0.0f) return l; q.x /= l; q.y /= l; q.z /= l; q.w /= l; return l; }

inline void tNormalizeScale(tVec2& v, float s)				/* Normalize then scale. Resizes the vector. */				{ float l = tLength(v); v.x *= s/l; v.y *= s/l; }
inline void tNormalizeScale(tVec3& v, float s)				/* Normalize then scale. Resizes the vector. */				{ float l = tLength(v); v.x *= s/l; v.y *= s/l; v.z *= s/l; }
inline void tNormalizeScale(tVec4& v, float s)				/* Normalize then scale. Resizes the vector. */				{ float l = tLength(v); v.x *= s/l; v.y *= s/l; v.z *= s/l; v.w *= s/l; }

inline bool tNormalizeScaleSafe(tVec2& v, float s)																		{ float l = tLength(v); if (l == 0.0f) return false; v.x *= s/l; v.y *= s/l; return true; }
inline bool tNormalizeScaleSafe(tVec3& v, float s)																		{ float l = tLength(v); if (l == 0.0f) return false; v.x *= s/l; v.y *= s/l; v.z *= s/l; return true; }
inline bool tNormalizeScaleSafe(tVec4& v, float s)																		{ float l = tLength(v); if (l == 0.0f) return false; v.x *= s/l; v.y *= s/l; v.z *= s/l; v.w *= s/l; return true; }

inline void tNormalizeFast(tVec2& v)																					{ float s = tLengthSq(v); s = tRecipSqrtFast(s); v.x *= s; v.y *= s; }
inline void tNormalizeFast(tVec3& v)																					{ float s = tLengthSq(v); s = tRecipSqrtFast(s); v.x *= s; v.y *= s; v.z *= s; }
inline void tNormalizeFast(tVec4& v)																					{ float s = tLengthSq(v); s = tRecipSqrtFast(s); v.x *= s; v.y *= s; v.z *= s; v.w *= s; }
inline void tNormalizeFast(tQuat& q)																					{ float s = tLengthSq(q); s = tRecipSqrtFast(s); q.x *= s; q.y *= s; q.z *= s; q.w *= s; }

inline bool tNormalizeSafeFast(tVec2& v)																				{ float s = tLengthSq(v); if (s == 0.0f) return false; s = tRecipSqrtFast(s); v.x *= s; v.y *= s; return true; }
inline bool tNormalizeSafeFast(tVec3& v)																				{ float s = tLengthSq(v); if (s == 0.0f) return false; s = tRecipSqrtFast(s); v.x *= s; v.y *= s; v.z *= s; return true; }
inline bool tNormalizeSafeFast(tVec4& v)																				{ float s = tLengthSq(v); if (s == 0.0f) return false; s = tRecipSqrtFast(s); v.x *= s; v.y *= s; v.z *= s; v.w *= s; return true; }
inline bool tNormalizeSafeFast(tQuat& q)																				{ float s = tLengthSq(q); if (s == 0.0f) return false; s = tRecipSqrtFast(s); q.x *= s; q.y *= s; q.z *= s; q.w *= s; return true; }

// Addition. Computes d = a + b or vqm += a.
inline void tAdd(tVec2& v, const tVec2& a)																				{ v.x += a.x; v.y += a.y; }
inline void tAdd(tVec2& d, const tVec2& a, const tVec2& b)																{ d.x = a.x + b.x; d.y = a.y + b.y; }
inline void tAdd(tVec3& v, const tVec3& a)																				{ v.x += a.x; v.y += a.y; v.z += a.z; }
inline void tAdd(tVec3& d, const tVec3& a, const tVec3& b)																{ d.x = a.x + b.x; d.y = a.y + b.y; d.z = a.z + b.z; }
inline void tAdd(tVec4& v, const tVec4& a)																				{ v.x += a.x; v.y += a.y; v.z += a.z; v.w += a.w; }
inline void tAdd(tVec4& d, const tVec4& a, const tVec4& b)																{ d.x = a.x + b.x; d.y = a.y + b.y; d.z = a.z + b.z; d.w = a.w + b.w; }
inline void tAdd(tQuat& q, const tQuat& a)																				{ q.x += a.x; q.y += a.y; q.z += a.z; q.w += a.w; }
inline void tAdd(tQuat& d, const tQuat& a, const tQuat& b)																{ d.x = a.x + b.x; d.y = a.y + b.y; d.z = a.z + b.z; d.w = a.w + b.w; }
inline void tAdd(tMat2& m, const tMat2& a)																				{ for (int c = 0; c < 4; c++) m.E[c] += a.E[c]; }
inline void tAdd(tMat2& d, const tMat2& a, const tMat2& b)																{ for (int c = 0; c < 4; c++) d.E[c] = a.E[c] + b.E[c]; }
inline void tAdd(tMat4& m, const tMat4& a)																				{ for (int c = 0; c < 16; c++) m.E[c] += a.E[c]; }
inline void tAdd(tMat4& d, const tMat4& a, const tMat4& b)																{ for (int c = 0; c < 16; c++) d.E[c] = a.E[c] + b.E[c]; }

// Subtraction. Computes d = a - b or vqm -= a.
inline void tSub(tVec2& v, const tVec2& a)																				{ v.x -= a.x; v.y -= a.y; }
inline void tSub(tVec2& d, const tVec2& a, const tVec2& b)																{ d.x = a.x - b.x; d.y = a.y - b.y; }
inline void tSub(tVec3& v, const tVec3& a)																				{ v.x -= a.x; v.y -= a.y; v.z -= a.z; }
inline void tSub(tVec3& d, const tVec3& a, const tVec3& b)																{ d.x = a.x - b.x; d.y = a.y - b.y; d.z = a.z - b.z; }
inline void tSub(tVec4& v, const tVec4& a)																				{ v.x -= a.x; v.y -= a.y; v.z -= a.z; v.w -= a.w; }
inline void tSub(tVec4& d, const tVec4& a, const tVec4& b)																{ d.x = a.x - b.x; d.y = a.y - b.y; d.z = a.z - b.z; d.w = a.w - b.w; }
inline void tSub(tQuat& q, const tQuat& a)																				{ q.x -= a.x; q.y -= a.y; q.z -= a.z; q.w -= a.w; }
inline void tSub(tQuat& d, const tQuat& a, const tQuat& b)																{ d.x = a.x - b.x; d.y = a.y - b.y; d.z = a.z - b.z; d.w = a.w - b.w; }
inline void tSub(tMat2& m, const tMat2& a)																				{ for (int c = 0; c < 4; c++) m.E[c] -= a.E[c]; }
inline void tSub(tMat2& d, const tMat2& a, const tMat2& b)																{ for (int c = 0; c < 4; c++) d.E[c] = a.E[c] - b.E[c]; }
inline void tSub(tMat4& m, const tMat4& a)																				{ for (int c = 0; c < 16; c++) m.E[c] -= a.E[c]; }
inline void tSub(tMat4& d, const tMat4& a, const tMat4& b)																{ for (int c = 0; c < 16; c++) d.E[c] = a.E[c] - b.E[c]; }

// Negation. Computes d = -a or -vqm.
inline void tNeg(tVec2& v)																								{ v.x = -v.x; v.y = -v.y; }
inline void tNeg(tVec2& d, const tVec2& a)																				{ d.x = -a.x; d.y = -a.y; }
inline void tNeg(tVec3& v)																								{ v.x = -v.x; v.y = -v.y; v.z = -v.z; }
inline void tNeg(tVec3& d, const tVec3& a)																				{ d.x = -a.x; d.y = -a.y; d.z = -a.z; }
inline void tNeg(tVec4& v)																								{ v.x = -v.x; v.y = -v.y; v.z = -v.z; v.w = -v.w; }
inline void tNeg(tVec4& d, const tVec4& a)																				{ d.x = -a.x; d.y = -a.y; d.z = -a.z; d.w = -a.w; }
inline void tNeg(tQuat& q)																								{ q.x = -q.x; q.y = -q.y; q.z = -q.z; q.w = -q.w; }
inline void tNeg(tQuat& d, const tQuat& a)																				{ d.x = -a.x; d.y = -a.y; d.z = -a.z; d.w = -a.w; }
inline void tNeg(tMat2& m)																								{ m.a11 = -m.a11; m.a21 = -m.a21; m.a12 = -m.a12; m.a22 = -m.a22; }
inline void tNeg(tMat2& d, const tMat2& a)																				{ d.a11 = -a.a11; d.a21 = -a.a21; d.a12 = -a.a12; d.a22 = -a.a22; }
inline void tNeg(tMat4& m)																								{ for (int c = 0; c < 16; c++) m.E[c] = -m.E[c]; }
inline void tNeg(tMat4& d, const tMat4& a)																				{ for (int c = 0; c < 16; c++) d.E[c] = -a.E[c]; }

// Conjugation. Computes d = a* or q*.
inline void tConjugate(tQuat& q)																						{ q.x = -q.x; q.y = -q.y; q.z = -q.z; }
inline void tConjugate(tQuat& d, const tQuat& a)																		{ d.x = -a.x; d.y = -a.y; d.z = -a.z; d.w = a.w; }

// Multiply. Computes d = a * b or vq *= a.
inline void tMul(tVec2& v, float a)																						{ v.x *= a; v.y *= a; }
inline void tMul(tVec2& d, const tVec2& a, float b)																		{ d.x = a.x*b; d.y = a.y*b; }
inline void tMul(tVec2& d, float a, const tVec2& b)																		{ d.x = a*b.x; d.y = a*b.y; }
inline void tMul(tVec2& d, const tMat2& a, const tVec2& b)																{ d.x = b.x*a.a11 + b.y*a.a12; d.y = b.x*a.a21 + b.y*a.a22; }

// Multiplies a 4x4 matrix by a 3x1 vector. Not mathematically defined but convenient for homogeneous coordinates. The
// vector is interpreted as a 4x1 with the w = 1. If the 4x4 is an affine matrix, it simply transforms the point.
       void tMul(tVec3& d, const tMat4& a, const tVec3& b);
inline void tMul(tVec3& v, float a)																						{ v.x *= a; v.y *= a; v.z *= a; }
inline void tMul(tVec3& d, const tVec3& a, float b)																		{ d.x = a.x*b; d.y = a.y*b; d.z = a.z*b; }
inline void tMul(tVec3& d, float a, const tVec3& b)																		{ d.x = a*b.x; d.y = a*b.y; d.z = a*b.z; }
       void tMul(tVec4& d, const tMat4& a, const tVec4& b);
inline void tMul(tVec4& v, float a)																						{ v.x *= a; v.y *= a; v.z *= a; v.w *= a; }
inline void tMul(tVec4& d, const tVec4& a, float b)																		{ d.x = a.x*b; d.y = a.y*b; d.z = a.z*b; d.w = a.w*b;}
inline void tMul(tVec4& d, float a, const tVec4& b)																		{ d.x = a*b.x; d.y = a*b.y; d.z = a*b.z; d.w = a*b.w; }
inline void tMul(tQuat& q, float a)																						{ q.x *= a; q.y *= a; q.z *= a; q.w *= a; }
inline void tMul(tQuat& d, const tQuat& a, float b)																		{ d.x = a.x*b; d.y = a.y*b; d.z = a.z*b; d.w = a.w*b; }
inline void tMul(tQuat& d, float a, const tQuat& b)																		{ d.x = a*b.x; d.y = a*b.y; d.z = a*b.z; d.w = a*b.w; }
inline void tMul(tQuat& q, const tQuat& a);
inline void tMul(tQuat& d, const tQuat& a, const tQuat& b);
inline void tMul(tQuat& q, const tVec3& a);					// Treats 'a' as an augmented (pure) quaternion. w = 0.
inline void tMul(tQuat& d, const tQuat& a, const tVec3& b);	// Treats 'b' as an augmented (pure) quaternion. w = 0.
inline void tMul(tMat2& m, float a)																						{ m.a11 *= a; m.a21 *= a; m.a12 *= a; m.a22 *= a; }
inline void tMul(tMat2& d, const tMat2& a, float b)																		{ d.a11 = a.a11*b; d.a21 = a.a21*b; d.a12 = a.a12*b; d.a22 = a.a22*b; }
inline void tMul(tMat2& d, float a, const tMat2& b)																		{ d.a11 = a*b.a11; d.a21 = a*b.a21; d.a12 = a*b.a12; d.a22 = a*b.a22; }
inline void tMul(tMat2& m, const tMat2& a);
inline void tMul(tMat2& d, const tMat2& a, const tMat2& b);
inline void tMul(tMat4& m, float a)																						{ for (int c = 0; c < 16; c++) m.E[c] *= a; }
inline void tMul(tMat4& d, const tMat4& a, float b)																		{ for (int c = 0; c < 16; c++) d.E[c] = a.E[c]*b; }
inline void tMul(tMat4& d, float a, const tMat4& b)																		{ for (int c = 0; c < 16; c++) d.E[c] = a*b.E[c]; }
inline void tMul(tMat4& m, const tMat4& a);
       void tMul(tMat4& d, const tMat4& a, const tMat4& b);

// Vectors in Tacent are column vectors. If you want to multiply a column vector a by the transpose of another vector b,
// to produce a matrix, use these functions. Note that a row vector by a column is just a dot product. With these
// functions it is always b that gets transposed. The versions that take tVec3s get interpreted as tVec4s with w = 0.
inline void tMulByTranspose(tMat4& d, const tVec3& a, const tVec3& b);
inline void tMulByTranspose(tMat4& d, const tVec4& a, const tVec4& b);

// Hadamard or entry-wise product. Both matrices must have same dimension. Each position is multiplied independently.
// Computes d = a * b or vm *= a.
inline void tMulE(tVec2& v, const tVec2& a)																				{ v.x *= a.x; v.y *= a.y; }
inline void tMulE(tVec2& d, const tVec2& a, const tVec2& b)																{ d.x = a.x*b.x; d.y = a.y*b.y; }
inline void tMulE(tVec3& v, const tVec3& a)																				{ v.x *= a.x; v.y *= a.y; v.z *= a.z; }
inline void tMulE(tVec3& d, const tVec3& a, const tVec3& b)																{ d.x = a.x*b.x; d.y = a.y*b.y; d.z = a.z*b.z; }
inline void tMulE(tVec4& v, const tVec4& a)																				{ v.x *= a.x; v.y *= a.y; v.z *= a.z; v.w *= a.w; }
inline void tMulE(tVec4& d, const tVec4& a, const tVec4& b)																{ d.x = a.x*b.x; d.y = a.y*b.y; d.z = a.z*b.z; d.w = a.w*b.w; }
inline void tMulE(tMat2& m, const tMat2& a)																				{ tMulE(m.C1, a.C1); tMulE(m.C2, a.C2); }
inline void tMulE(tMat2& d, const tMat2& a, const tMat2& b)																{ tMulE(d.C1, a.C1, b.C1); tMulE(d.C2, a.C2, b.C2); }
inline void tMulE(tMat4& m, const tMat4& a)																				{ tMulE(m.C1, a.C1); tMulE(m.C2, a.C2); tMulE(m.C3, a.C3); tMulE(m.C4, a.C4); }
inline void tMulE(tMat4& d, const tMat4& a, const tMat4& b)																{ tMulE(d.C1, a.C1, b.C1); tMulE(d.C2, a.C2, b.C2); tMulE(d.C3, a.C3, b.C3); tMulE(d.C4, a.C4, b.C4); }

// Divide. Computes d = a / b or vqm /= a.
inline void tDiv(tVec2& v, float a)																						{ v.x /= a; v.y /= a; }
inline void tDiv(tVec2& d, const tVec2& a, float b)																		{ d.x = a.x/b; d.y = a.y/b; }
inline void tDiv(tVec3& v, float a)																						{ v.x /= a; v.y /= a; v.z /= a; }
inline void tDiv(tVec3& d, const tVec3& a, float b)																		{ d.x = a.x/b; d.y = a.y/b; d.z = a.z/b; }
inline void tDiv(tVec4& v, float a)																						{ v.x /= a; v.y /= a; v.z /= a; v.w /= a; }
inline void tDiv(tVec4& d, const tVec4& a, float b)																		{ d.x = a.x/b; d.y = a.y/b; d.z = a.z/b; d.w = a.w/b; }
inline void tDiv(tQuat& q, float a)																						{ q.x /= a; q.y /= a; q.z /= a; q.w /= a; }
inline void tDiv(tQuat& d, const tQuat& a, float b)																		{ d.x = a.x/b; d.y = a.y/b; d.z = a.z/b; d.w = a.w/b; }
inline void tDiv(tMat2& m, float a)																						{ m.a11 /= a; m.a21 /= a; m.a12 /= a; m.a22 /= a; }
inline void tDiv(tMat2& d, const tMat2& a, float b)																		{ d.a11 = a.a11/b; d.a21 = a.a21/b; d.a12 = a.a12/b; d.a22 = a.a22/b; }
inline void tDiv(tMat4& m, float a)																						{ for (int c = 0; c < 16; c++) m.E[c] /= a; }
inline void tDiv(tMat4& d, const tMat4& a, float b)																		{ for (int c = 0; c < 16; c++) d.E[c] = a.E[c]/b; }

// Component-wise divide. d = a / b or vm /= a.
inline void tDivE(tVec2& v, const tVec2& a)																				{ v.x /= a.x; v.y /= a.y; }
inline void tDivE(tVec2& d, const tVec2& a, const tVec2& b)																{ d.x = a.x/b.x; d.y = a.y/b.y; }
inline void tDivE(tVec3& v, const tVec3& a)																				{ v.x /= a.x; v.y /= a.y; v.z /= a.z; }
inline void tDivE(tVec3& d, const tVec3& a, const tVec3& b)																{ d.x = a.x/b.x; d.y = a.y/b.y; d.z = a.z/b.z; }
inline void tDivE(tVec4& v, const tVec4& a)																				{ v.x /= a.x; v.y /= a.y; v.z /= a.z; v.w /= a.w; }
inline void tDivE(tVec4& d, const tVec4& a, const tVec4& b)																{ d.x = a.x/b.x; d.y = a.y/b.y; d.z = a.z/b.z; d.w = a.w/b.w; }
inline void tDivE(tMat2& m, const tMat2& a)																				{ m.a11 /= a.a11; m.a21 /= a.a21; m.a12 /= a.a12; m.a22 /= a.a22; }
inline void tDivE(tMat2& d, const tMat2& a, const tMat2& b)																{ d.a11 = a.a11/b.a11; d.a21 = a.a21/b.a21; d.a12 = a.a12/b.a12; d.a22 = a.a22/b.a22; }
inline void tDivE(tMat4& m, const tMat4& a)																				{ for (int c = 0; c < 16; c++) m.E[c] /= a.E[c]; }
inline void tDivE(tMat4& d, const tMat4& a, const tMat4& b)																{ for (int c = 0; c < 16; c++) d.E[c] = a.E[c]/b.E[c]; }

inline float tDot(const tVec2& a, const tVec2& b)																		{ return a.x*b.x + a.y*b.y; }
inline float tDot(const tVec3& a, const tVec3& b)																		{ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float tDot(const tVec4& a, const tVec4& b)																		{ return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
inline float tDot(const tQuat& a, const tQuat& b)																		{ return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

// Linear interpolate/extrapolate. d stores the result.
// t in [0.0, 1.0] results in interpolation between a and b with a and b being the endpoints.
// t < 0.0 results in extrapolation to values less than a. t > 1.0 results in extrapolation to values greater than b.
inline void tLerp(tVec2& d, const tVec2& a, const tVec2& b, float t)													{ float t0 = 1.0f - t; d.x = t0*a.x + t*b.x; d.y = t0*a.y + t*b.y; }
inline void tLerp(tVec3& d, const tVec3& a, const tVec3& b, float t)													{ float t0 = 1.0f - t; d.x = t0*a.x + t*b.x; d.y = t0*a.y + t*b.y; d.z = t0*a.z + t*b.z; }
inline void tLerp(tVec4& d, const tVec4& a, const tVec4& b, float t)													{ float t0 = 1.0f - t; d.x = t0*a.x + t*b.x; d.y = t0*a.y + t*b.y; d.z = t0*a.z + t*b.z; d.w = t0*a.w + t*b.w; }

inline void tIdentity(tMat2& m)																							{ m.a11 = 1.0f; m.a21 = 0.0f; m.a12 = 0.0f; m.a22 = 1.0f; }
inline void tIdentity(tMat4& m)																							{ tZero(m); m.a11 = 1.0f; m.a22 = 1.0f; m.a33 = 1.0f; m.a44 = 1.0f; }

inline void tTranspose(tMat2& m)																						{ tStd::tSwap(m.a12, m.a21); }
inline void tTranspose(tMat2& d, const tMat2& a)																		{ d.a11 = a.a11; d.a21 = a.a12; d.a12 = a.a21; d.a22 = a.a22; }
inline void tTranspose(tMat4& m);
inline void tTranspose(tMat4& d, const tMat4& a)																		{ tSet(d, a); tTranspose(d); }

inline float tDeterminant(const tMat2& m)																				{ return m.a11*m.a22 - m.a12*m.a21; }
       float tDeterminant(const tMat4& m);

inline float tTrace(const tMat2& m)																						{ return m.a11 + m.a22; }
inline float tTrace(const tMat4& m)																						{ return m.a11 + m.a22 + m.a33 + m.a44; }

inline bool tInvert(tMat2& m);
inline bool tInvert(tMat2& d, const tMat2& a);
       bool tInvert(tMat4& m);
       bool tInvert(tMat4& d, const tMat4& a);

// Faster if inverting a matrix where the 4th column is the translation, and the upper-left 3x3 is the
// rotation/scale/skew.
inline void tInvertAffine(tMat4& m);
       void tInvertAffine(tMat4& d, const tMat4& a);

inline float tDistBetween(const tVec2& a, const tVec2& b)																{ tVec2 d; tSub(d, b, a); return tLength(d); }
inline float tDistBetweenSq(const tVec2& a, const tVec2& b)																{ tVec2 d; tSub(d, b, a); return tLengthSq(d); }
inline float tDistBetween(const tVec3& a, const tVec3& b)																{ tVec3 d; tSub(d, b, a); return tLength(d); }
inline float tDistBetweenSq(const tVec3& a, const tVec3& b)																{ tVec3 d; tSub(d, b, a); return tLengthSq(d); }

// Cross Product. Computes d = a X b or v X= a.
inline void tCross(tVec3& v, const tVec3& a);
inline void tCross(tVec3& d, const tVec3& a, const tVec3& b);
inline void tCross(tVec4& v, const tVec4& a);
inline void tCross(tVec4& d, const tVec4& a, const tVec4& b);
inline void tProject(tVec3& d, const tVec3& a, const tVec3& b)	/* Orthogonally projects b onto a. */					{ tMul(d, a, tDot(a, b)/tDot(a, a)); }

enum class tOrientationPath
{
	Shortest,
	Longest,
	Clockwise,
	CounterClockwise
};

// Spherical (great circle) linear interpolation. Const velocity but not-commutative. t = 0 yields a. t = 1 yields b.
// The version without the tOrientationPath is a bit faster than choosing shortest in the latter function. a and b may
// be the same quaternion as d for the lerp functions.
void tSlerp(tQuat& d, const tQuat& a, const tQuat& b, float t);
void tSlerp(tQuat& d, const tQuat& a, const tQuat& b, float t, tOrientationPath);

// Normalized linear interpolation. Shortest path through the sphere. Faster than slerp and commutative. Rotation
// sequences may be applied in any order with same results (order independent).
void tNlerp(tQuat& d, const tQuat& a, const tQuat& b, float t, tOrientationPath = tOrientationPath::Shortest);

float tRotationAngle(const tQuat& a);
void tRotate(tVec3& v, const tQuat& q);
void tRotate(tVec4& v, const tQuat& q);

inline void tMakeTranslate(tMat4& d, float x, float y, float z)															{ tIdentity(d); d.a14 = x; d.a24 = y; d.a34 = z; }
inline void tMakeTranslate(tMat4& d, const tVec3& t)																	{ tIdentity(d); d.a14 = t.x; d.a24 = t.y; d.a34 = t.z; }

void tMakeRotateX(tMat4& d, float angle);
void tMakeRotateY(tMat4& d, float angle);
void tMakeRotateZ(tMat4& d, float angle);
void tMakeRotate(tMat4& d, const tVec3& axis, float angle);
void tMakeRotate(tMat4& d, const tVec3& a, const tVec3& b);	// Rotates from from a to b. Normalize a and b first.

// The Euler angle functions make rotation matrices that are essentially concatenations of the RotateX, RotateY, and
// RotateZ calls above. There are 6 possible ways the successive rotations may be applied, and they are all implemented.
// The function tMakeRotateXYZ rotates by the x-angle first, then y, then z. That is, it generates RzRyRx. Remember
// v' = Mv. In all cases the angles are in radians and the rotations are specified about the 3 cardinal axes: x, y, z.
// An excellent reference can be found here: http://www.songho.ca/opengl/gl_anglestoaxes.html. I suggest using the XYZ
// more often than not as it is Maya's default. eulerX is sometimes referred to as Psi, AngleX or pitch, eulerY as
// Theta, Ay, or yaw, and eulerZ as Phi, Az, or roll.
void tMakeRotateXYZ(tMat4& d, float eulerX, float eulerY, float eulerZ);		// RzRyRx. Default for Maya.
void tMakeRotateYZX(tMat4& d, float eulerX, float eulerY, float eulerZ);		// RxRzRy
void tMakeRotateZXY(tMat4& d, float eulerX, float eulerY, float eulerZ);		// RyRxRz
void tMakeRotateZYX(tMat4& d, float eulerX, float eulerY, float eulerZ);		// RxRyRz
void tMakeRotateXZY(tMat4& d, float eulerX, float eulerY, float eulerZ);		// RyrzRx
void tMakeRotateYXZ(tMat4& d, float eulerX, float eulerY, float eulerZ);		// RzrxRy
inline void tMakeRotateXYZ(tMat4& d, const tVec3& euler)																{ tMakeRotateXYZ(d, euler.x, euler.y, euler.z); }
inline void tMakeRotateYZX(tMat4& d, const tVec3& euler)																{ tMakeRotateYZX(d, euler.x, euler.y, euler.z); }
inline void tMakeRotateZXY(tMat4& d, const tVec3& euler)																{ tMakeRotateZXY(d, euler.x, euler.y, euler.z); }
inline void tMakeRotateZYX(tMat4& d, const tVec3& euler)																{ tMakeRotateZYX(d, euler.x, euler.y, euler.z); }
inline void tMakeRotateXZY(tMat4& d, const tVec3& euler)																{ tMakeRotateXZY(d, euler.x, euler.y, euler.z); }
inline void tMakeRotateYXZ(tMat4& d, const tVec3& euler)																{ tMakeRotateYXZ(d, euler.x, euler.y, euler.z); }

inline void tMakeScale(tMat4& d, float scale)																			{ tIdentity(d); d.a11 = d.a22 = d.a33 = scale; }
inline void tMakeScale(tMat4& d, float scaleX, float scaleY, float scaleZ)												{ tIdentity(d); d.a11 = scaleX; d.a22 = scaleY; d.a33 = scaleZ; }
inline void tMakeScale(tMat4& d, const tVec3& scale)																	{ tIdentity(d); d.a11 = scale.x; d.a22 = scale.y; d.a33 = scale.z; }

inline void tMakeReflectXY(tMat4& d)																					{ tIdentity(d); d.a33 = -1.0f; }
inline void tMakeReflectXZ(tMat4& d)																					{ tIdentity(d); d.a22 = -1.0f; }
inline void tMakeReflectYZ(tMat4& d)																					{ tIdentity(d); d.a11 = -1.0f; }

void tMakeLookAt(tMat4& d, const tVec3& eye, const tVec3& look, const tVec3& up);

// Functions to make various types of projection matrix. Persp for perspective. Sym for symmetric. FovV for vertical
// field-of-view, FovH for field-of-view horizontal. The functions below all generate projection matrices that
// transform points to a clip space with z E [-1, 1] rather than the DX convention starting at 0.
void tMakeProjPersp(tMat4& d, float left, float right, float bottom, float top, float nearPlane, float farPlane);
void tMakeProjPersp(tMat4& d, const tVec3& boxMin, const tVec3& boxMax);
void tMakeProjPerspSym(tMat4& d, float right, float top, float nearPlane, float farPlane);

// Angles are named using the axis about which they rotate, so vertical FovV may also be written FovX, and horizontal
// FovH may be written FovY.
void tMakeProjPerspSymFovV(tMat4& d, float fovX, float aspect, float nearPlane, float farPlane);
void tMakeProjPerspSymFovH(tMat4& d, float fovY, float aspect, float nearPlane, float farPlane);

// The offsets in these functions allow creation of oblique (asymmetric) projection matrices. A (1, 1) offset will
// puts the camera at the upper right corner of the near/far planes. The visual effect of changing offsets is to pan
// the image around as if it were a big poster _without_ affecting the apparent perspective. This is useful if you want
// to offset an object on the screen without having it distort.
void tMakeProjPerspOffset
(
	tMat4& d, float right, float top,
	float nearPlane, float farPlane,
	float offsetX, float offsetY
);
void tMakeProjPerspFovVOffset
(
	tMat4& d, float fovX, float aspect,
	float nearPlane, float farPlane,
	float offsetX, float offsetY
);
void tMakeProjPerspFovHOffset
(
	tMat4& d, float fovY, float aspect,
	float nearPlane, float farPlane,
	float offsetX, float offsetY
);

// There is no 'orthogonal' projection matrix creation function since an ortho projection is a type of parallel
// projection. It is the view matrix that determines the direction in the world. i.e. What is colloquially known as an
// ortho projection is actually a parallel projection with an appropriate view matrix to align to a basis axis.
void tMakeProjParallel(tMat4& d, const tVec3& boxMin, const tVec3& boxMax);

// MakeOrthoNormal ensures the upper 3x3 matrix has orthogonal columns AND that they each have length one.
// MakeOrthoUniform yields orthogonal columns and equalizes (rather than normalizes) their lengths. Uniform scale safe.
// MakeOrthoNonUniform ensures orthogonal columns AND preserves each columns length separately. Non-uniform scale safe.
// MakeOrtho is synonymous with MakeOrthoNonUniform which preserves the most information (the safest).
void tMakeOrthoNormal(tMat4& m);
void tMakeOrthoUniform(tMat4& m);
void tMakeOrthoNonUniform(tMat4& m);
inline void tMakeOrtho(tMat4& m)																						{ tMakeOrthoNonUniform(m); }

// The following functions are used to extract information from various types of homogeneous matrix. We use the word
// extract here rather than decompose since decomposition of a matrix usually means factorizing (finding BC in A = BC).
// The naming generally follows tExtract_MatrixType_PropertiesExtracted.
void tExtractAffineScale(tVec3& scale, const tMat4& affine);

// This set of functions deal with projection
// matrices -- extracting FOVs, aspect ratios, clip planes etc. The tExtractProjectionPlanes returns vectors that
// represent the coefficients of the plane equation ax + by + cz + d = 0. The abcd members of the tVec4 class may be
// used as a convenience. The plane array is filled in the order Right, Left, Top, Bottom, Near, Far which matches
// the same order as the tFrustum class (+x,-x,+y,-y,+z,-z). The vectors may also be used to construct tPlane objects.
// If normalizePlanes is true, the plane normal length |(a,b,c)| = 1 and d is the distance from the origin.
void tExtractProjectionPlanes
(
	tVec4 planes[6], const tMat4& projection,
	bool outwardNormals = false, bool normalizePlanes = true
);

// These work with generic (perspective, oblique, and parallel) projections.
float tExtractProjectionNear(const tMat4&);				// Returns near plane dist (assumes frustum is not oblique).
float tExtractProjectionFar(const tMat4&);				// Returns far plane dist (assumes frustum is not oblique).
inline float tExtractProjectionAspect(const tMat4& m)	/* Returns aspect ratio, the width / height. */					{ tAssert(m.a11 != 0.0f); return m.a22 / m.a11; }
inline float tExtractProjectionOffsetX(const tMat4& m)	/* Returns offsetX, the horizontal oblique pan offset. */		{ return -m.a13; }
inline float tExtractProjectionOffsetY(const tMat4& m)	/* Returns offsetY, the vertical oblique pan offset. */			{ return -m.a23; }

// These are specific to perspective transforms.
inline float tExtractPerspectiveFovV(const tMat4& m)	/* Returns FovX. */												{ tAssert(m.a22 != 0.0f); return 2.0f * tArcTan(1.0f, m.a22); }
inline float tExtractPerspectiveFovH(const tMat4& m)	/* Returns FovY. */												{ tAssert(m.a11 != 0.0f); return 2.0f * tArcTan(1.0f, m.a11); }
void tExtractPerspective(float& fovV, float& fovH, float& aspect, float& nearPlane, float& farPlane, const tMat4&);
void tExtractPerspective
(
	float& fovV, float& fovH, float& aspect,
	float& nearPlane, float& farPlane,
	float& offsetX, float& offsetY,
	const tMat4&
);

// This function extracts Euler angles from a rotation matrix. It assumes a particular order of rotations were applied.
// tExtractRotationEulerXYZ assumes an X, Y, Z order meaning the rotation matrix is RzRyRx. The other orders are not yet
// implemented.
//
// tExtractAffineEulerXYZ can extract rotations for affine transformations that do NOT include shear because the basis
// vectors aren't orthogonal. i.e. tExtractAffineEulerXYZ can handle non-uniform scale -- affine transforms preserve
// parallel lines and include Euclidean rotation/translation, non-uniform scale, and shear (which we are excluding).
//
// If you know the matrix is orthonormal (columns are orthogonal AND of unit length), use the Euclidian version:
// tExtractRotationEulerXYZ). It will be faster.
//
// Any particular orientation can be achieved by at least 2 sets of Euler rotations, so there are 2 sets of Euler angles
// returned. When the middle rotation (eulerY) satisfies Cos(eulerY) = 0 (the middle one is Y for the XYZ order, which
// we will now assume for the rest of this comment) there are an infinite number of solutions. This is called gimbal
// lock.
//
// In gimbal lock you usually want to choose one of the infinity of solutions. In particular the eulerZ angle may be
// anything, eulerY will be +- Pi/2 (cos == 0), and eulerX depends on your choice of eulerZ. This is what gimbalZValue
// is for, it allows EulerX to be calculated for you -- it is your 'choice' of the Z rotation when in gimbal lock. Note
// that both sol1 and sol2 are equal when in gimbal lock. They both represent the chosen solution. IF you want to know
// what ALL solutions are, just leave gimbalZValue at zero. Choose any value for EulerZ (myZ). If EulerY > 0, then
// EulerZ = EulerX + myZ. If EulerY < 0, then EulerZ = EulerX - myZ.
//
// All returned angles are in [(-Pi, Pi)]. See comment by tIntervalBias for a description of my bias notation. This
// function Returns true if in gimbal lock or if it's close to gimbal lock as determined by an internal epsilon on
// Cos(eulerY). The extraction algorithm is based on this paper:
// http://staff.city.ac.uk/~sbbh653/publications/euler.pdf by Gregory G. Slabaugh. That's it. Simple, right?
bool tExtractRotationEulerXYZ
(
	tVec3& sol1, tVec3& sol2,
	const tMat4& rot,
	float gimbalZValue = 0.0f, tIntervalBias = tIntervalBias::Low
);

bool tExtractAffineEulerXYZ
(
	tVec3& sol1, tVec3& sol2,
	const tMat4& aff,
	float gimbalZValue = 0.0f, tIntervalBias = tIntervalBias::Low
);

// Linearly interpolate value towards dest at rate. t is delta time. Value is updated. Returns if got to dest.
bool tApproach(float& value, float dest, float rate, float dt);

// Angle approaches destination via shortest angular direction. Angle gets modified and will be E [0, 2Pi].
bool tApproachAngle(float& angle, float dest, float rate, float dt);
bool tApproachOrientation(tQuat& orientation, const tQuat& dest, float rate, float dt);

// Linear scale. t = 0 returns min. t = 1 returns max. No clamping. Works for non-pod vector and matrix types, but not
// the pod-types as they don't have the necessary operator overloading.
template <typename T> inline T tLinearScale(float t, T min, T max)														{ return min + (max - min)*t; }
template <typename T> inline T tLisc(float t, T min, T max)																{ return min + (max - min)*t; }

// Many libraries have a, perhaps badly-named, Lerp (linear interpolation) function that does a linear scale. It's a bit
// of a misnomer because in general it also extrapolates. In any case, the synonym is included here. Does same as tLisc.
template <typename T> inline T tLerp(float t, T min, T max)																{ return min + (max - min)*t; }

// Linear interpolation and extrapolation. Requires 2 points on the line. Input the domain values (d) and it'll give you
// the looked-up range value (r). No clamping.
template <typename T> inline T tLinearLookup(float d, float d0, float d1, T r0, T r1)									{ return tLinearScale((d-d0)/(d1-d0), r0, r1); }
template <typename T> inline T tLilo(float d, float d0, float d1, T r0, T r1)											{ return tLisc((d-d0)/(d1-d0), r0, r1); }

// Linear interpolate. Same as tLinearLookup except that it clamps d to [d0, d1].
template <typename T> inline T tLinearInterp(float d, float d0, float d1, T r0, T r1)									{ return tLinearScale(tSaturate((d-d0)/(d1-d0)), r0, r1); }
template <typename T> inline T tLiin(float d, float d0, float d1, T r0, T r1)											{ return tLisc(tSaturate((d-d0)/(d1-d0)), r0, r1); }


}


// Implementation below this line.


inline void tMath::tSet
(
	tMat4& d,
	float a11, float a21, float a31, float a41,
	float a12, float a22, float a32, float a42,
	float a13, float a23, float a33, float a43,
	float a14, float a24, float a34, float a44
)
{
	d.a11 = a11; d.a21 = a21; d.a31 = a31; d.a41 = a41;
	d.a12 = a12; d.a22 = a22; d.a32 = a32; d.a42 = a42;
	d.a13 = a13; d.a23 = a23; d.a33 = a33; d.a43 = a43;
	d.a14 = a14; d.a24 = a24; d.a34 = a34; d.a44 = a44;
}


inline void tMath::tGet
(
	float& a11, float& a21, float& a31, float& a41,
	float& a12, float& a22, float& a32, float& a42,
	float& a13, float& a23, float& a33, float& a43,
	float& a14, float& a24, float& a34, float& a44,
	const tMat4& s
)
{
	a11 = s.a11; a21 = s.a21; a31 = s.a31; a41 = s.a41;
	a12 = s.a12; a22 = s.a22; a32 = s.a32; a42 = s.a42;
	a13 = s.a13; a23 = s.a23; a33 = s.a33; a43 = s.a43;
	a14 = s.a14; a24 = s.a24; a34 = s.a34; a44 = s.a44;
}


inline bool tMath::tIsZero(const tMat4& m, tComponents c)
{
	return
		(!(c & tComponent_A11) || (m.a11 == 0.0f)) &&
		(!(c & tComponent_A21) || (m.a21 == 0.0f)) &&
		(!(c & tComponent_A31) || (m.a31 == 0.0f)) &&
		(!(c & tComponent_A41) || (m.a41 == 0.0f)) &&

		(!(c & tComponent_A12) || (m.a12 == 0.0f)) &&
		(!(c & tComponent_A22) || (m.a22 == 0.0f)) &&
		(!(c & tComponent_A32) || (m.a32 == 0.0f)) &&
		(!(c & tComponent_A42) || (m.a42 == 0.0f)) &&

		(!(c & tComponent_A13) || (m.a13 == 0.0f)) &&
		(!(c & tComponent_A23) || (m.a23 == 0.0f)) &&
		(!(c & tComponent_A33) || (m.a33 == 0.0f)) &&
		(!(c & tComponent_A43) || (m.a43 == 0.0f)) &&

		(!(c & tComponent_A14) || (m.a14 == 0.0f)) &&
		(!(c & tComponent_A24) || (m.a24 == 0.0f)) &&
		(!(c & tComponent_A34) || (m.a34 == 0.0f)) &&
		(!(c & tComponent_A44) || (m.a44 == 0.0f));
}


inline bool tMath::tApproxEqual(const tMat2& a, const tMat2& b, tComponents c, float e)
{
	return
		(!(c & tComponent_A11) || (tAbs(a.a11-b.a11) < e)) &&
		(!(c & tComponent_A21) || (tAbs(a.a21-b.a21) < e)) &&

		(!(c & tComponent_A12) || (tAbs(a.a12-b.a12) < e)) &&
		(!(c & tComponent_A22) || (tAbs(a.a22-b.a22) < e));
}


inline bool tMath::tApproxEqual(const tMat4& a, const tMat4& b, tComponents c, float e)
{
	return
		(!(c & tComponent_A11) || (tAbs(a.a11-b.a11) < e)) &&
		(!(c & tComponent_A21) || (tAbs(a.a21-b.a21) < e)) &&
		(!(c & tComponent_A31) || (tAbs(a.a31-b.a31) < e)) &&
		(!(c & tComponent_A41) || (tAbs(a.a41-b.a41) < e)) &&

		(!(c & tComponent_A12) || (tAbs(a.a12-b.a12) < e)) &&
		(!(c & tComponent_A22) || (tAbs(a.a22-b.a22) < e)) &&
		(!(c & tComponent_A32) || (tAbs(a.a32-b.a32) < e)) &&
		(!(c & tComponent_A42) || (tAbs(a.a42-b.a42) < e)) &&

		(!(c & tComponent_A13) || (tAbs(a.a13-b.a13) < e)) &&
		(!(c & tComponent_A23) || (tAbs(a.a23-b.a23) < e)) &&
		(!(c & tComponent_A33) || (tAbs(a.a33-b.a33) < e)) &&
		(!(c & tComponent_A43) || (tAbs(a.a43-b.a43) < e)) &&

		(!(c & tComponent_A14) || (tAbs(a.a14-b.a14) < e)) &&
		(!(c & tComponent_A24) || (tAbs(a.a24-b.a24) < e)) &&
		(!(c & tComponent_A34) || (tAbs(a.a34-b.a34) < e)) &&
		(!(c & tComponent_A44) || (tAbs(a.a44-b.a44) < e));
}


inline bool tMath::tEqual(const tMat4& a, const tMat4& b, tComponents c)
{
	return
		(!(c & tComponent_A11) || (a.a11 == b.a11)) &&
		(!(c & tComponent_A21) || (a.a21 == b.a21)) &&
		(!(c & tComponent_A31) || (a.a31 == b.a31)) &&
		(!(c & tComponent_A41) || (a.a41 == b.a41)) &&

		(!(c & tComponent_A12) || (a.a12 == b.a12)) &&
		(!(c & tComponent_A22) || (a.a22 == b.a22)) &&
		(!(c & tComponent_A32) || (a.a32 == b.a32)) &&
		(!(c & tComponent_A42) || (a.a42 == b.a42)) &&

		(!(c & tComponent_A13) || (a.a13 == b.a13)) &&
		(!(c & tComponent_A23) || (a.a23 == b.a23)) &&
		(!(c & tComponent_A33) || (a.a33 == b.a33)) &&
		(!(c & tComponent_A43) || (a.a43 == b.a43)) &&

		(!(c & tComponent_A11) || (a.a14 == b.a14)) &&
		(!(c & tComponent_A21) || (a.a24 == b.a24)) &&
		(!(c & tComponent_A31) || (a.a34 == b.a34)) &&
		(!(c & tComponent_A41) || (a.a44 == b.a44));
}


inline bool tMath::tNotEqual(const tMat4& a, const tMat4& b, tComponents c)
{
	return
		((c & tComponent_A11) && (a.a11 != b.a11)) ||
		((c & tComponent_A21) && (a.a21 != b.a21)) ||
		((c & tComponent_A31) && (a.a31 != b.a31)) ||
		((c & tComponent_A41) && (a.a41 != b.a41)) ||

		((c & tComponent_A12) && (a.a12 != b.a12)) ||
		((c & tComponent_A22) && (a.a22 != b.a22)) ||
		((c & tComponent_A32) && (a.a32 != b.a32)) ||
		((c & tComponent_A42) && (a.a42 != b.a42)) ||

		((c & tComponent_A13) && (a.a13 != b.a13)) ||
		((c & tComponent_A23) && (a.a23 != b.a23)) ||
		((c & tComponent_A33) && (a.a33 != b.a33)) ||
		((c & tComponent_A43) && (a.a43 != b.a43)) ||

		((c & tComponent_A14) && (a.a14 != b.a14)) ||
		((c & tComponent_A24) && (a.a24 != b.a24)) ||
		((c & tComponent_A34) && (a.a34 != b.a34)) ||
		((c & tComponent_A44) && (a.a44 != b.a44));
}


inline float tMath::tRotationAngle(const tQuat& q)
{
	float a = tMod(2.0f*tMath::tArcCos(q.w), tMath::TwoPi);
	if (a > 0.0f)
		a *= -1.0f;
	if (a < 0.0f)
		a += tMath::TwoPi;

	return a;
}


inline void tMath::tRotate(tVec3& v, const tQuat& q)
{
	tQuat c;	tConjugate(c, q);
	tQuat p;	tSet(p, q.v);
	tQuat r;	tMul(r, q, p);	tMul(r, c);		// r = qpc.
	tSet(v, r.x, r.y, r.z);
}


inline void tMath::tRotate(tVec4& v, const tQuat& q)
{
	float x = q.w*v.x + q.y*v.z - q.z*v.y;
	float y = q.w*v.y + q.z*v.x - q.x*v.z;
	float z = q.w*v.z + q.x*v.y - q.y*v.x;
	float w = -q.x*v.x - q.y*v.y - q.z*v.z;
	tSet(v, x, y, z, w);
}


inline void tMath::tMul(tQuat& q, const tQuat& a)
{
	float x = q.w*a.x + q.x*a.w + q.y*a.z - q.z*a.y;
	float y = q.w*a.y + q.y*a.w + q.z*a.x - q.x*a.z;
	float z = q.w*a.z + q.z*a.w + q.x*a.y - q.y*a.x;
	float w = q.w*a.w - q.x*a.x - q.y*a.y - q.z*a.z;
	tSet(q, x, y, z, w);
}


inline void tMath::tMul(tQuat& d, const tQuat& a, const tQuat& b)
{
	d.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
	d.y = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
	d.z = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
	d.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
}


inline void tMath::tMul(tQuat& q, const tVec3& a)
{
	float x =  q.w*a.x + q.y*a.z - q.z*a.y;
	float y =  q.w*a.y + q.z*a.x - q.x*a.z;
	float z =  q.w*a.z + q.x*a.y - q.y*a.x;
	float w = -q.x*a.x - q.y*a.y - q.z*a.z;
	tSet(q, x, y, z, w);
}


inline void tMath::tMul(tQuat& d, const tQuat& a, const tVec3& b)
{
	float x =  a.w*b.x + a.y*b.z - a.z*b.y;
	float y =  a.w*b.y + a.z*b.x - a.x*b.z;
	float z =  a.w*b.z + a.x*b.y - a.y*b.x;
	float w = -a.x*b.x - a.y*b.y - a.z*b.z;
	tSet(d, x, y, z, w);
}


inline void tMath::tMul(tMat2& m, const tMat2& b)
{
	tMat2 a;
	tSet(a, m);
	tMul(m, a, b);
}


inline void tMath::tMul(tMat2& d, const tMat2& a, const tMat2& b)
{
	d.a11 = a.a11*b.a11 + a.a12*b.a21;
	d.a21 = a.a21*b.a11 + a.a22*b.a21;
	d.a12 = a.a11*b.a12 + a.a12*b.a22;
	d.a22 = a.a21*b.a12 + a.a22*b.a22;
}


inline void tMath::tMul(tMat4& m, const tMat4& b)
{
	tMat4 a;
	tSet(a, m);
	tMul(m, a, b);
}


inline void tMath::tMulByTranspose(tMat4& d, const tVec3& a, const tVec3& b)
{
	tZero(d);
	d.a11 = a.x * b.x;
	d.a21 = a.y * b.x;
	d.a31 = a.z * b.x;

	d.a12 = a.x * b.y;
	d.a22 = a.y * b.y;
	d.a32 = a.z * b.y;

	d.a13 = a.x * b.z;
	d.a23 = a.y * b.z;
	d.a33 = a.z * b.z;
}


inline void tMath::tMulByTranspose(tMat4& d, const tVec4& a, const tVec4& b)
{
	tZero(d);
	d.a11 = a.x * b.x;	d.a21 = a.y * b.x;	d.a31 = a.z * b.x;	d.a41 = a.w * b.x;
	d.a12 = a.x * b.y;	d.a22 = a.y * b.y;	d.a32 = a.z * b.y;	d.a42 = a.w * b.y;
	d.a13 = a.x * b.z;	d.a23 = a.y * b.z;	d.a33 = a.z * b.z;	d.a43 = a.w * b.z;
	d.a14 = a.x * b.w;	d.a24 = a.y * b.w;	d.a34 = a.z * b.w;	d.a44 = a.w * b.w;
}


inline void tMath::tTranspose(tMat4& m)
{
	#if defined(PLATFORM_WIN)
	__m128* a = (__m128*)m.E;
	if (tIsAligned16(a))
	{
		_MM_TRANSPOSE4_PS(a[0], a[1], a[2], a[3]);
	}
	else
	#endif
	{
		tStd::tSwap(m.a21, m.a12);
		tStd::tSwap(m.a31, m.a13);
		tStd::tSwap(m.a41, m.a14);
		tStd::tSwap(m.a32, m.a23);
		tStd::tSwap(m.a42, m.a24);
		tStd::tSwap(m.a43, m.a34);
	}
}


inline bool tMath::tInvert(tMat2& d, const tMat2& s)
{
	// We leave the matrix alone if it's not invertible.
	float det = tDeterminant(s);
	if (det == 0.0f)
		return false;

	d.a11 = s.a22;
	d.a21 = -s.a21;
	d.a12 = -s.a12;
	d.a22 = s.a11;

	tDiv(d, det);
}


inline bool tMath::tInvert(tMat2& m)
{
	tMat2 s;
	tSet(s, m);
	return tInvert(m, s);
}


inline void tMath::tInvertAffine(tMat4& m)
{
	tMat4 s;
	tSet(s, m);
	tInvertAffine(m, s);
}


inline void tMath::tCross(tVec3& v, const tVec3& a)
{
	float x =  v.y*a.z - v.z*a.y;
	float y =  v.z*a.x - v.x*a.z;
	float z =  v.x*a.y - v.y*a.x;
	tSet(v, x, y, z);
}


inline void tMath::tCross(tVec3& d, const tVec3& a, const tVec3& b)
{
	d.x =  a.y*b.z - a.z*b.y;
	d.y =  a.z*b.x - a.x*b.z;
	d.z =  a.x*b.y - a.y*b.x;
}


inline void tMath::tCross(tVec4& v, const tVec4& a)
{
	float x =  v.y*a.z - v.z*a.y;
	float y =  v.z*a.x - v.x*a.z;
	float z =  v.x*a.y - v.y*a.x;
	tSet(v, x, y, z, 1.0f);
}


inline void tMath::tCross(tVec4& d, const tVec4& a, const tVec4& b)
{
	d.x =  a.y*b.z - a.z*b.y;
	d.y =  a.z*b.x - a.x*b.z;
	d.z =  a.x*b.y - a.y*b.x;
	d.w = 1.0f;
}


inline void tMath::tMakeRotateX(tMat4& d, float a)
{
	float cos = tCos(a);
	float sin = tSin(a);

	// Matrix is being populated column major so it looks like the transpose of a math text but it's correct.
	d.a11 = 1.0f;	d.a21 = 0.0f;	d.a31 = 0.0f;	d.a41 = 0.0f;
	d.a12 = 0.0f;	d.a22 =  cos;	d.a32 =  sin;	d.a42 = 0.0f;
	d.a13 = 0.0f;	d.a23 = -sin;	d.a33 =  cos;	d.a43 = 0.0f;
	d.a14 = 0.0f;	d.a24 = 0.0f;	d.a34 = 0.0f;	d.a44 = 1.0f;
}


inline void tMath::tMakeRotateY(tMat4& d, float a)
{
	float cos = tCos(a);
	float sin = tSin(a);

	d.a11 =  cos;	d.a21 = 0.0f;	d.a31 = -sin;	d.a41 = 0.0f;
	d.a12 = 0.0f;	d.a22 = 1.0f;	d.a32 = 0.0f;	d.a42 = 0.0f;
	d.a13 =  sin;	d.a23 = 0.0f;	d.a33 =  cos;	d.a43 = 0.0f;
	d.a14 = 0.0f;	d.a24 = 0.0f;	d.a34 = 0.0f;	d.a44 = 1.0f;
}


inline void tMath::tMakeRotateZ(tMat4& d, float a)
{
	float cos = tCos(a);
	float sin = tSin(a);

	d.a11 =  cos;	d.a21 =  sin;	d.a31 = 0.0f;	d.a41 = 0.0f;
	d.a12 = -sin;	d.a22 =  cos;	d.a32 = 0.0f;	d.a42 = 0.0f;
	d.a13 = 0.0f;	d.a23 = 0.0f;	d.a33 = 1.0f;	d.a43 = 0.0f;
	d.a14 = 0.0f;	d.a24 = 0.0f;	d.a34 = 0.0f;	d.a44 = 1.0f;
}


inline void tMath::tExtractAffineScale(tVec3& scale, const tMat4& affine)
{
	scale.x = tLength(affine.C1);
	scale.y = tLength(affine.C2);
	scale.z = tLength(affine.C3);
}


inline float tMath::tExtractProjectionNear(const tMat4& m)
{
	// To describe how this works we need to do a little math. For a general projection matrix
	// a33 = -(f+n)/(f-n)		(eq1) and
	// a34 = -2fn/(f-n)			(eq2)
	// So we now have a system of 2 equations with 2 unknowns. Perfect. We're in simple algebra-land. Let a=a33, b=a34.
	// Eq1) -(f+n)=(f-n)a -> -af-f=-an+n -> f(a+1)=n(a-1) -> f=n(a-1)/(a+1) -> f=nc where c is now a known constant.
	// Subs nc for f in Eq2) ncn/(nc-n)=b/-2 -> cn/(c-1)=b/-2 -> n=(c-1)b/-2c. There's our near plane. For the far replce
	// n with f/c to get f=(c-1)b/-2. Done.
	float c = (m.a33 - 1.0f)/(m.a33 + 1.0f);
	return (c - 1.0f)*m.a34/(-2.0f*c);
}


inline float tMath::tExtractProjectionFar(const tMat4& m)
{
	// See math for tExtractProjectionNear.
	float c = (m.a33 - 1.0f)/(m.a33 + 1.0f);
	return (c - 1.0f)*m.a34/(-2.0f);
}


inline void tMath::tExtractPerspective(float& fovV, float& fovH, float& aspect, float& nearPlane, float& farPlane, const tMat4& persp)
{
	fovV = tExtractPerspectiveFovV(persp);
	fovH = tExtractPerspectiveFovH(persp);
	aspect = tExtractProjectionAspect(persp);
	nearPlane = tExtractProjectionNear(persp);
	farPlane = tExtractProjectionFar(persp);
}


inline void tMath::tExtractPerspective(float& fovV, float& fovH, float& aspect, float& nearPlane, float& farPlane, float& offsetX, float& offsetY, const tMat4& persp)
{
	tExtractPerspective(fovV, fovH, aspect, nearPlane, farPlane, persp);
	offsetX = tExtractProjectionOffsetX(persp);
	offsetY = tExtractProjectionOffsetY(persp);
}


inline bool tMath::tApproach(float& value, float dest, float rate, float t)
{
	float diff  = tAbs(dest - value);
	float delta = rate * t;

	if (diff <= delta)
	{
		value = dest;
		return true;
	}

	value += (value < dest) ? delta : -delta;
	return false;
}
