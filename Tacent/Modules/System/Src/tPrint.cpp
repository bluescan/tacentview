// tPrint.cpp
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

#ifdef PLATFORM_WIN
#include <windows.h>
#endif
#include <Foundation/tStandard.h>
#include <Foundation/tArray.h>
#include <Math/tLinearAlgebra.h>
#include <Math/tHash.h>
#include "System/tMachine.h"
#include "System/tTime.h"
#include "System/tFile.h"
#include "System/tPrint.h"
using namespace tMath;


namespace tSystem
{
	// Global settings for all print functionality.
	static int DefaultPrecision = 4;

	// This class receives the final properly formatted characters. As it receives them it counts how many were
	// received. If you construct with either an external character buffer or external string, it populates them.
	class Receiver
	{
	public:
		// This constructor creates a receiver that only counts characters received.
		Receiver()																										: Buffer(nullptr), ReceiveLimit(-1), String(nullptr), NumReceived(0) { }

		// Populates buffer as chars are received. Buffer is owned externally and its lifespan must outlast Receiver.
		Receiver(tArray<char>* buffer)																					: Buffer(buffer), ReceiveLimit(-1), String(nullptr), NumReceived(0) { }

		// Populates string as chars are received. Buffer is owned externally and its lifespan must outlast Receiver.
		// The caller must ensure enough room in string for all the receives that will be called.
		Receiver(char* string)																							: Buffer(nullptr), ReceiveLimit(-1), String(string), NumReceived(0) { }

		// Populates string as chars are received. Buffer is owned externally and its lifespan must outlast Receiver.
		// The caller must ensure enough room in string for all the receives that will be called. After receiveLimit
		// characters are received, string will no longer be written to.
		Receiver(char* string, int receiveLimit)																		: Buffer(nullptr), ReceiveLimit(receiveLimit), String(string), NumReceived(0) { }

		void Receive(char chr);
		void Receive(const char* str);						// Assumes null termination.
		void Receive(const char* str, int numChars);		// No null termination necessary.
		void Receive(const tArray<char>&);
		int GetNumReceived() const																						{ return NumReceived; }

	private:
		// We could have used a tString here but it wouldn't have been very efficient since appending a single character
		// would cause a memcpy.
		tArray<char>* Buffer;

		// This string is not owned by this class. It is supplied by the caller of one of the string-printf style
		// functions. A receive limit of -1 means no limit.
		int ReceiveLimit;
		char* String;
		int NumReceived;
	};

	// This is the workhorse. It processes the format string and deposits the resulting formatted text in the receiver.
	void Process(Receiver&, const char* format, va_list);

	// Channel system. This is lazy initialized (using the name hash as the state) without any need for shutdown.
	uint32 ComputerNameHash																								= 0;
	tChannel OutputChannels																								= tChannel_Systems;
	bool SupplimentaryDebuggerOutput																					= true;
	RedirectCallback* StdoutRedirectCallback																			= nullptr;

	// A format specification consists of the information stored in the expression:
	// %[flags] [width] [.precision] [:typesize][|typesize]type
	// except for the type character.
	enum Flag
	{
		Flag_ForcePosOrNegSign					= 1 << 0,
		Flag_SpaceForPosSign					= 1 << 1,
		Flag_LeadingZeros						= 1 << 2,
		Flag_LeftJustify						= 1 << 3,
		Flag_DecorativeFormatting				= 1 << 4,
		Flag_DecorativeFormattingAlt			= 1 << 5,
		Flag_BasePrefix							= 1 << 6
	};

	struct FormatSpec
	{
		FormatSpec()																									: Flags(0), Width(0), Precision(-1), TypeSizeBytes(0) { }
		FormatSpec(const FormatSpec& src)																				: Flags(src.Flags), Width(src.Width), Precision(src.Precision), TypeSizeBytes(src.TypeSizeBytes) { }
		FormatSpec& operator=(const FormatSpec& src)																	{ Flags = src.Flags; Width = src.Width; Precision = src.Precision; TypeSizeBytes = src.TypeSizeBytes; return *this; }

		uint32 Flags;
		int Width;
		int Precision;
		int TypeSizeBytes;				// Defaults to 0, not set.
	};

	// Type handler stuff below.
	typedef void (*HandlerFn)(Receiver& out, const FormatSpec&, void* data);
	struct HandlerInfo
	{
		char Type;						// The type specifier. eg. 'f' or 'd' or 'X'.
		int DefaultByteSize;
		HandlerFn Handler;
	};
	extern const int NumHandlers;
	extern HandlerInfo HandlerInfos[];
	extern int HandlerJumpTable[256];
	HandlerInfo* FindHandler(char format);
	bool IsValidFormatSpecifierCharacter(char);

	// Does the heavy-lifting of converting (built-in) integer types to strings. This function can handle both 32 and
	// 64 bit integers (signed and unsigned). To print Tacent integral types or bit-fields of 128, 256, or 512 bits
	// please see the function HandlerHelper_IntegerTacent.
	void HandlerHelper_IntegerNative
	(
		tArray<char>&, const FormatSpec&, void* data, bool treatAsUnsigned,
		int bitSize, bool upperCase, int base, bool forcePrefixLowerCase = false
	);

	void HandlerHelper_IntegerTacent
	(
		tArray<char>&, const FormatSpec&, void* data, bool treatAsUnsigned,
		int bitSize, bool upperCase, int base, bool forcePrefixLowerCase = false
	);

	enum class PrologHelperFloat
	{
		None,
		NeedsPlus,
		NeedsNeg,
		NeedsSpace,
		NoZeros,
	};
	PrologHelperFloat HandlerHelper_FloatNormal
	(
		tArray<char>&, const FormatSpec&, double value, bool treatPrecisionAsSigDigits = false
	);
	bool HandlerHelper_HandleSpecialFloatTypes(tArray<char>&, double value);
	int  HandlerHelper_FloatComputeExponent(double value);
	void HandlerHelper_Vector(Receiver&, const FormatSpec&, const float* components, int numComponents);
	void HandlerHelper_JustificationProlog(Receiver&, int itemLength, const FormatSpec&);
	void HandlerHelper_JustificationEpilog(Receiver&, int itemLength, const FormatSpec&);

	// Here are all the handler functions. One per type.
	void Handler_b(Receiver& out, const FormatSpec&, void* data);
	void Handler_o(Receiver& out, const FormatSpec&, void* data);
	void Handler_d(Receiver& out, const FormatSpec&, void* data);
	void Handler_i(Receiver& out, const FormatSpec&, void* data);
	void Handler_u(Receiver& out, const FormatSpec&, void* data);
	void Handler_x(Receiver& out, const FormatSpec&, void* data);
	void Handler_X(Receiver& out, const FormatSpec&, void* data);
	void Handler_p(Receiver& out, const FormatSpec&, void* data);

	void Handler_e(Receiver& out, const FormatSpec&, void* data);
	void Handler_f(Receiver& out, const FormatSpec&, void* data);
	void Handler_g(Receiver& out, const FormatSpec&, void* data);
	void Handler_v(Receiver& out, const FormatSpec&, void* data);
	void Handler_q(Receiver& out, const FormatSpec&, void* data);

	void Handler_m(Receiver& out, const FormatSpec&, void* data);
	void Handler_c(Receiver& out, const FormatSpec&, void* data);
	void Handler_s(Receiver& out, const FormatSpec&, void* data);
	void Handler_z(Receiver& out, const FormatSpec&, void* data);
}


void tSystem::tRegister(uint32 machineNameHash, tSystem::tChannel channelsToSee)
{
	if (!ComputerNameHash)
		ComputerNameHash = tMath::tHashStringFast32( tSystem::tGetCompName() );

	if (machineNameHash == ComputerNameHash)
		tSetChannels(channelsToSee);
}


void tSystem::tRegister(const char* machineName, tSystem::tChannel channelsToSee)
{
	if (!machineName)
		return;

	tRegister(tMath::tHashStringFast32(machineName), channelsToSee);
}


void tSystem::tSetChannels(tChannel channelsToSee)
{
	OutputChannels = channelsToSee;
}


void tSystem::tSetStdoutRedirectCallback(RedirectCallback cb)
{
	StdoutRedirectCallback = cb;
}


void tSystem::tSetSupplimentaryDebuggerOutput(bool enable)
{
	SupplimentaryDebuggerOutput = enable;
}


int tSystem::tPrint(const char* text, tSystem::tChannel channels)
{
	if (!(channels & OutputChannels))
		return 0;

	return tPrint(text, tFileHandle(0));
}


int tSystem::tPrint(const char* text, tFileHandle fileHandle)
{
	int numPrinted = 0;
	if (!text || (*text == '\0'))
		return numPrinted;

	// Print supplimentary output unfiltered.
	#ifdef PLATFORM_WIN
	if (!fileHandle && SupplimentaryDebuggerOutput && IsDebuggerPresent())
		OutputDebugStringA(text);
	#endif

	// If we have an OutputCallback and the output destination is stdout we redirect to the output callback and we're done.
	if (!fileHandle && StdoutRedirectCallback)
	{
		int numChars = tStd::tStrlen(text);
		StdoutRedirectCallback(text, numChars);
		return numChars;
	}

	#ifdef PLATFORM_WIN
	// Skip some specific undesirable characters.
	const char* startValid = text;
	while (*startValid)
	{
		const char* endValid = startValid;

		while ((*endValid) && (*endValid != '\r'))
			endValid++;

		if ((endValid - startValid) > 0)
		{
			if (fileHandle)
				tSystem::tWriteFile(fileHandle, startValid, int(endValid - startValid));
			else
				tSystem::tWriteFile(stdout, startValid, int(endValid - startValid));
		}

		if (*endValid != '\r')
			startValid = endValid;
		else
			startValid = endValid + 1;
	}

	tFlush(stdout);
	numPrinted = int(startValid - text);

	#else
	int len = auStrlen(text);
	if (fileHandle)
		tSystem::tWriteFile(text, 1, len, fileHandle);
	else
		tSystem::tWriteFile(text, 1, len, stdout);

	fflush(stdout);
	numPrinted = len;
	#endif

	return numPrinted;
}


void tSystem::tSetDefaultPrecision(int precision)
{
	DefaultPrecision = precision;
}


int tSystem::tGetDefaultPrecision()
{
	return DefaultPrecision;
}


void tSystem::Receiver::Receive(char c)
{
	// Are we full?
	if (String && (ReceiveLimit != -1) && (NumReceived >= ReceiveLimit))
		return;

	if (Buffer)
		Buffer->Append(c);

	if (String)
	{
		*String = c;
		String++;
	}

	NumReceived++;
}


void tSystem::Receiver::Receive(const char* str)
{
	if (!str)
		return;

	int len = tStd::tStrlen(str);

	// How much room is avail? May need to reduce len.
	if (String && (ReceiveLimit != -1))
	{
		// Are we full?
		if (NumReceived >= ReceiveLimit)
			return;

		int remaining = ReceiveLimit - NumReceived;
		if (len > remaining)
			len = remaining;
	}

	if (!len)
		return;

	if (Buffer)
		Buffer->Append(str, len);

	if (String)
	{
		tStd::tMemcpy(String, str, len);
		String += len;
	}

	NumReceived += len;
}


void tSystem::Receiver::Receive(const char* str, int numChars)
{
	if (!numChars || !str)
		return;

	// How much room is avail? May need to reduce len.
	if (String && (ReceiveLimit != -1))
	{
		// Are we full?
		if (NumReceived >= ReceiveLimit)
			return;

		int remaining = ReceiveLimit - NumReceived;
		if (numChars > remaining)
			numChars = remaining;
	}

	if (Buffer)
		Buffer->Append(str, numChars);

	if (String)
	{
		tStd::tMemcpy(String, str, numChars);
		String += numChars;
	}

	NumReceived += numChars;
}

		
void tSystem::Receiver::Receive(const tArray<char>& buf)
{
	int len = buf.GetNumElements();
	Receive(buf.GetElements(), len);
}


// Don't forget to update the jump table if you add a new handler to this table. Also note that the
// default size may be overridden by the format spec. For example, %d can be used for tint256 with the
// string "%:8X", "%!32X", or "%|256d".
tSystem::HandlerInfo tSystem::HandlerInfos[] =
{
	//	Type Char		Default Size (bytes)		Handler Function				Fast Jump Index
	{ 'b',				4,							tSystem::Handler_b },			// 0
	{ 'o',				4,							tSystem::Handler_o },			// 1
	{ 'd',				4,							tSystem::Handler_d },			// 2
	{ 'i',				4,							tSystem::Handler_i },			// 3
	{ 'u',				4,							tSystem::Handler_u },			// 4
	{ 'x',				4,							tSystem::Handler_x },			// 5
	{ 'X',				4,							tSystem::Handler_X },			// 6
	{ 'p',				sizeof(void*),				tSystem::Handler_p },			// 7
	{ 'e',				8,							tSystem::Handler_e },			// 8
	{ 'f',				8,							tSystem::Handler_f },			// 9
	{ 'g',				8,							tSystem::Handler_g },			// 10
	{ 'v',				sizeof(tVec3),				tSystem::Handler_v },			// 11
	{ 'q',				sizeof(tQuat),				tSystem::Handler_q },			// 12
	{ 'm',				sizeof(tMat4),				tSystem::Handler_m },			// 13
	{ 'c',				4,							tSystem::Handler_c },			// 14
	{ 's',				sizeof(char*),				tSystem::Handler_s },			// 15
	{ '%',				0,							tSystem::Handler_z },			// 16
};

// Filling this in correctly will speed things up. However, not filling it in or filling it in incorrectly will still
// work. Fill it in by looking at the type character in the handler info table. Find the letter entry in the jump table,
// and populate it with the fast jump index.
const int tSystem::NumHandlers = sizeof(tSystem::HandlerInfos) / sizeof(*tSystem::HandlerInfos);
int tSystem::HandlerJumpTable[256] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [0, 15]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [16, 31]

	//                   %
	-1, -1, -1, -1, -1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [32, 47]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [48, 63]

	//   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [64, 79]

	//   Q   R   S   T   U   V   W   X   Y   Z
	-1, -1, -1, -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1,		// [80, 95]

	//   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
	-1, -1,  0, 14,  2,  8,  9, 10, -1,  3, -1, -1, -1, 13, -1,  1,		// [96, 111]

	//   q   r   s   t   u   v   w   x   y   z
	 7, 12, -1, 15, -1,  4, 11, -1,  5, -1, -1, -1, -1, -1, -1, -1,		// [112, 127]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [128, 143]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [144, 159]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [160, 175]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [176, 191]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [192, 207]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [208, 223]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// [224, 239]
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1		// [240, 255]
};


int tvPrintf(const char* format, va_list argList)
{
	if (!format)
		return 0;

	tArray<char> buffer;
	tSystem::Receiver receiver(&buffer);

	Process(receiver, format, argList);
	tSystem::tPrint(buffer.GetElements());
	return receiver.GetNumReceived() - 1;
}


int tvPrintf(tSystem::tChannel channels, const char* format, va_list argList)
{
	if (!format)
		return 0;

	tArray<char> buffer;
	tSystem::Receiver receiver(&buffer);

	Process(receiver, format, argList);
	tSystem::tPrint(buffer.GetElements(), channels);
	return receiver.GetNumReceived() - 1;
}


int tvsPrintf(char* dest, const char* format, va_list argList)
{
	if (!dest || !format)
		return 0;

	tSystem::Receiver receiver(dest);
	Process(receiver, format, argList);
	return receiver.GetNumReceived() - 1;
}


int tsPrintf(char* dest, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	int count = tvsPrintf(dest, format, argList);
	va_end(argList);
	return count;
}


tString& tvsPrintf(tString& dest, const char* format, va_list marker)
{
	int reqChars = tvcPrintf(format, marker);
	dest.Reserve(reqChars);
	tvsPrintf(dest.Text(), format, marker);
	return dest;
}


tString& tsPrintf(tString& dest, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	tvsPrintf(dest, format, argList);
	va_end(argList);
	return dest;
}


int tvsPrintf(char* dest, int destSize, const char* format, va_list argList)
{
	if (!dest || !format || (destSize <= 0))
		return 0;

	if (destSize == 1)
	{
		dest[0] = '\0';
		return 0;
	}

	tSystem::Receiver receiver(dest, destSize);
	Process(receiver, format, argList);

	// Possibly write a missing terminating 0 if we filled up.
	int rec = receiver.GetNumReceived();
	int len = rec - 1;
	if (destSize == rec)
		dest[len] = '\0';
	return len;
}


int tsPrintf(char* dest, int destSize, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	int count = tvsPrintf(dest, destSize, format, argList);
	va_end(argList);
	return count;
}


int tcPrintf(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	int count = tvcPrintf(format, argList);
	va_end(argList);
	return count;
}


int tvcPrintf(const char* format, va_list argList)
{
	if (!format)
		return 0;

	tSystem::Receiver receiver;
	Process(receiver, format, argList);
	return receiver.GetNumReceived() - 1;
}


int tfPrintf(tFileHandle dest, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	int count = tvfPrintf(dest, format, argList);
	va_end(argList);
	return count;
}


int tvfPrintf(tFileHandle dest, const char* format, va_list argList)
{
	if (!format || !dest)
		return 0;

	tArray<char> buffer;
	tSystem::Receiver receiver(&buffer);

	Process(receiver, format, argList);
	tSystem::tPrint(buffer.GetElements(), dest);
	return receiver.GetNumReceived() - 1;
}


int ttfPrintf(tFileHandle dest, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	int count = ttvfPrintf(dest, format, argList);
	va_end(argList);
	return count;
}


int ttvfPrintf(tFileHandle dest, const char* format, va_list argList)
{
	if (!format || !dest)
		return 0;

	tString stamp = tSystem::tConvertTimeToString(tSystem::tGetTimeLocal(), tSystem::tTimeFormat::Short) + " ";
	int count = tSystem::tPrint(stamp.Pod(), dest);

	tArray<char> buffer;
	tSystem::Receiver receiver(&buffer);

	Process(receiver, format, argList);
	tSystem::tPrint(buffer.GetElements(), dest);
	return count + receiver.GetNumReceived() - 1;
}


void tFlush(tFileHandle handle)
{
	fflush(handle);
}


tSystem::HandlerInfo* tSystem::FindHandler(char type)
{
	if (type == '\0')
		return nullptr;

	// First we try to use the jump table.
	int index = HandlerJumpTable[int(type)];
	if ((index != -1) && (index < NumHandlers) && (index >= 0))
	{
		HandlerInfo* h = &HandlerInfos[index];
		tAssert(h);
		if (h->Type == type)
			return h;
	}

	// No go? Do a full search.
	for (int i = 0; i < NumHandlers; i++)
	{
		HandlerInfo* h = &HandlerInfos[i];
		tAssert(h);
		if (h->Type == type)
			return h;
	}

	return nullptr;
}


bool tSystem::IsValidFormatSpecifierCharacter(char c)
{
	// Tests for valid character after a %. First we check optional flag characters.
	if ((c == '-') || (c == '+') || (c == ' ') || (c == '0') || (c == '#') || (c == '_') || (c == '~') || (c == '#'))
		return true;

	// Next test for width and precision.
	if (tStd::tIsdigit(c) || (c == '.') || (c == '*'))
		return true;

	// Next check for typesize. We've already checked for the digit part.
	if ((c == ':') || (c == '!') || (c == '|'))
		return true;

	// Finally check for type.
	for (int i = 0; i < NumHandlers; i++)
		if (c == HandlerInfos[i].Type)
			return true;

	return false;
}


void tSystem::Process(Receiver& receiver, const char* format, va_list argList)
{
	while (format[0] != '\0')
	{
		if (format[0] != '%')
		{
			// Nothing special. Just receive the character.
			receiver.Receive(format[0]);
			format++;
		}
		else if (!IsValidFormatSpecifierCharacter(format[1]))
		{
			// Invalid character after the % so receive that character. This allows stuff like %% to work.
			receiver.Receive(format[1]);
			format += 2;
		}
		else
		{
			// Time to process a format specification. Again, it looks like:
			// %[flags][width][.precision][:typesize][!typesize][|typesize]type
			format++;
			FormatSpec spec;

			while ((format[0] == '-') || (format[0] == '+') || (format[0] == ' ') || (format[0] == '0') || (format[0] == '_') || (format[0] == '~') || (format[0] == '#'))
			{
				switch (format[0])
				{
					case '-':	spec.Flags |= Flag_LeftJustify;				break;
					case '+':	spec.Flags |= Flag_ForcePosOrNegSign;		break;
					case ' ':	spec.Flags |= Flag_SpaceForPosSign;			break;
					case '0':	spec.Flags |= Flag_LeadingZeros;			break;
					case '_':	spec.Flags |= Flag_DecorativeFormatting;	break;
					case '~':	spec.Flags |= Flag_DecorativeFormattingAlt;	break;
					case '#':	spec.Flags |= Flag_BasePrefix;				break;
				}
				format++;
			}

			// From docs: If 0 and – appear, the 0 is ignored.
			if ((spec.Flags & Flag_LeadingZeros) && (spec.Flags & Flag_LeftJustify))
				spec.Flags &= ~Flag_LeadingZeros;

			// Read optional width specification. The '*' means get the value from tha argument list.
			if (format[0] != '*')
			{
				while (tStd::tIsdigit(format[0]))
				{
					spec.Width = spec.Width * 10 + ( format[0] - '0' ) ;
					format++;
				}
			}
			else
			{
				spec.Width = va_arg(argList, int);
				format++;
			}

			// Read optional precision specification. The '*' means get the value from tha argument list.
			if (format[0] == '.')
			{
				spec.Precision = 0;
				format++;

				if (format[0] != '*')
				{
					while (tStd::tIsdigit(format[0]))
					{
						spec.Precision = spec.Precision * 10 + ( format[0] - '0' ) ;
						format++;
					}
				}
				else
				{
					spec.Precision = va_arg(argList, int);
					format++;
				}
			}

			// Read optional type size specification. Tacent-specific and way better than posix or ansi.
			if ((format[0] == ':') || (format[0] == '!') || (format[0] == '|'))
			{
				char typeUnit = format[0];
				spec.TypeSizeBytes = 0;
				format++;
				while (tStd::tIsdigit(format[0]))
				{
					spec.TypeSizeBytes = spec.TypeSizeBytes * 10 + ( format[0] - '0' ) ;
					format++;
				}

				switch (typeUnit)
				{
					case ':':	spec.TypeSizeBytes *= 4;	break;
					case '|':	spec.TypeSizeBytes /= 8;	break;
				}
			}

			// Format now points to the type character.
			HandlerInfo* handler = FindHandler(*format);
			tAssert(handler);
			if (!spec.TypeSizeBytes)
				spec.TypeSizeBytes = handler->DefaultByteSize;

			// Note the type promotions caused by the variadic calling convention,
			// float -> double. char, short, int -> int.
			//
			// GNU:
			// Normal (int, float, enum, etc) types are placed in registers... so you MUST use va_arg to access.
			// Structs and classes must be POD types. The address gets placed in a register.
			// You can access the pointer by casting the va_list. Not portable though.
			//
			// WIN/MSVC:
			// Everything goes on the stack. Casting of the va_list always to gets a pointer to the object.
			//
			// I think for now we'll do the less efficient, but more portable, va_arg method in all cases.
			// It isn't quite as fast cuz it always creates a byte for byte copy. The variables below are holders
			// of the va_arg retrieved data. The holders below must be POD types, specifically no constructor or
			// destructor because on windows we want to ensure that after va_arg does it's byte-wise copy, that
			// the copy (that was not properly constructed) is not destructed. By having knowledge of how the
			// specific data type works, we can, and have, safely made tPrintf support types like tString directly
			// on windows.
			struct val4type  { uint8 buf[4]; } val4;
			struct val8type  { uint8 buf[8]; } val8;		// 64 bit types.
			struct val12type { uint8 buf[12]; } val12;		// 96 bit types (like tVector3).
			struct val16type { uint8 buf[16]; } val16;		// 128 bit types (like tuint128 and tVector4).
			struct val32type { uint8 buf[32]; } val32;		// 256 bit types (like tuint256).
			struct val64type { uint8 buf[64]; } val64;		// 512 bit types (like tMatrix4, tuint512, and tbit512).

			void* pval = nullptr;
			switch (spec.TypeSizeBytes)
			{
				case 0:												pval = nullptr;		break;
				case 4:		val4 = va_arg(argList, val4type);		pval = &val4;		break;
				case 8:		val8 = va_arg(argList, val8type);		pval = &val8;		break;
				case 12:	val12 = va_arg(argList, val12type);		pval = &val12;		break;
				case 16:	val16 = va_arg(argList, val16type);		pval = &val16;		break;
				case 32:	val32 = va_arg(argList, val32type);		pval = &val32;		break;
				case 64:	val64 = va_arg(argList, val64type);		pval = &val64;		break;
				default:	tAssert(!"Cannot deal with this size print vararg.");
			}

			// Here's where the work is done... call the handler.
			(handler->Handler)(receiver, spec, pval);

			// We've now processed the whole format specification.
			format++;
		}
	}

	// Write the terminating 0.
	receiver.Receive('\0');
}


// Below are all the handlers and their helper functions.


void tSystem::HandlerHelper_JustificationProlog(Receiver& receiver, int itemLength, const FormatSpec& spec)
{
	// Prolog only outputs characters if we are right justifying.
	if (spec.Flags & Flag_LeftJustify)
		return;

	// Right justify.
	for (int s = 0; s < (spec.Width - itemLength); s++)
		if (spec.Flags & Flag_LeadingZeros)
			receiver.Receive('0');
		else
			receiver.Receive(' ');
}


void tSystem::HandlerHelper_JustificationEpilog(Receiver& receiver, int itemLength, const FormatSpec& spec)
{
	// Epilog only outputs characters if we are left justifying.
	if (!(spec.Flags & Flag_LeftJustify))
		return;

	// Left justify.
	for (int s = 0; s < (spec.Width - itemLength); s++)
		receiver.Receive(' ');
}


void tSystem::HandlerHelper_IntegerNative
(
	tArray<char>& convBuf, const FormatSpec& spec, void* data, bool treatAsUnsigned,
	int bitSize, bool upperCase, int base, bool forcePrefixLowerCase
)
{
	tAssert((bitSize == 32) || (bitSize == 64));
	uint64 rawValue = (bitSize == 32) ? (*((uint32*)data)) : (*((uint64*)data));
	bool negative = (rawValue >> (bitSize-1)) ? true : false;
	int remWidth = spec.Width;

	if (base == 10)
	{
		if (!treatAsUnsigned && negative)
		{
			// Negative values need a - in front. Then we can print the rest as if it were positive.
			rawValue = -( int64(rawValue) );
			convBuf.Append('-');
			remWidth--;
		}
		else if (spec.Flags & Flag_ForcePosOrNegSign)
		{
			convBuf.Append('+');
			remWidth--;
		}
		else if (spec.Flags & Flag_SpaceForPosSign)
		{
			convBuf.Append(' ');
			remWidth--;
		}
	}

	if (bitSize == 32)
		rawValue &= 0x00000000FFFFFFFF;

	// According to the standard, the # should only cause the prefix to be appended if the value
	// is non-zero. Also, we support a %p pointer type, where we DO want the prefix even for a
	// null pointer... that what forcePrefix is for.
	if (((spec.Flags & Flag_BasePrefix) && rawValue) || forcePrefixLowerCase)
	{
		switch (base)
		{
			case 8:
				convBuf.Append('0');
				remWidth--;
				break;

			case 16:
				convBuf.Append((!upperCase || forcePrefixLowerCase) ? "0x" : "0X", 2);
				remWidth -= 2;
				break;
		}
	}

	char baseBiggerThanTenOffsetToLetters = 'a' - '9' - 1;
	if	(upperCase)
		baseBiggerThanTenOffsetToLetters = 'A' - '9' - 1;

	// According to MS printf docs if 0 is specified with an integer format (i, u, x, X, o, d) and a precision
	// specification is also present (for example, %04.d), the 0 is ignored. Note that default 'precision' for
	// integral types is 1.
	uint32 flags = spec.Flags;
	int precision = spec.Precision;
	if (precision == -1)
		precision = 1;
	else
		flags &= ~Flag_LeadingZeros;

	// It needs to be this big to handle 64 bit in binary.
	char buf[128];
	buf[127] = '\0';
	char* curr = &buf[126];

	while ((precision-- > 0) || rawValue)
	{
		char digit = char((rawValue % base) + '0');
		rawValue /= base;
		if (digit > '9')
			digit += baseBiggerThanTenOffsetToLetters;
		*curr = digit;
		curr--;
	}

	curr++;
	if (flags & Flag_LeadingZeros)
	{
		int numZeroes = remWidth - tStd::tStrlen(curr);
		for (int z = 0; z < numZeroes; z++)
		{
			curr--;
			*curr = '0';
		}
	}

	if (flags & Flag_DecorativeFormatting)
	{
		int len = tStd::tStrlen(curr);
		int mod = 4 - (len % 4);
		for (int i = 0; i < len; i++)
		{
			convBuf.Append(curr[i]);
			if (!(++mod % 4) && (i != (len-1)))
				convBuf.Append('_');
		}
	}
	else if (flags & Flag_DecorativeFormattingAlt)
	{
		int len = tStd::tStrlen(curr);
		int mod = 3 - (len % 3);
		for (int i = 0; i < len; i++)
		{
			convBuf.Append(curr[i]);
			if (!(++mod % 3) && (i != (len-1)))
				convBuf.Append(',');
		}
	}
	else
	{
		convBuf.Append(curr, tStd::tStrlen(curr));
	}
}


void tSystem::HandlerHelper_IntegerTacent
(
	tArray<char>& convBuf, const FormatSpec& spec, void* data, bool treatAsUnsigned,
	int bitSize, bool upperCase, int base, bool forcePrefixLowerCase
)
{
	tAssert((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tuint512 rawValue;
	if (bitSize == 128)
		rawValue = *((tuint128*)data);
	else if (bitSize == 256)
		rawValue = *((tuint256*)data);
	else
		rawValue = *((tuint512*)data);

	bool negative = (rawValue >> (bitSize-1)) ? true : false;
	int remWidth = spec.Width;

	if (base == 10)
	{
		if (!treatAsUnsigned && negative)
		{
			// Negative values need a - in front. Then we can print the rest as if it were positive.
			rawValue = -( tint512(rawValue) );
			convBuf.Append('-');
			remWidth--;
		}
		else if (spec.Flags & Flag_ForcePosOrNegSign)
		{
			convBuf.Append('+');
			remWidth--;
		}
		else if (spec.Flags & Flag_SpaceForPosSign)
		{
			convBuf.Append(' ');
			remWidth--;
		}
	}

	if (bitSize == 128)
		rawValue &= tuint512("0x000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
	if (bitSize == 256)
		rawValue &= tuint512("0x0000000000000000000000000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

	// According to the standard, the # should only cause the prefix to be appended if the value
	// is non-zero. Also, we support a %p pointer type, where we DO want the prefix even for a
	// null pointer... that is what forcePrefix is for.
	if (((spec.Flags & Flag_BasePrefix) && rawValue) || forcePrefixLowerCase)
	{
		switch (base)
		{
			case 8:
				convBuf.Append('0');
				remWidth--;
				break;

			case 16:
				convBuf.Append((!upperCase || forcePrefixLowerCase) ? "0x" : "0X", 2);
				remWidth -= 2;
				break;
		}
	}

	char baseBiggerThanTenOffsetToLetters = 'a' - '9' - 1;
	if	(upperCase)
		baseBiggerThanTenOffsetToLetters = 'A' - '9' - 1;

	uint32 flags = spec.Flags;
	int precision = spec.Precision;
	if (precision == -1)
		precision = 1;
	else
		flags &= ~Flag_LeadingZeros;

	// It needs to be big enough to handle a 512 bit integer as a string in binary.
	char buf[1024];
	buf[1023] = '\0';
	char* curr = &buf[1022];

	while ((precision-- > 0) || rawValue)
	{
		int modVal = rawValue % base;
		char digit = char(modVal + '0');
		rawValue /= base;
		if (digit > '9')
			digit += baseBiggerThanTenOffsetToLetters;
		*curr = digit;
		curr--;
	}

	curr++;
	if (flags & Flag_LeadingZeros)
	{
		int numZeroes = remWidth - tStd::tStrlen(curr);
		for (int z = 0; z < numZeroes; z++)
		{
			curr--;
			*curr = '0';
		}
	}

	if (flags & Flag_DecorativeFormatting)
	{
		int len = tStd::tStrlen(curr);
		int mod = 8 - (len % 8);
		for (int i = 0; i < len; i++)
		{
			convBuf.Append(curr[i]);
			if ((!(++mod % 8)) && (i != (len-1)))
				convBuf.Append('_');
		}
	}
	else if (flags & Flag_DecorativeFormattingAlt)
	{
		int len = tStd::tStrlen(curr);
		int mod = 3 - (len % 3);
		for (int i = 0; i < len; i++)
		{
			convBuf.Append(curr[i]);
			if ((!(++mod % 3)) && (i != (len-1)))
				convBuf.Append(',');
		}
	}
	else
	{
		convBuf.Append(curr, tStd::tStrlen(curr));
	}
}


void tSystem::Handler_b(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = true;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = false;
	int base = 2;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	// Tacent integers are quite a bit bigger and will require more buffer space. Enough for 512 binary digits.
	// Native: max 64 (*2) = 128. Tacent max 512 (*2) = 1024. Or, if the native needed X bytes (for 64bit number)
	// then the tacent type will need 8 times that (4*X) cuz it's 512 bits.
	int bufSize = nativeInt ? 128 : 1024;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_o(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = true;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = false;
	int base = 8;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	int bufSize = nativeInt ? 64 : 512;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_d(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = false;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = false;
	int base = 10;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	int bufSize = nativeInt ? 64 : 512;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_i(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = false;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = false;
	int base = 10;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	int bufSize = nativeInt ? 64 : 512;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_u(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = true;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = false;
	int base = 10;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	int bufSize = nativeInt ? 64 : 512;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_x(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = true;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = false;
	int base = 16;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	int bufSize = nativeInt ? 64 : 512;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_X(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool treatAsUnsigned = true;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = true;
	int base = 16;

	bool nativeInt = ((bitSize == 32) || (bitSize == 64));
	bool tacentInt = ((bitSize == 128) || (bitSize == 256) || (bitSize == 512));
	tAssert(nativeInt || tacentInt);

	int bufSize = nativeInt ? 64 : 512;
	tArray<char> convInt(bufSize, 0);
	if (nativeInt)
		HandlerHelper_IntegerNative(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);
	else
		HandlerHelper_IntegerTacent(convInt, spec, data, treatAsUnsigned, bitSize, upperCase, base);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), spec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), spec);
}


void tSystem::Handler_p(Receiver& receiver, const FormatSpec& spec, void* data)
{
	FormatSpec pspec = spec;
	pspec.Flags |= Flag_LeadingZeros;
	if (!spec.Width)
		pspec.Width = 2 + 2*spec.TypeSizeBytes;
	bool treatAsUnsigned = true;
	int bitSize = spec.TypeSizeBytes*8;
	bool upperCase = true;
	int base = 16;
	bool forcePrefixLowerCase = true;
	tArray<char> convInt(64, 0);
	HandlerHelper_IntegerNative(convInt, pspec, data, treatAsUnsigned, bitSize, upperCase, base, forcePrefixLowerCase);

	HandlerHelper_JustificationProlog(receiver, convInt.GetNumElements(), pspec);
	receiver.Receive(convInt);
	HandlerHelper_JustificationEpilog(receiver, convInt.GetNumElements(), pspec);
}


int tSystem::HandlerHelper_FloatComputeExponent(double value)
{
	int exponent = 0;
	value = (value < 0.0) ? -value : value;
	if (value >= 10.0)
	{
		while (value >= 10.0)
		{
			value /= 10.0;
			exponent++;
		}
	}
	else if (value < 1.0)
	{
		int digit = int(value);
		while (value && !digit)
		{
			value *= 10.0;
			exponent--;
			digit = int(value);
		}
	}

	return exponent;
}


bool tSystem::HandlerHelper_HandleSpecialFloatTypes(tArray<char>& convBuf, double value)
{
	tStd::tFloatType ft = tStd::tGetFloatType(value);
	switch (ft)
	{
		case tStd::tFloatType::PSNAN:	convBuf.Append("nan(snan)", 9);		return true;
		case tStd::tFloatType::PQNAN:	convBuf.Append("nan", 3);			return true;
		case tStd::tFloatType::NSNAN:	convBuf.Append("-nan(snan)", 10);	return true;
		case tStd::tFloatType::NQNAN:	convBuf.Append("-nan", 4);			return true;
		case tStd::tFloatType::IQNAN:	convBuf.Append("-nan(ind)", 9);		return true;
		case tStd::tFloatType::PINF:	convBuf.Append("inf", 3);			return true;
		case tStd::tFloatType::NINF:	convBuf.Append("-inf", 4);			return true;
		default:
		case tStd::tFloatType::NORM:
			break;
	}

	return false;
}


void tSystem::Handler_e(Receiver& receiver, const FormatSpec& spec, void* data)
{
	// Variable argument specifies data should be treated data as double. i.e. %f is 64 bits.
	double v = *((double*)data);

	// Check for early exit infinities and NANs.
	tArray<char> convBuf(64, 32);
	if (HandlerHelper_HandleSpecialFloatTypes(convBuf, v))
	{
		receiver.Receive(convBuf);
		return;
	}

	// @todo Fix this like the Handler_f was fixed so it can handle appending directly into appending to the dynamically growing convBuf.
	char result[64];
	const int maxLeadingZeroes = 16;
	char* curr = result + maxLeadingZeroes;
	bool negative = false;

	if (v < 0.0f)
	{
		v = -v;
		negative = true;
	}

	double val = double(v);
	int exponent = HandlerHelper_FloatComputeExponent(val);

	// Convert val so it is a single non-zero digit before the decimal point.
	double power10 = 1.0;
	int absExp = (exponent < 0) ? -exponent : exponent;
	for (int e = 0; e < absExp; e++)
		power10 *= 10.0;

	if (exponent != 0)
		val = (exponent < 0) ? (val * power10) : (val / power10);

	// Sometimes errors can cause 9.999999 -> 10.0.
	while (val >= 10.0)
	{
		val /= 10.0;
		exponent++;
	}

	// Default floating point printf precision. ANSI is 6, ours is 4.
	int precision = spec.Precision;
	if (precision == -1)
		precision = DefaultPrecision;

	power10 = 1.0;
	for (int e = 0; e < precision; e++)
		power10 *= 10.0;
	double precisionRound = 0.5 / power10;
	val += precisionRound;

	bool firstDigit = true;
	while (precision)
	{
		int digit = int(val);
		val -= digit;
		val *= 10.0;
		*curr++ = '0' + digit;
		if (firstDigit)
			*curr++ = '.';
		else
			precision--;

		firstDigit = false;
	}

	*curr++ = 'e';			// Need to pass in an uppercase boolean.
	if (exponent >= 0)
	{
		*curr++ = '+';
	}
	else
	{
		*curr++ = '-';
		exponent = -exponent;
	}

	// @todo Make width here controllable by opt display flag.
	const int expWidthMax = 3;

	// First we need to write the exponent characters into a temp buffer backwards. This is so we have to whole thing
	// before we don't process leading zeroes.
	int expBuf[expWidthMax] = { 0, 0, 0 };
	for (int n = expWidthMax-1; n >= 0; n--)
	{
		int digit = exponent % 10;
		exponent /= 10;
		expBuf[n] = digit;
	}

	// We always include the last two least-significant digits of the base 10 exponent, even if they are both zeroes.
	// We only include the first digit if it is non-zero. This can only happen with doubles, not floats which max at 38.
	if (expBuf[0] != 0)
		*curr++ = '0' + expBuf[0];
	*curr++ = '0' + expBuf[1];
	*curr++ = '0' + expBuf[2];
	*curr++ = '\0';
	
	// If there are no leading zeroes any possible plus or negative sign must go beside the first valid character of the
	// converted string. However, if there ARE leading zeroes, we still need to place the plus or negative based on the
	// width.
	curr = result + maxLeadingZeroes;
	if (!(spec.Flags & Flag_LeadingZeros))
	{
		if (negative)
			*--curr = '-';
		else if (spec.Flags & Flag_ForcePosOrNegSign)
			*--curr = '+';
		else if (!negative && (spec.Flags & Flag_SpaceForPosSign))
			*--curr = ' ';
	}
	else
	{
		int numZeroes = spec.Width - tStd::tStrlen(curr);
		if (numZeroes > maxLeadingZeroes)
			numZeroes = maxLeadingZeroes;
		while (numZeroes-- > 0)
			*--curr = '0';

		if (negative)
			*curr = '-';
		else if (spec.Flags & Flag_ForcePosOrNegSign)
			*curr = '+';
	}

	receiver.Receive(curr, tStd::tStrlen(curr));
}


tSystem::PrologHelperFloat tSystem::HandlerHelper_FloatNormal(tArray<char>& convBuf, const FormatSpec& spec, double value, bool treatPrecisionAsSigDigits)
{
	tArray<char> buf(64, 32);
	buf.Append('0');

	// Default floating point printf precision. ANSI is 6, ours is 4.
	int precision = spec.Precision;
	if (precision == -1)
		precision = DefaultPrecision;

	bool wasNeg = (value < 0.0f) ? true : false;
	if (value < 0.0f)
		value = -value;

	// We always need to use a minus sign if val was negative.
	PrologHelperFloat ret = PrologHelperFloat::None;
	if (wasNeg)
		ret = PrologHelperFloat::NeedsNeg;
	else if (spec.Flags & Flag_ForcePosOrNegSign)
		ret = PrologHelperFloat::NeedsPlus;
	else if (spec.Flags & Flag_SpaceForPosSign)
		ret = PrologHelperFloat::NeedsSpace;

	double dec = 1.0;
	while (dec < value)
		dec *= 10.0;

	if (dec > value)
		dec /= 10.0;

	// Is there a mantissa?
	bool hasMantissa = false;
	while (dec >= 1.0)
	{
		char digit = char(value / dec);
		value -= digit * dec;
		buf.Append(digit + '0');
		if (treatPrecisionAsSigDigits && (precision > 0))
			precision--;
		dec /= 10.0;
		hasMantissa = true;
	}

	// No mantissa means use a 0 instead.
	if (!hasMantissa)
		buf.Append('0');

	if (precision > 0)
		buf.Append('.');

	// We're now after the decimal point... how far we go depends on precision.
	while (precision--)
	{
		value *= 10.0;
		char digit = char(value);

		value -= digit;
		dec += digit;
		buf.Append(digit + '0');
	}

	bool useIdxZeroForResult = false;
	if ((value * 10.0) >= 5.0)
	{
		// Round. We need to start at the end and work BACKWARDS to the left.
		// We gave already reserved a character at the beginning of the buffer for a possible carry.
		char* end = buf.GetElements() + buf.GetNumElements() - 1;
		while (1)
		{
			if (*end == '9')
			{
				*end = '0';
			}
			else if (*end == '.')
			{
				end--;
				continue;
			}
			else
			{
				break;
			}

			end--;
		}

		// Write to the buffer.
		(*end)++ ;

		// The first character of buf was reserved just for this.
		if (end == &buf[0])
			useIdxZeroForResult = true;
	}

	buf.Append('\0');
	char* result = &buf[1];
	if (useIdxZeroForResult)
		result = &buf[0];

	// This is tricky. If there are no leading zeroes any possible plus or negative sign must go beside
	// the first valid character of the converted string. However, if there ARE leading zeroes, we still
	// need to place the plus or negative based on the width, which is done outside this helper.
	if (!(spec.Flags & Flag_LeadingZeros))
	{
		if (ret == PrologHelperFloat::NeedsNeg)
		{
			convBuf.Append('-');
			ret = PrologHelperFloat::None;
		}
		else if (ret == PrologHelperFloat::NeedsPlus)
		{
			convBuf.Append('+');
			ret = PrologHelperFloat::None;
		}
	}

	convBuf.Append(result, tStd::tStrlen(result));
	return ret;
}


void tSystem::Handler_f(Receiver& receiver, const FormatSpec& spec, void* data)
{
	// Variable arg rules say you must treat the data as double. It converts automatically. That's why %f is always 64 bits.
	double value = *((double*)data);
	tArray<char> convFloat(64, 32);

	// Check for early exit infinities and NANs.
	PrologHelperFloat res = PrologHelperFloat::None;
	if (HandlerHelper_HandleSpecialFloatTypes(convFloat, value))
		res = PrologHelperFloat::NoZeros;
	else
		res = HandlerHelper_FloatNormal(convFloat, spec, value);

	FormatSpec modSpec(spec);
	int effectiveLength = convFloat.GetNumElements();
	switch (res)
	{
		case PrologHelperFloat::NeedsNeg:		receiver.Receive('-');	effectiveLength++;	break;
		case PrologHelperFloat::NeedsPlus:		receiver.Receive('+');	effectiveLength++;	break;
		case PrologHelperFloat::NeedsSpace:		receiver.Receive(' ');	effectiveLength++;	break;
		case PrologHelperFloat::NoZeros:		modSpec.Flags &= ~Flag_LeadingZeros;		break;
		case PrologHelperFloat::None:														break;
	}

	HandlerHelper_JustificationProlog(receiver, effectiveLength, modSpec);
	receiver.Receive(convFloat);
	HandlerHelper_JustificationEpilog(receiver, effectiveLength, modSpec);
}


void tSystem::Handler_g(Receiver& receiver, const FormatSpec& spec, void* data)
{
	// Variable argument specifies data should be treated data as double. i.e. %f is 64 bits.
	double v = *((double*)data);
	tArray<char> convBuf(64, 32);

	// Default floating point printf precision. ANSI is 6, ours is 4.
	// For %g, the precision is treated as significant digits, not number of digits after the decimal point.
	int precision = spec.Precision;
	if (precision == -1)
		precision = DefaultPrecision;

	double noExpFormatThreshold = tPow(10.0, double(precision));
	if (v < noExpFormatThreshold)
	{
		// Check for early exit infinities and NANs.
		PrologHelperFloat res = PrologHelperFloat::None;
		if (HandlerHelper_HandleSpecialFloatTypes(convBuf, v))
			res = PrologHelperFloat::NoZeros;
		else
			res = HandlerHelper_FloatNormal(convBuf, spec, v, true);

		FormatSpec modSpec(spec);
		int effectiveLength = convBuf.GetNumElements();
		switch (res)
		{
			case PrologHelperFloat::NeedsNeg:		receiver.Receive('-');	effectiveLength++;	break;
			case PrologHelperFloat::NeedsPlus:		receiver.Receive('+');	effectiveLength++;	break;
			case PrologHelperFloat::NeedsSpace:		receiver.Receive(' ');	effectiveLength++;	break;
			case PrologHelperFloat::NoZeros:		modSpec.Flags &= ~Flag_LeadingZeros;		break;
			case PrologHelperFloat::None:														break;
		}

		HandlerHelper_JustificationProlog(receiver, effectiveLength, modSpec);
		receiver.Receive(convBuf);
		HandlerHelper_JustificationEpilog(receiver, effectiveLength, modSpec);
		return;
	}

	// Check for early exit infinities and NANs.
	if (HandlerHelper_HandleSpecialFloatTypes(convBuf, v))
	{
		receiver.Receive(convBuf);
		return;
	}

	// @todo Fix this like the Handler_f was fixed so it can handle appending directly into appending to the dynamically growing convBuf.
	char result[64];
	const int maxLeadingZeroes = 16;
	char* curr = result + maxLeadingZeroes;
	bool negative = false;

	if (v < 0.0f)
	{
		v = -v;
		negative = true;
	}

	double val = double(v);
	int exponent = HandlerHelper_FloatComputeExponent(val);

	// Convert val so it is a single non-zero digit before the decimal point.
	double power10 = 1.0;
	int absExp = (exponent < 0) ? -exponent : exponent;
	for (int e = 0; e < absExp; e++)
		power10 *= 10.0;

	if (exponent != 0)
		val = (exponent < 0) ? (val * power10) : (val / power10);

	// Sometimes errors can cause 9.999999 -> 10.0.
	while (val >= 10.0)
	{
		val /= 10.0;
		exponent++;
	}

	power10 = 1.0;
	for (int e = 0; e < precision; e++)
		power10 *= 10.0;
	double precisionRound = 0.5 / power10;
	val += precisionRound;

	bool firstDigit = true;
	while (precision)
	{
		int digit = int(val);
		val -= digit;
		val *= 10.0;
		precision--;
		// Round the last digit up if necessary. There's a subtle error here: if the digit is
		// 9 we just truncate, whereas we really need another rounding loop to carry the round upwards
		// through the 9s.
		if ((precision == 0) && (int(val) >= 5) && (digit < 9))
			digit++;
		*curr++ = '0' + digit;

		if (firstDigit)
			*curr++ = '.';

		firstDigit = false;
	}

	*curr++ = 'e';			// Need to pass in an uppercase boolean.
	if (exponent >= 0)
	{
		*curr++ = '+';
	}
	else
	{
		*curr++ = '-';
		exponent = -exponent;
	}

	// @todo Make width here controllable by opt display flag.
	const int expWidthMax = 3;

	// First we need to write the exponent characters into a temp buffer backwards. This is so we have to whole thing
	// before we don't process leading zeroes.
	int expBuf[expWidthMax] = { 0, 0, 0 };
	for (int n = expWidthMax-1; n >= 0; n--)
	{
		int digit = exponent % 10;
		exponent /= 10;
		expBuf[n] = digit;
	}

	// We always include the last two least-significant digits of the base 10 exponent, even if they are both zeroes.
	// We only include the first digit if it is non-zero. This can only happen with doubles, not floats which max at 38.
	if (expBuf[0] != 0)
		*curr++ = '0' + expBuf[0];
	*curr++ = '0' + expBuf[1];
	*curr++ = '0' + expBuf[2];
	*curr++ = '\0';
	
	// If there are no leading zeroes any possible plus or negative sign must go beside the first valid character of the
	// converted string. However, if there ARE leading zeroes, we still need to place the plus or negative based on the
	// width.
	curr = result + maxLeadingZeroes;
	if (!(spec.Flags & Flag_LeadingZeros))
	{
		if (negative)
			*--curr = '-';
		else if (spec.Flags & Flag_ForcePosOrNegSign)
			*--curr = '+';
		else if (!negative && (spec.Flags & Flag_SpaceForPosSign))
			*--curr = ' ';
	}
	else
	{
		int numZeroes = spec.Width - tStd::tStrlen(curr);
		if (numZeroes > maxLeadingZeroes)
			numZeroes = maxLeadingZeroes;
		while (numZeroes-- > 0)
			*--curr = '0';

		if (negative)
			*curr = '-';
		else if (spec.Flags & Flag_ForcePosOrNegSign)
			*curr = '+';
	}

	receiver.Receive(curr, tStd::tStrlen(curr));
}


void tSystem::HandlerHelper_Vector(Receiver& receiver, const FormatSpec& spec, const float* components, int numComponents)
{
	if (spec.Flags & Flag_DecorativeFormatting)
	{
		for (int c = 0; c < numComponents; c++)
		{
			double comp = double(components[c]);
			Handler_f(receiver, spec, &comp);
			if (c < (numComponents-1))
				receiver.Receive(' ');
		}
	}
	else
	{
		receiver.Receive('(');
		for (int c = 0; c < numComponents; c++)
		{
			double comp = double(components[c]);
			Handler_f(receiver, spec, &comp);
			if (c < (numComponents-1))
				receiver.Receive(", ", 2);
		}
		receiver.Receive(')');
	}
}


void tSystem::Handler_v(Receiver& receiver, const FormatSpec& spec, void* data)
{
	int numComponents = spec.TypeSizeBytes >> 2;
	tAssert((numComponents >= 2) && (numComponents <= 4));
	tVec4* vec = (tVec4*)data;
	float* components = &vec->x;

	HandlerHelper_Vector(receiver, spec, components, numComponents);
}


void tSystem::Handler_q(Receiver& receiver, const FormatSpec& spec, void* data)
{
	tQuat* quat = (tQuat*)data;

	if (spec.Flags & Flag_DecorativeFormatting)
	{
		receiver.Receive('(');
		double w = double(quat->w);
		Handler_f(receiver, spec, &w);
		receiver.Receive(", (", 3);

		double x = double(quat->x);
		Handler_f(receiver, spec, &x);
		receiver.Receive(", ", 2);

		double y = double(quat->y);
		Handler_f(receiver, spec, &y);
		receiver.Receive(", ", 2);

		double z = double(quat->z);
		Handler_f(receiver, spec, &z);
		receiver.Receive("))", 2);
	}
	else
	{
		float* components = &quat->x;
		receiver.Receive('(');
		for (int c = 0; c < 4; c++)
		{
			double comp = double(components[c]);
			Handler_f(receiver, spec, &comp);
			if (c < 3)
				receiver.Receive(", ", 2);
		}
		receiver.Receive(')');
	}
}


void tSystem::Handler_m(Receiver& receiver, const FormatSpec& spec, void* data)
{
	bool is4x4 = (spec.TypeSizeBytes == sizeof(tMat4)) ? true : false;
	bool is2x2 = (spec.TypeSizeBytes == sizeof(tMat2)) ? true : false;
	tAssert(is4x4 || is2x2);

	if (is4x4)
	{
		tMat4* mat = (tMat4*)data;

		if (spec.Flags & Flag_DecorativeFormatting)
		{
			FormatSpec vecSpec(spec);
			if (!spec.Width)
				vecSpec.Width = 9;
			if (spec.Precision == -1)
				vecSpec.Precision = 4;

			tVec4 row1 = { mat->C1.x, mat->C2.x, mat->C3.x, mat->C4.x };
			tVec4 row2 = { mat->C1.y, mat->C2.y, mat->C3.y, mat->C4.y };
			tVec4 row3 = { mat->C1.z, mat->C2.z, mat->C3.z, mat->C4.z };
			tVec4 row4 = { mat->C1.w, mat->C2.w, mat->C3.w, mat->C4.w };

			receiver.Receive("[ ", 2);		HandlerHelper_Vector(receiver, vecSpec, &row1.x, 4);		receiver.Receive('\n');
			receiver.Receive("  ", 2);		HandlerHelper_Vector(receiver, vecSpec, &row2.x, 4);		receiver.Receive('\n');
			receiver.Receive("  ", 2);		HandlerHelper_Vector(receiver, vecSpec, &row3.x, 4);		receiver.Receive('\n');
			receiver.Receive("  ", 2);		HandlerHelper_Vector(receiver, vecSpec, &row4.x, 4);		receiver.Receive(" ]\n", 3);
		}
		else
		{
			receiver.Receive('(');
			HandlerHelper_Vector(receiver, spec, &mat->C1.x, 4);
			receiver.Receive(", ", 2);
			HandlerHelper_Vector(receiver, spec, &mat->C2.x, 4);
			receiver.Receive(", ", 2);
			HandlerHelper_Vector(receiver, spec, &mat->C3.x, 4);
			receiver.Receive(", ", 2);
			HandlerHelper_Vector(receiver, spec, &mat->C4.x, 4);
			receiver.Receive(')');
		}
	}
	else
	{
		tMat2* mat = (tMat2*)data;

		if (spec.Flags & Flag_DecorativeFormatting)
		{
			FormatSpec vecSpec(spec);
			if (!spec.Width)
				vecSpec.Width = 9;
			if (spec.Precision == -1)
				vecSpec.Precision = 4;

			tVec2 row1 = { mat->C1.x, mat->C2.x };
			tVec2 row2 = { mat->C1.y, mat->C2.y };

			receiver.Receive("[ ", 2);		HandlerHelper_Vector(receiver, vecSpec, &row1.x, 2);		receiver.Receive('\n');
			receiver.Receive("  ", 2);		HandlerHelper_Vector(receiver, vecSpec, &row2.x, 2);		receiver.Receive(" ]\n", 3);
		}
		else
		{
			receiver.Receive('(');
			HandlerHelper_Vector(receiver, spec, &mat->C1.x, 2);
			receiver.Receive(", ", 2);
			HandlerHelper_Vector(receiver, spec, &mat->C2.x, 2);
			receiver.Receive(')');
		}
	}
}


void tSystem::Handler_c(Receiver& receiver, const FormatSpec& spec, void* data)
{
	const char chr = *((const char*)data);
	receiver.Receive(chr);
}


void tSystem::Handler_z(Receiver& receiver, const FormatSpec& spec, void* data)
{
	receiver.Receive('%');
}


void tSystem::Handler_s(Receiver& receiver, const FormatSpec& spec, void* data)
{
	const char* str = *((const char**)data);

	int numToAppend = tStd::tStrlen(str);
	if ((spec.Precision != -1) && (numToAppend > spec.Precision))
		numToAppend = spec.Precision;

	HandlerHelper_JustificationProlog(receiver, numToAppend, spec);
	receiver.Receive(str, numToAppend);
	HandlerHelper_JustificationEpilog(receiver, numToAppend, spec);
}
