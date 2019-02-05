// tUnits.h
//
// Definitions of various units for length, time, and mass along with the SI multiplier prefixes.
//
// Copyright (c) 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once


namespace tUnit
{
	enum class tPrefix
	{
		Invalid																											= -1,

		Yocto,							// 10^-24
		Zepto,							// 10^-21
		Atto,							// 10^-18
		Femto,							// 10^-15
		Pico,							// 10^-12
		Nano,							// 10^-9
		Micro,							// 10^-6
		Milli,							// 10^-3
		Centi,							// 10^-2
		Deci,							// 10^-1
		Unit,							// 10^0 Not an SI prefix, but a handy one.
		Deca,							// 10^1
		Hecto,							// 10^2
		Kilo,							// 10^3
		Mega,							// 10^6
		Giga,							// 10^9
		Tera,							// 10^12
		Peta,							// 10^15
		Exa,							// 10^18
		Zetta,							// 10^21
		Yotta,							// 10^24
		
		// Non SI prefixes.
		// Bi and Di are missing on purpose because they are ambiguous. In chemistry they both mean '2 of' like
		// sodium-BIcarbonate and carbon DIoxide, but sometimes they mean half. Never mind confusion about being paid
		// bi-monthly. Certainly semi-monthly is clear. Actually, we should look here for more:
		// https://en.wikipedia.org/wiki/Numeral_prefix like Duo, Quad, Sept, Hex, etc.
		Thrice,
		Twice,
		Semi,							// Half of.
		Half,
		Quarter,
		Eighth,
		Sixteenth,
		Thirtysecondth,
		Sixtyfourth,

		PrefixCount
	};
	
	enum class tTime
	{
		Unspecified																										= -1,
		
		PlankTime,						// 5.39E-44 seconds.
		Chronon,						// 6.97E-24 seconds. Smallest indivisible unit?!

		// These next 6 should perhaps not be here since we could use the tPrefix and Second units. Still, they are
		// common enough that perhaps they do deserve their own units.
		Attosecond,
		Femtosecond,
		Picosecond,
		Nanosecond,
		Microsecond,
		Millisecond,
		
		Tick,							// 1/60 of a second.
		Second,
		She,							// Babylonian. 0.3 seconds.
		Helek,							// Hebrew. Plural: halakim. 10/3 seconds.
		Minute,
		Hour,
		Day,							// Precisely 24 hours. Not an Earth rotation 23h 59m 56s.
		Week,							// Precisely 7 days.

		Fortnight,						// Precisely 2 weeks.
										// No months because they vary in duration.
		Year,							// Precisely 365 days.
		Annum,							// Precisely 365 days.
		Century,						// 100 years.
		Millennium,						// 1000 years.
		GalacticYear,					// One GY is 250 million years.
		
		NumTimeUnits
	};
	
	enum class tLength
	{
		Unspecified																										= -1,
		Angstrom,
		Meter,
		Kilometer,						// Could use the tPrefix, but it's a common measure.
		Inch,
		Foot,
		Yard,
		Fathom,
		Mile,
		NauticalMile,
		AstronomicalUnit,
		NumLengthUnits
	};
	
	enum class tMass
	{
		Unspecified																										= -1,
		Gram,
		Kilogram,						// Could use the tPrefix, but it's a common measure.
		Slug,
		NumMassUnits
	};
	
	// No compound units until we figure out how they'll work. For example, force (LBS, Newtons, etc) is composed of:
	// mass-unit * distance-unit / (time-unit * time-unit).

	// These return pointers to persistent (static) char strings. They return nullptr if Unspecified is passed in.
	const char* GetUnitName(tTime);
	const char* GetUnitName(tLength);
	const char* GetUnitName(tMass);

	// Given a unit name, these return the associated unit. Case insensitive compares, but no other fuzziness. They
	// return Unspecified if the name can't be found.
	tTime GetTimeUnit(const char* name);
	tLength GetLengthUnit(const char* name);
	tMass GetMassUnit(const char* name);
}
