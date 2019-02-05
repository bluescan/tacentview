// tWorld.h
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
// Version History
// 1.0 Initial release.
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
#include <Foundation/tPlatform.h>
#include <Foundation/tList.h>
#include "Scene/tCamera.h"
#include "Scene/tLight.h"
#include "Scene/tPath.h"
#include "Scene/tMaterial.h"
#include "Scene/tSkeleton.h"
#include "Scene/tPolyModel.h"
#include "Scene/tLodGroup.h"
#include "Scene/tInstance.h"
#include "Scene/tSelection.h"
namespace tScene
{


const uint32 SceneMajorVersion = 1;
const uint32 SceneMinorVersion = 0;


class tWorld
{
public:
	// When loading a tac file, what object types do we process. Filters can be combined.
	enum tLoadFilter
	{
		tLoadFilter_None									= 0x00000000,
		tLoadFilter_Cameras									= 0x00000001,
		tLoadFilter_Lights									= 0x00000002,
		tLoadFilter_Paths									= 0x00000004,
		tLoadFilter_Materials								= 0x00000008,
		tLoadFilter_Skeletons								= 0x00000010,
		tLoadFilter_Models									= 0x00000020,
		tLoadFilter_LodGroups								= 0x00000040,
		tLoadFilter_Instances								= 0x00000080,
		tLoadFilter_Selections								= 0x00000100,
		tLoadFilter_All										= 0xFFFFFFFF
	};

	tWorld()																											{ }

	// Loads scene from a tac chunk file. Any externally referenced files get turned into absolute paths. For example,
	// a tMaterial may refer to a dds file relative to it's location on disk. After the tWorld is created, the path to
	// the dds will be absolute. This allows the tWorld Load call to be called multiple times and load several files
	// into a single world.
	tWorld(const tString& tacFile, uint32 filter = tLoadFilter_All)														{ Load(tacFile, filter); }

	// This constructor can load from multiple tac files and merges them all into one world.
	tWorld(const tItList<tString>& tacFiles, uint32 filter = tLoadFilter_All)											{ Load(tacFiles, filter); }
	virtual ~tWorld()																									{ Clear(); }

	void Clear();											// Clears the scene. Frees any lists or internal objects.
	void Scale(float);										// Scales the scene and all its objects.

	// @todo Allow save to take filters.
	void Save(const tString& tacFile) const;

	// Load may be called more than once to load more objects into the same world.
	void Load(const tString& tacFile, uint32 filter = tLoadFilter_All);
	void Load(const tItList<tString>& tacFiles, uint32 filter = tLoadFilter_All)										{ for (tItList<tString>::Iter file = tacFiles.First(); file.IsValid(); ++file) Load(*file, filter); }

	void AddOffsetToAllIDs(uint32 offset);

	// This merges the srcWorld into the current world. In doing so, the srcWorld is left empty and invalid.
	void MergeScene(tWorld& srcWorld);

	// This function merges various tObject types into the current world. Since some tObject types can refer to other
	// tObject types, it is important that you supply the objects you want to merge _and_ any objects that may be
	// referenced. You need to go 'all the way down the tree' until you are referencing nothing. Here's a chart that
	// shows what tObjects may reference others:
	//
	// cameras			reference		nothing
	// lights			reference		nothing
	// paths			reference		nothing
	// materials		reference		nothing (external files only)
	// skeletons		reference		nothing
	// polyModels		reference		materials, skeletons
	// lodGroups		reference		polyModels
	// instances		reference		lodGroups, polyModels, cameras, lights, paths
	// selectionSets	reference		instances
	//
	// The passed-in object lists are emptied and merged into the scene. Returns true if all the necessary objects were
	// supplied and the merge was successful.
	bool MergeItems
	(
		tItList<tCamera>*, tItList<tLight>*, tItList<tPath>*,
		tItList<tMaterial>*, tItList<tSkeleton>*, tItList<tPolyModel>*,
		tItList<tLodGroup>*, tItList<tInstance>*, tItList<tSelection>*
	);

	// CombinePolyModelInstances combines a list of tPolyModel instances, their meshes and their materials into a
	// single new instance called instName. The polymodel instances you want to combine must already be in the scene.
	// The polymodel instances you provide may no longer in the scene after this call if there are no other instances
	// that refer to it. The new instance will have it's own new polymodel. Returns success.
	bool CombinePolyModelInstances(tItList<tInstance>& polymodelInstances, tString newInstName);

	// Returns the number of cameras in the world. If name is supplied, returns the number with that particular name.
	int GetNumCameras(const tString& name = tString()) const;

	// Inserts the supplied camera into the scene.
	void InsertCamera(tCamera*);

	// Returns first camera with supplied name. Returns nullptr if not found.
	tCamera* FindCamera(const tString& name) const;

	// Returns the camera having the id supplied. Returns nullptr if not found.
	tCamera* FindCamera(uint32 id) const;

	// The rest of these functions are similar to the 4 above except for different tObject types.
	int GetNumLights(const tString& name = tString()) const;
	void InsertLight(tLight*);
	tLight* FindLight(const tString& name) const;
	tLight* FindLight(uint32 id) const;

	int GetNumPaths(const tString& name = tString()) const;
	void InsertPath(tPath*);
	tPath* FindPath(const tString& name) const;
	tPath* FindPath(uint32 id) const;

	int GetMaterials(const tMesh& mesh, tItList<tMaterial>&);	// Returns the materials used by a particular mesh.
	int GetNumMaterials(const tString& name = tString()) const;
	void InsertMaterial(tMaterial*);
	tMaterial* FindMaterial(const tString& name) const;
	tMaterial* FindMaterial(uint32 id) const;

	int GetNumSkeletons(const tString& name = tString()) const;
	void InsertSkeleton(tSkeleton*);
	tSkeleton* FindSkeleton(const tString& name) const;
	tSkeleton* FindSkeleton(uint32 id) const;

	int GetNumModels(const tString& name = tString()) const;
	void InsertPolyModel(tPolyModel*);
	tPolyModel* FindPolyModel(const tString& name) const;
	tPolyModel* FindPolyModel(uint32 id) const;

	int GetNumLodGroups(const tString& name = tString()) const;
	void InsertLodGroup(tLodGroup*);
	tLodGroup* FindLodGroup(const tString& name) const;
	tLodGroup* FindLodGroup(uint32 id) const;

	int GetNumInstances(const tString& name = tString()) const;
	void InsertInstance(tInstance*);
	tInstance* FindInstance(const tString& name) const;
	tInstance* FindInstance(uint32 id) const;
	bool IsInstanceInSelection(const tInstance*, const tSelection*) const;

	int GetNumSelections(const tString& name = tString()) const;
	void InsertSelection(tSelection*);
	tSelection* FindSelection(const tString& name) const;
	tSelection* FindSelection(uint32 id) const;

	// Returns a list of pointers to all selection sets that contain partialName somewhere in their name. You must
	// empty the list yourself afterwards (you own it), but you don't own the things the pointers on the list point to
	// (the selection sets).
	void FindSelectionsContaining(tItList<tSelection*>&, const tString& partialName) const;

	// This call generates additional tLodGroups based on a model naming convention. All models must be present in the
	// scene before calling this. The model naming convention is: "ModelName_LOD_n" where n is the percent of screen
	// space above which the model will be used. The "ModelName" becomes the name of the group, and all models that
	// follow the pattern get their IsLodGroupMember member set. Returns the number of additional tLodGroups that were
	// added to the scene.
	//
	// This functionality should arguably be in the exporter code itself, but since it's a lot of work to make Lod plugins
	// for some modeling packages we've decided a naming convention is reasonable. Multiple modeling SW may need this
	// functionality so it makes sense to put it in a shared location like the tWorld (here). It is ok if there are
	// already LodGroups in the scene. Any new ones simply get assigned new IDs starting at a value one higher than the
	// highest of any current LodGroups.
	int GenerateLodGroupsFromModelNamingConvention();

private:
	// These are helper functions to save and load different types of tObjects.
	void SaveMaterials(tChunkWriter&) const;
	void SaveObjects(tChunkWriter&) const;
	void SaveGroups(tChunkWriter&) const;
	void SaveInstances(tChunkWriter&) const;
	void SaveSelections(tChunkWriter&) const;

	void LoadMaterials(const tChunk&, tItList<tMaterial>&);
	void LoadObjects
	(
		const tChunk&,
		tItList<tPolyModel>&, tItList<tSkeleton>&, tItList<tCamera>&, tItList<tLight>&, tItList<tPath>&,
		uint32 loadFilter
	);
	void LoadGroups(const tChunk&, tItList<tLodGroup>&);
	void LoadInstances(const tChunk&, tItList<tInstance>&);
	void LoadSelections(const tChunk&, tItList<tSelection>&);

	void CorrectCameraIDs(tItList<tCamera>&, tItList<tInstance>&);
	void CorrectLightIDs(tItList<tLight>&, tItList<tInstance>&);
	void CorrectPathIDs(tItList<tPath>&, tItList<tInstance>&);
	void CorrectMaterialIDs(tItList<tMaterial>&, tItList<tPolyModel>&);
	void CorrectSkeletonIDs(tItList<tSkeleton>&, tItList<tPolyModel>&);
	void CorrectPolyModelIDs(tItList<tPolyModel>&, tItList<tLodGroup>&, tItList<tInstance>&);
	void CorrectLodGroupIDs(tItList<tLodGroup>&);
	void CorrectInstanceIDs(tItList<tInstance>&, tItList<tSelection>&);
	void CorrectSelectionIDs(tItList<tSelection>&);
	void MergeToExistingSelections(tItList<tSelection>&);

public:
	tString Name;
	tString LastLoadedFilename;

	// These don't reference anything else in the world, only external file references.
	tItList<tCamera> Cameras;
	tItList<tLight> Lights;
	tItList<tPath> Paths;
	tItList<tMaterial> Materials;
	tItList<tSkeleton> Skeletons;

	// These may reference materials and skeletons.
	tItList<tPolyModel> PolyModels;

	// These may reference poly-models.
	tItList<tLodGroup> LodGroups;

	// Instances may reference anything above.
	tItList<tInstance> Instances;

	// Selection sets may reference instances.
	tItList<tSelection> Selections;

	// These are always the version numbers of the last file loaded.
	uint32 MajorVersion = SceneMajorVersion;
	uint32 MinorVersion = SceneMinorVersion;

	// Used to assign new IDs sequentially.
	uint32 NextCameraID = 0;
	uint32 NextLightID = 0;
	uint32 NextPathID = 0;
	uint32 NextMaterialID = 0;
	uint32 NextSkeletonID = 0;
	uint32 NextPolyModelID = 0;
	uint32 NextLodGroupID = 0;
	uint32 NextInstanceID = 0;
	uint32 NextSelectionID = 0;
};


}
