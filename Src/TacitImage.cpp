// TacitImage.cpp
//
// An image class that can load a file from disk into main memory and to VRAM.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <mutex>
#include <chrono>
#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#include <Math/tHash.h>
#include <Math/tFundamentals.h>
#include <Image/tTexture.h>
#include <Image/tImageGIF.h>
#include <System/tFile.h>
#include <System/tTime.h>
#include <System/tMachine.h>
#include <System/tChunk.h>
#include "TacitImage.h"
#include "Settings.h"
using namespace tStd;
using namespace tSystem;
using namespace tImage;
using namespace tMath;
using namespace TexView;
int TacitImage::ThumbnailNumThreadsRunning = 0;
tString TacitImage::ThumbCacheDir;
namespace TexView { extern Settings Config; }


const int TacitImage::ThumbWidth		= 256;
const int TacitImage::ThumbHeight		= 144;
const int TacitImage::ThumbMinDispWidth	= 64;


TacitImage::TacitImage() :
	Filename(),
	Filetype(tFileType::Unknown),
	FileSizeB(0),
	LoadParams()
{
	tMemset(&FileModTime, 0, sizeof(FileModTime));
	ResetLoadParams();
}


TacitImage::TacitImage(const tString& filename) :
	Filename(filename),
	Filetype(tGetFileType(filename)),
	FileSizeB(0),
	LoadParams()
{
	tMemset(&FileModTime, 0, sizeof(FileModTime));
	ResetLoadParams();
	tSystem::tFileInfo info;
	if (tSystem::tGetFileInfo(info, filename))
	{
		FileModTime = info.ModificationTime;
		FileSizeB = info.FileSize;
	}
}


TacitImage::~TacitImage()
{
	// If we're being destroyed before the thumbnail thread is done, we have to wait because that thread
	// accesses the thumbnail picture of this object... so 'this' must be valid.
	if (ThumbnailThread.joinable())
		ThumbnailThread.join();

	// It is important that the thread count decrements if necessary since TacitImages can be deleted
	// when changing folders. The threads need to be available to do more work in a new folder.
	if (ThumbnailRequested && ThumbnailThreadRunning)
	{
		ThumbnailNumThreadsRunning--;
		tiClampMin(ThumbnailNumThreadsRunning, 0);
	}

	// Free GPU image mem and texture IDs.
	Unload(true);
}


void TacitImage::ResetLoadParams()
{
	LoadParams = tImage::tPicture::LoadParams();
	LoadParams.GammaValue = TexView::Config.MonitorGamma;
}


bool TacitImage::Load(const tString& filename)
{
	if (filename.IsEmpty())
		return false;

	Filename = filename;
	Filetype = tGetFileType(Filename);
	tSystem::tFileInfo info;
	if (tSystem::tGetFileInfo(info, filename))
	{
		FileModTime = info.ModificationTime;
		FileSizeB = info.FileSize;
	}

	return Load();
}


bool TacitImage::Load()
{
	if (IsLoaded() && !Dirty)
	{
		LoadedTime = tSystem::tGetTime();
		return true;
	}

	if (Filetype == tFileType::Unknown)
		return false;

	Info.SrcPixelFormat = tPixelFormat::Invalid;
	bool success = false;
	try
	{
		if (Filetype == tSystem::tFileType::DDS)
		{
			success = DDSCubemap.Load(Filename);
			if (success)
			{
				Info.SrcPixelFormat = DDSCubemap.GetSide(tImage::tCubemap::tSide::PosX)->GetPixelFormat();
			}
			else
			{
				success = DDSTexture2D.Load(Filename);
				Info.SrcPixelFormat = DDSTexture2D.GetPixelFormat();
			}
		}
		else if (Filetype == tSystem::tFileType::GIF)
		{
			tImageGIF gif;
			bool ok = gif.Load(Filename.Chars());
			if (!ok)
				return false;

			int numFrames = gif.GetNumFrames();
			for (int f = 0; f < numFrames; f++)
			{
				tImageGIF::Frame* frame = gif.StealFrame(0);
				tPicture* picture = new tPicture();
				picture->Set(gif.GetWidth(), gif.GetHeight(), frame->Pixels, false);
				picture->Duration = frame->Duration;
				delete frame;
				Pictures.Append(picture);
			}
			Info.SrcPixelFormat = gif.SrcPixelFormat;
			success = true;
		}
		else
		{
			// Some image files (like tiff and exr files) may store multiple images in one file. These are called 'parts'.
			int partNum = 0;
			bool ok = false;
			do
			{
				tPicture* picture = new tPicture();
				ok = picture->Load(Filename, partNum, LoadParams);
				if (ok)
				{
					Pictures.Append(picture);
					partNum++;
				}
			} while (ok);

			if (Pictures.NumItems() > 0)
			{
				success = true;
				Info.SrcPixelFormat = Pictures.First()->SrcPixelFormat;
			}
		}
	}
	catch (tError error)
	{
		success = false;
	}

	if (!success)
		return false;

	if (Filetype == tSystem::tFileType::DDS)
	{
		if (DDSCubemap.IsValid())
			ConvertCubemapToPicture();
		else if (DDSTexture2D.IsValid())
			ConvertTexture2DToPicture();
	}

	LoadedTime = tSystem::tGetTime();

	// Fill in rest of info struct.
	Info.Opaque				= IsOpaque();
	Info.FileSizeBytes		= tSystem::tGetFileSize(Filename);
	Info.MemSizeBytes		= GetMemSizeBytes();

	// Create alt image if possible.
	if (DDSCubemap.IsValid())
		CreateAltPictureFromDDS_Cubemap();

	else if (DDSTexture2D.IsValid() && (DDSTexture2D.GetNumMipmaps() > 1))
		CreateAltPictureFromDDS_2DMipmaps();

	ClearDirty();
	return true;
}


int TacitImage::GetMemSizeBytes() const
{
	int numBytes = 0;
	for (tPicture* pic = Pictures.First(); pic; pic = pic->Next())
		numBytes += pic->GetNumPixels() * sizeof(tPixel);

	numBytes += AltPicture.IsValid() ? AltPicture.GetNumPixels()*sizeof(tPixel) : 0;
	return numBytes;
}


void TacitImage::CreateAltPictureFromDDS_2DMipmaps()
{
	int width = 0;
	for (tPicture* layer = Pictures.First(); layer; layer = layer->Next())
		width += layer->GetWidth();
	int height = GetHeight();

	AltPicture.Set(width, height, tPixel::transparent);
	int originY = 0;
	int originX = 0;
	for (tPicture* layer = Pictures.First(); layer; layer = layer->Next())
	{
		for (int y = 0; y < layer->GetHeight(); y++)
		{
			for (int x = 0; x < layer->GetWidth(); x++)
			{
				tPixel pixel = layer->GetPixel(x, y);
				AltPicture.SetPixel(originX + x, y, pixel);
			}
		}
		originX += layer->GetWidth();
	}
}


void TacitImage::CreateAltPictureFromDDS_Cubemap()
{
	int width = Pictures.First()->GetWidth();
	int height = Pictures.First()->GetHeight();

	AltPicture.Set(width*4, height*3, tPixel::transparent);
	int originX, originY;
	
	// PosZ
	tPicture* pic = Pictures.First();
	originX = width; originY = height;
	for (int y = 0; y < pic->GetHeight(); y++)
		for (int x = 0; x < pic->GetWidth(); x++)
			AltPicture.SetPixel(originX + x, originY + y, pic->GetPixel(x, y));

	// NegZ
	pic = pic->Next();
	originX = 3*width; originY = height;
	for (int y = 0; y < pic->GetHeight(); y++)
		for (int x = 0; x < pic->GetWidth(); x++)
			AltPicture.SetPixel(originX + x, originY + y, pic->GetPixel(x, y));

	// PosX
	pic = pic->Next();
	originX = 2*width; originY = height;
	for (int y = 0; y < pic->GetHeight(); y++)
		for (int x = 0; x < pic->GetWidth(); x++)
			AltPicture.SetPixel(originX + x, originY + y, pic->GetPixel(x, y));

	// NegX
	pic = pic->Next();
	originX = 0; originY = height;
	for (int y = 0; y < pic->GetHeight(); y++)
		for (int x = 0; x < pic->GetWidth(); x++)
			AltPicture.SetPixel(originX + x, originY + y, pic->GetPixel(x, y));

	// PosY
	pic = pic->Next();
	originX = width; originY = 2*height;
	for (int y = 0; y < pic->GetHeight(); y++)
		for (int x = 0; x < pic->GetWidth(); x++)
			AltPicture.SetPixel(originX + x, originY + y, pic->GetPixel(x, y));

	// NegY
	pic = pic->Next();
	originX = width; originY = 0;
	for (int y = 0; y < pic->GetHeight(); y++)
		for (int x = 0; x < pic->GetWidth(); x++)
			AltPicture.SetPixel(originX + x, originY + y, pic->GetPixel(x, y));
}


bool TacitImage::Unload(bool force)
{
	if (!IsLoaded())
		return true;

	// Not allowed to unload if dirty (modified).
	if (Dirty && !force)
		return false;

	Unbind();
	DDSTexture2D.Clear();
	DDSCubemap.Clear();
	AltPicture.Clear();
	AltPictureEnabled = false;
	Pictures.Clear();
	Info.MemSizeBytes = 0;

	LoadedTime = -1.0f;
	return true;
}


void TacitImage::Unbind()
{
	for (tPicture* pic = Pictures.First(); pic; pic = pic->Next())
	{
		if (pic->TextureID != 0)
		{
			glDeleteTextures(1, &pic->TextureID);
			pic->TextureID = 0;
		}
	}

	if (TexIDAlt != 0)
	{
		glDeleteTextures(1, &TexIDAlt);
		TexIDAlt = 0;
	}
}


bool TacitImage::IsOpaque() const
{
	if (DDSCubemap.IsValid())
		return DDSCubemap.AllSidesOpaque();

	if (DDSTexture2D.IsValid())
		return DDSTexture2D.IsOpaque();

	tPicture* picture = Pictures.First();
	if (picture && picture->IsValid())
		return picture->IsOpaque();

	return true;
}


int TacitImage::GetWidth() const
{
	if (AltPicture.IsValid() && AltPictureEnabled)
		return AltPicture.GetWidth();

	tPicture* picture = GetCurrentPic();
	if (picture && picture->IsValid())
		return picture->GetWidth();

	return 0;
}


int TacitImage::GetHeight() const
{
	if (AltPicture.IsValid() && AltPictureEnabled)
		return AltPicture.GetHeight();

	tPicture* picture = GetCurrentPic();
	if (picture && picture->IsValid())
		return picture->GetHeight();

	return 0;
}


tColouri TacitImage::GetPixel(int x, int y) const
{
	if (AltPicture.IsValid() && AltPictureEnabled)
		return AltPicture.GetPixel(x, y);

	tPicture* picture = GetCurrentPic();
	if (picture && picture->IsValid())
		return picture->GetPixel(x, y);

	// Generally the PictureImage should always be valid. When dds files (tTextures) are loaded, they get
	// uncompressed into valid PictureImage files so the pixel info can be read.
	return tColouri::black;
}


void TacitImage::Rotate90(bool antiClockWise)
{
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Rotate90(antiClockWise);

	Dirty = true;
}


void TacitImage::Flip(bool horizontal)
{
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Flip(horizontal);

	Dirty = true;
}


void TacitImage::Crop(int newWidth, int newHeight, int originX, int originY)
{
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Crop(newWidth, newHeight, originX, originY);

	Dirty = true;
}


void TacitImage::PrintInfo()
{
	tPixelFormat format = tPixelFormat::Invalid;
	if (Filetype == tSystem::tFileType::DDS)
	{
		if (DDSCubemap.IsValid())
			format = DDSCubemap.GetSide(tCubemap::tSide::PosX)->GetPixelFormat();
		else
			format = DDSTexture2D.GetPixelFormat();
	}
	else
	{
		tPicture* picture = Pictures.First();
		if (picture)
			format = picture->IsOpaque() ? tPixelFormat::R8G8B8 : tPixelFormat::R8G8B8A8;
	}
}


uint64 TacitImage::Bind()
{
	if (AltPictureEnabled && AltPicture.IsValid())
	{
		if (TexIDAlt != 0)
		{
			glBindTexture(GL_TEXTURE_2D, TexIDAlt);
			return TexIDAlt;
		}

		glGenTextures(1, &TexIDAlt);
		if (TexIDAlt == 0)
			return 0;

		tList<tLayer> layers;
		layers.Append
		(
			new tLayer
			(
				tPixelFormat::R8G8B8A8, AltPicture.GetWidth(), AltPicture.GetHeight(),
				(uint8*)AltPicture.GetPixelPointer()
			)
		);

		BindLayers(layers, TexIDAlt);
		return TexIDAlt;
	}

	if (GetCurrentPic()->TextureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, GetCurrentPic()->TextureID);
		return GetCurrentPic()->TextureID;
	}

	if (!IsLoaded())
		return 0;

	for (tPicture* pic = Pictures.First(); pic; pic = pic->Next())
		glGenTextures(1, &pic->TextureID);

	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
	{
		if (picture && picture->IsValid())
		{
			tList<tLayer> layers;
			layers.Append
			(
				new tLayer
				(
					tPixelFormat::R8G8B8A8, picture->GetWidth(), picture->GetHeight(),
					(uint8*)picture->GetPixelPointer()
				)
			);

			BindLayers(layers, picture->TextureID);
		}
	}
	return GetCurrentPic()->TextureID;
}


void TacitImage::BindLayers(const tList<tLayer>& layers, uint texID)
{
	if (layers.IsEmpty())
		return;

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

	#ifdef PLATFORM_WINDOWS
	tAssert(GLEW_ARB_texture_compression);
	#else
	tAssert(GL_EXT_texture_compression_s3tc);
	#endif
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


bool TacitImage::ConvertTexture2DToPicture()
{
	if (!DDSTexture2D.IsValid() || !(Pictures.Count() <= 0))
		return false;

	int w = DDSTexture2D.GetWidth();
	int h = DDSTexture2D.GetHeight();

	// We need to get the data into the GPU so we cat read the uncompressed version back.
	uint tempTexID = 0;
	glGenTextures(1, &tempTexID);
	if (tempTexID == 0)
		return false;

	const tList<tLayer>& layers = DDSTexture2D.GetLayers();
	BindLayers(layers, tempTexID);

	int numMipmaps = DDSTexture2D.GetNumLayers();
	for (int level = 0; level < numMipmaps; level++)
	{
		int mipW = w >> level;
		tiClampMin(mipW, 1);
		int mipH = h >> level;
		tiClampMin(mipH, 1);
		uint8* rgbaData = new uint8[mipW * mipH * 4];
		glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
		Pictures.Append(new tPicture(mipW, mipH, (tPixel*)rgbaData, false));
	}

	glDeleteTextures(1, &tempTexID);
	return true;
}


bool TacitImage::ConvertCubemapToPicture()
{
	if (!DDSCubemap.IsValid() || !(Pictures.Count() <= 0))
		return false;

	tTexture* tex = DDSCubemap.GetSide(tCubemap::tSide::PosX);
	int w = tex->GetWidth();
	int h = tex->GetHeight();

	// We want the front (+Z) to be the first image.
	int sideOrder[int(tCubemap::tSide::NumSides)] =
	{
		int(tCubemap::tSide::PosZ),
		int(tCubemap::tSide::NegZ),
		int(tCubemap::tSide::PosX),
		int(tCubemap::tSide::NegX),
		int(tCubemap::tSide::PosY),
		int(tCubemap::tSide::NegY)
	};

	for (int s = 0; s < int(tCubemap::tSide::NumSides); s++)
	{
		int side = sideOrder[s];
		uint tempTexID = 0;
		glGenTextures(1, &tempTexID);

		tTexture* tex = DDSCubemap.GetSide(tCubemap::tSide(side));
		tList<tLayer> layers;
		layers.Append(new tLayer(*tex->GetLayers().First()));
		BindLayers(layers, tempTexID);

		uint8* rgbaData = new uint8[w * h * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
		Pictures.Append(new tPicture(w, h, (tPixel*)rgbaData, false));

		layers.Empty();
		glDeleteTextures(1, &tempTexID);
	}
	return true;
}


uint64 TacitImage::BindThumbnail()
{
	if (!ThumbnailRequested)
		return 0;

	if (!ThumbnailThreadFlag.test_and_set())
	{
		ThumbnailThread.join();
		ThumbnailThreadRunning = false;
		ThumbnailNumThreadsRunning--;
	}

	if (ThumbnailThreadRunning)
		return 0;

	// We only ever access ThumbnailPicture once the worker thread is completed,
	// If the worker thread failed, ThumbnailPicture will be invalid and we return 0.
	if (ThumbnailInvalidateRequested)
	{
		ThumbnailRequested = false;
		ThumbnailInvalidateRequested = false;
		ThumbnailPicture.Clear();
		if (TexIDThumbnail != 0)
		{
			glDeleteTextures(1, &TexIDThumbnail);
			TexIDThumbnail = 0;
		}
		return 0;
	}

	if (ThumbnailPicture.IsValid())
	{
		if (TexIDThumbnail != 0)
		{
			glBindTexture(GL_TEXTURE_2D, TexIDThumbnail);
			return TexIDThumbnail;
		}

		glGenTextures(1, &TexIDThumbnail);
		if (TexIDThumbnail == 0)
			return 0;

		tList<tLayer> layers;
		layers.Append
		(
			new tLayer
			(
				tPixelFormat::R8G8B8A8, ThumbnailPicture.GetWidth(), ThumbnailPicture.GetHeight(),
				(uint8*)ThumbnailPicture.GetPixelPointer()
			)
		);

		BindLayers(layers, TexIDThumbnail);
		return TexIDThumbnail;
	}

	return 0;
}


void TacitImage::GenerateThumbnailBridge(TacitImage* tacitImage)
{
	tacitImage->GenerateThumbnail();
}


void TacitImage::GenerateThumbnail()
{
	// This thread (only) is allowed to access ThumbnailPicture. The main thread will leave it alone until GenerateThumbnail is complete.
	if (ThumbnailPicture.IsValid())
		return;

	// Retrieve from cache if possible.
	tuint256 hash = 0;
	int thumbVersion = 1;
	tFileInfo fileInfo;
	tGetFileInfo(fileInfo, Filename);
	hash = tHashData256((uint8*)&thumbVersion, sizeof(thumbVersion));
	hash = tHashString256(Filename, hash);
	hash = tHashData256((uint8*)&fileInfo.FileSize, sizeof(fileInfo.FileSize), hash);
	hash = tHashData256((uint8*)&fileInfo.CreationTime, sizeof(fileInfo.CreationTime), hash);
	hash = tHashData256((uint8*)&fileInfo.ModificationTime, sizeof(fileInfo.ModificationTime), hash);
	hash = tHashData256((uint8*)&ThumbWidth, sizeof(ThumbWidth), hash);
	hash = tHashData256((uint8*)&ThumbHeight, sizeof(ThumbHeight), hash);
	tString hashFile;
	tsPrintf(hashFile, "%s%032|256X.bin", ThumbCacheDir.Chars(), hash);
	if (tFileExists(hashFile))
	{
		tChunkReader chunk(hashFile);
		ThumbnailPicture.Load(chunk.First());
		return;
	}

	// We need an opengl context if we are processing dds files (for now... opengl is used for decompression). GLFW doesn't support creating
	// contexts without an associated window. However, contexts with hidden windows can be created with the GLFW_VISIBLE window hint.
	GLFWwindow* offscreenContext = nullptr;
	if (Filetype == tFileType::DDS)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		offscreenContext = glfwCreateWindow(32, 32, "", nullptr, nullptr);
		if (!offscreenContext)
			return;

		glfwMakeContextCurrent(offscreenContext);
	}

	TacitImage thumbLoader;
	thumbLoader.Load(Filename);

	if (Filetype == tFileType::DDS)
	{
		glfwMakeContextCurrent(nullptr);
		glfwDestroyWindow(offscreenContext);
	}

	// Thumbnails are generated from the primary (first) picture in the picture list.
	tPicture* srcPic = thumbLoader.GetPrimaryPic();
	tAssert(srcPic);

	// We make the thumbnail keep its aspect ratio.
	int srcW = srcPic->GetWidth();
	int srcH = srcPic->GetHeight();
	float scaleX = float(ThumbWidth)  / float(srcW);
	float scaleY = float(ThumbHeight) / float(srcH);
	int iw, ih;
	if (scaleX < scaleY)
	{
		iw = ThumbWidth;
		ih = int(tRound(float(srcH)*scaleX));
	}
	else
	{
		ih = ThumbHeight;
		iw = int(tRound(float(srcW)*scaleY));
	}
	tAssert((iw == ThumbWidth) || (ih == ThumbHeight));

	// Create an image that is big (or small) enough to exactly match either the width or height without ruining the aspect.
	srcPic->Resample(iw, ih, tPicture::tFilter::Bilinear);

	// Center-crop the image to what we need. Cropping to a bigger size adds transparent pixels.
	srcPic->Crop(ThumbWidth, ThumbHeight);

	ThumbnailPicture.Set(*srcPic);

	// Write to cache file.
	tChunkWriter writer(hashFile);
	ThumbnailPicture.Save(writer);
	// std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


void TacitImage::RequestThumbnail()
{
	if (ThumbnailRequested)
		return;

	// Leave two cores free unless we are on a three core or lower machine, in which case we always use a min of 2 threads.
	int numThreadsMax = tClampMin((tSystem::tGetNumCores()) - 2, 2);
	if (ThumbnailNumThreadsRunning >= numThreadsMax)
		return;

	ThumbnailRequested = true;
	ThumbnailThreadRunning = true;
	ThumbnailNumThreadsRunning++;
	ThumbnailThreadFlag.test_and_set();
	ThumbnailThread = std::thread
	(
		[this]
		{
			GenerateThumbnailBridge(this);
			ThumbnailThreadFlag.clear();
		}
	);
}


void TacitImage::UnrequestThumbnail()
{
	if (ThumbnailRequested && !ThumbnailThreadRunning && !ThumbnailPicture.IsValid())
		ThumbnailRequested = false;
}


void TacitImage::RequestInvalidateThumbnail()
{
	if (!ThumbnailRequested)
		return;

	ThumbnailInvalidateRequested = true;
}


void TacitImage::Play()
{
	PartCurrCountdown = PartDurationOverrideEnabled ? PartDurationOverride : GetCurrentPic()->Duration;
	PartPlaying = true;
}


void TacitImage::Stop()
{
	PartPlaying = false;
}


void TacitImage::UpdatePlaying(float dt)
{
	if (!PartPlaying)
		return;

	int numParts = GetNumParts();
	if (numParts <= 1)
		return;

	PartCurrCountdown -= dt;
	if (PartCurrCountdown <= 0.0f)
	{
		if (!PartPlayRev)
		{
			PartNum++;
			if (PartNum >= numParts)
			{
				if (PartPlayLooping)
					PartNum = 0;
				else
				{
					PartNum = numParts-1;
					PartPlaying = false;
				}
			}
		}
		else
		{
			PartNum--;
			if (PartNum <= 0)
			{
				if (PartPlayLooping)
					PartNum = numParts-1;
				else
				{
					PartNum = 0;
					PartPlaying = false;
				}
			}
		}
		if (PartDurationOverrideEnabled)
			PartCurrCountdown = PartDurationOverride;
		else
			PartCurrCountdown = GetCurrentPic()->Duration;
	}
}
