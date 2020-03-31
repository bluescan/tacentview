// tRegex.cpp
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

#include <Foundation/tMemory.h>
#include "System/tThrow.h"
#include "System/tRegex.h"
#include "System/tPrint.h"
using namespace tStd;
using namespace tMem;
//#define REGEX_DEBUG_PRINT
namespace tSystem
{


enum tOperator
{
	// Invalid is 255 on purpose.
	tOperator_Invalid										= tCharInvalid,
	tOperator_First,
	tOperator_Greedy										= tOperator_First,
	tOperator_Or,
	tOperator_Expr,
	tOperator_NoCapExpr,
	tOperator_Dot,
	tOperator_Class,
	tOperator_CClass,
	tOperator_NClass,
	tOperator_Range,
	tOperator_Char,
	tOperator_EOL,
	tOperator_BOL,
	tOperator_WB,
	tOperator_NumOperators									= tOperator_WB - tOperator_Invalid
};


#ifdef REGEX_DEBUG_PRINT
static const char* OperatorNames[tOperator_NumOperators] =
{
	"Greedy",
	"Or",
	"Expr",
	"NoCapExpr",
	"Dot",
	"Class",
	"CClass",
	"NClass",
	"Range",
	"Char",
	"EOL",
	"BOL",
	"WB"
};
#endif


const static char iSymbol_AnyChar							= '.';
const static char iSymbol_GreedyOneOrMore					= '+';
const static char iSymbol_GreedyZeroOrMore					= '*';
const static char iSymbol_GreedyZeroOrOne					= '?';
const static char iSymbol_Branch							= '|';
const static char iSymbol_StringEnd							= '$';
const static char iSymbol_StringBegin						= '^';
const static char iSymbol_Escape							= '\\';


int tRegex::NewNode(int type)
{
	tRegex::Node n;
	n.Type = type;
	n.Next = n.Right = n.Left = -1;

	if (type == tOperator_Expr)
		n.Right = NumSubExpr++;

	if (NumNodesAllocated < (NumNodes + 1))
	{
		int origSize = NumNodesAllocated * sizeof(tRegex::Node);
		NumNodesAllocated *= 2;
		tRegex::Node* newNodes = (tRegex::Node*)tMalloc(NumNodesAllocated * sizeof(tRegex::Node));
		if (Nodes)
		{
			tMemcpy(newNodes, Nodes, origSize);
			tFree(Nodes);
		}
		Nodes = newNodes;
	}

	Nodes[NumNodes++] = n;
	int newid = NumNodes - 1;
	return newid;
}


void tRegex::Expect(int n)
{
	if ((*Curr) != n) 
		throw tError("Expected bracket.");

	Curr++;
}


char tRegex::EscapeChar()
{
	if (*Curr == iSymbol_Escape)
	{
		Curr++;
		switch (*Curr)
		{
			case 'v':
				Curr++;
				return '\v';

			case 'n':
				Curr++;
				return '\n';

			case 't':
				Curr++;
				return '\t';

			case 'r':
				Curr++;
				return '\r';

			case 'f':
				Curr++;
				return '\f';

			default:
				return (*Curr++);
		}
	}
	else if (!tIsprint(*Curr))
		throw tError("Letter expected.");

	return (*Curr++);
}


int tRegex::CharClass(int classid)
{
	int n = NewNode(tOperator_CClass);
	Nodes[n].Left = classid;
	return n;
}


int tRegex::CharNode(bool isclass)
{
	if (*Curr == iSymbol_Escape)
	{
		Curr++;
		switch (*Curr)
		{
			case 'n':
				Curr++;
				return NewNode('\n');

			case 't':
				Curr++;
				return NewNode('\t');

			case 'r':
				Curr++;
				return NewNode('\r');

			case 'f':
				Curr++;
				return NewNode('\f');

			case 'v':
				Curr++;
				return NewNode('\v');

			case 'a': case 'A': case 'w': case 'W':
			case 's': case 'S': case 'd': case 'D':
			case 'x': case 'X': case 'c': case 'C':
			case 'p': case 'P': case 'l': case 'u':
			{
				char t = *Curr;
				Curr++;
				return CharClass(t);
			}

			case 'b':
			case 'B':
				if (!isclass)
				{
					int node = NewNode(tOperator_WB);
					Nodes[node].Left = *Curr;
					Curr++; 
					return node;
				}

			default:
			{
				char t = *Curr;
				Curr++;
				return NewNode(t);
			}
		}
	}
	else if (!tIsprint(*Curr))
	{
		throw tError("Letter expected.");
	}

	char t = *Curr;
	Curr++; 
	return NewNode(t);
}


int tRegex::Class()
{
	int ret = -1;
	if (*Curr == iSymbol_StringBegin)
	{
		ret = NewNode(tOperator_NClass);
		Curr++;
	}
	else
		ret = NewNode(tOperator_Class);

	if (*Curr == ']')
		throw tError("Empty class.");

	int first = -1;
	int chain = ret;
	while (*Curr != ']' && Curr != EOL)
	{
		if (*Curr == '-' && first != -1)
		{
			int r;
			if (*Curr++ == ']')
				throw tError("Unfinished range.");

			r = NewNode(tOperator_Range);
			if (first > *Curr)
				throw tError("Invalid range.");

			if (Nodes[first].Type == tOperator_CClass)
				throw tError("Cannot use character classes in ranges.");

			Nodes[r].Left = Nodes[first].Type;
			int t = EscapeChar();
			Nodes[r].Right = t;
			Nodes[chain].Next = r;
			chain = r;
			first = -1;
		}
		else
		{
			if (first != -1)
			{
				int c = first;
				Nodes[chain].Next = c;
				chain = c;
			}
			first = CharNode(true);
		}
	}

	if (first != -1)
	{
		int c = first;
		Nodes[chain].Next = c;
		chain = c;
		first = -1;
	}

	// A hack?
	Nodes[ret].Left = Nodes[ret].Next;
	Nodes[ret].Next = -1;
	return ret;
}


int tRegex::ParseNumber()
{
	int ret = *Curr - '0';
	int positions = 10;
	Curr++;

	while (tIsdigit(*Curr))
	{
		ret = ret*10 + (*Curr++ - '0');
		if (positions == 1000000000)
			throw tError("Overflow in numeric constant.");

		positions *= 10;
	};

	return ret;
}


int tRegex::Element()
{
	int ret = -1;
	switch (*Curr)
	{
		case '(':
		{
			int expr;
			Curr++;

			if (*Curr =='?')
			{
				Curr++;
				Expect(':');
				expr = NewNode(tOperator_NoCapExpr);
			}
			else
				expr = NewNode(tOperator_Expr);

			int newn = ListRec();
			Nodes[expr].Left = newn;
			ret = expr;
			Expect(')');
			break;
		}

		case '[':
			Curr++;
			ret = Class();
			Expect(']');
			break;

		case iSymbol_StringEnd:
			Curr++;
			ret = NewNode(tOperator_EOL);
			break;

		case iSymbol_AnyChar:
			Curr++;
			ret = NewNode(tOperator_Dot);
			break;

		default:
			ret = CharNode(false);
			break;
	}

	int op;
	bool isgreedy = false;
	uint16 p0 = 0, p1 = 0;
	switch (*Curr)
	{
		case iSymbol_GreedyZeroOrMore:
			p0 = 0;
			p1 = 0xFFFF;
			Curr++;
			isgreedy = true;
			break;

		case iSymbol_GreedyOneOrMore:
			p0 = 1;
			p1 = 0xFFFF;
			Curr++;
			isgreedy = true;
			break;

		case iSymbol_GreedyZeroOrOne:
			p0 = 0;
			p1 = 1;
			Curr++;
			isgreedy = true;
			break;

		case '{':
			Curr++;
			if (!tIsdigit(*Curr))
				throw tError("Number expected.");

			p0 = uint16(ParseNumber());
			switch (*Curr)
			{
				case '}':
					p1 = p0;
					Curr++;
					break;

				case ',':
					Curr++;
					p1 = 0xFFFF;
					if (tIsdigit(*Curr))
						p1 = uint16(ParseNumber());
					Expect('}');
					break;

				default:
					throw tError("A , or } was expected.");
			}

			isgreedy = true; 
			break;
	}

	if (isgreedy)
	{
		int nnode = NewNode(tOperator_Greedy);
		op = tOperator_Greedy;
		Nodes[nnode].Left = ret;
		Nodes[nnode].Right = ((p0)<<16) | p1;
		ret = nnode;
	}

	if
	(
		(*Curr != iSymbol_Branch) && (*Curr != ')') &&
		(*Curr != iSymbol_GreedyZeroOrMore) && (*Curr != iSymbol_GreedyOneOrMore) &&
		(*Curr != '\0')
	)
	{
		int nnode = Element();
		Nodes[ret].Next = nnode;
	}

	return ret;
}


int tRegex::ListRec()
{
	int ret = -1, e;
	if (*Curr == iSymbol_StringBegin)
	{
		Curr++;
		ret = NewNode(tOperator_BOL);
	}

	e = Element();
	if (ret != -1)
		Nodes[ret].Next = e;
	else
		ret = e;

	if (*Curr == iSymbol_Branch)
	{
		int temp, tright;
		Curr++;
		temp = NewNode(tOperator_Or);
		Nodes[temp].Left = ret;
		tright = ListRec();
		Nodes[temp].Right = tright;
		ret = temp;
	}

	return ret;
}


bool tRegex::MatchCClass(int cclass, char c)
{
	switch (cclass)
	{
		case 'a':
			return tIsalpha(c) ? true : false;

		case 'A':
			return !tIsalpha(c) ? true : false;

		case 'w':
			return (tIsalnum(c) || c == '_') ? true : false;

		case 'W':
			return (!tIsalnum(c) && c != '_') ? true : false;

		case 's':
			return tIsspace(c) ? true : false;

		case 'S':
			return !tIsspace(c) ? true : false;

		case 'd':
			return tIsdigit(c) ? true : false;

		case 'D':
			return !tIsdigit(c) ? true : false;

		case 'x':
			return tIsxdigit(c) ? true : false;

		case 'X':
			return !tIsxdigit(c) ? true : false;

		case 'c':
			return tIscntrl(c) ? true : false;

		case 'C':
			return !tIscntrl(c) ? true : false;

		case 'p':
			return tIspunct(c) ? true : false;

		case 'P':
			return !tIspunct(c) ? true : false;

		case 'l':
			return tIslower(c) ? true : false;

		case 'u':
			return tIsupper(c) ? true : false;
	}

	return false;
}


bool tRegex::MatchClass(const tRegex::Node* node, char c) const
{
	do
	{
		switch (node->Type)
		{
			case tOperator_Range:
				if (c >= node->Left && c <= node->Right)
					return true;
				break;

			case tOperator_CClass:
				if (MatchCClass(node->Left, c))
					return true;
				break;

			default:
				if (c == node->Type)
					return true;
		}
	}
	while ((node->Next != -1) && (node = &Nodes[node->Next]));

	return false;
}


const char* tRegex::MatchNode(const tRegex::Node* node, const char* str, const tRegex::Node* next) const
{
	int type = node->Type;
	switch (type)
	{
		case tOperator_Greedy:
		{
			const tRegex::Node* greedystop = 0;
			int p0 = (node->Right >> 16) & 0x0000FFFF, p1 = node->Right & 0x0000FFFF, nmaches = 0;
			const char* s = str, *good = str;

			if (node->Next != -1)
				greedystop = &Nodes[node->Next];
			else
				greedystop = next;

			while ((nmaches == 0xFFFF || nmaches < p1))
			{
				if (!(s = MatchNode(&Nodes[node->Left], s, greedystop)))
					break;

				nmaches++;
				good = s;
				if (greedystop)
				{
					// Checks that 0 matches satisfy the expression and if so skips.
					// If not would always stop (for instance if is a '?').
					if
					(
						greedystop->Type != tOperator_Greedy ||
						(greedystop->Type == tOperator_Greedy && ((greedystop->Right >> 16)&0x0000FFFF) != 0)
					)
					{
						tRegex::Node* gnext = 0;
						if (greedystop->Next != -1)
							gnext = &Nodes[greedystop->Next];
						else if (next && next->Next != -1)
							gnext = &Nodes[next->Next];

						const char* stop = MatchNode(greedystop, s, gnext);
						if (stop)
						{
							// If satisfied stop it.
							if (p0 == p1 && p0 == nmaches)
								break;
							else if (nmaches >= p0 && p1 == 0xFFFF)
								break;
							else if (nmaches >= p0 && nmaches <= p1)
								break;
						}
					}
				}

				if (s >= EOL)
					break;
			}

			if (p0 == p1 && p0 == nmaches)
				return good;

			else if (nmaches >= p0 && p1 == 0xFFFF)
				return good;

			else if (nmaches >= p0 && nmaches <= p1)
				return good;

			return nullptr;
		}

		case tOperator_Or:
		{
			const char* asd = str;
			tRegex::Node* temp = &Nodes[node->Left];
			while ((asd = MatchNode(temp, asd, 0)))
			{
				if (temp->Next != -1)
					temp = &Nodes[temp->Next];
				else
					return asd;
			}

			asd = str;
			temp = &Nodes[node->Right];
			while ((asd = MatchNode(temp, asd, 0)))
			{
				if (temp->Next != -1)
					temp = &Nodes[temp->Next];
				else
					return asd;
			}
			return nullptr;
		}

		case tOperator_Expr:
		case tOperator_NoCapExpr:
		{
			tRegex::Node* n = &Nodes[node->Left];
			const char* cur = str;
			int capture = -1;
			if (node->Type != tOperator_NoCapExpr && node->Right == CurrSubExpr)
			{
				capture = CurrSubExpr;
				Matches[capture].Begin = cur;
				CurrSubExpr++;
			}

			do
			{
				const tRegex::Node* subnext = 0;
				if (n->Next != -1)
					subnext = &Nodes[n->Next];
				else
					subnext = next;

				if (!(cur = MatchNode(n, cur, subnext)))
				{
					if (capture != -1)
					{
						Matches[capture].Begin = 0;
						Matches[capture].Length = 0;
					}
					return nullptr;
				}
			}
			while ((n->Next != -1) && (n = &Nodes[n->Next]));

			if (capture != -1)
				Matches[capture].Length = int(cur - Matches[capture].Begin);

			return cur;
		}

		case tOperator_WB:
			if
			(
				(str == BOL && !tIsspace(*str)) || (str == EOL && !tIsspace(*(str-1))) ||
				(!tIsspace(*str) && tIsspace(*(str+1))) || (tIsspace(*str) && !tIsspace(*(str+1)))
			)
			{
				return (node->Left == 'b') ? str : 0;
			}
			return (node->Left == 'b') ? 0 : str;

		case tOperator_BOL:
			if (str == BOL)
				return str;
			return nullptr;

		case tOperator_EOL:
			if (str == EOL)
				return str;
			return nullptr;

		case tOperator_Dot:
			str++;
			return str;

		case tOperator_NClass:
		case tOperator_Class:
			if
			(
				MatchClass(&Nodes[node->Left], *str) ?
				(type == tOperator_Class ? true : false) :
				(type == tOperator_NClass ? true : false)
			)
			{
				str++;
				return str;
			}
			return nullptr;

		case tOperator_CClass:
			if (MatchCClass(node->Left, *str))
			{
				str++;
				return str;
			}
			return nullptr;

		default:
			if (*str != node->Type)
				return nullptr;
			str++;
			return str;
	}

	return nullptr;
}


void tRegex::CompileInternal()
{
	tAssert(Pattern && !EOL && !BOL && !NumNodes && !Matches && !NumSubExpr);
	Curr = Pattern;
	NumNodesAllocated = tStrlen(Pattern) * sizeof(char);
	Nodes = (tRegex::Node*)tMalloc(NumNodesAllocated * sizeof(tRegex::Node));

	First = NewNode(tOperator_Expr);
	int res = ListRec();
	Nodes[First].Left = res;
	if (*Curr != '\0')
		throw tError("Unexpected character.");

	#ifdef REGEX_DEBUG_PRINT
	int nsize = NumNodes;
	tRegex::Node* t = &Nodes[0];
	tPrintf("\n");
	for (int i = 0;i < nsize; i++)
	{
		if (Nodes[i].Type >= tOperator_First)
			tPrintf("[%02d] %10s ", i, OperatorNames[Nodes[i].Type-tOperator_First]);
		else
			tPrintf("[%02d] %10c ", i, Nodes[i].Type);
		tPrintf("left %02d right %02d next %02d\n", Nodes[i].Left, Nodes[i].Right, Nodes[i].Next);
	}
	tPrintf("\n");
	#endif

	Matches = (MatchInternal*)tMalloc(NumSubExpr * sizeof(MatchInternal));
	tMemset(Matches, 0, NumSubExpr * sizeof(MatchInternal));
}


void tRegex::Clear()
{
	CurrSubExpr = 0;
	if (Matches)
		tFree(Matches);
	Matches = 0;

	NumSubExpr = 0;
	NumNodes = 0;
	NumNodesAllocated = 0;

	if (Nodes)
		tFree(Nodes);
	Nodes = 0;

	First = -1;
	Curr = 0;
	BOL = 0;
	EOL = 0;

	if (Pattern)
		tFree(Pattern);
	Pattern = 0;
}


void tRegex::Compile(const tString& pattern)
{
	Clear();
	if (pattern.Length() == 0)
		return;

	Pattern = (char*)tMalloc((pattern.Length() + 1) * sizeof(char));
	tStrcpy(Pattern, pattern.ConstText());
	CompileInternal();
}


void tRegex::Compile(const char* pattern)
{
	Clear();
	if (!pattern)
		return;

	int len = tStrlen(pattern);
	if (!len)
		return;

	Pattern = (char*)tMalloc((len + 1) * sizeof(char));
	tStrcpy(Pattern, pattern);
	CompileInternal();
}


bool tRegex::IsMatch(const char* text) const
{
	const char* res = 0;
	BOL = text;
	EOL = text + tStrlen(text);
	CurrSubExpr = 0;
	res = MatchNode(Nodes, text, 0);
	if (res == 0 || res != EOL)
		return false;

	return true;
}


void tRegex::Search(const char* textBegin, const char* textEnd, tList<Match>& matches) const
{
	const char* cur = 0;
	int node = First;
	const char* origBegin = textBegin;
	if (textBegin >= textEnd)
		return;

	BOL = textBegin;
	EOL = textEnd;
	do
	{
		cur = textBegin;
		while (node != -1)
		{
			CurrSubExpr = 0;
			cur = MatchNode(&Nodes[node], cur, 0);
			if (!cur)
				break;

			node = Nodes[node].Next;
		}

		textBegin++;
	}
	while (cur == 0 && textBegin != textEnd);

	if (cur == 0)
		return;

	--textBegin;

	// Populate the matches list.
	tAssert(Matches);
	for (int m = 0; m < NumSubExpr; m++)
	{
		MatchInternal& mi = Matches[m];
		matches.Append(new Match(int(mi.Begin-origBegin), mi.Length));
	}
}


}
