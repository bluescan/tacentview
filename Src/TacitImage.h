// TacitImage.h
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

#pragma once
#include <thread>
#include <atomic>
#include <GL/glew.h>
#include <Foundation/tList.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include <Image/tPicture.h>
#include <Image/tTexture.h>
#include <Image/tCubemap.h>


class TacitImage : public tLink<TacitImage>
{
public:
	TacitImage();
	TacitImage(const tString& filename);
	virtual ~TacitImage();

	bool Load();						// Load into main memory.
	bool Load(const tString& filename);
	bool IsLoaded() const																								{ return (Pictures.Count() > 0); }

	bool IsOpaque() const;
	bool Unload();
	float GetLoadedTime() const																							{ return LoadedTime; }

	// Bind to a texture ID and load into VRAM. If already in VRAM, it makes the texture current. Since some ImGui
	// functions require a texture ID as parameter, this function return the ID.
	// If the alt image is enabled, the bound texture and ID  will be the alt image's.
	// Returns 0 (invalid id) if there was a problem.
	uint64 Bind();
	void Unbind();
	int GetWidth() const;
	int GetHeight() const;
	tColouri GetPixel(int x, int y) const;

	void Rotate90(bool antiClockWise);
	void Flip(bool horizontal);

	struct ImgInfo
	{
		bool IsValid() const	{ return (Width > 0) && (Height > 0); }
		int Width				= 0;
		int Height				= 0;
		tString PixelFormat;
		int SrcFileBitDepth		= -1;
		bool Opaque				= false;
		int FileSizeBytes		= 0;
		int MemSizeBytes		= 0;
		int Mipmaps				= 0;
	};
	void PrintInfo();
	tImage::tPicture* GetPrimaryPicture()																				{ return Pictures.First(); }

	bool IsAltMipmapsPictureAvail() const																				{ return DDSTexture2D.IsValid() && AltPicture.IsValid(); }
	bool IsAltCubemapPictureAvail() const																				{ return DDSCubemap.IsValid() && AltPicture.IsValid(); }
	void EnableAltPicture(bool enabled)																					{ AltPictureEnabled = enabled; }
	bool IsAltPictureEnabled() const																					{ return AltPictureEnabled; }

	// Thumbnail generation is done on a seperate thread. Calling RequestThumbnail starts the thread. You should call it
	// over and over as it will only ever start one thread, and it may not start it if too mnay threads are already
	// working. BindThumbnail will at some point return a non-zero texture ID, but not necessarily right away. Just keep
	// calling it. Unloaded images remain unloaded after thumbnail generation.
	void RequestThumbnail();

	// You are allowed to unrequest. It will succeed if a worker was never assigned.
	void UnrequestThumbnail();
	bool IsThumbnailWorkerActive() const { return ThumbnailThreadRunning; }
	uint64 BindThumbnail();

	ImgInfo Info;						// Info is only valid AFTER loading.
	tString Filename;					// Valid before load.
	tSystem::tFileType Filetype;		// Valid before load.
	uint64 FileModTime;					// Valid before load.
	uint64 FileSizeB;					// Valid before load.

	const static int ThumbWidth			= 256;
	const static int ThumbHeight		= 144;
	const static int ThumbMinDispWidth	= 64;
	static tString ThumbCacheDir;

private:
	// Dds files are special and already in HW ready format. The tTexture can store dds files, while tPicture stores
	// other types (tga, gif, jpg, bmp, tif, png, etc). If the image is a dds file, the tTexture is valid and in order
	// to read pixel data, the image is fetched from the framebuffer to ALSO make a valid PictureImage.
	//
	// Note: A tTexture contains all mipmap levels while a tPicture does not. That's why we have a list of tPictures.
	tImage::tTexture DDSTexture2D;
	tImage::tCubemap DDSCubemap;

	tList<tImage::tPicture> Pictures;

	// The 'alternative' picture is valid when there is another valid way of displaying the image.
	// Specifically for cubemaps and dds files with mipmaps this offers an alternative view.
	bool AltPictureEnabled = false;
	tImage::tPicture AltPicture;

	bool ThumbnailRequested = false;			// True if ever requested.
	bool ThumbnailThreadRunning = false;		// Only true while worker thread going.
	static int ThumbnailNumThreadsRunning;		// How many worker threads active.
	std::thread ThumbnailThread;
	std::atomic_flag ThumbnailThreadFlag = ATOMIC_FLAG_INIT;
	tImage::tPicture ThumbnailPicture;

	// These 2 functions run on a helper thread.
	static void GenerateThumbnailBridge(TacitImage* tacitImage);
	void GenerateThumbnail();

	// Zero is invalid and means texture has never been bound and loaded into VRAM.
	uint TexIDPrimary	= 0;
	uint TexIDAlt		= 0;
	uint TexIDThumbnail	= 0;

	// Returns the approx main mem size of this image. Considers the Pictures list and the AltPicture.
	int GetMemSizeBytes() const;
	bool ConvertTexture2DToPicture();
	bool ConvertCubemapToPicture();
	void GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tImage::tPixelFormat);
	void BindLayers(const tList<tImage::tLayer>&, uint texID);
	void CreateAltPictureDDS2DMipmaps();
	void CreateAltPictureDDSCubemap();

	float LoadedTime = -1.0f;
};
