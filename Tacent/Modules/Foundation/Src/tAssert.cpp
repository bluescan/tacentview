// tAssert.cpp
//
// Tacent asserts and warnings.
//
// Copyright (c) 2004-2006, 2015, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <stdio.h>
#include "Foundation/tPlatform.h"
#ifdef PLATFORM_WIN
#include <signal.h>
#include <Windows.h>
#endif


void tAbort()
{
	// Note the use of stdio printf here to reduce dependencies.
	printf("%s(%d) Tacent Abort\n", __FILE__, __LINE__);
	abort();
}


void tAssertPrintBreak(const char* expr, const char* fileName, int lineNum, const char* msg)
{
	const int assertMessageSize = 4*1024;
	char message[assertMessageSize];

	snprintf
	(
		message, assertMessageSize,
		"Tacent Assert Failed.\n\n"
		"Expr: [%s]\n"
		"File: [%s]\n"
		"Line: [%d]\n"
		"Msg : [%s]\n\n"
		#ifdef PLATFORM_WIN
		"Press 'Abort' to abort the program completely.\n"
		"Press 'Retry' to start debugging.\n"
		"Press 'Ignore' to try and move past this assert and continue running.\n"
		#endif
		,
		expr, fileName, lineNum, msg ? msg : "None"
	);
	printf(message);

	#ifdef PLATFORM_WIN
	// In windows we bring up a message box.
	int retCode = ::MessageBox
	(
		0, message, "Tacent Assert",
		MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL
	);

	switch (retCode)
	{
		case IDABORT:
			// Exit ungracefully.
			raise(SIGABRT);
			_exit(200);
			break;

		case IDRETRY:
			// Attempt break to debugger.
			__debugbreak();
			return;

		case IDIGNORE:
			return;
	}
	#endif
}
