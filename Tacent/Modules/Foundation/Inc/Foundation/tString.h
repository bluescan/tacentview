// tString.h
//
// tString is a simple and readable string class that implements sensible operators, including implicit casts. There is
// no UCS2 or UTF16 support since UTF8 is, in my opinion, superior and the way forward. tStrings will work with UTF8.
// You cannot stream (from cin etc) more than 512 chars into a string. This restriction is only for wacky << streaming.
//
// Copyright (c) 2004-2006, 2015, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include "Foundation/tStandard.h"
#include "Foundation/tList.h"


struct tString
{
	tString()																											{ TextData = &EmptyChar; }
	tString(const tString&);

	// Construct a string with enough room for length characters. Length+1 characters are reserved to make room for the
	// null terminator. The reserved space is zeroed.
	explicit tString(int length);
	tString(const char*);
	tString(char);
	virtual ~tString();

	tString& operator=(const tString&);
	bool operator==(const tString& s) const																				{ return( !tStd::tStrcmp(TextData, s.TextData) ); }
	bool operator==(const char* s) const																				{ return( !tStd::tStrcmp(TextData, s) ); }
	bool operator!=(const tString& s) const																				{ return( !!tStd::tStrcmp(TextData, s.TextData) ); }
	bool operator!=(const char* s) const																				{ return( !!tStd::tStrcmp(TextData, s) ); }

	bool IsEqual(const tString& s) const																				{ return( !tStd::tStrcmp(TextData, s.TextData) ); }
	bool IsEqual(const char* s) const																					{ return( !tStd::tStrcmp(TextData, s) ); }
	bool IsEqualCI(const tString& s) const																				{ return( !tStd::tStricmp(TextData, s.TextData) ); }
	bool IsEqualCI(const char* s) const																					{ return( !tStd::tStricmp(TextData, s) ); }

	operator const char*()																								{ return TextData; }
	operator const char*() const																						{ return TextData; }
	char& operator[](int i)																								{ return TextData[i]; }
	friend tString operator+(const tString& prefix, const tString& suffix);
	tString& operator+=(const tString&);

	void Set(const char*);
	int Length() const																									{ return int(tStd::tStrlen(TextData)); }
	bool IsEmpty() const																								{ return (TextData == &EmptyChar) || !tStd::tStrlen(TextData); }
	void Clear()																										{ if (TextData != &EmptyChar) delete[] TextData; TextData = &EmptyChar; }

	bool IsAlphabetic(bool includeUnderscore = true) const;
	bool IsNumeric(bool includeDecimal = false) const;
	bool IsAlphaNumeric(bool includeUnderscore = true, bool includeDecimal = false) const;

	// Current string data is lost and enough space is reserved for length characters. The reserved memory is zeroed.
	void Reserve(int length);

	// Returns a tString of the characters before the first marker. Returns the entire string if marker was not found.
	tString Prefix(const char marker) const;
	tString Suffix(const char marker) const;				// Same as Prefix but chars after last marker.
	
	// Returns a tString of the first count chars. If there are not enough characters, an empty string is returned.
	tString Prefix(int count) const;
	tString Suffix(int count) const;						// Same as Prefix but returns last count chars.

	// Returns a tString of the first count chars. Removes these from the current string. If the count is greater than
	// the string length nothing is extracted.
	tString ExtractPrefix(int count);

	// Returns a tString of the last count chars. Removes these from the current string. If the count is greater than
	// the string length nothing is extracted.
	tString ExtractSuffix(int count);

	// Extracts first word up to and not including first divider encountered. The tString is left with the remainder,
	// not including the divider. If divider isn't found, the entire string is returned and the tString is left empty.
	tString ExtractFirstWord(const char divider = ' ');

	// Extracts word after last divider. The tString is left with the remainder, not including the divider. If the
	// divider isn't found, the entire string is returned and the tString is left empty.
	tString ExtractLastWord(const char divider = ' ');

	char* Text()																										{ return TextData; }
	const char* ConstText() const																						{ return TextData; }

	// Returns POD representation (Plain Old Data). For use with tPrintf and %s.
	const char* Pod() const																								{ return TextData; }

	// Returns index of first/last occurrence of char in the string. -1 if not found. Finds last if backwards flag is
	// set. The starting point may be specified. If backwards is false, the search proceeds forwards from the starting
	// point. If backwards is true, it proceeds backwards. If startIndex is -1, 0 is the starting point for a forward
	// search and length-1 is the starting point for a backwards search.
	int FindChar(const char, bool backwards = false, int startIndex = -1) const;

	// Returns the index of the first character in the tString that is also somewhere in the null-terminated string
	// searchChars. Returns -1 if none of them match.
	int FindAny(const char* searchChars) const;

	// Returns index of first character of the string s in the string. Returns -1 if not found.
	int FindString(const char* s, int startIndex = 0) const;

	// Replace all occurrences of character c with character r. Returns number of characters replaced.
	int Replace(const char c, const char r);

	// Replace all occurrences of string search with string replace. Returns the number of replacements. The replacement
	// is done in a forward direction. If replace is a different size than search, memory will be managed to accomadate
	// the larger or smaller resulting string and keep the memory footprint as small as possible. If they are the same
	// size, the function is faster and doesn't need to mess with memory. If replace is "" or 0, all occurrences of
	// search will be removed (replaced by nothing).
	int Replace(const char* search, const char* replace);

	// Remove all occurrences of the character rem. Returns the number of characters removed.
	int Remove(const char rem);

	// Removing a string simply calls Replace with a null second string. Returns how many rem strings were removed.
	int Remove(const char* rem)																							{ return Replace(rem, nullptr); }

	int RemoveLeading(const char* removeThese);				// removeThese is a null-terminated list of chars to remove.
	int RemoveTrailing(const char* removeThese);			// removeThese is a null-terminated list of chars to remove.
	int CountChar(char c) const;							// Counts the number of occurrences of c.

	void ToUpper()																										{ tStd::tStrupr(TextData); }
	void ToLower()																										{ tStd::tStrlwr(TextData); }

	// UpCase and LowCase both modify the object as well as return a reference to it. Returning a reference makes it
	// easy to string together expressions such as: if (name.LowCase() == "ah")
	tString& UpCase()																									{ ToUpper(); return *this; }
	tString& LowCase()																									{ ToLower(); return *this; }

	// See comment for tStrtoiT in tStandard.h for format requirements. The summary is that if base is -1, the function
	// looks one of the following prefixes in the string, defaulting to base 10 if none found.
	//
	// Base 16 prefixes: x X 0x 0X #
	// Base 10 prefixes: d D 0d 0D
	// Base 8  prefixes: o O 0o 0O @
	// Base 2  prefixes: b B 0b 0B
	int GetAsInt(int base = -1) const																					{ return GetAsInt32(base); }
	int32 GetAsInt32(int base = -1) const																				{ return tStd::tStrtoi32(TextData, base); }
	int64 GetAsInt64(int base = -1) const																				{ return tStd::tStrtoi64(TextData, base); }
	uint GetAsUInt(int base = -1) const																					{ return GetAsUInt32(base); }
	uint32 GetAsUInt32(int base = -1) const																				{ return tStd::tStrtoui32(TextData, base); }
	uint64 GetAsUInt64(int base = -1) const																				{ return tStd::tStrtoui64(TextData, base); }

	// Case insensitive. Interprets "true", "t", "yes", "y", "on", "enable", "enabled", "1", "+", and strings that
	// represent non-zero integers as boolean true. Otherwise false.
	bool GetAsBool() const																								{ return tStd::tStrtob(TextData); }

	float GetAsFloat() const									/* Base 10 interpretation only. */						{ return tStd::tStrtof(TextData); }
	double GetAsDouble() const									/* Base 10 interpretation only. */						{ return tStd::tStrtod(TextData); }

	// Shorter synonyms.
	int AsInt(int base = -1) const																						{ return GetAsInt(base); }
	int AsInt32(int base = -1) const																					{ return GetAsInt32(base); }
	int64 AsInt64(int base = -1) const																					{ return GetAsInt64(base); }
	uint AsUInt(int base = -1) const																					{ return GetAsUInt(base); }
	uint AsUInt32(int base = -1) const																					{ return GetAsUInt32(base); }
	uint64 AsUInt64(int base = -1) const																				{ return GetAsUInt64(base); }
	bool AsBool() const																									{ return GetAsBool(); }
	float AsFloat() const																								{ return GetAsFloat(); }

protected:
	char* TextData;
	static char EmptyChar;										// All empty strings can use this.
};


// The tStringItem class is just the tString class except they can be placed on tLists.
struct tStringItem : public tLink<tStringItem>, public tString
{
public:
	tStringItem()																										: tString() { }

	// The tStringItem copy cons is missing, because as a list item can only be on one list at a time.
	tStringItem(const tString& s)																						: tString(s) { }
	tStringItem(int length)																								: tString(length) { }
	tStringItem(const char* c)																							: tString(c) { }
	tStringItem(char c)																									: tString(c) { }

	// This call does NOT change the list that the tStringItem is on. The link remains unmodified.
	tStringItem& operator=(const tStringItem&);
};


// Some utility functions that act on strings.
namespace tStd
{
	// Separates the src string into components based on the divider. If src was "abc_def_ghi", components will get
	// "abc", "def", and "ghi" appended to it. Returns the number of components appended to the components list. The
	// version that takes the string divider allows for multicharacter dividers. Note that "abc__def_ghi" will explode
	// to "abc", "", "def", and "ghi". Empty strings are preserved allowing things like exploding text files based on
	// linefeeds. You'll get one item per line even if the line only contains a linefeed.
	int tExplode(tList<tStringItem>& components, const tString& src, char divider = '_');
	int tExplode(tList<tStringItem>& components, const tString& src, const tString& divider);
}


// Implementation below this line.


inline tString::tString(const char* t)
{
	if (t)
	{
		int len = int(tStd::tStrlen(t));
		if (len > 0)
		{
			TextData = new char[1 + len];
			tStd::tStrcpy(TextData, t);
			return;
		}
	}

	TextData = &EmptyChar;
}


inline tString::tString(const tString& s)
{
	TextData = new char[1 + tStd::tStrlen(s.TextData)];
	tStd::tStrcpy(TextData, s.TextData);
}


inline tString::tString(char c)
{
	TextData = new char[2];
	TextData[0] = c;
	TextData[1] = '\0';
}


inline tString::tString(int length)
{
	if (!length)
	{
		TextData = &EmptyChar;
	}
	else
	{
		TextData = new char[1+length];
		tStd::tMemset(TextData, 0, 1+length);
	}
}


inline void tString::Reserve(int length)
{
	if (TextData != &EmptyChar)
		delete[] TextData;

	if (length <= 0)
	{
		TextData = &EmptyChar;
		return;
	}

	TextData = new char[length+1];
	tStd::tMemset(TextData, 0, length+1);
}


inline int tString::CountChar(char c) const
{
	char* i = TextData;
	int count = 0;
	while (*i != '\0')
		count += (*i++ == c) ? 1 : 0;

	return count;
}


inline void tString::Set(const char* s)
{
	Clear();
	if (!s)
		return;

	int len = tStd::tStrlen(s);
	if (len <= 0)
		return;

	TextData = new char[1 + len];
	tStd::tStrcpy(TextData, s);
}


inline tString& tString::operator=(const tString& src)
{
	if (this == &src)
		return *this;

	if (TextData != &EmptyChar)
		delete[] TextData;

	TextData = new char[1 + src.Length()];
	tStd::tStrcpy(TextData, src.TextData);
	return *this;
}


inline tString operator+(const tString& preStr, const tString& sufStr)
{
	tString buf( preStr.Length() + sufStr.Length() );
	tStd::tStrcpy(buf.TextData, preStr.TextData);
	tStd::tStrcpy(buf.TextData + preStr.Length(), sufStr.TextData);

	return buf;
}


inline tString& tString::operator+=(const tString& sufStr)
{
	if (sufStr.IsEmpty())
		return *this;
	else
	{
		char* newTextData = new char[ Length() + sufStr.Length() + 1 ];
		tStd::tStrcpy(newTextData, TextData);
		tStd::tStrcpy(newTextData + Length(), sufStr.TextData);

		if (TextData != &EmptyChar)
			delete[] TextData;

		TextData = newTextData;
		return *this;
	}
}


inline bool tString::IsAlphabetic(bool includeUnderscore) const 
{
	if (TextData == &EmptyChar)
		return false;

	const char* c = TextData;
	while (*c)
	{
		if ( !((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (includeUnderscore && *c == '_')) )
			return false;
		c++;
	}

	return true;
}


inline bool tString::IsNumeric(bool includeDecimal) const 
{
	if (TextData == &EmptyChar)
		return false;

	const char* c = TextData;
	while (*c)
	{
		if ( !((*c >= '0' && *c <= '9') || (includeDecimal && *c == '.')) )
			return false;
		c++;
	}

	return true;
}


inline bool tString::IsAlphaNumeric(bool includeUnderscore, bool includeDecimal) const
{
	return (IsAlphabetic(includeUnderscore) || IsNumeric(includeDecimal));
}


inline int tString::FindAny(const char* chars) const
{
	if (TextData == &EmptyChar)
		return -1;
	
	int i = 0;
	while (TextData[i])
	{
		int j = 0;
		while (chars[j])
		{
			if (chars[j] == TextData[i])
				return i;
			j++;
		}
		i++;
	}
	return -1;
}


inline int tString::FindChar(const char c, bool reverse, int start) const
{
	const char* pc = nullptr;

	if (start == -1)
	{
		if (reverse)
			start = Length() - 1;
		else
			start = 0;
	}

	if (reverse)
	{
		for (int i = start; i >= 0; i--)
			if (TextData[i] == c)
			{
				pc = TextData + i;
				break;
			}
	}
	else
		pc = tStd::tStrchr(&TextData[start], c);

	if (!pc)
		return -1;

	// Returns the index.
	return int(pc - TextData);
}


inline int tString::FindString(const char* s, int start) const
{
	int len = Length();
	if (!len)
		return -1;

	tAssert((start >= 0) && (start < Length()));
	const char* found = tStd::tStrstr(&TextData[start], s);
	if (found)
		return int(found - TextData);

	return -1;
}


inline int tString::Replace(const char c, const char r)
{
	int numReplaced = 0;
	for (int i = 0; i < Length(); i++)
	{
		if (TextData[i] == c)
		{
			numReplaced++;
			TextData[i] = r;
		}
	}

	return numReplaced;
}


inline tString::~tString()
{
	if (TextData != &EmptyChar)
		delete[] TextData;
}


inline tStringItem& tStringItem::operator=(const tStringItem& src)
{
	if (this == &src)
		return *this;

	if (TextData != &EmptyChar)
		delete[] TextData;

	TextData = new char[1 + src.Length()];
	tStd::tStrcpy(TextData, src.TextData);
	return *this;
}
