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
	TacitImage();
	TacitImage(const tString& filename);

	bool Load();						// Load into main memory.
	bool Load(const tString& filename);
	bool IsLoaded() const;

	bool Bind();						// Bind to a particulr texture ID and load into VRAM. If image is already in
										// VRAM, it only binds (makes texture current).
	int GetWidth() const;
	int GetHeight() const;
	tColouri GetPixel(int x, int y) const;

	void PrintInfo();

	tString Filename;
	tSystem::tFileType Filetype;

	// Dds files are special and already in HW ready format. The tTexture will store any dds files, while tPicture stores
	// other types (tga, gif, jpg, bmp, tif, png, etc). If the image is a dds file, the tTexture is valid and in order to
	// read pixel data, the image is fetched from the framebuffer to ALSO make a valid PictureImage.
	tImage::tTexture TextureImage;
	tImage::tPicture PictureImage;

	// Zero is invalid and means texture has never been bound and loaded into VRAM.
	uint GLTextureID;

private:
	void GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tImage::tPixelFormat);
	void BindLayers(const tList<tImage::tLayer>&);

	bool ConvertTextureToPicture();
};
