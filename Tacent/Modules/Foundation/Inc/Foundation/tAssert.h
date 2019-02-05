// tAssert.h
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

#pragma once
#include <Foundation/tPlatform.h>


// Asserts to be used to detect coding errors only. Not for input errors or validation. Asserts are serious and get
// compiled out only in profile and ship configurations.
void tAssertPrintBreak(const char* expr, const char* fileName, int lineNum, const char* message);
#if defined(CONFIG_PROFILE) || defined(CONFIG_SHIP)
#define tAssert(expr) ((void)(expr));
#define tAssertMsg(expr, msg) ((void)(expr));
#else
#define tAssert(expr) if (!(expr)) tAssertPrintBreak(#expr, __FILE__, __LINE__, nullptr);
#define tAssertMsg(expr, msg) if (!(expr)) tAssertPrintBreak(#expr, __FILE__, __LINE__, msg);
#endif
#define tStaticAssert(expr) static_assert(expr, #expr)
#define tStaticAssertMsg(expr, msg) static_assert(expr, #expr##" \""##msg##"\"")


void tAbort();


// These next defines are handy for leaving notes in the code that show in the compiler output. You can also
// double-click them to be taken to the precise source code line in the IDE.
//
// In your source code enter a line like one of the following two:
// Todo(Do not forget to fix this)
// Note("Here's a note")
#if defined(PLATFORM_WIN)
#define tMsgHelper2(x) #x
#define tMsgHelper1(x) tMsgHelper2(x)
#define tMsgTodo __FILE__ "("tMsgHelper1(__LINE__)") : ToDo: "
#define tMsgNote __FILE__ "("tMsgHelper1(__LINE__)") : Note: "
#define Todo(x) __pragma(message(tMsgTodo #x))
#define Note(x) __pragma(message(tMsgNote #x))
#else
#define Todo(x) _Pragma(#x)
#define Note(x) _Pragma(#x)
#endif
