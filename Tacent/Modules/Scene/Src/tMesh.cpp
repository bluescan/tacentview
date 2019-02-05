// tMesh.cpp
//
// This file implements a mesh. A tMesh is not a tObject, but rather a member of a tPolyModel.
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

#include <System/tChunk.h>
#include "Scene/tMesh.h"
using namespace tStd;
using namespace tMath;
namespace tScene
{


tMesh& tMesh::operator=(const tMesh& src)
{
	if (this == &src)
		return *this;
	Clear();

	NumFaces = src.NumFaces;
	FaceTableVertPositionIndices = src.FaceTableVertPositionIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableVertPositionIndices)
		tMemcpy(FaceTableVertPositionIndices, src.FaceTableVertPositionIndices, NumFaces * sizeof(tTriFace));

	FaceTableVertWeightSetIndices = src.FaceTableVertWeightSetIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableVertWeightSetIndices)
		tMemcpy(FaceTableVertWeightSetIndices, src.FaceTableVertWeightSetIndices, NumFaces * sizeof(tTriFace));

	FaceTableVertNormalIndices = src.FaceTableVertNormalIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableVertNormalIndices)
		tMemcpy(FaceTableVertNormalIndices, src.FaceTableVertNormalIndices, NumFaces * sizeof(tTriFace));

	FaceTableUVIndices = src.FaceTableUVIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableUVIndices)
		tMemcpy(FaceTableUVIndices, src.FaceTableUVIndices, NumFaces * sizeof(tTriFace));

	FaceTableNormalMapUVIndices = src.FaceTableNormalMapUVIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableNormalMapUVIndices)
		tMemcpy(FaceTableNormalMapUVIndices, src.FaceTableNormalMapUVIndices, NumFaces * sizeof(tTriFace));

	FaceTableColourIndices = src.FaceTableColourIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableColourIndices)
		tMemcpy(FaceTableColourIndices, src.FaceTableColourIndices, NumFaces * sizeof(tTriFace));

	FaceTableFaceNormals = src.FaceTableFaceNormals ? new tVector3[NumFaces] : nullptr;
	if (FaceTableFaceNormals)
		tMemcpy(FaceTableFaceNormals , src.FaceTableFaceNormals, NumFaces * sizeof(tVector3));

	FaceTableMaterialIDs = src.FaceTableMaterialIDs ? new uint32[NumFaces] : nullptr;
	if (FaceTableMaterialIDs)
		tMemcpy(FaceTableMaterialIDs, src.FaceTableMaterialIDs, NumFaces * sizeof(uint32));

	FaceTableTangentIndices = src.FaceTableTangentIndices ? new tTriFace[NumFaces] : nullptr;
	if (FaceTableTangentIndices)
		tMemcpy(FaceTableTangentIndices, src.FaceTableTangentIndices, NumFaces * sizeof(tTriFace));

	NumEdges = src.NumEdges;
	EdgeTableVertPositionIndices = src.EdgeTableVertPositionIndices ? new tEdge[NumEdges] : nullptr;
	if (EdgeTableVertPositionIndices)
		tMemcpy(EdgeTableVertPositionIndices, src.EdgeTableVertPositionIndices, NumEdges * sizeof(tEdge));

	NumVertPositions = src.NumVertPositions;
	VertTablePositions = src.VertTablePositions ? new tVector3[NumVertPositions] : nullptr;
	if (VertTablePositions)
		tMemcpy(VertTablePositions, src.VertTablePositions, NumVertPositions * sizeof(tVector3));

	NumVertWeightSets = src.NumVertWeightSets;
	VertTableWeightSets = src.VertTableWeightSets ? new tWeightSet[NumVertWeightSets] : nullptr;
	if (VertTableWeightSets)
		tMemcpy(VertTableWeightSets, src.VertTableWeightSets, NumVertWeightSets * sizeof(tWeightSet));

	NumVertNormals = src.NumVertNormals;
	VertTableNormals = src.VertTableNormals ? new tVector3[NumVertNormals] : nullptr;
	if (VertTableNormals)
		tMemcpy(VertTableNormals, src.VertTableNormals, NumVertNormals * sizeof(tVector3));

	NumVertUVs = src.NumVertUVs;
	VertTableUVs = src.VertTableUVs ? new tVector2[NumVertUVs] : nullptr;
	if (VertTableUVs)
		tMemcpy(VertTableUVs, src.VertTableUVs, NumVertUVs * sizeof(tVector2));

	NumVertNormalMapUVs = src.NumVertNormalMapUVs;
	VertTableNormalMapUVs = src.VertTableNormalMapUVs ? new tVector2[NumVertNormalMapUVs] : nullptr;
	if (VertTableNormalMapUVs)
		tMemcpy(VertTableNormalMapUVs, src.VertTableNormalMapUVs, NumVertNormalMapUVs * sizeof(tVector2));

	NumVertColours = src.NumVertColours;
	VertTableColours = src.VertTableColours ? new tColouri[NumVertColours] : nullptr;
	if (VertTableColours)
		tMemcpy(VertTableColours, src.VertTableColours, NumVertColours * sizeof(tColouri));

	NumVertTangents = src.NumVertTangents;
	VertTableTangents = src.VertTableTangents ? new tVector4[NumVertTangents] : nullptr;
	if (VertTableTangents)
		tMemcpy(VertTableTangents, src.VertTableTangents, NumVertTangents * sizeof(tVector4));

	return *this;
}


void tMesh::Clear()
{
	SetNumFaces(0);
	DestroyFaceTableVertPositionIndices();
	DestroyFaceTableVertWeightSetIndices();
	DestroyFaceTableVertNormalIndices();
	DestroyFaceTableFaceNormals();
	DestroyFaceTableUVIndices();
	DestroyFaceTableNormalMapUVIndices();
	DestroyFaceTableColourIndices();
	DestroyFaceTableMaterialIDs();
	DestroyFaceTableTangentIndices();

	SetNumEdges(0);
	DestroyEdgeTableVertPositionIndices();

	SetNumVertPositions(0);
	DestroyVertTablePositions();

	SetNumVertWeightSets(0);
	DestroyVertTableWeightSets();

	SetNumVertNormals(0);
	DestroyVertTableNormals();

	SetNumVertUVs(0);
	DestroyVertTableUVs();

	SetNumVertNormalMapUVs(0);
	DestroyVertTableNormalMapUVs();

	SetNumVertColours(0);
	DestroyVertTableColours();

	SetNumVertTangents(0);
	DestroyVertTableTangents();
}


void tMesh::Load(const tChunk& meshChunk)
{
	tAssert(meshChunk.ID() == tChunkID::Scene_Mesh);
	Clear();

	for (tChunk chunk = meshChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_MeshProperties:
				chunk.GetItem(NumFaces);
				chunk.GetItem(NumEdges);
				chunk.GetItem(NumVertPositions);
				chunk.GetItem(NumVertWeightSets);
				chunk.GetItem(NumVertNormals);
				chunk.GetItem(NumVertUVs);
				chunk.GetItem(NumVertNormalMapUVs);
				chunk.GetItem(NumVertColours);
				chunk.GetItem(NumVertTangents);
				break;

			case tChunkID::Scene_FaceTable_VertPositionIndices:
			{
				tAssert(NumFaces);
				FaceTableVertPositionIndices = new tTriFace[NumFaces];
				tTriFace* faces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableVertPositionIndices[f] = faces[f];

				break;
			}

			case tChunkID::Scene_FaceTable_VertWeightSetIndices:
			{
				tAssert(NumFaces);
				FaceTableVertWeightSetIndices = new tTriFace[NumFaces];
				tTriFace* faces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableVertWeightSetIndices[f] = faces[f];

				break;
			}

			case tChunkID::Scene_FaceTable_VertNormalIndices:
			{
				tAssert(NumFaces);
				FaceTableVertNormalIndices = new tTriFace[NumFaces];
				tTriFace* normalFaces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableVertNormalIndices[f] = normalFaces[f];

				break;
			}

			case tChunkID::Scene_FaceTable_FaceNormals:
			{
				tAssert(NumFaces);
				FaceTableFaceNormals = new tVector3[NumFaces];
				tVector3* normals = (tVector3*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableFaceNormals[f] = normals[f];

				break;
			}

			case tChunkID::Scene_FaceTable_TexCoordIndices:
			{
				tAssert(NumFaces);
				FaceTableUVIndices = new tTriFace[NumFaces];
				tTriFace* uvFaces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableUVIndices[f] = uvFaces[f];

				break;
			}

			case tChunkID::Scene_FaceTable_NormalMapTexCoordIndices:
			{
				tAssert(NumFaces);
				FaceTableNormalMapUVIndices = new tTriFace[NumFaces];
				tTriFace* normalMapUVFaces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableNormalMapUVIndices[f] = normalMapUVFaces[f];

				break;
			}

			case tChunkID::Scene_FaceTable_ColourIndices:
			{
				tAssert(NumFaces);
				FaceTableColourIndices = new tTriFace[NumFaces];
				tTriFace* colourFaces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableColourIndices[f] = colourFaces[f];

				break;
			}

			case tChunkID::Scene_FaceTable_MaterialIDs:
			{
				tAssert(NumFaces);
				FaceTableMaterialIDs = new uint32[NumFaces];
				uint32* matIDs = (uint32*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableMaterialIDs[f] = matIDs[f];

				break;
			}

			case tChunkID::Scene_FaceTable_TangentIndices:
			{
				tAssert(NumFaces);
				FaceTableTangentIndices = new tTriFace[NumFaces];
				tTriFace* tangentFaces = (tTriFace*)chunk.Data();
				for (int f = 0; f < NumFaces; f++)
					FaceTableTangentIndices[f] = tangentFaces[f];

				break;
			}

			case tChunkID::Scene_EdgeTable_VertPositionIndices:
			{
				tAssert(NumEdges);
				EdgeTableVertPositionIndices = new tEdge[NumEdges];
				tEdge* edges = (tEdge*)chunk.Data();
				for (int e = 0; e < NumEdges; e++)
					EdgeTableVertPositionIndices[e] = edges[e];

				break;
			}

			case tChunkID::Scene_VertTable_Positions:
			{
				tAssert(NumVertPositions);
				VertTablePositions = new tVector3[NumVertPositions];
				tVector3* verts = (tVector3*)chunk.Data();
				for (int v = 0; v < NumVertPositions; v++)
					VertTablePositions[v] = verts[v];

				break;
			}

			case tChunkID::Scene_VertTable_WeightSets:
			{
				tAssert(NumVertWeightSets);
				VertTableWeightSets = new tWeightSet[NumVertWeightSets];
				tWeightSet* weightSets = (tWeightSet*)chunk.GetData();
				for (int s = 0; s < NumVertWeightSets; s++)
					VertTableWeightSets[s] = weightSets[s];

				break;
			}

			case tChunkID::Scene_VertTable_Normals:
			{
				tAssert(NumVertNormals);
				VertTableNormals = new tVector3[NumVertNormals];
				tVector3* normals = (tVector3*)chunk.Data();
				for (int n = 0; n < NumVertNormals; n++)
					VertTableNormals[n] = normals[n];

				break;
			}

			case tChunkID::Scene_VertTable_TexCoords:
			{
				tAssert(NumVertUVs);
				VertTableUVs = new tVector2[NumVertUVs];
				tVector2* uvs = (tVector2*)chunk.Data();
				for (int uv = 0; uv < NumVertUVs; uv++)
					VertTableUVs[uv] = uvs[uv];

				break;
			}

			case tChunkID::Scene_VertTable_NormalMapTexCoords:
			{
				tAssert(NumVertNormalMapUVs);
				VertTableNormalMapUVs = new tVector2[NumVertNormalMapUVs];
				tVector2* uvs = (tVector2*)chunk.Data();
				for (int uv = 0; uv < NumVertNormalMapUVs; uv++)
					VertTableNormalMapUVs[uv] = uvs[uv];

				break;
			}

			case tChunkID::Scene_VertTable_Colours:
			{
				tAssert(NumVertColours);
				VertTableColours = new tColouri[NumVertColours];
				tColouri* colours = (tColouri*)chunk.Data();
				for (int c = 0; c < NumVertColours; c++)
					VertTableColours[c] = colours[c];

				break;
			}

			case tChunkID::Scene_VertTable_Tangents:
			{
				tAssert(NumVertTangents);
				VertTableTangents = new tVector4[NumVertTangents];
				tVector4* tangents = (tVector4*)chunk.Data();
				for (int t = 0; t < NumVertTangents; t++)
					VertTableTangents[t] = tangents[t];

				break;
			}
		}
	}
}


void tMesh::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Mesh);
	{
		chunk.Begin(tChunkID::Scene_MeshProperties);
		{
			chunk.Write(NumFaces);
			chunk.Write(NumEdges);
			chunk.Write(NumVertPositions);
			chunk.Write(NumVertWeightSets);
			chunk.Write(NumVertNormals);
			chunk.Write(NumVertUVs);
			chunk.Write(NumVertNormalMapUVs);
			chunk.Write(NumVertColours);
			chunk.Write(NumVertTangents);
		}
		chunk.End();

		// Table chunks only written if table is present.
		if (FaceTableVertPositionIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_VertPositionIndices);
			chunk.Write(FaceTableVertPositionIndices, NumFaces);
			chunk.End();
		}

		if (FaceTableVertWeightSetIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_VertWeightSetIndices);
			chunk.Write(FaceTableVertWeightSetIndices, NumFaces);
			chunk.End();
		}

		if (FaceTableVertNormalIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_VertNormalIndices);
			chunk.Write(FaceTableVertNormalIndices, NumFaces);
			chunk.End();
		}

		if (FaceTableFaceNormals)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_FaceNormals);
			chunk.Write(FaceTableFaceNormals, NumFaces);
			chunk.End();
		}

		if (FaceTableUVIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_TexCoordIndices);
			chunk.Write(FaceTableUVIndices, NumFaces);
			chunk.End();
		}

		if (FaceTableNormalMapUVIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_NormalMapTexCoordIndices);
			chunk.Write(FaceTableNormalMapUVIndices, NumFaces);
			chunk.End();
		}

		if (FaceTableColourIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_ColourIndices);
			chunk.Write(FaceTableColourIndices, NumFaces);
			chunk.End();
		}

		if (FaceTableMaterialIDs)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_MaterialIDs);
			chunk.Write(FaceTableMaterialIDs, NumFaces);
			chunk.End();
		}

		if (FaceTableTangentIndices)
		{
			chunk.Begin(tChunkID::Scene_FaceTable_TangentIndices);
			chunk.Write(FaceTableTangentIndices, NumFaces);
			chunk.End();
		}

		if (EdgeTableVertPositionIndices)
		{
			chunk.Begin(tChunkID::Scene_EdgeTable_VertPositionIndices);
			chunk.Write(EdgeTableVertPositionIndices, NumEdges);
			chunk.End();
		}

		if (VertTablePositions)
		{
			chunk.Begin(tChunkID::Scene_VertTable_Positions);
			chunk.Write(VertTablePositions, NumVertPositions);
			chunk.End();
		}

		if (VertTableWeightSets)
		{
			chunk.Begin(tChunkID::Scene_VertTable_WeightSets);
			chunk.Write((char*)VertTableWeightSets, NumVertWeightSets * sizeof(tWeightSet));
			chunk.End();
		}

		if (VertTableNormals)
		{
			chunk.Begin(tChunkID::Scene_VertTable_Normals);
			chunk.Write(VertTableNormals, NumVertNormals);
			chunk.End();
		}

		if (VertTableUVs)
		{
			chunk.Begin(tChunkID::Scene_VertTable_TexCoords);
			chunk.Write(VertTableUVs, NumVertUVs);
			chunk.End();
		}

		if (VertTableNormalMapUVs)
		{
			chunk.Begin(tChunkID::Scene_VertTable_NormalMapTexCoords);
			chunk.Write(VertTableNormalMapUVs, NumVertNormalMapUVs);
			chunk.End();
		}

		if (VertTableColours)
		{
			chunk.Begin(tChunkID::Scene_VertTable_Colours);
			chunk.Write(VertTableColours, NumVertColours);
			chunk.End();
		}

		if (VertTableTangents)
		{
			chunk.Begin(tChunkID::Scene_VertTable_Tangents);
			chunk.Write(VertTableTangents, NumVertTangents);
			chunk.End();
		}
	}
	chunk.End();
}


void tMesh::Scale(float scale)
{
	for (int v = 0; v < NumVertPositions; v++)
		VertTablePositions[v] *= scale;
}


void tMesh::ReverseWinding()
{
	for (int f = 0; f < NumFaces; f++)
	{
		if (FaceTableVertPositionIndices)
			tStd::tSwap(FaceTableVertPositionIndices[f].Index[0], FaceTableVertPositionIndices[f].Index[2]);

		if (FaceTableVertWeightSetIndices)
			tStd::tSwap(FaceTableVertWeightSetIndices[f].Index[0], FaceTableVertWeightSetIndices[f].Index[2]);

		if (FaceTableVertNormalIndices)
			tStd::tSwap(FaceTableVertNormalIndices[f].Index[0], FaceTableVertNormalIndices[f].Index[2]);

		if (FaceTableUVIndices)
			tStd::tSwap(FaceTableUVIndices[f].Index[0], FaceTableUVIndices[f].Index[2]);

		if (FaceTableNormalMapUVIndices)
			tStd::tSwap(FaceTableNormalMapUVIndices[f].Index[0], FaceTableNormalMapUVIndices[f].Index[2]);

		if (FaceTableColourIndices)
			tStd::tSwap(FaceTableColourIndices[f].Index[0], FaceTableColourIndices[f].Index[2]);

		if (FaceTableTangentIndices)
			tStd::tSwap(FaceTableTangentIndices[f].Index[0], FaceTableTangentIndices[f].Index[2]);
	}
}


}
