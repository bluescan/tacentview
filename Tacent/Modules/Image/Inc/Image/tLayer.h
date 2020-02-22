// tLayer.h
//
// A tLayer is a data container for texture pixel data. It is used only by the tTexture class to store image data. It
// contains data that may be in a variety of hardware-ready formats, like dxt5. It is primarily used to store the
// multiple mipmap layers of a texture. Main members are width, height, pixel format, and a function to compute data
// size based on those three variables. It knows how to save and load itself in tChunk format.
//
// tLayers may have any width and height in [1, MaxLayerDimension]. If the pixel format is block-based (4x4 pixels)
// the tLayer still allows smaller than 4 width and height. However, a whole block is still needed so the number of
// bytes will be the block size for the particular BC format. For example, a 1x1 BC1 format layer still needs 8 bytes.
// A 5x5 BC format layer would need 4 blocks (same as an 8x8). The tLayer does not place further constraints on width
// and height. A higher level system, for example, may want to ensure power-of-two sizes, or multiple of 4, but that
// shouldn't and doesn't happen here.
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
#include <System/tChunk.h>
#include <Image/tPixelFormat.h>
namespace tImage
{


class tLayer : public tLink<tLayer>
{
public:
	tLayer()																											: PixelFormat(tPixelFormat::Invalid), Width(0), Height(0), Data(nullptr), OwnsData(true) { }

	// Constructs the layer from the chunk. Normally it copies the layer data from the chunk. However, if ownData is
	// false no mem-copy is performed and the layer just points into the chunk data. In this case it is vital that the
	// chunk object exists for the entire lifespan of the layer.
	tLayer(const tChunk& chunk, bool ownsData = true)																	: PixelFormat(tPixelFormat::Invalid), Width(0), Height(0), Data(nullptr), OwnsData(ownsData) { Load(chunk, ownsData); }

	// Constructs a layer having the supplied width, height, and number of bytes. If steal memory is true, the data
	// array is passed off to this class and it will manage the array deleting it when necessary. If steal is false,
	// the data gets mem-copied into this object.
	tLayer(tPixelFormat, int width, int height, uint8* data, bool steal = false);

	tLayer(const tLayer&);
	virtual ~tLayer()																									{ Clear(); }

	bool IsValid() const																								{ return Data ? true : false; }

	// Returns the size of the data in bytes. For BC formats the data size will be a multiple of the block size. For
	// example, a 1x1 BC1 format layer still needs 8 bytes. A 5x5 BC1 format layer would need a whole 4 blocks (same as
	// an 8x8) and would yield 32 bytes.
	int GetDataSize() const;

	void Save(tChunkWriter&) const;

	// Loads the layer from a chunk. Any previous layer data gets destroyed. The bool ownData works the same as with
	// the constructor.
	void Load(const tChunk&, bool ownData);

	// Frees internal layer data and makes the layer invalid.
	void Clear()																										{ PixelFormat = tPixelFormat::Invalid; Width = Height = 0; if (OwnsData) delete[] Data; Data = nullptr; OwnsData = true; }

	// This just checks the pixel format to see if it supports alpha. It does NOT check the data.
	bool IsOpaqueFormat() const;

	// An invalid layer is never considered equal to another layer even if the other layer is also invalid. Whether the
	// layer owns the data is considered irrelevant for equivalency purposes.
	bool operator==(const tLayer&) const;
	bool operator!=(const tLayer& src) const																			{ return !(*this == src); }

	tPixelFormat PixelFormat;
	int Width, Height;
	uint8* Data;
	bool OwnsData;

	// Most hardware can handle up to a 4096 x 4096 texture.
	const static int MaxLayerDimension = 4096;
	const static int MinLayerDimension = 1;
};


// Implementation below this line.


inline tLayer::tLayer(tPixelFormat format, int width, int height, uint8* data, bool steal) :
	PixelFormat(format),
	Width(width),
	Height(height),
	Data(nullptr),
	OwnsData(true)
{
	if (!width || !height || !data)
		return;

	if (steal)
	{
		Data = data;
	}
	else
	{
		int dataSize = GetDataSize();
		Data = new uint8[dataSize];
		tStd::tMemcpy(Data, data, dataSize);
	}
}


inline tLayer::tLayer(const tLayer& src) :
	PixelFormat(src.PixelFormat),
	Width(src.Width),
	Height(src.Height),
	OwnsData(src.OwnsData)
{
	if (OwnsData)
	{
		int dataSize = src.GetDataSize();
		Data = new uint8[dataSize];
		tStd::tMemcpy(Data, src.Data, dataSize);
	}
	else
	{
		Data = src.Data;
	}
}


inline int tLayer::GetDataSize() const
{
	if (!Width || !Height || (PixelFormat == tPixelFormat::Invalid))
		return 0;

	int numBytes = 0;
	if (tIsBlockFormat(PixelFormat))
	{
		int widthNumBlocks = (Width + ((Width%4) ? 4-(Width%4) : 0)) >> 2;
		int heightNumBlocks = (Height + ((Height%4) ? 4-(Height%4) : 0)) >> 2;
		int numBlocks = widthNumBlocks * heightNumBlocks;
		numBytes = numBlocks * tGetBytesPer4x4PixelBlock(PixelFormat);
	}
	else
	{
		numBytes = Width * Height * (tGetBitsPerPixel(PixelFormat) >> 3);
	}

	return numBytes;
}


}
