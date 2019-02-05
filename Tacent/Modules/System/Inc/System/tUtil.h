// tUtil.h
//
// Utility functions. These include hardware querying utilities like supported instruction sets, number or cores,
// hardware timer functions, and computer name/ip accessors.
//
// Copyright (c) 2004-2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tString.h>
#include "System/tThrow.h"
#include "System/tPrint.h"
namespace tSystem
{


#ifdef PLATFORM_WIN
// Returns true if the processor and the OS support Streaming SIMD Extensions.
bool tSupportsSSE();
bool tSupportsSSE2();
#endif

// Returns the computer's name.
tString tGetCompName();

// Returns the ip address of the machine if it is set. String will look like: "xxx.xxx.xxx.xxx". Returns an empty
// string if it is not set.
tString tGetIPAddress();

#ifdef PLATFORM_WIN
// Returns the number of cores (processors) the current machine has.
int tGetNumCores();

// Gets the current local time in 100ns intervals since Jan 1, 1601. @todo Move to tTime.h
uint64 tGetTimeLocal();

// Gets the Universal Time Coordinated, formerly known as Greenwich Mean Time. Units are in 100ns intervals since
// Jan 1, 1601.
uint64 tGetTimeUTC();
uint64 tGetTimeGMT();

// Return the time as a string.
enum class tTimeFormat
{
	Extended,
	Standard
};
tString tConvertTimeToString(uint64, tTimeFormat = tTimeFormat::Extended);
#endif // PLATFORM_WIN
// High accuracy cross platform timing functions.
int64 tGetHardwareTimerFrequency();
int64 tGetHardwareTimerCount();
void tSleep(int milliSeconds);


}


// The following utility error objects may be thrown by functions in the tUtil namespace.
struct tUtilError : public tError
{
	tUtilError(const char* format, ...)																					: tError("tUtil Module. ") { va_list marker; va_start(marker, format); Message += tvsPrintf(format, marker); }
	tUtilError(const tString& message)																					: tError("tUtil Module. ") { Message += message; }
	tUtilError()																										: tError("tUtil Module.") { }
};
