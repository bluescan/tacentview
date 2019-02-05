// tPolyModel.h
//
// This file implements tScene polygonal models. A tPolyModel contains a tMesh and a functional interface over it.
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
#include "Scene/tMesh.h"
namespace tScene
{


class tPolyModel : public tObject
{
public:
	tPolyModel()																										{ }
	tPolyModel(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tPolyModel()																								{ }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);
	void Clear()																										{ tObject::Clear(); IsLodGroupMember = false; Mesh.Clear(); }
	void Scale(float scale)																								{ Mesh.Scale(scale); }

	// The bounding radius is the same as the maxRadius, but if you need both it's more efficient to use the
	// ComputeMinMaxRadius function.
	float ComputeBoundingRadius() const;
	void ComputeMinMaxRadius(float& minRadius, float& maxRadius) const;
	tMath::tBox ComputeBoundingBox() const;
	float ComputeApproxArea() const;
	tPolyModel& operator=(const tPolyModel&);

	bool IsLodGroupMember = false;		// This will be true if this model is a member of any LOD group.
	tMesh Mesh;
};


// Implementation below this line.


inline tPolyModel& tPolyModel::operator=(const tPolyModel& src)
{
	if (this == &src)
		return *this;

	tObject::operator=(src);
	IsLodGroupMember = src.IsLodGroupMember;
	Mesh = src.Mesh;
}


}