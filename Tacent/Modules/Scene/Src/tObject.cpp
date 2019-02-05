// tObject.cpp
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

#include "Scene/tObject.h"
namespace tScene
{


void tObject::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Object);
	{
		chunk.Begin(tChunkID::Core_ID);
		chunk.Write(ID);
		chunk.End();

		chunk.Begin(tChunkID::Core_Name);
		chunk.Write(Name);
		chunk.End();

		Attributes.Save(chunk);
	}
	chunk.End();
}


void tObject::Load(const tChunk& objChunk)
{
	Clear();
	tAssert(objChunk.ID() == tChunkID::Scene_Object);

	for (tChunk chunk = objChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Core_ID:
				chunk.GetItem(ID);
				break;

			case tChunkID::Core_Name:
				chunk.GetItem(Name);
				break;

			case tChunkID::Scene_AttributeList:
				Attributes.Load(chunk);
				break;
		}
	}
}


}
