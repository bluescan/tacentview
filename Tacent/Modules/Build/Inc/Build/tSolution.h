// tSolution.h
//
// The classes in this file are able to parse Visual Studio solution (sln) and project (vcxproj) files in the simplest
// way. For solution files all it cares about are the vcxproj files that are referred to. For the project files it is
// able to quickly extract the file dependencies inside it. Windows platform only.
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
#ifndef PLATFORM_WINDOWS
#error tSolution only supported on Windows.
#endif
#include <Foundation/tString.h>
#include <Foundation/tList.h>
namespace tBuild
{


class tVisualStudioSolution
{
public:
	tVisualStudioSolution(const tString& slnFile);
	virtual ~tVisualStudioSolution()																					{ ProjectFiles.Empty(); }

	tString Directory;
	tList<tStringItem> ProjectFiles;
};


class tVisualStudioProject
{
public:
	tVisualStudioProject(const tString& projFile);
	virtual ~tVisualStudioProject()																						{ Files.Empty(); }

	tString Directory;
	tList<tStringItem> Files;
};


}


// @todo Move into PCH when we set them up.
struct tSolutionError : public tError
{
	tSolutionError(const char* format, ...)																				: tError("[tSolution] ") { va_list marker; va_start(marker, format); Message += tvsPrintf(Message, format, marker); }
	tSolutionError(const tString& m)																					: tError("[tSolution] ") { Message += m; }
	tSolutionError()																									: tError("[tSolution] ") { }
};
