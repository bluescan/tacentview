// tPolyModel.cpp
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

#include <Math/tFundamentals.h>
#include "Scene/tPolyModel.h"
using namespace tMath;
namespace tScene
{


void tPolyModel::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_PolyModel);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Core_Bool);
		{
			chunk.Write(IsLodGroupMember);
		}
		chunk.End();

		Mesh.Save(chunk);
	}
	chunk.End();
}


void tPolyModel::Load(const tChunk& modelChunk)
{
	tAssert(modelChunk.ID() == tChunkID::Scene_PolyModel);
	Clear();

	for (tChunk chunk = modelChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Core_Bool:
				chunk.GetItem(IsLodGroupMember);
				break;

			case tChunkID::Scene_Mesh:
				Mesh.Load(chunk);
				break;
		}
	}
}


float tPolyModel::ComputeBoundingRadius() const
{
	float maxRadiusSq = 0.0f;
	for (int v = 0; v < Mesh.NumVertPositions; v++)
	{
		float distSq = Mesh.VertTablePositions[v].LengthSq();
		if (distSq > maxRadiusSq)
			maxRadiusSq = distSq;
	}

	return tSqrt(maxRadiusSq);
}


void tPolyModel::ComputeMinMaxRadius(float& minRadius, float& maxRadius) const
{
	float minRadiusSq = Infinity;
	float maxRadiusSq = 0.0f;
	for (int v = 0; v < Mesh.NumVertPositions; v++)
	{
		float distSq = Mesh.VertTablePositions[v].LengthSq();
		if (distSq > maxRadiusSq)
			maxRadiusSq = distSq;

		if (distSq < minRadiusSq)
			minRadiusSq = distSq;
	}

	minRadius = tSqrt(minRadiusSq);
	maxRadius = tSqrt(maxRadiusSq);
}


tBox tPolyModel::ComputeBoundingBox() const
{
	tBox boundingBox;

	for (int v = 0; v < Mesh.NumVertPositions; v++)
		boundingBox.AddPoint(Mesh.VertTablePositions[v]);

	return boundingBox;
}


float tPolyModel::ComputeApproxArea() const
{
	// See http://mathworld.wolfram.com/TriangleArea.html and http://mathworld.wolfram.com/Circumradius.html
	float area = 0.0f;
	for (int f = 0; f < Mesh.NumFaces; f++)
	{
		int start = f*3;
		tMath::tTriFace& face = Mesh.FaceTableVertPositionIndices[f];

		tVector3& v0 = Mesh.VertTablePositions[face.Index[0]];
		tVector3& v1 = Mesh.VertTablePositions[face.Index[1]];
		tVector3& v2 = Mesh.VertTablePositions[face.Index[2]];
		tVector3 A = v1 - v0;	float a = A.NormalizeGetLength();
		tVector3 B = v2 - v0;	float b = B.NormalizeGetLength();
		tVector3 C = v1 - v2;	float c = C.NormalizeGetLength();

		// The link above explains this name.
		float circumradius = a*b*c / tMath::tSqrt((a+b+c)*(b+c+-a)*(c+a-b)*(a+b-c));
		float triArea = a*b*c / (4.0f * circumradius);
		area += triArea;
	}

	return area;
}


}
