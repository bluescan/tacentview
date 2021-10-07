// Image.h
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

#pragma once
#include <thread>
#include <atomic>
#include <glad/glad.h>
#include <Foundation/tList.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include <Image/tPicture.h>
#include <Image/tTexture.h>
#include <Image/tCubemap.h>
#include <Image/tImageHDR.h>
#include "Settings.h"
#include "Undo.h"
namespace Viewer
{


class Image : public tLink<Image>
{
public:
	Image();

	// This constructor does not actually load the image, but Load() may be called at any point afterwards.
	Image(const tString& filename);
	virtual ~Image();

	// These params are in principle different to the ones in tPicture since a Image does not necessarily
	// only use tPicture to do the loading. For example, we might include dds load params here.
	void ResetLoadParams();
	tImage::tPicture::LoadParams LoadParams;

	void Play();
	void Stop();
	void UpdatePlaying(float dt);
	bool FrameDurationPreviewEnabled	= false;
	float FrameDurationPreview			= 1.0f/30.0f;
	float FrameCurrCountdown			= 0.0f;
	bool FramePlaying					= false;
	bool FramePlayRev					= false;
	bool FramePlayLooping				= true;
	int FrameNum						= 0;

	static void GetCanLoad(tSystem::tExtensions&);					// Clears the extensions ref before populating.
	bool Load(const tString& filename);
	bool Load();													// Load into main memory.
	bool IsLoaded() const																								{ return (Pictures.Count() > 0); }
	int GetNumFrames() const																							{ return Pictures.Count(); }

	bool IsOpaque() const;
	bool Unload(bool force = false);
	float GetLoadedTime() const																							{ return LoadedTime; }

	// Bind to a texture ID and load into VRAM. If already in VRAM, it makes the texture current. Since some ImGui
	// functions require a texture ID as parameter, this function return the ID.
	// If the alt image is enabled, the bound texture and ID  will be the alt image's.
	// Returns 0 (invalid id) if there was a problem.
	uint64 Bind();
	void Unbind();
	int GetWidth() const;
	int GetHeight() const;
	int GetArea() const;
	tColouri GetPixel(int x, int y) const;

	// Some images can store multiple complete images inside a single file (multiple frames).
	// The primary one is the first one.
	tImage::tPicture* GetPrimaryPic() const																				{ return Pictures.First(); }
	tImage::tPicture* GetCurrentPic() const																				{ tImage::tPicture* pic = Pictures.First(); for (int i = 0; i < FrameNum; i++) pic = pic ? pic->Next() : nullptr; return pic; }
	tList<tImage::tPicture>& GetPictures()																				{ return Pictures; }

	// Functions that edit and cause dirty flag to be set.
	void Rotate90(bool antiClockWise);
	void Rotate(float angle, const tColouri& fill, tImage::tResampleFilter upFilter, tImage::tResampleFilter downFilter);
	void Flip(bool horizontal);
	void Crop(int newWidth, int newHeight, int originX, int originY, const tColouri& fillColour = tColour::black);
	void Crop(int newWidth, int newHeight, tImage::tPicture::Anchor, const tColouri& fillColour = tColour::black);
	void Crop(const tColouri& borderColour, uint32 channels = tMath::ColourChannel_RGBA);
	void Resample(int newWidth, int newHeight, tImage::tResampleFilter filter, tImage::tResampleEdgeMode edgeMode);
	void SetPixelColour(int x, int y, const tColouri&, bool pushUndo, bool supressDirty = false);
	void SetFrameDuration(float duration, bool allFrames = false);

	// Undo and redo functions.
	void Undo()																											{ UndoStack.Undo(Pictures, Dirty); }
	void Redo()																											{ UndoStack.Redo(Pictures, Dirty); }
	bool IsUndoAvailable() const																						{ return UndoStack.UndoAvailable(); }
	bool IsRedoAvailable() const																						{ return UndoStack.RedoAvailable(); }
	tString GetUndoDesc() const																							{ tString desc; tsPrintf(desc, "[%s]", UndoStack.GetUndoDesc().Chars()); return desc; }
	tString GetRedoDesc() const																							{ tString desc; tsPrintf(desc, "[%s]", UndoStack.GetRedoDesc().Chars()); return desc; }

	// Since from outside this class you can save to any filename, we need the ability to clear the dirty flag.
	void ClearDirty()																									{ Dirty = false; }
	bool IsDirty() const																								{ return Dirty; }

	struct ImgInfo
	{
		bool IsValid() const				{ return (SrcPixelFormat != tImage::tPixelFormat::Invalid); }
		tImage::tPixelFormat SrcPixelFormat	= tImage::tPixelFormat::Invalid;
		bool Opaque							= false;
		int FileSizeBytes					= 0;
		int MemSizeBytes					= 0;
	};

	bool IsAltMipmapsPictureAvail() const																				{ return DDSTexture2D.IsValid() && AltPicture.IsValid(); }
	bool IsAltCubemapPictureAvail() const																				{ return DDSCubemap.IsValid() && AltPicture.IsValid(); }
	void EnableAltPicture(bool enabled)																					{ AltPictureEnabled = enabled; }
	bool IsAltPictureEnabled() const																					{ return AltPictureEnabled; }

	// Thumbnail generation is done on a seperate thread. Calling RequestThumbnail starts the thread. You should call it
	// over and over as it will only ever start one thread, and it may not start it if too mnay threads are already
	// working. BindThumbnail will at some point return a non-zero texture ID, but not necessarily right away. Just keep
	// calling it. Unloaded images remain unloaded after thumbnail generation.
	void RequestThumbnail();

	// Call this if you need to invaidate the thumbnail. For example, if the file was saved/edited this should be called
	// to force regeneration.
	void RequestInvalidateThumbnail();

	// You are allowed to unrequest. It will succeed if a worker was never assigned.
	void UnrequestThumbnail();
	bool IsThumbnailWorkerActive() const																				{ return ThumbnailThreadRunning; }
	uint64 BindThumbnail();
	inline static int GetThumbnailNumThreadsRunning()																	{ return ThumbnailNumThreadsRunning; }

	ImgInfo Info;						// Info is only valid AFTER loading.
	tString Filename;					// Valid before load.
	tSystem::tFileType Filetype;		// Valid before load. Based on extension.
	std::time_t FileModTime;			// Valid before load.
	uint64 FileSizeB;					// Valid before load.
	int CachePrimaryWidth	= 0;		// Valid once thumbnail loaded. Used for sorting without having to do full load.
	int CachePrimaryHeight	= 0;
	int CachePrimaryArea	= 0;

	const static uint32 ThumbChunkInfoID;
	const static int ThumbWidth;		// = 256;
	const static int ThumbHeight;		// = 144;
	const static int ThumbMinDispWidth;	// = 64;
	static tString ThumbCacheDir;

	bool TypeSupportsProperties() const;

private:
	void PushUndo(const tString& desc)																					{ UndoStack.Push(Pictures, desc, Dirty); }

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
	bool ThumbnailInvalidateRequested = false;
	bool ThumbnailThreadRunning = false;		// Only true while worker thread going.
	static int ThumbnailNumThreadsRunning;		// How many worker threads active.
	std::thread ThumbnailThread;
	std::atomic_flag ThumbnailThreadFlag = ATOMIC_FLAG_INIT;
	tImage::tPicture ThumbnailPicture;

	// These 2 functions run on a helper thread.
	static void GenerateThumbnailBridge(Image*);
	void GenerateThumbnail();

	// Zero is invalid and means texture has never been bound and loaded into VRAM.
	uint TexIDAlt			= 0;
	uint TexIDThumbnail		= 0;

	// Returns the approx main mem size of this image. Considers the Pictures list and the AltPicture.
	int GetMemSizeBytes() const;
	bool ConvertTexture2DToPicture();
	bool ConvertCubemapToPicture();
	void GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tImage::tPixelFormat);
	void BindLayers(const tList<tImage::tLayer>&, uint texID);
	void CreateAltPictureFromDDS_2DMipmaps();
	void CreateAltPictureFromDDS_Cubemap();

	float LoadedTime = -1.0f;
	bool Dirty = false;

	// Undo / Redo
	Undo::Stack UndoStack;
};


// Implementation below.


inline bool Image::TypeSupportsProperties() const
{
	return
	(
		(Filetype == tSystem::tFileType::HDR) ||
		(Filetype == tSystem::tFileType::EXR)
	);
}


}
