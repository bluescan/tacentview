// tUnits.cpp
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

#include "Foundation/tStandard.h"
#include "Foundation/tUnits.h"
namespace tUnit
{


const char* TimeUnitNames[int(tTime::NumTimeUnits)] =
{
	"PlankTime",
	"Chronon",
	"Attosecond",
	"Femtosecond",
	"Picosecond",
	"Nanosecond",
	"Microsecond",
	"Millisecond",
	"Tick",
	"Second",
	"She",
	"Helek",
	"Minute",
	"Hour",
	"Day",
	"Week",
	"Fortnight",
	"Year",
	"Annum",
	"Century",
	"Millennium",
	"GalacticYear"
};


const char* LengthUnitNames[int(tLength::NumLengthUnits)] =
{
	"Angstrom",
	"Meter",
	"Kilometer",
	"Inch",
	"Foot",
	"Yard",
	"Fathom",
	"Mile",
	"NauticalMile",
	"AstronomicalUnit"
};


const char* MassUnitNames[int(tMass::NumMassUnits)] =
{
	"Gram",
	"Kilogram",
	"Slug"
};


const char* tUnit::GetUnitName(tTime timeUnit)
{
	if (timeUnit == tTime::Unspecified)
		return nullptr;

	return TimeUnitNames[int(timeUnit)];
}


const char* tUnit::GetUnitName(tLength lengthUnit)
{
	if (lengthUnit == tLength::Unspecified)
		return nullptr;

	return LengthUnitNames[int(lengthUnit)];
}


const char* tUnit::GetUnitName(tMass massUnit)
{
	if (massUnit == tMass::Unspecified)
		return nullptr;

	return MassUnitNames[int(massUnit)];
}


tTime tUnit::GetTimeUnit(const char* unitName)
{
	for (int u = 0; u < int(tTime::NumTimeUnits); u++)
	{
		if (!tStd::tStricmp(unitName, TimeUnitNames[u]))
			return tTime(u);
	}

	return tTime::Unspecified;
}


tLength tUnit::GetLengthUnit(const char* unitName)
{
	for (int u = 0; u < int(tLength::NumLengthUnits); u++)
	{
		if (!tStd::tStricmp(unitName, LengthUnitNames[u]))
			return tLength(u);
	}

	return tLength::Unspecified;
}


tMass tUnit::GetMassUnit(const char* unitName)
{
	for (int u = 0; u < int(tMass::NumMassUnits); u++)
	{
		if (!tStd::tStricmp(unitName, MassUnitNames[u]))
			return tMass(u);
	}

	return tMass::Unspecified;
}


}
