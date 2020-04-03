// tMachine.cpp
//
// Hardware ans OS access functions like querying supported instruction sets, number or cores, and computer name/ip
// accessors.
//
// Copyright (c) 2004-2006, 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#include <limits.h>
#include <sys/sysinfo.h>
#endif
#include "Foundation/tStandard.h"
#include "System/tFile.h"
#include "System/tMachine.h"


#ifdef PLATFORM_WINDOWS
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
	#ifdef PLATFORM_WINDOWS
	char name[128];
	ulong nameSize = 128;

	WinBool success = GetComputerName(name, &nameSize);
	if (success)
		return name;

	#else
	char hostname[HOST_NAME_MAX];
	int err = gethostname(hostname, HOST_NAME_MAX);
	if (!err)
		return hostname;

	#endif
	return tString();
}


int tSystem::tGetNumCores()
{
	#ifdef PLATFORM_WINDOWS
	// Lets cache this value as it never changes.
	static int numCores = 0;
	if (numCores > 0)
		return numCores;

	SYSTEM_INFO sysinfo;
	tStd::tMemset(&sysinfo, 0, sizeof(sysinfo));
	GetSystemInfo(&sysinfo);

	// dwNumberOfProcessors is unsigned, so can't say just > 0.
	if ((sysinfo.dwNumberOfProcessors == 0) || (sysinfo.dwNumberOfProcessors == -1))
		numCores = 1;
	else
		numCores = sysinfo.dwNumberOfProcessors;

	return numCores;

	#else	
	return get_nprocs_conf();
	
	#endif
}


bool tSystem::tOpenSystemFileExplorer(const tString& dir, const tString& file)
{
	#ifdef PLATFORM_WINDOWS
	tString fullName = dir + file;
	HWND hWnd = ::GetActiveWindow();

	// Just open an explorer window if the dir is invalid.
	if (!tSystem::tDirExists(dir))
	{
		// 20D04FE0-3AEA-1069-A2D8-08002B30309D is the CLSID of "This PC" on Windows.
		ShellExecute(hWnd, "open", "explorer", "/n,::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", 0, SW_SHOWNORMAL);
		return false;
	}

	if (tSystem::tFileExists(fullName))
	{
		fullName.Replace('/', '\\');
		tString options;
		tsPrintf(options, "/select,\"%s\"", fullName.Chars());
		ShellExecute(hWnd, "open", "explorer", options.ConstText(), 0, SW_SHOWNORMAL);
	}
	else
	{
		ShellExecute(hWnd, "open", dir.ConstText(), 0, dir.ConstText(), SW_SHOWNORMAL);
	}
	return true;
	#elif defined(PLATFORM_LINUX)
	// @todo Implament 
	return false;
	#else
	// @todo Implament 
	return false;
	#endif
}


bool tSystem::tOpenSystemFileExplorer(const tString& fullFilename)
{
	return tOpenSystemFileExplorer(tSystem::tGetDir(fullFilename), tSystem::tGetFileName(fullFilename));
}
