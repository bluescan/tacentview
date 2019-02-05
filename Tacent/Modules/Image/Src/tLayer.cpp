// tLayer.cpp
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

#include "Image/tLayer.h"
using namespace tImage;


bool tLayer::IsOpaqueFormat() const
{
	// We assume that if the layer is using a format that supports transparency that there are, in fact, transparent
	// pixels present. It won't hurt if there aren't, but it would be rendered with blending on if the material was
	// opaque and might be a bit slower.
	switch (PixelFormat)
	{
		case tPixelFormat::R8G8B8A8:
		case tPixelFormat::B8G8R8A8:
		case tPixelFormat::BC1_DXT1BA:
		case tPixelFormat::BC2_DXT3:
		case tPixelFormat::BC3_DXT5:
		case tPixelFormat::G3B5A1R5G2:
		case tPixelFormat::G4B4A4R4:
		case tPixelFormat::L8A8:
			return false;

		default:
			return true;
	}

	return true;
}


bool tLayer::operator==(const tLayer& src) const
{
	if (!IsValid() || !src.IsValid())
		return false;

	if (PixelFormat != src.PixelFormat)
		return false;

	if ((Width != src.Width) || (Height != src.Height))
		return false;

	int dataSize = GetDataSize();
	return tStd::tMemcmp(Data, src.Data, dataSize) ? false : true;
}


void tLayer::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Image_Layer);
	{
		// This chunk must be saved first.
		chunk.Begin(tChunkID::Image_LayerProperties);
		{
			chunk.Write(PixelFormat);
			chunk.Write(Width);
			chunk.Write(Height);
		}
		chunk.End();

		chunk.Begin(tChunkID::Image_LayerData);
		{
			int dataSize = GetDataSize();
			chunk.Write(Data, dataSize);
		}
		chunk.End();
	}
	chunk.End();
}


void tLayer::Load(const tChunk& chunk, bool ownsData)
{
	Clear();
	if (chunk.ID() != tChunkID::Image_Layer)
		return;

	OwnsData = ownsData;
	for (tChunk ch = chunk.First(); ch.IsValid(); ch = ch.Next())
	{
		switch (ch.ID())
		{
			// This chunk must be loaded first.
			case tChunkID::Image_LayerProperties:
			{
				ch.GetItem(PixelFormat);
				ch.GetItem(Width);
				ch.GetItem(Height);
				break;
			}

			case tChunkID::Image_LayerData:
			{
				int dataSize = GetDataSize();
				if (dataSize <= 0)
					return;

				tAssert(!Data);
				if (OwnsData)
				{
					Data = new uint8[dataSize];
					tStd::tMemcpy(Data, ch.Data(), dataSize);
				}
				else
				{
					Data = (uint8*)ch.Data();
				}
				break;
			}
		}
	}
}
