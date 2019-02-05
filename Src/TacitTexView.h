// UnitTests.h
//
// Tacent unit test framework.
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

#pragma once
#include <System/tPrint.h>
#include <System/tFile.h>
//#define UNIT_TEST_FORCE_PRINT_ALL_OUTPUT
//#define UNIT_TEST_CALLSTACK_ON_FAIL_REQUIREMENT
//#define UNIT_TEST_CALLSTACK_ON_FAIL_GOAL


namespace tUnitTest
{


// tRequire takes a boolean expression as input. A failure of a tRequire (false evaluation of the expression)
// means the test will fail and the overall result of the unit tests will be a fail.
#ifdef UNIT_TEST_CALLSTACK_ON_FAIL_REQUIREMENT
	#define tRequire(expr) { tCheckRequire((expr) ? true : false, #expr, __FILE__, __LINE__, 0); tAssert(expr); }
#else
	#define tRequire(expr) tCheckRequire((expr) ? true : false, #expr, __FILE__, __LINE__, 0);
#endif


// Use tGoal if it would be nice if the expression evaluates to true. A failure will be reported, but it will not
// prevent the test suite from receiving a pass.
#ifdef UNIT_TEST_CALLSTACK_ON_FAIL_GOAL
	#define tGoal(expr) { bool goalPass = (expr)?true:false; tCheckGoal(goalPass, #expr, __FILE__, __LINE__, 0); tAssert(goalPass); }
#else
	#define tGoal(expr) tCheckGoal((expr) ? true : false, #expr, __FILE__, __LINE__, 0);
#endif


#define tTestUnit(name) void name()
#define tSkipUnit(name) { tUnitTest::UnitsSkipped++; tUnitTest::rPrintf("Skipping " #name " Tests\n"); return; }
#define tTest(name)  { tUnitTest::rPrintf("\nTesting " #name "\n"); tUnitTest::UnitRequirementNumber = tUnitTest::UnitGoalNumber = 0; tUnitTest::name(); }
int tTestResults(bool waitForReturn = true);


// Implementation below this line.


extern int UnitRequirementNumber;
extern int UnitGoalNumber;
extern int UnitsSkipped;
extern int TotalRequirements;
extern int RequirementsPassed;
extern int TotalGoals;
extern int GoalsPassed;


inline int rPrintf(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_TestResult, f, l);
	va_end(l);			return n;
}


inline void tCheckRequire(bool pass, const char* expr, const char* fileName, int lineNum, const char* msg)
{
	rPrintf("Require %03d ", UnitRequirementNumber);
	if (pass)
	{
		rPrintf("Pass  [ %s ]\n", expr);
		RequirementsPassed++;
	}
	else
	{;
		rPrintf("Fail  [ %s ]  File: %s  Line: %d\n", expr, tSystem::tGetFileName(fileName).Pod(), lineNum);
	}
	UnitRequirementNumber++;
	TotalRequirements++;
}


inline void tCheckGoal(bool pass, const char* expr, const char* fileName, int lineNum, const char* msg)
{
	rPrintf("Goal    %03d ", UnitGoalNumber);
	if (pass)
	{
		rPrintf("Pass  [ %s ]\n", expr);
		GoalsPassed++;
	}
	else
	{
		rPrintf("Fail  [ %s ]  File: %s  Line: %d\n", expr, tSystem::tGetFileName(fileName).Pod(), lineNum);
	}
	UnitGoalNumber++;
	TotalGoals++;
}


inline int tTestResults(bool waitForReturn)
{
	int errorCode = 1;
	char* result = "Fail";
	if (RequirementsPassed+GoalsPassed == TotalRequirements+TotalGoals)
	{
		result = "Pass";
		errorCode = 0;
	}
	else if (RequirementsPassed == TotalRequirements)
	{
		result = "Pass. Not all goals achieved.";
		errorCode = 0;
	}

	rPrintf("\nTests Complete\n");
	rPrintf("Units Skipped : %d\n", UnitsSkipped);
	rPrintf("Requirements  : %d/%d\n", RequirementsPassed, TotalRequirements);
	rPrintf("Goals Passed  : %d/%d\n", GoalsPassed, TotalGoals);
	rPrintf("Total Passed  : %d/%d\n", RequirementsPassed+GoalsPassed, TotalRequirements+TotalGoals);
	rPrintf("Final Result  : %s\n", result);

	if (waitForReturn)
	{
		tUnitTest::rPrintf("Press Enter to exit.\n");
		int ch = getchar();
	}

	return errorCode;
}


}
