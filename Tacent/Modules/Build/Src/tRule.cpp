// tRule.cpp
//
// The base class for a rule. Rules support functionality such as setting targets/dependencies, and checking if the
// build rule is out of date.
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <System/tThrow.h>
#include <System/tPrint.h>
#include <System/tFile.h>
#include "Build/tRule.h"
#ifdef PLATFORM_WIN
#include "Build/tSolution.h"
#endif
using namespace tBuild;


void tRule::SetTarget(const tString& target)
{
	while (tStringItem* s = Dependencies.Remove())
		delete s;

	Target = target;
}


bool tRule::MaybeAddToDependenciesCaseInsensitive(const tString& dep)
{
	// This function only adds to the dependency list if the file in question hasn't already been added.
	// @todo Does not support changing between relative and absolute.
	if (dep.IsEmpty())
		return false;

	tString lowCase = dep;
	lowCase.Replace('\\', '/');
	lowCase.LowCase();

	for (tStringItem* s = Dependencies.First(); s; s = s->Next())
	{
		tString lowCasePresent = *s;
		lowCasePresent.Replace('\\', '/');
		lowCasePresent.LowCase();

		if (lowCase == lowCasePresent)
			return false;
	}

	// If we get here the file hasn't already been added... so we add it now.
	Dependencies.Append(new tStringItem(dep));
	return true;
}


void tRule::AddDependency(const tString& dep)
{
	if (!tSystem::tFileExists(dep))
		throw tRuleError("Cannot add dependency [%s]", dep.Pod());

	MaybeAddToDependenciesCaseInsensitive(dep);
}



void tRule::AddDependency(tStringItem* dep)
{
	if (!tSystem::tFileExists(*dep))
		throw tRuleError("Cannot add dependency [%s]", dep->Pod());

	MaybeAddToDependenciesCaseInsensitive(*dep);
}


void tRule::AddDependencies(tList<tStringItem>& deps)
{
	bool success = true;

	tString badDependency;
	while (tStringItem* dep = deps.Remove())
	{
		if (!tSystem::tFileExists(*dep))
		{
			success = false;
			badDependency = *dep;
			delete dep;
		}
		else
		{
			MaybeAddToDependenciesCaseInsensitive(*dep);
		}
	}

	if (!success)
		throw tRuleError("Cannot add dependency [%s]", badDependency);
}


#ifdef PLATFORM_WIN
void tRule::AddDependenciesVS(const tString& solutionOrProjectFile)
{
	if (tSystem::tGetFileExtension(solutionOrProjectFile) == "sln")
		AddDependenciesVSSln(solutionOrProjectFile);
	else if (tSystem::tGetFileExtension(solutionOrProjectFile) == "vcxproj")
		AddDependenciesVCXProj(solutionOrProjectFile);
}


void tRule::AddDependenciesVSSln(const tString& solutionFile)
{
	tVisualStudioSolution sln(solutionFile);
	for (tStringItem* projFile = sln.ProjectFiles.First(); projFile; projFile = projFile->Next())
		AddDependenciesVCXProj(*projFile);
}


void tRule::AddDependenciesVCXProj(const tString& projectFile)
{
	if (tSystem::tGetFileExtension(projectFile) == "vcxproj")
	{
		tVisualStudioProject proj(projectFile);
		for (tStringItem* depFile = proj.Files.First(); depFile; depFile = depFile->Next())
			MaybeAddToDependenciesCaseInsensitive(*depFile);
	}
}
#endif


void tRule::AddDependencyDir(const tString& dir, const tString& filter)
{
	tList<tStringItem> deps;
	bool includeHidden = false;
	tSystem::tFindFilesInDir(deps, dir, filter, includeHidden);

	AddDependencies(deps);
	tAssert(deps.IsEmpty());
}


void tRule::AddDependencyDirRec(const tString& dir, const tString& filter)
{
	tList<tStringItem> deps;
	bool includeHidden = false;
	tSystem::tFindFilesRecursive(deps, dir, filter, includeHidden);

	AddDependencies(deps);
	tAssert(deps.IsEmpty());
}


bool tRule::OutOfDate(bool checkClean)
{
	// Returns true if target has been specified and target doesn't exist or is older than any dependency.
	// Also returns false if any dep doesn't exist.
	if (Target.IsEmpty())
		return false;

	tStringItem* dep = Dependencies.First();
	while (dep)
	{
		if (!tSystem::tFileExists(*dep))
			throw tRuleError("Cannot find dependency [%s] while targetting [%s].", dep->ConstText(), Target.ConstText());

		dep = dep->Next();
	}

	if (!tSystem::tFileExists(Target))
		return true;

	if (checkClean && Clean)
		return true;

	dep = Dependencies.First();
	while (dep)
	{
		if (tSystem::tIsFileNewer(*dep, Target))
			return true;

		dep = dep->Next();
	}

	return false;
}
