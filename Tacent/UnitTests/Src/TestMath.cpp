// TestMath.cpp
//
// Math module tests.
//
// Copyright (c) 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tFundamentals.h>
#include <Math/tSpline.h>
#include <Math/tRandom.h>
#include <Math/tQuaternion.h>
#include "UnitTests.h"
using namespace tMath;
namespace tUnitTest
{


tTestUnit(Fundamentals)
{
	int val = 256;
	bool isPow2 = tIsPower2(val);
	tPrintf("Val:%d   Pow2:%s\n", val, isPow2 ? "true" : "false");
	tRequire(isPow2);

	val = 257;
	isPow2 = tIsPower2(val);
	tPrintf("Val:%d   Pow2:%s\n", val, isPow2 ? "true" : "false");
	tRequire(!isPow2);

	val = 0;
	isPow2 = tIsPower2(val);
	tPrintf("Val:%d   Pow2:%s\n", val, isPow2 ? "true" : "false");
	tRequire(!isPow2);

	val = 1;
	isPow2 = tIsPower2(val);
	tPrintf("Val:%d   Pow2:%s\n", val, isPow2 ? "true" : "false");
	tRequire(isPow2);

	val = 16;
	int nextPow2 = tNextLowerPower2(val);
	tPrintf("Val:%d   NextLowerPower2:%d\n", val, nextPow2);
	tRequire(nextPow2 == 8);

	val = 16;
	nextPow2 = tNextHigherPower2(val);
	tPrintf("Val:%d   NextHigherPower2:%d\n", val, nextPow2);
	tRequire(nextPow2 == 32);

	val = 127;
	nextPow2 = tNextLowerPower2(val);
	tPrintf("Val:%d   NextLowerPower2:%d\n", val, nextPow2);
	tRequire(nextPow2 == 64);

	val = 127;
	nextPow2 = tNextHigherPower2(val);
	tPrintf("Val:%d   NextHigherPower2:%d\n", val, nextPow2);
	tRequire(nextPow2 == 128);

	val = 0;
	nextPow2 = tNextLowerPower2(val);
	tPrintf("Val:%d   NextLowerPower2:%d\n", val, nextPow2);

	val = 0;
	nextPow2 = tNextHigherPower2(val);
	tPrintf("Val:%d   NextHigherPower2:%d\n", val, nextPow2);

	val = 1;
	nextPow2 = tNextLowerPower2(val);
	tPrintf("Val:%d   NextLowerPower2:%d\n", val, nextPow2);

	val = 1;
	nextPow2 = tNextHigherPower2(val);
	tPrintf("Val:%d   NextHigherPower2:%d\n", val, nextPow2);

	tPrintf("Log2 Tests.\n");
	for (int v = -3; v < 257; v++)
		tPrintf("Log2(%d) = %d\n", v, tLog2(v));

	for (uint v = 0x7FFFFFF0; v != 0x80000000; v++)
		tPrintf("Log2(%d) = %d\n", v, tLog2(v));

	tPrintf("tCeiling(-2.5f) : %f.\n", tCeiling(-2.5f));
	tRequire(tCeiling(-2.5f) == -2.0f);

	// Test rounding.
	tPrintf("tRound(0.0f) : %f.\n", tRound(0.0f));
	tRequire(tRound(0.0f) == 0.0f);

	tPrintf("tRound(2.0f) : %f.\n", tRound(2.0f));
	tRequire(tRound(2.0f) == 2.0f);

	tPrintf("tRound(2.1f) : %f.\n", tRound(2.1f));
	tRequire(tRound(2.1f) == 2.0f);

	tPrintf("tRound(2.5f) : %f.\n", tRound(2.5f));
	tRequire(tRound(2.5f) == 3.0f);

	tPrintf("tRound(2.9f) : %f.\n", tRound(2.9f));
	tRequire(tRound(2.9f) == 3.0f);

	tPrintf("tRound(-1.5f) : %f.\n", tRound(-1.5f));
	tRequire(tRound(-1.5f) == -1.0f);
}


tTestUnit(Spline)
{
	tVector3 CVs[4];
	CVs[0] = tVector3(0.0, 0.0, 0.0);
	CVs[1] = tVector3(1.0, 1.0, 0.0);
	CVs[2] = tVector3(2.0, 1.0, 0.0);
	CVs[3] = tVector3(3.0, 0.0, 0.0);

	// Testing path.
	tBezierPath path;
	path.SetControlVerts(CVs, 4, tBezierPath::tMode::ExternalCVs, tBezierPath::tType::Open);
	tRequire(!path.IsClosed());

	for (int n = 0; n <= 10; n++)
	{
		float t = float(n)/10.0f;
		tVector3 p;
		path.GetPoint(p, t);
		tPrintf("Path: Param=%f  Point=%v\n", t, p.Pod());
	}

	// Testing curve.
	tBezierCurve curve(CVs);
	for (int n = 0; n <= 10; n++)
	{
		float t = float(n)/10.0f;
		tVector3 p;
		curve.GetPoint(p, t);
		tPrintf("Curve: Param=%f  Point=%v\n", t, p.Pod());
	}
	float closestParam = curve.GetClosestParam(tVector3(4.0, 0.0, 0.0));
	tRequire(tMath::tApproxEqual(closestParam, 1.0f));	
	tPrintf("Closest Param=%f\n", closestParam);
}

tTestUnit(Hash)
{
	const char* testString = "This is the text that is being used for testing hash functions.";
	tPrintf("%s\n\n", testString);

	tPrintf("Fast 32  bit hash: %08x\n", tHashStringFast32(testString));
	tPrintf("Good 32  bit hash: %08x\n", tHashString32(testString));
	tPrintf("Good 64  bit hash: %016|64x\n", tHashString64(testString));

	const char* md5String = "The quick brown fox jumps over the lazy dog";
	tuint128 md5HashComputed = tHashStringMD5(md5String);
	tuint128 md5HashCorrect("0x9e107d9d372bb6826bd81d3542a419d6");
	tPrintf("MD5 String   : %s\n", md5String);
	tPrintf("MD5 Correct  : 9e107d9d372bb6826bd81d3542a419d6\n");
	tPrintf("MD5 Computed : %032|128x\n\n", md5HashComputed);
	tRequire(md5HashComputed == md5HashCorrect);

	tuint256 hash256 = tHashString256(testString);
	tPrintf("Good 256 bit hash: %064|256X\n\n", hash256);

	tPrintf("Fast 32  bit hash: %08x\n", tHashStringFast32(testString));
	tPrintf("Good 32  bit hash: %08x\n", tHashString32(testString));
	tPrintf("Good 64  bit hash: %016|64x\n\n", tHashString64(testString));

	uint32 hash32single = tHashStringFast32("This is a string that will be separated into two hash computations.");
	tPrintf("Fast 32 bit single hash  : %08x\n", hash32single);
	uint32 part32 = tHashStringFast32("This is a string that will be sepa");
	part32 = tHashStringFast32("rated into two hash computations.", part32);
	tPrintf("Fast 32 bit two part hash: %08x\n\n", part32);
	tRequire(hash32single == part32);

	// From the header: The HashData32/64/128/256 and variants do _not_ guarantee the same hash value if they are chained together.
	tPrintf("Single 64 bit hash  : %016|64x\n", tHashString64("This is a string that will be separated into two hash computations."));
	uint64 part64 = tHashString64("This is a string that will be sepa");
	part64 = tHashString64("rated into two hash computations.", part64);
	tPrintf("Two part 64 bit hash: %016|64x\n\n", part64);

	// This makes sure nobody changes how the hash functions work, which would be bad. It does this by hardcoding the
	// result into the test. @todo We should do this for all hash function variants.
	tuint256 hashString256 = tHashString256(testString);
	const char* realHashString256 = "6431af73 c538aa59 318121fd 25696a9f e3c05e59 8cb3c9c2 74bfbde6 3b1be458";
	tuint256 hashStringCorrect256(realHashString256, 16);
	tPrintf
	(
		"Good 256 bit hash: %064|256x\n"
		"Real 256 bit hash: %s\n\n",
		hashString256, realHashString256
	);
	tRequire(hashString256 == hashStringCorrect256);
}


tTestUnit(Random)
{
	uint32 seeds[1024];
	for (int i = 0; i < 1024; i++)
		seeds[i] = i;

	tRandom::tDefaultGeneratorType gen(seeds, 1025);

	tPrintf("Random Bits\n");
	for (int i = 0; i < 16; i++)
		tPrintf("Bit %02d     : %08X\n", i, gen.GetBits());

	tPrintf("Random Bits Again\n");
	for (int i = 0; i < 16; i++)
		tPrintf("Bit %02d     : %08X\n", i, tRandom::tGetBits());

	tPrintf("Random Integers in [-10, 10]\n");
	for (int i = 0; i < 16; i++)
	{
		int randomInt = tRandom::tGetBounded(-10, 10, gen);
		tPrintf("Integer %02d : %d\n", i, randomInt);
		tRequire(tInRange(randomInt, -10, 10));
	}

	tPrintf("Random Floats in [0.0f, 1.0f]\n");
	for (int i = 0; i < 16; i++)
	{
		float randomFloat = tRandom::tGetFloat(gen);
		tPrintf("Float %02d   : %f\n", i, randomFloat);
		tRequire(tInRange(randomFloat, 0.0f, 1.0f));
	}

	tPrintf("Random Doubles in [0.0, 1.0]\n");
	for (int i = 0; i < 16; i++)
	{
		double randomDouble = tRandom::tGetDouble(gen);
		tPrintf("Double %02d  : %f\n", i, randomDouble);
		tRequire(tInRange(randomDouble, 0.0, 1.0));
	}

	tPrintf("Random Bounded Vector2s in [(-10.0, -10.0), (10.0, 10.0)]\n");
	for (int i = 0; i < 16; i++)
	{
		tVector2 r = tRandom::tGetBounded(tVector2(-10.0f), tVector2(10.0f));
		tPrintf("Vector2 %02d : %:2v\n", i, pod(r));
		tRequire(tInRange(r.x, -10.0f, 10.0f));
		tRequire(tInRange(r.y, -10.0f, 10.0f));
	}

	tPrintf("Random Extent Bounded Vector2s in [(40.0, 40.0), (60.0, 60.0)]\n");
	for (int i = 0; i < 16; i++)
	{
		tVector2 r = tRandom::tGetExtentBounded(tVector2(50.0f), tVector2(10.0f));
		tPrintf("Vector2 %02d : %:2v\n", i, pod(r));
		tRequire(tInRange(r.x, 40.0f, 60.0f));
		tRequire(tInRange(r.y, 40.0f, 60.0f));
	}
}


tTestUnit(Matrix)
{
	tMatrix4 a;
	tMatrix4 b;
	a.Identity();
	b.Identity();
	a.a11 = 0.0f; a.a12 = 1.0f;
	a.a21 = -1.0f;
	b.a11 = 4.0f; b.a12 = 3.0f;
	b.a21 = 5.0f;

	a += b;
	a -= b;
	a *= b;
	a = a + b;
	a = a - b;
	a = a * b;

	a /= 0.5f;
	a = a / 3.0f;

	a *= 3.0f;
	a = a * 0.2f;

	if (a == b)
		tPrintf("Mats equal\n");
	tRequire(!(a == b));

	if (a != b)
		tPrintf("Mats not equal\n");
	tRequire(a != b);

	a = +a;
	a = -a;

	tVector3 v3; v3.Zero();
	tVector4 v4; v4.Zero();
	v3 = a*v3;
	v4 = a*v4;
	tPrintf("Vector3: %:3v\n", v3.Pod());

	tMatrix4 prod = a * b;
	tPrintf("Prod matrix4: %m\n", prod);

	tMatrix4 m1;
	tMatrix4 m2;

	m1.MakeRotate( tVector3(3.0f, 4.0f, 5.0f), 2.6436f );
	tPrintf("m1 matrix4: %m\n", m1);

	m2 = m1;
	m2.Invert();
	tPrintf("m2 (inverse of m1): %m\n", m2);

	m2 = m2 * m1;
	tPrintf("Product of inverses: %m\n", m2);
	tRequire(m2.ApproxEqual(tMatrix4::identity));

	tPrintf("Test matrix multiply.\n");
	tMatrix4 m;
	m.MakeRotateY(2.0f);
	tSet(m.C4, 2.0f, 3.0f, 4.0f, 1.0f);

	tVector4 v(20.0f, 30.0f, 40.0f, 0.0f);
	tVector4 r = m*v;
	tPrintf("Mult result: %4v\n", r);

	tVector4 e = m.Col1()*v.x + tVector4(m.C2)*v.y + tVector4(m.C3)*v.z + tVector4(m.C4)*v.w;
	tPrintf("Explicit result: %4v\n", e);
	tRequire(r == e);
}


tTestUnit(Quaternion)
{
	tMatrix4 identMat;
	identMat.Identity();

	tQuaternion cq(identMat);
	tPrintf("Quat from ident mat %q\n", cq);

	tVector4 v = tVector4::zero;
	tVector4 v2(v);

	identMat.Set(cq);
	tPrintf("Mat from ident quat: %m\n", identMat);

	tQuaternion qi(0.5f, 0.5f, 0.5f, 0.5f);
	tRequire(tApproxEqual(qi.Length(), 1.0f));
	qi.Normalize();
	tPrintf("Quat before %q\n", qi);

	tMatrix4 m(qi);
	tQuaternion qf(m);
	tPrintf("Quat after %q\n\n", qf);
	tRequire(qi.ApproxEqual(qf));

	tMatrix4 matBefore = tMatrix4::identity;
	matBefore.E[5] = -1.0f;
	matBefore.E[10] = -1.0f;
	tPrintf("Mat Before conversion:\n%_m\n\n", matBefore);

	tQuaternion qm(matBefore);
	tMatrix4 matAfter(qm);
	tPrintf("Mat After conversion:\n%_m\n\n", matAfter);
	tRequire(matBefore.ApproxEqual(matAfter));
}


tTestUnit(Geometry)
{
	tRay ray;
	ray.Start.Set(1.0f, 0.5f, 0.5f);
	ray.Dir.Set(-1.0f, 0.0f, 0.0f);

	// CCW winding.
	tTriangle tri;
	tri.A.Set(0.0f, 0.0f, 0.0f);
	tri.B.Set(0.0f, 1.0f, 0.0f);
	tri.C.Set(0.0f, 0.0f, 1.0f);

	bool intersects = tIntersectTestRayTriangle(ray, tri);
	tPrintf("Ray intersects triangle: %s\n", intersects ? "true" : "false");
	tRequire(intersects);

	ray.Start.Set(10.0f, 10.5f, 10.5f);
	ray.Dir.Set(1.0f, 2.0f, 3.0f);
	intersects = tIntersectTestRayTriangle(ray, tri);
	tPrintf("Ray intersects triangle: %s\n", intersects ? "true" : "false");
	tRequire(!intersects);
}


}
