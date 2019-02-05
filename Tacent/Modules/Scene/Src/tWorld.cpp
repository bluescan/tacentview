// tWorld.cpp
//
// This class represents entire worlds of tObjects. Instances, materials, models, paths, lights, skeletons, and cameras
// may all be present in a tWorld. It is able to load multiple tChunk-based binary tac files. It can also save them.
//
// Every saved tac file contains a version chunk with both major and minor version numbers. Increment the minor number
// if there is a change to a tObject that does not break binary compatibility. For example, you can add a new chunk ID
// as long as loading it is optional. The major version number must be incremented if file compatibility is broken.
// This can happen if chunks are rearranged or chunk contents are altered. This allows an exporters to change, perhaps
// a new chunk was added, without the tac file parsing tool needing an upgrade immediately (forward compatibility). The
// minor version should be reset to 0 when the major number is incremented. Any tools, by looking at the major number,
// can maintain backwards compatibility.
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

#include "Scene/tWorld.h"
using namespace tMath;
using namespace tStd;
namespace tScene
{


void tWorld::Clear()
{
	Name.Clear();
	LastLoadedFilename.Clear();

	Cameras.Empty();
	Lights.Empty();
	Paths.Empty();
	Materials.Empty();
	Skeletons.Empty();
	PolyModels.Empty();
	LodGroups.Empty();
	Instances.Empty();
	Selections.Empty();

	// Empty scenes can be assumed to be at the current version.
	MajorVersion = SceneMajorVersion;
	MinorVersion = SceneMinorVersion;

	NextCameraID = 0;
	NextLightID = 0;
	NextPathID = 0;
	NextMaterialID = 0;
	NextPolyModelID = 0;
	NextSkeletonID = 0;
	NextLodGroupID = 0;
	NextInstanceID = 0;
	NextSelectionID = 0;
}


void tWorld::Scale(float scale)
{
	for (tItList<tCamera>::Iter camera = Cameras.First(); camera; ++camera)
		camera->Scale(scale);

	for (tItList<tLight>::Iter light = Lights.First(); light; ++light)
		light->Scale(scale);

	for (tItList<tPath>::Iter path = Paths.First(); path; ++path)
		path->Scale(scale);

	for (tItList<tPolyModel>::Iter model = PolyModels.First(); model; ++model)
		model->Scale(scale);

	for (tItList<tSkeleton>::Iter skeleton = Skeletons.First(); skeleton; ++skeleton)
		skeleton->Scale(scale);

	for (tItList<tInstance>::Iter instance = Instances.First(); instance; ++instance)
		instance->Scale(scale);
}


void tWorld::MergeScene(tWorld& sceneDestroyed)
{
	MergeItems
	(
		&sceneDestroyed.Cameras, &sceneDestroyed.Lights, &sceneDestroyed.Paths,
		&sceneDestroyed.Materials, &sceneDestroyed.Skeletons, &sceneDestroyed.PolyModels,
		&sceneDestroyed.LodGroups, &sceneDestroyed.Instances, &sceneDestroyed.Selections
	);
}


void tWorld::AddOffsetToAllIDs(uint32 offset)
{
	for (tItList<tCamera>::Iter it = Cameras.First(); it; ++it)
		it->ID += offset;

	for (tItList<tLight>::Iter it = Lights.First(); it; ++it)
		it->ID += offset;

	for (tItList<tPath>::Iter it = Paths.First(); it; ++it)
		it->ID += offset;

	for (tItList<tMaterial>::Iter it = Materials.First(); it; ++it)
		it->ID += offset;

	for (tItList<tSkeleton>::Iter it = Skeletons.First(); it; ++it)
		it->ID += offset;

	for (tItList<tPolyModel>::Iter it = PolyModels.First(); it; ++it)
	{
		tPolyModel* p = it;
		p->ID += offset;
		if (p->Mesh.FaceTableMaterialIDs)
		{
			for (int f = 0; f < p->Mesh.NumFaces; ++f)
				p->Mesh.FaceTableMaterialIDs[f] += offset;
		}
	}

	for (tItList<tLodGroup>::Iter it = LodGroups.First(); it; ++it)
		it->ID += offset;

	for (tItList<tInstance>::Iter it = Instances.First(); it; ++it)
	{
		it->ID += offset;
		it->ObjectID += offset;
	}

	for (tItList<tSelection>::Iter ss = Selections.First(); ss; ++ss)
	{
		ss->ID += offset;
		for (tItList<uint32>::Iter instID = ss->InstanceIDs.First(); instID; ++instID)
		{
			uint32* id = instID.GetObject();
			*id += offset;
		}
	}
}


bool tWorld::MergeItems
(
	tItList<tCamera>* cameras, tItList<tLight>* lights, tItList<tPath>* paths,
	tItList<tMaterial>* materials, tItList<tSkeleton>* skeletons, tItList<tPolyModel>* polyModels,
	tItList<tLodGroup>* lodGroups, tItList<tInstance>* instances, tItList<tSelection>* selections
)
{
	//@todo The tree hierarchy sanity-checking here and return false if we're screwed.
	tItList<tCamera> emptyCameras;
	tItList<tLight> emptyLights;
	tItList<tPath> emptyPaths;
	tItList<tMaterial> emptyMaterials;
	tItList<tSkeleton> emptySkeletons;
	tItList<tPolyModel> emptyPolyModels;
	tItList<tLodGroup> emptyLodGroups;
	tItList<tInstance> emptyInstances;
	tItList<tSelection> emptySelections;

	tItList<tCamera>& newCameras = cameras ? *cameras : emptyCameras;
	tItList<tLight>& newLights = lights ? *lights : emptyLights;
	tItList<tPath>& newPaths = paths ? *paths : emptyPaths;
	tItList<tMaterial>& newMaterials = materials ? *materials : emptyMaterials;
	tItList<tSkeleton>& newSkeletons = skeletons ? *skeletons : emptySkeletons;
	tItList<tPolyModel>& newPolyModels = polyModels ? *polyModels : emptyPolyModels;
	tItList<tLodGroup>& newLodGroups = lodGroups ? *lodGroups : emptyLodGroups;
	tItList<tInstance>& newInstances = instances ? *instances : emptyInstances;
	tItList<tSelection>& newSelections = selections ? *selections : emptySelections;

	// Make sure every type of object has a unique ID. Object lists that refer to the corrected lists are also fixed
	// up. The first arg is a list of items that need their IDs adjusted to make them unique -- the remaining arg(s)
	// are items that refer to those adjusted IDs and need to be updated.
	if (cameras)
		CorrectCameraIDs(newCameras, newInstances);

	if (lights)
		CorrectLightIDs(newLights, newInstances);

	if (paths)
		CorrectPathIDs(newPaths, newInstances);

	if (materials)
		CorrectMaterialIDs(newMaterials, newPolyModels);

	if (skeletons)
		CorrectSkeletonIDs(newSkeletons, newPolyModels);

	if (polyModels)
		CorrectPolyModelIDs(newPolyModels, newLodGroups, newInstances);

	// For now nothing refers to LOD groups. just keep IDs unique. Eventually instances may refer to these.
	if (lodGroups)
		CorrectLodGroupIDs(newLodGroups);

	if (instances)
		CorrectInstanceIDs(newInstances, newSelections);

	// Currently nothing refers to Selections. New selections that have the same name as existing selection get merged
	// into the existing. For selections it is based on the selection name.
	if (selections)
	{
		MergeToExistingSelections(newSelections);
		CorrectSelectionIDs(newSelections);
	}

	// We can now add the groups, cameras, lights, materials, skeletons, models, and instances to the scene.
	while (tCamera* c = newCameras.Remove())
		Cameras.Append(c);

	while (tLight* l = newLights.Remove())
		Lights.Append(l);

	while (tPath* s = newPaths.Remove())
		Paths.Append(s);

	while (tMaterial* m = newMaterials.Remove())
		Materials.Append(m);

	while (tSkeleton* s = newSkeletons.Remove())
		Skeletons.Append(s);

	while (tPolyModel* m = newPolyModels.Remove())
		PolyModels.Append(m);

	while (tLodGroup* l = newLodGroups.Remove())
		LodGroups.Append(l);

	while (tInstance* i = newInstances.Remove())
		Instances.Append(i);

	while (tSelection* s = newSelections.Remove())
		Selections.Append(s);

	return true;
}


bool tWorld::CombinePolyModelInstances(tItList<tInstance>& polymodelInstances, tString newInstName)
{
	tPolyModel* newModel = new tPolyModel();
	newModel->ID = NextPolyModelID++;
	newModel->Name = tString("ModelFor_") +  newInstName;
	newModel->IsLodGroupMember = false;

	tInstance* newInstance = new tInstance();
	newInstance->ID = NextInstanceID++;
	newInstance->Name = newInstName;
	newInstance->ObjectType = tInstance::tType::PolyModel;
	newInstance->ObjectID = newModel->ID;
	newInstance->Transform = tMatrix4::identity;	

	tMesh* newMesh = &newModel->Mesh;

	// Vert tables.
	int numVertsTotal = 0;
	int numFacesTotal = 0;
	int numEdgeTotal =0;
	int numWeightSetsTotal = 0;
	int numNormalsTotal =0;
	int numUVTotal =0;
	int numNMUVTotal = 0;
	int numColourTotal = 0;
	int numTangentTotal =0;
	
	tItList<tPolyModel> combinedModels;
	for (tItList<tInstance>::Iter it = polymodelInstances.First(); it; ++it)
	{
		tInstance* inst = it;
		tAssert(inst->ObjectType == tInstance::tType::PolyModel);
		tPolyModel* model = FindPolyModel(inst->ObjectID);

		combinedModels.Append(model);
		tMesh* mesh = &model->Mesh;

		numVertsTotal += mesh->NumVertPositions;
		numFacesTotal += mesh->NumFaces;
		numEdgeTotal += mesh->NumEdges;
		numWeightSetsTotal += mesh->NumVertWeightSets;
		numNormalsTotal += mesh->NumVertNormals;
		numUVTotal += mesh->NumVertUVs;
		numNMUVTotal += mesh->NumVertNormalMapUVs;
		numColourTotal += mesh->NumVertColours;
		numTangentTotal += mesh->NumVertTangents;
	}

	newMesh->NumVertPositions = numVertsTotal;
	newMesh->NumFaces = numFacesTotal;
	if (numVertsTotal == 0 || numFacesTotal == 0)
		return false;

	newMesh->NumEdges = numEdgeTotal;
	newMesh->NumVertWeightSets = numWeightSetsTotal;
	newMesh->NumVertNormals = numNormalsTotal;
	newMesh->NumVertUVs = numUVTotal;
	newMesh->NumVertNormalMapUVs = numNMUVTotal;
	newMesh->NumVertColours = numColourTotal;
	newMesh->NumVertTangents = numTangentTotal;

	newMesh->FaceTableVertPositionIndices	= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->FaceTableVertWeightSetIndices	= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->FaceTableVertNormalIndices		= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->FaceTableUVIndices				= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->FaceTableNormalMapUVIndices	= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->FaceTableColourIndices			= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->FaceTableFaceNormals			= newMesh->NumFaces > 0 ?				new tVector3[newMesh->NumFaces]						: nullptr;
	newMesh->FaceTableMaterialIDs			= newMesh->NumFaces > 0 ?				new uint32[newMesh->NumFaces]						: nullptr;
	newMesh->FaceTableTangentIndices		= newMesh->NumFaces > 0 ?				new tMath::tTriFace[newMesh->NumFaces]				: nullptr;
	newMesh->EdgeTableVertPositionIndices	= newMesh->NumEdges > 0 ?				new tMath::tEdge[newMesh->NumEdges]					: nullptr;
	newMesh->VertTablePositions				= newMesh->NumVertPositions > 0 ?		new tVector3[newMesh->NumVertPositions]				: nullptr;
	newMesh->VertTableWeightSets			= newMesh->NumVertWeightSets > 0 ?		new tWeightSet[newMesh->NumVertWeightSets]			: nullptr;
	newMesh->VertTableNormals				= newMesh->NumVertNormals > 0 ?			new tVector3[newMesh->NumVertNormals]				: nullptr;
	newMesh->VertTableUVs					= newMesh->NumVertUVs > 0 ?				new tMath::tVector2[newMesh->NumVertUVs]			: nullptr;
	newMesh->VertTableNormalMapUVs			= newMesh->NumVertNormalMapUVs > 0 ?	new tMath::tVector2[newMesh->NumVertNormalMapUVs]	: nullptr;
	newMesh->VertTableColours				= newMesh->NumVertColours > 0 ?			new tColouri[newMesh->NumVertColours]				: nullptr;
	newMesh->VertTableTangents				= newMesh->NumVertTangents > 0 ?		new tMath::tVector4[newMesh->NumVertTangents]		: nullptr;

	int curFace = 0;
	int curEdge = 0;
	int curVert = 0;
	int curWeightSet = 0;
	int curNormal = 0;
	int curUV = 0;
	int curNMUV = 0;
	int curColour = 0;
	int curTangent = 0;

	for (tItList<tInstance>::Iter it = polymodelInstances.First(); it;)
	{
		tItList<tInstance>::Iter next = it + 1;
		tInstance* inst = it;
		tAssert(inst->ObjectType == tInstance::tType::PolyModel);

		tPolyModel* model = FindPolyModel(inst->ObjectID);
		tMesh* mesh = &model->Mesh;

		for (int f = 0; f < mesh->NumFaces; ++f)
		{
			// Offset face indices.
			if (newMesh->FaceTableVertPositionIndices && mesh->FaceTableVertPositionIndices)
			{
				newMesh->FaceTableVertPositionIndices[f+curFace].Index[0] = mesh->FaceTableVertPositionIndices[f].Index[0] + curVert;
				newMesh->FaceTableVertPositionIndices[f+curFace].Index[1] = mesh->FaceTableVertPositionIndices[f].Index[1] + curVert;
				newMesh->FaceTableVertPositionIndices[f+curFace].Index[2] = mesh->FaceTableVertPositionIndices[f].Index[2] + curVert;
			}		

			if (newMesh->FaceTableVertWeightSetIndices && mesh->FaceTableVertWeightSetIndices)
			{
				newMesh->FaceTableVertWeightSetIndices[f+curFace].Index[0] = mesh->FaceTableVertWeightSetIndices[f].Index[0] + curWeightSet;
				newMesh->FaceTableVertWeightSetIndices[f+curFace].Index[1] = mesh->FaceTableVertWeightSetIndices[f].Index[1] + curWeightSet;
				newMesh->FaceTableVertWeightSetIndices[f+curFace].Index[2] = mesh->FaceTableVertWeightSetIndices[f].Index[2] + curWeightSet;
			}

			if (newMesh->FaceTableVertNormalIndices && mesh->FaceTableVertNormalIndices)
			{
				newMesh->FaceTableVertNormalIndices[f+curFace].Index[0] = mesh->FaceTableVertNormalIndices[f].Index[0] + curNormal;
				newMesh->FaceTableVertNormalIndices[f+curFace].Index[1] = mesh->FaceTableVertNormalIndices[f].Index[1] + curNormal;
				newMesh->FaceTableVertNormalIndices[f+curFace].Index[2] = mesh->FaceTableVertNormalIndices[f].Index[2] + curNormal;
			}

			if (newMesh->FaceTableUVIndices && mesh->FaceTableUVIndices)
			{
				newMesh->FaceTableUVIndices[f+curFace].Index[0] = mesh->FaceTableUVIndices[f].Index[0] + curUV;
				newMesh->FaceTableUVIndices[f+curFace].Index[1] = mesh->FaceTableUVIndices[f].Index[1] + curUV;
				newMesh->FaceTableUVIndices[f+curFace].Index[2] = mesh->FaceTableUVIndices[f].Index[2] + curUV;
			}	

			if (newMesh->FaceTableNormalMapUVIndices)
			{
				newMesh->FaceTableNormalMapUVIndices[f+curFace].Index[0] = mesh->FaceTableNormalMapUVIndices == 0 ? -1 : mesh->FaceTableNormalMapUVIndices[f].Index[0] + curNMUV;
				newMesh->FaceTableNormalMapUVIndices[f+curFace].Index[1] = mesh->FaceTableNormalMapUVIndices == 0 ? -1 : mesh->FaceTableNormalMapUVIndices[f].Index[1] + curNMUV;
				newMesh->FaceTableNormalMapUVIndices[f+curFace].Index[2] = mesh->FaceTableNormalMapUVIndices == 0 ? -1 : mesh->FaceTableNormalMapUVIndices[f].Index[2] + curNMUV;
			}	

			if (newMesh->FaceTableColourIndices)
			{
				newMesh->FaceTableColourIndices[f+curFace].Index[0] = mesh->FaceTableColourIndices == 0 ? -1 : mesh->FaceTableColourIndices[f].Index[0] + curColour;
				newMesh->FaceTableColourIndices[f+curFace].Index[1] = mesh->FaceTableColourIndices == 0 ? -1 : mesh->FaceTableColourIndices[f].Index[1] + curColour;
				newMesh->FaceTableColourIndices[f+curFace].Index[2] = mesh->FaceTableColourIndices == 0 ? -1 : mesh->FaceTableColourIndices[f].Index[2] + curColour;
			}

			if (newMesh->FaceTableTangentIndices && mesh->FaceTableTangentIndices)
			{
				newMesh->FaceTableTangentIndices[f+curFace].Index[0] = mesh->FaceTableTangentIndices[f].Index[0] + curTangent;
				newMesh->FaceTableTangentIndices[f+curFace].Index[1] = mesh->FaceTableTangentIndices[f].Index[1] + curTangent;
				newMesh->FaceTableTangentIndices[f+curFace].Index[2] = mesh->FaceTableTangentIndices[f].Index[2] + curTangent;
			}	
		}

		tMemcpy( &newMesh->FaceTableFaceNormals[curFace], mesh->FaceTableFaceNormals, sizeof(tVector3) * mesh->NumFaces);
		tMemcpy( &newMesh->FaceTableMaterialIDs[curFace], mesh->FaceTableMaterialIDs, sizeof(uint32) * mesh->NumFaces);
		tMemcpy( &newMesh->EdgeTableVertPositionIndices[curEdge], mesh->EdgeTableVertPositionIndices, mesh->NumEdges * sizeof(tEdge) );
		for (int e = 0; e < mesh->NumEdges; ++e)
		{
			newMesh->EdgeTableVertPositionIndices[curEdge + e ].Index[0] += curVert;
			newMesh->EdgeTableVertPositionIndices[curEdge + e ].Index[1] += curVert;
		}

		// Convert to world space.
		for (int v = 0; v < mesh->NumVertPositions; ++v)
			tMath::tMul(newMesh->VertTablePositions[curVert+v] , inst->Transform, mesh->VertTablePositions[v]);

		tMemcpy(&newMesh->VertTableWeightSets[curWeightSet],mesh->VertTableWeightSets,		mesh->NumVertWeightSets * sizeof(tWeightSet));

		tMatrix4 rotOnly = inst->Transform;
		tSet(rotOnly.C4, 0.0f, 0.0f, 0.0f, 1.0f);
		for (int n = 0; n < mesh->NumVertNormals; ++n)
		{
			// Convert to world space (rotation only).
			tMath::tMul(newMesh->VertTableNormals[curNormal+n] , rotOnly, mesh->VertTableNormals[n]);
			newMesh->VertTableNormals[curNormal+n].Normalize();
		}

		tMemcpy(&newMesh->VertTableUVs[curUV],				mesh->VertTableUVs,				mesh->NumVertUVs * sizeof(tVector2));
		tMemcpy(&newMesh->VertTableNormalMapUVs[curNMUV],	mesh->VertTableNormalMapUVs,	mesh->NumVertNormalMapUVs * sizeof(tVector2));
		tMemcpy(&newMesh->VertTableColours[curColour],		mesh->VertTableColours,			mesh->NumVertColours* sizeof(tColouri));
		tMemcpy(&newMesh->VertTableTangents[curTangent],	mesh->VertTableTangents,		mesh->NumVertTangents * sizeof(tVector4));
		
		curFace += mesh->NumFaces;
		curEdge += mesh->NumEdges;
		curVert += mesh->NumVertPositions;
		curWeightSet += mesh->NumVertWeightSets;
		curNormal += mesh->NumVertNormals;
		curUV += mesh->NumVertUVs;
		curNMUV += mesh->NumVertNormalMapUVs;
		curColour += mesh->NumVertColours;
		curTangent += mesh->NumVertTangents;

		Instances.Remove( it );
		delete inst;

		it = next;
	}

	// Now remove any models left stranded.
	tItList<tPolyModel> strandedModels;
	for (tItList<tPolyModel>::Iter it = combinedModels.First(); it;)
	{
		tItList<tPolyModel>::Iter next = it + 1;
		tPolyModel* model = it;

		bool stranded = true;
		for (tItList<tInstance>::Iter it2 = Instances.First(); stranded && it2; ++it2)
		{
			tInstance* inst = it2;
			if (inst->ObjectType != tInstance::tType::PolyModel)
				continue;

			if (inst->ObjectID == model->ID)
				stranded = false;
		}

		if (stranded)
			strandedModels.Append(model);

		it = next;
	}

	for (tItList< tPolyModel >::Iter it = PolyModels.First(); it;)
	{
		tItList<tPolyModel>::Iter next = it+1;
		tPolyModel* m1 = it;
		for (tItList<tPolyModel>::Iter it2 = strandedModels.First(); it2; ++it2)
		{
			tPolyModel* m2 = it2;
			if (m1 == m2)
			{
				PolyModels.Remove(it);
				delete m1;
				break;
			}
		}
		it = next;
	}

	Instances.Append(newInstance);
	PolyModels.Append(newModel);
	return true;
}


void tWorld::Load(const tString& tacFile, uint32 loadFilter)
{
	Name = tacFile;
	LastLoadedFilename = tacFile;
	tChunkReader tac(tacFile);

	tItList<tCamera>		newCameras;
	tItList<tLight>			newLights;
	tItList<tPath>			newPaths;
	tItList<tMaterial>		newMaterials;
	tItList<tSkeleton>		newSkeletons;
	tItList<tPolyModel>		newPolyModels;
	tItList<tLodGroup>		newLodGroups;
	tItList<tInstance>		newInstances;
	tItList<tSelection>		newSelections;

	for (tChunk tacChunk = tac.First(); tacChunk.Valid(); tacChunk = tacChunk.Next())
	{
		switch (tacChunk.ID())
		{
			case tChunkID::Core_Version:
				tacChunk.GetItem(MajorVersion);
				tacChunk.GetItem(MinorVersion);
				break;

			case tChunkID::Scene_Scene:
			{
				for (tChunk chunk = tacChunk.First(); chunk.Valid(); chunk = chunk.Next())
				{
					switch (chunk.ID())
					{
						case tChunkID::Scene_MaterialList:
							if (loadFilter & tLoadFilter_Materials)
								LoadMaterials(chunk, newMaterials);
							break;

						case tChunkID::Scene_ObjectList:
							LoadObjects(chunk, newPolyModels, newSkeletons, newCameras, newLights, newPaths, loadFilter);
							break;

						case tChunkID::Scene_GroupList:
							if (loadFilter & tLoadFilter_LodGroups)
								LoadGroups(chunk, newLodGroups);
							break;

						case tChunkID::Scene_InstanceList:
							if (loadFilter & tLoadFilter_Instances)
								LoadInstances(chunk, newInstances);
							break;

						case tChunkID::Scene_SelectionList:
							if (loadFilter & tLoadFilter_Selections)
								LoadSelections(chunk, newSelections);
							break;
					}
				}
				break;
			}
		}
	}

	// Fixup external references. The main external references are diffuse texture files found in the materials and the
	// shader file (shd).
	tString tacDir = tSystem::tGetDir(tacFile);

	for (tItList<tMaterial>::Iter m = newMaterials.First(); m; ++m)
	{
		// We need to simplify paths by removing any up-directory markers ".." and same-directory markers ".". This
		// step is essential since two different tac files may refer to the same external file so we want the path to
		// the file to look the same no matter the original tac file location. This allows correct optimization when
		// checking for duplicate materials.
		if (!m->TextureDiffuse.IsEmpty())
			m->TextureDiffuse = tSystem::tGetSimplifiedPath(tacDir + m->TextureDiffuse);

		if (!m->TextureNormalMap.IsEmpty())
			m->TextureNormalMap = tSystem::tGetSimplifiedPath(tacDir + m->TextureNormalMap);

		if (!m->TextureA.IsEmpty())
			m->TextureA = tSystem::tGetSimplifiedPath(tacDir + m->TextureA);

		if (!m->TextureB.IsEmpty())
			m->TextureB = tSystem::tGetSimplifiedPath(tacDir + m->TextureB);

		if (!m->TextureC.IsEmpty())
			m->TextureC = tSystem::tGetSimplifiedPath(tacDir + m->TextureC);

		if (!m->TextureD.IsEmpty())
			m->TextureD = tSystem::tGetSimplifiedPath(tacDir + m->TextureD);

		if (!m->TextureE.IsEmpty())
			m->TextureE = tSystem::tGetSimplifiedPath(tacDir + m->TextureE);

		if (!m->ShaderFile.IsEmpty())
			m->ShaderFile = tSystem::tGetSimplifiedPath(tacDir + m->ShaderFile);
	}

	// Make sure every type of object has a unique ID. Object lists that refer to the corrected lists are also fixed
	// up. i.e. The first arg is a list of items that need their IDs adjusted to make them unique -- the remaining
	// arg(s) are items that refer to those adjusted IDs and need to be updated.
	CorrectCameraIDs(newCameras, newInstances);
	CorrectLightIDs(newLights, newInstances);
	CorrectPathIDs(newPaths, newInstances);
	CorrectMaterialIDs(newMaterials, newPolyModels);
	CorrectSkeletonIDs(newSkeletons, newPolyModels);

	CorrectPolyModelIDs(newPolyModels, newLodGroups, newInstances);

	// For now nothing refers to LOD groups -- just keep IDs unique. Eventually instances may refer to these.
	CorrectLodGroupIDs(newLodGroups);
	CorrectInstanceIDs(newInstances, newSelections);

	// Nothing refers to selections. New selections that have the same name as existing selections get merged into the
	// existing. For selections it is based on the selection name.
	MergeToExistingSelections(newSelections);
	CorrectSelectionIDs(newSelections);

	// We can now add the groups, cameras, lights, materials, skeletons, models, and instances to the scene.
	while (tCamera* c = newCameras.Remove())
		Cameras.Append(c);

	while (tLight* l = newLights.Remove())
		Lights.Append(l);

	while (tPath* s = newPaths.Remove())
		Paths.Append(s);

	while (tMaterial* m = newMaterials.Remove())
		Materials.Append(m);

	while (tSkeleton* s = newSkeletons.Remove())
		Skeletons.Append(s);

	while (tPolyModel* m = newPolyModels.Remove())
		PolyModels.Append(m);

	while (tLodGroup* l = newLodGroups.Remove())
		LodGroups.Append(l);

	while (tInstance* i = newInstances.Remove())
		Instances.Append(i);

	while (tSelection* s = newSelections.Remove())
		Selections.Append(s);
}


void tWorld::CorrectCameraIDs(tItList<tCamera>& newCameras, tItList<tInstance>& newInstances)
{
	int numNewCameras = newCameras.GetNumItems();
	if (!numNewCameras)
		return;

	uint32* newIDTable = new uint32[numNewCameras];

	// Decide on some new camera ID values.
	for (int i = 0; i < numNewCameras; i++)
		newIDTable[i] = NextCameraID++;

	// Correct all references to these IDs by the instances.
	for (tItList<tInstance>::Iter inst = newInstances.First(); inst; ++inst)
	{
		if (inst->ObjectType != tInstance::tType::Camera)
			continue;

		uint32 origID = inst->ObjectID;

		// Find the correct cameras.
		int newCameraIndex = 0;
		bool foundCamera = false;
		for (tItList<tCamera>::Iter camera = newCameras.First(); camera; ++camera)
		{
			if (origID == camera->ID)
			{
				foundCamera = true;
				break;
			}

			newCameraIndex++;
		}

		if (!foundCamera)
			throw tError("Could not find camera with ID %d. Could be that the list has 2 models with the same ID.", origID);

		inst->ObjectID = newIDTable[newCameraIndex];
	}

	// Assign the cameras their new ID.
	int index = 0;
	for (tItList<tCamera>::Iter l = newCameras.First(); l; ++l)
		l->ID = newIDTable[index++];

	delete[] newIDTable;
}


void tWorld::CorrectLightIDs(tItList<tLight>& newLights, tItList<tInstance>& newInstances)
{
	int numNewLights = newLights.GetNumItems();
	if (!numNewLights)
		return;

	uint32* newIDTable = new uint32[numNewLights];

	// Decide on some new light ID values.
	for (int i = 0; i < numNewLights; i++)
		newIDTable[i] = NextLightID++;

	// Correct all references to these IDs by the instances.
	for (tItList<tInstance>::Iter inst = newInstances.First(); inst; ++inst)
	{
		if (inst->ObjectType != tInstance::tType::Light)
			continue;

		uint32 origID = inst->ObjectID;

		// Find the correct lights.
		int newLightIndex = 0;
		bool foundLight = false;
		for (tItList<tLight>::Iter light = newLights.First(); light; ++light)
		{
			if (origID == light->ID)
			{
				foundLight = true;
				break;
			}

			newLightIndex++;
		}

		if (!foundLight)
			throw tError("Could not find light with ID %d. Could be that the list has 2 models with the same ID.", origID);

		inst->ObjectID = newIDTable[newLightIndex];
	}

	// Assign the lights their new ID.
	int index = 0;
	for (tItList<tLight>::Iter l = newLights.First(); l; ++l)
		l->ID = newIDTable[index++];

	delete[] newIDTable;
}


void tWorld::CorrectPathIDs(tItList<tPath>& newPaths, tItList<tInstance>& newInstances )
{
	int numNewPaths = newPaths.GetNumItems();
	if (!numNewPaths)
		return;

	uint32* newIDTable = new uint32[numNewPaths];

	// Decide on some new path ID values.
	for (int i = 0; i < numNewPaths; i++)
		newIDTable[i] = NextPathID++;

	// Correct all references to these IDs by the instances.
	for (tItList<tInstance>::Iter inst = newInstances.First(); inst; ++inst)
	{
		if (inst->ObjectType != tInstance::tType::Path)
			continue;

		uint32 origID = inst->ObjectID;

		// Find the correct path.
		int newPathIndex = 0;
		bool foundPath = false;
		for (tItList<tPath>::Iter path = newPaths.First(); path; ++path)
		{
			if (origID == path->ID)
			{
				foundPath = true;
				break;
			}

			newPathIndex++;
		}

		tAssert(foundPath);
		inst->ObjectID = newIDTable[newPathIndex];
	}

	// Assign the paths their new ID.
	int index = 0;
	for (tItList<tPath>::Iter s = newPaths.First(); s; ++s)
		s->ID = newIDTable[index++];

	delete[] newIDTable;
}


void tWorld::CorrectMaterialIDs(tItList<tMaterial>& newMaterials, tItList<tPolyModel>& newPolyModels)
{
	// We need to make sure that there are no duplicated material IDs before adding the new models and materials to the
	// scene. We do this by assigning material IDs sequentially starting at 0 for any new material. We then traverse
	// all the models and adjust their material face IDs. This cannot be done in one pass -- if the models are added
	// before the adjustment then previously present models will be adjusted incorrectly. This is because the exporter
	// makes no guarantees about what material IDs it uses.
	int numNewMaterials = newMaterials.GetNumItems();
	if (numNewMaterials)
	{
		uint32* newIDTable = new uint32[numNewMaterials];

		// Decide on some new material ID values.
		for (int i = 0; i < numNewMaterials; i++)
			newIDTable[i] = NextMaterialID++;

		// Correct all references to these IDs in the models. Loop through all the faces on all the new models.
		for (tItList<tPolyModel>::Iter model = newPolyModels.First(); model; ++model)
		{
			tMesh& mesh = model->Mesh;

			if (!mesh.FaceTableMaterialIDs)
				continue;

			for (int f = 0; f < mesh.NumFaces; f++)
			{
				uint32 origID = mesh.FaceTableMaterialIDs[f];

				// Find the correct material.
				int newMatIndex = 0;
				bool foundMat = false;
				for (tItList<tMaterial>::Iter mat = newMaterials.First(); mat; ++mat)
				{
					if (origID == mat->ID)
					{
						foundMat = true;
						break;
					}

					newMatIndex++;
				}

				tAssert(foundMat);
				mesh.FaceTableMaterialIDs[f] = newIDTable[newMatIndex];
			}
		}

		// Assign the materials their new ID.
		int index = 0;
		for (tItList<tMaterial>::Iter m = newMaterials.First(); m; ++m)
			m->ID = newIDTable[index++];

		delete[] newIDTable;
	}
}


void tWorld::CorrectSkeletonIDs(tItList<tSkeleton>& newSkeletons, tItList<tPolyModel>& newPolyModels)
{
	int numNewSkeletons = newSkeletons.GetNumItems();
	if (numNewSkeletons)
	{
		uint32* newIDTable = new uint32[numNewSkeletons];

		// Decide on some new skeleton ID values.
		for (int i = 0; i < numNewSkeletons; i++)
			newIDTable[i] = NextSkeletonID++;

		// Correct all references to these IDs in the models.
		for (tItList<tPolyModel>::Iter model = newPolyModels.First(); model; ++model)
		{
			tMesh& mesh = model->Mesh;

			if (!mesh.VertTableWeightSets)
				continue;

			// Loop through all the weight sets on all the new models.
			for (int set = 0; set < mesh.NumVertWeightSets; set++)
			{
				tWeightSet* weightSet = &mesh.VertTableWeightSets[set];

				for (int w = 0; w < weightSet->NumWeights; w++)
				{
					uint32 origID = weightSet->Weights[w].SkeletonID;

					// Find the correct skeleton.
					int newSkelIndex = 0;
					bool foundSkel = false;
					for (tItList<tSkeleton>::Iter skel = newSkeletons.First(); skel; ++skel)
					{
						if (origID == skel->ID)
						{
							foundSkel = true;
							break;
						}

						newSkelIndex++;
					}

					tAssert(foundSkel);
					weightSet->Weights[w].SkeletonID = newIDTable[newSkelIndex];
				}
			}
		}

		// Assign the skeletons their new ID.
		int index = 0;
		for (tItList<tSkeleton>::Iter s = newSkeletons.First(); s; ++s)
		{
			s->ID = newIDTable[index++];
		}

		delete[] newIDTable;
	}
}


void tWorld::CorrectPolyModelIDs(tItList<tPolyModel>& newPolyModels, tItList<tLodGroup>& newLodGroups, tItList<tInstance>& newInstances)
{
	int numNewModels = newPolyModels.GetNumItems();
	if (!numNewModels)
		return;

	uint32* newIDTable = new uint32[numNewModels];

	// Decide on some new poly model ID values.
	for (int i = 0; i < numNewModels; i++)
		newIDTable[i] = NextPolyModelID++;

	// Correct all references to these IDs by the LOD groups.
	for (tItList<tLodGroup>::Iter group = newLodGroups.First(); group; ++group)
	{
		for (tItList<tLodParam>::Iter lod = group->LodParams.First(); lod; ++lod)
		{
			uint32 origID = lod->ModelID;

			// Find the correct poly model.
			int newModelIndex = 0;
			bool foundModel = false;
			for (tItList<tPolyModel>::Iter model = newPolyModels.First(); model; ++model)
			{
				if (origID == model->ID)
				{
					foundModel = true;
					break;
				}

				newModelIndex++;
			}

			tAssert(foundModel);
			lod->ModelID = newIDTable[newModelIndex];
		}
	}

	// Correct all references to these IDs by the instances.
	for (tItList<tInstance>::Iter inst = newInstances.First(); inst; ++inst)
	{
		if (inst->ObjectType != tInstance::tType::PolyModel)
			continue;

		uint32 origID = inst->ObjectID;

		// Find the correct poly model.
		int newModelIndex = 0;
		bool foundModel = false;
		for (tItList<tPolyModel>::Iter model = newPolyModels.First(); model; ++model)
		{
			if (origID == model->ID)
			{
				foundModel = true;
				break;
			}

			newModelIndex++;
		}

		if (!foundModel)
			throw tError("Could not find model with ID %d. Could be that the list has 2 models with the same ID.", origID);

		inst->ObjectID = newIDTable[newModelIndex];
	}

	// Assign the poly models their new ID.
	int index = 0;
	for (tItList<tPolyModel>::Iter m = newPolyModels.First(); m; ++m)
		m->ID = newIDTable[index++];

	delete[] newIDTable;
}


void tWorld::CorrectLodGroupIDs(tItList<tLodGroup>& newLodGroups)
{
	// Just keep IDs unique. Groups are top level so no-one needs to be told about the changes.
	for (tItList<tLodGroup>::Iter group = newLodGroups.First(); group; ++group)
	{
		group->ID = NextLodGroupID;
		NextLodGroupID++;
	}
}


void tWorld::CorrectInstanceIDs(tItList<tInstance>& newInstances, tItList<tSelection>& newSelections)
{
	int numNewInstances = newInstances.GetNumItems();
	if (!numNewInstances)
		return;

	uint32* newIDTable = new uint32[numNewInstances];

	// Decide on some new instance ID values.
	for (int i = 0; i < numNewInstances; i++)
		newIDTable[i] = NextInstanceID++;

	// Correct all references to these IDs by the selections.
	for (tItList<tSelection>::Iter sel = newSelections.First(); sel; ++sel)
	{
		// Loop through all selection's instance IDs.
		for (tItList<uint32>::Iter instIDIter = sel->InstanceIDs.First(); instIDIter; ++instIDIter)
		{
			uint32* instID = instIDIter.GetObject();
			uint32 origID = *instID;

			// Find the correct instances.
			int newInstanceIndex = 0;
			bool foundInstance = false;
			for (tItList<tInstance>::Iter inst = newInstances.First(); inst; ++inst)
			{
				if (origID == inst->ID)
				{
					foundInstance = true;
					break;
				}

				newInstanceIndex++;
			}

			if (!foundInstance)
				throw tError("Could not find instance with ID %d while resolving selections.", origID);

			*instID = newIDTable[newInstanceIndex];
		}
	}

	// Assign the instances their new ID.
	int index = 0;
	for (tItList<tInstance>::Iter i = newInstances.First(); i; ++i)
		i->ID = newIDTable[index++];

	delete[] newIDTable;
}


void tWorld::MergeToExistingSelections(tItList<tSelection>& newSelections)
{
	tItList<tSelection>::Iter newSel = newSelections.First();
	while (newSel)
	{
		tItList<tSelection>::Iter nextSel = newSel;
		++nextSel;

		// Search for an existing selection with the same name as the new one.
		tSelection* existingSel = FindSelection(newSel->Name);
		if (!existingSel)
		{
			newSel = nextSel;
			continue;
		}

		// Transfer all the instance IDs to the existing selection if they aren't there already.
		for (tItList<uint32>::Iter instID = newSel->InstanceIDs.First(); instID; ++instID)
		{
			uint32 newID = *(instID.GetObject());
			if (existingSel->ContainsInstance(newID))
				continue;

			existingSel->InstanceIDs.Append(new uint32(newID));
		}

		// newSel has been merged so we're done with it.
		delete newSelections.Remove(newSel);
		newSel = nextSel;
	}
}


void tWorld::CorrectSelectionIDs(tItList<tSelection>& newSelections)
{
	// Just keep IDs unique. Selections are top level so no-one needs to be told about the changes.
	for (tItList<tSelection>::Iter sel = newSelections.First(); sel; ++sel)
	{
		sel->ID = NextSelectionID;
		NextSelectionID++;
	}
}


void tWorld::Save(const tString& tacFile) const
{
	tChunkWriter chunk(tacFile);

	chunk.Begin(tChunkID::Core_Version);
	{
		chunk.Write(SceneMajorVersion);
		chunk.Write(SceneMinorVersion);
	}
	chunk.End();

	chunk.Begin(tChunkID::Scene_Scene);
	{
		SaveMaterials(chunk);
		SaveObjects(chunk);
		SaveGroups(chunk);
		SaveInstances(chunk);
		SaveSelections(chunk);
	}
	chunk.End();
}


void tWorld::SaveMaterials(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_MaterialList);
	{
		for (tItList<tMaterial>::Iter material = Materials.First(); material; ++material)
			material->Save(chunk);
	}
	chunk.End();
}


void tWorld::SaveObjects(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_ObjectList);
	{
		for (tItList<tSkeleton>::Iter skeleton = Skeletons.First(); skeleton; ++skeleton)
			skeleton->Save(chunk);

		for (tItList<tPolyModel>::Iter polyModel = PolyModels.First(); polyModel; ++polyModel)
			polyModel->Save(chunk);

		for (tItList<tCamera>::Iter camera = Cameras.First(); camera; ++camera)
			camera->Save(chunk);
		
		for (tItList<tLight>::Iter light = Lights.First(); light; ++light)
			light->Save(chunk);

		for (tItList<tPath>::Iter path = Paths.First(); path; ++path)
			path->Save(chunk);
	}
	chunk.End();
}


void tWorld::SaveGroups(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_GroupList);
	{
		for (tItList<tLodGroup>::Iter lodGroup = LodGroups.First(); lodGroup; ++lodGroup)
			lodGroup->Save(chunk);
	}
	chunk.End();
}


void tWorld::SaveInstances(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_InstanceList);
	{
		for (tItList<tInstance>::Iter instance = Instances.First(); instance; ++instance)
			instance->Save(chunk);
	}
	chunk.End();
}


void tWorld::SaveSelections(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_SelectionList);
	{
		for (tItList<tSelection>::Iter sel = Selections.First(); sel; ++sel)
			sel->Save(chunk);
	}
	chunk.End();
}


void tWorld::LoadMaterials(const tChunk& matListChunk, tItList<tMaterial>& materials)
{
	tAssert(matListChunk.ID() == tChunkID::Scene_MaterialList);
	tAssert(materials.IsEmpty());

	for (tChunk chunk = matListChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Material:
				materials.Append(new tMaterial(chunk));
				break;
		}
	}
}


void tWorld::LoadObjects(const tChunk& objListChunk, tItList<tPolyModel>& polyModels, tItList<tSkeleton>& skeletons, tItList<tCamera>& cameras, tItList<tLight>& lights, tItList<tPath>& paths, uint32 loadFlags)
{
	tAssert(objListChunk.ID() == tChunkID::Scene_ObjectList);
	tAssert(polyModels.IsEmpty());
	tAssert(skeletons.IsEmpty());
	tAssert(cameras.IsEmpty());
	tAssert(lights.IsEmpty());
	tAssert(paths.IsEmpty());

	for (tChunk chunk = objListChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_PolyModel:
				if (loadFlags & tLoadFilter_Models)
					polyModels.Append(new tPolyModel(chunk));
				break;

			case tChunkID::Scene_Skeleton:
				if (loadFlags & tLoadFilter_Skeletons)
					skeletons.Append(new tSkeleton(chunk));
				break;

			case tChunkID::Scene_Camera:
				if (loadFlags & tLoadFilter_Cameras)
					cameras.Append(new tCamera(chunk));
				break;

			case tChunkID::Scene_Light:
				if (loadFlags & tLoadFilter_Lights)
					lights.Append(new tLight(chunk));
				break;

			case tChunkID::Scene_Path:
				if (loadFlags & tLoadFilter_Paths)
					paths.Append(new tPath(chunk));
				break;

			case tChunkID::Scene_PatchModel:
				// Not implemented.
				break;
		}
	}
}


void tWorld::LoadGroups(const tChunk& groupListChunk, tItList<tLodGroup>& lodGroups)
{
	tAssert(groupListChunk.GetID() == tChunkID::Scene_GroupList);
	tAssert(lodGroups.IsEmpty());

	for (tChunk chunk = groupListChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.GetID())
		{
			case tChunkID::Scene_LodGroup:
				lodGroups.Append(new tLodGroup(chunk));
				break;
		}
	}
}


void tWorld::LoadInstances(const tChunk& instListChunk, tItList<tInstance>& instances)
{
	tAssert(instListChunk.ID() == tChunkID::Scene_InstanceList);
	tAssert(instances.IsEmpty());

	for (tChunk chunk = instListChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Instance:
				instances.Append(new tInstance(chunk));
				break;
		}
	}
}


void tWorld::LoadSelections(const tChunk& selListChunk, tItList<tSelection>& selections)
{
	tAssert(selListChunk.ID() == tChunkID::Scene_SelectionList);
	tAssert(selections.IsEmpty());

	for (tChunk chunk = selListChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Selection:
				selections.Append(new tSelection(chunk));
				break;
		}
	}
}


int tWorld::GetMaterials(const tMesh& mesh, tItList<tMaterial>& materials)
{
	if (!mesh.FaceTableMaterialIDs)
		return 0;

	int count = 0;
	for (int f = 0; f < mesh.NumFaces; ++f)
	{
		tMaterial* mat = FindMaterial(mesh.FaceTableMaterialIDs[f]);
		if (mat)
		{
			if (!materials.Find(mat))
			{
				materials.Append(mat);
				count++;
			}
		}
	}

	return count;
}


int tWorld::GetNumMaterials(const tString& name) const
{
	if (name.IsEmpty())
		return Materials.GetNumItems();

	int count = 0;
	for (tItList<tMaterial>::Iter m = Materials.First(); m; ++m)
		if (m->Name == name)
			count++;

	return count;
}


void tWorld::InsertMaterial(tMaterial* material)
{
	Materials.Append(material);
}


tMaterial* tWorld::FindMaterial(const tString& name) const
{
	for (tItList<tMaterial>::Iter m = Materials.First(); m; ++m)
		if (m->Name == name)
			return m;

	return nullptr;
}


tMaterial* tWorld::FindMaterial(uint32 id) const
{
	for (tItList<tMaterial>::Iter m = Materials.First(); m; ++m)
		if (m->ID == id)
			return m;

	return nullptr;
}


int tWorld::GetNumSkeletons(const tString& name) const
{
	if (name.IsEmpty())
		return Skeletons.GetNumItems();

	int count = 0;
	for (tItList<tSkeleton>::Iter s = Skeletons.First(); s; ++s)
		if (s->Name == name)
			count++;

	return count;
}


void tWorld::InsertSkeleton(tSkeleton* skeleton)
{
	Skeletons.Append(skeleton);
}


tSkeleton* tWorld::FindSkeleton(const tString& name) const
{
	for (tItList<tSkeleton>::Iter s = Skeletons.First(); s; ++s)
		if (s->Name == name)
			return s;

	return nullptr;
}


tSkeleton* tWorld::FindSkeleton(uint32 id) const
{
	for (tItList<tSkeleton>::Iter s = Skeletons.First(); s; ++s)
		if (s->ID == id)
			return s;

	return nullptr;
}


int tWorld::GetNumModels(const tString& name) const
{
	if (name.IsEmpty())
		return PolyModels.GetNumItems();

	int count = 0;
	for (tItList<tPolyModel>::Iter m = PolyModels.First(); m; ++m)
		if (m->Name == name)
			count++;

	return count;
}


void tWorld::InsertPolyModel(tPolyModel* polyModel)
{
	PolyModels.Append(polyModel);
}


tPolyModel* tWorld::FindPolyModel(const tString& name) const
{
	for (tItList<tPolyModel>::Iter m = PolyModels.First(); m; ++m)
		if (m->Name == name)
			return m;

	return nullptr;
}


tPolyModel* tWorld::FindPolyModel(uint32 id) const
{
	for (tItList<tPolyModel>::Iter m = PolyModels.First(); m; ++m)
		if (m->ID == id)
			return m;

	return nullptr;
}


int tWorld::GetNumCameras(const tString& name) const
{
	if (name.IsEmpty())
		return Cameras.GetNumItems();

	int count = 0;
	for (tItList<tCamera>::Iter c = Cameras.First(); c; ++c)
		if (c->Name == name)
			count++;

	return count;
}


void tWorld::InsertCamera(tCamera* camera)
{
	Cameras.Append(camera);
}


tCamera* tWorld::FindCamera(const tString& name) const
{
	for (tItList<tCamera>::Iter c = Cameras.First(); c; ++c)
		if (c->Name == name)
			return c;

	return nullptr;
}


tCamera* tWorld::FindCamera(uint32 id) const
{
	for (tItList<tCamera>::Iter c = Cameras.First(); c; ++c)
		if (c->ID == id)
			return c;

	return nullptr;
}


int tWorld::GetNumLights(const tString& name) const
{
	if (name.IsEmpty())
		return Lights.GetNumItems();

	int count = 0;
	for (tItList<tLight>::Iter l = Lights.First(); l; ++l)
		if (l->Name == name)
			count++;

	return count;
}


void tWorld::InsertLight(tLight* light)
{
	Lights.Append(light);
}


tLight* tWorld::FindLight(const tString& name) const
{
	for (tItList<tLight>::Iter l = Lights.First(); l; ++l)
		if (l->Name == name)
			return l;

	return nullptr;
}


tLight* tWorld::FindLight(uint32 id) const
{
	for (tItList<tLight>::Iter l = Lights.First(); l; ++l)
		if (l->ID == id)
			return l;

	return nullptr;
}


int tWorld::GetNumPaths(const tString& name) const
{
	if (name.IsEmpty())
		return Paths.GetNumItems();

	int count = 0;
	for (tItList<tPath>::Iter s = Paths.First(); s; ++s)
		if (s->Name == name)
			count++;

	return count;
}


void tWorld::InsertPath(tPath* path)
{
	Paths.Append(path);
}


tPath* tWorld::FindPath(const tString& name) const
{
	for (tItList<tPath>::Iter s = Paths.First(); s; ++s)
		if (s->Name == name)
			return s;

	return nullptr;
}


tPath* tWorld::FindPath(uint32 id) const
{
	for (tItList<tPath>::Iter s = Paths.First(); s; ++s)
		if (s->ID == id)
			return s;

	return nullptr;
}


int tWorld::GetNumLodGroups(const tString& name) const
{
	if (name.IsEmpty())
		return LodGroups.GetNumItems();

	int count = 0;
	for (tItList<tLodGroup>::Iter g = LodGroups.First(); g; ++g)
		if (g->Name == name)
			count++;

	return count;
}


void tWorld::InsertLodGroup(tLodGroup* lodGroup)
{
	LodGroups.Append(lodGroup);
}


tLodGroup* tWorld::FindLodGroup(const tString& name) const
{
	for (tItList<tLodGroup>::Iter g = LodGroups.First(); g; ++g)
		if (g->Name == name)
			return g;

	return nullptr;
}


tLodGroup* tWorld::FindLodGroup(uint32 id) const
{
	for (tItList<tLodGroup>::Iter g = LodGroups.First(); g; ++g)
		if (g->ID == id)
			return g;

	return nullptr;
}


int tWorld::GenerateLodGroupsFromModelNamingConvention()
{
	int numGroupsCreated = 0;

	// Figure out the starting ID for any new groups.
	uint32 nextLodGroupID = 0;
	for (tItList<tLodGroup>::Iter grp = LodGroups.First(); grp; ++grp)
		if (grp->ID > nextLodGroupID)
			nextLodGroupID = grp->ID;

	nextLodGroupID++;

	// Go through all the models and check the names, creating new groups as necessary.
	for (tItList<tPolyModel>::Iter model = PolyModels.First(); model; ++model)
	{
		// If the model is already a member of an LOD group then we just skip it.
		if (model->IsLodGroupMember)
			continue;

		// Look at the model's name and decide if it needs to be added to an LOD group. If it does, it sets the model's
		// bIsLodGroupMember to true and adds it to the correctly named group. The following name manipulation works
		// fine for non-lod names. We check if the end of the name contains "LOD_n", where n is a percent. If it isn't
		// then it's a normal model and we have nothing to do here.
		tString baseName = model->Name;
		tString lodThreshold = baseName.ExtractLastWord('_');
		tString lodTag = baseName.ExtractLastWord('_');

		lodTag.LowCase();
		if (lodTag != "lod")
			continue;

		model->IsLodGroupMember = true;

		// Find the correct LOD group to add this to, or create a new one if it doesn't exist.
		tLodGroup* group = FindLodGroup(baseName);
		if (!group)
		{
			group = new tLodGroup();
			group->ID = nextLodGroupID++;
			group->Name = baseName;
			LodGroups.Append(group);
			numGroupsCreated++;
		}

		tLodParam* newInfo = new tLodParam();
		newInfo->ModelID = model->ID;

		// Deal with the threshold. If the lodThreshold string is empty a value of 0.0f will be used. The value is
		// entered in the content creation tool as a percent so we divide by 100.
		newInfo->Threshold = lodThreshold.GetAsFloat() / 100.0f;
		group->LodParams.Append(newInfo);
	}

	// All LOD groups must be sorted.
	for (tItList<tLodGroup>::Iter group = LodGroups.First(); group; ++group)
		group->Sort();

	return numGroupsCreated;
}


int tWorld::GetNumInstances(const tString& name) const
{
	if (name.IsEmpty())
		return Instances.GetNumItems();

	int count = 0;
	for (tItList<tInstance>::Iter inst = Instances.First(); inst; ++inst)
		if (inst->Name == name)
			count++;

	return count;
}


void tWorld::InsertInstance(tInstance* instance)
{
	Instances.Append(instance);
}


tInstance* tWorld::FindInstance(const tString& name) const
{
	for (tItList<tInstance>::Iter inst = Instances.First(); inst; ++inst)
		if (inst->Name == name)
			return inst;

	return nullptr;
}


tInstance* tWorld::FindInstance(uint32 id) const
{
	for (tItList<tInstance>::Iter inst = Instances.First(); inst; ++inst)
		if (inst->ID == id)
			return inst;

	return nullptr;
}


bool tWorld::IsInstanceInSelection(const tInstance* inst, const tSelection* sel) const
{
	if (!sel || !inst)
		return false;

	return sel->ContainsInstance(inst->ID) ? true : false;
}


int tWorld::GetNumSelections(const tString& name) const
{
	if (name.IsEmpty())
		return Selections.GetNumItems();

	int count = 0;
	for (tItList<tSelection>::Iter inst = Selections.First(); inst; ++inst)
		if (inst->Name == name)
			count++;

	return count;
}


void tWorld::InsertSelection(tSelection* sel)
{
	Selections.Append(sel);
}


tSelection* tWorld::FindSelection(const tString& name) const
{
	for (tItList<tSelection>::Iter sel = Selections.First(); sel; ++sel)
		if (sel->Name == name)
			return sel;

	return nullptr;
}


void tWorld::FindSelectionsContaining(tItList<tSelection*>& sels, const tString& partialName) const
{
	for (tItList<tSelection>::Iter sel = Selections.First(); sel; ++sel)
	{
		if (sel->Name.FindString(partialName) != -1)
		{
			tSelection** newSelPointer = new (tSelection*);
			*newSelPointer = sel.GetObject();
			sels.Append(newSelPointer);
		}
	}
}


tSelection* tWorld::FindSelection(uint32 id) const
{
	for (tItList<tSelection>::Iter sel = Selections.First(); sel; ++sel)
		if (sel->ID == id)
			return sel;

	return nullptr;
}


}