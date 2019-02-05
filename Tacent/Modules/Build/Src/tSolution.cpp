// tSolution.cpp
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

#include <System/tFile.h>
#include <Math/tFundamentals.h>
#include "Build/tSolution.h"
using namespace tBuild;


tVisualStudioSolution::tVisualStudioSolution(const tString& slnFile) :
	Directory(),
	ProjectFiles()
{
	if (!tSystem::tFileExists(slnFile))
		throw tSolutionError("Cannot process solution file [%s] for dependencies.", slnFile.Pod());

	int numBytes = tSystem::tGetFileSize(slnFile);
	if (numBytes <= 0)
		return;

	Directory = tSystem::tGetDir(slnFile);

	// Load the file into a string.
	tString fileData(numBytes);
	tSystem::tLoadFile(slnFile, (uint8*)fileData.Text());
	tString extension(".vcxproj");

	int startIndex = 0;
	int extIndex = fileData.FindString(extension, startIndex);
	while (extIndex != -1)
	{
		int startOfProjFile = fileData.FindChar('"', true, extIndex);
		if (startOfProjFile == -1)
			break;
		startOfProjFile++;

		int endOfProjFile = extIndex + extension.Length();
		endOfProjFile--;

		tString vcxProjFile(256);
		for (int c = 0; c < (endOfProjFile-startOfProjFile)+1; c++)
			vcxProjFile[c] = fileData[startOfProjFile+c];

		vcxProjFile.Replace('\\', '/');
		vcxProjFile = tSystem::tGetSimplifiedPath(Directory + vcxProjFile);

		ProjectFiles.Append(new tStringItem(vcxProjFile));

		startIndex = extIndex+1;
		extIndex = fileData.FindString(extension, startIndex);
	}
}


tVisualStudioProject::tVisualStudioProject(const tString& projFile) :
	Directory(),
	Files()
{
	if (!tSystem::tFileExists(projFile))
		throw tSolutionError("Cannot process project file [%s] for dependencies.", projFile.Pod());

	int numBytes = tSystem::tGetFileSize(projFile);
	if (numBytes <= 0)
		return;

	Directory = tSystem::tGetDir(projFile);

	// Load the file into a string.
	tString fileData(numBytes);
	tSystem::tLoadFile(projFile, (uint8*)fileData.Text());
	tString tagA("ClCompile Include=");
	tString tagB("ClInclude Include=");

	int startIndex = 0;
	int tagAIndex = fileData.FindString(tagA, startIndex);
	int tagBIndex = fileData.FindString(tagB, startIndex);
	int tagIndex = tMath::tMin(tagAIndex, tagBIndex);
	while (tagIndex != -1)
	{
		int startOfFile = fileData.FindChar('"', false, tagIndex);
		if (startOfFile == -1)
			break;
		startOfFile++;

		int endOfFile = fileData.FindChar('"', false, startOfFile);
		endOfFile--;

		tString vcFile(256);
		for (int c = 0; c < (endOfFile-startOfFile)+1; c++)
			vcFile[c] = fileData[startOfFile+c];

		vcFile.Replace('\\', '/');
		vcFile = tSystem::tGetSimplifiedPath(Directory + vcFile);

		Files.Append(new tStringItem(vcFile));

		startIndex = endOfFile;
		tagAIndex = fileData.FindString(tagA, startIndex);
		tagBIndex = fileData.FindString(tagB, startIndex);
		tagIndex = tMath::tMin(tagAIndex, tagBIndex);
	}
}
