// tLodGroup.cpp
//
// This unit implements scene LOD groups. LOD groups basically specify what model to use based on a size threshold.
// The threshold is based on screen-size, not distance. This is much more correct as it allows narrow camera FOVs
// without things looking pixelated.
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

#include "Scene/tLodGroup.h"
namespace tScene
{


void tLodParam::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_LodParam);
	{
		chunk.Write(ModelID);
		chunk.Write(Threshold);
	}
	chunk.End();
}


void tLodParam::Load(const tChunk& chunk)
{
	tAssert(chunk.GetID() == tChunkID::Scene_LodParam);
	chunk.GetItem(ModelID);
	chunk.GetItem(Threshold);
}


void tLodGroup::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_LodGroup);
	{
		if (!LodParams.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_LodParams);
			{
				for (tItList<tLodParam>::Iter info = LodParams.First(); info; ++info)
					info->Save(chunk);
			}
			chunk.End();
		}
	}
	chunk.End();
}


void tLodGroup::Load(const tChunk& lodChunk)
{
	tAssert(lodChunk.GetID() == tChunkID::Scene_LodGroup);
	Clear();

	for (tChunk chunk = lodChunk.First(); chunk.IsValid(); chunk = chunk.Next())
	{
		switch (chunk.GetID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_LodParams:
				for (tChunk info = chunk.First(); info.Valid(); info = info.Next())
				{
					switch (info.GetID())
					{
						case tChunkID::Scene_LodParam:
							LodParams.Append(new tLodParam(info));
							break;
					}
				}
				break;
		}
	}
}


}
