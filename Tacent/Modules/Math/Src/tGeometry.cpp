// tGeometry.cpp
//
// Idealized geometric shapes. These include triangles, circles, boxes, spheres, rays, lines, planes, cylinders,
// capsules, frustums, and ellipses. When the shape primitives have a number in their name, it refers to the space the
// shape is in, not the dimensionality of the shape itself. eg. A tCircle3 is a (2D) circle in R3 while a tCircle2 is a
// 2D circle in R2. For shapes that are in R3 we drop the 3 because the R3 primitives are more general.
//
// Copyright (c) 2006, 2016 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "Math/tGeometry.h"


namespace tMath
{
	static tIntersectResult IntersectFindLineLineHelper(const tLine2& a, const tLine2& b, float& ua, float& ub);
};


bool tMath::tORect2::IsInside(const tVector2& point) const
{
	tVector2 d = point - Center;
	for (int a = 0; a < 2; a++)
	{
		float dot = tAbs(d * Axes[a]);
		if (dot > Extents.E[a])
			return false;
	}

	return true;
}


void tMath::tABox::Transform(const tMatrix4& t)
{
	// Transform the 8 box points to the new space and grow a new box around them.
	tABox box;
	box.AddPoint( (t * tVector4(Min.x, Min.y, Min.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Max.x, Min.y, Min.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Min.x, Max.y, Min.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Max.x, Max.y, Min.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Min.x, Min.y, Max.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Max.x, Min.y, Max.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Min.x, Max.y, Max.z, 1.0f)).GetCartesian() );
	box.AddPoint( (t * tVector4(Max.x, Max.y, Max.z, 1.0f)).GetCartesian() );

	*this = box;
}


bool tMath::tOBox::IsInside(const tVector3& point) const
{
	tVector3 d = point - Center;
	for (int a = 0; a < 3; a++)
	{
		float dot = tAbs(d * Axes[a]);
		if (dot > Extents.E[a])
			return false;
	}

	return true;
}


void tMath::tPlane::ComputeOrthogonalBasisVectors(tVector3& u, tVector3& v) const
{
	// The algorithm is 'householder orthogonalization' as described here:
	// http://math.stackexchange.com/questions/64430/find-extra-arbitrary-two-points-for-a-plane-given-the-normal-and-a-point-that-l
	tVector3 n = Normal;
	n.Normalize();

	// Compute the 'mirror' vector w = (nx+1,ny,nz).
	tVector3 w = n;
	w.x += 1.0f;

	// Compute the 3x3 Householder matrix H = I - 2(wwT/wTw) where T means transpose.
	tMatrix4 wwT = w.MulByTranspose(w);
	float wTw = w*w;
	tMatrix4 H = tMatrix4::identity - 2.0f*(wwT/wTw);

	// Row1 of H will be a unit vector parallel to n. Row 2 and 3 will be unit vectors orthogonal to n and each other.
	// We transpose to get the rows out easily.
	H.Transpose();
	u = H.C2;
	v = H.C3;
}


void tMath::tFrustum::Set(const tMatrix4& mat)
{	
	// We put the matrix in row major format so we can do the trick described
	// in Akenine-Moller&Haines, Real-Time Rendering, 2nd Ed., pp.613-614.
	tMatrix4 m(mat);
	m.Transpose();
	tVector4 t;

	tSub(t, m.C4, m.C1);		Right.Set(t);
	tAdd(t, m.C4, m.C1);		Left.Set(t);
	tSub(t, m.C4, m.C2);		Top.Set(t);
	tAdd(t, m.C4, m.C2);		Bottom.Set(t);
	tSub(t, m.C4, m.C3);		Far.Set(t);
	tAdd(t, m.C4, m.C3);		Near.Set(t);

	Right.Normalize();
	Left.Normalize();
	Top.Normalize();
	Bottom.Normalize();
	Near.Normalize();
	Far.Normalize();
}


bool tMath::tComputeSmallEnclosingSphere(tSphere& sphere, const tVector3* points, int numPoints, float minRadius)
{
	if (!points || (numPoints <= 0))
		return false;

	if (minRadius < Epsilon)
		minRadius = Epsilon;

	float& radius = sphere.Radius;
	tVector3& center = sphere.Center;

	radius = minRadius;
	center = points[0];
	if (numPoints == 1)
		return true;

	for (int t = 0; t < 2; t++)
	{
		for (int p = 0; p < numPoints; p++)
		{
			float distSq = (points[p] - center).LengthSq();
			float radSq = radius*radius;
			if (distSq > radSq)
			{
				float dist = tSqrt(distSq);
				float param = dist / radius;
				float paramInv = 1.0f/param;
				float paramInvSq = paramInv*paramInv;
				float onePlus = 1.0f + paramInvSq;
				float oneMinus = 1.0f - paramInvSq;

				radius = 0.5f * (param + paramInv) * radius;
				center = (onePlus*center + oneMinus*points[p]) / 2.0f;
			}
		}
	}

	for (int p = 0; p < numPoints; p++)
	{
		tVector3 diff = points[p] - center;
		float distSq = diff.LengthSq();
		float radSq = radius*radius;
		if (distSq > radSq)
		{
			float dist = tSqrt(distSq);
			radius = (radius + dist) / 2.0f;
			center += diff * (dist - radius) / dist;
		}
	}

	return true;
}


bool tMath::tComputeSmallEnclosingSphere_Unstable(tSphere& sphere, const tVector3* points, int numPoints)
{
	if (!points || (numPoints <= 0))
		return false;

	float& radius = sphere.Radius;
	tVector3& center = sphere.Center;
	if (numPoints == 1)
	{
		radius = 0.0f;
		center = points[0];
		return true;
	}

	tVector3 xmin(PosInfinity, PosInfinity, PosInfinity);
	tVector3 xmax(NegInfinity, NegInfinity, NegInfinity);
	tVector3 ymin(PosInfinity, PosInfinity, PosInfinity);
	tVector3 ymax(NegInfinity, NegInfinity, NegInfinity);
	tVector3 zmin(PosInfinity, PosInfinity, PosInfinity);
	tVector3 zmax(NegInfinity, NegInfinity, NegInfinity);
	for (int p = 0; p < numPoints; p++)
	{
		const tVector3& point = points[p];
		if (xmin.x > point.x)
			xmin = point;

		if (xmax.x < point.x)
			xmax = point;

		if (ymin.y > point.y)
			ymin = point;

		if (ymax.y < point.y)
			ymax = point;

		if (zmin.z > point.z)
			zmin = point;

		if (zmax.z < point.z)
			zmax = point;
	}

	// Span the distance between min and max squared.
	float xspan = (xmax - xmin).LengthSq();
	float yspan = (ymax - ymin).LengthSq();
	float zspan = (zmax - zmin).LengthSq();

	// Find maximally separated pair.
	tVector3 diam1(xmin);
	tVector3 diam2(xmax);
	float maxspan = xspan;

	if (yspan > maxspan)
	{
		maxspan = yspan;
		diam1 = ymin;
		diam2 = ymax;
	}

	if (zspan > maxspan)
	{
		diam1 = zmin;
		diam2 = zmax;
	}

	// Calculate initial center.
	center.Set( (diam1.x+diam2.x)*0.5f, (diam1.y+diam2.y)*0.5f, (diam1.z+diam2.z)*0.5f );

	// Calculate initial radius.
	tVector3 radVec(diam2.x - center.x, diam2.y - center.y, diam2.z - center.z);
	float radSq = radVec.LengthSq();
	radius = tSqrt(radSq);

	// Second pass. Increment current sphere.
	for (int p = 0; p < numPoints; p++)
	{
		const tVector3& point = points[p];
		tVector3 diff = point - center;
		float distSq = diff.LengthSq();

		// DistSq is guaranteed to be > 0 so the divide is safe.
		if (distSq > radSq)
		{
			float dist = tSqrt(distSq);
			radius = (radius + dist) / 2.0f;
			float diff = dist - radius;
			center = (center*radius + point*diff) / dist;
		}
	}

	return true;
}


float tMath::tDistanceToLine(const tVector3& p, const tLine& l)
{
	tVector3 u = l.B - l.A;
	u.Normalize();
	tVector3 w = p - l.A;
	u %= w;						// Cross product.

	return u.Length();
}


tMath::tIntersectResult tMath::IntersectFindLineLineHelper(const tLine2& a, const tLine2& b, float& ua, float& ub)
{
	// Based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
	float x4mx3 = b.B.x - b.A.x;
	float y1my3 = a.A.y - b.A.y;
	float y4my3 = b.B.y - b.A.y;
	float x1mx3 = a.A.x - b.A.x;
	float x2mx1 = a.B.x - a.A.x;
	float y2my1 = a.B.y - a.A.y;

	float d  = y4my3*x2mx1 - x4mx3*y2my1;
	float na = x4mx3*y1my3 - y4my3*x1mx3;
	float nb = x2mx1*y1my3 - y2my1*x1mx3;
	if (d == 0.0f)
	{
		// We're parallel, possibly coincident.
		if ((na == 0.0f) || (nb == 0.0f))
			return tIntersectResult::Coincident;
		else
			return tIntersectResult::Parallel;
	}

	ua = na/d;
	ub = nb/d;
	return tIntersectResult::Point;
}


tMath::tIntersectResult tMath::tIntersectFindLineLine(const tLine2& a, const tLine2& b, tVector2& i)
{
	float ua, ub;
	tIntersectResult r = IntersectFindLineLineHelper(a, b, ua, ub);
	if (r != tIntersectResult::Point)
		return r;

	i = a.A + ua*(a.B - a.A);
	return tIntersectResult::Point;
}


tMath::tIntersectResult tMath::tIntersectFindSegSeg(const tLineSeg2& a, const tLineSeg2& b, tVector2& i)
{
	float ua, ub;
	tIntersectResult r = IntersectFindLineLineHelper(a, b, ua, ub);
	if (r != tIntersectResult::Point)
		return r;

	if (tInRange(ua, 0.0f, 1.0f) && tInRange(ub, 0.0f, 1.0f))
	{
		i = a.A + ua*(a.B - a.A);
		return tIntersectResult::Point;
	}

	return tIntersectResult::None;
}


tMath::tIntersectResult tMath::tIntersectFindRaySeg(const tRay2& r, const tLineSeg2& b, tVector2& i)
{
	tLine2 a(r.Start, r.Start+r.Dir);

	float ua, ub;
	tIntersectResult res = IntersectFindLineLineHelper(a, b, ua, ub);
	if (res != tIntersectResult::Point)
		return res;

	if ((ua >= 0.0f) && tInRange(ub, 0.0f, 1.0f))
	{
		i = a.A + ua*(a.B - a.A);
		return tIntersectResult::Point;
	}

	return tIntersectResult::None;
}


int tMath::tIntersectFindRayRect(const tRay2& r, const tARect2& b, tVector2 ia[2], uint32* sides)
{
	int numIntersections = 0;

	// The box is made of 4 segments that we test in turn.
	tVector2 tl(b.Min.x, b.Max.y);
	tVector2 br(b.Max.x, b.Min.y);
	tLineSeg2 seg;
	tVector2 i;
	if (sides)
		*sides = 0;

	// Bottom segment.
	seg.A = b.Min;
	seg.B = br;
	if (tIntersectFindRaySeg(r, seg, i) == tIntersectResult::Point)
	{
		ia[numIntersections++] = i;
		if (sides)
			*sides |= tARect2::Side_Bottom;
	}

	// Right segment.
	seg.A = br;
	seg.B = b.Max;
	if (tIntersectFindRaySeg(r, seg, i) == tIntersectResult::Point)
	{
		ia[numIntersections++] = i;
		if (sides)
			*sides |= tARect2::Side_Right;
	}
	if (numIntersections == 2)
		return numIntersections;

	// Top segment.
	seg.A = b.Max;
	seg.B = tl;
	if (tIntersectFindRaySeg(r, seg, i) == tIntersectResult::Point)
	{
		ia[numIntersections++] = i;
		if (sides)
			*sides |= tARect2::Side_Top;
	}
	if (numIntersections == 2)
		return numIntersections;

	// Left segment.
	seg.A = tl;
	seg.B = b.Min;
	if (tIntersectFindRaySeg(r, seg, i) == tIntersectResult::Point)
	{
		ia[numIntersections++] = i;
		if (sides)
			*sides |= tARect2::Side_Left;
	}

	return numIntersections;
}


bool tMath::tIntersectFindRaySphere(float& t, const tRay& ray, const tSphere& sphere)
{
	const tVector3& p = ray.Start;
	const tVector3& d = ray.Dir;
	const tVector3& c = sphere.Center;
	const float& r = sphere.Radius;

	// rayDir must have length 1. Solution is quadratic equation with A=1 and B, C as below.
	float B = 2.0f*( d.x*(p.x-c.x) + d.y*(p.y-c.y) + d.z*(p.z-c.z) );
	float C = (p.x-c.x)*(p.x-c.x) + (p.y-c.y)*(p.y-c.y) + (p.z-c.z)*(p.z-c.z) - r*r;

	float discriminant = B*B - 4.0f*C;
	if (discriminant < 0.0f)
		return false;

	float root = tSqrt(discriminant);

	t = (-B - root)/2.0f;

	// May need the other root.
	if (t < 0.0f)
		t = (-B + root)/2.0f;

	if (t < 0.0f)
		// The sphere is behind us.
		return false;

	// The intersection = p + d*t.
	// t0, t1 = (- B + (B^2 - 4*C)^1/2) / 2 where t0 is for (-) and t1 is for (+).
	// If the discriminant is < 0.0 then there is no real root and no intersection. If there is a real root
	// (discriminant > = 0.0) then the smaller positive root is the closest intersection point. We compute t0 and if it
	// is positive we are done. If it is negative we compute t1. The intersection point is given by: 
	// Ri = [xi, yi, zi] = [x0 + xd * ti ,  y0 + yd * ti,  z0 + zd * ti]
	return true;
}


bool tMath::tIntersectTestLineSegSphere(const tLineSeg& segmentAB, const tSphere& sphere)
{
	tRay rayAB(segmentAB);
	tLineSeg segmentBA(segmentAB.B, segmentAB.A);
	tRay rayBA(segmentBA);

	float u, v;
	if (tIntersectFindRaySphere(u, rayAB, sphere) && tIntersectFindRaySphere(v, rayBA, sphere))
		return true;
	else
		return false;
}


bool tMath::tIntersectFindRayEllipse(float& t, const tRay2& ray, const tEllipse2& ellipse)
{
	float r1 = ellipse.RadiusX;
	float r2 = ellipse.RadiusY;
	tAssert(r1 && r2);

	float dx = ray.Dir.x;
	float dy = ray.Dir.y;
	tAssert(dx || dy);

	float discriminant = 1.0f/( (dx*dx/(r1*r1)) + (dy*dy/(r2*r2)) );
	if (discriminant < 0.0f)
		return false;

	t = tSqrt(discriminant);
	return true;
}


bool tMath::tIntersectTestRayTriangle(const tRay& ray, const tTriangle& tri)
{
	// A reconditioned version of the Segura-Feito method.
	tVector3 a = tri.A - ray.Start;
	tVector3 b = tri.B - ray.Start;
	tVector3 c = tri.C - ray.Start;

	tVector3 t;
	tCross(t, a, ray.Dir);
	if (c*t > 0.0f)
		return false;

	if (b*t < 0.0f)
		return false;

	tCross(t, b, ray.Dir);
	if (c*t < 0.0f)
		return false;

	return true;
}


bool tMath::tIntersectTestFrustumSphere(const tFrustum& f, const tSphere& s)
{
	for (int p = 0; p < int(tFrustum::Plane::NumPlanes); p++)
	{
		// Use the sphere's center in the normalized plane equation to get the projected distance to plane.
		if (s.Center * f.Planes[p].Normal + f.Planes[p].Distance < -s.Radius)
			return false;
	}

	return true;
}
