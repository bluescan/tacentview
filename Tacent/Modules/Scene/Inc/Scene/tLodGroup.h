// tLodGroup.h
//
// This unit implements scene LOD groups. LOD groups basically specify what model to use based on a size threshold.
// The threshold is based on screen-size, not distance. This is much more correct as it allows narrow camera FOVs
// without things looking pixelated.
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
#include <Foundation/tList.h>
#include "Scene/tObject.h"
namespace tScene
{


class tLodParam
{
public:
	tLodParam()																											{ }
	tLodParam(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tLodParam()																								{ }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	uint32 ModelID = 0;

	// The threshold is the proportion of screen width at which the LOD swap should occur. Doing it this way makes it
	// respect the camera FOV and does not tie it to screen resolution. The runtime can decide to consider the current
	// screen resolution if it likes. A value of 1.0 means that the object takes up the entire screen, 0.5 means half
	// the screen. The values are NOT restricted to [0.0, 1.0]. It is valid to have values > 1.0. If a runtime object
	// takes up less room than the threshold then the ModelID should not be used. The biggest lod info threshold value
	// that is less than the current percent is the right one.
	float Threshold = 0.0f;
};


class tLodGroup : public tObject
{
public:
	tLodGroup()																											: tObject(), LodParams() { }
	tLodGroup(const tChunk& chunk)																						: tObject(), LodParams() { Load(chunk); }
	virtual ~tLodGroup()																								{ }

	void Load(const tChunk&);
	void Save(tChunkWriter&) const;
	void Clear()																										{ tObject::Clear(); LodParams.Empty(); }
	int GetNumLodInfos() const																							{ return LodParams.GetNumItems(); }

	// This call will sort all the lod params from highest threshold to lowest.
	void Sort()																											{ LodParams.Sort(ThresholdCompare); }
	tItList<tLodParam> LodParams;

private:
	// Implements a > b so the list is sorted in descending order.
	inline static bool ThresholdCompare(const tLodParam& a, const tLodParam& b)											{ return (a.Threshold > b.Threshold) ? true : false; }
};


}
