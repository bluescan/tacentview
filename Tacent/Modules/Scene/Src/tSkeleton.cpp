// tSkeleton.cpp
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

#include <Math/tVector3.h>
#include <Math/tQuaternion.h>
#include "Scene/tSkeleton.h"
using namespace tMath;
namespace tScene
{


void tJoint::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Joint);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_JointTranslation);
		chunk.Write(Translation);
		chunk.End();

		chunk.Begin(tChunkID::Scene_JointOrientation);
		chunk.Write(Orientation);
		chunk.End();

		chunk.Begin(tChunkID::Scene_JointScale);
		chunk.Write(JointScale);
		chunk.End();

		chunk.Begin(tChunkID::Scene_JointParentID);
		chunk.Write(ParentID);
		chunk.End();

		chunk.Begin(tChunkID::Scene_JointNumChildren);
		chunk.Write(NumChildren);
		chunk.End();

		chunk.Begin(tChunkID::Scene_JointChildIDTable);
		chunk.Write(Children, NumChildren);
		chunk.End();
	}
	chunk.End();
}


void tJoint::Load(const tChunk& jointChunk)
{
	tAssert(jointChunk.GetID() == tChunkID::Scene_Joint);
	Clear();

	for (tChunk chunk = jointChunk.First(); chunk.IsValid(); chunk = chunk.Next())
	{
		switch (chunk.GetID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_JointTranslation:
				chunk.GetItem(Translation);
				break;

			case tChunkID::Scene_JointOrientation:
				chunk.GetItem(Orientation);
				break;

			case tChunkID::Scene_JointScale:
				chunk.GetItem(JointScale);
				break;

			case tChunkID::Scene_JointParentID:
				chunk.GetItem(ParentID);
				break;

			case tChunkID::Scene_JointNumChildren:
				chunk.GetItem(NumChildren);
				break;

			case tChunkID::Scene_JointChildIDTable:
			{
				tAssert(NumChildren > 0);
				Children = new uint32[NumChildren];
				chunk.GetItems(Children, NumChildren);
				break;
			}
		}
	}
}


void tJoint::AddChildID(uint32 id)
{
	tAssert(NumChildren < MaxChildJoints);
	if (!Children)
	{
		Children = new uint32[MaxChildJoints];
		for (int i = 0; i < MaxChildJoints; i++)
			Children[i] = 0;
	}

	Children[NumChildren++] = id;
}


void tPose::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Pose);
	{
		chunk.Begin(tChunkID::Scene_PoseInfo);
		{
			chunk.Write(FrameNumber);
			chunk.Write(NumJoints);
		}
		chunk.End();

		chunk.Begin(tChunkID::Scene_QuaternionTable);
		{
			chunk.Write(Quaternions, NumJoints);
		}
		chunk.End();

		chunk.Begin(tChunkID::Scene_ScaleTable);
		{
			chunk.Write(Scales, NumJoints);
		}
		chunk.End();
	}
	chunk.End();
}


void tPose::Load(const tChunk& poseChunk)
{
	tAssert(poseChunk.GetID() == tChunkID::Scene_Pose);

	for (tChunk chunk = poseChunk.First(); chunk.IsValid(); chunk = chunk.Next())
	{
		switch (chunk.GetID())
		{
			case tChunkID::Scene_PoseInfo:
				chunk.GetItem(FrameNumber);
				chunk.GetItem(NumJoints);
				break;

			case tChunkID::Scene_QuaternionTable:
			{
				tAssert((FrameNumber != -1) && (NumJoints > 0));
				Quaternions = new tQuaternion[NumJoints];
				chunk.GetItems(Quaternions, NumJoints);
				break;
			}

			case tChunkID::Scene_ScaleTable:
			{
				tAssert((FrameNumber != -1) && (NumJoints > 0));
				Scales = new tVector4[NumJoints];
				chunk.GetItems(Scales, NumJoints);
				break;
			}
		}
	}	
}


void tSkeleton::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Skeleton);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_SkeletonInfo);
		{
			chunk.Write(NumJoints);
			chunk.Write(NumPoses);
			chunk.Write(FrameFrequency);
		}
		chunk.End();

		chunk.Begin(tChunkID::Scene_JointList);
		{
			for (tItList<tJoint>::Iter joint = Joints.First(); joint; joint.Next())
				joint->Save(chunk);
		}
		chunk.End();

		chunk.Begin(tChunkID::Scene_PoseList);
		{
			for (tItList<tPose>::Iter pose = Poses.First(); pose; ++pose)
				pose->Save(chunk);
		}
		chunk.End();
	}
}


void tSkeleton::Load(const tChunk& skelChunk)
{
	tAssert(skelChunk.GetID() == tChunkID::Scene_Skeleton);
	Clear();

	for (tChunk chunk = skelChunk.First(); chunk.IsValid(); chunk = chunk.Next())
	{
		switch (chunk.GetID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_SkeletonInfo:
				chunk.GetItem(NumJoints);
				chunk.GetItem(NumPoses);
				chunk.GetItem(FrameFrequency);
				break;

			case tChunkID::Scene_JointList:
			{
				tAssert((chunk.GetID() == tChunkID::Scene_JointList) && (Joints.IsEmpty()));
				for (tChunk c = chunk.First(); c.IsValid(); c = c.Next())
				{
					switch (c.GetID())
					{
						case tChunkID::Scene_Joint:
							Joints.Append(new tJoint(c));
							break;
					}
				}
				break;
			}

			case tChunkID::Scene_PoseList:
			{
				tAssert((chunk.GetID() == tChunkID::Scene_PoseList) && (Poses.IsEmpty()));
				for (tChunk c = chunk.First(); c.IsValid(); c = c.Next())
				{
					switch (c.GetID())
					{
						case tChunkID::Scene_Pose:
							Poses.Append(new tPose(c));
							break;
					}
				}
				break;
			}
		}
	}
}


void tSkeleton::Pose(int poseNum)
{
	if (poseNum >= NumPoses)
		poseNum = NumPoses - 1;

	tItList<tPose>::Iter pose = Poses.First();
	while (poseNum)
	{
		++pose;
		poseNum--;
	}

	int jointNum = 0;
	for (tItList<tJoint>::Iter joint = Joints.First(); joint; ++joint)
	{
		joint->Orientation = pose->Quaternions[jointNum];
		joint->JointScale = pose->Scales[jointNum];
		jointNum++;
	}
}


}
