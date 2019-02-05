// tMaterial.h
//
// This file implements a tScene material. The parameters represent a legacy Blinn-Phong material which more or less
// matches the fixed function pipeline of OpenGL. More advanced materials/shader-models are handled by explicitly
// setting the ShaderFile member to point to a shader, in which case the Blinn-Phong parameters are ignored.
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


class tMaterial : public tObject
{
public:
	// The default constructor makes a reasonable default texture with non-zero ambient colours etc. This allows it to
	// be used to create a default material that can be used to display a default diffuse texture. All texture names
	// are left empty.
	tMaterial()																											{ Clear(); }
	tMaterial(const tMaterial& src)																						{ *this = src; }

	tMaterial(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tMaterial()																								{ }

	void Clear();
	void Load(const tChunk&);
	void Save(tChunkWriter&) const;
	float GetOpacity() const																							{ return 1.0f - float(Transparency.R + Transparency.G + Transparency.B) / 765.0f; } // 765 = 255 * 3.

	// Checks if 2 materials are visually identical. If a shader file (shd) is specified then it is checked and the
	// other quantities are not since shd files override all other material parameters. The ID/Name is not checked as
	// it has no effect visually. The attributes must match (but the order doesn't matter).
	bool IsFunctionallySame(const tMaterial&) const;

	// These check everything including Name and ID.
	bool operator==(const tMaterial& mat) const																			{ return IsFunctionallySame(mat) && (tObject::operator== (mat)); }
	bool operator!=(const tMaterial& mat) const																			{ return !(*this == mat); }
	tMaterial& operator=(const tMaterial& src);

	tColouri AmbientColour;
	tColouri DiffuseColour;
	tColouri SpecularColour;

	float Shininess;																									// In [0.0, 1.0]
	float Reflectivity;
	float Diffusion;																									// In [0.0, 1.0]

	tColouri ReflectedColour;
	tColouri Transparency;
	tColouri Incandescence;
	bool UseDiffuseColour;																								// Whether to use the diffuse colour, or the diffusion variable.  If the exporter supports it, this is better if it's true.

	tString TextureDiffuse;
	tString TextureNormalMap;

	// Additional diffuse textures for multi-texturing or swapping.
	tString TextureA;
	tString TextureB;
	tString TextureC;
	tString TextureD;
	tString TextureE;

	tString ShaderFile;
};


}
