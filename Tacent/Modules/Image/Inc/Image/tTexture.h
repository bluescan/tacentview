// tTexture.h
//
// A tTexture is a 'hardware-ready' format. tTextures contain functionality for creating mipmap layers in a variety of
// block-compressed and uncompressed formats. A tTexture stores each mipmap layer in a tLayer. A tTexture can be
// created from either a tPicture or a dds file. The purpose of a dds file is so that content-creators have control
// over the authoring of each mipmap level and the exact pixel format used. Basically if you've created a dds file,
// you're saying you want the final hardware to use the image data unchanged and as authored -- same mip levels, same
// pixel format, same dimensions. For this reason, dds files should not be loaded into tPictures where image
// manipulation occurs and possibly lossy block-compressed dds images would be decompressed. A dds file may contain more
// than one image if it is a cubemap, but a tTexture only ever represents a single image. The tTexture dds constructor
// allows you to decide which one gets loaded. tTextures can save and load to a tChunk-based format, and are therefore
// useful at both pipeline and for runtime loading. To save to a tChunk file format a tTexture will call the Save
// method of all the tLayers.
//
// Copyright (c) 2006, 2016, 2017, 2019 Tristan Grimmer.
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
#include <Foundation/tString.h>
#include <System/tChunk.h>
#include "Image/tFileDDS.h"
#include "Image/tPicture.h"
namespace tImage
{


class tTexture : public tLink<tTexture>
{
public:
	// Creates an empty and initially invalid tTexture. You must manually call Set or Load.
	tTexture()																											{ }

	// This constructor is for cases wher you just have a list of layers that you want to give the tTexture. After
	// construction the layer list will be empty. This is used by the tCubemap class so it can grab all 6 sides of a
	// cubemap dds and give them to 6 different tTextures.
	tTexture(tList<tLayer>& layers)																						{ Set(layers); }

	// Constructs from a dds file or dds object. Since dds files can have up to 6 images in them, these are explicitly
	// different constructors than the ones that load other image types. Additionally, the dds constructors don't mess
	// with compression or mip layers. What you put in the dds you get. correctRowOrder should normally be left set to
	// true unless you are loading a cubemap surface. Essentially dds files are upside down in terms of row order, but
	// only the cubemap loader know about it (and they use left handed side ordering).
	tTexture
	(
		const tString& ddsFile, tFileDDS::tSurfIndex surface = tFileDDS::tSurfIndex_Default,
		bool correctRowOrder = true
	)																													{ Load(ddsFile, surface, correctRowOrder); }

	// Similar to above but accepts an in-memory object. The ddsObject will be invalid after as the layers are stolen
	// from it. If necessary, a constructor can easily be added that does a copy and keeps it valid, but it will be
	// less efficient.
	tTexture(tFileDDS& ddsObject, tFileDDS::tSurfIndex surface = tFileDDS::tSurfIndex_Default)							{ Set(ddsObject, surface); }

	// The other constructors require you to know a quality setting for resampling (mipmap generation) and compression.
	// For simplicity there is only Fast and Production quality settings, and it affects resampling _and_ compression.
	enum class tQuality
	{
		Fast,		// Bilinear resample filter. Fast BCn compress mode.
		Production	// Lanczos or Kaiser sinc-based resample filter. High quality BCn compression.
	};

	// This constructor creates a texture from an image file such as a jpg, gif, tga, or bmp. It does this by creating
	// a temporary tPicture object. If tPixelFormat is Auto, the constructor automatically chooses the most appropriate
	// format for the texture based on the image's properties. DXT1 is chosen if the image is perfectly opaque, and
	// DXT5 if it has alphas. This constructor forces power-of-2 texture dimensions and will resample to the nearest
	// power-of-2 if required. If forceWidth is > 0, the image will be resampled if necessary to have that width. The
	// same logic applies to forceHeight. Both forceWidth and forceHeight, if supplied, must be powers of 2.
	tTexture
	(
		const tString& imageFile, bool generateMipMaps, tPixelFormat pixelFormat = tPixelFormat::Auto,
		tQuality quality = tQuality::Production, int forceWidth = 0, int forceHeight = 0
	)																													{ Load(imageFile, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight); }

	// Same as above except that an in-memory tPicture is used instead of a filename. The supplied tPicture will be
	// invalid after this constructor. This is because resampling may occur on the tPicture.
	tTexture
	(
		tPicture& imageObject, bool generateMipMaps, tPixelFormat pixelFormat = tPixelFormat::Auto,
		tQuality quality = tQuality::Production, int forceWidth = 0, int forceHeight = 0
	)																													{ Set(imageObject, generateMipMaps, pixelFormat, quality, forceWidth, forceHeight); }

	virtual ~tTexture()																									{ Clear(); }

	// If any constructor fails, you will be left with an invalid object.
	bool IsValid() const																								{ return (Layers.GetNumItems() > 0) ? true : false; }

	// See the corresponding constructors for a description of the behaviour of these functions. These functions all
	// return true on success. On failure, the tTexture is left invalid and in the case of the layer list Set, the list
	// is emptied.
	bool Set(tList<tLayer>&);
	bool Load
	(
		const tString& ddsFile, tFileDDS::tSurfIndex surface = tFileDDS::tSurfIndex_Default,
		bool correctRowOrder = true
	);
	bool Set(tFileDDS& ddsObject, tFileDDS::tSurfIndex = tFileDDS::tSurfIndex_Default);
	bool Load
	(
		const tString& imageFile, bool generateMipMaps, tPixelFormat = tPixelFormat::Auto,
		tQuality = tQuality::Production, int forceWidth = 0, int forceHeight = 0
	);
	bool Set
	(
		tPicture& imageObject, bool generateMipMaps, tPixelFormat = tPixelFormat::Auto,
		tQuality = tQuality::Production, int forceWidth = 0, int forceHeight = 0
	);

	void Clear()																										{ Layers.Clear(); Opaque = true; }

	int GetWidth() const				/* Returns width of the main layer. */											{ return IsValid() ? Layers.First()->Width : 0; }
	int GetHeight() const				/* Returns width of the main layer. */											{ return IsValid() ? Layers.First()->Height : 0; }
	tPixelFormat GetPixelFormat() const																					{ return IsValid() ? Layers.First()->PixelFormat : tPixelFormat::Invalid; }
	bool IsMipmapped() const																							{ return (Layers.GetNumItems() > 1) ? true : false; }
	void RemoveMipmaps();
	bool IsOpaque() const																								{ return Opaque; }
	int GetNumLayers() const																							{ return Layers.GetNumItems(); }
	int GetNumMipmaps() const																							{ return Layers.GetNumItems(); }
	tLayer* GetFirstLayer() const																						{ return Layers.First(); }
	tLayer* GetMainLayer() const																						{ return Layers.First(); }
	void StealLayers(tList<tLayer>&);																					// Leaves the object invalid.
	const tList<tLayer>& GetLayers()																					{ return Layers; }
	int GetTotalPixelDataSize() const;

	// Save and Load to tChunk format.
	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	// Returns 1 + log2( max(width, height) ). The returned number is how many mipmaps it would take to make the
	// smallest a 1x1 square. Some pipelines may care about this and require all of them if mipmapping at all.
	int ComputeMaxNumberOfMipmaps() const;

	// Textures are considered equal if the pixel format, opacity, and layers are the same. Invalid textures are always
	// considered not equal to other textures, even other invalid textures.
	bool operator==(const tTexture&) const;
	bool operator!=(const tTexture& src) const																			{ return !(*this == src); }

private:
	tPixelFormat DeterminePixelFormat(const tPicture&);
	tPicture::tFilter DetermineFilter(tQuality);
	void ProcessImageTo_R8G8B8_Or_R8G8B8A8(tPicture&, tPixelFormat, bool generateMipmaps, tQuality);
	void ProcessImageTo_G3B5R5G3(tPicture&, bool generateMipmaps, tQuality);
	void ProcessImageTo_BCTC(tPicture&, tPixelFormat, bool generateMipmaps, tQuality);

	// This is legacy code for using Squish directly instead of nVidia Texture Tools 2. Once the Texture Tools version
	// is tested, this can be removed.
	#ifdef TEXTURE_USE_SQUISH_LIB
	void ProcessImageTo_BCTC_Squish(tPicture&, tPixelFormat, bool generateMipmaps, tQuality);
	#endif

	bool Opaque = true;										// Only true if the texture is completely opaque.

	// The tTexture is only valid if there is at least one layer. The texture is considered to have mipmaps if the
	// number of layers is > 1.
	tList<tLayer> Layers;
};


// Implementation below this line.


inline int tTexture::GetTotalPixelDataSize() const
{
	int total = 0;
	for (tLayer* layer = Layers.First(); layer; layer = layer->Next())
		total += layer->GetDataSize();

	return total;
}


inline tPixelFormat tTexture::DeterminePixelFormat(const tPicture& image)
{
	if (Opaque)
		return tPixelFormat::BC1_DXT1;
	else
		return tPixelFormat::BC3_DXT5;
}


inline tPicture::tFilter tTexture::DetermineFilter(tQuality quality)
{
	switch (quality)
	{
		case tQuality::Fast:
			return tPicture::tFilter::Bilinear;

		case tQuality::Production:
			return tPicture::tFilter::Lanczos;
	}
	return tPicture::tFilter::Lanczos;
}


inline void tTexture::RemoveMipmaps()
{
	if (!IsMipmapped())
		return;

	tLayer* main = Layers.Remove();
	Layers.Empty();
	Layers.Append(main);
}


inline void tTexture::StealLayers(tList<tLayer>& layers)
{
	while (!Layers.IsEmpty())
		layers.Append(Layers.Remove());

	Clear();
}


}
