// tTimer.h
//
// Simple timer class. Like a stopwatch. Supports keeping track of time in a number of different units. Accuracy is all
// up to you - you call the update function. This code does not access low-level timer hardware.
//
// Copyright (c) 2005, 2017, 2019 Tristan Grimmer.
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
#include <Foundation/tAssert.h>
#include <Foundation/tUnits.h>
namespace tSystem
{


#ifdef PLATFORM_WIN
// Gets the current local time in 100ns intervals since Jan 1, 1601.
uint64 tGetTimeLocal();

// Gets the Universal Time Coordinated, formerly known as Greenwich Mean Time.
// Units are in 100ns intervals since Jan 1, 1601.
uint64 tGetTimeUTC();
uint64 tGetTimeGMT();

// Gets the number of seconds since application start. Uses the high-performance counter,
float tGetTime();
double tGetTimeDouble();

// Return the time as a string.
enum class tTimeFormat
{
	Standard,		// Eg. 2020-01-14 01:47:12
	Extended,		// Eg. Tuesday January 14 2020 - 01:36:34
	Short,			// Eg. Tue Jan 14 2020  1:38:58
};
tString tConvertTimeToString(uint64, tTimeFormat = tTimeFormat::Standard);
#endif // PLATFORM_WIN


// High accuracy cross platform timing functions.
int64 tGetHardwareTimerFrequency();
int64 tGetHardwareTimerCount();
void tSleep(int milliSeconds);


class tTimer
{
public:
	// Creates a timer. You can specify whether the timer is constructed started or not. Internally a floating-point
	// (double) member is used to keep track of the elapsed time. You can choose the units it represents, which has
	// implications for overall timer precision. In general, use an internal unit that matches your domain - timing
	// oscillations of visible light? Use nanoseconds. If unit is Unspecified seconds are used.
	tTimer(bool start = true, tUnit::tTime unit = tUnit::tTime::Second)													: UnitInternal(unit), Running(start), Time(0.0f) { if (UnitInternal == tUnit::tTime::Unspecified) UnitInternal = tUnit::tTime::Second; }

	// Call this frequently. If Unit is not specified, internal units are used.
	void Update(float timeElapsed, tUnit::tTime = tUnit::tTime::Unspecified);

	// Does nothing (idempotent) if timer already started (or stopped).
	void Start()																										{ Running = true; }
	void Stop()																											{ Running = false; }
	void Reset(bool start = true)																						{ Time = 0.0; Running = start; }

	// Returns the time in the units of Unit. If Unit is not specified, uses internal units.
	float GetTime(tUnit::tTime = tUnit::tTime::Unspecified) const;
	bool IsRunning() const																								{ return Running; }
	tUnit::tTime GetInternalUnit() const																				{ return UnitInternal; }

	static double Convert(double time, tUnit::tTime from, tUnit::tTime to);

private:
	// @todo Add getters that format the time into a number of different string formats.
	const static double UnitConversionTable[tUnit::tTime::NumTimeUnits][tUnit::tTime::NumTimeUnits];

	tUnit::tTime UnitInternal;
	bool Running;
	double Time;

public:
	// For developers only. Easily add a new unit and recreate the unit conversion table.
	static void PrintHighPrecisionConversionTable(const char* outputFile);
};


}
