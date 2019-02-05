// tSkeleton.h
//
// This file implements scene skeletons as a hierarchy of joints and poses that, well, pose a skeleton.
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
#include "Scene/tObject.h"
namespace tScene
{


class tJoint : public tObject
{
public:
	tJoint()																											{ }
	tJoint(const tChunk& chunk)																							{ Load(chunk); }
	virtual ~tJoint()																									{ Clear(); }

	void Clear();
	void AddChildID(uint32 id);
	void Scale(float scale)																								{ Translation *= scale; }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	static const int MaxChildJoints																						= 16;
	tMath::tVector4 Translation;
	tMath::tQuaternion Orientation;
	tMath::tVector4 JointScale;

	uint32 ParentID = tObject::InvalidID;					// InvalidID means no parent.
	int NumChildren = 0;
	uint32* Children = nullptr;
};


class tPose
{
public:
	tPose()																												{ }
	tPose(const tChunk& chunk)																							{ Load(chunk); }
	virtual ~tPose()																									{ delete[] Quaternions; delete[] Scales; }

	void Load(const tChunk&);
	void Save(tChunkWriter&) const;

	int FrameNumber = -1;
	int NumJoints = 0;

	// Order of quaternions and the scales matches the order of the joints using a normal depth first traversal.
	tMath::tQuaternion* Quaternions = nullptr;
	tMath::tVector4* Scales = nullptr;
};


class tSkeleton : public tObject
{
public:
	tSkeleton()																											{ }
	tSkeleton(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tSkeleton()																								{ }

	void Clear();
	void Pose(int poseNum);
	void Scale(float scale)																								{ for (tItList<tJoint>::Iter joint = Joints.First(); joint; ++joint) joint->Scale(scale); }

	tJoint* GetJoint(uint32 jointID);
	tJoint* GetRootJoint()																								{ return GetJoint(0); }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

public:
	int NumJoints = 0;
	int NumPoses = 0;
	float FrameFrequency = 30.0f;
	tItList<tJoint> Joints;
	tItList<tPose> Poses;
};


// Implementation below this line.


inline void tJoint::Clear()
{
	tObject::Clear();
	Translation.Zero();
	Orientation = tMath::tQuaternion::unit;
	JointScale.Set(1.0f, 1.0f, 1.0f, 1.0f);
	ParentID = tObject::InvalidID;
	NumChildren = 0;
	delete Children;
	Children = nullptr;
}


inline void tSkeleton::Clear()
{
	tObject::Clear();
	NumJoints = 0;
	NumPoses = 0;
	FrameFrequency = 30.0f;
	Joints.Empty();
	Poses.Empty();
}


inline tJoint* tSkeleton::GetJoint(uint32 jointID)
{
	for (tItList<tJoint>::Iter joint = Joints.First(); joint; joint.Next())
		if (joint->ID == jointID)
			return joint;

	return 0;
}


}
