// tCubemap.h
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

#pragma once
#include <System/tChunk.h>
#include "Image/tTexture.h"
namespace tImage
{


class tCubemap : public tLink<tCubemap>
{
public:
	// Creates an empty and initially invalid tCubemap. You must manually call Set or Load.
	tCubemap()																											{ }

	// Construct from a ddsfile. The dds file must contain a cubemap or the object will be left invalid.
	tCubemap(const tString& ddsFile)																					{ Load(ddsFile); }

	// Construct from 6 non-dds image files directly. You do not need them all to be valid. tCubemap supports having one
	// to six sides. Use LH coord system. See tTexture for the last 6 processing arguments.
	tCubemap
	(
		const tString& imageFilePosX, const tString& imageFileNegX,
		const tString& imageFilePosY, const tString& imageFileNegY,
		const tString& imageFilePosZ, const tString& imageFileNegZ,
		bool generateMipMaps, tPixelFormat pixelFormat = tPixelFormat::Auto,
		tTexture::tQuality quality = tTexture::tQuality::Production,
		int forceWidth = 0, int forceHeight = 0
	)																													{ Load(imageFilePosX, imageFileNegX, imageFilePosY, imageFileNegY, imageFilePosZ, imageFileNegZ, generateMipMaps, pixelFormat, quality, forceWidth,forceHeight); }

	// Constructs from an in-memory tFileDDS object. The passed in object will be invalid afterwards.
	tCubemap(tFileDDS& ddsObj)																							{ Set(ddsObj); }

	// Constructs from 6 in-memory tPicture objects. The passed in objects will be invalid afterwards. You do not need
	// them all to be valid. tCubemap supports having one to six sides. Use LH coord system. See tTexture for the last
	// 6 processing arguments.
	tCubemap
	(
		tPicture& imagePosX, tPicture& imageNegX,
		tPicture& imagePosY, tPicture& imageNegY,
		tPicture& imagePosZ, tPicture& imageNegZ,
		bool generateMipMaps, tPixelFormat pixelFormat = tPixelFormat::Auto,
		tTexture::tQuality quality = tTexture::tQuality::Production,
		int forceWidth = 0, int forceHeight = 0
	)																													{ Set(imagePosX, imageNegX, imagePosY, imageNegY, imagePosZ, imageNegZ, generateMipMaps, pixelFormat, quality, forceWidth,forceHeight); }
	virtual ~tCubemap()																									{ Clear(); }

	// At least one side must be valid for the cubemap to be valid.
	bool IsValid() const;
	void Clear();
	bool AllSidesOpaque() const;

	// The load and set calls all return success. If the input is non const, it invalidates the passed in object.
	bool Load(const tString& ddsFile, bool reverseRowOrder = true);
	bool Load
	(
		const tString& imageFilePosX, const tString& imageFileNegX,
		const tString& imageFilePosY, const tString& imageFileNegY,
		const tString& imageFilePosZ, const tString& imageFileNegZ,
		bool generateMipMaps, tPixelFormat = tPixelFormat::Auto,
		tTexture::tQuality = tTexture::tQuality::Production,
		int forceWidth = 0, int forceHeight = 0
	);
	bool Set(tFileDDS&);
	bool Set
	(
		tPicture& imagePosX, tPicture& imageNegX,
		tPicture& imagePosY, tPicture& imageNegY,
		tPicture& imagePosZ, tPicture& imageNegZ,
		bool generateMipMaps, tPixelFormat = tPixelFormat::Auto,
		tTexture::tQuality = tTexture::tQuality::Production,
		int forceWidth = 0, int forceHeight = 0
	);

	enum class tSide
	{
		Invalid				= -1,
		PosX,		Right	= PosX,
		NegX,		Left	= NegX,
		PosY,		Top		= PosY,
		NegY,		Bottom	= NegY,
		PosZ,		Front	= PosZ,
		NegZ,		Back	= NegZ,
		NumSides
	};
	tTexture* GetSide(tSide);

	// Save and Load to tChunk format.
	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	// Cubemaps are considered equal if all their individual tTextures represent the same cubemap side and all the
	// tTextures are equal. Invalid cubemaps are not equal to anything, including other invalids.
	bool operator==(const tCubemap&) const;
	bool operator!=(const tCubemap& src) const																			{ return !(*this == src); }

private:

	struct SideTex
	{
		SideTex()																										: Side(tSide::Invalid), Texture() { }
		tSide Side;
		tTexture Texture;
	};

	// Not all sides are required to be valid.
	SideTex Sides[int(tSide::NumSides)];
};


}
