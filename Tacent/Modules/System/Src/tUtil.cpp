// tUtil.cpp
//
// Utility functions. These include hardware querying utilities like supported instruction sets, number or cores,
// hardware timer functions, and computer name/ip accessors.
//
// Copyright (c) 2004-2006, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WIN
#include <Windows.h>
#include <intrin.h>
#endif
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
	// @todo Implement. Maybe use gethostname.
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
#endif
