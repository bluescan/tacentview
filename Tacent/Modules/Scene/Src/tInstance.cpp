// tInstance.cpp
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

#include "Scene/tInstance.h"
namespace tScene
{


void tInstance::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Instance);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_InstanceParameters);
		{
			chunk.Write(ObjectType);
			chunk.Write(ObjectID);
			chunk.Write(Transform);
		}
		chunk.End();
	}
	chunk.End();
}


void tInstance::Load(const tChunk& instChunk)
{
	tAssert(instChunk.ID() == tChunkID::Scene_Instance);
	Clear();

	for (tChunk chunk = instChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_InstanceParameters:
			{
				chunk.GetItem(ObjectType);
				chunk.GetItem(ObjectID);
				chunk.GetItem(Transform);
				break;
			}
		}
	}
}


}
