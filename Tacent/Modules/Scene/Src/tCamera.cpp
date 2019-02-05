// tCamera.cpp
//
// The camera tScene object. The primary members are a 4x4 homogeneous projection matrix and a type member that
// indicates the type of projection matrix being stored. We could add depth-of-field members at a later time.
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

#include "Scene/tCamera.h"
namespace tScene
{


void tCamera::Scale(float scale)
{
	switch (ProjectionMode)
	{
		case tProjectionMode::Perspective:
		case tProjectionMode::PerspectiveOblique:
		{
			float fovV, fovH, aspect, nearPlane, farPlane, offsetX, offsetY;
			Projection.ExtractPerspective(fovV, fovH, aspect, nearPlane, farPlane, offsetX, offsetY);
			nearPlane *= scale;
			farPlane *= scale;
			Projection.MakeProjPerspFovVOffset(fovV, aspect, nearPlane, farPlane, offsetX, offsetY);
			break;
		}

		case tProjectionMode::Parallel:
		{
			tMath::tMatrix4 scaleMat;
			scaleMat.MakeScale(scale);
			Projection = scale*Projection;
			break;
		}
	}
}


void tCamera::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Camera);
	{
		chunk.Begin(tChunkID::Scene_Object);
		tObject::Save(chunk);
		chunk.End();

		chunk.Begin(tChunkID::Scene_CameraParameters);
		chunk.Write(ProjectionMode);
		chunk.End();

		chunk.Begin(tChunkID::Scene_Projection);
		chunk.Write(Projection);
		chunk.End();
	}
	chunk.End();
}


void tCamera::Load(const tChunk& camChunk)
{
	tAssert(camChunk.ID() == tChunkID::Scene_Camera);
	Clear();

	for (tChunk chunk = camChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_CameraParameters:
				chunk.GetItem(ProjectionMode);
				break;

			case tChunkID::Scene_Projection:
				chunk.GetItem(Projection);
				break;
		}
	}
}


}
