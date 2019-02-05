// tPlatform.cpp
//
// Tacent platform defines, architecture, and endianness detection. The Tacent library has some preprocessor define
// requirements. One of PLATFORM_NNN, ARCHITECTURE_NNN, and CONFIG_NNN need to be defined. If you haven't bothered
// to define these in the project file with a /D switch, they will be defined for you automatically if you are
// building Windows x64.
//
// Copyright (c) 2004-2006, 2015, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "Foundation/tPlatform.h"
#include "Foundation/tString.h"


tPlatform tGetPlatform()
{
	#if defined(PLATFORM_WIN)
	return tPlatform::Win;
	#elif defined(PLATFORM_LIN)
	return tPlatform::Lin;
	#elif defined(PLATFORM_OSX)
	return tPlatform::OSX;
	#elif defined(PLATFORM_AND)
	return tPlatform::And;
	#elif defined(PLATFORM_IOS)
	return tPlatform::iOS;
	#else
	return tPlatform::Invalid;
	#endif
}


tPlatform tGetPlatform(const tString& name)
{
	for (int p = 0; p < int(tPlatform::NumPlatforms); p++)
		if (name == tGetPlatformName(tPlatform(p)))
			return tPlatform(p);

	return tPlatform::Invalid;
}


const char* tGetPlatformName(tPlatform plat)
{
	const static char* platNames[] =
	{
		"Win",
		"Lin",
		"OSX",
		"And",
		"iOS",
		"All",
		"Invalid",
	};
	tStaticAssert( ((sizeof(platNames)/sizeof(*platNames)) - 2) == int(tPlatform::NumPlatforms) );

	if (plat == tPlatform::Invalid)
		return platNames[int(tPlatform::NumPlatforms)+1];

	return platNames[int(plat)];
}


const char* tGetPlatformNameLong(tPlatform plat)
{
	const static char* platNames[] =
	{
		"Windows",
		"Linux",
		"OSX",
		"Android",
		"iOS",
		"All Platforms",
		"Invalid",
	};
	tStaticAssert( ((sizeof(platNames)/sizeof(*platNames)) - 2) == int(tPlatform::NumPlatforms) );

	if (plat == tPlatform::Invalid)
		return platNames[int(tPlatform::NumPlatforms)+1];

	return platNames[int(plat)];
}


tArchitecture tGetArchitecture()
{
	#if defined(ARCHITECTURE_X64)
	return tArchitecture::x64;
	#elif defined(ARCHITECTURE_A64)
	return tArchitecture::A64;
	#else
	return tArchitecture::Invalid;
	#endif
}


const char* tGetArchitectureName(tArchitecture arch)
{
	const static char* archNames[] =
	{
		"x64",
		"A64",
		"Invalid"
	};
	tStaticAssert( ((sizeof(archNames)/sizeof(*archNames)) - 1) == int(tArchitecture::NumArchitectures) );

	if (arch == tArchitecture::Invalid)
		return archNames[int(tArchitecture::NumArchitectures)];

	return archNames[int(arch)];
}


const char* tGetArchitectureNameLong(tArchitecture arch)
{
	const static char* archNames[] =
	{
		"x64|AMD64|x86-64|64bit",
		"A64|AArch64|64bit",
		"Invalid"
	};
	tStaticAssert( ((sizeof(archNames)/sizeof(*archNames)) - 1) == int(tArchitecture::NumArchitectures) );

	if (arch == tArchitecture::Invalid)
		return archNames[int(tArchitecture::NumArchitectures)];

	return archNames[int(arch)];
}


tEndianness tGetEndianness(tPlatform plat)
{
	switch (plat)
	{
		case tPlatform::Win:
		case tPlatform::Lin:
		case tPlatform::OSX:
		case tPlatform::And:
		case tPlatform::iOS:
			return tEndianness::Little;
	}

	return tEndianness::Invalid;
}
