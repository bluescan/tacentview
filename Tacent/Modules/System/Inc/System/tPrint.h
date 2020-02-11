// tPrint.h
//
// Formatted print functions that improve upon the standard printf family of functions. The functions found here
// support custom type handlers for things like vectors, matrices, and quaternions. They have more robust support for
// different type sizes and can print integral types in a variety of bases. Redirection via a callback as well as
// visibility channels are also supported.
//
// Copyright (c) 2004-2006, 2015, 2017, 2019, 2020 Tristan Grimmer.
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


namespace tSystem
{
	// If more than 64 channels are needed, instead of using a uint64, something like a tBitField<128> would be a
	// drop-in replacement. Initialization of values could use the aggregate construction syntax. Something like:
	// typedef tBitField<128> Channel;
	// const Channel Channel_Core = { 0x0000000000000000ull, 0x0000000000000002ull };
	typedef uint64 tChannel;
	const tChannel tChannel_None							= 0x0000000000000000ull;

	const tChannel tChannel_Default							= 0x0000000000000001ull;
	const tChannel tChannel_Core							= 0x0000000000000002ull;
	const tChannel tChannel_Gameplay						= 0x0000000000000004ull;
	const tChannel tChannel_Physics							= 0x0000000000000008ull;
	const tChannel tChannel_Sound							= 0x0000000000000010ull;
	const tChannel tChannel_Rendering						= 0x0000000000000020ull;
	const tChannel tChannel_AI								= 0x0000000000000040ull;
	const tChannel tChannel_Input							= 0x0000000000000080ull;

	const tChannel tChannel_User0							= 0x0000000000000100ull;
	const tChannel tChannel_User1							= 0x0000000000000200ull;
	const tChannel tChannel_User2							= 0x0000000000000400ull;
	const tChannel tChannel_User3							= 0x0000000000000800ull;
	const tChannel tChannel_User4							= 0x0000000000001000ull;
	const tChannel tChannel_User5							= 0x0000000000002000ull;
	const tChannel tChannel_User6							= 0x0000000000004000ull;
	const tChannel tChannel_User7							= 0x0000000000008000ull;

	const tChannel tChannel_TestResult						= 0x0000000000010000ull;
	const tChannel tChannel_All								= 0xFFFFFFFFFFFFFFFFull;
	const tChannel tChannel_Systems							= tChannel_Default	| tChannel_Core			|
															  tChannel_Gameplay	| tChannel_Physics		|
															  tChannel_Sound	| tChannel_Rendering	|
															  tChannel_AI		| tChannel_Input		;
	const tChannel tChannel_Users							= tChannel_User0	| tChannel_User1		|
															  tChannel_User2	| tChannel_User3		|
															  tChannel_User4	| tChannel_User5		|
															  tChannel_User6	| tChannel_User7		;
	const tChannel tChannel_Tristan							= tChannel_User0;

	// To only see specific channels, simply call this with your machine name and the bitwise or of the channels you
	// wish to see (on your machine). If this is not called you will see the Systems channels. For many programmers
	// the right choice is something like: Channel_Systems | Channel_ProgrammerName. Do not call a function to retrieve
	// the machineName dynamically or you will turn on the supplied channels for everyone. Instead, the intention is
	// there will be multiple tRegister calls. One for each dev that wants specific channels on.
	void tRegister(const char* machineName, tChannel channelsToSee);
	void tRegister(uint32 machineNameHash, tChannel channelsToSee);

	// If you wish top turn on or off channels regardless of computer name, just call this with the channels you want.
	// Any channel not specified will no longer be displayed.
	void tSetChannels(tChannel channelsToSee);

	// After all print formatting, a simple stream output function is called, possibly multiple times, for the Print,
	// tPrintf, and tvPrintf functions. By default those functions output to stdout. By setting the OutputCallback
	// function below, all output normally destined for stdout may be redirected to wherever you like. Calling with
	// nullptr resets to no redirection of stdout output. The 'text' supplied to the callback has 'numChars'
	// non-zero characters. The 'numChars+1'th character of 'text' IS guaranteed to be a '\0' if you want to treat
	// the text as a null-terminated string.
	typedef void RedirectCallback(const char* text, int numChars);
	void tSetStdoutRedirectCallback(RedirectCallback = nullptr);

	// Windows only. Sets supplementary output to include any attached debugger. Defaults to true. Only output
	// normally destined to stdout will also be sent to the debugger. If a StdoutRedirectCallback is set, it will
	// still be called regardless of what you call SetSupplimentaryDebuggerOutput with.
	void tSetSupplimentaryDebuggerOutput(bool enable = true);

	// This is a non-formatting print. Just prints the string you give it to the supplied FileHandle. If the supplied
	// FileHandle is set to 0 then stdout is used. When stdout is the destination this function performs filtering on
	// the characters that are printed. On some platforms there are unprintable stdout characters that this function
	// will skip. If FileHandle is 0 and an OutputCallback is specified the callback is called without any filtering.
	// Returns the actual number of characters printed which, therefore, may be less than the string length. No
	// filtering is done if FileHandle is non-zero. Note that this return value is NOT used by other functions in this
	// header like tPrintf due to the filtering. Not that this function ignores output channels.
	int tPrint(const char* string, tFileHandle);

	// Same as above but will only print to stdout if the channel is active. Essentially it calls the above print to
	// output to stdout if the channel is active.
	int tPrint(const char* string, tChannel channels = tChannel_Default);

	// Note also that the default ANSI %f precision is 6. Tacent uses 4 by default, but you can globally change it
	// with this.
	void tSetDefaultPrecision(int precision);
	int tGetDefaultPrecision();
};


// The following print functions return the number of characters written to either stdout or to the supplied buffer.
// Format specification strings take the form below, with [] indicating an optional field.
//
// %[flags][width][.precision][:typeSizeElements][!typeSizeBytes][|typeSizeBits]type
//
// The format string is a superset of the ANSI standard for printf. It adds more types (vectors and matrices),
// consistent typesize fields and support for extra flags, including an alternative presentation flag. See
// http://www.cplusplus.com/reference/cstdio/printf/ for the standard types supported.
//
// [flags]
// Must be one or more of: - + ' ' 0 # _ ~ (where ' ' is a space character)
//
//		Standard:	hyphen (-) means left justify. Plus (+) means show sign for numeric types. # means force decimal
//					point for e, E, f, g, and G. It also means prefix with 0, 0x, or 0X for o, x, and X.
//		Enhanced:	Underscore (_) or tilde (~) chooses a more readable decorative or alternative printing method.
//					Integral Types:
//						For built-in integral types, an _ inserts an underscore every 4 characters starting from the
//						right. Handy for making binary values readable. For Tacent integer types like tint128 and
//						tuint256, the spacing is every 8 characters.
//						For any integral type, a ~ will place a comma every 3 digits starting from thr right. Makes
//						big integers readable. eg. 23,456,789
//					Matrix Types:
//						A _ will print the 4x4 (or 2x2) matrix in 4 (or 2) separate rows surrounded by square []
//						brackets. Uses a default width of 9 and precision of 4, resulting in 1234.5678 for each
//						element. Default expected matrix size is :16 (a 4x4 matrix). If you pass in a 2x2, you must
//						specify the typesize.
//					Vector Types:
//						A _ causes just the components (space separated) to print. No decoration. By convention (a, b)
//						is shorthand for the column vector:	[	a
//																b	]
//						Default expected vector size is :3 (3 elements). If you pass in a 2 or 4 component vector you
//						must specify the size.
//
// [width]
// A number representing the minimum num chars printed, or a * to indicate read width from the vararg list.
//
// [precision]
// A number representing decimal accuracy when printing floats, or a * to indicate read precision from the vararg list.
//
// [typeSize]
// A number after a colon (:) or a bang (!) or a pipe (|). This is not ANSI (ANSI ones are the odd h, l, ll, I32, I64).
// The colon means to interpret the number following as how many 32bit elements there are in the type. For example, :2,
// :3, :4 may be used with the vector 'v' type to indicate how many components are present. The bang (!) means
// interpret the number as a byte (octet I suppose, but really, what processor now days has a 7-bit byte? 32-bit chars
// are much more common). A value after a pipe symbol means the number is interpreted as a number of bits. For example,
// |32 or |64 may be used with integer types d, i, o, u, x, X, and b to specify the bit-size of the integer to print.
//
// [type]
// A single letter chosen from the types below:
//		Integer:	b o d i u x X	For these types 'i' is same as 'd'. 'o' is octal. 'b' is binary. Handles Tacent
//									integer types tint128, tint256, as well as all the built-in integral types.
//		Pointer:	p				ISO C. Similar to %#010X (32bit systems) and %#018X (on 64bit systems) with the
//									following two exceptions:
//										a)	The prefix 0x (lower case x) is used with capital ABCDEF (normally these
//											are forced to match).
//										b)	The prefix is always used, even if the value is 0 (which is not the case
//											for # in general).
//		Float:		f g				Handles floats and doubles. g is not implemented and currently behaves as f.
//		Vector:		v				Use with :typesize for 2, 3, and 4 dimensional vectors. Default is 3. Must call
//									pod(vec) or vec.Pod() for tVectors, or use tVecNb POD type. All other spec
//									fields are per vector component. Not the whole thing.
//		Quaternion:	q				Quaternions. Typesize is ignored. Must call pod for tQuaternions. Normal format is
//									(w, x, y, z). Alternate _ format is (w, (x, y, z)).
//		Matrix:		m				For use with tMatrix4 and tMatrix2 POD types. Matrix is printed on one line using
//									() brackets. Use decorative _ flag for 4 (or 2) rows. All other spec fields are per
//									vector component. The typesize is used to determine 2x2 or 4x4 (default).
//		Character:	c				A single character.
//		String:		s				You must call pod(string) or string.Pod() for tStrings, or use char*.
//					t				Windows only. Allows passing of non-POD tString directly. Warning: You cannot pass
//									in an tStringItem! You must either cast to an tString or call pod() and use %s.
//		Percent:	%				Displays percent sign.
//
// The functions return the number of characters printed. They do NOT care about channels in that they always report
// what would have been printed if the channel was visible. Channels are an output filter only, not something that
// changes behaviour depending on what machine you are on.
//
// Examples:
// uint32 a = 0x1234ABCD;
// tPrintf("%08X", a);
//
// uint64 b = 0x1234ABCD5678ABCDLL;
// tPrintf("%016|64X or %016!8X or %016:2X", b, b, b);
//
// tuint256 c = 42;
// tPrintf("%064|256X or %064!32X or %064:8X", c, c);


int tPrintf(const char* format, ...);						// Prints to generic channel.
int tvPrintf(const char* format, va_list);					// Prints to generic channel.
int tPrintf(tSystem::tChannel channels, const char* format, ...);
int tvPrintf(tSystem::tChannel channels, const char* format, va_list);


// These are a few shortcut functions. Notice that in release, these become inline empty functions. By these, I only
// mean the ones below that output to the screen or stdout.
int tPrintCore(const char* format, ...);
int tPrintGameplay(const char* format, ...);
int tPrintPhysics(const char* format, ...);
int tPrintSound(const char* format, ...);
int tPrintRendering(const char* format, ...);
int tPrintAI(const char* format, ...);
int tPrintInput(const char* format, ...);
int tPrintT(const char* format, ...);


// In some cases, possibly before reserving buffer space, you need to know how many characters would be used in an
// actual printf call. The next two functions work that out. They are not affected by what channels are turned on.
int tcPrintf(const char* format, ...);
int tvcPrintf(const char* format, va_list);


// Formatted print functions that print into a character buffer. The dest buffer must be big enough. Use tcPrintf or
// tvcPrintf to find out how big. The tString versions will do the size calc first and return a ref to the same string
// passed in. Use Length to find out number characters printed.
int tvsPrintf(char* dest, const char* format, va_list);
int tsPrintf(char* dest, const char* format, ...);
tString& tvsPrintf(tString& dest, const char* format, va_list);
tString& tsPrintf(tString& dest, const char* format, ...);
inline tString tvsPrintf(const char* format, va_list args)																{ tString dest; return tvsPrintf(dest, format, args); }
inline tString tsPrintf(const char* format, ...)																		{ va_list marker; va_start(marker, format); return tvsPrintf(format, marker); }


// These overloads are 'safe' in that they guarantee no overrun of the dest buffer. You enter the full number of bytes
// in the dest buffer. The dest buffer is guaranteed to be null terminated afterwards, and the number of characters
// (not including the terminator) is returned. The dest may contain a truncated version of the formatted string if the
// dest buffer is not big enough. Returns the number of non-null characters inserted into the buffer. destSize must
// be greater than 0, but if that's all you give it, there's only room for the terminator. Dest sizes <= 1 always
// return zero.
int tvsPrintf(char* dest, int destSize, const char* format, va_list);
int tsPrintf(char* dest, int destSize, const char* format, ...);


// Here is the file handle print
int tfPrintf(tFileHandle dest, const char* format, ...);
int tvfPrintf(tFileHandle dest, const char* format, va_list);


// These variants print a timestamp (ttf = tacent timestamp file) before any content is printed.
int ttfPrintf(tFileHandle dest, const char* format, ...);
int ttvfPrintf(tFileHandle dest, const char* format, va_list);

// tFlush may be called with any FileHandle such as stdout or stderr as well as other file handles opened with
// tSystem::tOpenFile.
void tFlush(tFileHandle);


// The tdPrintf variations print to both the destination string or file, as well as printing to whatever channels are
// specified. The 'd' is for dual. Useful for things like log files and unit tests that also need to print to stdout.
// The dest file or string is always printed to. The channels only affect what is output to stdout and the return
// values will match what the plain print-to-string (or file) would have returned.
int tdsPrintf(char* dest, const char* format, ...);
int tdsPrintf(tSystem::tChannel channels, char* dest, const char* format, ...);
tString& tdsPrintf(tString& dest, const char* format, ...);
tString& tdsPrintf(tSystem::tChannel channels, tString& dest, const char* format, ...);
int tdsPrintf(char* dest, int destSize, const char* format, ...);
int tdsPrintf(tSystem::tChannel channels, char* dest, int destSize, const char* format, ...);
int tdfPrintf(tFileHandle dest, const char* format, ...);
int tdfPrintf(tSystem::tChannel channels, tFileHandle dest, const char* format, ...);


// Implementation below this line.


#if !defined(CONFIG_PROFILE) && !defined(CONFIG_SHIP)
inline int tPrintf(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(f, l);
	va_end(l);			return n;
}


inline int tPrintf(uint32 c, const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(c, f, l);
	va_end(l);			return n;
}


inline int tPrintCore(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Core, f, l);
	va_end(l);			return n;
}


inline int tPrintGameplay(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Gameplay, f, l);
	va_end(l);			return n;
}


inline int tPrintPhysics(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Physics, f, l);
	va_end(l);			return n;
}


inline int tPrintSound(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Sound, f, l);
	va_end(l);			return n;
}


inline int tPrintRendering(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Rendering, f, l);
	va_end(l);			return n;
}


inline int tPrintAI(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_AI, f, l);
	va_end(l);			return n;
}


inline int tPrintInput(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Input, f, l);
	va_end(l);			return n;
}


inline int tPrintT(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Tristan, f, l);
	va_end(l);			return n;
}


#else
inline int tPrintf(const char* format, ...)																				{ return 0; }
inline int tvPrintf(const char* format, va_list)																		{ return 0; }
inline int tPrintf(uint32 channels, const char* format, ...)															{ return 0; }
inline int tvPrintf(uint32 channels, const char* format, va_list)														{ return 0; }
inline int tPrintCore(const char* format, ...)																			{ return 0; }
inline int tPrintGameplay(const char* format, ...)																		{ return 0; }
inline int tPrintPhysics(const char* format, ...)																		{ return 0; }
inline int tPrintSound(const char* format, ...)																			{ return 0; }
inline int tPrintRendering(const char* format, ...)																		{ return 0; }
inline int tPrintAI(const char* format, ...)																			{ return 0; }
inline int tPrintInput(const char* format, ...)																			{ return 0; }
inline int tPrintT(const char* format, ...)																				{ return 0; }
#endif


inline int tdsPrintf(char* dest, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(format, marker); 
	return tvsPrintf(dest, format, marker);
}


inline int tdsPrintf(tSystem::tChannel channels, char* dest, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(channels, format, marker); 
	return tvsPrintf(dest, format, marker);
}


inline tString& tdsPrintf(tString& dest, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(format, marker); 
	return tvsPrintf(dest, format, marker);
}


inline tString& tdsPrintf(tSystem::tChannel channels, tString& dest, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(channels, format, marker); 
	return tvsPrintf(dest, format, marker);
}


inline int tdsPrintf(char* dest, int destSize, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(format, marker); 
	return tvsPrintf(dest, destSize, format, marker);
}


inline int tdsPrintf(tSystem::tChannel channels, char* dest, int destSize, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(channels, format, marker); 
	return tvsPrintf(dest, destSize, format, marker);
}


inline int tdfPrintf(tFileHandle dest, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(format, marker); 
	return tvfPrintf(dest, format, marker);
}


inline int tdfPrintf(tSystem::tChannel channels, tFileHandle dest, const char* format, ...)
{
	va_list marker;
	va_start(marker, format);
	tvPrintf(channels, format, marker); 
	return tvfPrintf(dest, format, marker);
}
