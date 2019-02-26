// tScript.cpp
//
// This module contains script file readers and writers. Tacent supports two text script formats. The main one is in
// the spirit of Church's lambda calculus and used 'symbolic expressions'. ex. [a b c]. See tExpression. I personally
// find this notation much more elegant than stuff like the lame XML notation. This site agrees:
// http://c2.com/cgi/wiki?XmlIsaPoorCopyOfEssExpressions
// The tExpression reader class in this file parses 'in-place'. That is, the entire file is just read into memory once
// and accessed as const data. This reduces memory fragmentation but may have made implementation more complex. 
//
// The second format is a functional format. ex. a(b,c) See tFunExtression.
//
// Copyright (c) 2006, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tString.h>
#include "System/tFile.h"
#include "System/tScript.h"
using namespace tMath;


tExpression tExpression::Car() const
{
	tAssert( IsValid() );

	const char* c = ValueData + 1;

	if (*c == '\0')
		return tExpression();

	int lineCount;
	c = EatWhiteAndComments(c, lineCount);

	// Look for empty list.
	if (*c == ']')
		return tExpression();

	return tExpression(c, LineNumber+lineCount);
}


tExpression tExpression::CarCdrN(int n) const
{
	tExpression e = Car();

	for (int i = 0; i < n; i++)
	{
		if (!e.Valid())
			return e;

		e = e.Next();
	}

 	return e;
}


tExpression tExpression::Next() const
{
	tAssert( IsValid() );

	const char* c = ValueData;
	int count = 0;
	int lineNum = LineNumber;

	if (!IsAtom())
	{
		// Skip the expression by counting square brackets till we are at zero again. Opening bracket adds 1, closing subtracts 1. Simple.
		// @todo Slight bug here. If there is a string inside the expression with []s, they must match.
		do
		{
			if (*c == '[')
				count++;

			if (*c == ']')
				count--;

			if (*c == '\0')
				return tExpression();

			if (*c == '\r')
				lineNum++;

			c++;
		}
		while (count);
	}
	else
	{
		// It's an atom. If it begins with a quote we must find where it ends.
		if (*c == '"')
		{
			c++;

			// Keep going until next quote.
			c = strchr(c, '"');

			if (!c)
			{
				if (lineNum != -1)
					throw tScriptError("Begin quote found but no end quote on line %d.", lineNum);
				else
					throw tScriptError("Begin quote found but no end quote.");
			}
			c++;
		}

		// If it begins with an open bracket (not a square bracket, a British bracket... a parenthesis in "American") we must find the closing bracket.
		else if (*c == '(')
		{
			c++;

			// Keep going until closing bracket.
			c = strchr(c, ')');

			if (!c)
			{
				if (lineNum != -1)
					throw tScriptError("Opening bracket found but no closing bracket on line %d.", lineNum);
				else
					throw tScriptError("Opening bracket found but no closing bracket.");
			}
			c++;
		}
		else
		{
			// The ';' and '<' should also be terminators for the current argument so that EatWhiteAndComments will get everything.
			while ((*c != ' ') && (*c != '\t') && (*c != '[') && (*c != ']') && (*c != '\0') && (*c != ';') && (*c != '<') && (*c != '"'))
			{
				if (*c == '\n')
					lineNum++;

				c++;
			}
		}
	}

	int lineCount;
	c = EatWhiteAndComments(c, lineCount);
	lineNum += lineCount;

	if ((*c == ']') || (*c == '\0'))
		// We're at the end of the argument list.
		return tExpression();

	return tExpression(c, lineNum);
}


bool tExpression::IsAtom() const
{
	if (ValueData && (*ValueData != '['))
		return true;
	else
		return false;
}


tString tExpression::GetAtomString() const
{
	if (!IsAtom())
	{
		if (LineNumber)
			throw tScriptError("Atom expected on line %d near: %s", LineNumber, GetContext().Pod());
		else
			throw tScriptError("Atom expected near: %s", GetContext().Pod());
	}

	const char* start;
	const char* end;
	if (*ValueData == '"')
	{
		start = ValueData + 1;
		end = strchr(start, '"');

		// Keep going until next quote.
		if (!end)
		{
			if (LineNumber)
				throw tScriptError("Begin quote found but no end quote on line %d.", LineNumber);
			else
				throw tScriptError("Begin quote found but no end quote.");
		}
	}
	else
	{
		start = ValueData;
		end = ValueData;

		while ((*end != ' ') && (*end != '\t') && (*end != '[') && (*end != ']') && (*end != '\0') && (*end != '\r') && (*end != '\n'))
			end++;
	}

	// Creates a tString full of '\0's.
	tString atom(int(end - start));
	tStd::tStrncpy(atom.Text(), start, int(end - start));

	return atom;
}


tString tExpression::GetAtomTupleString() const
{
	if (!IsAtom())
	{
		if (LineNumber)
			throw tScriptError("Atom expected on line %d near: %s", LineNumber, GetContext().Pod());
		else
			throw tScriptError("Atom expected near: %s", GetContext().Pod());
	}

	const char* start = 0;
	const char* end = 0;

	if (*ValueData != '(')
	{
		if (LineNumber)
			throw tScriptError("Tuple atom expected on line %d near: %s", LineNumber, GetContext().Pod());
		else
			throw tScriptError("Tuple atom expected but not found.");
	}

	start = ValueData + 1;
	end = strchr(start, ')');

	// If no end bracket was found we're in trouble.
	if (!end)
	{
		if (LineNumber)
			throw tScriptError("Opening bracket but no corresponding closing bracket on line %d.", LineNumber);
		else
			throw tScriptError("Opening bracket but no corresponding closing bracket.");
	}

	// Creates a tString full of '\0's.
	tString tuple(int(end - start));
	tStd::tStrncpy(tuple.Text(), start, int(end - start));

	// Now remove the '(', the ')', and any spaces.
	tuple.Remove('(');
	tuple.Remove(')');
	tuple.Remove(' ');

	return tuple;
}


tVector2 tExpression::GetAtomVector2() const
{
	// Vectors should be of the form (x, y). Spaces or not after and before the comma.
	tString str = GetAtomTupleString();
	tVector2 v;

	tString component = str.ExtractFirstWord(',');
	v.x = component.GetAsFloat();

	v.y = str.GetAsFloat();
	return v;
}


tVector3 tExpression::GetAtomVector3() const
{
	// Vectors should be of the form (x, y, z). Spaces or not after and before the commas.
	tString str = GetAtomTupleString();
	tVector3 v;

	tString component = str.ExtractFirstWord(',');
	v.x = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	v.y = component.GetAsFloat();

	v.z = str.GetAsFloat();
	return v;
}


tVector4 tExpression::GetAtomVector4() const
{
	// Vectors should be of the form (x, y, z, w). Spaces or not after and before the commas.
	tString str = GetAtomTupleString();
	tVector4 v;

	tString component = str.ExtractFirstWord(',');
	v.x = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	v.y = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	v.z = component.GetAsFloat();

	v.w = str.GetAsFloat();
	return v;
}


tQuaternion tExpression::GetAtomQuaternion() const
{
	// Quaternions should be of the form (w, x, y, z). Spaces or not after and before the commas.
	tString str = GetAtomTupleString();
	tQuaternion q;

	tString component = str.ExtractFirstWord(',');
	q.x = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	q.y = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	q.z = component.GetAsFloat();

	q.w = str.GetAsFloat();
	return q;
}


tMatrix2 tExpression::GetAtomMatrix2() const
{
	// Matrices should be of the form (a11, a21, a12, a22). Spaces or not after and before the commas.
	tString str = GetAtomTupleString();
	tMatrix2 m;

	tString component = str.ExtractFirstWord(',');
	m.C1.x = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	m.C1.y = component.GetAsFloat();

	component = str.ExtractFirstWord(',');
	m.C2.x = component.GetAsFloat();

	m.C2.y = str.GetAsFloat();
	return m;
}


tMatrix4 tExpression::GetAtomMatrix4() const
{
	// Matrices should be of the form (a11, a21, a31, a41, a12, a22, ..., a44).
	// Spaces or not after and before the commas.
	tString str = GetAtomTupleString();
	tMatrix4 m;

	tString component = str.ExtractFirstWord(',');
	m.E[0] = component.GetAsFloat();

	for (int e = 1; e < 15; e++)
	{
		component = str.ExtractFirstWord(',');
		m.E[e] = component.GetAsFloat();
	}

	m.E[15] = str.GetAsFloat();
	return m;
}


tColouri tExpression::GetAtomColour() const
{
	// Colours should be of the form (r, g, b, a). Spaces or not after and before the commas.
	tString str = GetAtomTupleString();
	tColouri c;

	tString component = str.ExtractFirstWord(',');
	c.R = component.GetAsInt();

	component = str.ExtractFirstWord(',');
	c.G = component.GetAsInt();

	component = str.ExtractFirstWord(',');
	c.B = component.GetAsInt();

	c.A = str.GetAsInt();
	return c;
}


const char* tExpression::EatWhiteAndComments(const char* c, int& lineCount)
{
	// There are two types of comment. Single-line comments using a semi-colon go to the end of the current line.
	// Multi-line comments are delimited with less-than and greater-than characters <>. Note that <> are still allowed
	// inside a string "like > this" without being considered as begin or end comment markers.
	bool inSingleLineComment = false;
	int inMultiLineComment = 0;
	bool inString = false;

	lineCount = 0;

	while
	(
		(*c == ' ') || (*c == '\t') || (*c == '\n') || (*c == '\r') || (*c == 9) ||
		(*c == ';') || (*c == '<') || (*c == '>') || inSingleLineComment || inMultiLineComment
	)
	{
		if ((*c == '<') && !inSingleLineComment && !inString)
			inMultiLineComment++;

		else if ((*c == '>') && !inSingleLineComment && inMultiLineComment && !inString)
			inMultiLineComment--;

		else if ((*c == ';') && !inMultiLineComment)
			inSingleLineComment = true;

		else if (inSingleLineComment && ((*c == '\r') || (*c == '\n') || (*c == '\0')))
			inSingleLineComment = false;

		else if (inMultiLineComment && (*c == '"'))
			inString = !inString;

		if ((*c == '\n'))
			lineCount++;

		c++;
	}

	return c;
}


tString tExpression::GetContext() const
{
	tString context(ContextSize);

	// We're allowed to do this because as soon as strncpy sees a 0 in the src string it stops reading from it.
	tStd::tStrncpy(context.Text(), ValueData, ContextSize);

	int newLine = context.FindChar('\r');

	// Only go to the end of the current line.
	if (newLine != -1)
		context[newLine] = '\0';

	return context;
}


tScriptReader::tScriptReader(int argc, char** argv) :
	tExpression(),
	ReadBuffer(0)
{
	// Here we're just concatenating all the argv strings into one.
	tString scriptString = "[";

	for (int i = 0; i < argc; i++)
	{
		tString itemToAdd(argv[i]);
		if (itemToAdd.IsEmpty())
			continue;

		// OK, here we need to interpret the item as a string atom if there is a space. If there is a space, we may
		// need to add quotes if they aren't already there. Normally there won't be any spaces because spaces are
		// precisely what separates the argv arguments from each other. However, sometimes argv[0] can be a single
		// string something like "C:\Program File\SomeDir\SomeFile.txt" and it needs to be treated as a single item even
		// though it has a space.
		//
		// The reason we check for the double quotes is because it is perfectly ok for the command line to contain escaped
		// quotes, in which case the work of inserting them is already done.
		//
		// There is one last caveat: the command line: a.exe [b "c d"] will come in as: 'a.exe', '[b', 'c d]' because there
		// was no space after the last double quote. It turns out it is safe to ignore this if the atom does not have a
		// space, and if it does, we can simply replace the FIRST occurence of a ']' with a '"]' We don't need to handle
		// multiple close brackets and subsequent open brackets are also ok because the tScript syntax demands a space
		// between all arguments. Come to think of it we also need to deal with ["a b" c] going to '[a b' and 'c]' but
		// whatever, this is getting too complicated, I'm just going to require escaped quotes in the command line.
		if ((itemToAdd.FindChar(' ') != -1) && (itemToAdd[0] != '"') && (itemToAdd[itemToAdd.Length()-1] != '"'))
		{
			// Add a beginning quote.
			itemToAdd = "\"" + itemToAdd + "\"";
		}

		scriptString += itemToAdd;

		// Add a space if it's not the last argv string.
		if (i < argc-1)
			scriptString += " ";
	}

	scriptString += "]";

	ReadBuffer = new char[scriptString.Length() + 1];
	tStd::tStrcpy(ReadBuffer, scriptString);

	ValueData = ReadBuffer;
	LineNumber = 1;
}


void tScriptReader::Load(const tString& name, bool isFile)
{
	Clear();
	if (name.IsEmpty())
		return;

	if (isFile)
	{
		tFileHandle file = tSystem::tOpenFile(name, "rb");

		// @todo Consider just becoming an invalid expression.
		tAssert(file);

		int fileSize = tSystem::tGetFileSize(file);

		// Create a buffer big enough for the file, the uber []'s, two line-endings (one for each square bracket), and a terminating 0.
		int bufferSize = fileSize + 7;

		ReadBuffer = new char[bufferSize];
		ReadBuffer[0] = '[';
		ReadBuffer[1] = '\r';
		ReadBuffer[2] = '\n';

		// Load the entire thing into memory.
		int numRead = tSystem::tReadFile(file, (uint8*)(ReadBuffer+3), fileSize);
		if (numRead != fileSize)
			throw tScriptError("Cannot read file [%s].", name.Pod());
		tSystem::tCloseFile(file);

		ReadBuffer[bufferSize-4] = '\r';
		ReadBuffer[bufferSize-3] = '\n';
		ReadBuffer[bufferSize-2] = ']';
		ReadBuffer[bufferSize-1] = '\0';
	}
	else
	{
		int stringSize = name.Length();
		int bufferSize = stringSize + 7;
		ReadBuffer = new char[bufferSize];

		ReadBuffer[0] = '[';
		ReadBuffer[1] = '\r';
		ReadBuffer[2] = '\n';
		tStd::tStrcpy(ReadBuffer+3, name);
		ReadBuffer[bufferSize-4] = '\r';
		ReadBuffer[bufferSize-3] = '\n';
		ReadBuffer[bufferSize-2] = ']';
		ReadBuffer[bufferSize-1] = '\0';
	}

	LineNumber = 1;
	ValueData = EatWhiteAndComments(ReadBuffer, LineNumber);
}


tScriptWriter::tScriptWriter(const tString& filename) :
	CurrIndent(0)
{
	ScriptFile = tSystem::tOpenFile(filename, "wt");

	if (!ScriptFile)
		throw tScriptError("Cannot open file [%s].", pod(filename));
}


void tScriptWriter::BeginExpression()
{
	char beg[] = "[ ";
	int numWritten = tSystem::tWriteFile(ScriptFile, beg, 2);
	if (numWritten != 2)
		throw tScriptError("Cannot write to script file.");
}


void tScriptWriter::EndExpression()
{
	char end[] = "] ";
	int numWritten = tSystem::tWriteFile(ScriptFile, end, 2);
	if (numWritten != 2)
		throw tScriptError("Cannot write to script file.");
}


void tScriptWriter::WriteAtom(const tString& atom)
{
	char qu = '"';

	bool hasSpace = true;
	if (atom.FindChar(' ') == -1)
		hasSpace = false;

	bool isTuple = true;
	if (atom.FindChar('(') == -1)
		isTuple = false;

	// Here we determine whether to use quotes if necessary. If the atom is a tuple (a vector or matrix etc) then we do
	// not use quotes even if spaces are present.
	bool useQuotes = hasSpace && !isTuple;
	int numWritten = 0;
	if (useQuotes)
		numWritten += tSystem::tWriteFile(ScriptFile, &qu, 1);

	int atomLen = atom.Length();
	numWritten += tSystem::tWriteFile(ScriptFile, atom, atomLen);
	if (useQuotes)
		numWritten += tSystem::tWriteFile(ScriptFile, &qu, 1);

	char sp = ' ';
	numWritten += tSystem::tWriteFile(ScriptFile, &sp, 1);

	if (numWritten != (1 + (useQuotes ? 2 : 0) + atomLen))
		throw tScriptError("Cannot write atom [%s] to script file.", pod(atom));
}


void tScriptWriter::WriteAtom(const char* atom)
{
	char qu = '"';

	bool hasSpace = false;
	if (tStd::tStrchr(atom, ' '))
		hasSpace = true;

	bool isTuple = false;
	if (tStd::tStrchr(atom, '('))
		isTuple = true;

	// Here we determine whether to use quotes if necessary. If the atom is a tuple (a vector or matrix etc) then we do
	// not use quotes even if spaces are present.
	bool useQuotes = hasSpace && !isTuple;
	int numWritten = 0;
	if (useQuotes)
		numWritten += tSystem::tWriteFile(ScriptFile, &qu, 1);

	int atomLen = tStd::tStrlen(atom);
	numWritten += tSystem::tWriteFile(ScriptFile, atom, atomLen);
	if (useQuotes)
		numWritten += tSystem::tWriteFile(ScriptFile, &qu, 1);

	char sp = ' ';
	numWritten += tSystem::tWriteFile(ScriptFile, &sp, 1);

	if (numWritten != (1 + (useQuotes ? 2 : 0) + atomLen))
		throw tScriptError("Cannot write atom '%s' to script file.", atom);
}


void tScriptWriter::WriteAtom(const bool atom)
{
	if (atom)
		WriteAtom("True");
	else
		WriteAtom("False");
}


void tScriptWriter::WriteAtom(const uint32 atom)
{
	char val[36];
	tStd::tItoa(atom, val, 36, 10);
	WriteAtom(val);
}


void tScriptWriter::WriteAtom(const uint64 atom)
{
	char val[48];
	tStd::tItoa(atom, val, 48, 10);
	WriteAtom(val);
}


void tScriptWriter::WriteAtom(const int atom)
{
	char val[36];
	tStd::tItoa(atom, val, 36, 10);
	WriteAtom(val);
}


void tScriptWriter::WriteAtom(const float atom)
{
	tString val(32);
	if (tStd::tIsSpecial(atom))
		tsPrintf(val.Text(), "0.0");
	else
		tsPrintf(val.Text(), "%8.8f", atom);

	int l = val.Length();

	// Add a trailing 0 because it looks better.
	if (val[l-1] == '.')
	{
		val[l] = '0';
		val[l+1] = '\0';
	}

	WriteAtom(val);
}


void tScriptWriter::WriteAtom(const tVector2& v)
{
	tString str("(");
	for (int e = 0; e < 2; e++)
	{
		tString val(32);
		if (tStd::tIsSpecial(v.E[e]))
			tsPrintf(val.Text(), "0.0");
		else
			tsPrintf(val.Text(), "%8.8f", v.E[e]);

		int l = val.Length();

		// Add a trailing 0 because it looks better.
		if (val[l-1] == '.')
		{
			val[l] = '0';
			val[l+1] = '\0';
		}
		str += val;
		if (e != 1)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteAtom(const tVector3& v)
{
	tString str("(");
	for (int e = 0; e < 3; e++)
	{
		tString val(32);
		if (tStd::tIsSpecial(v.E[e]))
			tsPrintf(val.Text(), "0.0");
		else
			tsPrintf(val.Text(), "%8.8f", v.E[e]);

		int l = val.Length();

		// Add a trailing 0 because it looks better.
		if (val[l-1] == '.')
		{
			val[l] = '0';
			val[l+1] = '\0';
		}
		str += val;
		if (e != 2)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteAtom(const tVector4& v)
{
	tString str("(");
	for (int e = 0; e < 4; e++)
	{
		tString val(32);
		if (tStd::tIsSpecial(v.E[e]))
			tsPrintf(val.Text(), "0.0");
		else
			tsPrintf(val.Text(), "%8.8f", v.E[e]);

		int l = val.Length();

		// Add a trailing 0 because it looks better.
		if (val[l-1] == '.')
		{
			val[l] = '0';
			val[l+1] = '\0';
		}
		str += val;
		if (e != 3)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteAtom(const tQuaternion& q)
{
	tString str("(");
	for (int e = 0; e < 4; e++)
	{
		tString val(32);
		if (tStd::tIsSpecial(q.E[e]))
			tsPrintf(val.Text(), "0.0");
		else
			tsPrintf(val.Text(), "%8.8f", q.E[e]);

		int l = val.Length();

		// Add a trailing 0 because it looks better.
		if (val[l-1] == '.')
		{
			val[l] = '0';
			val[l+1] = '\0';
		}
		str += val;
		if (e != 3)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteAtom(const tMatrix2& m)
{
	tString str("(");
	for (int e = 0; e < 4; e++)
	{
		tString val(32);
		if (tStd::tIsSpecial(m.E[e]))
			tsPrintf(val.Text(), "0.0");
		else
			tsPrintf(val.Text(), "%8.8f", m.E[e]);

		int l = val.Length();

		// Add a trailing 0 because it looks better.
		if (val[l-1] == '.')
		{
			val[l] = '0';
			val[l+1] = '\0';
		}
		str += val;
		if (e != 3)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteAtom(const tMatrix4& m)
{
	tString str("(");
	for (int e = 0; e < 16; e++)
	{
		tString val(32);
		if (tStd::tIsSpecial(m.E[e]))
			tsPrintf(val.Text(), "0.0");
		else
			tsPrintf(val.Text(), "%8.8f", m.E[e]);

		int l = val.Length();

		// Add a trailing 0 because it looks better.
		if (val[l-1] == '.')
		{
			val[l] = '0';
			val[l+1] = '\0';
		}
		str += val;
		if (e != 15)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteAtom(const tColouri& c)
{
	tString str("(");
	for (int e = 0; e < 4; e++)
	{
		char val[36];
		tStd::tItoa(c.E[e], val, 36, 10);

		str += val;
		if (e != 3)
			str += ", ";
	}
	str += ")";
	WriteAtom(str);
}


void tScriptWriter::WriteComment(const char* comment)
{
	char sc[] = "; ";
	int numWritten = tSystem::tWriteFile(ScriptFile, sc, 2);
	int commentLen = 0;
	if (comment)
	{
		commentLen = tStd::tStrlen(comment);
		numWritten += tSystem::tWriteFile(ScriptFile, comment, commentLen);
	}

	if (numWritten != (2 + commentLen))
		throw tScriptError("Cannot write to script file.");

	NewLine();
}


void tScriptWriter::WriteCommentBegin()
{
	char sc[] = "<\n";
	int numWritten = tSystem::tWriteFile(ScriptFile, sc, 2);
	if (numWritten != 2)
		throw tScriptError("Cannot write to script file.");
}


void tScriptWriter::WriteCommentLine(const char* comment)
{
	int numWritten = 0;
	int commentLen = 0;
	if (comment)
	{
		commentLen = tStd::tStrlen(comment);
		numWritten += tSystem::tWriteFile(ScriptFile, comment, commentLen);
	}

	if (numWritten != commentLen)
		throw tScriptError("Cannot write to script file.");

	NewLine();
}


void tScriptWriter::WriteCommentEnd()
{
	char sc[] = ">\n";
	int numWritten = tSystem::tWriteFile(ScriptFile, sc, 2);
	if (numWritten != 2)
		throw tScriptError("Cannot write to script file.");
}


void tScriptWriter::NewLine()
{
	char nl = '\n';
	char sp = ' ';

	int numWritten = tSystem::tWriteFile(ScriptFile, &nl, 1);
	for (int s = 0; s < CurrIndent; s++)
		numWritten += tSystem::tWriteFile(ScriptFile, &sp, 1);

	if (numWritten != CurrIndent + 1)
		throw tScriptError("Cannot write to script file.");
}


void tScriptWriter::WriteItem(const tString& name, const tString& value)
{
	int numTabs = CurrIndent / IndentDelta;
	for (int t = 0; t < numTabs; t++)
		tfPrintf(ScriptFile, "\t");

	tfPrintf(ScriptFile,"[%s %s]\n", name.ConstText(), value.ConstText());
}


// Next follow the types for the functional scripts of the form f(a, b).
tFunExtression::tFunExtression(const char* function)
{
	const int maxExpressionSize = 512;

	tString buffer(maxExpressionSize);
	tStd::tStrncpy(buffer.Text(), function, maxExpressionSize);
	buffer[maxExpressionSize-1] = '\0';

	// We need to find the trailing ')' that is at the end of our expression. We do this by incrementing and decrementing
	// based on '(' or ')'
	int beginningParen = buffer.FindChar('(');
	int endParen = beginningParen;
	int openCount = 0;

	for (int c = 0; c < maxExpressionSize-1; c++)
	{
		if (buffer[endParen] == '(')
			openCount++;

		if (buffer[endParen] == ')')
			openCount--;

		if (!openCount)
			break;

		endParen++;
	}

	if (openCount)
		throw tScriptError("Expression too long. Missing bracket? Max size is %d chars. Look for [%s]", maxExpressionSize, buffer.Pod());

	buffer[endParen] = '\0';
	int origLength = buffer.Length();
	buffer[beginningParen] = '\0';

	// Now replace all occurences of space, tab, and comma with nulls. Double quote string arguments are handled correctly.
	bool inString = false;
	openCount = 0;
	for (int c = 0; c < origLength; c++)
	{
		int ch = buffer[c];
		if (ch == '"')
		{
			inString = !inString;
			buffer[c] = '\0';
		}

		if (ch == '(')
			openCount++;

		if (ch == ')')
			openCount--;

		if ((!inString) && (!openCount))
		{
			if ((ch == '\t') || (ch == ',') || (ch == '\n') || (ch == '\r') || (ch == 9) || (ch == ' '))
				buffer[c] = '\0';
		}
	}

	// We now have a string of the form: name00arga000argb0argc00. The function name is the first word.
	Function = buffer;
	int pos = Function.Length();

	// Now lets get arguments until there are no more.
	while (1)
	{
		while ((buffer[pos] == '\0') && (pos < endParen))
			pos++;

		if (pos == endParen)
			break;

		tStringItem* arg = new tStringItem( &buffer[pos] );
		Arguments.Append(arg);
		pos += arg->Length();
	}
}


void tFunScript::Load(const tString& fileName)
{
	Clear();
	tFileHandle file = tSystem::tOpenFile(fileName.ConstText() , "rb");
	tAssert(file);

	// Create a buffer big enough for the file.
	int fileSize = tSystem::tGetFileSize(file);
	char* buffer = new char[fileSize + 1];

	// Load the entire thing into memory.
	int numRead = tSystem::tReadFile(file, (uint8*)buffer, fileSize);
	tAssert(numRead == fileSize);

	// This makes buffer a valid null terminated string.
	buffer[fileSize] = '\0';
	tSystem::tCloseFile(file);

	char* currChar = EatWhiteAndComments(buffer);
	while (*currChar != '\0')
	{
		Expressions.Append(new tFunExtression(currChar));

		// Get to the next expression.
		currChar = strchr(currChar, '(');
		int openCount = 0;
		while (1)
		{
			if (*currChar == '(')
				openCount++;

			if (*currChar == ')')
				openCount--;

			currChar++;
			if (!openCount)
				break;
		}

		currChar = EatWhiteAndComments(currChar);
	}

	delete[] buffer;
}


void tFunScript::Save(const tString& fileName)
{
	tFileHandle file = tSystem::tOpenFile(fileName, "wt");

	if (!file)
		throw tScriptError("Cannot open file '%s'.", fileName.ConstText());

	// All we need to do is traverse the expression list and write out each one.
	for (tFunExtression* exp = Expressions.First(); exp; exp = exp->Next())
	{
		tfPrintf(file, "%s(", exp->Function.Text());
		for (tStringItem* arg = exp->Arguments.First(); arg; arg = arg->Next())
		{
			tfPrintf(file, "%s", arg->Text());

			// If it's not the last argument we put a comma there.
			if (arg->Next())
				tfPrintf(file, ", ");
		}
		tfPrintf(file, ")\n");
	}

	tSystem::tCloseFile(file);
}


char* tFunScript::EatWhiteAndComments(char* c)
{
	bool inComment = false;
	while ((*c == ' ') || (*c == '\t') || (*c == '\n') || (*c == '\r') || (*c == 9) || (*c == '/') || inComment)
	{
		if (*c == '/')
			inComment = true;

		if ((*c == '\r') || (*c == '\n') || (*c == '\0'))
			inComment = false;

		c++;
	}

	return c;
}
