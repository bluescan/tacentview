// tRule.h
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

#pragma once
#include <Foundation/tString.h>
#include <Foundation/tList.h>
#include <System/tThrow.h>
namespace tPipeline
{


class tRule : public tLink<tRule>
{
public:
	tRule()																												: Target(), Dependencies(), Clean(false), Config(tConfig::Default) { }
	virtual ~tRule()																									{ Dependencies.Empty(); }
	virtual void Build()																								{ } // You may override this to build the rule.
	virtual char* GetName() const																						{ return nullptr; }

	// If clean is set to true, the OutOfDate() function will always return true.
	void SetClean(bool clean = true)																					{ Clean = clean; }

	enum class tConfig
	{
		Default,
		Debug,
		Develop,
		Release,
		Ship
		// Whatever else is needed can just be added.
	};

	// The build function can optionally get access to the configuration variable set by this function.
	void SetConfig(tConfig config = tConfig::Default)																	{ Config = config; }

	// Clears the dependencies and sets the target.
	void SetTarget(const tString& fullTargetName);

	// Adds a dependency. If the dependency doesn't exist a tRuleError object is thrown and the dependency is not
	// added. If the dependency was already added, this function does nothing.
	void AddDependency(const tString& fullDepName);

	// Adds a dependency. The string pointer is dereferenced and copied. If the dependency doesn't exist a tRuleError
	// object is thrown and the dependency is not added. If the dependency was already added, this function does
	// nothing.
	void AddDependency(tStringItem* fullDepName);

	// Adds multiple dependencies. The list is left empty and the strings are managed by the tRule. If any dependency
	// doesn't exist a tRuleError object is thrown and that dependency is not added. All dependencies that do exist
	// will be added if they aren't already added.
	void AddDependencies(tList<tStringItem>& deps);

	#ifdef PLATFORM_WINDOWS
	// Adds dependencies that are found inside a Visual Studio project or solution file. If the supplied file is a .sln
	// file, it is parsed for all .vcxproj files it refers to. The vcxproj file contains the actual file names that are
	// added as dependencies. This function does checks to see if the dependency has already been added.
	void AddDependenciesVS(const tString& solutionOrProjectFile);

	// Adds deps from a sln file by parsing it for the individual project files.
	void AddDependenciesVSSln(const tString& projectFile);

	// Adds deps from a single vcxproj project file.
	void AddDependenciesVCXProj(const tString& projectFile);
	#endif

	// Adds multiple dependencies. Does not act recursively on the directory. Extension can be something like "txt".
	// Throws a tRuleError if there are any problems. All dependencies that can be added will be. Checks for duplicate
	// deps. Ignores hidden files and folders and will not add them or recurse into them.
	void AddDependencyDir(const tString& dir, const tString& extension = tString());

	// Adds multiple dependencies. Acts recursively on the directory. Extension can be something like "txt". Throws a
	// tRuleError if there are any problems. All dependencies that can be added will be. Checks for duplicate deps.
	// Ignores hidden files and folders and will not add them or recurse into them.
	void AddDependencyDirRec(const tString& dir, const tString& extension = tString());

	// Returns true if target has been specified and target doesn't exist or is older than any dependency or if a clean
	// build is requested (the latter only if checkCleanFlag == true). Returns false if there is no need to build.
	// Throws a tRuleError if any dependency doesn't exist.
	bool OutOfDate(bool checkCleanFlag = true);

	// Here are some aliases so you don't have to type as much.
	void AddDep(const tString& fullDepName)																				{ AddDependency(fullDepName); }
	void AddDep(tStringItem* fullDepName)																				{ AddDependency(fullDepName); }
	void AddDeps(tList<tStringItem>& deps)																				{ AddDependencies(deps); }
	void AddDepsVS(const tString& solnOrProjFile)																		{ AddDependenciesVS(solnOrProjFile); }
	void AddDepDir(const tString& dir, const tString& filter = "*.*")													{ AddDependencyDir(dir, filter); }
	void AddDepDirRec(const tString& dir, const tString& filter = "*.*")												{ AddDependencyDirRec(dir, filter); }

protected:
	tString Target;
	tList<tStringItem> Dependencies;
	bool Clean;
	tConfig Config;

private:
	bool MaybeAddToDependenciesCaseInsensitive(const tString&);
};


}


// @todo Move into PCH when we set them up.
struct tRuleError : public tError
{
	tRuleError(const char* format, ...)																					: tError("[tRule] ") { va_list marker; va_start(marker, format); Message += tvsPrintf(Message, format, marker); }
	tRuleError(const tString& m)																						: tError("[tRule] ") { Message += m; }
	tRuleError()																										: tError("[tRule] ") { }
};
