// tSpline.h
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

#pragma once
#include <Foundation/tList.h>
#include "Math/tLinearAlgebra.h"
#include "Math/tVector3.h"
namespace tMath
{


class tPiecewiseLinearPath
{
public:
	tPiecewiseLinearPath();

	// The points array is copied during construction. The points are interpolated and numPoints must be >= 2.
	tPiecewiseLinearPath(const tVector3* points, int numPoints);
	tPiecewiseLinearPath(const tPiecewiseLinearPath& src);
	virtual ~tPiecewiseLinearPath()																						{ delete[] Points; delete[] PercentagePoints; }

	// Removes any previous points and uses the new ones.
	void SetPoints(const tVector3* points, int numPoints);

	// If t E [0, numPoints-1] where numPoints-1 is the number of path segments, this function returns a point on the
	// path between the first and last points. For t outside this range linear extrapolation is performed.
	tVector3 GetPoint(float t) const;

	// Same as GetPoint except when t E [0.0, 1.0] it returns the position that 'percentage' along the path. t = 0 is
	// the beginning of the path, and t = 1 is the end.
	tVector3 GetPercentPoint(float t) const;

	bool IsValid() const																								{ return Points ? true : false; }
	void Clear();
	int GetNumCurveSegments() const																						{ return NumPoints - 1; }
	float GetLength() const																								{ return TotalLength; }

private:
	int NumPoints;

	// The full list of control points managed by this class.
	tVector3* Points;

	// Each element is the percentage along the path of the corresponding point.
	float* PercentagePoints;
	float TotalLength;
};


// A tBezierCurve represents a single segment of a Bezier path. It knows how to interpret 4 CVs using the Bezier basis
// functions. The class implements cubic Bezier curves -- not linear or quadratic.
class tBezierCurve
{
public:
	// The 4 Bezier control points read by this class are managed externally and are not deleted by this class. Make
	// sure they exist for the lifetime of this object.
	tBezierCurve(tVector3* points)																						: ControlVerts(points) { }
	void GetPoint(tVector3&, float t) const;				// t E [0, 1].
	void GetTangent(tVector3&, float t) const;				// t E [0, 1].
	float GetClosestParam
	(
		const tVector3& pos, tComponents components = tMath::tComponent_All,
		float paramThreshold = tMath::Epsilon
	) const																												{ return GetClosestParamRec(pos, components, 0.0f, 1.0f, paramThreshold); }

private:
	float GetClosestParamRec(const tVector3& pos, tComponents components, float beginT, float endT, float thresholdT) const;
	const tVector3* ControlVerts;							// Not owned by this class.
};


// A tBezierPath is made of a collection of Bezier curves. If two points are supplied they become the end points of one
// BezierCurve and the 2 interior CVs are generated, creating a small straight line. For 3 points the middle point will
// be on both BezierCurves and each curve will have equal tangents at that point.
class tBezierPath
{
public:
	enum class tMode
	{
		InternalCVs,					// Supplied points are copied on construction and managed by BezierPath.
		ExternalCVs						// Points are referenced by the BezierCurve and must have required lifetime.
	};

	enum class tType
	{
		Open,
		Closed
	};

	// Construct an initially invalid path. Call SetControlVerts or InterpolatePoints sometime later.
	tBezierPath()																										: Mode(tMode::InternalCVs), Type(tType::Open), NumCurveSegments(0), NumControlVerts(0), ControlVerts(nullptr) { }

	// The term 'knot' is another name for a point right on the path (an interpolated point). With this constructor the
	// knots are supplied and interpolated. NumKnots must be >= 2. Interior Cvs are generated transparently and
	// automatically. The points array is copied on construction and the mode is set to InternalCVs.
	tBezierPath(const tVector3* knots, int numKnots, tType type = tType::Open)											: Mode(tMode::InternalCVs), Type(tType::Open), NumCurveSegments(0), NumControlVerts(0), ControlVerts(nullptr) { InterpolatePoints(knots, numKnots, type); }

	// This constructor allows all CVs including non-interpolated ones to be set. Gives fine control over the tangents.
	// For open paths numCVs must be some multiple of 3 plus an initial minimum of 4 CVs (4, 7, 10, 13, etc). For a
	// closed path the last CV must match the first and the minimum number is 7 (7, 10, 13, 16, etc).
	tBezierPath(const tVector3* CVs, int numCVs, tMode mode, tType type = tType::Open)									: Mode(tMode::InternalCVs), Type(tType::Open), NumCurveSegments(0), NumControlVerts(0), ControlVerts(nullptr) { SetControlVerts(CVs, numCVs, mode, type); }

	// The copy constructor retains the source's ownership mode and copies the points only if mode is InternalCVs. 
	tBezierPath(const tBezierPath&);
	virtual ~tBezierPath()																								{ Clear(); }

	tType GetType() const																								{ return Type; }
	bool IsClosed() const																								{ return (Type == tType::Closed) ? true : false; }
	bool IsValid() const																								{ return (NumCurveSegments > 0) ? true : false; }
	void Clear();

	// A closed path will have one more segment than an open for the same number of interpolated points.
	int GetNumCurveSegments() const																						{ return NumCurveSegments; }
	float GetMaxParam() const																							{ return float(NumCurveSegments); }

	// Access to the raw CVs.
	int GetNumControlVerts() const																						{ return NumControlVerts; }
	const tVector3* GetControlVerts() const																				{ return ControlVerts; }
	float ComputeApproxParamPerCoordinateUnit(tComponents = tComponent_All) const;

	// Sets the mode to internal and interpolates the supplied points. Internally generates any interior CVs. numPoints
	// must be >= 2. @todo Allow the mode to be set by returning the new array of CVs if the mode is external.
	void InterpolatePoints(const tVector3* knots, int numKnots, tType);

	// Properly deletes any owned previous points. If the new mode is external the CVs must remain valid for the
	// lifetime of the path. For internal mode the CVs are copied. For a closed path the last CV must match the first.
	void SetControlVerts(const tVector3* CVs, int numCVs, tMode, tType);

	// t E [0, numSegments]. If the type is closed, the number of segments is one more than the equivalent open path.
	void GetPoint(tVector3&, float t) const;

	// Does the same as GetPoint except that t is normalized to be E [0, 1] over all segments. The beginning of the curve
	// is at t = 0 and the end at t = 1. Closed paths allow a value bigger than 1 in which case they loop.
	void GetPointNorm(tVector3& point, float t) const																	{ GetPoint(point, t * float(NumCurveSegments)); }

	// Similar to GetPoint but returns the tangent at the specified point on the path. The tangent is not normalized.
	// The longer the tangent the 'more influence' it has pulling the path in that direction.
	void GetTangent(tVector3&, float t) const;

	// Does the same as GetYangent except that t is normalized to be E [0, 1] over all segments.
	void GetTangentNorm(tVector3& tangent, float t) const																{ GetTangent(tangent, t * float(NumCurveSegments)); }

	// This is an _optional_ object the client may create and maintain to make the GetClosestParam function work far
	// more quickly when the position being passed in is not jumping around. The client is not required to call the
	// member functions of this object.
	struct tFastSectionState
	{
		tFastSectionState()																								: Components(tComponent_All), Sections(true) { }
		tFastSectionState(const tFastSectionState&);
		tFastSectionState& operator=(const tFastSectionState&);
		~tFastSectionState()																							{ }

		void Clear() const																								{ Sections.Empty(); Components = tComponent_All; }

		// Initializes the state. A bit slower than update.
		void Set(const tVector3& pos, tComponents components, const tVector3* cvs, int numCVs) const;
		void Update(const tVector3& pos, const tVector3* cvs) const;

		struct tSectionInfo : public tLink<tSectionInfo>
		{
			tSectionInfo(int i)																							: StartIndex(i) { }
			int StartIndex;
		};

		static tComponents CompareComponents;
		static tVector3 ComparePos;
		static const tVector3* CompareControlVerts;
		static bool CompareSections(const tSectionInfo& a, const tSectionInfo& b);

		mutable tComponents Components;
		mutable tList<tSectionInfo> Sections;
	};

	// This function returns a single closest point. There may be more than one point on the path at the same distance.
	// Use ComputeApproxParamPerCoordinateUnit to determine a good paramThreshold. eg. Working in 3D in meters and you
	// want a 15cm threshold, use: paramThreshold = ComputeApproxParamPerCoordinateUnit(X | Y | Z) * 0.15f.
	float GetClosestParam
	(
		const tVector3& pos, uint32 coords, float paramThreshold,
		const tFastSectionState& = tFastSectionState()
	) const;

	// Same as GetClosestParam but returns the param normalized to [0.0, 1.0].
	float GetClosestParamNorm
	(
		const tVector3& pos, uint32 coords, float paramThreshold,
		const tFastSectionState& optObj = tFastSectionState()
	) const																												{ return GetClosestParam(pos, coords, paramThreshold, optObj) / float(NumCurveSegments); }

private:
	tMode Mode;
	tType Type;
	int NumCurveSegments;
	int NumControlVerts;
	tVector3* ControlVerts;
};


// A Nonuniform, Nonrational, Cubic Basis Spline. Nonuniform allows knots to be spaced unevenly in the parametric
// parameter t. Nonrational means no ratio of polynomials, so no conics and not invariant under perspective
// transformations.
class tNNBSpline
{
public:
	// The number of values in the knotValueSeq is assumed to be 4 greater than numControlPoints. All memory is managed
	// by whoever calls this constructor and is not given to tNNBSpline. The paramRange var specifies how
	// large t may get in subsequent GetPoint calls.
	tNNBSpline(int* knotValueSeq, tVector3* CVs, int numCVs, float paramRange = 100.0f);
	virtual ~tNNBSpline();

	// In this call the t value should range from 0.0 to paramRange inclusive.
	tVector3 GetPoint(float t);

private:
	// Curve segment Qi is defined over 3 <= i < m, t sub i <= t < t sub (i+1)
	tVector3 Q(int i, float t);

	// Blending functions. Internal t values are as specified as in Foley/Van Damm. Different from externally seen t.
	float B1(int i, float t);
	float B2(int i, float t);
	float B3(int i, float t);
	float B4(int i, float t);

	int* KVS;												// The knot value sequence.
	tVector3* CPs;											// The control points.

	// CurrIndex is stored for efficiency so that when GetPoint is called with a t value close to the previous time, we
	// can find the curve segment quickly.
	float CurrIndex;
	float MinParam;											// This is t sub 3 actually.
	float MaxParam;											// This is t sub (m+1) actually.
	int MaxCPIndex;											// The number of control points - 1.
};


}


// Implementation below this line.


inline tMath::tPiecewiseLinearPath::tPiecewiseLinearPath() :
	NumPoints(0),
	Points(0),
	PercentagePoints(0),
	TotalLength(0.0f)
{
}


inline tMath::tPiecewiseLinearPath::tPiecewiseLinearPath(const tVector3* points, int numPoints)	:
	NumPoints(0),
	Points(0),
	PercentagePoints(0),
	TotalLength(0.0f)
{
	SetPoints(points, numPoints);
}


inline tMath::tPiecewiseLinearPath::tPiecewiseLinearPath(const tPiecewiseLinearPath& src)	:
	NumPoints(0),
	Points(0),
	PercentagePoints(0),
	TotalLength(0.0f)
{
	SetPoints(src.Points, src.NumPoints);
}


inline void tMath::tPiecewiseLinearPath::Clear()
{
	delete[] Points;
	Points = nullptr;

	delete[] PercentagePoints;
	PercentagePoints = nullptr;

	NumPoints = 0;
	TotalLength = 0.0f;
}
