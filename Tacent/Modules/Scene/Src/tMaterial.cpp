// tMaterial.cpp
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
#include "Scene/tMaterial.h"
namespace tScene
{


void tMaterial::Clear()
{
	tObject::Clear();
	Shininess = 0.0f;
	Reflectivity = 0.0f;
	Diffusion = 0.0f;
	TextureDiffuse.Clear();
	TextureNormalMap.Clear();
	TextureA.Clear();
	TextureB.Clear();
	TextureC.Clear();
	TextureD.Clear();
	TextureE.Clear();
	ShaderFile.Clear();
	SpecularColour.MakeBlack();
	ReflectedColour.MakeBlack();
	DiffuseColour.MakeBlack();
	Transparency.MakeBlack();
	AmbientColour.MakeBlack();
	Incandescence.MakeBlack();
	UseDiffuseColour = false;
}


bool tMaterial::IsFunctionallySame(const tMaterial& mat) const
{
	// Attributes always must be the same for a true return value.
	if (!tObject::IsFunctionallySame(mat)) return false;
	if (!ShaderFile.IsEmpty() && (ShaderFile == mat.ShaderFile))
		return true;

	if (AmbientColour != mat.AmbientColour) return false;
	if (DiffuseColour != mat.DiffuseColour) return false;
	if (SpecularColour != mat.SpecularColour) return false;

	if (Shininess != mat.Shininess) return false;
	if (Reflectivity != mat.Reflectivity) return false;
	if (Diffusion != mat.Diffusion) return false;

	if (ReflectedColour != mat.ReflectedColour) return false;
	if (Transparency != mat.Transparency) return false;
	if (Incandescence != mat.Incandescence) return false;
	if (UseDiffuseColour != mat.UseDiffuseColour) return false;

	if (TextureDiffuse != mat.TextureDiffuse) return false;
	if (TextureNormalMap != mat.TextureNormalMap) return false;

	if (TextureA != mat.TextureA) return false;
	if (TextureB != mat.TextureB) return false;
	if (TextureC != mat.TextureC) return false;
	if (TextureD != mat.TextureD) return false;
	if (TextureE != mat.TextureE) return false;

	return true;
}


void tMaterial::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_Material);
	{
		tObject::Save(chunk);

		chunk.Begin(tChunkID::Scene_MaterialProperties);
		{
			chunk.Write(AmbientColour);
			chunk.Write(DiffuseColour);
			chunk.Write(SpecularColour);
			chunk.Write(Shininess);
			chunk.Write(Reflectivity);
			chunk.Write(Diffusion);
			chunk.Write(ReflectedColour);
			chunk.Write(Transparency);
			chunk.Write(Incandescence);
			chunk.Write(UseDiffuseColour);
		}
		chunk.End();

		if (!TextureDiffuse.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureDiffuse);
			chunk.Write(TextureDiffuse);
			chunk.End();
		}

		if (!TextureNormalMap.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureNormalMap);
			chunk.Write(TextureNormalMap);
			chunk.End();
		}

		if (!TextureA.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureA);
			chunk.Write(TextureA);
			chunk.End();
		}

		if (!TextureB.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureB);
			chunk.Write(TextureB);
			chunk.End();
		}

		if (!TextureC.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureC);
			chunk.Write(TextureC);
			chunk.End();
		}

		if (!TextureD.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureD);
			chunk.Write(TextureD);
			chunk.End();
		}

		if (!TextureE.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialTextureE);
			chunk.Write(TextureE);
			chunk.End();
		}

		if (!ShaderFile.IsEmpty())
		{
			chunk.Begin(tChunkID::Scene_MaterialShaderFile);
			chunk.Write(ShaderFile);
			chunk.End();
		}
	}
	chunk.End();
}


void tMaterial::Load(const tChunk& matChunk)
{
	Clear();
	tAssert(matChunk.ID() == tChunkID::Scene_Material);

	for (tChunk chunk = matChunk.First(); chunk.Valid(); chunk = chunk.Next())
	{
		switch (chunk.ID())
		{
			case tChunkID::Scene_Object:
				tObject::Load(chunk);
				break;

			case tChunkID::Scene_MaterialProperties:
				chunk.GetItem(AmbientColour);
				chunk.GetItem(DiffuseColour);
				chunk.GetItem(SpecularColour);
				chunk.GetItem(Shininess);
				chunk.GetItem(Reflectivity);
				chunk.GetItem(Diffusion);
				chunk.GetItem(ReflectedColour);
				chunk.GetItem(Transparency);
				chunk.GetItem(Incandescence);
				chunk.GetItem(UseDiffuseColour);
				break;

			case tChunkID::Scene_MaterialTextureDiffuse:
				chunk.GetItem(TextureDiffuse);
				break;

			case tChunkID::Scene_MaterialTextureNormalMap:
				chunk.GetItem(TextureNormalMap);
				break;

			case tChunkID::Scene_MaterialTextureA:
				chunk.GetItem(TextureA);
				break;

			case tChunkID::Scene_MaterialTextureB:
				chunk.GetItem(TextureB);
				break;

			case tChunkID::Scene_MaterialTextureC:
				chunk.GetItem(TextureC);
				break;

			case tChunkID::Scene_MaterialTextureD:
				chunk.GetItem(TextureD);
				break;

			case tChunkID::Scene_MaterialTextureE:
				chunk.GetItem(TextureE);
				break;

			case tChunkID::Scene_MaterialShaderFile:
				chunk.GetItem(ShaderFile);
				break;
		}
	}
}


tMaterial& tMaterial::operator=(const tMaterial& src)
{
	if (this == &src)
		return *this;

	tObject::operator=(src);

	AmbientColour = src.AmbientColour;
	DiffuseColour = src.DiffuseColour;
	SpecularColour = src.SpecularColour;

	Shininess = src.Shininess;
	Reflectivity = src.Reflectivity;
	Diffusion = src.Diffusion;

	ReflectedColour = src.ReflectedColour;
	Transparency = src.Transparency;
	Incandescence = src.Incandescence;
	UseDiffuseColour = src.UseDiffuseColour;

	TextureDiffuse = src.TextureDiffuse;
	TextureNormalMap = src.TextureNormalMap;

	TextureA = src.TextureA;
	TextureB = src.TextureB;
	TextureC = src.TextureC;
	TextureD = src.TextureD;
	TextureE = src.TextureE;

	ShaderFile = src.ShaderFile;
	return *this;
}


}