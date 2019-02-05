// tPath.cpp
//
// This file implements scene splines and paths.
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

#include "Scene/tPath.h"
using namespace tMath;
namespace tScene
{


tPath::tPath(const tBezierPath& src, const tString& name, uint32 id) :
	tObject(name, id)
{
	if (!src.IsValid())
		return;

	Type = tType::CubicBezier;
	IsClosed = src.IsClosed();

	NumControlVerts = src.GetNumControlVerts();
	tAssert((NumControlVerts >= 4) && (((NumControlVerts-1) % 3) == 0));

	ControlVerts = new tVector3[NumControlVerts];
	for (int cv = 0; cv < NumControlVerts; cv++)
		ControlVerts[cv] = src.GetControlVerts()[cv];
}


void tPath::Save(tChunkWriter& chunk) const
{
	if (!IsValid())
		return;
	tAssert((NumControlVerts > 0) && ControlVerts);

	chunk.Begin(tChunkID::Scene_Path);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_PathParameters);
		chunk.Write(Type);
		chunk.Write(NumControlVerts);
		chunk.Write(IsClosed);
		chunk.End();

		chunk.Begin(tChunkID::Scene_PathControlVertTable);
		chunk.Write(ControlVerts, NumControlVerts);
		chunk.End();
	}
	chunk.End();
}


void tPath::Load(const tChunk& splineChunk)
{
	tAssert(splineChunk.ID() == tChunkID::Scene_Path);
	Clear();

	for (tChunk chunk = splineChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_PathParameters:
				chunk.GetItem(Type);
				chunk.GetItem(NumControlVerts);
				chunk.GetItem(IsClosed);
				break;

			case tChunkID::Scene_PathControlVertTable:
				tAssert((NumControlVerts > 0) && !ControlVerts);
				ControlVerts = new tVector3[NumControlVerts];
				tStd::tMemcpy(ControlVerts, chunk.GetData(), sizeof(tVector3) * NumControlVerts);
				break;
		}
	}
}


}
