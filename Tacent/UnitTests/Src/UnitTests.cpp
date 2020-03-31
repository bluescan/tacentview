// UnitTests.cpp
//
// Tacent unit tests.
//
// Copyright (c) 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tVersion.h>
#include <System/tCommand.h>
#include "UnitTests.h"
#include "TestBuild.h"
#include "TestFoundation.h"
#include "TestImage.h"
#include "TestMath.h"
#include "TestScene.h"
#include "TestSystem.h"


using namespace tStd;
tCommand::tOption PrintAllOutput("Print all output.", 'a', "all");


namespace tUnitTest
{
	int UnitRequirementNumber = 0;
	int UnitGoalNumber = 0;
	int UnitsSkipped = 0;
	int TotalRequirements = 0;
	int RequirementsPassed = 0;
	int TotalGoals = 0;
	int GoalsPassed = 0;
}


int main(int argc, char** argv)
{
	tCommand::tParse(argc, argv);

#ifdef UNIT_TEST_FORCE_PRINT_ALL_OUTPUT
	PrintAllOutput.Present = true;
#endif

	if (PrintAllOutput)
		tSystem::tSetChannels(tSystem::tChannel_All);
	else
		tSystem::tSetChannels(tSystem::tChannel_TestResult);

	tUnitTest::rPrintf("Testing Tacent Version %d.%d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision, 0);

	#if !defined(UNIT_TEST_ONLY_ONE_TEST)

	// Foundation tests.
	tTest(Types);
	tTest(Array);
	tTest(List);
	tTest(Sort);
	tTest(FixInt);
	tTest(Bitfield);
	tTest(String);
	tTest(RingBuffer);
	tTest(PriorityQueue);
	tTest(MemoryPool);

	// Math tests.
	tTest(Fundamentals);
	tTest(Spline);
	tTest(Hash);
	tTest(Random);
	tTest(Matrix);
	tTest(Quaternion);
	tTest(Geometry);

	// System tests.
	tTest(CmdLine);
	tTest(Task);
	tTest(Print);
	tTest(Regex);
	
	tTest(Script);
	tTest(Chunk);
	
	tTest(File);
	tTest(Time);

	#ifndef PLATFORM_LINUX
	// Build tests.
	tTest(Process);
	#endif

	// Image tests.
	tTest(Image);

	#else
	tTest(MemoryPool);

	#endif

	return tUnitTest::tTestResults();
}
