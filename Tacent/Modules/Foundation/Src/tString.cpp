// tString.cpp
//
// tString is a simple and readable string class that implements sensible operators, including implicit casts. There is
// no UCS2 or UTF16 support since UTF8 is, in my opinion, superior and the way forward. tStrings will work with UTF8.
// You cannot stream (from cin etc) more than 512 chars into a string. This restriction is only for wacky << streaming.
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

#include "Foundation/tString.h"
#include "Foundation/tStandard.h"


char tString::EmptyChar = '\0';


tString tString::Prefix(const char c) const
{
	int pos = FindChar(c);
	if (pos == -1)
		return *this;
	if (pos == 0)
		return tString();

	// Remember, this zeros the memory, so tStrncpy not dealing with the terminating null is ok.
	tString buf(pos);
	tStd::tStrncpy(buf.TextData, TextData, pos);
	return buf;
}


tString tString::Suffix(const char c) const
{
	int pos = FindChar(c, true);
	if (pos == -1)
		return *this;

	int length = Length();
	if (pos == (length-1))
		return tString();

	// Remember, this zeros the memory, so tStrncpy not dealing with the terminating null is ok.
	tString buf(length - 1 - pos);
	tStd::tStrncpy(buf.TextData, TextData + pos + 1, length - 1 - pos);
	return buf;
}


tString tString::Prefix(int i) const
{
	if (i > Length())
		return tString();

	// Remember, this zeros the memory, so tStrncpy not dealing with the terminating null is ok.
	tString buf(i);
	tStd::tStrncpy(buf.TextData, TextData, i);
	return buf;
}


tString tString::Suffix(int i) const
{
	int length = Length();
	if (i > length)
		return tString();

	// Remember, this zeros the memory, so tStrncpy not dealing with the terminating null is ok.
	tString buf( length - 1 - i );
	tStd::tStrncpy(buf.TextData, TextData + i + 1, length - 1 - i );
	return buf;
}


tString tString::ExtractPrefix(int i)
{
	tAssert(i >= 0)
	int length = Length();
	if ((i == 0) || (i > length))
		return tString();
	
	tString prefix = Prefix(i);

	int newLength = length - i;
	if (newLength == 0)
	{
		if (TextData != &EmptyChar)
			delete[] TextData;
		TextData = &EmptyChar;
		return prefix;
	}

	char* newText = new char[newLength+1];
	strcpy(newText, TextData+i);

	if (TextData != &EmptyChar)
		delete[] TextData;
	TextData = newText;

	return prefix;
}


tString tString::ExtractSuffix(int i)
{
	tAssert(i >= 0);
	int length = Length();
	if ((i == 0) || (i > length))
		return tString();
	
	tString suffix = Suffix(i);

	int newLength = length - i;
	if (newLength == 0)
	{
		// It couldn't have been empty before.
		delete TextData;
		TextData = &EmptyChar;
		return suffix;
	}

	char* newText = new char[newLength+1];
	TextData[length - i] = '\0';

	tStd::tStrcpy(newText, TextData);

	if (TextData != &EmptyChar)
		delete[] TextData;
	TextData = newText;

	return suffix;
}


tString tString::ExtractFirstWord(const char divider)
{
	int pos = FindChar(divider);
	if (pos == -1)
	{
		tString buf(Text());
		Clear();
		return buf;
	}

	// Remember, this constructor zeros the memory, so strncpy not dealing with the terminating null is ok.
	tString buf(pos);
	tStd::tStrncpy(buf.TextData, TextData, pos);

	int length = Length();
	char* newText = new char[length-pos];

	// This will append the null.
	tStd::tStrncpy(newText, TextData+pos+1, length-pos);

	if (TextData != &EmptyChar)
		delete[] TextData;
	TextData = newText;

	return buf;
}


tString tString::ExtractLastWord(const char divider)
{
	int pos = FindChar(divider, true);
	if (pos == -1)
	{
		tString buf(Text());
		Clear();
		return buf;
	}

	int wordLength = Length() - pos - 1;

	// Remember, this constructor zeros the memory, so strncpy not dealing with the terminating null is ok.
	tString buf(wordLength);
	tStd::tStrncpy(buf.TextData, TextData+pos+1, wordLength);

	char* newText = new char[pos+1];
	tStd::tStrncpy(newText, TextData, pos);
	newText[pos] = '\0';

	if (TextData != &EmptyChar)
		delete[] TextData;
	TextData = newText;

	return buf;
}


int tString::Replace(const char* s, const char* r)
{
	if (!s || (s[0] == '\0'))
		return 0;

	int origTextLength = tStd::tStrlen(TextData);
	int searchStringLength = tStd::tStrlen(s);
	int replaceStringLength = r ? tStd::tStrlen(r) : 0;
	int replaceCount = 0;

	if (searchStringLength != replaceStringLength)
	{
		// Since the replacement string is a different size, we'll need to reallocate
		// out memory. We start by finding out how many replacements we will need to do.
		char* searchStart = TextData;

		while (searchStart < (TextData + origTextLength))
		{
			char* foundString = tStd::tStrstr(searchStart, s);
			if (!foundString)
				break;

			replaceCount++;
			searchStart = foundString + searchStringLength;
		}

		// The new length may be bigger or smaller than the original. If the newlength is precisely
		// 0, it means that the entire string is being replaced with nothing, so we can exit early.
		// Ex. Replace "abcd" in "abcdabcd" with ""
		int newTextLength = origTextLength + replaceCount*(replaceStringLength - searchStringLength);
		if (!newTextLength)
		{
			if (TextData != &EmptyChar)
				delete[] TextData;
			TextData = &EmptyChar;
			return replaceCount;
		}

		char* newText = new char[newTextLength + 16];
		newText[newTextLength] = '\0';

		tStd::tMemset( newText, 0, newTextLength + 16 );

		int newTextWritePos = 0;

		searchStart = TextData;
		while (searchStart < (TextData + origTextLength))
		{
			char* foundString = tStd::tStrstr(searchStart, s);

			if (foundString)
			{
				tStd::tMemcpy(newText+newTextWritePos, searchStart, int(foundString-searchStart));
				newTextWritePos += int(foundString-searchStart);

				tStd::tMemcpy(newText+newTextWritePos, r, replaceStringLength);
				newTextWritePos += replaceStringLength;
			}
			else
			{
				tStd::tStrcpy(newText+newTextWritePos, searchStart);
				break;
			}

			searchStart = foundString + searchStringLength;
		}

		if (TextData != &EmptyChar)
			delete[] TextData;
		TextData = newText;
	}
	else
	{
		// In this case the replacement string is exactly the same length at the search string.
		// Much easier to deal with and no need for memory allocation.
		char* searchStart = TextData;

		while (searchStart < (TextData + origTextLength))
		{
			char* foundString = tStd::tStrstr(searchStart, s);
			if (foundString)
			{
				tStd::tMemcpy(foundString, r, replaceStringLength);
				replaceCount++;
			}
			else
			{
				break;
			}

			searchStart = foundString + searchStringLength;
		}
	}

	return replaceCount;
}


int tString::Remove(const char c)
{
	int destIndex = 0;
	int numRemoved = 0;

	// This operation can be done in place.
	for (int i = 0; i < Length(); i++)
	{
		if (TextData[i] != c)
		{
			TextData[destIndex] = TextData[i];
			destIndex++;
		}
		else
		{
			numRemoved++;
		}
	}
	TextData[destIndex] = '\0';

	return numRemoved;
}


int tString::RemoveLeading(const char* removeThese)
{
	if (!TextData || (TextData == &EmptyChar) || !removeThese)
		return 0;

	int cnt = 0;
	while (TextData[cnt])
	{
		bool matches = false;
		int j = 0;
		while (removeThese[j] && !matches)
		{
			if (removeThese[j] == TextData[cnt])
				matches = true;
			j++;
		}
		if (matches) 		
			cnt++;
		else
			break;
	}

	if (cnt > 0)
	{
		int oldlen = Length();
		char* newtext = new char[oldlen-cnt+1];
		tStd::tStrcpy(newtext, &TextData[cnt]);
		TextData = newtext;
	}

	return cnt;
}


int tString::RemoveTrailing(const char* removeThese)
{
	if (!TextData || (TextData == &EmptyChar) || !removeThese)
		return 0;

	int oldlen = Length();
			
	int i = oldlen - 1;
	while (i > -1)
	{
		bool matches = false;
		int j = 0;
		while (removeThese[j] && !matches)
		{
			if (removeThese[j] == TextData[i])
				matches = true;
			j++;
		}
		if (matches) 		
			i--;
		else
			break;
	}
	int numRemoved = oldlen - i;
	TextData[i+1] = '\0';

	return numRemoved;
}


int tStd::tExplode(tList<tStringItem>& components, const tString& src, char divider)
{
	tString source = src;
	int startCount = components.GetNumItems();
	while (source.FindChar(divider) != -1)
	{
		tString component = source.ExtractFirstWord(divider);
		components.Append(new tStringItem(component));
	}

	// If there's anything left in source we need to add it.
	if (!source.IsEmpty())
		components.Append(new tStringItem(source));

	return components.GetNumItems() - startCount;
}


int tStd::tExplode(tList<tStringItem>& components, const tString& src, const tString& divider)
{
	// Well, this is a bit of a cheezy way of doing this.  We just assume that ASCII character 31,
	// the "unit separator", is meant for this kind of thing and not otherwise present in the src string.
	tString source = src;
	char sep[2];
	sep[0] = 31;
	sep[1] = 0;
	source.Replace(divider, sep);
	return tExplode(components, source, 31);
}
