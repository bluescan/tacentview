// tRandom.cpp
//
// Functions necessary to generate pseudo-random numbers using a number of different algorithms. Random bits, integers,
// and floats may all be generated. The generators are objects and state isn't shared. This allows multiple generators
// that can optionally be run on different threads. Sharing a generator between threads is not supported. One of the
// generators implements the Mersenne Twistor algorithm by M. Matsumoto & T. Nishimura.
//
// Copyright (c) 2005, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "Math/tRandom.h"
using namespace tMath;


tRandom::tDefaultGeneratorType tRandom::DefaultGenerator;


void tRandom::tGeneratorMersenneTwister::SetSeed(uint32 seed)
{
	StateVector[0] = seed;
	for (StateIndex = 1; StateIndex < Dimensions; StateIndex++)
		StateVector[StateIndex] = 0x6C078965 * (StateVector[StateIndex-1] ^ (StateVector[StateIndex-1] >> 30)) + StateIndex;
}


void tRandom::tGeneratorMersenneTwister::SetSeed(const uint32* seeds, int numSeeds)
{
	tAssert(numSeeds > 0);
	SetSeed(uint32(0x54FFC97A));

	int i = 1; int j = 0;
	int countMax = tMax(int(Dimensions), numSeeds);
	for (int count = 0; count < countMax; count++)
	{
		StateVector[i] = (StateVector[i] ^ ((StateVector[i-1] ^ (StateVector[i-1] >> 30)) * 0x0019660D)) + seeds[j] + j;
		i++;
		j++;
		if (i >= Dimensions)
		{
			StateVector[0] = StateVector[Dimensions-1];
			i = 1;
		}

		if (j >= numSeeds)
			j = 0;
	}

	for (int count = 0; count < Dimensions-1; count++)
	{
		StateVector[i] = (StateVector[i] ^ ((StateVector[i-1] ^ (StateVector[i-1] >> 30)) * 0x5D588B65)) - i;
		if (++i >= Dimensions)
		{
			StateVector[0] = StateVector[Dimensions-1];
			i = 1;
		}
	}

	StateVector[0] = 0x80000000;
}


uint32 tRandom::tGeneratorMersenneTwister::GetBits() const
{
	if (StateIndex >= Dimensions)
	{
		uint32 lowerBits = (1LU << LeftShift) - 1;
		uint32 upperBits = -1L << LeftShift;

		const uint32 toggle[2] = { 0, Toggle };
		for (int i = 0; i < Dimensions-PartialDims; i++)
		{
			uint32 y = (StateVector[i] & upperBits) | (StateVector[i+1] & lowerBits);
			StateVector[i] = StateVector[i+PartialDims] ^ (y>>1) ^ toggle[ y&1 ];
		}

		for (int i = Dimensions-PartialDims; i < Dimensions-1; i++)
		{
			uint32 y = (StateVector[i] & upperBits) | (StateVector[i+1] & lowerBits);
			StateVector[i] = StateVector[i+PartialDims-Dimensions] ^ (y>>1) ^ toggle[ y&1 ];
		}

		uint32 y = (StateVector[Dimensions-1] & upperBits) | (StateVector[0] & lowerBits);
		StateVector[Dimensions-1] = StateVector[PartialDims-1] ^ (y>>1) ^ toggle[ y&1 ];
		StateIndex = 0;
	}

	uint32 bits = StateVector[StateIndex++];

	// Tempering.
	bits ^=	 bits >> TemperShiftA;
	bits ^= (bits << TemperShiftB) & TemperB;
	bits ^= (bits << TemperShiftC) & TemperC;
	bits ^=	 bits >> TemperShiftD;

	return bits;
}


double tRandom::tGetDouble(const tGenerator& gen)
{
	union
	{
		double real;
		uint32 pattern[2];
	} convert;

	uint32 bits = gen.GetBits();

	// This assumes knowledge of the IEEE double precision floating point representation.
	#ifdef ENDIAN_LITTLE
	convert.pattern[0] =  bits << 20;
	convert.pattern[1] = (bits >> 12) | 0x3FF00000;
	#else
	convert.pattern[1] =  bits << 20;
	convert.pattern[0] = (bits >> 12) | 0x3FF00000;
	#endif

	return convert.real - 1.0;
}


tVector3 tRandom::tGetDir(const tVector3& centerDir, float extentAngle, const tGenerator& gen)
{
	// @todo This doesn't work for arbitrary central directions. For example, centerDir.z should be allowed to be zero.
	float theta = tArcTan( tSqrt(centerDir.x*centerDir.x + centerDir.y*centerDir.y), centerDir.z );
	float phi = tArcTan( centerDir.y, centerDir.x );
	float normSpread = (Pi - (tGetFloat(gen)*extentAngle)) / Pi;

	// Rotate around the axis.
	theta += -((extentAngle / 2.0f)) + (tGetFloat(gen) * extentAngle);

	// Rotate away from the axis phi radians at angle theta.
	phi += (-extentAngle/2.0f) + (tGetFloat(gen) * normSpread * extentAngle);

	float sinTheta = tSin(theta);
	float cosTheta = tCos(theta);
	float sinPhi = tSin(phi);
	float cosPhi = tCos(phi);
	return tVector3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}


// @todo Make these work.


tVector3 tRandom::tGetPointOnTriangle(const tTriangle& tri, const tGenerator& gen)
{
	// P = a*A + b*B + c*C with a + b + c = 1. We generate a and b randomly E [0, 1/2]. Choosing 1/2 for the upper
	// bound ensures we don't go over 1. http://www.cgafaq.info/wiki/Random_Point_In_Triangle
	float a = tGetFloat(gen) / 2.0f;
	float b = tGetFloat(gen) / 2.0f;
	float c = 1.0f - a - b;
	return tri.A*a + tri.B*b + tri.C*c;
}


tVector3 tRandom::tGetPointOnUnitSquare(const tPlane& plane, const tGenerator& gen)
{
	// Requires normalized plane. http://mathworld.wolfram.com/Plane.html
	tVector3 u, v;
	plane.ComputeOrthogonalBasisVectors(u, v);

	float randomU = tGetFloat(gen) - 0.5f;
	float randomV = tGetFloat(gen) - 0.5f;
	tVector3 point =  plane.Normal*plane.Distance + u*randomU + v*randomV;
	return point;
}


tVector3 tRandom::tGetPointOnBox(const tBox& box, const tGenerator& gen)
{
	tVector3 extent = box.ComputeExtents();
	tPlane plane;
	tVector3 u, v;

	int face = tGetBounded(1, 6, gen);
	switch (face)
	{
		case 1:
		case 2:
			plane.Distance = extent.x;
			plane.Normal = tVector3::i;
			u = extent.y * tVector3::j;
			v = extent.z * tVector3::k;
			break;

		case 3:
		case 4:
			plane.Distance = extent.y;
			plane.Normal = tVector3::j;
			u = extent.x * tVector3::i;
			v = extent.z * tVector3::k;
			break;

		case 5:
		case 6:
			plane.Distance = extent.z;
			plane.Normal = tVector3::k;
			u = extent.x * tVector3::i;
			v = extent.y * tVector3::j;
			break;
	}

	if (face % 2)
		plane.Normal = -plane.Normal;
	
	float randomU = tGetBounded(-1.0f, 1.0f, gen);
	float randomV = tGetBounded(-1.0f, 1.0f, gen);

	return plane.Normal*plane.Distance + u*randomU + v*randomV;
}


tVector3 tRandom::tGetPointOnSphere(const tSphere& sphere, const tGenerator& gen)
{
	// See http://mathworld.wolfram.com/SphericalCoordinates.html
	float theta = tGetBounded(0.0f, TwoPi, gen);
	float phi = tGetBounded(0.0f, Pi, gen);
	
	// 's' for Sin, 'c' for Cos.
	float ct, st, cp, sp;
	tCosSin(ct, st, theta);
	tCosSin(cp, sp, phi);

	tVector3 point(ct*sp, st*sp, cp);
	point *= sphere.Radius;
	point += sphere.Center;
	return point;
}


tVector3 tRandom::tGetPointOnCylinder(const tCylinder& cylinder, const tGenerator& gen)
{
	// See http://mathworld.wolfram.com/CylindricalCoordinates.html
	tVector3 axis = cylinder.B - cylinder.A;
	axis.Normalize();

	// This function works by first computing a plane perpendicular to the major cylinder axis
	// and placing a circle there. We simply randomly choose how far up the cylinder the plane
	// is, and then choose a random angle around the circle.
	tPlane plane(axis);
	tVector3 u, v;
	plane.ComputeOrthogonalBasisVectors(u, v);

	axis *= tGetFloat(gen);									// Random distance along axis.
	float theta = tGetBounded(0.0f, TwoPi, gen);			// Random angle around circle.
	float st = tSin(theta);
	float ct = tCos(theta);

	tVector3 point = cylinder.A + axis;						// Point in middle of cylinder.
	point += cylinder.Radius * (u*st + v*ct);				// Point on circle.
	return point;
}


tVector3 tRandom::tGetPointInBox(const tBox& box, const tGenerator& gen)
{
	float xr = tGetFloat(gen);
	float yr = tGetFloat(gen);
	float zr = tGetFloat(gen);
	tVector3 offset
	(
		xr * (box.Max.x - box.Min.x),
		yr * (box.Max.y - box.Min.y),
		zr * (box.Max.z - box.Min.z)
	);
	return box.Min + offset;
}


tVector3 tRandom::tGetPointInSphere(const tSphere& sphere, const tGenerator& gen)
{
	// See http://mathworld.wolfram.com/SphericalCoordinates.html
	float theta = tGetBounded(0.0f, TwoPi, gen);
	float phi = tGetBounded(0.0f, Pi, gen);
	float radius = tGetBounded(0.0f, sphere.Radius, gen);
	
	// 's' for Sin, 'c' for Cos.
	float ct, st, cp, sp;
	tCosSin(ct, st, theta);
	tCosSin(cp, sp, phi);

	tVector3 point(ct*sp, st*sp, cp);
	point *= radius;
	point += sphere.Center;
	return point;
}


tVector3 tRandom::tGetPointInCylinder(const tCylinder& cylinder, const tGenerator& gen)
{
	// See http://mathworld.wolfram.com/CylindricalCoordinates.html
	tVector3 axis = cylinder.B - cylinder.A;
	axis.Normalize();

	// This function works by first computing a plane perpendicular to the major cylinder axis and placing a circle
	// there. We simply randomly choose how far up the cylinder the plane is, and then choose a random angle around
	// the circle.
	tPlane plane(axis);
	tVector3 u, v;
	plane.ComputeOrthogonalBasisVectors(u, v);

	axis *= tGetFloat(gen);									// Random distance along axis.
	float radius = tGetBounded(0.0f, cylinder.Radius, gen);	// Random radius out.
	float theta = tGetBounded(0.0f, TwoPi, gen);			// Random angle around circle.
	float st = tSin(theta);
	float ct = tCos(theta);

	tVector3 point = cylinder.A + axis;						// Point in middle of cylinder.
	point += radius * (u*st + v*ct);						// Point on circle.
	return point;
}
