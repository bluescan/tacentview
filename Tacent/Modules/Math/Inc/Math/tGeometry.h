// tGeometry.h
//
// Idealized geometric shapes. These include triangles, circles, boxes, spheres, rays, lines, planes, cylinders,
// capsules, frustums, and ellipses. When the shape primitives have a number in their name, it refers to the space the
// shape is in, not the dimensionality of the shape itself. eg. A tCircle3 is a (2D) circle in R3 while a tCircle2 is a
// 2D circle in R2. For shapes that are in R3 we drop the 3 because the R3 primitives are more general.
//
// Copyright (c) 2006, 2016, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include "Math/tVector2.h"
#include "Math/tVector3.h"
#include "Math/tVector4.h"
#include "Math/tMatrix2.h"
#include "Math/tMatrix4.h"
namespace tMath
{


// Edges are simply line segments with two end points. The tEdge data structure is just a set of 2 indexes into any
// sort of vertex array. It is therefore dimension-agnostic and can be used for edges in R2 and R3.
struct tEdge
{
	tEdge()																												{ }
	tEdge(const tEdge& s)																								{ for (int i = 0; i < 2; i++) Index[i] = s.Index[i]; }
	bool operator==(const tEdge& e) const																				{ return (Index[0] == e.Index[0]) && (Index[1] == e.Index[1]); }
	bool operator!=(const tEdge& e) const																				{ return (Index[0] != e.Index[0]) || (Index[1] != e.Index[1]); }
	tEdge& operator=(const tEdge& e)																					{ Index[0] = e.Index[0]; Index[1] = e.Index[1]; return *this; }
	int Index[2];
};


// A triangular face. It has 3 vertices and 3 edges. It may be degenerate. The tTriFace data structure is just a set of
// 3 indexes into any sort of vertex array. It is therefore dimension-agnostic and can be used for tris in R2 and R3.
struct tTriFace
{
	tTriFace()																											{ }
	tTriFace(const tTriFace& s)																							{ for (int i = 0; i < 3; i++) Index[i] = s.Index[i]; }
	bool operator==(const tTriFace& f) const																			{ return (Index[0] == f.Index[0]) && (Index[1] == f.Index[1]) && (Index[2] == f.Index[2]); }
	bool operator!=(const tTriFace& f) const																			{ return (Index[0] != f.Index[0]) || (Index[1] != f.Index[1]) || (Index[2] != f.Index[2]); }
	tTriFace& operator=(const tTriFace& f)																				{ Index[0] = f.Index[0]; Index[1] = f.Index[1]; Index[2] = f.Index[2]; return *this; }
	int Index[3];
};
typedef tTriFace tTri;										// A short synonym for a triangular face.


// The same as a tTriFace but using only 16 bit indices.
struct tTriFaceShort
{
	tTriFaceShort()																										{ }
	tTriFaceShort(const tTriFaceShort& s)																				{ for (int v = 0; v < 3; v++) Index[v] = s.Index[v]; }
	bool operator==(const tTriFaceShort& f) const																		{ return (Index[0] == f.Index[0]) && (Index[1] == f.Index[1]) && (Index[2] == f.Index[2]); }
	bool operator!=(const tTriFaceShort& f) const																		{ return (Index[0] != f.Index[0]) || (Index[1] != f.Index[1]) || (Index[2] != f.Index[2]); }
	tTriFaceShort& operator=(const tTriFaceShort& f)																	{ Index[0] = f.Index[0]; Index[1] = f.Index[1]; Index[2] = f.Index[2]; return *this; }
	uint16 Index[3];
};
typedef tTriFaceShort tTriShort;							// A short synonym for a short triangular face.


// A quad face. It has 4 vertices and 4 edges. It may be degenerate. The tQuadFace data structure is just a set of
// 4 indexes into any sort of vertex array. It is therefore dimension-agnostic and can be used for quads in R2 and R3.
struct tQuadFace
{
	tQuadFace()																											{ }
	tQuadFace(const tQuadFace& s)																						{ for (int i = 0; i < 4; i++) Index[i] = s.Index[i]; }
	bool operator==(const tQuadFace& f) const																			{ return (Index[0] == f.Index[0]) && (Index[1] == f.Index[1]) && (Index[2] == f.Index[2]) && (Index[3] == f.Index[3]); }
	bool operator!=(const tQuadFace& f) const																			{ return (Index[0] != f.Index[0]) || (Index[1] != f.Index[1]) || (Index[2] != f.Index[2]) || (Index[3] != f.Index[3]); }
	tQuadFace& operator=(const tQuadFace& f)																			{ Index[0] = f.Index[0]; Index[1] = f.Index[1]; Index[2] = f.Index[2]; Index[3] = f.Index[3]; return *this; }
	int Index[4];
};
typedef tQuadFace tQuad;									// A short synonym for a quadrilateral face.


// The same as a tQuadFace but using only 16 bit indices.
struct tQuadFaceShort
{
	tQuadFaceShort()																									{ }
	tQuadFaceShort(const tQuadFaceShort& s)																				{ for (int i = 0; i < 4; i++) Index[i] = s.Index[i]; }
	bool operator==(const tQuadFaceShort& f) const																		{ return (Index[0] == f.Index[0]) && (Index[1] == f.Index[1]) && (Index[2] == f.Index[2]) && (Index[3] == f.Index[3]); }
	bool operator!=(const tQuadFaceShort& f) const																		{ return (Index[0] != f.Index[0]) || (Index[1] != f.Index[1]) || (Index[2] != f.Index[2]) || (Index[3] != f.Index[3]); }
	tQuadFaceShort& operator=(const tQuadFaceShort& f)																	{ Index[0] = f.Index[0]; Index[1] = f.Index[1]; Index[2] = f.Index[2]; Index[3] = f.Index[3]; return *this; }
	uint16 Index[4];
};
typedef tQuadFace tQuad;									// A short synonym for a quadrilateral face.


// A line segment goes from point A to point B. Ends are included. The 2 means in R2.
struct tLineSeg2
{
	tLineSeg2()																											{ }
	tLineSeg2(float x0, float y0, float x1, float y1)																	{ A.Set(x0, y0); B.Set(x1, y1); }
	tLineSeg2(const tVector2& a, const tVector2& b)																		: A(a), B(b) { }
	tLineSeg2(const tLineSeg2& s)																						: A(s.A), B(s.B) { }
	union
	{
		struct { tVector2 A, B; };
		tVector2 E[2];
	};
};


// A line goes through point A and point B.
struct tLine2
{
	tLine2()																											{ }
	tLine2(float x0, float y0, float x1, float y1)																		{ A.Set(x0, y0); B.Set(x1, y1); }
	tLine2(const tVector2& a, const tVector2& b)																		: A(a), B(b) { }
	tLine2(const tLineSeg2& s)																							: A(s.A), B(s.B) { }
	tLine2(const tLine2& s)																								: A(s.A), B(s.B) { }
	union
	{
		struct { tVector2 A, B; };
		tVector2 E[2];
	};
};


// A ray from Start in (normalized) direction Dir.
struct tRay2
{
	tRay2()																												{ }
	tVector2 Start;
	tVector2 Dir;											// Normalized.
};


// An axis aligned rectangle in R2.
struct tARect2
{
	enum Side
	{
		Side_None																										= 0x00000000,
		Side_Left																										= 0x00000001,
		Side_Right																										= 0x00000002,
		Side_Bottom																										= 0x00000004,
		Side_Top																										= 0x00000008
	};

	tARect2()																											{ Clear(); }
	tARect2(const tARect2& src)																							{ Set(src); }
	tARect2(const tVector2& origin, float sideLength)																	{ Set(origin, sideLength); }
	tARect2(const tVector2& min, float width, float height)																{ Set(min, width, height); }
	tARect2(const tVector2& min, const tVector2& max)																	{ Set(min, max); }
	tARect2(float minx, float miny, float maxx, float maxy)																{ Set(minx, miny, maxx, maxy); }

	void Set(const tARect2& src)																						{ Min.Set(src.Min); Max.Set(src.Max); }
	void Set(const tVector2& origin, float sideLength)																	{ Min.Set(origin.x - sideLength/2.0f, origin.y - sideLength/2.0f); Max.Set(origin.x + sideLength/2.0f, origin.y + sideLength/2.0f); }
	void Set(const tVector2& min, float width, float height)															{ Min.Set(min); Max.Set(min.x + width, min.y + height); }
	void Set(const tVector2& min, const tVector2& max)																	{ Min.Set(min); Max.Set(max); }
	void Set(float minx, float miny, float maxx, float maxy)															{ Min.Set(minx, miny); Max.Set(maxx, maxy); }

	void AddPoint(const tVector2&);
	void Expand(float e)																								{ Min.x -= e; Min.y -= e; Max.x += e; Max.y += e; }
	void Clear()																										{ Min = tVector2(PosInfinity, PosInfinity); Max = tVector2(NegInfinity, NegInfinity); }
	void Empty()																										{ Clear(); }

	// Boundary included.
	bool IsPointInside(const tVector2& point) const																		{ return ((point.x >= Min.x) && (point.x <= Max.x) && (point.y >= Min.y) && (point.y <= Max.y)); }
	
	// The boundary is included. Positive margin values increase the rectangle size.
	bool IsPointInside(const tVector2& point, float margin) const														{ return ((point.x >= Min.x-margin) && (point.x <= Max.x+margin) && (point.y >= Min.y-margin) && (point.y <= Max.y+margin)); }

	// Similar to above except you can control which boundary sides are included. With default bias values this
	// function includes the 2 minimum boundary lines and does not include the 2 maximum lines. This is useful for
	// situations such as spatial partitioning where a single point must not be inside multiple adjacent boxes.
	bool IsPointInsideBias
	(
		const tVector2& point,
		tIntervalBias biasX = tIntervalBias::Low,
		tIntervalBias biasY = tIntervalBias::Low
	) const;

	// Transforms Min and Max vectors and builds the new box from them.
	void Transform(const tMatrix2& xform);

	tVector2 ComputeCenter() const																						{ return (Min + Max)/2.0f; }
	tVector2 ComputeExtents() const																						{ return (Max - Min)/2.0f; }
	bool Overlaps(const tARect2&) const;

	tVector2 Min;
	tVector2 Max;
};
typedef tARect2 tRect2;


struct tORect2
{
	tORect2()																											: Center(0.0f, 0.0f), Extents(0.0f, 0.0f) { Axes[0].Set(1.0f, 0.0f); Axes[1].Set(0.0f, 1.0f); }
	bool IsInside(const tVector2& point) const;
	tVector2 Center;
	tVector2 Axes[2];
	tVector2 Extents;
};


struct tCircle2
{
	tCircle2()																											{ }
	float Area() const																									{ return Pi * Radius * Radius; }
	float Radius;
	tVector2 Center;
};


// An ellipse in R2. The ellipse equation is: (x-a)^2 / r1^2  +  (y-b)^2 / r2^2 = 1 where (a, b) is the center location,
// r1 is the x 'radius', and r2 is the y 'radius'.
struct tEllipse2
{
	tEllipse2()																											{ }
	float Area() const																									{ return Pi * RadiusX * RadiusY; }

	float RadiusX, RadiusY;
	tVector2 Center;
};


// Primitives in 3D space.


// A line segment has the same members as a line, but only goes from A to B (inclusive).
struct tLineSeg
{
	tLineSeg()																											{ }
	tLineSeg(const tVector3& a, const tVector3& b)																		: A(a), B(b) { }
	tLineSeg(const tLineSeg& src)																						: A(src.A), B(src.B) { }

	union
	{
		struct { tVector3 A, B; };
		tVector3 E[2];
	};
};


// A ray from Start in normalized direction Dir.
struct tRay
{
	tRay()																												{ }
	tRay(const tVector3& start, const tVector3& dir)																	: Start(start), Dir(dir) { }
	tRay(const tLineSeg& s)																								{ Set(s); }
	void Set(const tVector3& start, const tVector3& dir)																{ Start = start; Dir = dir; }
	void Set(const tLineSeg& s)																							{ Start = s.A; Dir = s.B - s.A; Dir.NormalizeSafe(); }

	tVector3 Start;
	tVector3 Dir;
};


// Goes through A and B and extends infinitely.
struct tLine
{
	tLine()																												{ }
	tLine(const tRay& ray)																								{ Set(ray); }
	tLine(const tLineSeg& seg)																							{ Set(seg); }
	void Set(const tRay& ray)																							{ A = ray.Start; B = ray.Start + ray.Dir; }
	void Set(const tLineSeg& seg)																						{ A = seg.A; B = seg.B; }

	union
	{
		struct { tVector3 A, B; };
		tVector3 E[2];
	};
};


// A cylinder. Represented as a line segment with a radius.
struct tCylinder
{
	tCylinder()																											{ }
	tCylinder(const tVector3& a, const tVector3& b, float radius)														: Radius(radius), A(a), B(b) { }
	tCylinder(const tLineSeg& s, float radius)																			: Radius(radius), A(s.A), B(s.B) { }

	float Radius;
	tVector3 A, B;
};


// A capsule is just a cylinder with hemispherical ends. It uses the same representation as a tCylinder. The only
// difference is in how the state varialble are interpreted. For a capsule, the ends parts also have a radius.
typedef tCylinder tCapsule;


struct tTriangle
{
	tTriangle()																											{ }
	tTriangle(const tVector3& a, const tVector3& b, const tVector3& c)													: A(a), B(b), C(c) { }

	union
	{
		struct { tVector3 A, B, C; };
		tVector3 E[3];
	};
};


// This is an axis-aligned box in R3.
struct tABox
{
	tABox()																												: Min(PosInfinity, PosInfinity, PosInfinity), Max(NegInfinity, NegInfinity, NegInfinity) { }
	tABox(const tABox& src)																								: Min(src.Min), Max(src.Max) { }
	tABox(const tVector3& min, float width, float height, float depth)													: Min(min), Max(min.x + width, min.y + height, min.z + depth) { }
	tABox(const tVector3& min, const tVector3& max)																		: Min(min), Max(max) { }
	tABox(float minx, float miny, float minz, float maxx, float maxy, float maxz)										: Min(minx, miny, minz), Max(maxx, maxy, maxz) { }

	void AddPoint(const tVector3&);
	void Expand(float e)																								{ Min.x -= e; Min.y -= e; Min.z -= e; Max.x += e; Max.y += e; Max.z += e; }
	void Empty()																										{ Min = tVector3(PosInfinity, PosInfinity, PosInfinity); Max = tVector3(NegInfinity, NegInfinity, NegInfinity); }
	void Transform(const tMatrix4& xform);					// Affine and projection transforms supported.

	// Boundary included.
	bool IsPointInside(const tVector3& point) const																		{ return ((point.x >= Min.x) && (point.x <= Max.x) && (point.y >= Min.y) && (point.y <= Max.y) && (point.z >= Min.z) && (point.z <= Max.z)); }

	// The boundary is included. Positive margin values increase the rectangle size.
	bool IsPointInside(const tVector3& point, float margin) const														{ return ((point.x >= Min.x-margin) && (point.x <= Max.x+margin) && (point.y >= Min.y-margin) && (point.y <= Max.y+margin) && (point.z >= Min.z-margin) && (point.z <= Max.z+margin)); }

	// Similar to above except you can control which boundary planes are included. With default bias values this
	// function includes the 3 minimum boundary planes and does not include the 3 maximum planes. This is useful for
	// situations such as spatial partitioning where a single point must not be inside multiple adjacent boxes.
	bool IsPointInsideBias
	(
		const tVector3& point,
		tIntervalBias biasX = tIntervalBias::Low,
		tIntervalBias biasY = tIntervalBias::Low,
		tIntervalBias biasZ = tIntervalBias::Low
	) const;

	tVector3 ComputeCenter() const																						{ return (Min + Max)/2.0f; }
	tVector3 ComputeExtents() const																						{ return (Max - Min)/2.0f; }
	bool Overlaps(const tABox&) const																					{ tAssert(!"Not implemented yet."); return false; }

	tVector3 Min;
	tVector3 Max;
};
typedef tABox tBox;
typedef tABox tAABB;


// @todo The oriented box is mostly unimplemented.
struct tOBox
{
	tOBox()																												: Center(0.0f, 0.0f, 0.0f), Extents(0.0f, 0.0f, 0.0f) { Axes[0].Set(1.0f, 0.0f, 0.0f); Axes[1].Set(0.0f, 1.0f, 0.0f); Axes[2].Set(0.0f, 0.0f, 1.0f); }
	bool IsInside(const tVector3& point) const;

	tVector3 Center;
	tVector3 Axes[3];
	tVector3 Extents;
};
typedef tOBox tOBB;


struct tCircle
{
	tCircle()																											{ }
	float Area() const																									{ return Pi * Radius * Radius; }

	float Radius;
	tVector3 Center;
	tVector3 Normal;
};


struct tSphere
{
	tSphere()																											{ }
	tSphere(const tVector3& center, float radius)																		: Center(center), Radius(radius) { }
	void Set(const tVector3& center, float radius)																		{ Center = center; Radius = radius; }

	tVector3 Center;
	float Radius;
};


// A tPlane essentially represents the plane equation ax + by + cz + d = 0 using two member variables: Normal and
// Distance. These two values act as the coefficients of the plane equation with a = Normal.x, b = Normal.y,
// c = Normal.z, and d = Distance.
struct tPlane
{
public:
	tPlane()																											{ }
	tPlane(float a, float b, float c, float d = 0.0f)																	{ Set(a, b, c, d); }
	tPlane(const tVector3& normal, float d = 0.0f)																		{ Set(normal, d); }
	tPlane(const tVector4& v)																							{ Set(v); }
	tPlane(const tVector3& normal, const tVector3& p)																	{ Set(normal, p); }
	tPlane(const tVector3& p1, const tVector3& p2, const tVector3& p3)													{ Set(p1, p2, p3); }

	void Set(float a, float b, float c, float d)																		{ Normal.Set(a, b, c); Distance = d; }
	void Set(const tVector3& normal, float d)																			{ Normal = normal; Distance = d; }
	void Set(const tVector4& v)																							{ Normal = tVector3(v); Distance = v.w; }
	void Set(const tVector3& normal, const tVector3& p)																	{ Normal = normal; Distance = -(normal*p); }
	void Set(const tVector3& p1, const tVector3& p2, const tVector3& p3)												{ tVector3 v1 = p2 - p1; tVector3 v2 = p3 - p1; Normal = v1 % v2; Distance = -Normal * p3; }

	// Makes Normal length 1. When this is done the Distance member will be how far away the plane is from the origin in
	// the direction of Normal. The Distance /= length in the implementation is CORRECT.
	void Normalize()																									{ float l = Normal.Length(); Normal /= l; Distance /= l; }

	// If the plane is normalized this returns the shortest distance from 'point' to the plane. Normalized or not, if
	// the result is < 0 then the point is in the negative half-space, > 0 for positive half-space, and 0 on the plane.
	float GetDistance(const tVector3& point) const																		{ return Normal.x*point.x + Normal.y*point.y + Normal.z*point.z + Distance; }

	// Computes two unit vectors for the plane that form an orthogonal basis. The plane d-value is irrelevant and the
	// plane may be considered to go through the origin. The plane does not need to be normalized before calling.
	void ComputeOrthogonalBasisVectors(tVector3& u, tVector3& v) const;

	// The Normal is a vector perpendicular to the surface of the plane.
	tVector3 Normal;

	// the Distance only represents the distance to the plane if the plane is normalized. If it is not normalized, this
	// is MOT a multiplier to get the normal to the plane because ax+by+cz+d=0 is the same plane as kax+kby+kcz+kd=0.
	// Distance (d) more as the offset of the point (a,b,c) from the origin. They grow and shrink together.
	float Distance;
};


class tFrustum
{
public:
	tFrustum()																											{ }
	tFrustum(const tMatrix4& m)																							{ Set(m); }
	tFrustum(const tPlane p[6])																							{ Set(p); }
	tFrustum(const tVector4 v[6])																						{ Set(v); }

	void Set(const tMatrix4&);
	void Set(const tPlane[6]);
	void Set(const tVector4[6]);

	// Use to index into the Planes array.
	enum class Plane
	{
		Right,
		Left,
		Top,
		Bottom,
		Near,
		Far,
		NumPlanes
	};

	// Normals are interior-facing. The near distance (mNear.mDist) will be the negative of the far.
	union
	{
		struct { tPlane Right, Left, Top, Bottom, Near, Far; };
		tPlane Planes[Plane::NumPlanes];
	};
};


// This function finds a small sphere that encloses the supplied points. The algorithm is 'stable' in that continuous
// changes in source positions do not result in discontinuities in the output sphere. Runs in expected O(n) time and
// finds the smallest, or rather something close to the smallest, sphere that encloses them all. If numpoints is <= 0
// or points is nullptr returns false and does not modify the sphere. For numPoints = 1 returns a sphere with the
// center at the point and radius minRadius. This also happens if all points have identical positions.
//
// Uses the Bouncing Bubble solution to the minimal enclosing ball problem. See
// "https://en.wikipedia.org/wiki/Bounding_sphere#Bouncing_Bubble". The minRadius must not be too close to zero or
// the algorithm will not work. The minRadius is forced to be >= Epsilon if too small a value is passed in. The
// implementation is based on Andres Hernandez's code as shown here:
// http://stackoverflow.com/questions/17331203/bouncing-bubble-algorithm-for-smallest-enclosing-sphere which is
// based on a paper by Tian Bo.
bool tComputeSmallEnclosingSphere(tSphere& result, const tVector3* points, int numPoints, float minRadius = 0.00001);

// This is an older unstable function for computing a minimal bounding sphere. It is based on Graphics Gems vol II.
// Runs fast but is an approximation only. For a precise answer we should use the algorithm found here:
// http://www.geometrictools.com/LibFoundation/Containment/Containment.html which is based on the Emo Welzl algorithm
// that has an expected O(n) running time and is generally much faster than the Megiddo algorithm that guarantees O(n).
// A possible benefit of this unstable implementation is no minimum radius. Performance and result quality have not been
// tested.
bool tComputeSmallEnclosingSphere_Unstable(tSphere& result, const tVector3* points, int numPoints);
float tDistanceToLine(const tVector3& point, const tLine&);

// @todo Extend the use of tIntersectResult to the 3D tests.
enum class tIntersectResult
{
	None,				// No intersection.
	Parallel,
	Point,				// Intersects at a single point.
	Segment,			// Intersects along a line segment.
	Coincident			// Primitives overlap.
};

// Intersections. These functions determine if various primitives intersect or not. If the function has the word 'test'
// in it, only that determination is made. If the function has the word 'find' in it, the location of intersection is
// also computed and returned. 'Find' functions are slower. The functions are named using the convention:
// tIntersectTestPrim1Prim2(...) and tIntersectFindPrim1Prim2(...).

// Finds the intersection point between 2 2D lines.
tIntersectResult tIntersectFindLineLine(const tLine2&, const tLine2&, tVector2& intersection);

// The segment intersection functions are currently not mathematically consistent since 'coincident' is returned if
// the segments lie on the same line even if they do not intersect. What should be returned in this case is either
// 'None' or, if they overlap, a line segment for the overlap region.
tIntersectResult tIntersectFindSegSeg(const tLineSeg2&, const tLineSeg2&, tVector2& intersection);
tIntersectResult tIntersectFindRaySeg(const tRay2&, const tLineSeg2&, tVector2& intersection);

// Returns the number of intersection points found. Only 0, 1, or 2 will ever be returned. You may only read that many
// results from the 'intersection' array. If the ray is lined up with one of the rectangle edges there will not be any
// results for that edge but you will still receive results for the adjacent edges and the function will still return
// 0, 1, or 2. If sides is supplied, it is filled with the tARect2 Side bitfields.
int tIntersectFindRayRect(const tRay2&, const tARect2&, tVector2 intersection[2], uint32* sides = nullptr);

// The tRay.Dir argument must be normalized. This function modifies t such that the intersection point is given by
// tRay.Start + tRay.Dir*t. t holds the closer of the two possible intersections and is always positive. Returns false
// if there was no intersection, true otherwise.
bool tIntersectFindRaySphere(float& t, const tRay&, const tSphere&);
bool tIntersectTestLineSegSphere(const tLineSeg&, const tSphere&);

bool tIntersectTestCapsulePoint(const tCapsule&, const tVector3&);

// This function requires that ray start from the center of the ellipse. The ellipse's center is ignored and so is the
// ray's start. The result 't' is set to how much the ray's dir needs to by multiplied by to land on the ellipse.
bool tIntersectFindRayEllipse(float& t, const tRay2&, const tEllipse2&);

// This test assumes that the triangle faces one direction. An anticlockwise winding of the verts allows the right-hand
// rule to be used in determining the direction of the triangle's normal -- assuming a right-handed coordinate system.
bool tIntersectTestRayTriangle(const tRay&, const tTriangle&);

// Returns true if the sphere is partly or completely inside the volume of the view frustum.
bool tIntersectTestFrustumSphere(const tFrustum&, const tSphere&);

// @todo Not implemented.
bool tIntersectTestTriangleTriangle(const tTriangle&, const tTriangle&);

// @todo Not implemented.
bool IntersectFindTriangleTriangle(tLineSeg& intersection, const tTriangle&, const tTriangle&);


}


// Implementation below this line.


inline void tMath::tARect2::AddPoint(const tVector2& p)
{
	Max.x = tMax(p.x, Max.x);
	Max.y = tMax(p.y, Max.y);
	Min.x = tMin(p.x, Min.x);
	Min.y = tMin(p.y, Min.y);
}


inline void tMath::tARect2::Transform(const tMatrix2& t)
{
	// Transform the four rectangle points to the new space and grow a new rectangle around them.
	tARect2 rect;
	rect.AddPoint(t * tVector2(Min.x, Min.y));
	rect.AddPoint(t * tVector2(Max.x, Min.y));
	rect.AddPoint(t * tVector2(Min.x, Max.y));
	rect.AddPoint(t * tVector2(Max.x, Max.y));
	*this = rect;
}


inline bool tMath::tARect2::Overlaps(const tARect2& rect) const
{
	tVector2 diff = rect.ComputeCenter() - ComputeCenter();
	tVector2 extents = rect.ComputeExtents() + ComputeExtents();
	if ((tAbs(diff.x) <= extents.x) && (tAbs(diff.y) <= extents.y))
		return true;

	return false;
}


inline bool tMath::tARect2::IsPointInsideBias(const tVector2& point, tIntervalBias biasX, tIntervalBias biasY) const
{
	std::function<bool(float,float)> lessX = tBiasLess(biasX);
	std::function<bool(float,float)> grtrX = tBiasGreater(biasX);
	bool inX = grtrX(point.x, Min.x) && lessX(point.x, Max.x);

	std::function<bool(float,float)> lessY = tBiasLess(biasY);
	std::function<bool(float,float)> grtrY = tBiasGreater(biasY);
	bool inY = grtrY(point.y, Min.y) && lessY(point.y, Max.y);

	return inX && inY;
}


inline void tMath::tABox::AddPoint(const tVector3& p)
{
	Max.x = tMax(p.x, Max.x);
	Max.y = tMax(p.y, Max.y);
	Max.z = tMax(p.z, Max.z);

	Min.x = tMin(p.x, Min.x);
	Min.y = tMin(p.y, Min.y);
	Min.z = tMin(p.z, Min.z);
}


inline bool tMath::tABox::IsPointInsideBias(const tVector3& point, tIntervalBias biasX, tIntervalBias biasY, tIntervalBias biasZ) const
{
	std::function<bool(float,float)> lessX = tBiasLess(biasX);
	std::function<bool(float,float)> grtrX = tBiasGreater(biasX);
	bool inX = grtrX(point.x, Min.x) && lessX(point.x, Max.x);

	std::function<bool(float,float)> lessY = tBiasLess(biasY);
	std::function<bool(float,float)> grtrY = tBiasGreater(biasY);
	bool inY = grtrY(point.y, Min.y) && lessY(point.y, Max.y);

	std::function<bool(float,float)> lessZ = tBiasLess(biasZ);
	std::function<bool(float,float)> grtrZ = tBiasGreater(biasZ);
	bool inZ = grtrZ(point.z, Min.z) && lessZ(point.z, Max.z);

	return inX && inY && inZ;
}


inline void tMath::tFrustum::Set(const tPlane planes[6])
{
	for (int p = 0; p < 6; p++)
		Planes[p] = planes[p];
}


inline void tMath::tFrustum::Set(const tVector4 planes[6])
{
	for (int p = 0; p < 6; p++)
		Planes[p] = tPlane(planes[p]);
}


inline bool tMath::tIntersectTestCapsulePoint(const tCapsule& c, const tVector3& p)
{
	return tMath::tIntersectTestLineSegSphere
	(
		tLineSeg(c.A, c.B),
		tSphere(p, c.Radius)
	);
}
