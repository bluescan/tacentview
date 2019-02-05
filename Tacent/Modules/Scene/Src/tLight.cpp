// tLight.cpp
//
// This file implements scene lights. It supports point, spot, directional, and ambient lights.
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

#include "Scene/tLight.h"
namespace tScene
{


void tLight::Clear()
{
	tObject::Clear();
	Type = tType::Point;
	AttenNearStart = 0.0f;
	AttenNearEnd = 0.0f;
	AttenFarStart = 0.0f;
	AttenFarEnd = 0.0f;
}


void tLight::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Light);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_LightType);
		{
			chunk.Write(Type);
		}
		chunk.End();

		chunk.Begin(tChunkID::Scene_LightParameters);
		{
			chunk.Write(AttenNearStart);
			chunk.Write(AttenNearEnd);
			chunk.Write(AttenFarStart);
			chunk.Write(AttenFarEnd);
		}
		chunk.End();
	}
	chunk.End();
}


void tLight::Load(const tChunk& lightChunk)
{
	tAssert(lightChunk.ID() == tChunkID::Scene_Light);
	Clear();

	for (tChunk chunk = lightChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_LightType:
				chunk.GetItem(Type);
				break;

			case tChunkID::Scene_LightParameters:
				chunk.GetItem(AttenNearStart);
				chunk.GetItem(AttenNearEnd);
				chunk.GetItem(AttenFarStart);
				chunk.GetItem(AttenFarEnd);
				break;
		}
	}
}


}
