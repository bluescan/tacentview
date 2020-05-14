// TestSystem.cpp
//
// System module tests.
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

#include <Foundation/tAssert.h>
#include <Foundation/tMemory.h>
#include <Math/tVector2.h>
#include <Math/tVector3.h>
#include <Math/tVector4.h>
#include <Math/tQuaternion.h>
#include <Math/tMatrix4.h>
#include <System/tCommand.h>
#include <System/tTask.h>
#include <System/tMachine.h>
#include <System/tRegex.h>
#include <System/tScript.h>
#include <System/tChunk.h>
#include <System/tTime.h>
#include "UnitTests.h"
#pragma warning (disable: 4723)
using namespace tSystem;
using namespace tMath;
namespace tUnitTest
{


tTestUnit(CmdLine)
{
	tPrintf("Testing tCommand command line parsing.\n");
	tCommand::tParam fromFile(1, "fromFile");
	tCommand::tParam toFile(2, "toFile");
	tCommand::tOption log("Specify log file.", "log", 'l', 1);
	tCommand::tOption overwrite("Overwrite dest.", "overwrite", 0);
	tCommand::tOption recurse("Recursive.", 'R', 0);
	tCommand::tOption program("Program mode.", 'p', 0);
	tCommand::tOption time("Print timestamp.", "time", 't', 0);
	tCommand::tOption stop("Stop early.", "stop", 's', 0);

	tCommand::tParse("-R --overwrite fileA.txt -pt fileB.txt --log log.txt -l log2.txt --notthere");
	//tCommand::tParse("MyProg.exe -R --overwrite fileA.txt -pt fileB.txt --log log.txt -l log2.txt --notthere", true);

	tCommand::tPrintUsage();

	tRequire(log.IsPresent());
	tRequire(!stop.IsPresent());
	tRequire(fromFile.IsPresent());
	tRequire(toFile.IsPresent());

	// More than one log entry simply adds to the numer option arguments. If an option took 2 args (A B) and was
	// specified twice, you would get A1 B1 A2 B2 for the arguments.
	tPrintf("Option log: %s\n", log.IsPresent() ? "present" : "absent");
	if (log.IsPresent())
		for (tStringItem* optArg = log.Args.First(); optArg; optArg = optArg->Next())
			tPrintf("    Log arg: %s\n", optArg->Pod());

	tPrintf("Param fromFile: %s\n", fromFile.IsPresent() ? "present" : "absent");
	if (fromFile.IsPresent())
		tPrintf("    FromFile: %s\n", fromFile.Param.Pod());

	tPrintf("Param toFile: %s\n", toFile.IsPresent() ? "present" : "absent");
	if (toFile.IsPresent())
		tPrintf("    toFile: %s\n", toFile.Param.Pod());
}


struct MyTask : public tTask
{
	MyTask() { }
	double Execute(double timeDelta) override;
	int ExecuteCount = 0;
};


double MyTask::Execute(double timeDelta)
{
	tPrintf("ExecuteCount: %d  TimeDelta: %f\n", ExecuteCount, timeDelta);
	tGoal(timeDelta < 0.2f);
	ExecuteCount++;

	double runAgainTime = 0.1; // Seconds.
	return runAgainTime;
}


tTestUnit(Task)
{
	int64 freq = tGetHardwareTimerFrequency();
	tTaskSet tasks(freq, 0.1);
	tTask* t1 = new MyTask();
	tTask* t2 = new MyTask();

	tasks.Insert(t1);
	tasks.Insert(t2);

	tPrintf("\n\nStarting Execute Loop\n");
	for (int y=0; y<100; y++)
	{
		tSleep(16);
		int64 count = tGetHardwareTimerCount();
		tasks.Update(count);
	}

	tPrintf("\nRemoving task...\n");
	tasks.Remove(t1);

	tPrintf("\n\nStarting Execute Loop\n");
	for (int y=0; y<50; y++)
	{
		tSleep(16);
		int64 count = tGetHardwareTimerCount();
		tasks.Update(count);
	}

	tPrintf("\nExiting loop\n");
}


// This compares the output of tvsPrintf to the standard vsprintf. Some differences are intended while others are not.
bool PrintCompare(const char* format, ...)
{
	tPrint("\nComparing formatted output. Next three entries: (format, tPrintf, printf)\n");
	tPrint(format);
	va_list args;

	char tbuf[512];
	va_start(args, format);
	int tcount = tvsPrintf(tbuf, format, args);
	va_end(args);

	char nbuf[512];
	va_start(args, format);
	int ncount = vsprintf(nbuf, format, args);
	va_end(args);

	tPrint(tbuf);
	tPrint(nbuf);
	bool match = tStd::tStrcmp(tbuf, nbuf) ? false : true;
	tPrintf
	(
		"Str Match: %s  Len Match: %s\n", match ? "True" : "False",
		(tcount == ncount) ? "True" : "False"
	);
	return match;
}


// Tests the tPrintf formatting engine.
void PrintTest(const char* format, ...)
{
	tPrint("Testing Tacent formatted output. Next two entries: (format, tPrintf)\n");
	tPrint(format);
	va_list args;

	va_start(args, format);
	int tcount = tvPrintf(format, args);
	va_end(args);

	tPrintf("Str Length: %d\n\n", tcount);
}


tTestUnit(Print)
{
	tSetDefaultPrecision(6);
	
	tPrint("tPrintf Tests.\n");
	tRequire(PrintCompare("Hex %#010X\n", 0x0123ABCD));
	tRequire(PrintCompare("Hex %#010x\n", 0));
	tRequire(PrintCompare("Hex %04x\n", 0xFFFFF101));

	PrintTest("Pointer %p\n", 0xFFFFF101);
	PrintTest("Pointer %p\n", 0x00ABC710);
	PrintTest("Pointer %p\n", 0);

	PrintTest("Integer 64bit value neg forty-two:   ___%|64d___\n", int64(-42));
	tRequire(PrintCompare("Integer value neg forty-two:         ___%d___\n", -42));
	tRequire(PrintCompare("Integer value forty-two:             ___%d___\n", 42));

	uint8 u8 = 0xA7;
	PrintTest
	(
		"Binary  1010 0111 (8 bit):\n"
		"      __%08b__\n", u8
	);

	uint16 u16 = 0xA70F;
	PrintTest
	(
		"Binary  1010 0111 0000 1111 (16 bit):\n"
		"      __%16b__\n", u16
	);

	uint32 u32 = 0xA70F1234;
	PrintTest
	(
		"Binary  1010 0111 0000 1111 0001 0010 0011 0100 (32 bit):\n"
		"      __%32b__\n", u32
	);

	uint64 u64 = 0x170F1234B8F0B8F0LL;
	PrintTest
	(
		"Binary  0001 0111 0000 1111 0001 0010 0011 0100 1011 1000 1111 0000 1011 1000 1111 0000 (64 bit):\n"
		"      __%0_64|64b__\n", u64
	);

	PrintTest
	(
		"Octal   0001 3417 0443 2270 7413 4360 (64 bit):\n"
		"      __%0_24:2o__\n", u64
	);

	tRequire(PrintCompare("Percent symbol.                      ___%%___\n"));

	// I prefer the behaviour of windows printf here. If char after % is invalid, just print the character and
	// do NOT print the percent. The only way to get a percent should be %%. Clang and MSVC behave differently.
	#ifdef PLATFORM_WINDOWS
	tRequire(PrintCompare("Invalid char after percent.          ___%^___\n"));
	tRequire(PrintCompare("Invalid char after percent.          ___%%%^___\n"));
	#endif
	
	tRequire(PrintCompare("Float value forty-two:               ___%f___\n", float(42.0f)));
	tRequire(PrintCompare("Float value neg forty-two:           ___%f___\n", float(-42.0f)));
	tRequire(PrintCompare("Double value forty-two:              ___%f___\n", double(42.0)));
	tRequire(PrintCompare("Double value neg forty-two:          ___%f___\n", double(-42.0)));

	tRequire(PrintCompare("Float 42 width 10 leading 0:         ___%010f___\n", float(42.0f)));
	tRequire(PrintCompare("Int 42 width 10 leading 0:           ___%010d___\n", 42));
	tRequire(PrintCompare("Float width 10 lead 0 Left:          ___%-010f___\n", float(42.0f)));
	tRequire(PrintCompare("Int width 10 lead 0 Left:            ___%-010d___\n", 42));

	tRequire(PrintCompare("Int 1234 with prec 6:                ___%.6d___\n", 1234));
	tRequire(PrintCompare("Float value forty-two width 10:      ___%010f___\n", float(42.0f)));

	tVec3 v3b; v3b.x = 1.0f; v3b.y = 2.0f; v3b.z = 3.0f;
	tVector2 v2(1.0f, 2.0f);
	tVector3 v3(1.0f, 2.0f, 3.0f);
	tVector4 v4(1.0f, 2.0f, 3.0f, 4.0f);

	PrintTest("Vector 2D:                           ___%:2v___\n", v2.Pod());
	PrintTest("Vector 3D pod:                       ___%.3v___\n", pod(v3));
	PrintTest("Vector 3D base:                      ___%:3v___\n", v3b);
	PrintTest("Vector 4D:                           ___%:4v___\n", pod(v4));
	PrintTest("Vector 4D %%06.2:4v:                 ___%06.2:4v___\n", pod(v4));
	PrintTest("Vector 4D Alternative:               ___%_:4v___\n", v4.Pod());

	tQuaternion quat(8.0f, 7.0f, 6.0f, 5.0f);
	tStaticAssert(sizeof(tQuaternion) == 16);
	tStaticAssert(sizeof(tVector4) == 16);
	tStaticAssert(sizeof(tVector3) == 12);
	tStaticAssert(sizeof(tVector2) == 8);
	tStaticAssert(sizeof(tMatrix4) == 64);
	PrintTest("Quaternion: %q\n", pod(quat));
	PrintTest("Quaternion Alternate: %_q\n", pod(quat));

	tMatrix4 mat;
	mat.Identity();
	tVector4 c4(1.0f, 2.0f, 3.0f, 4.0f);
	mat.C4 = c4;

	PrintTest("Matrix Normal:\n%05.2m\n", pod(mat));
	PrintTest("Matrix Decorated:\n%_m\n", mat.Pod());

	tString test("This is the tString.");
	tRequire(PrintCompare("tString: %s\n", pod(test)));
	tRequire(PrintCompare("Reg String: %s\n", "A regular string"));

	tRequire(PrintCompare("Char %c\n", 65));
	tRequire(PrintCompare("Char %c %c %c\n", 65, 66, 67));

	#ifdef PLATFORM_WINDOWS
	tPrintf("Windows non-POD tString print.\n");
	tString str = "This sentence is the tString.";
	PrintTest("The string is '%t'. This is a number:%d.\n", str, 42);

	tPrintf("Windows non-POD tMatrix4 print.\n");
	PrintTest("Matrix Decorated:\n%_m\n", mat);

	tString strA("This is string A");
	tString strB("This is string B");

	// Note that you may NOT pass an tStringItem for the %t format specifier.
	tPrintf("StringA:%t  StringB:%t\n", strA, strB);
	char buff[512];
	tsPrintf(buff, "StringA:%t  StringB:%t\n", strA, strB);
	tPrintf("tsPrintf buffer:%s\n", buff);

	#else
	tPrintf("Non-windows platform. Skipping all non-POD print tests.\n");
	#endif

	// Test counting and string printf.
	tVector3 vv(1.0f, 2.0f, 3.0f);
	char buf[256];
	tStd::tMemset(buf, 1, 256);
	int len = tsPrintf(buf, "Vector in string is: %v", pod(vv));
	tPrintf("Str: [%s] LenRet:%d LenAct:%d\n", buf, len, strlen(buf));

	tStd::tMemset(buf, 'Z', 256);
	tsPrintf(buf, 24, "string len 24 vec: %v", pod(vv));
	tPrintf("Str: [%s] LenRet:%d LenAct:%d\n", buf, len, strlen(buf));

	tPrint("Buffer contains:\n");
	tPrint("123456789012345678901234567890\n");
	for (int i = 0; i < 30; i++)
		if (!buf[i])
			tPrintf("~");
		else
			tPrintf("%c", buf[i]);

	tStd::tMemset(buf, 'Z', 256);
	tsPrintf(buf, 24, "v: %4.2v", pod(vv));
	tPrint("\n\nBuffer contains:\n");
	tPrint("123456789012345678901234567890\n");
	for (int i = 0; i < 30; i++)
		if (!buf[i])
			tPrintf("~");
		else
			tPrintf("%c", buf[i]);

	tPrintf("\n\n");

	float negValFlt = -0.65f;
	tRequire(PrintCompare("Lead Zero With Negative Float:%07.3f\n", negValFlt));

	tPrintf("\n\n");
	int negValInt = -42;
	tRequire(PrintCompare("Lead Zero With Negative Int:%07d\n", negValInt));

	// Test special floating-point bitpatterns.
	tRequire(PrintCompare("Float PSNAN: %f\n", tStd::tFloatPSNAN()));
	tRequire(PrintCompare("Float NSNAN: %f\n", tStd::tFloatNSNAN()));
	tRequire(PrintCompare("Float PQNAN: %f\n", tStd::tFloatPQNAN()));
	tRequire(PrintCompare("Float IQNAN: %f\n", tStd::tFloatIQNAN()));
	tRequire(PrintCompare("Float NQNAN: %f\n", tStd::tFloatNQNAN()));
	tRequire(PrintCompare("Float PINF : %f\n", tStd::tFloatPINF()));
	tRequire(PrintCompare("Float NINF : %f\n", tStd::tFloatNINF()));

	tRequire(PrintCompare("tSqrt(-1.0f): %08.3f\n", tSqrt(-1.0f)));
	float fone = 1.0f; float fzero = 0.0f;
	tRequire(PrintCompare("fone/fzero: %08.3f\n", fone/fzero));
	tRequire(PrintCompare("fzero/fzero: %08.3f\n", fzero/fzero));

	tRequire(PrintCompare("Double PSNAN: %f\n", tStd::tDoublePSNAN()));
	tRequire(PrintCompare("Double NSNAN: %f\n", tStd::tDoubleNSNAN()));
	tRequire(PrintCompare("Double PQNAN: %f\n", tStd::tDoublePQNAN()));
	tRequire(PrintCompare("Double IQNAN: %f\n", tStd::tDoubleIQNAN()));
	tRequire(PrintCompare("Double NQNAN: %f\n", tStd::tDoubleNQNAN()));
	tRequire(PrintCompare("Double PINF : %f\n", tStd::tDoublePINF()));
	tRequire(PrintCompare("Double NINF : %f\n", tStd::tDoubleNINF()));

	tPrintf("SpaceForPos and Leading zeros:% 08.3f\n", 65.5775f);
	tRequire(PrintCompare("SpaceForPos and Leading zeros:% 08.3f\n", 65.5775f));

	tRequire(PrintCompare("Test %%f:%f\n", 65.12345678f));	
	tRequire(PrintCompare("Test %%e:%e\n", 65e24));
	tRequire(PrintCompare("Test %%e:%e\n", 123456789.123456789f));
	tRequire(PrintCompare("Test %%e:%e\n", 12345678900.0f));
	tRequire(PrintCompare("Test %%e:%e\n", 1.0f));
	tRequire(PrintCompare("Test %%g:%g\n", 1234567.123456789f));
	tRequire(PrintCompare("Test %%g:%g\n", 65.12345678f));
	tRequire(PrintCompare("Test %%g:%g\n", 651.2345678f));

	tSetDefaultPrecision(4);

	tFileHandle handle = tOpenFile("TestData/Written.log", "wt");
	ttfPrintf(handle, "Log: Here is some timestamped log data. Index = %d\n", 42);
	ttfPrintf(handle, "Warning: And a second log line.\n");
	tCloseFile(handle);
}


void RegexPattern(const char* pattern, const char* test, const char* desc)
{
	tPrintf("%s\n", desc);
	tRegex regex(pattern);
	bool perfectMatch = regex.IsMatch(test);
	tPrintf("Pattern:%s  Test String:%s  Perfect Match:%s\n", pattern, test, perfectMatch ? "Yes" : "No");

	tList<tRegex::Match> matches;
	regex.Search(test, matches);
	for (tRegex::Match* m = matches.First(); m; m = m->Next())
		tPrintf("Submatch index:%d  Length:%d  String:%s\n", m->IndexStart, m->Length, m->GetString(test).Pod());
	tPrintf("\n");
}


tTestUnit(Regex)
{
	tString pattern = "[ABC][DEF]";
	tRegex regex(pattern);
	tString test;
	bool match = false;;

	test = "AB";
	match = regex.IsMatch(test);
	tPrintf("Pattern:%s  Test:%s  Perfect Match:%s\n", pattern.Pod(), test.Pod(), match ? "Yes" : "No");
	tRequire(!match);

	test = "BF";
	match = regex.IsMatch(test);
	tPrintf("Pattern:%s  Test:%s  Perfect Match:%s\n", pattern.Pod(), test.Pod(), match ? "Yes" : "No");
	tRequire(match);

	RegexPattern(".....", "Hello World", "Test '.' to match any character.");
	RegexPattern("(H..).(o..)", "Hello World", "Test '()' groupings.");
	RegexPattern("l+", "Hello World", "Test '+' to match the preceding pattern element one or more times.");
	RegexPattern("Hellp?o World", "Hello World", "Test '?' to match the preceding pattern element zero or one times.");
	RegexPattern("Hellp?o World", "Hellpo World", "Test '?' to match the preceding pattern element zero or one times.");
	RegexPattern("Hellp?o World", "Hellppo World", "Test '?' to match the preceding pattern element zero or one times.");
	RegexPattern("z*bar*en*ess", "barrenness", "Test '*' to match the preceding pattern element zero or more times.");
	RegexPattern("a{4}A", "aaaA", "Test {n} to match exactly n times.");
	RegexPattern("a{4}A", "aaaaA", "Test {n} to match exactly n times.");
	RegexPattern("a{4}", "aaaaa", "Test {n} to match exactly n times.");
	RegexPattern("Ab{3,}C", "AbbC", "Test {n,} to match n or more times.");
	RegexPattern("Ab{3,}C", "AbbbC", "Test {n,} to match n or more times.");
	RegexPattern("Ab{3,}C", "AbbbbC", "Test {n,} to match n or more times.");
	RegexPattern("H{2,4}", "H", "Test {n,m} to match from n to m times.");
	RegexPattern("H{2,4}", "HH", "Test {n,m} to match from n to m times.");
	RegexPattern("H{2,4}", "HHH", "Test {n,m} to match from n to m times.");
	RegexPattern("H{2,4}", "HHHH", "Test {n,m} to match from n to m times.");
	RegexPattern("H{2,4}", "HHHHH", "Test {n,m} to match from n to m times.");
	RegexPattern("Vow[AEIO]", "Vow", "Test [...] to match one item inside.");
	RegexPattern("Vow[AEIO]", "VowI", "Test [...] to match one item inside.");
	RegexPattern("One|Two|Three", "One", "Test | to match alternate possibilities.");
	RegexPattern("One|Two|Three", "Four", "Test | to match alternate possibilities.");
	RegexPattern("Req(One|Two|Three)", "ReqTwo", "Test | to match alternate possibilities.");
	RegexPattern("Req(One|Two|Three)", "ReqFour", "Test | to match alternate possibilities.");
	RegexPattern("llo\\b", "Hello", "Test \\b to match word boundary.");
	RegexPattern("ne\\b two\\b three", "one two three", "Test \\b to match word boundary.");

	// Now using a single regex object.
	tRegex rex("[ \\t]*[A-Za-z0-9\\:]+[ \\t]+[A-Za-z0-9\\:]+[ \\t]*\\([A-Za-z0-9\\:\\*\\&\\:\\, \\t]*\\)[ \\t]*");
	bool isMatch1 = rex.IsMatch("void Foo::Foo(int a, char* b)");
	tPrintf("Test1.  Should pass.  Result:%s\n", isMatch1 ? "Pass" : "Fail");
	tRequire(isMatch1);
	bool isMatch2 = rex.IsMatch("int* Foo::Foo(int a, char* b)");
	tPrintf("Test2.  Should fail.  Result:%s\n", isMatch2 ? "Pass" : "Fail");
	tRequire(!isMatch2);

	RegexPattern("\\w\\w\\w \\W\\W\\W", "a2B !@#", "Test \\w alphanumeric and \\W non-alphanumeric.");
	RegexPattern("\\w", "_", "Test \\w alphanumeric with underscore.");
	RegexPattern("\\W", "_", "Test \\W non-alpha-numeric with underscore.");
	RegexPattern("[^A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_]", "@Dd4_", "Test ^ the not operator.");
	RegexPattern("[^A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_]", "_Dd4_", "Test ^ the not operator.");
	RegexPattern("[^A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_][A-Za-z0-9_]", "bDd4_", "Test ^ the not operator.");
	RegexPattern("\\w*\\s[\\w]*\\s[\\w]*", "one two\tTHR33", "Test \\s whitespace operator.");
	RegexPattern("\\S*", "aw#$", "Test \\S non-whitespace operator.");
	RegexPattern("\\S*", "aw\n$", "Test \\S non-whitespace operator.");
	RegexPattern("\\d*\\D*", "72635JHWas", "Test \\d digit and \\D non-digit.");
	RegexPattern("\\d*", "7263A4190", "Test \\d digit.");
	RegexPattern("^Hello", "Hello", "Test ^ to match beginning of the string.");
	RegexPattern("^Hello ^World", "Hello World", "Test ^ to match beginning of the string.");
	RegexPattern("World$", "World", "Test $ to match end of the string.");
	RegexPattern("World$", "Hello World", "Test $ to match end of the string.");
	RegexPattern("\\a\\a\\a\\A\\A\\A", "abC123", "Test \\a to match letters and \\A to match non-letters.");
	RegexPattern("\\a\\a\\a\\A\\A\\A", "123abC", "Test \\a to match letters and \\A to match non-letters.");
}


tTestUnit(Script)
{
	if (!tDirExists("TestData/"))
		tSkipUnit(Script)

	{
		tScriptWriter ws("TestData/WrittenConfig.cfg");
		ws.Rem("This is a test config file.");
		ws.CR();
		ws.Comp("PosX", 10);
		ws.Comp("PosY", 20);
		ws.Comp("SizeW", 30);
		ws.Comp("SizeH", 40);
	}

	{
		tScriptReader rs("TestData/WrittenConfig.cfg");
		for (tExpression e = rs.First(); e.Valid(); e = e.Next())
		{
			switch (e.Command().Hash())
			{
				case tMath::tHashCT("PosX"):
					tRequire(int(e.Item1()) == 10);
					break;

				case tMath::tHashCT("PosY"):
					tRequire(int(e.Item1()) == 20);
					break;

				case tMath::tHashCT("SizeW"):
					tRequire(int(e.Item1()) == 30);
					break;

				case tMath::tHashCT("SizeH"):
					tRequire(int(e.Item1()) == 40);
					break;
			}
		}
	}

	{
		tScriptWriter ws("TestData/WrittenScript.txt");

		ws.WriteComment();
		ws.WriteComment("A comment!!");
		ws.WriteComment();
		ws.NewLine();

		ws.BeginExpression();
		ws.WriteAtom("A");
		ws.BeginExpression();
		ws.WriteAtom("B");
		ws.WriteAtom("C");
		ws.EndExpression();
		ws.EndExpression();

		ws.NewLine();
		ws.BeginExpression();
		ws.Indent();
		ws.NewLine();
			ws.WriteAtom("A longer atom");
			ws.BeginExpression();
			ws.WriteAtom( tString("M") );
			ws.WriteAtom(-3.0f);
			ws.WriteAtom(300000000000000000.0f);
			ws.WriteAtom(-4);
			ws.WriteAtom(true);
			ws.EndExpression();
			ws.Dedent();
			ws.NewLine();
		ws.EndExpression();
	}

	tPrintf("Testing reading a script.\n");
	int numExceptions = 0;
	try
	{
		tScriptReader rs("TestData/TestScript.txt");

		tExpression arg = rs.Arg0();			// [A [6.8 42 True]]

		tExpression cmd = arg.Command();		// A
		tPrintf("The first command is %s\n", cmd.GetAtomString().Pod());
		tExpression a = arg.Arg1();
		tExpression c = a.Command();
		tExpression d = a.Arg1();
		tExpression e = d.Next();
		tPrintf("c:%f d:%d e:%d\n", c.GetAtomFloat(), d.GetAtomInt(), e.GetAtomBool());

		tExpression arg2 = arg.Next();			// K
		tPrintf("Second main arg %s\n", arg2.GetAtomString().Pod());

		tExpression arg3 = arg2.Next();			// [d	e[ f g]]

		tExpression arg4 = arg3.Next();			// [[H I] "This is a bigger atom" ]
		tExpression cmd4 = arg4.Command();		// [H I]	
		tPrintf("Command4 is-atom: %d\n", cmd4.IsAtom());
		tRequire(!cmd4.IsAtom());

		tExpression arg5 = cmd4.Next();			// "This is a bigger atom"
		tExpression arg5to = arg4.Arg1();		// "This is a bigger atom"

		tPrintf("Last atom %s\n", arg5.GetAtomString().Pod());
		tPrintf("Last atomdup %s\n", arg5to.GetAtomString().Pod());
		tRequire(arg5.GetAtomString() == arg5to.GetAtomString());

		// Lets test variable number of args.
		tExpression argvar = arg4.Next();

		tExpression varcmd = argvar.Command();
		tExpr vararg = argvar.Arg1();
		while (vararg.IsValid())
		{
			tPrintf("Variable arg val :");
			if (vararg.IsAtom())
			{
				tPrintf("%d\n", vararg.GetAtomInt());
				tPrintf("Using implicit cast %d\n", int(vararg));
			}
			else
				tPrintf("Not Atom\n");
			vararg = vararg.Next();
		}

		tExpression quotetest = argvar.Next();
		tExpression quoted = quotetest.Command();
		tPrintf("Quoted atom:%s\n", quoted.GetAtomString().Pod());
		tRequire(quoted.GetAtomString() == "quoted");

		tExpression notquoted = quoted.Next();
		tPrintf("NotQuoted atom:%s\n", notquoted.GetAtomString().Pod());
		tRequire(notquoted.GetAtomString() == "notquoted");

		tExpression vectors = quotetest.Next();
		tVector2 v1 = vectors.Item0().GetAtomVector2();
		tVector2 v2 = vectors.Item1().GetAtomVector2();
		tPrintf("Vector1: (%f, %f)\n", v1.x, v1.y);
		tPrintf("Vector2: (%f, %f)\n", v2.x, v2.y);

		tVector3 v3 = vectors.Item2().GetAtomVector3();
		tVector3 v4 = vectors.Item3().GetAtomVector3();
		tPrintf("Vector3: (%f, %f, %f)\n", v3.x, v3.y, v3.z);
		tPrintf("Vector4: (%f, %f, %f)\n", v4.x, v4.y, v4.z);

		tVector4 v5 = vectors.Item4().GetAtomVector4();
		tVector4 v6 = vectors.Item5().GetAtomVector4();
		tPrintf("Vector5: (%f, %f, %f, %f)\n", v5.x, v5.y, v5.z, v5.w);
		tPrintf("Vector6: (%f, %f, %f, %f)\n", v6.x, v6.y, v6.z, v6.w);

		// This should generate an exception. Need to test that too.
		arg3.GetAtomString();
	}
	catch (tScriptError error)
	{
		numExceptions++;
		tPrintf(error.Message);
		tPrintf("\n");
	}
	tRequire(numExceptions == 1);
}


tTestUnit(Chunk)
{
	if (!tDirExists("TestData/"))
		tSkipUnit(Chunk)

	tPrintf("Testing writing a chunk file.\n");
	{
		tChunkWriter c("TestData/WrittenChunk.bin");
		c.Begin(0x02424242, 64);
		c.Write(tString("Does this work?"));
		c.Write( int8(0x12) );
		c.Write( int16(0x1234) );
		c.Write( int32(0x12345678) );
		c.Write( int64(0x1234567812345678) );
		c.End();

		c.Begin(0x03434343, 32);
		c.Write(tString("Next chunk..."));
		c.Write( int8(0x12) );
		c.Write( int16(0x1234) );
		c.Write( int32(0x12345678) );
		c.Write( int64(0x1234567812345678) );
		c.End();
	}
	tRequire(tFileExists("TestData/WrittenChunk.bin"));

	tPrintf("Testing reading a chunk file.\n");
	{
		tChunkReader c("TestData/WrittenChunk.bin");
		tChunk ch = c.GetFirstChunk();
		while (ch.Valid())
		{
			tPrintf("Chunk ID %x\n", ch.ID());
			tRequire((ch.ID() == 0x02424242) || (ch.ID() == 0x03434343));
			tPrintf("Data: %s\n", ch.GetData());
			tRequire((tString((char*)ch.GetData()) == "Does this work?") || (tString((char*)ch.GetData()) == "Next chunk..."));
			ch = ch.GetNextChunk();
		}
	}

	tPrintf("Another way to read.\n");
	{
		tChunkReader c("TestData/WrittenChunk.bin");
		for (tChunk ch = c.GetFirstChunk(); ch != ch.GetLastChunk(); ch = ch.GetNextChunk())
		{
			tPrintf("Chunk ID %x\n", ch.ID());
			tPrintf("Data %s\n", ch.GetData());
		}
	}

	tPrintf("Reading but managing the memory myself.\n");
	{
		uint8* buffer = (uint8*)tMem::tMalloc(tChunkReader::GetBufferSizeNeeded("TestData/WrittenChunk.bin"), tChunkReader::GetBufferAlignmentNeeded());
		tChunkReader c("TestData/WrittenChunk.bin", buffer);

		for (tChunk ch = c.GetFirstChunk(); ch != ch.GetLastChunk(); ch = ch.GetNextChunk())
		{
			tPrintf("Chunk ID %x\n", ch.ID());
			tPrintf("Data %s\n", ch.GetData());
		}
		tMem::tFree(buffer);
	}
}


tTestUnit(File)
{
	if (!tDirExists("TestData/"))
		tSkipUnit(File)

	tRequire(!tFileExists("TestData/ProbablyDoesntExist.txt"));

	tList<tStringItem> files;
	tFindFiles(files, "TestData/");
	for (tStringItem* file = files.Head(); file; file = file->Next())
		tPrintf("Found file: %s\n", file->Text());

	tString testWinPath = "c:/ADir/file.txt";
	tRequire(tGetDir(testWinPath) == "c:/ADir/");

	tString testLinPath = "/ADir/file.txt";
	tRequire(tGetDir(testLinPath) == "/ADir/");
	
	tList<tStringItem> subDirs;
	tFindDirs(subDirs, "TestData/", true);
	for (tStringItem* subd = subDirs.Head(); subd; subd = subd->Next())
		tPrintf("SubDir: %s\n", subd->Text());

	files.Empty();
	tFindFilesRecursive(files, "TestData/", true);
	for (tStringItem* file = files.Head(); file; file = file->Next())
		tPrintf("Recursive Found File: %s\n", file->Text());

	// Create a directory. Create a file in it. Then delete them all.
	tCreateDir("TestData/CreatedDirectory/");
	tRequire(tDirExists("TestData/CreatedDirectory/"));
	tRequire(!tIsReadOnly("TestData/CreatedDirectory/"));

	tCreateFile("TestData/CreatedDirectory/CreatedFile.txt", "File Contents");
	tRequire(tFileExists("TestData/CreatedDirectory/CreatedFile.txt"));

	tDeleteDir("TestData/CreatedDirectory/");
	tRequire(!tDirExists("TestData/CreatedDirectory/"));

	tString normalPath = "Q:/Projects/Calamity/Crypto/../../Reign/./Squiggle/";
	tPrintf("Testing GetSimplifiedPath on '%s'\n", normalPath.Pod());

	tString simpPath = tGetSimplifiedPath(normalPath);
	tPrintf("Simplified Path '%s'\n", simpPath.Pod());
	tRequire(simpPath =="Q:/Projects/Reign/Squiggle/");
}


tTestUnit(Time)
{
	uint64 epochTime = tGetTimeUTC();	//	h			d		y
	uint64 yearsSince1970 = epochTime / (	3600ULL *	24ULL *	365ULL);
	tPrintf("Years since 1970 UTC: %016|64d\n", yearsSince1970);

	// I wrote the following assert in 2020.
	tRequire(yearsSince1970 >= 50);

	float startTimeSeconds = tGetTime();
	double startTimeSecondsD = tGetTimeDouble();
	tPrintf("Start time seconds: %f %f\n", startTimeSeconds, startTimeSecondsD);
	tSleep(1000);
	float endTimeSeconds = tGetTime();
	double endTimeSecondsD = tGetTimeDouble();
	tPrintf("End time seconds: %f %f\n", endTimeSeconds, endTimeSecondsD);
	tPrintf("Elapsed time: %f %f\n", endTimeSeconds - startTimeSeconds, endTimeSecondsD - startTimeSecondsD);
	tRequire((endTimeSeconds - startTimeSeconds) > 0.99f);
	tRequire((endTimeSecondsD - startTimeSecondsD) > 0.99);

	tTimer timer;
	tPrintf("Timer running: %s\n", timer.IsRunning() ? "true" : "false");

	for (int c = 0; c < 100; c++)
		timer.Update(1.0f);

	tPrintf("100 seconds later.\n");
	tPrintf("Time (seconds)  : %f\n", timer.GetTime());
	tPrintf("Time (seconds)  : %f\n", timer.GetTime(tUnit::tTime::Second));
	tRequire(tApproxEqual(timer.GetTime(), 100.0f));

	timer.Stop();
	tPrintf("Timer running: %s\n", timer.IsRunning() ? "true" : "false");
	for (int c = 0; c < 100; c++)
		timer.Update(1.0f);

	tPrintf("These 100 seconds the timer was stopped.\n");
	tPrintf("Time (seconds)   : %f\n", timer.GetTime());
	tPrintf("Time (seconds)   : %f\n", timer.GetTime(tUnit::tTime::Second));
	tRequire(tApproxEqual(timer.GetTime(tUnit::tTime::Second), 100.0f));

	tPrintf("Time (minutes)   : %f\n", timer.GetTime(tUnit::tTime::Minute));
	tRequire(tApproxEqual(timer.GetTime(tUnit::tTime::Minute), 1.666666f));

	tPrintf("Time (millisecs) : %f\n", timer.GetTime(tUnit::tTime::Millisecond));
	tRequire(tApproxEqual(timer.GetTime(tUnit::tTime::Millisecond), 100000.0f));

	tPrintf("Time (microsecs) : %f\n", timer.GetTime(tUnit::tTime::Microsecond));
	tRequire(tApproxEqual(timer.GetTime(tUnit::tTime::Microsecond), 100000000.0f));

	tPrintf("Time (heleks)    : %f\n", timer.GetTime(tUnit::tTime::Helek));
	tRequire(tApproxEqual(timer.GetTime(tUnit::tTime::Helek), 30.0f));
}


}

