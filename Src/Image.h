// Image.h
//
// An image class that can load a file from disk into main memory and to VRAM.
//
// Copyright (c) 2019-2024 Tristan Grimmer.
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
#include <Image/tMetaData.h>

// For save params.
#include <Image/tImageAPNG.h>
#include <Image/tImageBMP.h>
#include <Image/tImageGIF.h>
#include <Image/tImageICO.h>
#include <Image/tImageJPG.h>
#include <Image/tImagePNG.h>
#include <Image/tImageQOI.h>
#include <Image/tImageTGA.h>
#include <Image/tImageTIFF.h>
#include <Image/tImageWEBP.h>
#include <Image/tImageASTC.h>
#include <Image/tImagePKM.h>
#include <Image/tImageDDS.h>
#include <Image/tImagePVR.h>
#include <Image/tImageEXR.h>
#include <Image/tImageHDR.h>
#include <Image/tImageKTX.h>
#include "Config.h"
#include "Undo.h"
namespace tImage { class tLayer; }
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

	void ResetLoadParams();
	tImage::tImageASTC::LoadParams LoadParams_ASTC;
	tImage::tImageDDS::LoadParams  LoadParams_DDS;
	tImage::tImagePVR::LoadParams  LoadParams_PVR;
	tImage::tImageEXR::LoadParams  LoadParams_EXR;
	tImage::tImageHDR::LoadParams  LoadParams_HDR;
	tImage::tImageTGA::LoadParams  LoadParams_TGA;
	tImage::tImageJPG::LoadParams  LoadParams_JPG;
	tImage::tImageKTX::LoadParams  LoadParams_KTX;
	tImage::tImagePKM::LoadParams  LoadParams_PKM;
	tImage::tImagePNG::LoadParams  LoadParams_PNG;
	bool LoadParams_DetectAPNGInsidePNG = false;

	void RegenerateShuffleValue();
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

	bool Load(const tString& filename, bool loadParamsFromConfig = true);
	bool Load(bool loadParamsFromConfig = true);																		// Load into main memory.
	bool IsLoaded() const																								{ return (Pictures.Count() > 0); }

	// These are structs used for specifying parameters when saving. Different image types support different
	// features and therefore each needs a unique set of parameters. When calling Save you can optionally ask for these
	// structures to be used to grab the parameters from. If they are not used, then the settings in the config
	// file are used.
	tImage::tImageAPNG::SaveParams SaveParamsAPNG;
	tImage::tImageBMP::SaveParams  SaveParamsBMP;
	tImage::tImageGIF::SaveParams  SaveParamsGIF;
	tImage::tImageJPG::SaveParams  SaveParamsJPG;
	tImage::tImagePNG::SaveParams  SaveParamsPNG;
	tImage::tImageQOI::SaveParams  SaveParamsQOI;
	tImage::tImageTGA::SaveParams  SaveParamsTGA;
	tImage::tImageTIFF::SaveParams SaveParamsTIFF;
	tImage::tImageWEBP::SaveParams SaveParamsWEBP;

	// Not all fileTypes are supported for save. Handles single and multi-frame images. If useConfigSaveParams is true
	// any paramteres used for saving that are stored in the viewer config file will override the setting in the save
	// param structures above. Parameters not in the config will use whatever is in the structs. If useConfigSaveParams
	// is false, the parameter structs above are used exclusively. Is onlyCurrentPic is true, only the single frame
	// defined by FrameNum will be saved. Returns success.
	bool Save(const tString& outFile, tSystem::tFileType fileType, bool useConfigSaveParams = true, bool onlyCurrentPic = false) const;

	int GetNumFrames() const																							{ return Pictures.Count(); }
	int GetNumPictures() const																							{ return Pictures.Count(); }

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
	static const int MaxDim					/* Max width or height. */													= 65536;
	tColour4b GetPixel(int x, int y) const;

	// Some images can store multiple complete images inside a single file (multiple frames).
	// The primary one is the first one.
	tImage::tPicture* GetPrimaryPic() const																				{ return Pictures.First(); }
	tImage::tPicture* GetFirstPic() const																				{ return Pictures.First(); }
	tImage::tPicture* GetCurrentPic() const																				{ tImage::tPicture* pic = Pictures.First(); for (int i = 0; i < FrameNum; i++) pic = pic ? pic->Next() : nullptr; return pic; }
	const tList<tImage::tPicture>& GetPictures() const																	{ return Pictures; }

	// Functions that edit and cause dirty flag to be set. Functions that return a bool will return false if the image
	// is unmodified and the dirty flag is untouched. Functions that are void should be assumed to modify the image.
	void Rotate90(bool antiClockWise);
	bool Rotate(float angle, const tColour4b& fill, tImage::tResampleFilter upFilter, tImage::tResampleFilter downFilter);

	// Quantize image colours based on a fixed palette. numColours must be 256 or less. checkExact means no change to
	// the image will be made if it already contains fewer colours than numColours already. This may or may not be
	// desireable as the computed or fixed palette would not be used.
	void QuantizeFixed(int numColours, bool checkExact = true);

	// Similar to above but uses spatial quantization to generate the palette. If ditherLevel is 0.0 it will compute a
	// good dither amount for you based on the image dimensions and number of colours. Filter size must be 1, 3, or 5.
	void QuantizeSpatial(int numColours, bool checkExact = true, double ditherLevel = 0.0, int filterSize = 3);

	// Similar to above but uses neuquant algorighm to generate the palette. With a sampling factor of 1 the entire
	// image is used in the learning phase. With a factor of 10, a pseudo-random subset of 1/10 of the pixels are used
	// in the learning phase. sampleFactor must be in [1, 30]. Bigger values are faster but lower quality.
	void QuantizeNeu(int numColours, bool checkExact = true, int sampleFactor = 1);

	// Similar to above but uses Wu algorighm to generate the palette.
	void QuantizeWu(int numColours, bool checkExact = true);

	bool AdjustmentBegin();
	enum class AdjChan { RGB, R, G, B, A };	// Adjustment is to individual RGBA channels or RGB/Intensity (default).
	static comp_t ComponentBits(AdjChan);	// Converts to tChannels.

	void AdjustBrightness(float brightness, AdjChan = AdjChan::RGB, bool allFrames = true);
	void AdjustContrast(float contrast, AdjChan = AdjChan::RGB, bool allFrames = true);
	void AdjustLevels
	(
		float blackPoint, float midPoint, float whitePoint, float blackOut, float whiteOut,
		bool powerMidGamma = true, AdjChan = AdjChan::RGB, bool allFrames = true
	);
	void AdjustRestoreOriginal(bool popUndo = false);

	// Some of the adjustment defaults are a function of the image properties. In particular the brightness range is
	// computed so that the full [0,1] range is used and no more. If the image doesn't have pixels that are both full
	// white and full black, the default (no modification) brightness may not be exactly in the middle (0.5). This
	// function must be called between begin/end and gets all the defaults at once.
	void AdjustGetDefaults(float& brightness, float& contrast, float& blackPoint, float& midPoint, float& whitePoint, float& blackOut, float& whiteOut) const;
	bool AdjustmentEnd();

	void Flip(bool horizontal);
	bool Crop(int newWidth, int newHeight, int originX, int originY, const tColour4b& fillColour = tColour4b::black);
	bool Crop(int newWidth, int newHeight, tImage::tPicture::Anchor, const tColour4b& fillColour = tColour4b::black);
	bool Paste(int regionW, int regionH, const tColour4b* regionPixels, int originX, int originY, comp_t channels = tCompBit_RGBA);
	bool Paste(int regionW, int regionH, const tColour4b* regionPixels, tImage::tPicture::Anchor, comp_t channels = tCompBit_RGBA);
	bool Deborder(const tColour4b& borderColour, comp_t channels = tCompBit_RGBA);
	bool Resample(int newWidth, int newHeight, tImage::tResampleFilter filter, tImage::tResampleEdgeMode edgeMode);
	void SetPixelColour(int x, int y, const tColour4b&, bool pushUndo, bool supressDirty = false);
	void SetAllPixels(const tColour4b& colour, comp_t channels = tCompBit_RGBA);

	// Spreads the specified single channel to all RGB channels.
	void Spread(tComp channel = tComp::R);

	// Swizzle colour channels. You specify the RGBA destination channels in that order. For example, to swap R and B
	// channels you would call Swizzle(B,G,R,A). You can also use tComp::Zero and tComp::Full to set the channel to zero
	// or full values. The default swizzle is RGBA which does nothing. If tComp::Auto is set for any channel, it just
	// uses the default for that channel.
	void Swizzle(tComp = tComp::R, tComp = tComp::G, tComp = tComp::B, tComp = tComp::A);

	// Computes RGB intensity and sets specified channels to that value. Any combination of RGBA allowed. This can be used,
	// for example, to create grey-scale images.
	void Intensity(comp_t channels = tCompBit_RGB);

	// Blends blendColour (background) into the RGB channels specified (usually RGB, but any combination of the 3 is
	// allowed) using the pixel alpha to modulate. The new pixel colour is alpha*component + (1-alpha)*blend_component.
	//
	// Eg. If pixel alpha is 255, then none of the blend colour is used for that pixel. If alpha is 0, all of it is
	// used. If alpha is 64, then 1/4 of the current pixel colour and 3/4 of the supplied,
	//
	// FinalAlpha should be in [-1, 255]. If finalAlpha is >= 0 then the alpha will be set to finalAlpha after the blend
	// is complete. If finalAlpha is -1, the alpha is left unmodified. By default the finalAlpha is 255 (opaque) which
	// means the operation essentially creates a premultiplied-alpha opaque image.
	// Note that the alpha of the supplied colour is ignored (since we use finalAlpha).
	// Note that unspecified RGB channels are keft unmodified.
	void AlphaBlendColour(const tColour4b& blendColour, comp_t = tCompBit_RGB, int finalAlpha = 255);
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
		tColourProfile SrcColourProfile					= tColourProfile::Unspecified;
		tAlphaMode AlphaMode							= tAlphaMode::Unspecified;
		tChannelType ChannelType						= tChannelType::Unspecified;

		enum class OpacityEnum { False, True, Varies };	// Varies is for when there is more than one picture in the image (animated, mipmaps, etc) and they are not set all the same.
		OpacityEnum Opacity								= OpacityEnum::False;
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
	uint32 ShuffleValue;								// Valid before load.

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

	// Zoom can be stored per-image so we can flip between images without losing the setting.
	Config::ProfileData::ZoomModeEnum ZoomMode = Config::ProfileData::ZoomModeEnum::DownscaleOnly;
	float ZoomPercent = 100.0f;
	void SetUndoEnabled(bool enabled)																					{ UndoEnabled = enabled; }

	bool OverrideBackgroundColour = false;
	tColour4b BackgroundColourOverride = tColour4b::black;

private:
	bool UndoEnabled = true;
	void PushUndo(const tString& desc)																					{ if (UndoEnabled) UndoStack.Push(Pictures, desc, Dirty); }
	void PopUndo()																										{ if (UndoEnabled) UndoStack.Pop(); }

	// There are multiple pictures for a few reasons. Images with multiple frames (gifs, exrs, tiffs, webps etc) store
	// the individual frames as separate pictures in the list, dds files may store a cubemap and the 6 sides are stored
	// in the picture list, and dds files may contain mipmaps, also stored in the list.
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

	// This function can handle DDS, PVR, and KTX images and populate the pictures list as well as create the
	// alternate image if necessary.
	void MultiSurfacePopulatePictures(const tImage::tBaseImage&);
	void MultiSurfaceCreateAltCubemapPicture(const teList<tImage::tLayer> layers[tImage::tFaceIndex::tFaceIndex_NumFaces]);
	void MultiSurfaceCreateAltMipmapPicture(const teList<tImage::tLayer>&);

	void GetGLFormatInfo(GLint& srcFormat, GLenum& srcType, GLint& dstFormat, bool& compressed, tImage::tPixelFormat);
	void BindLayers(const tList<tImage::tLayer>&, uint texID);

	float LoadedTime = -1.0f;
	bool Dirty = false;

	// Undo / Redo
	Undo::Stack UndoStack;
};


// Implementation below.


inline comp_t Image::ComponentBits(AdjChan channels)
{
	switch (channels)
	{
		case AdjChan::RGB:	return tCompBit_RGB;
		case AdjChan::R:	return tCompBit_R;
		case AdjChan::G:	return tCompBit_G;
		case AdjChan::B:	return tCompBit_B;
		case AdjChan::A:	return tCompBit_A;
	}
	return 0;
}


}
