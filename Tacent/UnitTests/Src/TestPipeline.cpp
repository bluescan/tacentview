// TestPipeline.cpp
//
// Pipeline module tests.
//
// Copyright (c) 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Pipeline/tProcess.h>
#include "UnitTests.h"
using namespace tPipeline;
namespace tUnitTest
{


tTestUnit(Process)
{
	if (!tSystem::tDirExists("TestData/"))
		tSkipUnit(Process)

	ulong exitCode;
	tString output;

	try
	{
		// This constructor blocks. It fills in the exitCode if you supply it. Output is appended to the output string.
		tProcess("cmd.exe dir", "TestData/", output, &exitCode);
		tPrintf("Output:\n[\n%s\n]\n", output.Pod());
	}
	catch (tError error)
	{
		tPrintf("%s\n", error.Message.Pod());
	}
	tRequire(exitCode == 0);

	try
	{
		tProcess("cmd.exe dir", "TestData/DoesNotExist/", output, &exitCode);
		tPrintf("Output:\n[\n%s\n]\n", output.Pod());
	}
	catch (tError error)
	{
		tPrintf("%s\n", error.Message.Pod());
		tPrintf("We expect an error here since an invalid directory was passed on purpose.\n");
	}
	tRequire(exitCode != 0);
}


}
