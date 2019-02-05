// tPath.h
//
// This file implements scene splines and paths.
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
#include <Math/tSpline.h>
#include "Scene/tObject.h"
namespace tScene
{


class tPath : public tObject
{
public:
	tPath()																												{ }
	tPath(const tChunk& chunk)																							{ Load(chunk); }
	tPath(const tMath::tBezierPath&, const tString& name, uint32 id);
	virtual ~tPath()																									{ Clear(); }

	enum class tType
	{
		Invalid																											= -1,
		PiecewiseLinear,
		CubicBezier,
		NonuniformNonrationalCubicBasis
	};

	bool IsValid() const																								{ return Type != tType::Invalid; }
	void Scale(float scale)																								{ for (int cv = 0; cv < NumControlVerts; cv++) ControlVerts[cv] *= scale; }
	void Clear()																										{ tObject::Clear(); Type = tType::Invalid; IsClosed = false; NumControlVerts = 0; delete[] ControlVerts; ControlVerts = nullptr; }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	tType Type = tType::Invalid;
	bool IsClosed = false;
	int NumControlVerts = 0;
	tMath::tVector3* ControlVerts = nullptr;
};


}
