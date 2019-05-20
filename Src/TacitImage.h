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

#pragma once
#include <Foundation/tList.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include <Image/tPicture.h>
#include <Image/tTexture.h>


class TacitImage : public tLink<TacitImage>
{
public:
	TacitImage(const tString& filename);

	bool Load();					// Load into main memory.
	bool IsLoaded() const;

	bool Bind();					// Bind to a particulr texture ID and load into VRAM. If image is already in VRAM, it only binds (makes texture current).
	bool IsBound() const;

	int GetWidth() const;
	int GetHeight() const;

	tString Filename;
	tSystem::tFileType Filetype;

	// Only one of these will be valid at a time. This is because dds files are special and already in HW ready format.
	// The tTexture will store any dds files, while tPicture stores other types (tga, gif, jpg, bmp, tif, png, etc).
	tImage::tTexture TextureImage;
	tImage::tPicture PictureImage;

	// Zero is invalid and means texture has never been bound and loaded into VRAM.
	uint GLTextureID;

private:
	void GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tImage::tPixelFormat);
	void BindLayers(const tList<tImage::tLayer>&);
};
