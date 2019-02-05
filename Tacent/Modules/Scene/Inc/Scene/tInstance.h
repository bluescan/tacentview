// tInstance.h
//
// This file implements tScene instances. An instance is essentially a transformation (position, rotation, scale) in
// addition to a reference to the object being instanced. In a tScene, this reference consists of the ID of a
// particular type of object.
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
#include "Scene/tObject.h"
namespace tScene
{


class tInstance : public tObject
{
public:
	tInstance()																											{ }
	tInstance(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tInstance()																								{ }

	void Clear()																										{ tObject::Clear(); ObjectType = tType::Unspecified; ObjectID = tObject::InvalidID; Transform.Identity(); }
	bool IsValid() const																								{ return (ObjectType != tType::Unspecified) ? true : false; }

	// This does not scale the object itself because when a world is scaled, the object being referred to is also
	// scaled. Here we just need to scale the translation.
	void Scale(float scale)																								{ tMath::tMul(Transform.C4, scale); Transform.C4.w = 1.0f; }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	enum class tType
	{
		Unspecified																										= -1,
		PolyModel,
		PatchModel,
		LodGroup,
		Camera,
		Light,
		Path,
		NumTypes
	};

	// This is the type and ID of the object being instanced.
	tType ObjectType = tType::Unspecified;
	uint32 ObjectID = tObject::InvalidID;

	// This is the transform of the instance. Places it in the world at a particular position, scale, and orientation.
	tMath::tMatrix4 Transform = tMath::tMatrix4::identity;
};


}
