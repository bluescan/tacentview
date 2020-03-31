// tProcess.h
//
// This module contains a class for spawning other processes and receiving their exit-codes as well as some simple
// commands for spawning one or many processes at once. Windows platform only.
//
// Copyright (c) 2005, 2017, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tPlatform.h>
#ifndef PLATFORM_WINDOWS
#error tProcess only supported on Windows.
#endif
#include <Foundation/tString.h>
#include <Foundation/tList.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>					// Requires windows because the build methods can send windows messages.
namespace tBuild
{


// Core reserved message IDs that won't conflict with Windows messages.
enum class tMessage
{
	StdOutString = WM_APP,				// Wparam is tString pointer.
	StdErrString,						// Wparam is tString pointer.
	ProcessDone,						// Wparam is the exit code. Lparam is the user data.
	FileChanged,

	// A message for live tuning. wparam contains index(2bytes) and offset (2bytes). Lparam contains value (4bytes).
	Tune,
	TuneFeedback,
	Last								// Users of Tacent that need their own messages should start here.
};


// Once created this object sends messages to the thread that constructed it.
class tProcess
{
public:
	typedef void (*tExitCallback)(void* userPointer, int exitCode);
	typedef void (*tPrintCallback)(void* userPointer, const char* text);

	// Non-blocking. These constructors send messages to the parent including output and exit-code. This allows the
	// caller to continue working up till the point at which the destructor is called. At that point the destructor may
	// block if still necessary. The messages sent are listed above. In general as a user of this interface you'll do
	// some work while waiting for the ProcessDone message, at which point you may delete the object. User data gets
	// sent as the second param to ProcessDone, the exit code is the first.
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, WindowHandle parent,
		uint32 userData = 0, bool clearEnvironmentVars = false, int numEnvPairs = 0, ...
	);
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, WindowHandle parent, uint32 userData,
		bool clearEnvironmentVars, int numEnvPairs, va_list args
	);

	// Non-blocking. Clears environment variables. You _must_ wait on waitHandle before the destructor is called.
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, WinHandle& waitHandle,
		ulong* exitCode = 0, bool clearEnvironmentVars = true
	);

	// Non-blocking. Supports print and exit callbacks. You simply destroy the object sometime after the exit callback.
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, tExitCallback, void* exitCallbackUserPointer = nullptr,
		tPrintCallback = nullptr, void* printCallbackUserPointer = nullptr
	);
	
	// Non-blocking. Creates a completely detached process. You get no exit code OR process output.
	tProcess(const tString& cmdLine, const tString& workingDir);

	// Blocking. These constructors fill in the exitCode if you supply it. Output is appended to the output arg.
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, tString& output, ulong* exitCode = 0,
		bool clearEnvironmentVars = false, int numEnvPairs = 0, ...
	);
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, tString& output, ulong* exitCode,
		bool clearEnvironmentVars, int numEnvPairs, va_list args
	);

	// Blocking. These constructors fill in the exitCode you supply. Output test is sent as it occurs to stdout and to,
	// optionally, a user-specified print function.
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, ulong* exitCode, bool clearEnvironmentVars = true,
		int numEnvPairs = 0, ...
	);
	tProcess
	(
		const tString& cmdLine, const tString& workingDir, ulong* exitCode, bool clearEnvironmentVars,
		int numEnvPairs, va_list args
	);

	tProcess
	(
		const tString& cmdLine, const tString& workingDir, ulong* exitCode,
		tPrintCallback = nullptr, void* printCallbackUserPointer = nullptr
	);

	// For non-blocking constructors other than the detached constructor, the destructor blocks until the process is
	// complete. i.e. If you delete the object before the exit message is received, you can expect to wait.
	virtual ~tProcess();

	bool IsRunning() const																								{ return ChildProcess ? true : false; }
	WinHandle GetWaitHandle() const																						{ return MonitorProcessExitThread; }

	// This will stop a non-blocked running process. Terminate is a bit of a sledge-hammer. It has no effect if the
	// process is not running because it's already completed. The done message, if any, will be posted asynchronously.
	void Terminate();

	// Same as above, but ensures termination before returning (blocks) and will not post any done messages.
	void TerminateHard();

	static uint32 GetEnvironmentDataLength_Ascii(void* enviro);
	static char* BuildNewEnvironmentData_Ascii(bool appendToExisting, int numPairs, va_list);

private:
	void CreateChildProcess(const tString& cmdLine, const tString& workingDir, bool detached = false);

	// If Parent is valid, output gets sent via messages to that window handle. If OutputString is valid output gets
	// appended to it.
	WindowHandle Parent;
	tString* OutputString;
	tPrintCallback PrintCallback;
	void* PrintCallbackUserPointer;
	tExitCallback ExitCallback;
	void* ExitCallbackUserPointer;

	WinHandle ChildProcess;
	WinHandle ChildThread;

	static ulong WinApi MonitorExitBridge(void* process)																{ ((tProcess*)process)->MonitorExit(); return 0; }
	void MonitorExit();

	static ulong WinApi MonitorStdOutBridge(void* process)																{ ((tProcess*)process)->MonitorStdOut(); return 0; }
	void MonitorStdOut();

	static ulong WinApi MonitorStdErrBridge(void* process)																{ ((tProcess*)process)->MonitorStdErr(); return 0; }
	void MonitorStdErr();

	WinHandle MonitorProcessExitThread;
	WinHandle MonitorProcessStdOutThread;
	WinHandle MonitorProcessStdErrThread;

	WinHandle StdOutRead;				// The read end of the stdout pipe that the MonitorProcessSdtOutThread uses.
	WinHandle StdOutWrite;				// The write end of the stdout pipe that the ChildProcess is using.

	WinHandle StdErrRead;				// The read end of the stderr pipe that the MonitorProcessSdtOutThread uses.
	WinHandle StdErrWrite;				// The write end of the stderr pipe that the ChildProcess is using.

	uint32 UserData;
	ulong* ExitCode;
	bool ClearEnvironment;
	
	// If we're blocking we do the wait immediately, otherwise non-blocking and we wait in the destructor.
	bool WaitInDestructor;
	char* Environment;					// Owned by this class.
};


// @todo Two handy utility functions that are less powerful than tProcess. Neither one prints or captures stdout or
// stderr. tRun blocks and returns the exit code, tGo does not block and you have no idea how the forked process is
// progressing.
//
// int tRun(const tString& cmdLine, const tString& workingDir);
// void tGo(const tString& cmdLine, const tString& workingDir);


}
