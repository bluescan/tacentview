// tObject.h
//
// The base class for all tScene objects. All objects have an ID, a name, and attributes.
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
#include <Foundation/tString.h>
#include <Foundation/tList.h>
#include <System/tChunk.h>
#include "Scene/tAttribute.h"
namespace tScene
{


class tObject
{
public:
	tObject()																											{ }
	tObject(const tObject& src)																							: ID(src.ID), Name(src.Name), Attributes(src.Attributes) { }
	tObject(const tString& name, uint32 id)																				: ID(id), Name(name) { }
	virtual ~tObject()																									{ }

	virtual void Save(tChunkWriter&) const;
	virtual void Load(const tChunk&);
	void Clear()																										{ ID = 0xFFFFFFFF; Name.Clear(); Attributes.Clear(); }

	bool IsFunctionallySame(const tObject& obj) const		/* Does not check name or ID. */							{ return (Attributes == obj.Attributes); }
	bool operator==(const tObject& obj) const																			{ return (ID == obj.ID) && (Name == obj.Name) && (Attributes == obj.Attributes); }
	bool operator!=(const tObject& obj) const																			{ return !(*this == obj); }
	tObject& operator=(const tObject& src)																				{ Clear(); ID = src.ID; Name = src.Name; Attributes = src.Attributes; return *this; }

	// The ID must be unique for all objects of the same type. Zero is a valid ID. 0xFFFFFFFF is the only invalid ID.
	const static uint32 InvalidID = 0xFFFFFFFF;
	uint32 ID = 0xFFFFFFFF;
	tString Name;
	tAttributes Attributes;
};


}
