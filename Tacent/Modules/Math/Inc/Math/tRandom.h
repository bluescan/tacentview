// tRandom.h
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

#pragma once
#include <Foundation/tPlatform.h>
#include "Math/tGeometry.h"
namespace tMath {
namespace tRandom {


// Abstract base class for all random number generators. A generator is only responsible for generating random bits.
// Each instance of a generator should have its own state and produces its own random stream.
class tGenerator
{
public:
	tGenerator()																										{ }
	virtual ~tGenerator()																								{ }

	// Reset (reseed) the generator.
	virtual void SetSeed(uint32 seed)																					= 0;
	virtual void SetSeed(uint64 seed)																					= 0;
	virtual void SetSeed(const uint32* seeds, int numSeeds)																= 0;

	// This is it. The main interface function. Returns 32 random bits every time it's called.
	virtual uint32 GetBits() const																						= 0;
};


// A random generator using the Mersenne Twistor algorithm by M. Matsumoto & T. Nishimura, Transactions of the
// Association for Computing Machinery, Vol 8, 1998. This generator has a large memory footprint: 19937 bits of state.
class tGeneratorMersenneTwister : public tGenerator
{
public:
	tGeneratorMersenneTwister()																							{ SetSeed(uint32(0x4242CDCD)); }
	tGeneratorMersenneTwister(uint32 seed)																				{ SetSeed(seed); }
	tGeneratorMersenneTwister(uint64 seed)																				{ SetSeed(seed); }
	tGeneratorMersenneTwister(const uint32* seeds, int numSeeds)														{ SetSeed(seeds, numSeeds); }

	// These may be used to reset (reseed) the generator.
	void SetSeed(uint32 seed) override;
	void SetSeed(uint64 seed) override																					{ SetSeed( ((const uint32*)(void*)&seed), 2 ); }
	void SetSeed(const uint32* seeds, int numSeeds) override;
	uint32 GetBits() const override;

private:
	static const int Dimensions																							= 624;
	static const int PartialDims																						= 397;
	static const int LeftShift																							= 31;
	static const uint32 Toggle																							= 0x9908B0DF;
	static const int TemperShiftA																						= 11;
	static const int TemperShiftB																						= 7;
	static const int TemperShiftC																						= 15;
	static const int TemperShiftD																						= 18;
	static const uint32 TemperB																							= 0x9D2C5680;
	static const uint32 TemperC																							= 0xEFC60000;

	// The complete state information.
	mutable uint32 StateVector[Dimensions];
	mutable int StateIndex;
};


// If we need a cryptographically secure random number generator we should finish implementing Fortuna.
class tGeneratorFortuna : public tGenerator
{
public:
	tGeneratorFortuna()																									{ SetSeed(uint32(0x4242CDCD)); }
	tGeneratorFortuna(uint32 seed)																						{ SetSeed(seed); }
	tGeneratorFortuna(uint64 seed)																						{ SetSeed(seed); }
	tGeneratorFortuna(const uint32* seeds, int numSeeds)																{ SetSeed(seeds, numSeeds); }

	void SetSeed(uint32 seed) override;
	void SetSeed(uint64 seed) override																					{ SetSeed( ((const uint32*)(void*)&seed), 2 ); }
	void SetSeed(const uint32* seeds, int numSeeds) override;
	uint32 GetBits() const override;

private:
	// The complete state information.
	mutable uint32 StateVector[1];
	mutable int StateIndex;
};


// We're going to use Mersenne-Twister as our default generator type.
using tDefaultGeneratorType = tGeneratorMersenneTwister;

// And here is the single global default random number generator. Additional generators can be created if desired.
extern tDefaultGeneratorType DefaultGenerator;

// Useful interface functions for generating random quantities in a variety of types from bool to double. Before
// calling these make sure the generator being used is initialized properly. For the global default generator, waiting
// till main() is safe, but during static initialization could cause problems. The random numbers from GetFloat and
// GetDouble are E [0.0, 1.0], and GetSign is either -1.0 or 1.0.
float tGetSign(const tGenerator& = DefaultGenerator);
bool tGetBool(const tGenerator& = DefaultGenerator);
uint32 tGetBits(const tGenerator& = DefaultGenerator);
float tGetFloat(const tGenerator& = DefaultGenerator);
double tGetDouble(const tGenerator& = DefaultGenerator);

// These compute a random value in [min, max]. The templated version can be used for vectors.
int tGetBounded(int min, int max, const tGenerator& = DefaultGenerator);
float tGetBounded(float min, float max, const tGenerator& = DefaultGenerator);
double tGetBounded(double min, double max, const tGenerator& = DefaultGenerator);
template <typename T> T tGetBounded(T min, T max, const tGenerator& = DefaultGenerator);

// This returns a random number centered around 'center' plus or minus 'extent'. eg. calling GetExtentBounded(10, 2)
// will result in a random number in the interval [8, 12].
template <typename T> T tGetExtentBounded(T center, T extent, const tGenerator& = DefaultGenerator);

// Gets a random direction centered about the input direction within extent angle. Essentially creating a cone.
tVector3 tGetDir(const tVector3& centerDir, float extentAngle, const tGenerator& = DefaultGenerator);

// Random point generation functions in R3 for various geometric primitives.
tVector3 tGetPointOnLineSeg(const tLineSeg&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointOnTriangle(const tTriangle&, const tGenerator& = DefaultGenerator);

// Plane must be normalized. Square is centered at (a,b,c).
tVector3 tGetPointOnUnitSquare(const tPlane&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointOnBox(const tBox&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointOnSphere(const tSphere&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointOnCylinder(const tCylinder&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointInBox(const tBox&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointInSphere(const tSphere&, const tGenerator& = DefaultGenerator);
tVector3 tGetPointInCylinder(const tCylinder&, const tGenerator& = DefaultGenerator);


}
}


// Implementation below this line.


inline float tMath::tRandom::tGetSign(const tGenerator& gen)
{
	return (gen.GetBits() & 0x00000001) ? 1.0f : -1.0f;
}


inline bool tMath::tRandom::tGetBool(const tGenerator& gen)
{
	return (gen.GetBits() & 0x00000001) ? true : false;
}


inline uint32 tMath::tRandom::tGetBits(const tGenerator& gen)
{
	return gen.GetBits();
}


inline float tMath::tRandom::tGetFloat(const tGenerator& gen)
{
	return float(tGetDouble(gen));
}


inline int tMath::tRandom::tGetBounded(int min, int max, const tGenerator& gen)
{
	tAssert(max >= min);
	return min + (tGetBits(gen) % (1 + max - min));
}


inline float tMath::tRandom::tGetBounded(float min, float max, const tGenerator& gen)
{
	tAssert(max >= min);
	return min + (tGetFloat(gen)*(max - min));
}


inline double tMath::tRandom::tGetBounded(double min, double max, const tGenerator& gen)
{
	tAssert(max >= min);
	return min + (tGetDouble(gen)*(max - min));
}


template <typename T> T tMath::tRandom::tGetBounded(T min, T max, const tGenerator& gen)
{
	T result;
	for (int c = 0; c < T::GetNumComponents(); c++)
		result[c] = tGetBounded(min[c], max[c], gen);

	return result;
}


template <typename T> T tMath::tRandom::tGetExtentBounded(T center, T extent, const tGenerator& gen)
{
	return tGetBounded(center - extent, center + extent, gen);
}


inline tMath::tVector3 tMath::tRandom::tGetPointOnLineSeg(const tLineSeg& seg, const tGenerator& gen)
{
	float t = tGetFloat(gen);
	return seg.A + ((seg.B - seg.A) * t );
}
