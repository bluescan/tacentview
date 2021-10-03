// Image.cpp
//
// An image class that can load a file from disk into main memory and to VRAM.
//
// Copyright (c) 2019, 2020, 2021 Tristan Grimmer.
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
#include <glad/glad.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#include <Foundation/tHash.h>
#include <Foundation/tFundamentals.h>
#include <Image/tTexture.h>
#include <System/tFile.h>
#include <System/tTime.h>
#include <System/tMachine.h>
#include <System/tChunk.h>
#include "Image.h"
#include "Settings.h"
using namespace tStd;
using namespace tSystem;
using namespace tImage;
using namespace tMath;
using namespace Viewer;
int Image::ThumbnailNumThreadsRunning = 0;
tString Image::ThumbCacheDir;
namespace Viewer { extern Settings Config; }


const uint32 Image::ThumbChunkInfoID	= 0x0B000000;
const int Image::ThumbWidth				= 256;
const int Image::ThumbHeight			= 144;
const int Image::ThumbMinDispWidth		= 64;


Image::Image() :
	Filename(),
	Filetype(tFileType::Unknown),
	FileSizeB(0),
	LoadParams()
{
	tMemset(&FileModTime, 0, sizeof(FileModTime));
	ResetLoadParams();
}


Image::Image(const tString& filename) :
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


Image::~Image()
{
	// If we're being destroyed before the thumbnail thread is done, we have to wait because that thread
	// accesses the thumbnail picture of this object... so 'this' must be valid.
	if (ThumbnailThread.joinable())
		ThumbnailThread.join();

	// It is important that the thread count decrements if necessary since Images can be deleted
	// when changing folders. The threads need to be available to do more work in a new folder.
	if (ThumbnailRequested && ThumbnailThreadRunning)
	{
		ThumbnailNumThreadsRunning--;
		tiClampMin(ThumbnailNumThreadsRunning, 0);
	}

	// Free GPU image mem and texture IDs.
	Unload(true);
}


void Image::ResetLoadParams()
{
	LoadParams = tImage::tPicture::LoadParams();
	LoadParams.GammaValue = Viewer::Config.MonitorGamma;
}


bool Image::Load(const tString& filename)
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


void Image::GetCanLoad(tSystem::tExtensions& extensions)
{
	extensions.Clear();
	tGetExtensions(extensions, tSystem::tFileType::APNG);
	tGetExtensions(extensions, tSystem::tFileType::BMP);
	tGetExtensions(extensions, tSystem::tFileType::DDS);
	tGetExtensions(extensions, tSystem::tFileType::EXR);
	tGetExtensions(extensions, tSystem::tFileType::GIF);
	tGetExtensions(extensions, tSystem::tFileType::HDR);
	tGetExtensions(extensions, tSystem::tFileType::ICO);
	tGetExtensions(extensions, tSystem::tFileType::JPG);
	tGetExtensions(extensions, tSystem::tFileType::PNG);
	tGetExtensions(extensions, tSystem::tFileType::TGA);
	tGetExtensions(extensions, tSystem::tFileType::TIFF);
	tGetExtensions(extensions, tSystem::tFileType::WEBP);
}


bool Image::Load()
{
	if (IsLoaded() && !Dirty)
	{
		LoadedTime = tSystem::tGetTime();
		return true;
	}

	if (Filetype == tFileType::Unknown)
		return false;

	// If the type is a png file, we may actually be dealing with an apng file inside.
	// It is more efficient to only use the apng loader if we need to (even though it will
	// handle non apng files). To this end, we modify the filetype used for loading if necessary.
	// Note also that from the outside, we only determine the type from the extension. The 
	// IsAnimatedPNG call is quite expensive -- it needs to load part of the fileinto memory,
	// not something we want to do before actually loading an image. If DetectAPNGInsidePNG is
	// false, the PNG loader will always be used for .png files even if they have an apng inside.
	// The designers of apng made the format backwards compatible with single-frame png loaders.
	tSystem::tFileType loadingFiletype = Filetype;
	if ((Filetype == tSystem::tFileType::PNG) && Config.DetectAPNGInsidePNG && tImageAPNG::IsAnimatedPNG(Filename))
		loadingFiletype = tSystem::tFileType::APNG;

	Info.SrcPixelFormat = tPixelFormat::Invalid;
	bool success = false;
	try
	{
		switch (loadingFiletype)
		{
			case tSystem::tFileType::APNG:
			{
				tImageAPNG apng;
				bool ok = apng.Load(Filename);
				if (!ok)
					return false;

				int numFrames = apng.GetNumFrames();
				for (int f = 0; f < numFrames; f++)
				{
					tFrame* frame = apng.StealFrame(0);
					tPicture* picture = new tPicture();

					picture->Set(frame->Width, frame->Height, frame->StealPixels(), false);
					picture->Duration = frame->Duration;

					// Since the pixels were stolen, the frame destructor will not delete them.
					delete frame;
					Pictures.Append(picture);
				}
				Info.SrcPixelFormat = apng.SrcPixelFormat;
				success = true;
				break;
			}

			case tSystem::tFileType::BMP:
			{
				tImageBMP bmp;
				bool ok = bmp.Load(Filename);
				if (!ok)
					return false;

				int width = bmp.GetWidth();
				int height = bmp.GetHeight();
				tPixel* pixels = bmp.StealPixels();

				Info.SrcPixelFormat = bmp.SrcPixelFormat;
				tPicture* picture = new tPicture(width, height, pixels, false);
				Pictures.Append(picture);
				success = true;
				break;
			}

			case tSystem::tFileType::DDS:
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
				if (success)
				{
					if (DDSCubemap.IsValid())
					{
						ConvertCubemapToPicture();
						CreateAltPictureFromDDS_Cubemap();			// Create cubemap alt image.
					}
					else if (DDSTexture2D.IsValid())
					{
						ConvertTexture2DToPicture();
						if (DDSTexture2D.GetNumMipmaps() > 1)
							CreateAltPictureFromDDS_2DMipmaps();	// Create mipmap alt image.
					}
				}
				break;
			}

			case tSystem::tFileType::EXR:
			{
				tImageEXR exr;
				bool ok = exr.Load
				(
					Filename,
					LoadParams.GammaValue,
					LoadParams.EXR_Exposure,
					LoadParams.EXR_Defog,
					LoadParams.EXR_KneeLow,
					LoadParams.EXR_KneeHigh
				);
				if (!ok)
					return false;

				int numFrames = exr.GetNumFrames();
				for (int f = 0; f < numFrames; f++)
				{
					tFrame* frame = exr.StealFrame(0);
					tPicture* picture = new tPicture();

					picture->Set(frame->Width, frame->Height, frame->StealPixels(), false);
					picture->Duration = frame->Duration;

					// Since the pixels were stolen, the frame destructor will not delete them.
					delete frame;
					Pictures.Append(picture);
				}
				Info.SrcPixelFormat = exr.SrcPixelFormat;
				success = true;
				break;
			}
	
			case tSystem::tFileType::GIF:
			{
				tImageGIF gif;
				bool ok = gif.Load(Filename);
				if (!ok)
					return false;

				int numFrames = gif.GetNumFrames();
				for (int f = 0; f < numFrames; f++)
				{
					tFrame* frame = gif.StealFrame(0);
					tPicture* picture = new tPicture();
					
					picture->Set(gif.GetWidth(), gif.GetHeight(), frame->StealPixels(), false);
					picture->Duration = frame->Duration;

					// Since the pixels were stolen, the frame destructor will not delete them.
					delete frame;
					Pictures.Append(picture);
				}
				Info.SrcPixelFormat = gif.SrcPixelFormat;
				success = true;
				break;
			}

			case tSystem::tFileType::HDR:
			{
				tImageHDR hdr;
				bool ok = hdr.Load(Filename, LoadParams.GammaValue, LoadParams.HDR_Exposure);
				if (!ok)
					return false;

				int width = hdr.GetWidth();
				int height = hdr.GetHeight();
				tPixel* pixels = hdr.StealPixels();

				Info.SrcPixelFormat = hdr.SrcPixelFormat;
				tPicture* picture = new tPicture(width, height, pixels, false);
				Pictures.Append(picture);
				success = true;
				break;
			}

			case tSystem::tFileType::ICO:
			{
				tImageICO ico;
				bool ok = ico.Load(Filename);
				if (!ok)
					return false;

				Info.SrcPixelFormat = ico.GetBestSrcPixelFormat();
				int numFrames = ico.GetNumFrames();
				for (int p = 0; p < numFrames; p++)
				{
					tFrame* frame = ico.StealFrame(0);
					tAssert(frame);
					tPicture* picture = new tPicture();
					picture->Set(frame->Width, frame->Height, frame->StealPixels(), false);

					// Since the pixels were stolen, the frame destructor will not delete them.
					delete frame;
					Pictures.Append(picture);
				}
				success = true;
				break;
			}

			case tSystem::tFileType::JPG:
			{
				tImageJPG jpg;
				bool ok = jpg.Load(Filename, Viewer::Config.StrictLoading);
				if (!ok)
					return false;

				int width = jpg.GetWidth();
				int height = jpg.GetHeight();
				tPixel* pixels = jpg.StealPixels();

				Info.SrcPixelFormat = jpg.SrcPixelFormat;
				tPicture* picture = new tPicture(width, height, pixels, false);
				Pictures.Append(picture);
				success = true;
				break;
			}

			case tSystem::tFileType::PNG:
			{
				tImagePNG png;
				bool ok = png.Load(Filename);
				if (!ok)
					return false;

				int width = png.GetWidth();
				int height = png.GetHeight();
				tPixel* pixels = png.StealPixels();

				Info.SrcPixelFormat = png.SrcPixelFormat;
				tPicture* picture = new tPicture(width, height, pixels, false);
				Pictures.Append(picture);
				success = true;
				break;
			}

			case tSystem::tFileType::TGA:
			{
				tImageTGA tga;
				bool ok = tga.Load(Filename);
				if (!ok)
					return false;

				int width = tga.GetWidth();
				int height = tga.GetHeight();
				tPixel* pixels = tga.StealPixels();

				Info.SrcPixelFormat = tga.SrcPixelFormat;
				tPicture* picture = new tPicture(width, height, pixels, false);
				Pictures.Append(picture);
				success = true;
				break;
			}

			case tSystem::tFileType::TIFF:
			{
				tImageTIFF tiff;
				bool ok = tiff.Load(Filename);
				if (!ok)
					return false;

				int numFrames = tiff.GetNumFrames();
				for (int f = 0; f < numFrames; f++)
				{
					tFrame* frame = tiff.StealFrame(0);
					tPicture* picture = new tPicture();
					picture->Set(frame->Width, frame->Height, frame->StealPixels(), false);
					picture->Duration = frame->Duration;

					// Since the pixels were stolen, the frame destructor will not delete them.
					delete frame;
					Pictures.Append(picture);
				}
				Info.SrcPixelFormat = tiff.SrcPixelFormat;
				success = true;
				break;
			}
	
			case tSystem::tFileType::WEBP:
			{
				tImageWEBP webp;
				bool ok = webp.Load(Filename);
				if (!ok)
					return false;

				int numFrames = webp.GetNumFrames();
				for (int f = 0; f < numFrames; f++)
				{
					tFrame* frame = webp.StealFrame(0);
					tPicture* picture = new tPicture();

					picture->Set(frame->Width, frame->Height, frame->StealPixels(), false);
					picture->Duration = frame->Duration;

					// Since the pixels were stolen, the frame destructor will not delete them.
					delete frame;
					Pictures.Append(picture);
				}
				Info.SrcPixelFormat = webp.SrcPixelFormat;
				success = true;
				break;
			}

			// I don't believe we'll get to this catchall. Everything should be handled above.
			default:
			{
				// Some image files may store multiple images (aka frames or parts) in one file.
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
				break;
			}
		}
	}
	catch (tError error)
	{
		success = false;
	}

	if (!success)
		return false;

	LoadedTime = tSystem::tGetTime();

	// Fill in rest of info struct.
	Info.Opaque				= IsOpaque();
	Info.FileSizeBytes		= tSystem::tGetFileSize(Filename);
	Info.MemSizeBytes		= GetMemSizeBytes();
	ClearDirty();
	return true;
}


int Image::GetMemSizeBytes() const
{
	int numBytes = 0;
	for (tPicture* pic = Pictures.First(); pic; pic = pic->Next())
		numBytes += pic->GetNumPixels() * sizeof(tPixel);

	numBytes += AltPicture.IsValid() ? AltPicture.GetNumPixels()*sizeof(tPixel) : 0;
	return numBytes;
}


void Image::CreateAltPictureFromDDS_2DMipmaps()
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


void Image::CreateAltPictureFromDDS_Cubemap()
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


bool Image::Unload(bool force)
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


void Image::Unbind()
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


bool Image::IsOpaque() const
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


int Image::GetWidth() const
{
	if (AltPicture.IsValid() && AltPictureEnabled)
		return AltPicture.GetWidth();

	tPicture* picture = GetCurrentPic();
	if (picture && picture->IsValid())
		return picture->GetWidth();

	return 0;
}


int Image::GetHeight() const
{
	if (AltPicture.IsValid() && AltPictureEnabled)
		return AltPicture.GetHeight();

	tPicture* picture = GetCurrentPic();
	if (picture && picture->IsValid())
		return picture->GetHeight();

	return 0;
}


int Image::GetArea() const
{
	if (AltPicture.IsValid() && AltPictureEnabled)
		return AltPicture.GetArea();

	tPicture* picture = GetCurrentPic();
	if (picture && picture->IsValid())
		return picture->GetArea();

	return 0;
}


tColouri Image::GetPixel(int x, int y) const
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


void Image::Rotate90(bool antiClockWise)
{
	tString desc; tsPrintf(desc, "Rotate 90 %s", antiClockWise ? "ACW" : "CW");
	PushUndo(desc);
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Rotate90(antiClockWise);

	Dirty = true;
}


void Image::Rotate(float angle, const tColouri& fill, tResampleFilter upFilter, tResampleFilter downFilter)
{
	tString desc; tsPrintf(desc, "Rotate %.1f", tRadToDeg(angle));
	PushUndo(desc);
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->RotateCenter(angle, fill, upFilter, downFilter);

	Dirty = true;
}


void Image::Flip(bool horizontal)
{
	tString desc; tsPrintf(desc, "Flip %s", horizontal ? "Horiz" : "Vert");
	PushUndo(desc);
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Flip(horizontal);

	Dirty = true;
}


void Image::Crop(int newWidth, int newHeight, int originX, int originY, const tColouri& fillColour)
{
	tString desc; tsPrintf(desc, "Crop %d %d", newWidth, newHeight);
	PushUndo(desc);
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Crop(newWidth, newHeight, originX, originY, fillColour);

	Dirty = true;
}


void Image::Crop(int newWidth, int newHeight, tPicture::Anchor anchor, const tColouri& fillColour)
{
	tString desc; tsPrintf(desc, "Crop %d %d", newWidth, newHeight);
	PushUndo(desc);
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Crop(newWidth, newHeight, anchor, fillColour);

	Dirty = true;
}


void Image::Crop(const tColouri& borderColour, uint32 channels)
{
	PushUndo("Crop Borders");
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Crop(borderColour, channels);

	Dirty = true;
}


void Image::Resample(int newWidth, int newHeight, tImage::tResampleFilter filter, tImage::tResampleEdgeMode edgeMode)
{
	tString desc; tsPrintf(desc, "Resample %d %d", newWidth, newHeight);
	PushUndo(desc);
	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
		picture->Resample(newWidth, newHeight, filter, edgeMode);

	Dirty = true;
}


void Image::SetPixelColour(int x, int y, const tColouri& colour, bool pushUndo, bool surpressDirty)
{
	if (pushUndo)
	{
		tString desc; tsPrintf(desc, "Pixel Colour (%d,%d)", x, y);
		PushUndo(desc);
	}

	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
	{
		if ((x > 0) && (x < picture->GetWidth()) && (y > 0) && (y < picture->GetHeight()))
			picture->SetPixel(x, y, colour);
	}

	if (!surpressDirty)
		Dirty = true;
}


void Image::SetFrameDuration(float duration, bool allFrames)
{
	tString desc; tsPrintf(desc, "Frame Dur %.3f", duration);
	PushUndo(desc);

	if (allFrames)
	{
		for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
			picture->Duration = duration;
	}
	else
	{
		tPicture* pic = GetCurrentPic();
		if (pic)
			pic->Duration = duration;
	}

	Dirty = true;
}


uint64 Image::Bind()
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
		AltPicture.GenerateLayers(layers, tResampleFilter(Config.MipmapFilter), tResampleEdgeMode::Clamp, Config.MipmapChaining);
		BindLayers(layers, TexIDAlt);
		return TexIDAlt;
	}

	tPicture* currPic = GetCurrentPic();
	if (currPic && (currPic->TextureID != 0))
	{
		glBindTexture(GL_TEXTURE_2D, currPic->TextureID);
		return currPic->TextureID;
	}

	if (!IsLoaded())
		return 0;

	for (tPicture* picture = Pictures.First(); picture; picture = picture->Next())
	{
		if (!picture->IsValid())
			continue;

		glGenTextures(1, &picture->TextureID);

		tList<tLayer> layers;
		picture->GenerateLayers(layers, tResampleFilter(Config.MipmapFilter), tResampleEdgeMode::Clamp, Config.MipmapChaining);
		BindLayers(layers, picture->TextureID);
	}
	return GetCurrentPic()->TextureID;
}


void Image::BindLayers(const tList<tLayer>& layers, uint texID)
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


void Image::GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tPixelFormat pixelFormat)
{
	srcFormat = GL_RGBA;
	srcType = GL_UNSIGNED_BYTE;

	// Note that the destination format only specifies the resolution of each colour component,
	// not the order or the specifics... they're up to the OpenGL driver. For example, nVidia cards
	// use an internal BGRA format when GL_RGBA8 is specified... that's why having srcFormat = GL_BGRA
	// and dstFormat = RGBA8 would be very efficient (no swizzling).
	dstFormat = GL_RGBA8;
	compressed = false;

	tAssert(GL_EXT_texture_compression_s3tc);
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


bool Image::ConvertTexture2DToPicture()
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


bool Image::ConvertCubemapToPicture()
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
		BindLayers(tex->GetLayers(), tempTexID);

		uint8* rgbaData = new uint8[w * h * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
		Pictures.Append(new tPicture(w, h, (tPixel*)rgbaData, false));

		glDeleteTextures(1, &tempTexID);
	}
	return true;
}


uint64 Image::BindThumbnail()
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
		ThumbnailPicture.GenerateLayers(layers, tResampleFilter(Config.MipmapFilter), tResampleEdgeMode::Clamp, Config.MipmapChaining);
		BindLayers(layers, TexIDThumbnail);
		return TexIDThumbnail;
	}

	return 0;
}


void Image::GenerateThumbnailBridge(Image* img)
{
	img->GenerateThumbnail();
}


void Image::GenerateThumbnail()
{
	// This thread (only) is allowed to access ThumbnailPicture. The main thread will leave it alone until GenerateThumbnail is complete.
	if (ThumbnailPicture.IsValid())
		return;

	// Retrieve from cache if possible.
	tuint256 hash = 0;
	int thumbVersion = 2;
	tFileInfo fileInfo;
	tGetFileInfo(fileInfo, Filename);
	hash = tHash::tHashData256((uint8*)&thumbVersion, sizeof(thumbVersion));
	hash = tHash::tHashString256(Filename, hash);
	hash = tHash::tHashData256((uint8*)&fileInfo.FileSize, sizeof(fileInfo.FileSize), hash);
	hash = tHash::tHashData256((uint8*)&fileInfo.CreationTime, sizeof(fileInfo.CreationTime), hash);
	hash = tHash::tHashData256((uint8*)&fileInfo.ModificationTime, sizeof(fileInfo.ModificationTime), hash);
	hash = tHash::tHashData256((uint8*)&ThumbWidth, sizeof(ThumbWidth), hash);
	hash = tHash::tHashData256((uint8*)&ThumbHeight, sizeof(ThumbHeight), hash);
	tString hashFile;
	tsPrintf(hashFile, "%s%032|256X.bin", ThumbCacheDir.Chars(), hash);
	if (tFileExists(hashFile))
	{
		bool loaded = false;
		tChunkReader chunk(hashFile);
		for (tChunk ch = chunk.First(); ch.IsValid(); ch = ch.Next())
		{
			switch (ch.ID())
			{
				case ThumbChunkInfoID:
					ch.GetItem(CachePrimaryWidth);
					ch.GetItem(CachePrimaryHeight);
					ch.GetItem(CachePrimaryArea);
					break;

				case tChunkID::Image_Picture:
					ThumbnailPicture.Load(ch);
					loaded = true;
					break;
			}
		}
		if (loaded)
			return;
	}

	// We need an opengl context if we are processing dds files (for now... opengl is used for decompression). GLFW doesn't support creating
	// contexts without an associated window. However, contexts with hidden windows can be created with the GLFW_VISIBLE window hint.
	GLFWwindow* offscreenContext = nullptr;
	if (Filetype == tFileType::DDS)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		offscreenContext = glfwCreateWindow(32, 32, "placeholdertitle", nullptr, nullptr);
		if (!offscreenContext)
			return;

		glfwMakeContextCurrent(offscreenContext);
	}

	Image thumbLoader;
	int maxLoadAttempts = 5;
	for (int attempt = 0; attempt < maxLoadAttempts; attempt++)
	{
		bool thumbLoaded = thumbLoader.Load(Filename);
		if (thumbLoaded)
		{
			if (attempt > 0)
				tPrintf("Loading of thumbnail %s succeeded on attempt %d.\n", Filename.Chars(), attempt+1);
			break;
		}
		else
		{
			tPrintf("Warning: Loading of thumbnail %s failed on attempt %d.\n", Filename.Chars(), attempt+1);
			tSystem::tSleep(250);
		}	
	}

	if (Filetype == tFileType::DDS)
	{
		glfwMakeContextCurrent(nullptr);
		glfwDestroyWindow(offscreenContext);
	}

	// Thumbnails are generated from the primary (first) picture in the picture list.
	tPicture* srcPic = thumbLoader.GetPrimaryPic();
	if (!srcPic)
	{
		tPrintf("Warning: Generation of thumbnail %s failed.\n", Filename.Chars());
		return;
	}

	// We make the thumbnail keep its aspect ratio.
	int srcW = srcPic->GetWidth();
	int srcH = srcPic->GetHeight();
	CachePrimaryWidth = srcW;
	CachePrimaryHeight = srcH;
	CachePrimaryArea = srcW * srcH;

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
	srcPic->Resample(iw, ih, tResampleFilter::Bilinear);

	// Center-crop the image to what we need. Cropping to a bigger size adds transparent pixels.
	srcPic->Crop(ThumbWidth, ThumbHeight);

	ThumbnailPicture.Set(*srcPic);

	// Write to cache file.
	tChunkWriter writer(hashFile);
	writer.Begin(ThumbChunkInfoID);
	writer.Write(CachePrimaryWidth);
	writer.Write(CachePrimaryHeight);
	writer.Write(CachePrimaryArea);
	writer.Write(0x00000000);
	writer.End();
	ThumbnailPicture.Save(writer);
	// std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


void Image::RequestThumbnail()
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


void Image::UnrequestThumbnail()
{
	if (ThumbnailRequested && !ThumbnailThreadRunning && !ThumbnailPicture.IsValid())
		ThumbnailRequested = false;
}


void Image::RequestInvalidateThumbnail()
{
	if (!ThumbnailRequested)
		return;

	ThumbnailInvalidateRequested = true;
}


void Image::Play()
{
	FrameCurrCountdown = FrameDurationPreviewEnabled ? FrameDurationPreview : GetCurrentPic()->Duration;
	FramePlaying = true;
}


void Image::Stop()
{
	FramePlaying = false;
}


void Image::UpdatePlaying(float dt)
{
	if (!FramePlaying)
		return;

	int numFrames = GetNumFrames();
	if (numFrames <= 1)
		return;

	FrameCurrCountdown -= dt;
	if (FrameCurrCountdown <= 0.0f)
	{
		if (!FramePlayRev)
		{
			FrameNum++;
			if (FrameNum >= numFrames)
			{
				if (FramePlayLooping)
					FrameNum = 0;
				else
				{
					FrameNum = numFrames-1;
					FramePlaying = false;
				}
			}
		}
		else
		{
			FrameNum--;
			if (FrameNum <= 0)
			{
				if (FramePlayLooping)
					FrameNum = numFrames-1;
				else
				{
					FrameNum = 0;
					FramePlaying = false;
				}
			}
		}
		if (FrameDurationPreviewEnabled)
			FrameCurrCountdown = FrameDurationPreview;
		else
			FrameCurrCountdown = GetCurrentPic()->Duration;
	}
}
