// tRegex.h
//
// Simple regular expression class. The code is based on TRex but has been significantly modified. The original license
// follows:
//
// Copyright (c) 2003-2006 Alberto Demichelis
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held
// liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to
// alter it and redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
// If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is
// not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original
// software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
// To be absolutely clear, the tRegex class found here is an 'altered source' version of the original. The alterations
// are under the following license:
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
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
#include "Foundation/tString.h"
namespace tSystem
{


// The format of the regular expression pattern strings is fairly standard. The following is supported:
//
// Expressions:
//		\		Quote the next meta-character.
//		^		Match the beginning of the string.
//		.		Match any character.
//		$		Match the end of the string.
//		|		Alternation.
//		()		Grouping (creates a capture).
//		[]		Character class.
//
// Greedy Closures:
//		*		Match 0 or more times.
//		+		Match 1 or more times.
//		?		Match 1 or 0 times.
//		{n}	Match exactly n times.
//		{n,}	Match at least n times.
//		{n,m}	Match at least n but not more than m times.
//
// Escape Characters.
//		\t		Tab (HT, TAB)
//		\n		Newline (LF, NL)
//		\r		Return (CR)
//		\f		Form feed (FF)
//
// Predefined Classes:
//		\l		Lowercase next char.
//		\u		Uppercase next char.
//		\a		Letters.
//		\A		Non-letters.
//		\w		Alphanumeric [0-9a-zA-Z].
//		\W		Non-alphanumeric.
//		\s		Space.
//		\S		Non-space.
//		\d		Digits.
//		\D		Non-digits.
//		\x		Hexadecimal digits.
//		\X		Non-hexadecimal digits.
//		\c		Control characters.
//		\C		Non-control characters.
//		\p		Punctuation.
//		\P		Non-punctuation.
//		\b		Word boundary.
//		\B		Non-word boundary.
class tRegex
{
public:
	tRegex()																											: Pattern(nullptr), Nodes(nullptr), Matches(nullptr) { Clear(); }
	tRegex(const tString& pattern)																						: Pattern(nullptr), Nodes(nullptr), Matches(nullptr) { Clear(); Compile(pattern); }
	tRegex(const char* pattern)																							: Pattern(nullptr), Nodes(nullptr), Matches(nullptr) { Clear(); Compile(pattern); }
	~tRegex()																											{ Clear(); }

	// Compiles a regular expression (described above). Any previously compiled expression is lost.
	void Compile(const tString& pattern);
	bool IsMatch(const tString& text) const																				{ return IsMatch(text.ConstText()); }
	void Compile(const char* pattern);
	bool IsMatch(const char* text) const;					// Returns true is a perfect match is attained.
	bool IsValid() const																								{ return Pattern ? true : false; }
	void Clear();

	// Returns the number of sub-expressions for the compiled pattern. If all expressions match a test pattern, this is
	// how many matches will be returned. For example, (a)(b) will return 3. The 3 are (a)(b), (a), and (b). You do not
	// need a perfect match from IsMatch for this to happen.
	int GetNumSubExpressions() const																					{ return IsValid() ? NumSubExpr : 0; }
	struct Match : public tLink<Match>
	{
		Match()																											: IndexStart(0), Length(0) { }
		Match(const Match& src)																							: IndexStart(src.IndexStart), Length(src.Length) { }
		Match(int startIndex, int length)																				: IndexStart(startIndex), Length(length) { }
		bool IsValid() const																							{ return (Length > 0) ? true : false; }
		tString GetString(const tString& text) const																	{ if (!IsValid()) return tString(); tString m(Length); tStd::tStrncpy(m.Text(), text.ConstText()+IndexStart, Length); return m; }

		int IndexStart;
		int Length;
	};

	// These populate the supplied list of matches. If no matches are appended to the list that means none were found.
	// The end pointer should be one past the last valid character to check.
	void Search(const char* begin, const char* end, tList<Match>&) const;
	void Search(const char* text, tList<Match>& matches) const															{ Search(text, text + tStd::tStrlen(text), matches); }
	void Search(const tString& text, tList<Match>& matches) const														{ Search(text.ConstText(), matches); }

private:
	struct Node
	{
		// These members may be an operator or the actual character. That's why they are ints and not tOperators.
		int Type;
		int Left;
		int Right;
		int Next;
	};

	struct MatchInternal
	{
		const char* Begin;
		int Length;
	};

	void CompileInternal();

	int NewNode(int type);
	int ListRec();
	void Expect(int n);
	char EscapeChar();
	int CharClass(int classid);
	int CharNode(bool isclass);
	int Class();
	int ParseNumber();
	int Element();
	static bool MatchCClass(int cclass, char c);
	bool MatchClass(const Node*, char c) const;
	const char* MatchNode(const Node*, const char* str, const Node* next) const;

	char* Pattern;											// Owned by this object.
	mutable const char* EOL;								// End of line.
	mutable const char* BOL;								// Beginning of line.
	const char* Curr;
	int First;
	Node* Nodes;
	int NumNodesAllocated;
	int NumNodes;
	int NumSubExpr;
	MatchInternal* Matches;
	mutable int CurrSubExpr;
};


}
