// tSelection.cpp
//
// This file implements scene selection sets. A selection is simply a collection of scene instances referenced by ID.
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

#include "Scene/tSelection.h"
namespace tScene
{


void tSelection::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Selection);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_SelectionInstanceIDs);
		{
			for (tItList<uint32>::Iter instIDIter = InstanceIDs.First(); instIDIter; ++instIDIter)
			{
				uint32* instID = instIDIter.GetObject();
				chunk.Write(*instID);
			}
		}
		chunk.End();
	}
	chunk.End();
}


void tSelection::Load(const tChunk& selSetChunk)
{
	tAssert(selSetChunk.ID() == tChunkID::Scene_Selection);
	Clear();

	for (tChunk chunk = selSetChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_SelectionInstanceIDs:
			{
				int numInstances = chunk.GetDataSize() / sizeof(uint32);
				for (int i = 0; i < numInstances; i++)
				{
					uint32 instID;
					chunk.GetItem(instID);
					InstanceIDs.Append(new uint32(instID));
				}
				break;
			}
		}
	}
}


}
