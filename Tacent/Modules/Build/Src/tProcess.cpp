// tProcess.cpp
//
// This module contains a class for spawning other processes and receiving their exit-codes as well as some simple
// commands for spawning one or many processes at once. Windows platform only.
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

#include <System/tThrow.h>
#include <System/tPrint.h>
#include <System/tUtil.h>
#include <Math/tFundamentals.h>
#include "Build/tProcess.h"
using namespace tBuild;


tProcess::tProcess(const tString& cmdLine, const tString& workDir, WindowHandle parent, uint32 userData, bool clearEnvironmentVars, int numEnvPairs, ...) :
	Parent(parent),
	OutputString(nullptr),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(userData),
	ExitCode(nullptr),
	ClearEnvironment(clearEnvironmentVars),
	WaitInDestructor(true),
	Environment(nullptr)
{
	if (numEnvPairs > 0)
	{
		va_list args;
		va_start(args, numEnvPairs);
		Environment = tProcess::BuildNewEnvironmentData_Ascii( !clearEnvironmentVars, numEnvPairs, args );
		va_end(args);
	}

	CreateChildProcess(cmdLine, workDir);
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, WindowHandle parent, uint32 userData, bool clearEnvironmentVars, int numEnvPairs, va_list args) :
	Parent(parent),
	OutputString(nullptr),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(userData),
	ExitCode(nullptr),
	ClearEnvironment(clearEnvironmentVars),
	WaitInDestructor(true),
	Environment(nullptr)
{
	if (numEnvPairs > 0)
		Environment = tProcess::BuildNewEnvironmentData_Ascii( !clearEnvironmentVars, numEnvPairs, args );

	CreateChildProcess(cmdLine, workDir);
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, WinHandle& waitHandle, ulong* exitCode, bool clearEnvironmentVars) :
	Parent(0),
	OutputString(nullptr),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(exitCode),
	ClearEnvironment(clearEnvironmentVars),

	// For this non-blocking constructor, the user of the object is required to wait on the handle!
	WaitInDestructor(false),
	Environment(nullptr)
{
	CreateChildProcess(cmdLine, workDir);
	waitHandle = MonitorProcessExitThread;
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, tString& output, ulong* exitCode, bool clearEnvironmentVars, int numEnvPairs, ...) :
	Parent(0),
	OutputString(&output),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(exitCode),
	ClearEnvironment(clearEnvironmentVars),
	WaitInDestructor(false),
	Environment(nullptr)
{
	if (numEnvPairs > 0)
	{
		va_list args;
		va_start(args, numEnvPairs);
		Environment = tProcess::BuildNewEnvironmentData_Ascii( !clearEnvironmentVars, numEnvPairs, args );
		va_end(args);
	}

	CreateChildProcess(cmdLine, workDir);

	// Wait for exit threads to finish.  After this point we can be sure that the exitCode has been
	// updated if it was non-null.  Note: The exit thread waits for the other threads to finish.
	// Since we are waiting here (blocking), no need to wait in destructor.
	WaitForSingleObject(MonitorProcessExitThread, INFINITE);
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, tString& output, ulong* exitCode, bool clearEnvironmentVars, int numEnvPairs , va_list args) :
	Parent(0),
	OutputString(&output),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(exitCode),
	ClearEnvironment(clearEnvironmentVars),
	WaitInDestructor(false),
	Environment(nullptr)
{
	if (numEnvPairs > 0)
		Environment = tProcess::BuildNewEnvironmentData_Ascii( !clearEnvironmentVars, numEnvPairs, args );

	CreateChildProcess(cmdLine, workDir);

	// Wait for exit threads to finish.  After this point we can be sure that the exitCode has been
	// updated if it was non-null.  Note: The exit thread waits for the other threads to finish.
	// Since we are waiting here (blocking), no need to wait in destructor.
	WaitForSingleObject(MonitorProcessExitThread, INFINITE);
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, ulong* exitCode, bool clearEnvironmentVars, int numEnvPairs, ...) :
	Parent(0),
	OutputString(nullptr),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(exitCode),
	ClearEnvironment(clearEnvironmentVars),
	WaitInDestructor(false),
	Environment(nullptr)
{
	if (numEnvPairs > 0)
	{
		va_list args;
		va_start(args, numEnvPairs);
		Environment = tProcess::BuildNewEnvironmentData_Ascii( !clearEnvironmentVars, numEnvPairs, args );
		va_end(args);
	}

	CreateChildProcess(cmdLine, workDir);

	// Wait for exit threads to finish.  After this point we can be sure that the exitCode has been
	// updated if it was non-null.  Note: The exit thread waits for the other threads to finish.
	// Since we are waiting here (blocking), no need to wait in destructor.
	WaitForSingleObject(MonitorProcessExitThread, INFINITE);
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, ulong* exitCode, bool clearEnvironmentVars, int numEnvPairs, va_list args) :
	Parent(0),
	OutputString(nullptr),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(exitCode),
	ClearEnvironment(clearEnvironmentVars),
	WaitInDestructor(false),
	Environment(nullptr)
{
	if (numEnvPairs > 0)
		Environment = tProcess::BuildNewEnvironmentData_Ascii( !clearEnvironmentVars, numEnvPairs, args );

	CreateChildProcess(cmdLine, workDir);

	// Wait for exit threads to finish.  After this point we can be sure that the exitCode has been
	// updated if it was non-null.  Note: The exit thread waits for the other threads to finish.
	// Since we are waiting here (blocking), no need to wait in destructor.
	WaitForSingleObject(MonitorProcessExitThread, INFINITE);
}


tProcess::tProcess(const tString& cmdLine, const tString& workDir, ulong* exitCode, tPrintCallback pc, void* user) :
	Parent(0),
	OutputString(nullptr),
	PrintCallback(pc),
	PrintCallbackUserPointer(user),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(exitCode),
	ClearEnvironment(true),
	WaitInDestructor(false),
	Environment(nullptr)
{
	CreateChildProcess(cmdLine, workDir);
	WaitForSingleObject(MonitorProcessExitThread, INFINITE);
}


tProcess::tProcess(const tString& cmd, const tString& wd, tExitCallback ec, void* ecud, tPrintCallback pc, void* pcud) :
	Parent(0),
	OutputString(nullptr),
	PrintCallback(pc),
	PrintCallbackUserPointer(pcud),
	ExitCallback(ec),
	ExitCallbackUserPointer(ecud),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(nullptr),
	ClearEnvironment(true),
	WaitInDestructor(true),
	Environment(nullptr)
{
	CreateChildProcess(cmd, wd);
}


tProcess::tProcess(const tString& cmd, const tString& wd) :
	Parent(0),
	OutputString(nullptr),
	PrintCallback(nullptr),
	PrintCallbackUserPointer(nullptr),
	ExitCallback(nullptr),
	ExitCallbackUserPointer(nullptr),
	ChildProcess(0),
	ChildThread(0),
	MonitorProcessExitThread(0),
	MonitorProcessStdOutThread(0),
	MonitorProcessStdErrThread(0),
	StdOutRead(0),
	StdOutWrite(0),
	StdErrRead(0),
	StdErrWrite(0),
	UserData(0),
	ExitCode(nullptr),
	ClearEnvironment(true),
	WaitInDestructor(false),
	Environment(nullptr)
{
	bool detached = true;
	CreateChildProcess(cmd, wd, detached);
}


void tProcess::CreateChildProcess(const tString& cmdLine, const tString& workingDir, bool detached)
{
	if (ExitCode)
		*ExitCode = 0;

	// Create the pipes we'll be using. 
	SECURITY_ATTRIBUTES secAttr;
	secAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 

	// Set the bInheritHandle flag so pipe handles are inherited.
	secAttr.bInheritHandle = detached ? False : True;
	secAttr.lpSecurityDescriptor = 0;

	// By setting the pipe size to be small, we get the process output much sooner
	// since it can't collect in the pipe.
	if (!detached)
	{
		const int suggestedPipeSizeBytes = 32;
		WinBool success = CreatePipe(&StdOutRead, &StdOutWrite, &secAttr, suggestedPipeSizeBytes);
		if (!success)
		{
			if (ExitCode)
				*ExitCode = 1;
			throw tError("Can not create child pipe.");
		}

		success = CreatePipe(&StdErrRead, &StdErrWrite, &secAttr, suggestedPipeSizeBytes);
		if (!success)
		{
			if (ExitCode)
				*ExitCode = 1;
			throw tError("Can not create child pipe.");
		}
	}

	// Create the Child process.
	STARTUPINFO startup;
	startup.cb =					sizeof(STARTUPINFO);
	startup.lpReserved =			0;
	startup.lpDesktop =				0;
	startup.lpTitle =				0;
	startup.dwX =					0;
	startup.dwY =					0;
	startup.dwXSize =				0;
	startup.dwYSize =				0;
	startup.dwXCountChars =			0;
	startup.dwYCountChars =			0;
	startup.dwFillAttribute =		0;
	startup.dwFlags =				detached ? 0 : STARTF_USESTDHANDLES;
	startup.wShowWindow =			0;
	startup.cbReserved2 =			0;
	startup.lpReserved2 =			0;
	startup.hStdInput =				0;
	startup.hStdOutput =			detached ? 0 : StdOutWrite;
	startup.hStdError =				detached ? 0 : StdErrWrite;

	// This struct gets filled out.
	PROCESS_INFORMATION procInfo;

	// The environment block sets the environment variables for the command. Here we make sure that there are
	// effectively none of them if the user requested such. This ensures that the running bahaviour is identical on any
	// machine no matter what system env variables might be set. A null env block (Environment) means inherit from the
	// parent process which is what we are trying to avoid in some cases.
	char* envBlock = Environment;

	// Assign a default env block in this case. We could probably call it with "\0\0", but this seems safer as the
	// windows docs to not explicitely mention passing in an empty block.
	if (ClearEnvironment)
		envBlock = "PIPELINE=true\0";

	int success = CreateProcess(0, (char*)cmdLine.ConstText(), 0, 0, TRUE, DETACHED_PROCESS, envBlock, workingDir.ConstText(), &startup, &procInfo);
	if (!success)
	{
		ulong lastError = GetLastError();
		if (ExitCode)
			*ExitCode = 1;

		throw tError("CreateProcess failed with %d (0x%08X). Possibly due to windows security settings or invalid working dir.", lastError, lastError);
	}
	ChildProcess = procInfo.hProcess;
	ChildThread = procInfo.hThread;

	if (detached)
	{
		CloseHandle(ChildProcess);
		ChildProcess = 0;
		CloseHandle(ChildThread);
		ChildThread = 0;
	}
	else
	{
		// Create Monitor threads. These ones send the messages or print the output.
		ulong threadID;
		MonitorProcessExitThread = CreateThread(0, 0, tProcess::MonitorExitBridge, this, 0, &threadID);
		MonitorProcessStdOutThread = CreateThread(0, 0, tProcess::MonitorStdOutBridge, this, 0, &threadID);
		MonitorProcessStdErrThread = CreateThread(0, 0, tProcess::MonitorStdErrBridge, this, 0, &threadID);
	}
}


void tProcess::Terminate()
{
	if (!ChildProcess)
		return;

	uint32 exitCode = 23;
	WinBool success = TerminateProcess(ChildProcess, exitCode);

	// Note we do NOT set ChildProcess to 0 here.  The MonitorExit thread should handle this for us.
}


void tProcess::TerminateHard()
{
	if (!ChildProcess)
		return;

	uint32 exitCode = 23;
	WinBool success = TerminateProcess(ChildProcess, exitCode);

	Parent = 0;
	WaitForSingleObject(MonitorProcessExitThread, INFINITE);
	WaitInDestructor = false;

	if (MonitorProcessExitThread)
	{
		CloseHandle(MonitorProcessExitThread);
		MonitorProcessExitThread = 0;
	}

	if (MonitorProcessStdOutThread)
	{
		CloseHandle(MonitorProcessStdOutThread);
		MonitorProcessStdOutThread = 0;
	}

	if (MonitorProcessStdErrThread)
	{
		CloseHandle(MonitorProcessStdErrThread);
		MonitorProcessStdErrThread = 0;
	}

	if (StdOutRead)
	{
		CloseHandle(StdOutRead);
		StdOutRead = 0;
	}

	if (StdOutWrite)
	{
		CloseHandle(StdOutWrite);
		StdOutWrite = 0;
	}

	if (StdErrRead)
	{
		CloseHandle(StdErrRead);
		StdErrRead = 0;
	}

	if (StdErrWrite)
	{
		CloseHandle(StdErrWrite);
		StdErrWrite = 0;
	}
}


tProcess::~tProcess()
{
	delete[] Environment;

	// Wait for exit threads to finish before closing the handles!  This destructor will block until
	// the process has finished what it is doing.  Once unblocked, if there is a parent window, we
	// send a message.  Note: The exit thread waits for the other threads to finish.
	if (WaitInDestructor)
		WaitForSingleObject(MonitorProcessExitThread, INFINITE);

	if (MonitorProcessExitThread)
		CloseHandle(MonitorProcessExitThread);

	if (MonitorProcessStdOutThread)
		CloseHandle(MonitorProcessStdOutThread);

	if (MonitorProcessStdErrThread)
		CloseHandle(MonitorProcessStdErrThread);

	if (StdOutRead)
		CloseHandle(StdOutRead);

	if (StdOutWrite)
		CloseHandle(StdOutWrite);

	if (StdErrRead)
		CloseHandle(StdErrRead);

	if (StdErrWrite)
		CloseHandle(StdErrWrite);
}


void tProcess::MonitorExit()
{
	// Wait until the child process is finished doing its thing.
	WaitForSingleObject(ChildProcess, INFINITE);

	// It's important that we set this to failure by default.  It seems that
	// sometimes the GetExitCode function will not touch it... in particular, if
	// the working dir did not exist.
	ulong exitCode = 42;
	GetExitCodeProcess(ChildProcess, &exitCode);

	// Now the pipe readers know that everything has stopped.
	CloseHandle(ChildProcess);
	ChildProcess = 0;
	CloseHandle(ChildThread);
	ChildThread = 0;

	if (ExitCode)
		*ExitCode = exitCode;

	if (ExitCallback)
		ExitCallback(ExitCallbackUserPointer, exitCode);

	WinHandle handles[] =
	{
		MonitorProcessStdOutThread,
		MonitorProcessStdErrThread,
	};

	// Wait until all output messages have been sent.
	WaitForMultipleObjects(2, handles, TRUE, INFINITE);

	if (Parent)
		PostMessage(Parent, uint(tMessage::ProcessDone), exitCode, UserData);

	ExitThread(0);
}


void tProcess::MonitorStdOut()
{
	bool lastWasNewline = false;
	while (1)
	{
		ulong avail;
		PeekNamedPipe(StdOutRead, 0, 0, 0, &avail, 0);

		while (avail > 0)
		{
			const int bufSize = 64;
			tString buf(bufSize);

			int numToRead = tMath::tMin(bufSize - 1, int(avail));

			ulong numRead;
			WinBool success = ReadFile(StdOutRead, buf.Text(), numToRead, &numRead, 0);
			if (success)
			{
				buf.Replace('\r', ' ');

				if (Parent)
					PostMessage(Parent, uint(tMessage::StdOutString), WinWParam(new tString(buf)), 0);

				if (OutputString)
					*OutputString += buf;

				if (PrintCallback)
					PrintCallback(PrintCallbackUserPointer, buf.ConstText());

				// We only go to stdout if all other methods failed.
				if (!Parent && !OutputString && !PrintCallback)
					tPrintf("%s", buf.Pod());

				avail -= numRead;
			}
		}

		if (!Parent)
			tFlush(stdout);

		if (!ChildProcess)
			ExitThread(0);

		// Not required... just keeps CPU overhead way down.
		tSystem::tSleep(10);
	}
}


void tProcess::MonitorStdErr()
{
	while (1)
	{
		ulong avail;
		PeekNamedPipe(StdErrRead, 0, 0, 0, &avail, 0);

		while (avail > 0)
		{
			const int bufSize = 64;

			// Remember, this gets all 0s.
			tString buf(bufSize);

			int numToRead = tMath::tMin(bufSize - 1, int(avail));

			ulong numRead;
			WinBool success = ReadFile(StdErrRead, buf.Text(), numToRead, &numRead, 0); 
			if (success)
			{
				buf.Replace('\r', ' ');

				if (Parent)
					PostMessage(Parent, uint(tMessage::StdErrString), WinWParam(new tString(buf)), 0);

				//if (OutputString)
				//	*OutputString += buf;

				if (PrintCallback)
					PrintCallback(PrintCallbackUserPointer, buf.ConstText());

				// We only go to stdout if all other methods failed.
				if (!Parent && !OutputString && !PrintCallback)
					tPrintf("%s", buf.Pod());

				avail -= numRead;
			}
		}

		if (!Parent)
			tFlush(stdout);

		if (!ChildProcess)
			ExitThread(0);

		// Not required... just keeps CPU overhead way down.
		tSystem::tSleep(10);
	}
}


uint32 tProcess::GetEnvironmentDataLength_Ascii(void* enviro)
{
	char* envStr = (char*)enviro;
	int envIx = 0;

	bool doneAll = false;
	
	while (!doneAll)
	{
		while (envStr[envIx++]);
		if (envStr[envIx] == 0)
		{
			envIx++;
			doneAll = true;
		}
	}

	return envIx;
}


char* tProcess::BuildNewEnvironmentData_Ascii(bool appendToExisting, int numPairs, va_list args)
{
	char* oldEnviro = 0;
	if (appendToExisting)
		oldEnviro = ::GetEnvironmentStrings();

	const char pairSeparatingCharacter = '\0';

	tList<tStringItem> names(true);
	tList<tStringItem> values(true);

	int oldSize = 0;
	if (oldEnviro)
		oldSize = tProcess::GetEnvironmentDataLength_Ascii(oldEnviro);

	int newSize = 0;
	for (int p = 0; p < numPairs; ++p)
	{
		const char* name = va_arg(args, const char*);
		const char* value = va_arg(args, const char*);

		newSize += int(strlen(name)) + 1;
		newSize += int(strlen(value)) + 1;

		names.Append( new tStringItem(name) );
		values.Append( new tStringItem(value) );
	}
	
	int totalSize = oldSize + newSize;

	char* newenvdata = new char[totalSize];
	int newDstIdx = 0;
	if (oldEnviro)
	{
		tStd::tMemcpy(newenvdata, oldEnviro, oldSize);
		newDstIdx = oldSize - 1;
	}
	
	tStringItem* name = names.First();
	tStringItem* value = values.First();
	for (; name && value; name = name->Next(), value = value->Next())
	{
		int namelen = name->Length();
		int valuelen = value->Length();

		tStd::tMemcpy(&newenvdata[newDstIdx], name->ConstText(), namelen);
		newenvdata[newDstIdx+namelen] = '=';
		tStd::tMemcpy(&newenvdata[newDstIdx+namelen+1], value->ConstText(), valuelen);
		newenvdata[newDstIdx+namelen+1+valuelen] = pairSeparatingCharacter;
		newDstIdx += namelen+1+valuelen+1;
	}	
	newenvdata[newDstIdx] = '\0';

	::FreeEnvironmentStrings(oldEnviro);
	return newenvdata;
}
