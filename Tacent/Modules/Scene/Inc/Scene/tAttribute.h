// tAttribute.h
//
// All scene objects may have attributes. Attributes are extra data (that has a type) that the content author
// attributed to the scene object.
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
#include <System/tChunk.h>
namespace tScene
{


class tAttribute
{
public:
	tAttribute()																										{ Clear(); }
	tAttribute(const tChunk& chunk)																						{ Clear(); Load(chunk); }
	virtual ~tAttribute()																								{ }

	enum class tType
	{
		Invalid							= -1,
		Bool,
		Int,
		Colour,							// 4 byte colour.
		Float,
		NumTypes
	};

	void Clear()																										{ Name.Clear(); Type = tType::Invalid; Value = 0; }
	bool IsValid() const																								{ return (Type != tType::Invalid) ? true : false; }
	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	bool IsBool() const																									{ return (Type == tType::Bool) ? true : false; }
	bool IsInt() const																									{ return (Type == tType::Int) ? true : false; }
	bool IsColour() const																								{ return (Type == tType::Colour) ? true : false; }
	bool IsFloat() const																								{ return (Type == tType::Float) ? true : false; }

	bool GetAsBool() const																								{ if (Type != tType::Bool) return false; return Value ? true : false; }
	int GetAsInt() const																								{ if (Type != tType::Int) return 0; return int(Value); }
	tColouri GetAsColour() const																						{ if (Type != tType::Colour) return tColour::black; return *((tColouri*)(&Value)); }
	float GetAsFloat() const																							{ if (Type != tType::Float) return 0.0f; return *((float*)(&Value)); }

	operator bool() const																								{ return GetAsBool(); }
	operator int() const																								{ return GetAsInt(); }
	operator tColouri() const																							{ return GetAsColour(); }
	operator float() const																								{ return GetAsFloat(); }
	bool operator==(const tAttribute& a) const																			{ if (Name != a.Name) return false; if (Type != a.Type) return false; if (Value != a.Value) return false; return true; }
	bool operator!=(const tAttribute& a) const																			{ return !(*this == a); }
	tAttribute& operator=(const tAttribute& a)																			{ Name = a.Name; Type = a.Type; Value = a.Value; }

	tString Name;
	tType Type;							// The type determines how to interpret Value.
	uint32 Value;
};


class tAttributes
{
public:
	tAttributes()																										{ }
	tAttributes(const tAttributes& src)																					{ *this = src; }
	tAttributes(const tChunk& chunk)																					{ Load(chunk); }
	virtual ~tAttributes()																								{ }

	void Clear()																										{ Attributes.Empty(); }
	void Save(tChunkWriter&) const;
	void Load(const tChunk&);
	bool Contains(const tAttribute&) const;
	int Count() const																									{ return Attributes.Count(); }

	// tAttributes are equal if they contain the exact same attributes in any order.
	bool operator==(const tAttributes&) const;
	bool operator!=(const tAttributes& a) const																			{ return !(*this == a); }
	tAttributes& operator=(const tAttributes&);

	mutable tItList<tAttribute> Attributes;
};


}
