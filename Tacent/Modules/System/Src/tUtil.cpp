// tUtil.cpp
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

#pragma warning (disable: 4091)
#include <sys/types.h>
#include <sys/stat.h>
#ifdef PLATFORM_WIN
#include <Windows.h>
#include <intrin.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Foundation/tStandard.h"
#include "System/tFile.h"
#include "System/tUtil.h"
#include "System/tPrint.h"


#ifdef PLATFORM_WIN
bool tSystem::tSupportsSSE()
{
	int cpuInfo[4];
	int infoType = 1;
	__cpuid(cpuInfo, infoType);

	int features = cpuInfo[3];

	// SSE feature bit is 25.
	if (features & (1 << 25))
		return true;
	else
		return false;
}


bool tSystem::tSupportsSSE2()
{
	int cpuInfo[4];
	int infoType = 1;
	__cpuid(cpuInfo, infoType);

	int features = cpuInfo[3];

	// SSE2 feature bit is 26.
	if (features & (1 << 26))
		return true;
	else
		return false;
}
#endif


tString tSystem::tGetCompName()
{
	#ifdef PLATFORM_WIN
	char name[128];
	ulong nameSize = 128;

	WinBool success = GetComputerName(name, &nameSize);
	if (success)
		return name;
	#endif

	return tString();
}


tString tSystem::tGetIPAddress()
{
	// @todo Implement.  Maybe use gethostname.
	return tString();
}


#ifdef PLATFORM_WIN
int tSystem::tGetNumCores()
{
	SYSTEM_INFO sysinfo;
	tStd::tMemset(&sysinfo, 0, sizeof(sysinfo));
	GetSystemInfo(&sysinfo);
	if ((sysinfo.dwNumberOfProcessors == 0) || (sysinfo.dwNumberOfProcessors == -1))
		return 1;
	
	return sysinfo.dwNumberOfProcessors;
}


uint64 tSystem::tGetTimeLocal()
{
	SystemTime sysTime;
	GetLocalTime(&sysTime);

	FileTime fileTime;
	tStd::tMemset(&fileTime, 0, sizeof(fileTime));
	SystemTimeToFileTime(&sysTime, &fileTime);

	uint64 retVal;
	retVal = fileTime.dwHighDateTime;
	retVal <<= 32;
	retVal |= fileTime.dwLowDateTime;
	return retVal;
}


uint64 tSystem::tGetTimeUTC()
{
	FileTime utcTime;
	GetSystemTimeAsFileTime(&utcTime);

	uint64 retVal;
	retVal = utcTime.dwHighDateTime;
	retVal <<= 32;
	retVal |= utcTime.dwLowDateTime;

	return retVal;
}


uint64 tSystem::tGetTimeGMT()
{
	return tGetTimeUTC();
}


tString tSystem::tConvertTimeToString(uint64 time, tTimeFormat format)
{
	FileTime fileTime;
	fileTime.dwHighDateTime = (time >> 32);
	fileTime.dwLowDateTime = (time & 0x00000000FFFFFFFF);
	SystemTime st;
	FileTimeToSystemTime(&fileTime, &st);

	tString timeStr;
	switch (format)
	{
		case tTimeFormat::Extended:
		{
			tString day;
			switch (st.wDayOfWeek)
			{
				case 0: day = "Sunday";			break;
				case 1: day = "Monday";			break;
				case 2: day = "Tuesday";		break;
				case 3: day = "Wednesday";		break;
				case 4: day = "Thursday";		break;
				case 5: day = "Friday";			break;
				case 6: day = "Saturday";		break;
			}

			tString month;
			switch (st.wMonth)
			{
				case 1:  month = "January";	break;
				case 2:  month = "February";	break;
				case 3:  month = "March";		break;
				case 4:  month = "April";		break;
				case 5:  month = "May";			break;
				case 6:  month = "June";		break;
				case 7:  month = "July";		break;
				case 8:  month = "August";		break;
				case 9:  month = "September";	break;
				case 10: month = "October";	break;
				case 11: month = "November";	break;
				case 12: month = "December";	break;
			}

			tsPrintf
			(
				timeStr,
				"%s %s %d %d - %02d:%02d:%02d",
				day.ConstText(), month.ConstText(), st.wDay, st.wYear,
				st.wHour, st.wMinute, st.wSecond
			);

			break;
		}

		case tTimeFormat::Standard:
		{
			tString day("NaD");
			switch (st.wDayOfWeek)
			{
				case 0: day = "Sun"; break;
				case 1: day = "Mon"; break;
				case 2: day = "Tue"; break;
				case 3: day = "Wed"; break;
				case 4: day = "Thu"; break;
				case 5: day = "Fri"; break;
				case 6: day = "Sat"; break;
			}

			tString month("NaM");
			switch (st.wMonth)
			{
				case 1:  month = "Jan"; break;
				case 2:  month = "Feb";	break;
				case 3:  month = "Mar"; break;
				case 4:  month = "Apr"; break;
				case 5:  month = "May"; break;
				case 6:  month = "Jun"; break;
				case 7:  month = "Jul"; break;
				case 8:  month = "Aug"; break;
				case 9:  month = "Sep"; break;
				case 10: month = "Oct"; break;
				case 11: month = "Nov"; break;
				case 12: month = "Dec"; break;
			}

			tsPrintf
			(
				timeStr,
				"%s %s %2d %4d %2d:%02d:%02d",
				day.ConstText(), month.ConstText(), st.wDay, st.wYear,
				st.wHour, st.wMinute, st.wSecond
			);

			break;
		}
	}

	return timeStr;
}
#endif // PLATFORM_WIN


int64 tSystem::tGetHardwareTimerFrequency()
{
	#ifdef PLATFORM_WIN
	int64 freq;
	QueryPerformanceFrequency((LargeInteger*)&freq);
	return freq;
	#endif
}


int64 tSystem::tGetHardwareTimerCount()
{
	#ifdef PLATFORM_WIN
	int64 count;
	QueryPerformanceCounter((LargeInteger*)&count);
	return count;
	#endif
}


void tSystem::tSleep(int milliSeconds)
{
	::Sleep(milliSeconds);
}