// Image.h
//
// An image class that can load a file from disk into main memory and to VRAM.
//
// Copyright (c) 2019-2022 Tristan Grimmer.
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
#include <Image/tMetaData.h>
#include "Config.h"
#include "Undo.h"
namespace Viewer
{


class Image : public tLink<Image>
{
public:
	Image();

	// These constructors do not actually load the image, but Load() may be called at any point afterwards.
	Image(const tString& filename);
	Image(const tSystem::tFileInfo& fileInfo);
	virtual ~Image();

	// These params are in principle different to the ones in tPicture since a Image does not necessarily
	// only use tPicture to do the loading. For example, we might include dds load params here.
	// WIP. Tidy up the load params. Differnt filetypes have different load-param requirements.
	/////////////////////////		
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

	bool Load(const tString& filename);
	bool Load();													// Load into main memory.
	bool IsLoaded() const																								{ return (Pictures.Count() > 0); }
	int GetNumFrames() const																							{ return Pictures.Count(); }

	bool IsOpaque() const;
	bool Unload(bool force = false);
	float GetLoadedTime() const																							{ return LoadedTime; }

	// Bind to a texture ID and load into VRAM. If already in VRAM, it makes the texture current. Since some ImGui
	// functions require a texture ID as parameter, this function return the ID. If the alt image is enabled, the bound
	// texture and ID will be the alt image's. Returns 0 (invalid id) if there was a problem.
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
	void Crop(const tColouri& borderColour, tcomps channels = tComp_RGBA);
	void Resample(int newWidth, int newHeight, tImage::tResampleFilter filter, tImage::tResampleEdgeMode edgeMode);
	void SetPixelColour(int x, int y, const tColouri&, bool pushUndo, bool supressDirty = false);
	void SetAllPixels(const tColouri& colour, tcomps channels = tComp_RGBA);
	void AlphaBlendColour(const tColouri& blendColour, bool resetAlpha);
	void SetFrameDuration(float duration, bool allFrames = false);

	// Undo and redo functions.
	void Undo()																											{ UndoStack.Undo(Pictures, Dirty); }
	void Redo()																											{ UndoStack.Redo(Pictures, Dirty); }
	bool IsUndoAvailable() const																						{ return UndoStack.UndoAvailable(); }
	bool IsRedoAvailable() const																						{ return UndoStack.RedoAvailable(); }
	tString GetUndoDesc() const																							{ tString desc; tsPrintf(desc, "[%s]", UndoStack.GetUndoDesc().Chr()); return desc; }
	tString GetRedoDesc() const																							{ tString desc; tsPrintf(desc, "[%s]", UndoStack.GetRedoDesc().Chr()); return desc; }

	// Since from outside this class you can save to any filename, we need the ability to clear the dirty flag.
	void ClearDirty()																									{ Dirty = false; }
	bool IsDirty() const																								{ return Dirty; }

	struct ImgInfo
	{
		bool IsValid() const							{ return (SrcPixelFormat != tImage::tPixelFormat::Invalid); }
		tImage::tPixelFormat SrcPixelFormat				= tImage::tPixelFormat::Invalid;
		bool Opaque										= false;
		int FileSizeBytes								= 0;
		int MemSizeBytes								= 0;
	};

	bool IsAltMipmapsPictureAvail() const																				{ return (AltPictureTyp == AltPictureType::MipmapSideBySide); }
	bool IsAltCubemapPictureAvail() const																				{ return (AltPictureTyp == AltPictureType::CubemapTLayout); }
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

	ImgInfo Info;										// Info is only valid AFTER loading.
	tString Filename;									// Valid before load.
	tSystem::tFileType Filetype;						// Valid before load. Based on extension.
	std::time_t FileModTime;							// Valid before load.
	uint64 FileSizeB;									// Valid before load.

	// Members starting with "Cached" are stored in the cache/thumbnail file and are valid
	// once the thumbnail is loaded. Used for sorting without having to do a full load.
	int Cached_PrimaryWidth		= 0;						
	int Cached_PrimaryHeight	= 0;
	int Cached_PrimaryArea		= 0;
	tImage::tMetaData Cached_MetaData;

	const static uint32 ThumbChunkInfoID;
	const static uint32 ThumbChunkMetaDataID;
	const static uint32 ThumbChunkMetaDatumID;

	const static int ThumbWidth;						// = 256;
	const static int ThumbHeight;						// = 144;
	const static int ThumbMinDispWidth;					// = 64;
	static tString ThumbCacheDir;

	bool TypeSupportsProperties() const;

	// Zoom is stored per-image so we can flip between images without losing the setting.
	float ZoomPercent = 100.0f;

private:
	void PushUndo(const tString& desc)																					{ UndoStack.Push(Pictures, desc, Dirty); }

	// If the image is from a DDS file, we keep the DDS around so we have access to the iage data in its original pixel
	// format. This will allow us to do things like cropping without a decode/recode step.
	// @todo For mow we don't support this feature and don't need the DDS to be remembered.
	// tImage::tImageDDS;

	// There are multiple pictures for a few reasons. Images with multiple frames (gifs, exrs, tiffs, webps etc) store
	// the individual frames as separate pictures in the list, dds files may store a cubemap and the 6 sides are stored
	// in the picture list, and dds files may contain mipmaps, also stored in the list.
	// @todo Yes, currently no way 
	// second to store the  
	tList<tImage::tPicture> Pictures;

	// The 'alternative' picture is valid when there is another valid way of displaying the image.
	// Specifically for cubemaps and dds files with mipmaps this offers an alternative view.
	bool AltPictureEnabled = false;
	enum class AltPictureType
	{
		None,				// Alt picture not in use.
		CubemapTLayout,
		MipmapSideBySide
	};
	AltPictureType AltPictureTyp = AltPictureType::None;
	tImage::tPicture AltPicture;

	bool ThumbnailRequested = false;					// True if ever requested.
	bool ThumbnailInvalidateRequested = false;
	bool ThumbnailThreadRunning = false;				// Only true while worker thread going.
	static int ThumbnailNumThreadsRunning;				// How many worker threads active.
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
	void PopulatePicturesDDS(const tImage::tImageDDS&);
	void CreateAltPicturesDDS(const tImage::tImageDDS&);

	void GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tImage::tPixelFormat);
	void BindLayers(const tList<tImage::tLayer>&, uint texID);

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
