// tStandard.h
//
// Tacent functions and types that are standard across all platforms. Includes global functions like itoa which are not
// available on some platforms, but are common enough that they should be.
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "Foundation/tPlatform.h"
#include "Foundation/tAssert.h"
#pragma warning (disable: 4996)
#pragma warning (disable: 4146)
#pragma warning (disable: 4307)


namespace tStd
{


// The 3 XOR trick is slower in most cases so we'll use a standard swap.
template<typename T> inline void tSwap(T& a, T& b)																		{ T t = a; a = b; b = t; }
inline void* tMemcpy(void* dest, const void* src, int numBytes)															{ return memcpy(dest, src, numBytes); }
inline void* tMemset(void* dest, uint8 val, int numBytes)																{ return memset(dest, val, numBytes); }
inline int tMemcmp(const void* a, const void* b, int numBytes)															{ return memcmp(a, b, numBytes); }

// For character strings we support regular 8 bit characters (ASCII) and full unicode via UTF8. We do not support either
// USC2 or UTF16. The CT (Compile-Time) strlen variant below can compute the string length at compile-time for constant
// string literals.
const int tCharInvalid																									= 0xFF;
inline int tStrcmp(const char* a, const char* b)																		{ tAssert(a && b); return strcmp(a, b); }
inline int tStricmp(const char* a, const char* b)																		{ tAssert(a && b); return stricmp(a, b); }
inline int tStrncmp(const char* a, const char* b, int n)																{ tAssert(a && b && n >= 0); return strncmp(a, b, n); }
inline int tStrnicmp(const char* a, const char* b, int n)																{ tAssert(a && b && n >= 0); return strnicmp(a, b, n); }
inline int tStrlen(const char* s)																						{ tAssert(s); return int(strlen(s)); }
inline constexpr int tStrlenCT(const char* s)																			{ return *s ? 1 + tStrlenCT(s + 1) : 0; }
inline char* tStrcpy(char* dst, const char* src)																		{ tAssert(dst && src); return strcpy(dst, src); }
inline char* tStrncpy(char* dst, const char* src, int n)																{ tAssert(dst && src && n >= 0); return strncpy(dst, src, n); }
inline const char* tStrchr(const char* s, int c)																		{ tAssert(s && c >= 0 && c < 0x100); return strchr(s, c); }
inline const char* tStrstr(const char* s, const char* r)																{ tAssert(s && r); return strstr(s, r); }
inline char* tStrstr(char* s, const char* r)																			{ tAssert(s && r); return strstr(s, r); }
inline char* tStrupr(char* s)																							{ tAssert(s); return _strupr(s); }
inline char* tStrlwr(char* s)																							{ tAssert(s); return _strlwr(s); }

// For these conversion calls, unknown digit characters for the supplied base are ignored. If base is not E [2, 36], the
// base in which to interpret the string is determined by passing a prefix in the string. Base 10 is used if no specific
// prefix is found. Base prefixes in use:
//
// Base 16 prefixes: x X 0x 0X #
// Base 10 prefixes: d D 0d 0D
// Base 8 prefixes: o O 0o 0O @
// Base 4  prefixes: n N 0n 0N (N for Nybble)
// Base 2  prefixes: b B 0b 0B !
//
// Negative/positive symbol may only be used with base 10 strings: eg. "d-769" or just "-769". The behaviour follows
// this recipe: If the base is invalid (not between 2 and 36) the first and maybe second characters are examined to
// determine base. Next, invalid characters are removed (implementation may just ignore them). Invalid characters
// include + and - for non base 10. Finally the conversion is performed. Valid digits for base 36 are 0-9, a-z, and A-Z.
// In all string-to-int functions, 0 is returned if there is no conversion. This can happen if all characters are
// invalid, the passed in string is null, or the passed in string is empty.
//
// If base is E (1, 36] AND a prefix is supplied, the supplied base is used. Using the prefix instead would be
// ambiguous. For example, "0x" is a valid base 36 number but "0x" is also a prefix. If you supply a prefix it must
// match the base or you will get undefined results. eg "0xff" or "xff" with base=36 interprets the 'x', correctly, as
// 33. "0xff" with base=16 works too because 'x' is an invalid hex digit and gets ignored. The '0' is leading so also
// does not interfere. The same behaviour holds for all prefixes and bases. Finally, don't pass a base-34 number in with
// the base set to -1, although that shouldn't happen anyways.
template <typename IntegralType> IntegralType tStrtoiT(const char*, int base = -1);
inline int32 tStrtoi32(const char* s, int base = -1)																	{ return tStrtoiT<int32>(s, base); }
inline uint32 tStrtoui32(const char* s, int base = -1)																	{ return tStrtoiT<uint32>(s, base); }
inline int64 tStrtoi64(const char* s, int base = -1)																	{ return tStrtoiT<int64>(s, base); }
inline uint64 tStrtoui64(const char* s, int base = -1)																	{ return tStrtoiT<uint64>(s, base); }
inline int tStrtoui(const char* s, int base = -1)																		{ return tStrtoui32(s, base); }
inline int tStrtoi(const char* s, int base = -1)																		{ return tStrtoi32(s, base); }
inline int tAtoi(const char* s)								/* Base 10 only. Use tStrtoi for arbitrary base. */			{ return tStrtoi32(s, 10); }

// These are both base 10 only. They return 0.0f if there is no conversion.
double tStrtod(const char*);
inline float tStrtof(const char* s)																						{ return float( tStrtod(s) ); }

// These are both base 10 only. They return 0.0 if there is no conversion.
inline float tAtof(const char* s)																						{ return tStrtof(s); }
inline double tAtod(const char* s)																						{ return tStrtod(s); }

// String to bool. Case insensitive. Interprets "true", "t", "yes", "y", "on", "enable", "enabled", "1", "+", and
// strings that represent non-zero integers as boolean true. Otherwise false.
bool tStrtob(const char*);

// Here are the functions for going from integral types to strings. strSize (as opposed to length) must include
// enough room for the terminating null. strSize should be the full size of the passed-in str buffer. The resulting
// string, if letter characters are called for (bases > 10) will be capital, not lower case. Base must be E [2, 36].
// Conversion problems boil down to passing null str, strSize being too small, and specifying an out-of-bounds base.
// Returns false in these cases, and true on success. The int-to-string functions are mainly available to handle
// arbitrary base E (1, 36] since tsPrintf only handles octal, decimal, and hex. Floating point conversions to
// strings should be handled by the tPrintf-style functions due to their superior formatting specifiers.
template <typename IntegralType> bool tItostrT(IntegralType value, char* str, int strSize, int base = 10);
inline bool tItostr(int32 value, char* str, int strSize, int base = 10)													{ return tItostrT<int32>(value, str, strSize, base); }
inline bool tItostr(int64 value, char* str, int strSize, int base = 10)													{ return tItostrT<int64>(value, str, strSize, base); }
inline bool tItostr(uint32 value, char* str, int strSize, int base = 10)												{ return tItostrT<uint32>(value, str, strSize, base); }
inline bool tItostr(uint64 value, char* str, int strSize, int base = 10)												{ return tItostrT<uint64>(value, str, strSize, base); }
inline bool tItoa(int32 value, char* str, int strSize, int base = 10)													{ return tItostr(value, str, strSize, base); }
inline bool tItoa(int64 value, char* str, int strSize, int base = 10)													{ return tItostr(value, str, strSize, base); }
inline bool tItoa(uint32 value, char* str, int strSize, int base = 10)													{ return tItostr(value, str, strSize, base); }
inline bool tItoa(uint64 value, char* str, int strSize, int base = 10)													{ return tItostr(value, str, strSize, base); }

inline bool tIsspace(char c)																							{ return isspace(int(c)) ? true : false; }
inline bool tIsdigit(char c)																							{ return isdigit(int(c)) ? true : false; }
inline bool tIsbdigit(char c)																							{ return ((c == '0') || (c == '1')) ? true : false; }
inline bool tIsodigit(char c)																							{ return ((c >= '0') && (c <= '7')) ? true : false; }
inline bool tIsxdigit(char c)																							{ return isxdigit(int(c)) ? true : false; }
inline bool tIsalpha(char c)																							{ return isalpha(int(c)) ? true : false; }
inline bool tIscntrl(char c)																							{ return iscntrl(int(c)) ? true : false; }
inline bool tIsalnum(char c)																							{ return isalnum(int(c)) ? true : false; }
inline bool tIsprint(char c)																							{ return isprint(int(c)) ? true : false; }
inline bool tIspunct(char c)																							{ return ispunct(int(c)) ? true : false; }
inline bool tIslower(char c)																							{ return islower(int(c)) ? true : false; }
inline bool tIsupper(char c)																							{ return isupper(int(c)) ? true : false; }
inline bool tIsHexDigit(char d)																							{ return ((d >= 'a' && d <= 'f')||(d >= 'A' && d <= 'F')||(d >= '0' && d <= '9')); }

inline char tChrlwr(char c)																								{ return tIslower(c) ? c : c + ('a' - 'A'); }
inline char tChrupr(char c)																								{ return tIsupper(c) ? c : c - ('a' - 'A'); }
void tStrrev(char* begin, char* end);

struct tDivt																											{ int Quotient; int Remainder; };
tDivt tDiv(int numerator, int denominator);
struct tDiv32t																											{ int32 Quotient; int32 Remainder; };
tDiv32t tDiv32(int32 numerator, int32 denominator);
struct tDivU32t																											{ uint32 Quotient; uint32 Remainder; };
tDivU32t tDivU32(uint32 numerator, uint32 denominator);
struct tDiv64t																											{ int64 Quotient; int64 Remainder; };
tDiv64t tDiv64(int64 numerator, int64 denominator);
struct tDivU64t																											{ uint64 Quotient; uint64 Remainder; };
tDivU64t tDivU64(uint64 numerator, uint64 denominator);

// NAN means not a number. P for positive. N for negative. I for indefinite. S for signaling. Q for quiet.
enum class tFloatType
{
	NORM,							// A normal floating point value (normalized or denormalized). Must be first type.
	FIRST_SPECIAL,
	FIRST_NAN		= FIRST_SPECIAL,
	PSNAN			= FIRST_NAN,	// Positive Signaling Not-A-Number. This must be the first NAN type.
	NSNAN,							// Negative Signaling Not-A-Number.
	PQNAN,							// Positive non-signaling (quiet) Not-A-Number (QNAN).
	NQNAN,							// Negative non-signaling (quiet) Not-A-Number (QNAN). Must be the last NAN type.
	IQNAN,							// Indefinite QNAN. For AMD64 processors only a single NQNAN is used for Indefinite. 
	LAST_NAN		= IQNAN,
	PINF,							// Positive INFinity.
	NINF,							// Negative INFinity.
	LAST_SPECIAL	= NINF
};
tFloatType tGetFloatType(float);	// Single precision get float type.
tFloatType tGetFloatType(double);	// Double precision get float type.
inline bool tIsNAN(float v)																								{ tFloatType t = tGetFloatType(v); return ((int(t) >= int(tFloatType::FIRST_NAN)) && (int(t) <= int(tFloatType::LAST_NAN))) ? true : false; }
inline bool tIsNAN(double v)																							{ tFloatType t = tGetFloatType(v); return ((int(t) >= int(tFloatType::FIRST_NAN)) && (int(t) <= int(tFloatType::LAST_NAN))) ? true : false; }
inline bool tIsSpecial(float v)																							{ tFloatType t = tGetFloatType(v); return ((int(t) >= int(tFloatType::FIRST_SPECIAL)) && (int(t) <= int(tFloatType::LAST_SPECIAL))) ? true : false; }
inline bool tIsSpecial(double v)																						{ tFloatType t = tGetFloatType(v); return ((int(t) >= int(tFloatType::FIRST_SPECIAL)) && (int(t) <= int(tFloatType::LAST_SPECIAL))) ? true : false; }
inline float tFrexp(float arg, int* exp)																				{ return frexpf(arg, exp); }

// Examples of non-NORM float types. These are only examples as in many cases there are multiple bitpatterns for the
// same tFloatType. For example a PSNAN can have any bitpattern between 0x7F800001 and 0x7FBFFFFF (inclusive).
inline float tFloatPSNAN()																								{ union LU { float Nan; uint32 B; } v; v.B = 0x7F800001; return v.Nan; }
inline float tFloatNSNAN()																								{ union LU { float Nan; uint32 B; } v; v.B = 0xFF800001; return v.Nan; }
inline float tFloatPQNAN()																								{ union LU { float Nan; uint32 B; } v; v.B = 0x7FC00000; return v.Nan; }
inline float tFloatIQNAN()																								{ union LU { float Nan; uint32 B; } v; v.B = 0xFFC00000; return v.Nan; }
inline float tFloatNQNAN()																								{ union LU { float Nan; uint32 B; } v; v.B = 0xFFC00001; return v.Nan; }
inline float tFloatPINF()																								{ union LU { float Inf; uint32 B; } v; v.B = 0x7F800000; return v.Inf; }
inline float tFloatNINF()																								{ union LU { float Inf; uint32 B; } v; v.B = 0xFF800000; return v.Inf; }

inline double tDoublePSNAN()																							{ union LU { double Nan; uint64 B; } v; v.B = 0x7FF0000000000001ULL; return v.Nan; }
inline double tDoubleNSNAN()																							{ union LU { double Nan; uint64 B; } v; v.B = 0xFFF0000000000001ULL; return v.Nan; }
inline double tDoublePQNAN()																							{ union LU { double Nan; uint64 B; } v; v.B = 0x7FF8000000000000ULL; return v.Nan; }
inline double tDoubleIQNAN()																							{ union LU { double Nan; uint64 B; } v; v.B = 0xFFF8000000000000ULL; return v.Nan; }
inline double tDoubleNQNAN()																							{ union LU { double Nan; uint64 B; } v; v.B = 0xFFF8000000000001ULL; return v.Nan; }
inline double tDoublePINF()																								{ union LU { double Inf; uint64 B; } v; v.B = 0x7FF0000000000000ULL; return v.Inf; }
inline double tDoubleNINF()																								{ union LU { double Inf; uint64 B; } v; v.B = 0xFFF0000000000000ULL; return v.Inf; }

// These ASCII separators may be used for things like replacing characters in strings for subsequent manipulation.
const char SeparatorSub																									= 26;	// 0x1A
const char SeparatorFile																								= 28;	// 0x1C
const char SeparatorGroup																								= 29;	// 0x1D
const char SeparatorRecord																								= 30;	// 0x1E
const char SeparatorUnit																								= 31;	// 0x1F

const char SeparatorA																									= SeparatorUnit;
const char SeparatorB																									= SeparatorRecord;
const char SeparatorC																									= SeparatorGroup;
const char SeparatorD																									= SeparatorFile;
const char SeparatorE																									= SeparatorSub;

extern const char* SeparatorSubStr;
extern const char* SeparatorFileStr;
extern const char* SeparatorGroupStr;
extern const char* SeparatorRecordStr;
extern const char* SeparatorUnitStr;

extern const char* SeparatorAStr;
extern const char* SeparatorBStr;
extern const char* SeparatorCStr;
extern const char* SeparatorDStr;
extern const char* SeparatorEStr;


}


// Implementation below this line.


inline tStd::tDivt tStd::tDiv(int numerator, int denominator)
{
	div_t d = div(numerator, denominator);
	tDivt r;
	r.Quotient = d.quot;
	r.Remainder = d.rem;
	return r;
}


inline tStd::tDiv32t tStd::tDiv32(int32 numerator, int32 denominator)
{
	div_t d = div(numerator, denominator);
	tDiv32t r;
	r.Quotient = d.quot;
	r.Remainder = d.rem;
	return r;
}


inline tStd::tDivU32t tStd::tDivU32(uint32 numerator, uint32 denominator)
{
	tDivU32t r;
	r.Quotient = numerator/denominator;
	r.Remainder = numerator - r.Quotient*denominator;
	return r;
}


inline tStd::tDiv64t tStd::tDiv64(int64 numerator, int64 denominator)
{
	lldiv_t d = div(numerator, denominator);
	tDiv64t r;
	r.Quotient = d.quot;
	r.Remainder = d.rem;
	return r;
}


inline tStd::tDivU64t tStd::tDivU64(uint64 numerator, uint64 denominator)
{
	tDivU64t r;
	r.Quotient = numerator/denominator;
	r.Remainder = numerator - r.Quotient*denominator;
	return r;
}


template<typename IntegralType> inline IntegralType tStd::tStrtoiT(const char* str, int base)
{
	if (!str || (*str == '\0'))
		return IntegralType(0);

	int len = tStrlen(str);
	const char* start = str;
	const char* end = str + len - 1;

	if ((base < 2) || (base > 36))
		base = -1;

	if (base == -1)
	{
		// Base is -1. Need to determine based on prefix.
		if ((len > 1) && (*start == '0'))
			start++;

		if ((*start == 'x') || (*start == 'X') || (*start == '#'))
			base = 16;
		else if ((*start == 'd') || (*start == 'D'))
			base = 10;
		else if ((*start == 'o') || (*start == 'O') || (*start == '@'))
			base = 8;
		else if ((*start == 'n') || (*start == 'N'))
			base = 4;
		else if ((*start == 'b') || (*start == 'B') || (*start == '!'))
			base = 2;

		if (base == -1)
			base = 10;
		else
			start++;
	}

	IntegralType val = 0;
	IntegralType colVal = 1;
	for (const char* curr = end; curr >= start; curr--)
	{
		if ((*curr == '-') && (base == 10))
		{
			val = -val;
			continue;
		}

		int digVal = -1;
		if (*curr >= '0' && *curr <= '9')
			digVal = *curr - '0';
		else if (*curr >= 'a' && *curr <= 'z')
			digVal = 10 + (*curr - 'a');
		else if (*curr >= 'A' && *curr <= 'Z')
			digVal = 10 + (*curr - 'A');

		if ((digVal == -1) || (digVal > base-1))
			continue;

		val += IntegralType(digVal)*colVal;
		colVal *= IntegralType(base);
	}

	return val;
}


template<typename IntegralType> inline bool tStd::tItostrT(IntegralType value, char* str, int strSize, int base)
{
	if (!str || strSize <= 0)
		return false;

	static char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// Validate base.
	if ((base < 2) || (base > sizeof(digits)/sizeof(*digits)))
		return false;

	// Take care of sign.
	IntegralType sign = value;
	if (value < 0)
		value = -value;

	// Conversion. Number is reversed.
	char* s = str;
	int numWritten = 0;
	int remainder;
	IntegralType quotient;
	do
	{
		remainder = value % base;
		tAssert((remainder < base) && (remainder >= 0));
		quotient = value / base;
		if (numWritten < strSize)
			*s++ = digits[remainder];
		else
			return false;
		numWritten++;
	} while ((value = quotient));

	if (sign < 0)
	{
		if (numWritten < strSize)
			*s++='-';
		else
			return false;
	}

	if (numWritten < strSize)
		*s = '\0';
	else
		return false;

	tStrrev(str, s-1);
	return true;
}


inline tStd::tFloatType tStd::tGetFloatType(float v)
{
	uint32 b = *((uint32*)&v);
		
	if ((b >= 0x7F800001) && (b <= 0x7FBFFFFF))
		return tFloatType::PSNAN;

	if ((b >= 0xFF800001) && (b <= 0xFFBFFFFF))
		return tFloatType::NSNAN;

	if ((b >= 0x7FC00000) && (b <= 0x7FFFFFFF))
		return tFloatType::PQNAN;

	if (b == 0xFFC00000)
		return tFloatType::IQNAN;

	if ((b >= 0xFFC00001) && (b <= 0xFFFFFFFF))
		return tFloatType::NQNAN;

	if (b == 0x7F800000)
		return tFloatType::PINF;

	if (b == 0xFF800000)
		return tFloatType::NINF;

	return tFloatType::NORM;
}


inline tStd::tFloatType tStd::tGetFloatType(double v)
{
	uint64 b = *((uint64*)&v);

	if ((b >= 0x7FF0000000000001ULL) && (b <= 0x7FF7FFFFFFFFFFFFULL))
		return tFloatType::PSNAN;

	if ((b >= 0xFFF0000000000001ULL) && (b <= 0xFFF7FFFFFFFFFFFFULL))
		return tFloatType::NSNAN;

	if ((b >= 0x7FF8000000000000ULL) && (b <= 0x7FFFFFFFFFFFFFFFULL))
		return tFloatType::PQNAN;

	if (b == 0xFFF8000000000000ULL)
		return tFloatType::IQNAN;

	if ((b >= 0xFFF8000000000001ULL) && (b <= 0xFFFFFFFFFFFFFFFFULL))
		return tFloatType::NQNAN;

	if (b == 0x7FF0000000000000ULL)
		return tFloatType::PINF;

	if (b == 0xFFF0000000000000ULL)
		return tFloatType::NINF;

	return tFloatType::NORM;
}
