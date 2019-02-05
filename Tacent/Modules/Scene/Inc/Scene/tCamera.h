// tCamera.h
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

#pragma once
#include <Foundation/tAssert.h>
#include "Scene/tObject.h"
namespace tScene
{


class tCamera : public tObject
{
public:
	tCamera()																											{ }
	tCamera(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tCamera()																									{ }

	void Save(tChunkWriter&) const override;
	void Load(const tChunk&) override;
	void Clear()																										{ ProjectionMode = tProjectionMode::Invalid; Projection = tMath::tMatrix4::identity; }

	// If the entire world is scales, this function properly scales any tCameras. Essentially this amounts to adjusting
	// the near and far planes if the projection is perspective.
	void Scale(float scale);
	
	enum class tProjectionMode
	{
		Invalid												= -1,
		Perspective,										// A symmetric perspective projection. The most common.
		PerspectiveOblique,									// An asymmetric perspective projection.
		Parallel											// A parallel projection.
	};

	// Valid accessors for all types of projection.
	void ExtractProjectionPlanes(tMath::tVector4 planes[6], bool outwardNormals = false, bool normalizePlanes = true)	{ Projection.ExtractProjectionPlanes(planes, outwardNormals, normalizePlanes); }

	// Valid accessors for Perspective and PerspectiveOblique projections. You must ensure the correct projection mode.
	float GetNearPlane() const																							{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractProjectionNear(); }
	float GetFarPlane() const																							{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractProjectionFar(); }
	float GetAspect() const																								{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractProjectionAspect(); }
	float GetFovV() const																								{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractPerspectiveFovV(); }
	float GetFovH() const																								{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractPerspectiveFovH(); }
	float ExtractProjectionOffsetX() const					/* Returns 0.0 for symmetric. */							{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractProjectionOffsetX(); }
	float ExtractProjectionOffsetY() const					/* Returns 0.0 for symmetric. */							{ tAssert(ProjectionMode != tProjectionMode::Parallel); return Projection.ExtractProjectionOffsetY(); }

	tProjectionMode ProjectionMode = tProjectionMode::Invalid;
	tMath::tMatrix4 Projection = tMath::tMatrix4::identity;
};


}
