// tMesh.h
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

#pragma once
namespace tScene
{


struct tVertWeight
{
	bool operator==(const tVertWeight& w) const																			{ return ((SkeletonID == w.SkeletonID) && (JointID == w.JointID) && (Weight == w.Weight)) ? true : false; }
	bool operator!=(const tVertWeight& w) const																			{ return !(*this == w); }

	uint32 SkeletonID;
	uint32 JointID;
	float Weight;
};


struct tWeightSet
{
	tWeightSet()																										{ tStd::tMemset(this, 0, sizeof(tWeightSet)); }
	bool operator==(const tWeightSet& s) const																			{ if (NumWeights != s.NumWeights) return false; for (int i = 0; i < NumWeights; i++) if (Weights[i] != s.Weights[i]) return false; return true; }
	bool operator!=(const tWeightSet& s) const																			{ return !(*this == s); }

	// The number of weights that are non-zero in the weight table.
	int NumWeights;

	// Max number of joints that can affect a single vertex. If you change this, you'll need to re-export everything.
	static const int MaxJointInfluences = 8;
	tVertWeight Weights[MaxJointInfluences];
};


class tMesh
{
public:
	tMesh()																												{ }
	tMesh(const tChunk& chunk)																							{ Load(chunk); }
	tMesh(const tMesh& src)																								{ *this = src; }
	virtual ~tMesh()																									{ }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);
	void Clear();
	void Scale(float);

	// Reverses the winding order of all face index tables. That is, those tables that are arrays of tTriFaces.
	void ReverseWinding();
	tMesh& operator=(const tMesh&);

	// Faces. Note that some tables may be nullptr. If a particular table does exist it will have NumFaces elements.
	// The Create table functions assume that the number of faces has been previously set. The created table is
	// uninitialized -- you must populate it. If num faces is 0 calling create will destroy the table. Setting the
	// number of faces does _not_ modify or delete the associated tables.
	void SetNumFaces(int numFaces)																						{ NumFaces = numFaces; }
	int GetNumFaces() const																								{ return NumFaces; }
	void CreateFaceTableVertPositionIndices()																			{ DestroyFaceTableVertPositionIndices(); if (NumFaces) FaceTableVertPositionIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableVertPositionIndices()																			{ delete[] FaceTableVertPositionIndices; FaceTableVertPositionIndices = nullptr; }
	void CreateFaceTableVertWeightSetIndices()																			{ DestroyFaceTableVertWeightSetIndices(); if (NumFaces) FaceTableVertWeightSetIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableVertWeightSetIndices()																			{ delete[] FaceTableVertWeightSetIndices; FaceTableVertWeightSetIndices = nullptr; }
	void CreateFaceTableVertNormalIndices()																				{ DestroyFaceTableVertNormalIndices(); if (NumFaces) FaceTableVertNormalIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableVertNormalIndices()																			{ delete[] FaceTableVertNormalIndices; FaceTableVertNormalIndices = nullptr; }
	void CreateFaceTableFaceNormals()																					{ DestroyFaceTableFaceNormals(); if (NumFaces) FaceTableFaceNormals = new tMath::tVector3[NumFaces]; }
	void DestroyFaceTableFaceNormals()																					{ delete[] FaceTableFaceNormals; FaceTableFaceNormals = nullptr; }
	void CreateFaceTableUVIndices()																						{ DestroyFaceTableUVIndices(); if (NumFaces) FaceTableUVIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableUVIndices()																					{ delete[] FaceTableUVIndices; FaceTableUVIndices = nullptr; }
	void CreateFaceTableNormalMapUVIndices()																			{ DestroyFaceTableNormalMapUVIndices(); if (NumFaces) FaceTableNormalMapUVIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableNormalMapUVIndices()																			{ delete[] FaceTableNormalMapUVIndices; FaceTableNormalMapUVIndices = nullptr; }
	void CreateFaceTableColourIndices()																					{ DestroyFaceTableColourIndices(); if (NumFaces) FaceTableColourIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableColourIndices()																				{ delete[] FaceTableColourIndices; FaceTableColourIndices = nullptr; }
	void CreateFaceTableMaterialIDs()																					{ DestroyFaceTableMaterialIDs(); if (NumFaces) FaceTableMaterialIDs = new uint32[NumFaces]; }
	void DestroyFaceTableMaterialIDs()																					{ delete[] FaceTableMaterialIDs; FaceTableMaterialIDs = nullptr; }
	void CreateFaceTableTangentIndices()																				{ DestroyFaceTableTangentIndices(); if (NumFaces) FaceTableTangentIndices = new tMath::tTriFace[NumFaces]; }
	void DestroyFaceTableTangentIndices()																				{ delete[] FaceTableTangentIndices; FaceTableTangentIndices = nullptr; }

	int NumFaces;
	tMath::tTriFace* FaceTableVertPositionIndices = nullptr;	// Contains indices into the vert position table.
	tMath::tTriFace* FaceTableVertWeightSetIndices = nullptr;	// Contains indices into the vert weight table.
	tMath::tTriFace* FaceTableVertNormalIndices = nullptr;		// Contains indices into the vertex normal table.
	tMath::tVector3* FaceTableFaceNormals = nullptr;			// Contains unit length face normals.
	tMath::tTriFace* FaceTableUVIndices = nullptr;				// Contains indices into the UV table.
	tMath::tTriFace* FaceTableNormalMapUVIndices = nullptr;		// Contains indices into the normal/bump map UV table.
	tMath::tTriFace* FaceTableColourIndices = nullptr;			// Contains indices into the colour table.
	uint32* FaceTableMaterialIDs = nullptr;						// Contains material ids.
	tMath::tTriFace* FaceTableTangentIndices = nullptr;			// Contains indices into the table of tangents.

	// Edges.
	void SetNumEdges(int numEdges)																						{ NumEdges = numEdges; }
	int GetNumEdges() const																								{ return NumEdges; }
	int FindEdgeIndex(const tMath::tEdge& edge) const																	{ for (int e = 0; e < NumEdges; e++) if (EdgeTableVertPositionIndices[e] == edge) return e; return -1; }
	void CreateEdgeTableVertPositionIndices()																			{ DestroyEdgeTableVertPositionIndices(); if (NumEdges) EdgeTableVertPositionIndices = new tMath::tEdge[NumEdges]; }
	void DestroyEdgeTableVertPositionIndices()																			{ delete[] EdgeTableVertPositionIndices; EdgeTableVertPositionIndices = nullptr; }
	int NumEdges;
	tMath::tEdge* EdgeTableVertPositionIndices = nullptr;		// Contains indices into the vert pos table, 2 per edge.
	
	// Vertices.
	void SetNumVertPositions(int numVertPositions)																		{ NumVertPositions = numVertPositions; }
	int GetNumVertPositions() const																						{ return NumVertPositions; }
	int FindVertPositionIndex(const tMath::tVector3& pos) const															{ for (int p = 0; p < NumVertPositions; p++) if (VertTablePositions[p] == pos) return p; return -1; }
	void CreateVertTablePositions()																						{ DestroyVertTablePositions(); if (NumVertPositions) VertTablePositions = new tMath::tVector3[NumVertPositions]; }
	void DestroyVertTablePositions()																					{ delete[] VertTablePositions; VertTablePositions = nullptr; }
	int NumVertPositions;
	tMath::tVector3* VertTablePositions = nullptr;

	void SetNumVertWeightSets(int numVertWeightSets)																	{ NumVertWeightSets = numVertWeightSets; }
	int GetNumVertWeightSets() const																					{ return NumVertWeightSets; }
	int FindWeightSetIndex(const tWeightSet& set) const																	{ for (int s = 0; s < NumVertWeightSets; s++) if (VertTableWeightSets[s] == set) return s; return -1; }
	void CreateVertTableWeightSets()																					{ DestroyVertTableWeightSets(); if (NumVertWeightSets) VertTableWeightSets = new tWeightSet[NumVertWeightSets]; }
	void DestroyVertTableWeightSets()																					{ delete[] VertTableWeightSets; VertTableWeightSets = nullptr; }
	int NumVertWeightSets;
	tWeightSet* VertTableWeightSets = nullptr;

	void SetNumVertNormals(int numVertNormals)																			{ NumVertNormals = numVertNormals; }
	int GetNumVertNormals() const																						{ return NumVertNormals; }
	int FindVertNormalIndex(const tMath::tVector3& normal) const														{ for (int n = 0; n < NumVertNormals; n++) if (VertTableNormals[n] == normal) return n; return -1; }
	void CreateVertTableNormals()																						{ DestroyVertTableNormals(); if (NumVertNormals) VertTableNormals = new tMath::tVector3[NumVertNormals]; }
	void DestroyVertTableNormals()																						{ delete[] VertTableNormals; VertTableNormals = nullptr; }
	int NumVertNormals;
	tMath::tVector3* VertTableNormals = nullptr;

	void SetNumVertUVs(int numVertUVs)																					{ NumVertUVs = numVertUVs; }
	int GetNumVertUVs() const																							{ return NumVertUVs; }
	int FindVertUVIndex(const tMath::tVector2& uv) const																{ for (int u = 0; u < NumVertUVs; u++) if (VertTableUVs[u] == uv) return u; return -1; }
	void CreateVertTableUVs()																							{ DestroyVertTableUVs(); if (NumVertUVs) VertTableUVs = new tMath::tVector2[NumVertUVs]; }
	void DestroyVertTableUVs()																							{ delete[] VertTableUVs; VertTableUVs = nullptr; }
	int NumVertUVs;
	tMath::tVector2* VertTableUVs = nullptr;

	void SetNumVertNormalMapUVs(int numVertNormalMapUVs)																{ NumVertNormalMapUVs = numVertNormalMapUVs; }
	int GetNumVertNormalMapUVs() const																					{ return NumVertNormalMapUVs; }
	int FindNormalMapUVIndex(const tMath::tVector2& uv) const															{ for (int u = 0; u < NumVertNormalMapUVs; u++) if (VertTableNormalMapUVs[u] == uv) return u; return -1; }
	void CreateVertTableNormalMapUVs()																					{ DestroyVertTableNormalMapUVs(); if (NumVertNormalMapUVs) VertTableNormalMapUVs = new tMath::tVector2[NumVertNormalMapUVs]; }
	void DestroyVertTableNormalMapUVs()																					{ delete[] VertTableNormalMapUVs; VertTableNormalMapUVs = nullptr; }
	int NumVertNormalMapUVs;
	tMath::tVector2* VertTableNormalMapUVs = nullptr;

	void SetNumVertColours(int numVertColours)																			{ NumVertColours = numVertColours; }
	int GetNumVertColours() const																						{ return NumVertColours; }
	int FindVertColourIndex(const tColouri& colour) const																{ for (int c = 0; c < NumVertColours; c++) if (VertTableColours[c] == colour) return c; return -1; }
	void CreateVertTableColours()																						{ DestroyVertTableColours(); if (NumVertColours) VertTableColours = new tColouri[NumVertColours]; }
	void DestroyVertTableColours()																						{ delete[] VertTableColours; VertTableColours = nullptr; }
	int NumVertColours;
	tColouri* VertTableColours = nullptr;

	void SetNumVertTangents(int numVertTangents)																		{ NumVertTangents = numVertTangents; }
	int GetNumVertTangents() const																						{ return NumVertTangents; }
	int FindVertTangentIndex(const tMath::tVector4& tangent) const														{ for (int t = 0; t < NumVertTangents; t++) if (VertTableTangents[t] == tangent) return t; return -1; }
	void CreateVertTableTangents()																						{ DestroyVertTableTangents(); if (NumVertTangents) VertTableTangents = new tMath::tVector4[NumVertTangents]; }
	void DestroyVertTableTangents()																						{ delete[] VertTableTangents; VertTableTangents = nullptr; }
	int NumVertTangents;
	tMath::tVector4* VertTableTangents = nullptr;
};


}
