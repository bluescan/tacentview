// tScript.h
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

#pragma once
#include <Foundation/tList.h>
#include <Foundation/tString.h>
#include <Math/tFundamentals.h>
#include <Math/tQuaternion.h>
#include <Math/tVector2.h>
#include <Math/tVector3.h>
#include <Math/tVector4.h>
#include <Math/tMatrix2.h>
#include <Math/tMatrix4.h>
#include <Math/tColour.h>
#include <Math/tHash.h>
#include "System/tThrow.h"
#include "System/tPrint.h"
#include "System/tFile.h"


// An s-expression has the following syntax: [command argument1 argument2 argument3] where both the commands and
// arguments are also expressions. At the leaf level an expression is an Atom. This is the essence of symbolic
// expressions. ex. (a.(b.(c.()))) == [a b c] is supported but (a.b) is not. Cdr is not much use in proper lists, but
// Car, Cadr, Caddr, etc are. i.e. Car([a b c]) = a. Cadr([a b c]) = b. etc. 
class tExpression
{
public:
	tExpression()																										: ValueData(nullptr), LineNumber(0) { }
	tExpression(const char* v)																							: ValueData(v), LineNumber(0) { }

	// If you want the expression to keep track of what line number it's on then you should supply the current line
	// number. Thrown error messages will include the line number if it's set.
	tExpression(const char* v, int lineNumber)																			: ValueData(v), LineNumber(lineNumber) { }
	virtual ~tExpression()																								{ }

	// Like in scheme. Contents of the Address Register from the old IBM days.
	tExpression Car() const;
	tExpression Cadr() const																							{ return CarCdrN(1); }
	tExpression Caddr() const																							{ return CarCdrN(2); }
	tExpression Cadddr() const																							{ return CarCdrN(3); }
	tExpression Caddddr() const																							{ return CarCdrN(4); }
	tExpression Cadddddr() const																						{ return CarCdrN(5); }
	tExpression Caddddddr() const																						{ return CarCdrN(6); }
	tExpression CarCdrN(int n) const;

	// If there aren't enough d's in the above commands or there are a variable number of items, use this until you get
	// an invalid expression.
	virtual tExpression Next() const;

	bool IsValid() const																								{ return ValueData ? true : false; }
	bool IsAtom() const;

	tString GetAtomString() const;
	bool GetAtomBool() const																							{ return GetAtomString().GetAsBool(); }
	uint GetAtomUint() const																							{ return GetAtomString().GetAsUInt(); }
	uint64 GetAtomUint64() const																						{ return GetAtomString().GetAsUInt64(); }
	int GetAtomInt() const																								{ return GetAtomString().GetAsInt(); }
	float GetAtomFloat() const																							{ return GetAtomString().GetAsFloat(); }
	uint32 GetAtomHash() const																							{ tMath::tHashString(GetAtomString()); }
	uint32 Hash() const																									{ return GetAtomHash(); }

	// Vectors, quaternions, matrices, and colours should be of the form (x, y, z). Colours are represented as
	// (r, g, b, a) where all elements are in [0, 255]. Matrices currently need to be specified similarly to vectors
	// except with 16 values. The values must be specified in column-major format.
	tMath::tVector2 GetAtomVector2() const;
	tMath::tVector3 GetAtomVector3() const;
	tMath::tVector4 GetAtomVector4() const;
	tMath::tQuaternion GetAtomQuaternion() const;
	tMath::tMatrix2 GetAtomMatrix2() const;
	tMath::tMatrix4 GetAtomMatrix4() const;
	tColouri GetAtomColour() const;

	// Implicit casting to various types. They can be used instead of the GetAtom methods. Notice that some of the
	// common math types, like vectors, are supported also.
	operator tString() const																							{ if (IsAtom()) return GetAtomString(); else return tString(); }
	operator bool() const																								{ if (IsAtom()) return GetAtomBool(); else return false; }
	operator int() const																								{ if (IsAtom()) return GetAtomInt(); else return 0; }
	operator uint32() const																								{ if (IsAtom()) return GetAtomUint(); else return 0; }
	operator long() const																								{ if (IsAtom()) return long(GetAtomInt()); else return 0; }
	operator float() const																								{ if (IsAtom()) return GetAtomFloat(); else return 0.0f; }
	operator tMath::tVector2() const																					{ if (IsAtom()) return GetAtomVector2(); else return tMath::tVector2::zero; }
	operator tMath::tVector3() const																					{ if (IsAtom()) return GetAtomVector3(); else return tMath::tVector3::zero; }
	operator tMath::tVector4() const																					{ if (IsAtom()) return GetAtomVector4(); else return tMath::tVector4::zero; }
	operator tMath::tQuaternion() const																					{ if (IsAtom()) return GetAtomQuaternion(); else return tMath::tQuaternion::zero; }
	operator tMath::tMatrix2() const																					{ if (IsAtom()) return GetAtomMatrix2(); else return tMath::tMatrix2::zero; }
	operator tMath::tMatrix4() const																					{ if (IsAtom()) return GetAtomMatrix4(); else return tMath::tMatrix4::zero; }
	operator tColouri() const																							{ if (IsAtom()) return GetAtomColour(); else return tColouri::black; }

	// Here are some alternate names for the functions above.
	tExpression Arg0() const								/* Arg0 is the command. */									{ return Car(); }
	tExpression Arg1() const																							{ return Cadr(); }
	tExpression Arg2() const																							{ return Caddr(); }
	tExpression Arg3() const																							{ return Cadddr(); }
	tExpression Arg4() const																							{ return Caddddr(); }
	tExpression Arg5() const																							{ return Cadddddr(); }
	tExpression Arg6() const																							{ return Caddddddr(); }
	tExpression ArgN(int n) const																						{ return CarCdrN(n); }
	int CountArgs() const									/* Not fast. */												{ if (!IsValid()) return 0; int c = 0; while (CarCdrN(c).IsValid()) c++; return c; }

	tExpression Item0() const																							{ return Car(); }
	tExpression Item1() const																							{ return Cadr(); }
	tExpression Item2() const																							{ return Caddr(); }
	tExpression Item3() const																							{ return Cadddr(); }
	tExpression Item4() const																							{ return Caddddr(); }
	tExpression Item5() const																							{ return Cadddddr(); }
	tExpression Item6() const																							{ return Caddddddr(); }
	tExpression ItemN(int n) const																						{ return CarCdrN(n); }
	int CountItems() const									/* Not fast. */												{ return CountArgs(); }

	tExpression Cmd() const																								{ return Car(); }
	tExpression Command() const																							{ return Car(); }
	tExpression First() const																							{ return Car(); }

	bool Valid() const																									{ return IsValid(); }
	bool Atom() const																									{ return IsAtom(); }

	// Some stuff for error reporting. In an error condition this returns the context of the problem.
	tString GetContext() const;
	int GetLineNumber() const																							{ return LineNumber; }

protected:
	// Chugs along the in-memory data ignoring stuff that is allowed to be ignored. Returns the number of new lines
	// encountered along the way.
	static const char* EatWhiteAndComments(const char*, int& lineCount);

	// The memory for this is owned by the tScriptRead class.
	const char* ValueData;

	// The first valid line number starts at 1.
	int LineNumber;

	// When throwing an error this is how much of the file is supplied to give a context.
	static const int ContextSize = 32;

private:
	// Parses atom strings of the form (a, b, c, ...). There may or may not be spaces. Returns the part inside the
	// brackets and removes all spaces. This function is a helper for getting atoms that are vectors, quaternions,
	// matrices, or colours.
	tString GetAtomTupleString() const;
};

// If you don't like to type.
typedef tExpression tExpr;


// Use this to read and parse an existing script. A script file is a list of expressions without []'s around the
// entire file. e.g. This is a valid script:
//
// [a b c]				; Arg0
// d					; Arg1
// [e f]				; Arg2
class tScriptReader : public tExpression
{
public:
	// Constructs an initially invalid tScriptReader.
	tScriptReader()																										: tExpression(), ReadBuffer(nullptr) { }

	// If isFile is true then the file 'name' is loaded, otherwise treats 'name' as the actual script string.
	tScriptReader(const tString& name, bool isFile = true)																: tExpression(), ReadBuffer(nullptr) { Load(name, isFile); }

	// Useful for command line utilities. Makes a script from standard command line argc and argv parameters. Honestly,
	// I'm not sure how useful this is now that we have tOption for parsing command lines in a nice way that is a bit
	// more standard.
	tScriptReader(int argc, char** argv);
	~tScriptReader()																									{ delete[] ReadBuffer; }

	// If isFile is true then the file 'name' is loaded, otherwise treats 'name' as the actual script string. The
	// object is cleared before the new information is loaded. Any previous information is lost.
	void Load(const tString& name, bool isFile = true);

	// The object will be invalid after this call.
	void Clear()																										{ delete[] ReadBuffer; ReadBuffer = 0; }
	bool IsValid() const																								{ return ReadBuffer ? true : false; }

private:
	char* ReadBuffer;
};


// Use this to create a script file.
class tScriptWriter
{
public:
	// Creates the file if it doesn't exist, overwrites it if it does.
	tScriptWriter(const tString& filename);
	~tScriptWriter()																									{ tSystem::tCloseFile(ScriptFile); }

	void BeginExpression();
	void EndExpression();

	// For the WriteAtom calls any attempt to write a floating point value that is a "special" IEEE bit-pattern (like
	// Infinity, NAN, etc) will result in a value of 0.0 being written.
	void WriteAtom(const tString&);
	void WriteAtom(const char*);
	void WriteAtom(const bool);
	void WriteAtom(const uint32);
	void WriteAtom(const uint64);
	void WriteAtom(const int);
	void WriteAtom(const float);
	void WriteAtom(const tMath::tVector2&);
	void WriteAtom(const tMath::tVector3&);
	void WriteAtom(const tMath::tVector4&);
	void WriteAtom(const tMath::tQuaternion&);
	void WriteAtom(const tMath::tMatrix2&);
	void WriteAtom(const tMath::tMatrix4&);
	void WriteAtom(const tColouri&);

	// Writes a single line comment to the script file.
	void WriteComment(const char* = 0);

	// Use these for multiline comments. They use the < > characters. They are not indented.
	void WriteCommentBegin();
	void WriteCommentLine(const char* = 0);
	void WriteCommentEnd();

	void Indent()																										{ CurrIndent += IndentDelta; }
	void Dedent()																										{ CurrIndent -= IndentDelta; if (CurrIndent < 0) CurrIndent = 0; }
	void NewLine();

	// Here are shortened versions of the commands so you don't have to type as much.
	void Beg()																											{ BeginExpression(); }
	void Begin()																										{ BeginExpression(); }
	void End()																											{ EndExpression(); }
	void Atom(const tString& s)																							{ WriteAtom(s); }
	void Atom(const char* c)																							{ WriteAtom(c); }
	void Atom(const bool b)																								{ WriteAtom(b); }
	void Atom(const uint32 u)																							{ WriteAtom(u); }
	void Atom(const int i)																								{ WriteAtom(i); }
	void Atom(const float f)																							{ WriteAtom(f); }
	void Atom(const tMath::tVector2& v)																					{ WriteAtom(v); }
	void Atom(const tMath::tVector3& v)																					{ WriteAtom(v); }
	void Atom(const tMath::tVector4& v)																					{ WriteAtom(v); }
	void Atom(const tMath::tQuaternion& q)																				{ WriteAtom(q); }
	void Atom(const tMath::tMatrix2& m)																					{ WriteAtom(m); }
	void Atom(const tMath::tMatrix4& m)																					{ WriteAtom(m); }
	void Atom(const tColouri& c)																						{ WriteAtom(c); }

	void Rem(const char* c = 0)																							{ WriteComment(c); }
	void RemBegin()																										{ WriteCommentBegin(); }
	void RemLine(const char* l = 0)																						{ WriteCommentLine(l); }
	void RemEnd()																										{ WriteCommentEnd(); }

	void Ind()																											{ Indent(); }
	void DInd()																											{ Dedent(); }
	void CR()																											{ NewLine(); }
	void Return()																										{ NewLine(); }

	// The Item functions write a list of 2 to 5 atoms (a command followed by 1 to 4 arguments). They're just
	// convenience functions to write stuff like [a b] followed by a carraige return. The first atom must always be a
	// string.
	template<typename T> void Item(const tString& item0, const T& item1)												{ Begin(); Atom(item0); Atom(item1); End(); CR(); }
	template<typename T> void Item(const tString& item0, const T& item1, const T& item2)								{ Begin(); Atom(item0); Atom(item1); Atom(item2); End(); CR(); }
	template<typename T> void Item(const tString& item0, const T& item1, const T& item2, const T& item3)				{ Begin(); Atom(item0); Atom(item1); Atom(item2); Atom(item3); End(); CR(); }
	template<typename T> void Item(const tString& item0, const T& item1, const T& item2, const T& item3, const T& item4){ Begin(); Atom(item0); Atom(item1); Atom(item2); Atom(item3); Atom(item4); End(); CR(); }
	void WriteItem(const tString& name, const tString& value);

private:
	int CurrIndent;
	static const int IndentDelta = 4;

protected:
	tFileHandle ScriptFile;
};


// A tFunExtression takes the form FunctionName(Arg1, Arg2, Arg3...)
class tFunExtression : public tLink<tFunExtression>
{
public:
	tFunExtression()																									: Function(), Arguments() { }

	// Argument must point to the first character of the function name.
	tFunExtression(const char*);
	virtual ~tFunExtression()																							{ while (tStringItem* arg = Arguments.Remove()) delete arg; }

	tString Function;
	tList<tStringItem> Arguments;
};


class tFunScript
{
public:
	tFunScript()																										: Expressions() { }
	tFunScript(const tString& fileName)																					: Expressions() { Load(fileName); }
	~tFunScript()																										{ Clear(); }

	void Clear()																										{ while (tFunExtression* exp = Expressions.Remove()) delete exp; }
	void Load(const tString& fileName);
	void Save(const tString& fileName);

	tFunExtression* First() const																						{ return Expressions.First(); }
	tFunExtression* Last() const																						{ return Expressions.Last(); }

	// A tFunScript is just a list of expressions. A tree may be more powerful?
	tList<tFunExtression> Expressions;	

private:
	char* EatWhiteAndComments(char* c);
};


// The following error objects may be thrown by script parsing functions.
struct tScriptError : public tError
{
	tScriptError(const char* format, ...)																				: tError("tScript Module. ") { va_list marker; va_start(marker, format); tString msg; Message += tvsPrintf(msg, format, marker); }
	tScriptError(const tString& m)																						: tError("tScript Module. ") { Message += m; }
	tScriptError()																										: tError("tScript Module.") { }
};
