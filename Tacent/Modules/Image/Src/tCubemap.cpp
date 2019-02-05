// tCubemap.cpp
//
// A cubemap is simply a collection of 6 tTextures. By convention each tTexture has layers of the same pixel format. A
// tCubemap may be created from a dds file (assuming the dds file contains a cubemap) or from multiple tTextures. When
// construction from a dds is performed, it chooses a different image to load into the tTexture for each cubemap side.
// When constructing from tTextures directly it is the user's responsibility to supply the necessary tTextures in the
// correct format before constructing a tCubemap. tCubemaps can also save and load themselves into a tChunk-based
// format. They do this by looping through their tTextures and calling their individual Save/Load functions.
//
// ATI has a tool called CubeMapGen 1.1 which is useful for making your own cube map dds files. Its installer also
// installs some example dds cube maps to play with. The CubeMapGen tool (as of v1.1) does NOT generate valid DXT1
// compressed cubemap files. The header is incorrect and the data is not compressed.
//
// nVidia's DDS tools can also generate dds cubemaps straight from photoshop. Just pull down the combo-box that usually
// says "2D TEXTURE" and there'll be an option for cubemaps. The nVidia dds plugin for photoshop does not have this
// issues with texture compression and cubemaps.
//
// The order of the images for a cubemap are +x -x +y -y +z -z. In the CubeMapGen tool the option for OpenGL and DX
// coord systems only affects the export of cube map faces -- it does not, apparently, affect the output dds cubemap.
// i.e. It seems the faces should always be specified using a LH coord system. The OpenGL calls for specifying a
// cubemap are also left handed which is inconsistent with other parts of the OpenGL API.
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

#include "Image/tCubemap.h"
using namespace tImage;


bool tCubemap::IsValid() const
{
	for (int side = 0; side < int(tSide::NumSides); side++)
		if ((Sides[side].Side != tSide::Invalid) && Sides[side].Texture.IsValid())
			return true;

	return false;
}


bool tCubemap::AllSidesOpaque() const
{
	for (int side = 0; side < int(tSide::NumSides); side++)
		if ((Sides[side].Side != tSide::Invalid) && Sides[side].Texture.IsValid() && !Sides[side].Texture.IsOpaque())
			return false;

	return true;
}


void tCubemap::Clear()
{
	for (int side = 0; side < int(tSide::NumSides); side++)
	{
		Sides[side].Side = tSide::Invalid;
		Sides[side].Texture.Clear();
	}
}


bool tCubemap::Load(const tString& ddsFile)
{
	Clear();
	if ((tSystem::tGetFileType(ddsFile) != tSystem::tFileType::DDS) || !tSystem::tFileExists(ddsFile))
		return false;

	bool correctRowOrder = false;
	tFileDDS dds(ddsFile, correctRowOrder);
	if (dds.IsValid() || !dds.IsCubemap())
		return false;

	return Set(dds);
}


bool tCubemap::Set(tFileDDS& dds)
{
	Clear();
	if (dds.IsValid() || !dds.IsCubemap())
		return false;

	tList<tLayer> layerSet[tFileDDS::tSurfIndex_NumSurfaces];
	tStaticAssert(int(tSide::NumSides) == tFileDDS::tSurfIndex_NumSurfaces);
	dds.StealCubemapLayers(layerSet);

	for (int side = 0; side < int(tSide::NumSides); side++)
	{
		Sides[side].Side = tSide(side);
		Sides[side].Texture.Set(layerSet[side]);
	}

	if (!IsValid())
	{
		Clear();
		return false;
	}

	return true;
}


bool tCubemap::Load
(
	const tString& imageFilePosX, const tString& imageFileNegX,
	const tString& imageFilePosY, const tString& imageFileNegY,
	const tString& imageFilePosZ, const tString& imageFileNegZ,
	bool generateMipMaps, tPixelFormat pixelFormat, tTexture::tQuality quality, int forceWidth, int forceHeight
)
{
	Clear();
	tPicture posX(imageFilePosX); tPicture negX(imageFileNegX);
	tPicture posY(imageFilePosY); tPicture negY(imageFileNegY);
	tPicture posZ(imageFilePosZ); tPicture negZ(imageFileNegZ);
	return Set
	(
		posX, negX, posY, negY, posZ, negZ,
		generateMipMaps, pixelFormat, quality, forceWidth, forceHeight
	);
}


bool tCubemap::Set
(
	tPicture& imagePosX, tPicture& imageNegX,
	tPicture& imagePosY, tPicture& imageNegY,
	tPicture& imagePosZ, tPicture& imageNegZ,
	bool generateMipMaps, tPixelFormat pixelFormat, tTexture::tQuality quality, int forceWidth, int forceHeight
)
{
	Clear();

	tTexture textures[int(tSide::NumSides)];
	textures[0].Set(imagePosX, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight);
	textures[1].Set(imageNegX, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight);
	textures[2].Set(imagePosY, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight);
	textures[3].Set(imageNegY, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight);
	textures[4].Set(imagePosZ, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight);
	textures[5].Set(imageNegZ, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight);

	for (int side = 0; side < int(tSide::NumSides); side++)
	{
		Sides[side].Side = textures[side].IsValid() ? tSide(side) : tSide::Invalid;

		// Steal the layers from the texture and give them to the cubemap.
		tList<tLayer> layers;
		textures[side].StealLayers(layers);
		Sides[side].Texture.Set(layers);
	}

	if (!IsValid())
	{
		Clear();
		return false;
	}

	return true;
}


void tCubemap::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Image_Cubemap);
	{
		chunk.Begin(tChunkID::Image_CubemapProperties);
		{
			chunk.Write( AllSidesOpaque() );
		}
		chunk.End();

		chunk.Begin(tChunkID::Image_CubemapSides);
		{
			for (int side = 0; side < int(tSide::NumSides); side++)
			{
				if ((Sides[side].Side == tSide::Invalid) || !Sides[side].Texture.IsValid())
					continue;

				chunk.Begin(tChunkID::Image_CubemapSide);
				{
					chunk.Begin(tChunkID::Image_CubemapSideProperties);
					{
						chunk.Write(tSide(side));
					}
					chunk.End();

					Sides[side].Texture.Save(chunk);
				}
				chunk.End();
			}
		}
		chunk.End();
	}
	chunk.End();
}


void tCubemap::Load(const tChunk& chunk)
{
	Clear();
	if (chunk.ID() != tChunkID::Image_Cubemap)
		return;

	for (tChunk ch = chunk.First(); ch.IsValid(); ch = ch.Next())
	{
		switch (ch.ID())
		{
			case tChunkID::Image_CubemapProperties:
			{
				bool allOpaque = false;
				ch.GetItem(allOpaque);
				break;
			}

			case tChunkID::Image_CubemapSides:
			{
				for (tChunk sideChunk = ch.First(); sideChunk.IsValid(); sideChunk = sideChunk.Next())
				{
					tAssert(sideChunk.ID() == tChunkID::Image_CubemapSide)
					tSide side = tSide::Invalid;
					for (tChunk sideSub = sideChunk.First(); sideSub.IsValid(); sideSub = sideSub.Next())
					{
						switch (sideSub.ID())
						{
							case tChunkID::Image_CubemapSideProperties:
								sideSub.GetItem(side);
								break;

							case tChunkID::Image_Texture:
								tAssert(side != tSide::Invalid);
								Sides[int(side)].Side = side;
								Sides[int(side)].Texture.Load(sideSub);
								break;
						}
					}
				}
				break;
			}
		}
	}
}


bool tCubemap::operator==(const tCubemap& src) const
{
	if (!IsValid() || !src.IsValid())
		return false;

	for (int side = 0; side < int(tSide::NumSides); side++)
	{
		if (Sides[side].Side != src.Sides[side].Side)
			return false;

		if (Sides[side].Texture != src.Sides[side].Texture)
			return false;
	}

	return true;
}
