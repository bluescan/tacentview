// TacitImage.h
//
// An image class that can load a file from disk into main memory and to VRAM.
//
// Copyright (c) 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <GL/glew.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#include <Image/tTexture.h>
#include <System/tFile.h>
#include "TacitImage.h"
using namespace tStd;
using namespace tSystem;
using namespace tImage;


TacitImage::TacitImage() :
	Filename(),
	GLTextureID(0)
{
	Filetype = tFileType::Unknown;
}


TacitImage::TacitImage(const tString& filename) :
	Filename(filename),
	GLTextureID(0)
{
	Filetype = tGetFileType(Filename);
}


bool TacitImage::Load(const tString& filename)
{
	if (filename.IsEmpty())
		return false;

	Filename = filename;
	Filetype = tGetFileType(Filename);

	return Load();
}


bool TacitImage::Load()
{
	if (IsLoaded())
		return true;

	if (Filetype == tFileType::Unknown)
		return false;

	bool success = false;
	try
	{
		if (Filetype == tSystem::tFileType::DDS)
			success = TextureImage.Load(Filename);
		else
			success = PictureImage.Load(Filename);
	}
	catch (tError error)
	{
		success = false;
	}

	if (Filetype == tSystem::tFileType::DDS)
		ConvertTextureToPicture();

	return success;
}


bool TacitImage::IsLoaded() const
{
	return (TextureImage.IsValid() || PictureImage.IsValid());
}


int TacitImage::GetWidth() const
{
	if (PictureImage.IsValid())
		return PictureImage.GetWidth();

	if (TextureImage.IsValid())
		return TextureImage.GetWidth();

	return 0;
}


int TacitImage::GetHeight() const
{
	if (PictureImage.IsValid())
		return PictureImage.GetHeight();

	if (TextureImage.IsValid())
		return TextureImage.GetHeight();

	return 0;
}


tColouri TacitImage::GetPixel(int x, int y) const
{
	if (PictureImage.IsValid())
		return PictureImage.GetPixel(x, y);

	// Generally the PictureImage should always be valid. When dds files (tTextures) are loaded, they get
	// uncompressed into valid PictureImage files so the pixel info can be read.
	return tColouri::black;
}


void TacitImage::PrintInfo()
{
	tPixelFormat format = tPixelFormat::Invalid;
	if (Filetype == tSystem::tFileType::DDS)
		format = TextureImage.GetPixelFormat();
	else
		format = PictureImage.IsOpaque() ? tPixelFormat::R8G8B8 : tPixelFormat::R8G8B8A8;
	tPrintf
	(
		"Image: %s Width: %d Height: %d PixelFormat: %s\n",
		tSystem::tGetFileName(Filename).ConstText(),
		GetWidth(), GetHeight(), tImage::tGetPixelFormatName(format)
	);
}


bool TacitImage::Bind()
{
	if (GLTextureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, GLTextureID);
		return true;
	}

	if (!IsLoaded())
		return false;

	glGenTextures(1, &GLTextureID);
	if (GLTextureID == 0)
		return false;

	// We try to bind the native tTexture first if possible.
	if (TextureImage.IsValid())
	{
		const tList<tLayer>& layers = TextureImage.GetLayers();
		BindLayers(layers);
		return true;
	}

	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gPicture.GetWidth(), gPicture.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());
	if (PictureImage.IsValid())
	{
		tList<tLayer> layers;
		layers.Append
		(
			new tLayer
			(
				tPixelFormat::R8G8B8A8, PictureImage.GetWidth(), PictureImage.GetHeight(),
				(uint8*)PictureImage.GetPixelPointer()
			)
		);

		BindLayers(layers);
		return true;
	}

	return false;
}


void TacitImage::BindLayers(const tList<tLayer>& layers)
{
	if (layers.IsEmpty())
		return;

	glBindTexture(GL_TEXTURE_2D, GLTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// If the texture format is a mipmapped one, we need to set up OpenGL slightly differently.
	bool mipmapped = layers.GetNumItems() > 1;
	if (mipmapped)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int mipmapLevel = 0;
	for (tLayer* layer = layers.First(); layer; layer = layer->Next(), mipmapLevel++)
	{
		GLint srcFormat, dstFormat;
		GLenum srcType;
		bool compressed;
		tPixelFormat pixelFormat = layers.First()->PixelFormat;
		GetGLFormatInfo(srcFormat, srcType, dstFormat, compressed, pixelFormat);

		if (compressed)
		{
			// For each layer (non-mipmapped formats will only have one) we need to submit the texture data.
			// Do a straight DMA. No conversion. Fast.
			glCompressedTexImage2D(GL_TEXTURE_2D, mipmapLevel, dstFormat, layer->Width, layer->Height, 0, layer->GetDataSize(), layer->Data);
		}
		else
		{
			// Although this call can handle compressing during the DMA, it should never need to do any work because
			// the internal and external texture formats should always be identical. This isn't always entirely true.
			// The nVidia paper "Achieving Efficient Bandwidth Rates" explains that the src data should be in BGRA,
			// while the dest can be RGBA8 (for 32bit textures). This is because internally to the driver the OpenGL
			// internalFormal GL_RGBA8 will be stored as BGRA so if the source isn't BGRA then some swizzling takes
			// place. This is why PixelFormat_B8G8R8A8 is quite efficient for example.
			glTexImage2D(GL_TEXTURE_2D, mipmapLevel, dstFormat, layer->Width, layer->Height, 0, srcFormat, srcType, layer->Data);
		}
	}
}


void TacitImage::GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tPixelFormat pixelFormat)
{
	srcFormat = GL_RGBA;
	srcType = GL_UNSIGNED_BYTE;

	// Note that the destination format only specifies the resolution of each colour component,
	// not the order or the specifics... they're up to the OpenGL driver. For example, nVidia cards
	// use an internal BGRA format when GL_RGBA8 is specified... that's why having srcFormat = GL_BGRA
	// and dstFormat = RGBA8 would be very efficient (no swizzling).
	dstFormat = GL_RGBA8;
	compressed = false;

	tAssert(GLEW_ARB_texture_compression);
	switch (pixelFormat)
	{
		case tPixelFormat::R8G8B8:
			srcFormat = GL_RGB;
			dstFormat = GL_RGB8;
			break;

		case tPixelFormat::R8G8B8A8:
			srcFormat = GL_RGBA;
			dstFormat = GL_RGBA8;
			break;

		case tPixelFormat::B8G8R8:		// Efficient transfer to VRAM.
			srcFormat = GL_BGR;
			dstFormat = GL_RGB8;
			break;

		case tPixelFormat::B8G8R8A8:	// Efficient transfer to VRAM.
			srcFormat = GL_BGRA;
			dstFormat = GL_RGBA8;
			break;

		case tPixelFormat::BC1_DXT1BA:
			srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			compressed = true;
			break;

		case tPixelFormat::BC1_DXT1:
			srcFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			compressed = true;
			break;

		case tPixelFormat::BC2_DXT3:
			srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			compressed = true;
			break;

		case tPixelFormat::BC3_DXT5:
			srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			compressed = true;
			break;

		case tPixelFormat::G3B5A1R5G2:
			srcFormat = GL_BGRA;								// The type reverses this order to ARGB, just like the PixelFormat. Cuz GL_UNSIGNED_SHORT is to be interpreted as little endian, the type swaps the bytes yielding the proper G3B5A1R5G2.
			srcType = GL_UNSIGNED_SHORT_1_5_5_5_REV;			// This type is a special case and applies to the entire BGRA group (unlike GL_UNSIGNED_BYTE).
			dstFormat = GL_RGB5_A1;
			break;

		case tPixelFormat::G4B4A4R4:
			srcFormat = GL_BGRA;								// The type reverses this order to ARGB, just like the PixelFormat. Cuz GL_UNSIGNED_SHORT is to be interpreted as little endian, the type swaps the bytes yielding the proper G4B4A4R4.
			srcType = GL_UNSIGNED_SHORT_4_4_4_4_REV;			// This type is a special case and applies to the entire BGRA group (unlike GL_UNSIGNED_BYTE).
			dstFormat = GL_RGBA4;
			break;

		case tPixelFormat::G3B5R5G3:
			srcFormat = GL_RGB;									// Cuz GL_UNSIGNED_SHORT is to be interpreted as little endian, the unsigned short modifies the format R5G6B5 to the desired src format of G3B5R5G3.
			srcType = GL_UNSIGNED_SHORT_5_6_5;					// This type is a special case and applies to the entire RGB group (unlike GL_UNSIGNED_BYTE).
			dstFormat = GL_RGB5;								// Usually the OpenGL driver gives you a 565 format for this. Don't know why a the exact internal format doesn't exist.
			break;
	}
}


bool TacitImage::ConvertTextureToPicture()
{
	if (!(TextureImage.IsValid() && !PictureImage.IsValid()))
		return false;

	int w = TextureImage.GetWidth();
	int h = TextureImage.GetHeight();
	Bind();
	uint8* rgbaData = new uint8[w * h * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
	PictureImage.Set(w, h, (tPixel*)rgbaData, false);
	return true;
}
