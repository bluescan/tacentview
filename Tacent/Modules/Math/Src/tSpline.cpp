// tSpline.cpp
//
// Implementations for splines and paths with various degrees of smoothness. A 'path', or 'spline', is arbitrarily long
// and may be composed of smaller path sections called 'curves'. For example, a Bezier path is made from multiple
// Bezier curves. Implementations for piecewise-linear paths, Hermite/Bezier curves and paths, and Nonuniform
// Nonrational Cubic-Basis splines can be found in this file.
//
// Regarding naming, the word 'spline' refers to any path that is composed of piecewise parts. Strictly speaking one
// could call a composite of multiple Bezier curves a 'Bezier Spline' but it is not a common term. In this file the
// word 'path' is used for a composite of Bezier curves or a composite of line segments, and we reserve the word spline
// for paths composed of multiple cubic polynomial pieces.
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tStandard.h>
#include "Math/tSpline.h"


void tMath::tPiecewiseLinearPath::SetPoints(const tVector3* points, int numPoints)
{
	tAssert(numPoints > 1);
	Clear();
	NumPoints = numPoints;
	Points = new tVector3[numPoints];
	tStd::tMemcpy(Points, points, numPoints*sizeof(tVector3));
	PercentagePoints = new float[numPoints];

	// Fill in the percentage points array with distances.
	PercentagePoints[0] = 0.0f;
	TotalLength = 0.0f;
	for (int p = 0; p < NumPoints-1; p++)
	{
		TotalLength += tDistBetween(Points[p], Points[p+1]);
		PercentagePoints[p+1] = TotalLength;
	}

	// Now modify the percentage points array so it represents percentages.
	PercentagePoints[NumPoints-1] = 1.0f;
	for (int p = 1; p < NumPoints-1; p++)
	{
		if (TotalLength)
			PercentagePoints[p] /= TotalLength;
		else
			PercentagePoints[p] = 0.0f;
	}
}


tMath::tVector3 tMath::tPiecewiseLinearPath::GetPoint(float t) const
{
	if (!IsValid())
		return tVector3(0.0f, 0.0f, 0.0f);

	// We need the slow truncating cast here.
	int firstIndex = int(t);
	int lastIndex = firstIndex + 1;
	if (t < 0.0f)
	{
		firstIndex = 0;
		lastIndex = 1;
	}
	else if (t > float(NumPoints - 1))
	{
		firstIndex = NumPoints - 2;
		lastIndex = NumPoints - 1;
	}

	t -= float(firstIndex);
	tVector3& first = Points[firstIndex];
	tVector3& last = Points[lastIndex];

	float x = tLesc(t, first.x, last.x);
	float y = tLesc(t, first.y, last.y);
	float z = tLesc(t, first.z, last.z);

	return tVector3(x, y, z);
}


tMath::tVector3 tMath::tPiecewiseLinearPath::GetPercentPoint(float t) const
{
	if (!IsValid())
		return tVector3::zero;

	tSaturate(t);
	tVector3* a = 0;
	tVector3* b = 0;
	float pa = 0.0f;
	float pb = 0.0f;

	for (int p = 0; p < NumPoints-1; p++)
	{
		pa = PercentagePoints[p];
		pb = PercentagePoints[p+1];
		if ((t >= pa) && (t <= pb))
		{
			a = Points + p;
			b = Points + p + 1;
			break;
		}
	}
	tAssert(a && b);

	t = (t-pa) / (pb-pa);
	float x = tLesc(t, a->x, b->x);
	float y = tLesc(t, a->y, b->y);
	float z = tLesc(t, a->z, b->z);

	return tVector3(x, y, z);
}


void tMath::tBezierCurve::GetPoint(tVector3& point, float t) const
{
	tAssert((t >= 0.0f) && (t <= 1.0f));
	float c = 1.0f - t;

	// The Bernstein polynomials.
	float bb0 = c*c*c;
	float bb1 = 3*t*c*c;
	float bb2 = 3*t*t*c;
	float bb3 = t*t*t;

	point = ControlVerts[0]*bb0 + ControlVerts[1]*bb1 + ControlVerts[2]*bb2 + ControlVerts[3]*bb3;
}


void tMath::tBezierCurve::GetTangent(tVector3& v, float t) const
{
	// See: http://bimixual.org/AnimationLibrary/beziertangents.html
	tAssert((t >= 0.0f) && (t <= 1.0f));

	const tVector3 q0 = ControlVerts[0] + ((ControlVerts[1] - ControlVerts[0]) * t);
	const tVector3 q1 = ControlVerts[1] + ((ControlVerts[2] - ControlVerts[1]) * t);
	const tVector3 q2 = ControlVerts[2] + ((ControlVerts[3] - ControlVerts[2]) * t);

	const tVector3 r0 = q0 + ((q1 - q0) * t);
	const tVector3 r1 = q1 + ((q2 - q1) * t);

	v = r1 - r0;
}


float tMath::tBezierCurve::GetClosestParamRec(const tVector3& p, tComponents components, float start, float end, float threshold) const
{
	float mid = (start + end)/2.0f;

	// Base case for recursion.
	if ((end - start) < threshold)
		return mid;

	// The two halves have param range [start, mid] and [mid, end]. We decide which one to use by using a midpoint param
	// calculation for each section.
	float paramA = (start+mid) / 2.0f;
	float paramB = (mid+end) / 2.0f;
	
	tVector3 pos(p);
	pos.Zero(~components);

	tVector3 posA;
	GetPoint(posA, paramA);
	posA.Zero(~components);

	tVector3 posB;
	GetPoint(posB, paramB);
	posB.Zero(~components);

	float distASq = (posA - pos).LengthSq();
	float distBSq = (posB - pos).LengthSq();

	if (distASq < distBSq)
		end = mid;
	else
		start = mid;

	// The (tail) recursive call.
	return GetClosestParamRec(p, components, start, end, threshold);
}


tMath::tComponents tMath::tBezierPath::tFastSectionState::CompareComponents = tMath::tComponent_All;
tMath::tVector3 tMath::tBezierPath::tFastSectionState::ComparePos = tVector3::zero;
const tMath::tVector3* tMath::tBezierPath::tFastSectionState::CompareControlVerts = nullptr;


tMath::tBezierPath::tBezierPath(const tBezierPath& src) :
	Mode(src.Mode),
	Type(src.Type),
	NumCurveSegments(src.NumCurveSegments),
	NumControlVerts(src.NumControlVerts)
{
	if (!src.IsValid())
	{
		ControlVerts = nullptr;
		return;
	}

	switch (Mode)
	{
		case tMode::InternalCVs:
			ControlVerts = new tVector3[NumControlVerts];
			tStd::tMemcpy(ControlVerts, src.ControlVerts, sizeof(tVector3) * NumControlVerts);
			break;

		case tMode::ExternalCVs:
			ControlVerts = src.ControlVerts;
			break;
	}
}


void tMath::tBezierPath::Clear()
{
	if (Mode == tMode::InternalCVs)
		delete[] ControlVerts;
	ControlVerts = 0;

	Mode = tMode::InternalCVs;
	Type = tType::Open;
	NumCurveSegments = 0;
	NumControlVerts = 0;
}


float tMath::tBezierPath::ComputeApproxParamPerCoordinateUnit(tComponents components) const
{
	if (!IsValid())
		return 0.0f;

	// For a closed path this still works if you consider the last point as separate from the first. That is, a closed
	// path is just like an open except the last interpolated point happens to match the first.
	int numInterpolatedPoints = NumCurveSegments + 1;
	if (numInterpolatedPoints < 2)
		return 0.0f;

	float totalDist = 0.0f;
	for (int n = 1; n < numInterpolatedPoints; n++)
	{
		tVector3 a = ControlVerts[(n-1)*3];
		tVector3 b = ControlVerts[n*3];
		a.Zero(~components);
		b.Zero(~components);
		totalDist += tDistBetween(a, b);
	}

	if (totalDist == 0.0f)
		return 0.0f;

	return float(NumCurveSegments) / totalDist;
}


void tMath::tBezierPath::InterpolatePoints(const tVector3* knots, int numKnots, tType type)
{
	tAssert(numKnots >= 2);
	Clear();

	Mode = tMode::InternalCVs;
	Type = type;
	switch (Type)
	{
		case tType::Open:
		{
			NumCurveSegments = numKnots - 1;
			NumControlVerts = 3*numKnots - 2;
			ControlVerts = new tVector3[NumControlVerts];

			// Place the interpolated CVs.
			for (int n = 0; n < numKnots; n++)
				ControlVerts[n*3] = knots[n];

			// Place the first and last non-interploated CVs.
			tVector3 initialPoint = (knots[1] - knots[0]) * 0.25f;

			// Interp 1/4 away along first segment.
			ControlVerts[1] = knots[0] + initialPoint;
			tVector3 finalPoint = (knots[numKnots-2] - knots[numKnots-1]) * 0.25f;

			// Interp 1/4 backward along last segment.
			ControlVerts[NumControlVerts-2] = knots[numKnots-1] + finalPoint;

			// Now we'll do all the interior non-interpolated CVs.
			for (int k = 1; k < NumCurveSegments; k++)
			{
				tVector3 a = knots[k-1] - knots[k];
				tVector3 b = knots[k+1] - knots[k];

				float aLen = a.Length();
				float bLen = b.Length();

				if (aLen && bLen)
				{
					float abLen = (aLen + bLen) / 8.0f;
					tVector3 ab = (b/bLen) - (a/aLen);
					ab.Normalize();
					ab *= abLen;

					ControlVerts[k*3 - 1] = knots[k] - ab;
					ControlVerts[k*3 + 1] = knots[k] + ab;
				}
				else
				{
					ControlVerts[k*3 - 1] = knots[k];
					ControlVerts[k*3 + 1] = knots[k];
				}
			}
			break;
		}

		case tType::Closed:
		{
			NumCurveSegments = numKnots;

			// We duplicate the first point at the end so we have contiguous memory to look of the curve value. That's
			// what the +1 is for.
			NumControlVerts = 3*numKnots + 1;
			ControlVerts = new tVector3[NumControlVerts];

			// First lets place the interpolated CVs and duplicate the first into the last CV slot.
			for (int n = 0; n < numKnots; n++)
				ControlVerts[n*3] = knots[n];

			ControlVerts[NumControlVerts-1] = knots[0];

			// Now we'll do all the interior non-interpolated CVs. We go to k=NumCurveSegments which will compute the
			// two CVs around the zeroeth knot (points[0]).
			for (int k = 1; k <= NumCurveSegments; k++)
			{
				int modkm1 = k-1;
				int modkp1 = (k+1) % NumCurveSegments;
				int modk = k % NumCurveSegments;

				tVector3 a = knots[modkm1] - knots[modk];
				tVector3 b = knots[modkp1] - knots[modk];

				float aLen = a.Length();
				float bLen = b.Length();

				int mod3km1 = 3*k - 1;

				// Need the -1 so the end point is a duplicated start point.
				int mod3kp1 = (3*k + 1) % (NumControlVerts-1);
				if (aLen && bLen)
				{
					float abLen = (aLen + bLen) / 8.0f;
					tVector3 ab = (b/bLen) - (a/aLen);
					ab.Normalize();
					ab *= abLen;

					ControlVerts[mod3km1] = knots[modk] - ab;
					ControlVerts[mod3kp1] = knots[modk] + ab;
				}
				else
				{
					ControlVerts[mod3km1] = knots[modk];
					ControlVerts[mod3kp1] = knots[modk];
				}
			}
			break;
		}
	}
}


void tMath::tBezierPath::SetControlVerts(const tVector3* cvs, int numCVs, tMode mode, tType type)
{
	tAssert(cvs);
	tAssert( ((type == tType::Open) && (numCVs >= 4)) || ((type == tType::Closed) && (numCVs >= 7)) );
	tAssert(((numCVs-1) % 3) == 0);

	Clear();
	Mode = mode;
	Type = type;

	NumControlVerts = numCVs;
	NumCurveSegments = ((numCVs - 1) / 3);
	switch (Mode)
	{
		case tMode::InternalCVs:
			ControlVerts = new tVector3[NumControlVerts];
			tStd::tMemcpy(ControlVerts, cvs, sizeof(tVector3) * NumControlVerts);
			break;

		case tMode::ExternalCVs:
			// Since the mode is external we know they won't be deleted so it's safe to cast the const away.
			ControlVerts = (tVector3*)cvs;
			break;
	}
}


void tMath::tBezierPath::GetPoint(tVector3& point, float t) const
{
	if (!IsValid())
		return;

	// Only closed paths accept t values out of range.
	if (Type == tType::Closed)
	{
		while (t < 0.0f)
			t += float(NumCurveSegments);

		while (t > float(NumCurveSegments))
			t -= float(NumCurveSegments);
	}
	else
	{
		tClamp(t, 0.0f, float(NumCurveSegments));
	}

	// Segment 0 is for t E [0, 1). The last segment is for t E [NumCurveSegments-1, NumCurveSegments].
	// The following 'if' statement deals with the final inclusive bracket on the last segment. The cast must truncate.
	int segment = int(t);
	if (segment >= NumCurveSegments)
		segment = NumCurveSegments - 1;

	tBezierCurve bc( &ControlVerts[3*segment] );
	bc.GetPoint(point, t - float(segment));
}


void tMath::tBezierPath::GetTangent(tVector3& v, float t) const
{
	if (!IsValid())
		return;

	// Only closed paths accept t values out of range.
	if (Type == tType::Closed)
	{
		while (t < 0.0f)
			t += float(NumCurveSegments);

		while (t > float(NumCurveSegments))
			t -= float(NumCurveSegments);
	}
	else
	{
		tClamp(t, 0.0f, float(NumCurveSegments));
	}

	// Segment 0 is for t E [0, 1). The last segment is for t E [NumCurveSegments-1, NumCurveSegments].
	// The following 'if' statement deals with the final inclusive bracket on the last segment. The cast must truncate.
	int segment = int(t);
	if (segment >= NumCurveSegments)
		segment = NumCurveSegments - 1;

	tBezierCurve bc( &ControlVerts[3*segment] );
	bc.GetTangent(v, t - float(segment));
}


bool tMath::tBezierPath::tFastSectionState::CompareSections(const tSectionInfo& a, const tSectionInfo& b)
{
	// My attempt at a stable section sorting function. Note that the versions that used all 4 points individually
	// fail to be stable because they are dependent on the particular sections being compared.
	tVector3 approxAvgA = (CompareControlVerts[a.StartIndex] + CompareControlVerts[a.StartIndex + 3]) / 2.0f;
	tVector3 approxAvgB = (CompareControlVerts[b.StartIndex] + CompareControlVerts[b.StartIndex + 3]) / 2.0f;

	approxAvgA.Zero(~CompareComponents);
	approxAvgB.Zero(~CompareComponents);

	float distSqA = tDistBetweenSq(ComparePos, approxAvgA);
	float distSqB = tDistBetweenSq(ComparePos, approxAvgB);
	return distSqA < distSqB;
}


void tMath::tBezierPath::tFastSectionState::Set(const tVector3& pos, tComponents components, const tVector3* cvs, int numCVs) const
{
	Clear();
	Components = components;
	for (int p = 0; p < numCVs - 1; p += 3)
		Sections.Append(new tSectionInfo(p));

	// @todo There's gotta be a more modern way to do this. Maybe with a closure.
	CompareComponents = Components;
	ComparePos = pos;
	CompareControlVerts = cvs;
	Sections.Sort(CompareSections, tListSortAlgorithm::Merge);
}


void tMath::tBezierPath::tFastSectionState::Update(const tVector3& pos, const tVector3* cvs) const
{
	CompareComponents = Components;
	ComparePos = pos;
	CompareControlVerts = cvs;
	Sections.Sort(CompareSections, tListSortAlgorithm::Bubble);
}


float tMath::tBezierPath::GetClosestParam(const tVector3& p, tComponents components, float paramThreshold, const tBezierPath::tFastSectionState& optObj) const
{
	// Can we use the supplied FastSectionState if it has the correct components, number of sections, and CVs. If
	// anything is wrong, we simply clear the optimization object for next time.
	if (((optObj.Sections.GetNumItems() * 3) + 1) != NumControlVerts)
		optObj.Clear();

	if (optObj.Components != components)
		optObj.Clear();

	if (!optObj.Sections.GetNumItems())
		optObj.Set(p, components, ControlVerts, NumControlVerts);
	else
		optObj.Update(p, ControlVerts);

	// We try the first 3 sections. In most cases the closest will be in the first but due to the nature of the sort
	// function, it is prudent to check one on either side.
	tFastSectionState::tSectionInfo* section = optObj.Sections.First();
	float minDistSq = MaxFloat;
	float closestParam = 0.0f;
	tVector3 pos(p);
	pos.Zero(~components);

	for (int s = 0; (s < 3) && section; s++, section = section->Next())
	{
		tBezierCurve curve(ControlVerts + section->StartIndex);
		float curveClosestParam = curve.GetClosestParam(pos, components, paramThreshold);

		tVector3 curvePos;
		curve.GetPoint(curvePos, curveClosestParam);
		curvePos.Zero(~components);
		float distSq = (curvePos - pos).LengthSq();
		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			float startParam = float(section->StartIndex) / 3.0f;
			closestParam = startParam + curveClosestParam;
		}
	}

	return closestParam;
}


tMath::tNNBSpline::tNNBSpline(int* knotValueSequence, tVector3* controlPoints, int numControlPoints, float paramRange) :
	KVS(knotValueSequence)
{
	tAssert(!"tNNBSpline not implemented.");
}


tMath::tNNBSpline::~tNNBSpline()
{
}


tMath::tVector3 tMath::tNNBSpline::Q(int i, float t)
{
	tVector3 t1 = CPs[i-3] * B4(i-3, t);
	tVector3 t2 = CPs[i-2] * B4(i-2, t);
	tVector3 t3 = CPs[i-1] * B4(i-1, t);
	tVector3 t4 = CPs[i  ] * B4(i  , t);

	return t1 + t2 + t3 + t4;
}


float tMath::tNNBSpline::B1(int i, float t)
{
	if ( (KVS[i] <= t) && (t < KVS[i+1]) )
		return 1.0f;
	else
		return 0.0f;
}


float tMath::tNNBSpline::B2(int i, float t)
{
	int ti0 = KVS[i];
	int ti1 = KVS[i+1];
	int ti2 = KVS[i+2];

	int denom1 = ti1-ti0;
	int denom2 = ti2-ti1;

	float term1 = 0;
	if (denom1)
		term1 = B1(i,t) * (t-ti0)/float(denom1);

	float term2 = 0;
	if (denom2)
		term2 = B1(i+1, t) * (ti2-t)/float(denom2);

	return term1 + term2;
}


float tMath::tNNBSpline::B3(int i, float t)
{
	int ti0 = KVS[i];
	int ti1 = KVS[i+1];
	int ti2 = KVS[i+2];
	int ti3 = KVS[i+3];

	int denom1 = ti2-ti0;
	int denom2 = ti3-ti1;

	float term1 = 0;
	if (denom1)
		term1 = B2(i,t) * (t-ti0)/float(denom1);

	float term2 = 0;
	if (denom2)
		term2 = B2(i+1, t) * (ti3-t)/float(denom2);

	return term1 + term2;
}


float tMath::tNNBSpline::B4(int i, float t)
{
	int ti0 = KVS[i];
	int ti1 = KVS[i+1];
	int ti3 = KVS[i+2];
	int ti4 = KVS[i+3];

	int denom1 = ti3-ti0;
	int denom2 = ti4-ti1;

	float term1 = 0;
	if (denom1)
		term1 = B3(i,t) * (t-ti0)/float(denom1);

	float term2 = 0;
	if (denom2)
		term2 = B3(i+1, t) * (ti4-t)/float(denom2);

	return term1 + term2;
}
