// TacentView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2018-2025 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WINDOWS
#include <dwmapi.h>
#include <locale.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.
#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#include <clip.h>					// This is the cross-platform clipboard library.
#include <Foundation/tVersion.cmake.h>
#include <Foundation/tHash.h>
#include <System/tCmdLine.h>
#include <Image/tPicture.h>
#include <Image/tImageICO.h>
#include <Image/tImageTGA.h>		// For paste from clipboard.
#include <Image/tImagePNG.h>		// For paste from clipboard.
#include <Image/tImageWEBP.h>		// For paste from clipboard.
#include <Image/tImageQOI.h>		// For paste from clipboard.
#include <Image/tImageBMP.h>		// For paste from clipboard.
#include <Image/tImageTIFF.h>		// For paste from clipboard.
#include <System/tFile.h>
#include <System/tTime.h>
#include <System/tScript.h>
#include <System/tMachine.h>
#include <Math/tVector2.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "imgui_internal.h"			// For ImGuiWindow.
#include "TacentView.h"
#include "GuiUtil.h"
#include "Image.h"
#include "ColourDialogs.h"
#include "ImportRaw.h"
#include "Dialogs.h"
#include "Details.h"
#include "Preferences.h"
#include "Properties.h"
#include "ContactSheet.h"
#include "MultiFrame.h"
#include "ThumbnailView.h"
#include "Crop.h"
#include "Quantize.h"
#include "Resize.h"
#include "Rotate.h"
#include "OpenSaveDialogs.h"
#include "Config.h"
#include "InputBindings.h"
#include "Command.h"
#include "RobotoFontBase85.cpp"
#include "Version.cmake.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;


namespace Viewer
{
	// These are the basic options available when starting the viewer. When in CLI mode there are many more in use that
	// can be seen in the Command.cpp module. OptionCLI and OptionHelp are the only two that turn in the CLI mode.
	// The OptionProfile is the only control option for GUI mode -- useful, for example, for launchin the GUI in the
	// kiosk profile regardless of the last used profile stored in the config file.
	tCmdLine::tParam  ParamImageFiles	("Files to open",												"ImageFiles",			0,	true	);
	tCmdLine::tOption OptionProfile		("Launch GUI with the specified profile active.",				"profile",		'p',	1			);
	tCmdLine::tOption OptionCLI			("Use command line mode (required when using CLI)",				"cli",			'c'					);
	tCmdLine::tOption OptionHelp		("Help on usage",												"help",			'h',	0,	true	);

	tFileTypes FileTypes_Load
	(
		tFileType::TGA,
		tFileType::PNG,
		tFileType::JPG,
		tFileType::GIF,
		tFileType::WEBP,
		tFileType::QOI,
		tFileType::DDS,
		tFileType::PVR,
		tFileType::KTX,
		tFileType::KTX2,
		tFileType::ASTC,
		tFileType::PKM,
		tFileType::EXR,
		tFileType::HDR,
		tFileType::APNG,
		tFileType::BMP,
		tFileType::ICO,
		tFileType::TIFF,
		tFileType::EOL
	);

	tFileTypes FileTypes_Save
	(
		tFileType::TGA,
		tFileType::PNG,
		tFileType::JPG,
		tFileType::GIF,
		tFileType::WEBP,
		tFileType::QOI,
		tFileType::APNG,
		tFileType::BMP,
		tFileType::TIFF,
		tFileType::EOL
	);

	// All multiframe/animated types that are loadable.
	tFileTypes FileTypes_MultiFrame
	(
		tFileType::GIF,
		tFileType::WEBP,
		tFileType::APNG,
		tFileType::PNG,
		tFileType::TIFF,
		tFileType::EOL
	);

	tFileTypes FileTypes_SaveMultiFrame
	(
		tFileType::GIF,
		tFileType::WEBP,
		tFileType::APNG,
		tFileType::TIFF,
		tFileType::EOL
	);

	tFileTypes FileTypes_SupportsProperties
	{
		tFileType::HDR,
		tFileType::EXR,
		tFileType::DDS,
		tFileType::PVR,
		tFileType::KTX,
		tFileType::KTX2,
		tFileType::ASTC,
		tFileType::PKM,
		tFileType::WEBP,
		tFileType::TGA,
		tFileType::EOL
	};

	// When a paste happens the file that gets created must not be lossy. These formats either
	// only support lossless or have it as an option like webp.
	tFileTypes FileTypes_ClipboardPaste
	(
		tFileType::TGA,
		tFileType::PNG,
		tFileType::WEBP,
		tFileType::QOI,
		tFileType::BMP,
		tFileType::TIFF,
		tFileType::EOL
	);

	// File types that may be created when importing raw data. All of these must be lossless and support saving. TIFF
	// is the default as it supports multiple lossless surfaces at different frame sizes (mipmaps). Additionally APNG
	// and WEBP support multiple lossless frames but require a single canvas size. These 3 can support mipmaps but not
	// as well as TIFF. The remainder only support single lossless images and mipmap import is disabled.
	tFileTypes FileTypes_ImportRaw
	(
		tFileType::TIFF,		// Supports mipmaps. Multiple lossless frames. Saved mipmaps frames have different sizes.
		tFileType::APNG,		// Supports mipmaps. Multiple lossless frames. Saves mipmaps on frames of same size.
		tFileType::WEBP,		// Supports mipmaps. Multiple lossless frames. Saves mipmaps on frames of same size.
		tFileType::TGA,
		tFileType::PNG,
		tFileType::QOI,
		tFileType::BMP,
		tFileType::EOL
	);

	// This is the list of lossy formats where the viewer supports lossless transformations.
	tFileTypes FileTypes_LosslessTransform
	(
		tFileType::JPG,
		tFileType::EOL
	);

	tFileDialog::FileDialog OpenFileDialog(tFileDialog::DialogMode::OpenFile, Viewer::FileTypes_Load);
	tFileDialog::FileDialog OpenDirDialog(tFileDialog::DialogMode::OpenDir);
	tFileDialog::FileDialog SaveAsDialog(tFileDialog::DialogMode::SaveFile, Viewer::FileTypes_Save);

	OutputLog OutLog;

	tString ImagesDir;
	tList<tStringItem> ImagesSubDirs;
	tList<Image> Images;
	tItList<Image> ImagesLoadTimeSorted(tListMode::External);		// We don't need static here cuz the list is only used after main().
	tuint256 ImagesHash												= 0;
	Image* CurrImage												= nullptr;
	tString ImageToLoad;

	void LoadAppImages(const tString& assetsDir);
	void UnloadAppImages();
	Image Image_Reticle;
	Image Image_Next_Prev;							// For Prev flip UVs horizontally.
	Image Image_NextSide_PrevSide;					// For PrevSide flip UVs horizontally.
	Image Image_File;
	Image Image_Folder;
	Image Image_FlipH;
	Image Image_FlipV;
	Image Image_RotCW_RotACW;						// For RotACW flip UVs horizontally.
	Image Image_RotateTheta;
	Image Image_Fullscreen;
	Image Image_Windowed;
	Image Image_SkipEnd_SkipBegin;					// For SkipBegin flip UVs horizontally.
	Image Image_Refresh;
	Image Image_ShowHidden;
	Image Image_Recycle;
	Image Image_PropEdit;
	Image Image_InfoOverlay;
	Image Image_MetaData;
	Image Image_Help;
	Image Image_Prefs;
	Image Image_Tile;
	Image Image_Stop;
	Image Image_Play_PlayRev;						// For PlayRev flip UVs horizontally.
	Image Image_PlayLoop;
	Image Image_PlayOnce;
	Image Image_ChannelFilter;
	Image Image_Levels;
	Image Image_ContentView;
	Image Image_UpFolder;
	Image Image_Crop;
	Image Image_AnchorBL;
	Image Image_AnchorBM;
	Image Image_AnchorML;
	Image Image_AnchorMM;
	Image Image_DefaultThumbnail;

	GLFWwindow* Window								= nullptr;
	double DisappearCountdown						= DisappearDuration;
	double SlideshowCountdown						= 0.0;
	bool SlideshowPlaying							= false;
	bool ReticleVisibleOnSelect						= false;
	bool WindowIconified							= false;

	bool Request_OpenFileModal						= false;
	bool Request_OpenDirModal						= false;
	bool Request_SaveCurrentModal					= false;
	bool Request_SaveAsModal						= false;
	bool Request_SaveAllModal						= false;
	bool Request_ResizeImageModal					= false;
	bool Request_ResizeCanvasModal					= false;
	bool Request_RotateImageModal					= false;
	bool Request_LevelsModal						= false;
	bool Request_QuantizeModal						= false;
	bool Request_ContactSheetModal					= false;
	bool Request_MultiFrameModal					= false;
	bool Request_ExtractFramesModal					= false;
	bool Request_DeleteFileModal					= false;
	bool Request_DeleteFileNoRecycleModal			= false;
	bool Request_RenameModal						= false;
	bool Request_SnapMessage_NoFileBrowse			= false;
	bool Request_SnapMessage_NoFrameTrans			= false;
	bool Request_Quit								= false;
	bool Request_CropLineConstrain					= false;
	Anchor Request_PanSnap							= Anchor::Invalid;
	LosslessTransformMode Request_LosslessTrnsModal	= LosslessTransformMode::None;
	bool BindingsWindowJustOpened					= false;
	bool ImportRawWindowJustOpened					= false;
	bool ImportRawWindowJustClosed					= false;
	bool CropMode									= false;
	bool LMBDown									= false;
	bool RMBDown									= false;
	bool DeleteAllCacheFilesOnExit					= false;
	bool PendingTransparentWorkArea					= false;
	bool DrawChannel_AsIntensity					= false;
	bool DrawChannel_R								= true;
	bool DrawChannel_G								= true;
	bool DrawChannel_B								= true;
	bool DrawChannel_A								= true;
	float ShutterFXCountdown						= 0.0f;				// Used when copying image to clipboard.
	int PendingFrameBufferBPC						= int(Config::GlobalData::FrameBufferBPCEnum::BPC_Default);
	int DragAnchorX									= 0;
	int DragAnchorY									= 0;

	// CursorX/Y are the real position. CursorMouseX/Y are updated when mouse clicked and update CursorX/Y.
	int CursorX										= 0;
	int CursorY										= 0;
	float CursorMouseX								= -1.0f;
	float CursorMouseY								= -1.0f;
	float RotateAnglePreview						= 0.0f;

	int DispW										= 1;
	int DispH										= 1;
	int PanOffsetX									= 0;
	int PanOffsetY									= 0;
	int PanDragDownOffsetX							= 0;
	int PanDragDownOffsetY							= 0;
	tColour4b PixelColour							= tColour4b::black;

	const tVector4 ColourEnabledTint				= tVector4(1.00f, 1.00f, 1.00f, 1.00f);
	const tVector4 ColourDisabledTint				= tVector4(0.54f, 0.54f, 0.54f, 1.00f);
	const tVector4 ColourBG							= tVector4(0.00f, 0.00f, 0.00f, 0.00f);
	const tVector4 ColourPressedBG					= tVector4(0.21f, 0.45f, 0.21f, 1.00f);
	const tVector4 ColourClear						= tVector4(0.10f, 0.10f, 0.12f, 1.00f);

	// UI scaling and HighDPI support. Scale is driven by decision on min/max font sizes.
	const float MinFontPointSize					= 14.0f;
	const float MaxFontPointSize					= 36.4f;
	const float MinUIScale							= 1.0f;
	const float MaxUIScale							= MaxFontPointSize / MinFontPointSize;
	const float FontStepPointSize					= (MaxFontPointSize - MinFontPointSize) / (int(Config::ProfileData::UISizeEnum::NumSizes) - 1);
	Config::ProfileData::UISizeEnum FontUISizeAdded	= Config::ProfileData::UISizeEnum::Invalid;
	Config::ProfileData::UISizeEnum CurrentUISize	= Config::ProfileData::UISizeEnum::Invalid;
	Config::ProfileData::UISizeEnum DesiredUISize	= Config::ProfileData::UISizeEnum::Invalid;

	uint64 FrameNumber								= 0;

	// This function expects a valid (non-auto) UI size. It updates the style and ImGui font that are in use.
	void SetUISize(Viewer::Config::ProfileData::UISizeEnum);

	void DrawBackground(float l, float r, float b, float t, float drawW, float drawH);
	void PrintRedirectCallback(const char* text, int numChars);
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }
	bool Compare_AlphabeticalAscending		(const tSystem::tFileInfo& a, const tSystem::tFileInfo& b)					{ return tStricmp(a.FileName.Chars(), b.FileName.Chars()) < 0; }
	bool Compare_FileCreationTimeAscending	(const tSystem::tFileInfo& a, const tSystem::tFileInfo& b)					{ return a.CreationTime < b.CreationTime; }
	bool Compare_ImageLoadTimeAscending		(const Image& a, const Image& b)											{ return a.GetLoadedTime() < b.GetLoadedTime(); }

	// This is a 'FunctionObject'. Basically an object that acts like a function. This is sorta cool as it allows state
	// to be stored in the object. In this case we use it as the compare function for a Sort call. Instead of a
	// whackload of separate compare functions, we now only need one and we use the state information to determine the
	// desired sort key and direction (ascending or descending). Note: when compare functions are used to sort, they
	// result in ascending order if they return a < b and descending if they return a > b.
	struct ImageCompareFunctionObject
	{
		ImageCompareFunctionObject(Config::ProfileData::SortKeyEnum key, bool ascending) : Key(key), Ascending(ascending) { }
		Config::ProfileData::SortKeyEnum Key;
		bool Ascending;

		// This is what makes it a magical function object.
		bool operator() (const Image& a, const Image& b) const;
	};

	tColour4b GetClipboard16BPPColour(uint16 data, uint32 rmask, int rshift, uint32 gmask, int gshift, uint32 bmask, int bshift, uint32 amask, int ashift);
	tColour4b GetClipboard24BPPColour(uint32 data, uint32 rmask, int rshift, uint32 gmask, int gshift, uint32 bmask, int bshift, uint32 amask, int ashift);
	tColour4b GetClipboard32BPPColour(uint32 data, uint32 rmask, int rshift, uint32 gmask, int gshift, uint32 bmask, int bshift, uint32 amask, int ashift);
	tColour4b GetClipboard64BPPColour(uint64 data, uint32 rmask, int rshift, uint32 gmask, int gshift, uint32 bmask, int bshift, uint32 amask, int ashift);

	inline bool OnNextImage(bool next);
	inline bool OnLastImage(bool last);
	inline void OnNextImageFrame(bool next);
	enum class MoveDir { Right, Left, Up, Down };
	inline void OnPixelMove(MoveDir);
	inline void OnUISizeInc(bool inc);
	inline void OnZoomIn();
	inline void OnZoomOut();
	inline void OnZoomFit();
	inline void OnZoomDownscaleOnly();
	inline void OnZoomOneToOne();
	inline void OnZoomPerImageToggle();
	inline void OnResetPan();
	inline void OnFlipVert(bool vert);
	inline void OnRotate90(bool cw);
	inline void OnRotate();
	inline void OnCrop();
	inline void OnResizeImage();
	inline void OnResizeCanvas();
	inline void OnPixelEdit();
	inline void OnPropertyEdit();
	inline void OnChannelFilter();
	inline void OnLevels();
	inline void OnQuantize();
	inline void OnRedChannel();
	inline void OnGreenChannel();
	inline void OnBlueChannel();
	inline void OnAlphaChannel();
	inline void OnChannelIntensity();
	inline void OnDetails();
	inline void OnTile();
	bool		OnCopyToClipboard();
	bool		OnPasteFromClipboard();		// Handles image data as well as the text paths to image folders.
	inline void OnImportRaw();
	inline void OnRefresh();
	inline void OnRefreshDir();
	inline void OnRename();
	inline void OnDelete();
	inline void OnDeletePermanent();
	inline void OnSave();
	inline void OnSaveAs();
	inline void OnSaveAll();
	inline void OnSaveContactSheet();
	inline void OnSaveMultiFrameImage();
	inline void OnSaveExtractFrames();
	inline void OnMenuBar();
	inline void OnNavBar();
	inline void OnThumbnails();
	inline void OnFileBrowser();
	inline void OnSlideshowTimer();
	inline void OnSlideshowReshuffle();
	inline void OnCheatSheet();
	inline void OnDebugLog();
	inline void OnProfileMain();
	inline void OnProfileBasic();
	inline void OnProfileKiosk();
	inline void OnProfileAlt();
	inline void OnPreferences();
	inline void OnKeyBindings();
	inline void OnChangeScreenMode();
	inline void OnEscape();
	inline void OnEscapeWithQuit();
	inline void OnQuit();
	inline void OnOpenFile();
	inline void OnOpenDir();
	inline void OnMetaData();

	void ApplyZoomDelta(float zoomDelta);
	void AutoPropertyWindow();

	tString FindImagesInImageToLoadDir(tList<tSystem::tFileInfo>& foundFiles);		// Returns the image folder.
	tuint256 ComputeImagesHash(const tList<tSystem::tFileInfo>& files);
	int RemoveOldCacheFiles(const tString& cacheDir);								// Returns num removed.

	CursorMove RequestCursorMove = CursorMove_None;
	bool IgnoreNextCursorPosCallback = false;

	void Update(GLFWwindow* window, double dt, bool dopoll = true);
	int  DoMainMenuBar();																								// Returns height.
	int  GetNavBarHeight();
	void DoNavBar(int dispWidth, int dispHeight, int barHeight);
	void WindowRefreshFun(GLFWwindow* window)																			{ Update(window, 0.0, false); }
	void KeyCallback(GLFWwindow*, int key, int scancode, int action, int modifiers);
	void MouseButtonCallback(GLFWwindow*, int mouseButton, int x, int y);
	void CursorPosCallback(GLFWwindow*, double x, double y);
	void ScrollWheelCallback(GLFWwindow*, double x, double y);
	void FileDropCallback(GLFWwindow*, int count, const char** paths);
	void FocusCallback(GLFWwindow*, int gotFocus);
	void IconifyCallback(GLFWwindow*, int iconified);
	void ChangProfile(Viewer::Profile);
}


bool Viewer::ImageCompareFunctionObject::operator() (const Image& a, const Image& b) const
{
	switch (Key)
	{
		default:
		case Config::ProfileData::SortKeyEnum::Natural:
		{
			const char8_t* A = a.Filename.Chars();
			const char8_t* B = b.Filename.Chars();
			return Ascending ? (tNstrcmp(A, B) < 0) : (tNstrcmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::FileName:
		{
			const char8_t* A = a.Filename.Chars();
			const char8_t* B = b.Filename.Chars();
			return Ascending ? (tPstrcmp(A, B) < 0) : (tPstrcmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::FileModTime:
		{
			time_t A = a.FileModTime;
			time_t B = b.FileModTime;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::FileSize:
		{
			uint64 A = a.FileSizeB;
			uint64 B = b.FileSizeB;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::FileType:
		{
			const char* A = tGetExtension(a.Filetype).Chr();
			const char* B = tGetExtension(b.Filetype).Chr();
			return Ascending ? (tStricmp(A, B) < 0) : (tStricmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::ImageArea:
		{
			int A = a.Cached_PrimaryArea;
			int B = b.Cached_PrimaryArea;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::ImageWidth:
		{
			int A = a.Cached_PrimaryWidth;
			int B = b.Cached_PrimaryWidth;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::ImageHeight:
		{
			int A = a.Cached_PrimaryHeight;
			int B = b.Cached_PrimaryHeight;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaLatitude:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::LatitudeDD].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::LatitudeDD].Float : -100.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::LatitudeDD].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::LatitudeDD].Float : -100.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaLongitude:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::LongitudeDD].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::LongitudeDD].Float : -200.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::LongitudeDD].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::LongitudeDD].Float : -200.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaAltitude:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::Altitude].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Altitude].Float : -1000.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Altitude].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Altitude].Float : -1000.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaRoll:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::Roll].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Roll].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Roll].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Roll].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaPitch:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::Pitch].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Pitch].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Pitch].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Pitch].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaYaw:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::Yaw].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Yaw].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Yaw].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Yaw].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaSpeed:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::Speed].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Speed].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Speed].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Speed].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaShutterSpeed:
		{
			// Camera Shutter 'Speed' is measured in 1/s. 125 => 1/125th second. 0.0 (infinite) is considered the default for sorting purposes.
			float A = a.Cached_MetaData[tImage::tMetaTag::ShutterSpeed].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::ShutterSpeed].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::ShutterSpeed].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::ShutterSpeed].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaExposureTime:
		{
			// Exposure time is how long the shutter is open for. Basically the inverse of the shutter speed.
			// I don't know why EXIF data duplicates this explicitely. 0.0s is considered the default for exposure time.
			float A = a.Cached_MetaData[tImage::tMetaTag::ExposureTime].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::ExposureTime].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::ExposureTime].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::ExposureTime].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaFStop:
		{
			// No existing lens can get down to an f-stop of 0.5. That;s why we use 0.5 as the default.
			float A = a.Cached_MetaData[tImage::tMetaTag::FStop].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::FStop].Float : 0.5f;
			float B = b.Cached_MetaData[tImage::tMetaTag::FStop].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::FStop].Float : 0.5f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaISO:
		{
			// ISO as low as 25 exist. 100-200 is 'normal' speed film. 400 is fast (but grainy). We use 0 as default.
			float A = a.Cached_MetaData[tImage::tMetaTag::ISO].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::ISO].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::ISO].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::ISO].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaAperture:
		{
			// Aperture in APEX units can't get down to 0. We use 0 as default.
			float A = a.Cached_MetaData[tImage::tMetaTag::Aperture].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Aperture].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Aperture].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Aperture].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaOrientation:
		{
			// All non-90-degree transforms are grouped at the start. All 90-degree transforms have larger values.
			// This allows for meaningful sorting. The default 0 means 'unspecified'.
			uint32 A = a.Cached_MetaData[tImage::tMetaTag::Orientation].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Orientation].Uint32 : 0;
			uint32 B = b.Cached_MetaData[tImage::tMetaTag::Orientation].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Orientation].Uint32 : 0;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaBrightness:
		{
			// Aperture in APEX Bv units. 0 is dark -- about 3.4candelas/(m^2). We use 0 as default.
			float A = a.Cached_MetaData[tImage::tMetaTag::Brightness].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Brightness].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Brightness].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Brightness].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaFlash:
		{
			// Flash used is 0 for not used, 1 for used. We use 0 for default.
			uint32 A = a.Cached_MetaData[tImage::tMetaTag::FlashUsed].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::FlashUsed].Uint32 : 0;
			uint32 B = b.Cached_MetaData[tImage::tMetaTag::FlashUsed].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::FlashUsed].Uint32 : 0;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaFocalLength:
		{
			// Focal length in mm.  We use 0 as default which means unknown.
			float A = a.Cached_MetaData[tImage::tMetaTag::FocalLength].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::FocalLength].Float : 0.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::FocalLength].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::FocalLength].Float : 0.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileData::SortKeyEnum::MetaTimeTaken:
		{
			// String sort works because fields are ordered nicely. "YYYY-MM-DD hh:mm:ss". From Wikipedia: "The first
			// photographic camera developed for commercial manufacture was a daguerreotype camera, built by Alphonse
			// Giroux in 1839". We'll use Jan 1 of that year for the default time taken beacuse there should be no photos
			// before that date, even if you wanted to add EXIF data after.
			const char8_t* A = a.Cached_MetaData[tImage::tMetaTag::DateTimeOrig].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::DateTimeOrig].String.Chars() : u8"1839-01-01 00:00:00";
			const char8_t* B = b.Cached_MetaData[tImage::tMetaTag::DateTimeOrig].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::DateTimeOrig].String.Chars() : u8"1839-01-01 00:00:00";
			return Ascending ? (tStrcmp(A, B) < 0) : (tStrcmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::MetaTimeModified:
		{
			// String sort works because fields are ordered nicely. "YYYY-MM-DD hh:mm:ss". We use the same default as TimeTaken.
			const char8_t* A = a.Cached_MetaData[tImage::tMetaTag::DateTimeChange].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::DateTimeChange].String.Chars() : u8"1839-01-01 00:00:00";
			const char8_t* B = b.Cached_MetaData[tImage::tMetaTag::DateTimeChange].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::DateTimeChange].String.Chars() : u8"1839-01-01 00:00:00";
			return Ascending ? (tStrcmp(A, B) < 0) : (tStrcmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::MetaCameraMake:
		{
			// Empty string used for default. Empty is less than all other non-empty strings.
			const char8_t* A = a.Cached_MetaData[tImage::tMetaTag::MakeModelSerial].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::MakeModelSerial].String.Chars() : u8"";
			const char8_t* B = b.Cached_MetaData[tImage::tMetaTag::MakeModelSerial].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::MakeModelSerial].String.Chars() : u8"";
			return Ascending ? (tStricmp(A, B) < 0) : (tStricmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::MetaDescription:
		{
			// Empty string used for default. Empty is less than all other non-empty strings.
			const char8_t* A = a.Cached_MetaData[tImage::tMetaTag::Description].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Description].String.Chars() : u8"";
			const char8_t* B = b.Cached_MetaData[tImage::tMetaTag::Description].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Description].String.Chars() : u8"";
			return Ascending ? (tStricmp(A, B) < 0) : (tStricmp(A, B) > 0);
		}

		case Config::ProfileData::SortKeyEnum::Shuffle:
		{
			uint32 A = a.ShuffleValue;
			uint32 B = b.ShuffleValue;
			return Ascending ? (A < B) : (A > B);
		}
	}

	return false;
}


Viewer::Config::ProfileData::ZoomModeEnum Viewer::GetZoomMode()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ZoomPerImage && CurrImage)
		return CurrImage->ZoomMode;
	else
		return profile.GetZoomMode();
}


void Viewer::SetZoomMode(Config::ProfileData::ZoomModeEnum mode)
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ZoomPerImage && CurrImage)
		CurrImage->ZoomMode = mode;
	else
		profile.SetZoomMode(mode);
}


float Viewer::GetZoomPercent()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ZoomPerImage && CurrImage)
		return CurrImage->ZoomPercent;
	else
		return profile.ZoomPercent;
}


void Viewer::SetZoomPercent(float zoom)
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ZoomPerImage && CurrImage)
		CurrImage->ZoomPercent = zoom;
	else
		profile.ZoomPercent = zoom;
}


void Viewer::PrintRedirectCallback(const char* text, int numChars)
{
	OutLog.AddLog("%s", text);
	
	#ifdef PLATFORM_LINUX
	// We have a terminal in Linux so use it.
	printf("%s", text);
	#endif
}


tString Viewer::FindImagesInImageToLoadDir(tList<tSystem::tFileInfo>& foundFiles)
{
	tString imagesDir;
	if (!ImageToLoad.IsEmpty() && tSystem::tIsAbsolutePath(ImageToLoad))
		imagesDir = tSystem::tGetDir(ImageToLoad);
	else
		imagesDir = tSystem::tGetCurrentDir();

	tPrintf("Finding image files in %s\n", imagesDir.Chr());
	tSystem::tExtensions extensions(FileTypes_Load);
	tSystem::tFindFiles(foundFiles, imagesDir, extensions);

	return imagesDir;
}


tuint256 Viewer::ComputeImagesHash(const tList<tSystem::tFileInfo>& files)
{
	tuint256 hash = 0;
	for (tSystem::tFileInfo* item = files.First(); item; item = item->Next())
	{
		hash = tHash::tHashString256(item->FileName.Chr(), hash);
		hash = tHash::tHashData256((uint8*)&item->FileSize, sizeof(item->FileSize), hash);
		hash = tHash::tHashData256((uint8*)&item->ModificationTime, sizeof(item->ModificationTime), hash);
	}

	return hash;
}


void Viewer::PopulateImagesSubDirs()
{
	ImagesSubDirs.Clear();

	#ifdef PLATFORM_WINDOWS
	if (ImagesDir == "/")
	{
		tList<tStringItem> foundDrives;
		tSystem::tGetDrives(foundDrives);
		for (tStringItem* drv = foundDrives.First(); drv; drv = drv->Next())
		{
			tDriveInfo driveInfo;
			bool ok = tSystem::tGetDriveInfo(driveInfo, *drv, false, true);
			if (driveInfo.DriveState == tDriveState::Ready)
				ImagesSubDirs.Append(new tStringItem(*drv));
		}
		return;
	}
	#endif

	tList<tStringItem> foundDirs;
	tFindDirs(foundDirs, ImagesDir, false);
	for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
	{
		tString relPath = tGetRelativePath(ImagesDir, *dir);
		relPath = tGetSimplifiedPath(relPath);
		if (relPath[relPath.Length()-1] == '/')
			relPath.ExtractRight(1);

		ImagesSubDirs.Append(new tStringItem(relPath));
	}
}


void Viewer::PopulateImages()
{
	Images.Clear();
	ImagesLoadTimeSorted.Clear();

	tList<tSystem::tFileInfo> foundFiles;
	ImagesDir = FindImagesInImageToLoadDir(foundFiles);
	PopulateImagesSubDirs();

	// We sort here so ComputeImagesHash always returns consistent values.
	foundFiles.Sort(Compare_AlphabeticalAscending, tListSortAlgorithm::Merge);
	ImagesHash = ComputeImagesHash(foundFiles);

	for (tSystem::tFileInfo* fileInfo = foundFiles.First(); fileInfo; fileInfo = fileInfo->Next())
	{
		// It is important we don't call Load after newing. We save memory by not having all images loaded.
		Image* newImg = new Image(*fileInfo);
		Images.Append(newImg);
		ImagesLoadTimeSorted.Append(newImg);
	}

	Config::ProfileData& profile = Config::GetProfileData();
	SortImages(profile.GetSortKey(), profile.SortAscending);
	CurrImage = nullptr;
}


void Viewer::SortImages(Config::ProfileData::SortKeyEnum key, bool ascending)
{
	ImageCompareFunctionObject compObj(key, ascending);
	Images.Sort(compObj);
}


Viewer::Image* Viewer::FindImage(const tString& filename)
{
	Image* img = nullptr;
	for (Image* si = Images.First(); si; si = si->Next())
	{
		if (si->Filename.IsEqualCI(filename))
		{
			img = si;
			break;
		}
	}

	return img;
}


bool Viewer::SetCurrentImage(const tString& currFilename, bool forceReload)
{
	bool found = false;
	for (Image* si = Images.First(); si; si = si->Next())
	{
		tString siName = tSystem::tGetFileName(si->Filename);
		tString imgName = tSystem::tGetFileName(currFilename);

		// We want a case-sensitive compare on Linux.
		if (tPstrcmp(siName.Chars(), imgName.Chars()) == 0)
		{
			CurrImage = si;
			found = true;
			break;
		}
	}

	if (!CurrImage)
	{
		CurrImage = Images.First();
		if (!currFilename.IsEmpty())
			tPrintf("Could not display [%s].\n", tSystem::tGetFileName(currFilename).Chr());
		if (CurrImage && !CurrImage->Filename.IsEmpty())
			tPrintf("First image [%s].\n", tSystem::tGetFileName(CurrImage->Filename).Chr());
	}

	if (CurrImage)
		LoadCurrImage(forceReload);

	return found;
}


void Viewer::AutoPropertyWindow()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.AutoPropertyWindow && CurrImage)
		profile.ShowPropsWindow = FileTypes_SupportsProperties.Contains(CurrImage->Filetype) || (CurrImage->GetNumFrames() > 1);

	if (SlideshowPlaying)
		profile.ShowPropsWindow = false;
}


void Viewer::LoadCurrImage(bool forceReload)
{
	tAssert(CurrImage);
	bool imgJustLoaded = false;

	if (!CurrImage->IsLoaded())
	{
		imgJustLoaded = CurrImage->Load();
	}
	else if (forceReload)
	{
		CurrImage->Unbind();
		CurrImage->Unload(true);
		imgJustLoaded = CurrImage->Load();
		CurrImage->Bind();
	}

	AutoPropertyWindow();
	Gutil::SetWindowTitle();
	if (!CurrImage->IsLoaded())
	{
		tPrintf("Warning: Failed to load [%s]\n", tGetFileName(CurrImage->Filename).Chr());
		return;
	}

	Config::ProfileData& profile = Config::GetProfileData();
	if
	(
		profile.AutoPlayAnimatedImages && !profile.ShowImportRaw &&
		(CurrImage->GetNumFrames() > 1) &&
		FileTypes_MultiFrame.Contains(CurrImage->Filetype)
	)
	{
		CurrImage->FramePlayLooping = true;
		CurrImage->FramePlayRev = false;
		CurrImage->Play();
	}

	ResetPan();
	Request_CropLineConstrain = true;

	// We only need to consider unloading an image when a new one is loaded... in this function.
	// We currently do not allow unloading when in slideshow and the frame duration is small.
	bool slideshowSmallDuration = SlideshowPlaying && (profile.SlideshowPeriod < 0.5f);
	if (imgJustLoaded && !slideshowSmallDuration)
	{
		ImagesLoadTimeSorted.Sort(Compare_ImageLoadTimeAscending);

		int64 usedMem = 0;
		for (tItList<Image>::Iter iter = ImagesLoadTimeSorted.First(); iter; iter++)
			usedMem += int64((*iter).Info.MemSizeBytes);

		int64 allowedMem = int64(profile.MaxImageMemMB) * 1024 * 1024;
		if (usedMem > allowedMem)
		{
			tPrintf("Used image mem (%|64d) bigger than max (%|64d). Unloading.\n", usedMem, allowedMem);
			for (tItList<Image>::Iter iter = ImagesLoadTimeSorted.First(); iter; iter++)
			{
				Image* i = iter.GetObject();

				// Never unload the current image.
				if (i->IsLoaded() && (i != CurrImage))
				{
					tPrintf("Unloading %s freeing %d Bytes\n", tSystem::tGetFileName(i->Filename).Chr(), i->Info.MemSizeBytes);
					usedMem -= i->Info.MemSizeBytes;
					i->Unload();
					if (usedMem < allowedMem)
						break;
				}
			}
			tPrintf("Used mem %|64dB out of max %|64dB.\n", usedMem, allowedMem);
		}
	}

	ReticleVisibleOnSelect = false;
}


bool Viewer::OnNextImage(bool next)
{
	if (!CurrImage)
		return false;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return false;

	bool circ = SlideshowPlaying && profile.SlideshowLooping;
	bool avail = next ? CurrImage->Next() : CurrImage->Prev();
	if (!circ && !avail)
		return false;

	if (SlideshowPlaying)
		SlideshowCountdown = profile.SlideshowPeriod;

	if (next)
		{ CurrImage = circ ? Images.NextCirc(CurrImage) : CurrImage->Next(); }
	else
		{ CurrImage = circ ? Images.PrevCirc(CurrImage) : CurrImage->Prev(); }
	
	LoadCurrImage();
	return true;
}


bool Viewer::OnLastImage(bool last)
{
	if (!CurrImage)
		return false;
	if ((last && !Images.Last()) || (!last && !Images.First()))
		return false;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return false;

	CurrImage = last ? Images.Last() : Images.First();
	LoadCurrImage();
	return true;
}


void Viewer::OnNextImageFrame(bool next)
{
	if (!CurrImage || !CurrImage->IsLoaded() || (CurrImage->GetNumFrames() <= 1))
		return;

	CurrImage->Stop();
	CurrImage->FrameNum = next ? tClampMax(CurrImage->FrameNum+1, CurrImage->GetNumFrames()-1) : tClampMin(CurrImage->FrameNum-1, 0);
}


void Viewer::OnPixelMove(MoveDir dir)
{
	if (!CurrImage || !CurrImage->IsLoaded())
		return;

	switch (dir)
	{
		case MoveDir::Right:	RequestCursorMove = CursorMove_Right;	break;
		case MoveDir::Left:		RequestCursorMove = CursorMove_Left;	break;
		case MoveDir::Up:		RequestCursorMove = CursorMove_Up;		break;
		case MoveDir::Down:		RequestCursorMove = CursorMove_Down;	break;
	}		
}


void Viewer::OnUISizeInc(bool inc)
{
	Config::ProfileData& profile = Config::GetProfileData();
	int sizeInt = profile.UISize;
	if (sizeInt == int(Config::ProfileData::UISizeEnum::Auto))
		sizeInt = int(Viewer::CurrentUISize);
	if (inc &&  (sizeInt >= int(Config::ProfileData::UISizeEnum::Smallest)) && (sizeInt <= int(Config::ProfileData::UISizeEnum::Largest)-1))
		sizeInt++;
	if (!inc && (sizeInt >= int(Config::ProfileData::UISizeEnum::Smallest)+1) && (sizeInt <= int(Config::ProfileData::UISizeEnum::Largest)))
		sizeInt--;
	if (profile.UISize != sizeInt)
	{
		profile.UISize = sizeInt;
		UpdateDesiredUISize();
	}
}


void Viewer::OnZoomIn()				{ if (CurrImage && CurrImage->IsLoaded()) ApplyZoomDelta(tMath::tRound(GetZoomPercent()*0.1f)); }
void Viewer::OnZoomOut()			{ if (CurrImage && CurrImage->IsLoaded()) ApplyZoomDelta(tMath::tRound(GetZoomPercent()*(0.909090909f - 1.0f))); }
void Viewer::OnZoomFit()			{ ResetPan(); SetZoomMode(Config::ProfileData::ZoomModeEnum::Fit); }
void Viewer::OnZoomDownscaleOnly()	{ ResetPan(); SetZoomMode(Config::ProfileData::ZoomModeEnum::DownscaleOnly); }
void Viewer::OnZoomOneToOne()		{ SetZoomPercent(100.0f); ResetPan(); SetZoomMode(Config::ProfileData::ZoomModeEnum::OneToOne); }
void Viewer::OnZoomPerImageToggle()	{ Config::ProfileData& profile = Config::GetProfileData(); profile.ZoomPerImage = !profile.ZoomPerImage; }
void Viewer::OnResetPan()			{ ResetPan(true, true); }


void Viewer::OnFlipVert(bool vert)
{
	if (!CurrImage || !CurrImage->IsLoaded() || CurrImage->IsAltPictureEnabled())
		return;

	if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
	{
		Request_LosslessTrnsModal = vert ? LosslessTransformMode::FlipV : LosslessTransformMode::FlipH;
	}
	else
	{
		CurrImage->Unbind();
		CurrImage->Flip(!vert);
		CurrImage->Bind();
		Gutil::SetWindowTitle();
	}
}


void Viewer::OnRotate90(bool cw)
{
	if (!CurrImage || !CurrImage->IsLoaded() || CurrImage->IsAltPictureEnabled())
		return;

	if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
	{
		Request_LosslessTrnsModal = cw ? LosslessTransformMode::Rot90CW : LosslessTransformMode::Rot90ACW;
	}
	else
	{
		CurrImage->Unbind();
		CurrImage->Rotate90(!cw);
		CurrImage->Bind();
		Gutil::SetWindowTitle();
	}
}


void Viewer::OnRotate()				{ if (CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled()) Request_RotateImageModal = true; }


void Viewer::OnCrop()
{
	bool cropAvail = CropMode || (CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled());
	if (!cropAvail)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	CropMode = !CropMode;
	if (CropMode)
		profile.Tile = false;
}


void Viewer::OnResizeImage()		{ if (CurrImage && CurrImage->IsLoaded()) Request_ResizeImageModal = true; }
void Viewer::OnResizeCanvas()		{ if (CurrImage && CurrImage->IsLoaded()) Request_ResizeCanvasModal = true; }
void Viewer::OnPixelEdit()			{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowPixelEditor = !profile.ShowPixelEditor; }
void Viewer::OnPropertyEdit()		{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowPropsWindow = !profile.ShowPropsWindow; }
void Viewer::OnChannelFilter()		{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowChannelFilter = !profile.ShowChannelFilter; }
void Viewer::OnLevels()				{ if (CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled()) Request_LevelsModal = true; }
void Viewer::OnQuantize()			{ if (CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled()) Request_QuantizeModal = true; }
void Viewer::OnRedChannel()			{ Config::ProfileData& profile = Config::GetProfileData(); if (DrawChannel_AsIntensity) { DrawChannel_R = true; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = false; } else DrawChannel_R = !DrawChannel_R; profile.ShowChannelFilter = true; }
void Viewer::OnGreenChannel()		{ Config::ProfileData& profile = Config::GetProfileData(); if (DrawChannel_AsIntensity) { DrawChannel_R = false; DrawChannel_G = true; DrawChannel_B = false; DrawChannel_A = false; } else DrawChannel_G = !DrawChannel_G; profile.ShowChannelFilter = true; }
void Viewer::OnBlueChannel()		{ Config::ProfileData& profile = Config::GetProfileData(); if (DrawChannel_AsIntensity) { DrawChannel_R = false; DrawChannel_G = false; DrawChannel_B = true; DrawChannel_A = false; } else DrawChannel_B = !DrawChannel_B; profile.ShowChannelFilter = true; }
void Viewer::OnAlphaChannel()		{ Config::ProfileData& profile = Config::GetProfileData(); if (DrawChannel_AsIntensity) { DrawChannel_R = false; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = true; } else DrawChannel_A = !DrawChannel_A; profile.ShowChannelFilter = true; }
void Viewer::OnChannelIntensity()	{ Config::ProfileData& profile = Config::GetProfileData(); DrawChannel_AsIntensity = !DrawChannel_AsIntensity; if (DrawChannel_AsIntensity) { DrawChannel_R = true; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = false; } else { DrawChannel_R = true; DrawChannel_G = true; DrawChannel_B = true; DrawChannel_A = true; } profile.ShowChannelFilter = true; }
void Viewer::OnDetails()			{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowImageDetails = !profile.ShowImageDetails; }
void Viewer::OnTile()				{ Config::ProfileData& profile = Config::GetProfileData(); if (CropMode) return; profile.Tile = !profile.Tile; if (!profile.Tile) ResetPan(); }


void Viewer::OnUndo()
{
	if (!CurrImage || !CurrImage->IsUndoAvailable())
		return;

	CurrImage->Unbind();
	CurrImage->Undo();
	CurrImage->Bind();
	Gutil::SetWindowTitle();
}


void Viewer::OnRedo()
{
	if (!CurrImage || !CurrImage->IsRedoAvailable())
		return;

	CurrImage->Unbind();
	CurrImage->Redo();
	CurrImage->Bind();
	Gutil::SetWindowTitle();
}


bool Viewer::OnCopyToClipboard()
{
	if (!CurrImage || !CurrImage->IsLoaded())
		return false;
	tImage::tPicture* pic = CurrImage->GetCurrentPic();
	if (!pic)
		pic = CurrImage->GetPrimaryPic();
	if (!pic || !pic->IsValid())
		return false;
	Config::ProfileData& profile = Config::GetProfileData();

	// We need to give the data to the clip system with first row at top.
	tImage::tPicture pict(*pic);
	pict.Flip(false);

	// Depending on the channel filters we may need to either fill some channels with the fill-colour
	// or with a specific channel's values. Specifically, if the intensity filter is chosen we copy
	// the R G B or A to the clipboard where R=G=B, A=1). In non-intensity mode individual channels
	// are copied to RGBA on the clipboard (R->R, G->G, etc) and unselected channels get the fill colour.
	// Default is RGBA all checked and non-intensity mode.
	if (Viewer::DrawChannel_AsIntensity)
	{
		if (Viewer::DrawChannel_R)
			pict.Spread(tComp::R);
		else if (Viewer::DrawChannel_G)
			pict.Spread(tComp::G);
		else if (Viewer::DrawChannel_B)
			pict.Spread(tComp::B);
		else if (Viewer::DrawChannel_A)
			pict.Spread(tComp::A);

		// Set alpha to full. We just want the intensity of the selected R, G, B, or A.
		pict.SetAll(tColour4b::black, tCompBit_A);
	}
	else
	{
		// This clipboard image has all the channels currently. We simply need to fill the
		// unselected channels with the corresponding copy-fill-colour channel value.
		comp_t fillChannels = (!Viewer::DrawChannel_R ? tCompBit_R : 0) | (!Viewer::DrawChannel_G ? tCompBit_G : 0) | (!Viewer::DrawChannel_B ? tCompBit_B : 0) | (!Viewer::DrawChannel_A ? tCompBit_A : 0);
		pict.SetAll(profile.ClipboardCopyFillColour, fillChannels);
	}

	clip::image_spec spec;
	spec.width			= pict.GetWidth();
	spec.height			= pict.GetHeight();
	spec.bits_per_pixel	= 32;
	spec.bytes_per_row	= spec.width*4;
	spec.red_mask		= 0x000000FF;
	spec.green_mask		= 0x0000FF00;
	spec.blue_mask		= 0x00FF0000;
	spec.alpha_mask		= 0xFF000000;
	spec.red_shift		= 0;
	spec.green_shift	= 8;
	spec.blue_shift		= 16;
	spec.alpha_shift	= 24;
	clip::image img(pict.GetPixels(), spec);
	bool success = clip::set_image(img);

	tPrintf("Copy Frame to Clipboard Result: %B\n", success);
	ShutterFXCountdown	= 0.12f;

	return success;
}


tColour4b Viewer::GetClipboard16BPPColour
(
	uint16 data,
	uint32 rmask, int rshift,
	uint32 gmask, int gshift,
	uint32 bmask, int bshift,
	uint32 amask, int ashift
)
{
	// There is a subtle bug here for the endpoints if we don't divide by the proper range.
	// See comment "Normalize to range" in tPixelutil.cpp.
	float rrange = float( (rmask >> rshift) );	// e.g. 5 bits set would yield 31.0f.
	float rf = float((data & rmask) >> rshift) / rrange;
	float grange = float( (gmask >> gshift) );
	float gf = float((data & gmask) >> gshift) / grange;
	float brange = float( (bmask >> bshift) );
	float bf = float((data & bmask) >> bshift) / brange;
	float af = 255;
	if (amask)
	{
		float arange = float( (amask >> ashift) );
		af = float((data & amask) >> ashift) / arange;
	}
	
	tColour4b col(rf, gf, bf, af);
	return col;
}


tColour4b Viewer::GetClipboard24BPPColour
(
	uint32 data,
	uint32 rmask, int rshift,
	uint32 gmask, int gshift,
	uint32 bmask, int bshift,
	uint32 amask, int ashift
)
{
	// We only support 8 bit RGB masks for 24bpp. This avoids float arithmetic at the
	// cost of possibly not supporting endpoints of some rare edge case pixel formats.
	uint8 r = ((data & rmask) >> rshift);
	uint8 g = ((data & gmask) >> gshift);
	uint8 b = ((data & bmask) >> bshift);
	uint8 a = 255;
	tColour4b col(r, g, b, a);
	return col;
}


tColour4b Viewer::GetClipboard32BPPColour
(
	uint32 data,
	uint32 rmask, int rshift,
	uint32 gmask, int gshift,
	uint32 bmask, int bshift,
	uint32 amask, int ashift
)
{
	// We only support 8 bit RGBA masks for 32bpp. This avoids float arithmetic at the
	// cost of possibly not supporting endpoints of some rare edge case pixel formats.
	uint8 r = ((data & rmask) >> rshift);
	uint8 g = ((data & gmask) >> gshift);
	uint8 b = ((data & bmask) >> bshift);
	uint8 a = 255;
	if (amask)
		a = ((data & amask) >> ashift);
	tColour4b col(r, g, b, a);
	return col;
}


tColour4b Viewer::GetClipboard64BPPColour
(
	uint64 data,
	uint32 rmask, int rshift,
	uint32 gmask, int gshift,
	uint32 bmask, int bshift,
	uint32 amask, int ashift
)
{
	// We only support 16 bit RGBA masks for 64bpp. This avoids float arithmetic at the
	// cost of possibly not supporting endpoints of some rare edge case pixel formats.
	uint8 r = ((data & rmask) >> rshift) >> 8;
	uint8 g = ((data & gmask) >> gshift) >> 8;
	uint8 b = ((data & bmask) >> bshift) >> 8;
	uint8 a = 255;
	if (amask)
		a = ((data & amask) >> ashift) >> 8;
	tColour4b col(r, g, b, a);
	return col;
}


bool Viewer::OnPasteFromClipboard()
{
	using namespace tImage;

	bool ok = false;
	ok = clip::has(clip::text_format());
	if (ok)
	{
		std::string text;
		ok = clip::get_text(text);
		if (ok)
		{
			tString path(text.c_str());
			tPrintf("Pasted Text: [%s]\n", text.c_str());

			// Deal with text input. Basically update any open file dialogs.
			// @todo Otherwise, perhaps, just go to the pasted image.
			if (OpenFileDialog.IsPopupOpen())
				return OpenFileDialog.SetPath(path);
			else if (OpenDirDialog.IsPopupOpen())
				return OpenDirDialog.SetPath(path);
			else if (SaveAsDialog.IsPopupOpen())
				return SaveAsDialog.SetPath(path);
			else
			{
				// Just set the current image.
				tPathStd(path);
				tString chosenFile, dir;
				tString file;
				if (tFileExists(path))
				{
					chosenFile = path;
				}
				else if (tDirExists(path))
				{
					tPathStdDir(path);
					chosenFile = path + "dummyfile.txt";
				}
				if (chosenFile.IsEmpty())
					return false;

				tPrintf("Opening file: %s\n", chosenFile.Chr());
				ImageToLoad = chosenFile;
				PopulateImages();
				SetCurrentImage(chosenFile);
				Gutil::SetWindowTitle();
				return true;
			}
			return false;
		}

		// Couldn't retrieve clipped text.
		return false;
	}

	ok = clip::has(clip::image_format());
	if (!ok)
		return false;

	clip::image img;
	ok = clip::get_image(img);

	// It is OK to just return as img goes out of scope and its destructor calls reset.
	if (!ok || !img.is_valid())
		return false;

	//
	// Get the data in the necessary tPixel RGBA format with rows starting at bottom.
	//
	const clip::image_spec& spec = img.spec();
	int width = spec.width;
	int height = spec.height;
	if ((width <= 0) || (height <= 0))
		return false;

	int bpp			= spec.bits_per_pixel;
	uint32 rmask	= spec.red_mask;		int rshift = spec.red_shift;
	uint32 gmask	= spec.green_mask;		int gshift = spec.green_shift;
	uint32 bmask	= spec.blue_mask;		int bshift = spec.blue_shift;
	uint32 amask	= spec.alpha_mask;		int ashift = spec.alpha_shift;

	// We support 16, 24, 32, and 64 bpp.
	if ((bpp != 16) && (bpp != 24) && (bpp != 32) && (bpp != 64))
		return false;

	// We require masks for RGB always.
	if (!rmask || !gmask || !bmask)
		return false;

	// For 24bpp we only support component masks that have 8 bits set and require a zero alpha mask.
	if (bpp == 24)
	{
		if ((rmask >> rshift) != 0xFF)				return false;
		if ((gmask >> gshift) != 0xFF)				return false;
		if ((bmask >> bshift) != 0xFF)				return false;
		if (spec.alpha_mask)						return false;
	}

	// For 32bpp we only support component masks that have 8 bits set.
	else if (bpp == 32)
	{
		if ((rmask >> rshift) != 0xFF)				return false;
		if ((gmask >> gshift) != 0xFF)				return false;
		if ((bmask >> bshift) != 0xFF)				return false;
		if (amask && ((amask >> ashift) != 0xFF))	return false;
	}

	// For 64bpp we only support component masks that have 16 bits set.
	else if (bpp == 64)
	{
		if ((rmask >> rshift) != 0xFFFF)			return false;
		if ((gmask >> gshift) != 0xFFFF)			return false;
		if ((bmask >> bshift) != 0xFFFF)			return false;
		if (amask && ((amask >> ashift) != 0xFFFF))	return false;
	}
	// We fall through here so that the 16bpp version of GetClipboard16BPPColour will be called.

	int bytesPerRow = spec.bytes_per_row;
	int bytesPerPixel = bpp / 8;
	uint8* srcData = (uint8*)img.data();
	tPixel4b* dstData = new tPixel4b[width*height];

	for (int y = 0; y < height; y++)
	{
		uint8* p = (srcData+bytesPerRow*y);
		for (int x = 0; x < width; x++, p += bytesPerPixel)
		{
			tColour4b col;
			switch (bpp)
			{
				case 16: col = GetClipboard16BPPColour( *((uint16*)p), rmask, rshift, gmask, gshift, bmask, bshift, amask, ashift);		break;
				case 24: col = GetClipboard24BPPColour( *((uint32*)p), rmask, rshift, gmask, gshift, bmask, bshift, amask, ashift);		break;
				case 32: col = GetClipboard32BPPColour( *((uint32*)p), rmask, rshift, gmask, gshift, bmask, bshift, amask, ashift);		break;
				case 64: col = GetClipboard64BPPColour( *((uint64*)p), rmask, rshift, gmask, gshift, bmask, bshift, amask, ashift);		break;
			}
			dstData[width*(height-1-y) + x] = col;
		}
    }

	// We are done with the img. We might as well clean it up now instead of waiting for scope to end.
	img.reset();
	Config::ProfileData& profile = Config::GetProfileData();

	// This is a workaround for systems that have content in the clipboard that is rolled a few
	// pixels either horizontally or vertically. It's slow but not needed by most users.
	if (profile.ClipboardPasteRollH)
	{
		tPixel4b* rolledData = new tPixel4b[width*height];
		int startCol = tMod(-profile.ClipboardPasteRollH, width);
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				rolledData[width*y + x] = dstData[width*y + startCol];
			}
			startCol++;
			startCol = tMod(startCol, width);
		}

		delete[] dstData;
		dstData = rolledData;
	}

	if (profile.ClipboardPasteRollV)
	{
		tPixel4b* rolledData = new tPixel4b[width*height];
		int startRow = tMod(-profile.ClipboardPasteRollV, height);
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				rolledData[width*y + x] = dstData[width*startRow + x];
			}
			startRow++;
			startRow = tMod(startRow, height);
		}

		delete[] dstData;
		dstData = rolledData;
	}

	// At this point we have width, height, and dstData all valid.

	// There are two paste-modes in tacent view: create a new image or paste into the current image.
	if (profile.ClipboardPasteCreatesImage)
	{
		// Essentially what we do is create a new image file (in a lossless/saveable image format as specified
		// by the user config), and then add a new Image based on it to the list, and finally set the current
		// image to it. The filename needs to be unique for the current directory.

		//
		// Step 1. Determine filename.
		//
		tFileType pasteType = tGetFileTypeFromName(profile.ClipboardPasteFileType);
		tString filename;
		tsPrintf
		(
			filename, "%sClipboardImage_%s.%s",
			Viewer::ImagesDir.Chr(),
			tConvertTimeToString(tGetTimeLocal(), tTimeFormat::Filename).Chr(),
			tGetExtension(pasteType).Chr()
		);

		//
		// Step 2. Save the file in the correct format losslessly. The dstData is given to the tImage. It will delete it.
		//
		bool saved = false;
		switch (pasteType)
		{
			case tFileType::TGA:
			{
				tImageTGA tga;
				tga.Set(dstData, width, height, true);
				tImageTGA::tFormat fmt = tga.Save(filename, tImageTGA::tFormat::Auto, tImageTGA::tCompression::None);
				saved = (fmt != tImageTGA::tFormat::Invalid);
				break;
			}

			case tFileType::PNG:
			{
				tImage::tImagePNG png;
				png.Set(dstData, width, height, true);
				tImagePNG::tFormat fmt = png.Save(filename, tImagePNG::tFormat::Auto);
				saved = (fmt != tImagePNG::tFormat::Invalid);
				break;
			}

			case tFileType::WEBP:
			{
				tImage::tImageWEBP webp;
				webp.Set(dstData, width, height, true);
				saved = webp.Save(filename, false);
				break;
			}

			case tFileType::QOI:
			{
				tImage::tImageQOI qoi;
				qoi.Set(dstData, width, height, true);
				tImageQOI::tFormat fmt = qoi.Save(filename, tImageQOI::tFormat::Auto);
				saved = (fmt != tImageQOI::tFormat::Invalid);
				break;
			}

			case tFileType::BMP:
			{
				tImage::tImageBMP bmp;
				bmp.Set(dstData, width, height, true);
				tImageBMP::tFormat fmt = bmp.Save(filename, tImageBMP::tFormat::Auto);
				saved = (fmt != tImageBMP::tFormat::Invalid);
				break;
			}

			case tFileType::TIFF:
			{
				tImage::tImageTIFF tiff;
				tiff.Set(dstData, width, height, true);
				tImageTIFF::SaveParams params;
				params.UseZLibCompression = true;
				saved = tiff.Save(filename, params);
				break;
			}

			default:
			{
				delete[] dstData;
				return false;
			}
		}

		if (!saved)
			return false;

		tPrintf("Pasted Filename: %s\n", filename.Chr());

		//
		// Step 3. Make image current. Add to images list, sort, and make current.
		//
		Image* newImg = new Image(filename);
		Images.Append(newImg);
		ImagesLoadTimeSorted.Append(newImg);
		SortImages(profile.GetSortKey(), profile.SortAscending);
		SetCurrentImage(filename);

		// In the clip sample code the clipboard is clear()-ed. I don't think we'd want to do that.
	}

	// The else is for the mode where we paste into the current image. At this point we have width, height, and dstData all valid.
	else
	{
		if (!CurrImage)
			return false;

		comp_t channels = (Viewer::DrawChannel_R ? tCompBit_R : 0) | (Viewer::DrawChannel_G ? tCompBit_G : 0) | (Viewer::DrawChannel_B ? tCompBit_B : 0) | (Viewer::DrawChannel_A ? tCompBit_A : 0);
		if (Viewer::DrawChannel_AsIntensity)
		{
			// If intensity channel view-mode is chosen we compute the clipboard RGB intensity and paste
			// to the specific selected single channel (which may be alpha). Here we modify dstData to
			// have intensity values only.
			for (int p = 0; p < width*height; p++)
			{
				int intensity = dstData[p].Intensity();
				dstData[p].Set(intensity, intensity, intensity, intensity);
			}
		}

		CurrImage->Unbind();
		CurrImage->Paste(width, height, dstData, profile.GetPasteAnchor(), channels);
		CurrImage->Bind();
		Gutil::SetWindowTitle();
	}

	return true;
}


void Viewer::OnImportRaw()
{
	Config::ProfileData& profile = Config::GetProfileData();
	profile.ShowImportRaw = !profile.ShowImportRaw;
	if (profile.ShowImportRaw)
		ImportRawWindowJustOpened = true;
	else
		ImportRawWindowJustClosed = true;
}


void Viewer::OnRefresh()
{
	if (!CurrImage)
		return;

	CurrImage->Unbind();
	CurrImage->Unload(true);
	CurrImage->Load();
	CurrImage->Bind();
	Gutil::SetWindowTitle();
}


void Viewer::OnRefreshDir()
{
	tList<tSystem::tFileInfo> files;
	ImagesDir = FindImagesInImageToLoadDir(files);
	PopulateImagesSubDirs();

	PopulateImages();

	// This deals with ImageToLoad being empty.
	SetCurrentImage(ImageToLoad);

	tPrintf("Refreshed directory %s\n", ImagesDir.Chr());
}


void Viewer::OnRename()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	Request_RenameModal = true;
}


void Viewer::OnDelete()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	Request_DeleteFileModal = true;
}


void Viewer::OnDeletePermanent()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	Request_DeleteFileNoRecycleModal = true;
}


void Viewer::OnSave()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	if (CurrImage->IsLoaded())
		Request_SaveCurrentModal = true;
}


void Viewer::OnSaveAs()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	if (CurrImage->IsLoaded())
		Request_SaveAsModal = true;
}


void Viewer::OnSaveAll()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	Request_SaveAllModal = true;
}


void Viewer::OnSaveContactSheet()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	if (Images.GetNumItems() > 1)
		Request_ContactSheetModal = true;
}


void Viewer::OnSaveMultiFrameImage()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	if (Images.GetNumItems() > 1)
		Request_MultiFrameModal = true;
}


void Viewer::OnSaveExtractFrames()
{
	if (!CurrImage)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowImportRaw)
		return;

	if (CurrImage->GetNumFrames() > 1)
		Request_ExtractFramesModal = true;
}


void Viewer::OnMenuBar()			{ Config::ProfileData& profile = Config::GetProfileData(); if (!CropMode) profile.ShowMenuBar = !profile.ShowMenuBar; }
void Viewer::OnNavBar()				{ Config::ProfileData& profile = Config::GetProfileData(); if (!CropMode) profile.ShowNavBar = !profile.ShowNavBar; }
void Viewer::OnThumbnails()			{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowThumbnailView = !profile.ShowThumbnailView; }


void Viewer::OnFileBrowser()
{
	#ifdef PACKAGE_SNAP
	static int messageCount = 2;
	if (messageCount-- > 0)
		Request_SnapMessage_NoFileBrowse = true;
	#else
	if (CurrImage)
		tSystem::tOpenSystemFileExplorer(CurrImage->Filename);
	#endif
}


void Viewer::OnSlideshowTimer()		{ Config::ProfileData& profile = Config::GetProfileData(); profile.SlideshowProgressArc = !profile.SlideshowProgressArc; }
void Viewer::OnSlideshowReshuffle()	{ Config::ProfileData& profile = Config::GetProfileData(); profile.SlideshowAutoReshuffle = !profile.SlideshowAutoReshuffle; }
void Viewer::OnCheatSheet()			{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowCheatSheet = !profile.ShowCheatSheet; }
void Viewer::OnDebugLog()			{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowOutputLog = !profile.ShowOutputLog; }
void Viewer::OnProfileMain()		{ if (!CropMode && (Config::GetProfile() != Profile::Main)) ChangProfile(Profile::Main); }
void Viewer::OnProfileBasic()		{ if (!CropMode && (Config::GetProfile() != Profile::Basic)) ChangProfile(Profile::Basic); }
void Viewer::OnProfileKiosk()		{ if (!CropMode && (Config::GetProfile() != Profile::Kiosk)) ChangProfile(Profile::Kiosk); }
void Viewer::OnProfileAlt()			{ if (!CropMode && (Config::GetProfile() != Profile::Alt)) ChangProfile(Profile::Alt); }
void Viewer::OnPreferences()		{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowPreferences = !profile.ShowPreferences; }
void Viewer::OnKeyBindings()		{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowBindingsWindow = !profile.ShowBindingsWindow; if (profile.ShowBindingsWindow) BindingsWindowJustOpened = true; }
void Viewer::OnChangeScreenMode()	{ Config::ProfileData& profile = Config::GetProfileData(); ChangeScreenMode(!profile.FullscreenMode); }


void Viewer::OnEscape()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.FullscreenMode)
		ChangeScreenMode(false);
	else if ((Config::GetProfile() == Profile::Basic) || (Config::GetProfile() == Profile::Kiosk))
		ChangProfile(Profile::Main);
}


void Viewer::OnEscapeWithQuit()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.FullscreenMode)
		ChangeScreenMode(false);
	else if ((Config::GetProfile() == Profile::Basic) || (Config::GetProfile() == Profile::Kiosk))
		ChangProfile(Profile::Main);
	else
		Viewer::Request_Quit = true;
}


void Viewer::OnQuit()				{ Request_Quit = true; }
void Viewer::OnOpenFile()			{ Request_OpenFileModal = true; }
void Viewer::OnOpenDir()			{ Request_OpenDirModal = true; }
void Viewer::OnMetaData()			{ Config::ProfileData& profile = Config::GetProfileData(); profile.ShowImageMetaData = !profile.ShowImageMetaData; }


void Viewer::ResetPan(bool resetX, bool resetY)
{
	if (resetX)
	{
		PanOffsetX = 0;
		PanDragDownOffsetX = 0;
	}

	if (resetY)
	{
		PanOffsetY = 0;
		PanDragDownOffsetY = 0;
	}
}


int Viewer::GetPanX()
{
	return PanOffsetX+PanDragDownOffsetX;
}


int Viewer::GetPanY()
{
	return PanOffsetY+PanDragDownOffsetY;
}


void Viewer::DrawBackground(float l, float r, float b, float t, float drawW, float drawH)
{
	if (Config::Global.TransparentWorkArea)
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	Config::ProfileData::BackgroundStyleEnum style = profile.GetBackgroundStyle();
	bool overrideBG = CurrImage && CurrImage->OverrideBackgroundColour;
	if (overrideBG)
		style = Config::ProfileData::BackgroundStyleEnum::SolidColour;

	switch (style)
	{
		case Config::ProfileData::BackgroundStyleEnum::None:
			return;

		case Config::ProfileData::BackgroundStyleEnum::Checkerboard:
		{
			// Semitransparent checkerboard background.
			float checkSize = float(profile.BackgroundCheckerboxSize);

			// This is for efficiency. Why draw checrboxes where we don'e have to (off screen)?
			// We cull in widths of 2*checkSize so the alternating checherbox colour works out.
			if (l < 0.0f)
			{
				int numXhidden = int((0.0f-l) / (2.0f*checkSize));
				l += float(numXhidden)*(2.0f*checkSize);
			}
			if (r >= drawW)
			{
				int numXhidden = int((r-drawW) / (2.0f*checkSize));
				r -= float(numXhidden)*(2.0f*checkSize);
			}

			if (b < 0.0f)
			{
				int numYhidden = int((0.0f-b) / (2.0f*checkSize));
				b += float(numYhidden)*(2.0f*checkSize);
			}
			if (t >= drawH)
			{
				int numYhidden = int((t-drawH) / (2.0f*checkSize));
				t -= float(numYhidden)*(2.0f*checkSize);
			}

			int x = 0;
			int y = 0;

			bool lineStartToggle = false;
			float bgH = t - b;
			float bgW = r - l;
			glBegin(GL_QUADS);

			int numCheckQuads = 0;
			while (y*checkSize < bgH)
			{
				bool colourToggle = lineStartToggle;

				while (x*checkSize < bgW)
				{
					if (colourToggle)
						glColor4f(0.3f, 0.3f, 0.35f, 1.0f);
					else
						glColor4f(0.4f, 0.4f, 0.45f, 1.0f);

					colourToggle = !colourToggle;

					float cw = checkSize;
					if ((x+1)*checkSize > bgW)
						cw -= (x+1)*checkSize - bgW;

					float ch = checkSize;
					if ((y+1)*checkSize > bgH)
						ch -= (y+1)*checkSize - bgH;

					float lc = (l+x*checkSize);
					float rc = (l+x*checkSize+cw);
					float bc = (b+y*checkSize);
					float tc = (b+y*checkSize+ch);

					glVertex2f(lc, bc);
					glVertex2f(lc, tc);
					glVertex2f(rc, tc);
					glVertex2f(rc, bc);
					numCheckQuads++;

					x++;
				}
				x = 0;
				y++;
				lineStartToggle = !lineStartToggle;
			}
			glEnd();
			break;
		}

		case Config::ProfileData::BackgroundStyleEnum::SolidColour:
		{
			tColour4b bgCol = profile.BackgroundColour;
			if (overrideBG)
				bgCol = CurrImage->BackgroundColourOverride;
			glColor4ubv(bgCol.E);
			glBegin(GL_QUADS);
			glVertex2f(l, b);
			glVertex2f(l, t);
			glVertex2f(r, t);
			glVertex2f(r, b);
			glEnd();
			break;
		}
	}
}


bool Viewer::ConvertScreenPosToImagePos
(
	int& imgX, int& imgY, const tVector2& scrPos,
	const tVector4& lrtb, const tVector2& uvOff
)
{
	Config::ProfileData& profile = Config::GetProfileData();

	float picX = scrPos.x - lrtb.L;
	float picY = (scrPos.y) - lrtb.B;
	float normX = picX / (lrtb.R-lrtb.L);
	float normY = picY / (lrtb.T-lrtb.B);
	if (profile.Tile)
	{
		normX = tMath::tMod(normX, 1.0f);
		if (normX < 0.0f) normX += 1.0f;

		normY = tMath::tMod(normY, 1.0f);
		if (normY < 0.0f) normY += 1.0f;
	}

	int imagewi = CurrImage->GetWidth();
	int imagehi = CurrImage->GetHeight();
	float imagew = float(imagewi);
	float imageh = float(imagehi);

	float imposX = imagew * tMath::tLilo(normX, 0.0f + uvOff.u, 1.0f + uvOff.u);
	float imposY = imageh * tMath::tLilo(normY, 0.0f + uvOff.v, 1.0f + uvOff.v);

	imgX = int(imposX);
	imgY = int(imposY);
	bool clamped = false;
	if (!profile.Tile)
	{
		if (((imgX < 0) || (imgX >= imagewi)) || ((imgY < 0) || (imgY >= imagehi)))
			clamped = true;

		tMath::tiClamp(imgX, 0, imagewi - 1);
		tMath::tiClamp(imgY, 0, imagehi - 1);
	}
	else
	{
		imgX %= imagewi;
		if (imgX < 0) imgX += imagewi;
		imgY %= imagehi;
		if (imgY < 0) imgY += imagehi;
	}

	return clamped;
}


void Viewer::ConvertImagePosToScreenPos
(
	tVector2& scrPos,
	int imposX, int imposY, const tVector4& lrtb,
	const tVector2& uvOff, bool centerPixel
)
{
	tMath::tiClamp(imposX, 0, CurrImage->GetWidth());
	tMath::tiClamp(imposY, 0, CurrImage->GetHeight());
	float imgX = float(imposX);
	float imgY = float(imposY);

	float imagew = float(CurrImage->GetWidth());
	float imageh = float(CurrImage->GetHeight());

	float umin = 0.0f + uvOff.u;
	float umax = 1.0f + uvOff.u;
	float u = (imgX/imagew - umin) / (umax-umin);

	float vmin = 0.0f + uvOff.v;
	float vmax = 1.0f + uvOff.v;
	float v = (imgY/imageh - vmin) / (vmax-vmin);

	float picX = u * (lrtb.R-lrtb.L);
	float picY = v * (lrtb.T-lrtb.B);

	scrPos.x = tCeiling(picX + lrtb.L);
	scrPos.y = tCeiling(picY + lrtb.B);

	if (centerPixel)
	{
		scrPos.x += 0.5f*(lrtb.R-lrtb.L)/imagew;
		scrPos.y += 0.5f*(lrtb.T-lrtb.B)/imageh;
	}
}


int Viewer::DoMainMenuBar()
{
	const ImGuiStyle& style = ImGui::GetStyle();

	// File requests.
	bool openFilePressed			= Request_OpenFileModal;			Request_OpenFileModal				= false;
	bool openDirPressed				= Request_OpenDirModal;				Request_OpenDirModal				= false;
	bool saveCurrentPressed			= Request_SaveCurrentModal;			Request_SaveCurrentModal			= false;
	bool saveAsPressed				= Request_SaveAsModal;				Request_SaveAsModal					= false;
	bool saveAllPressed				= Request_SaveAllModal;				Request_SaveAllModal				= false;
	bool saveContactSheetPressed	= Request_ContactSheetModal;		Request_ContactSheetModal			= false;
	bool saveMultiFramePressed		= Request_MultiFrameModal;			Request_MultiFrameModal				= false;
	bool saveExtractFramesPressed	= Request_ExtractFramesModal;		Request_ExtractFramesModal			= false;
	bool snapMessageNoFileBrowse	= Request_SnapMessage_NoFileBrowse;	Request_SnapMessage_NoFileBrowse	= false;
	bool snapMessageNoFrameTrans	= Request_SnapMessage_NoFrameTrans;	Request_SnapMessage_NoFrameTrans	= false;
	bool deleteFilePressed			= Request_DeleteFileModal;			Request_DeleteFileModal				= false;
	bool deleteFileNoRecycPressed	= Request_DeleteFileNoRecycleModal;	Request_DeleteFileNoRecycleModal	= false;
	bool renamePressed				= Request_RenameModal;				Request_RenameModal					= false;

	// Edit requests.
	bool resizeImagePressed			= Request_ResizeImageModal;			Request_ResizeImageModal			= false;
	bool resizeCanvasPressed		= Request_ResizeCanvasModal;		Request_ResizeCanvasModal			= false;
	bool rotateImagePressed			= Request_RotateImageModal;			Request_RotateImageModal			= false;
	bool levelsPressed				= Request_LevelsModal;				Request_LevelsModal					= false;
	bool quantizePressed			= Request_QuantizeModal;			Request_QuantizeModal				= false;
	LosslessTransformMode
	losslessTransformPressed		= Request_LosslessTrnsModal;		Request_LosslessTrnsModal			= LosslessTransformMode::None;

	int menuBarHeight = 0;
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.ShowMenuBar)
	{
		ImGui::SetNextWindowPos(tVector2(0.0f, 0.0f));
		ImGui::BeginMainMenuBar();

		tVector2 colourButtonSize	= Gutil::GetUIParamExtent(tVector2(26.0f, 26.0f), tVector2(64.0f, 64.0f));
		tVector2 toolImageSize		= Gutil::GetUIParamExtent(tVector2(24.0f, 24.0f), tVector2(62.0f, 62.0f));
		float zoomComboWidth		= Gutil::GetUIParamScaled(70.0f, 2.5f);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window)
			window->MenuBarImageButtonHeight = toolImageSize.y;

		Config::ProfileData::ZoomModeEnum zoomMode = GetZoomMode();

		//
		// File Menu.
		//
		if (ImGui::BeginMenu("File"))
		{
			// Show file menu items...
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));	// Push F
			bool imgAvail = CurrImage && CurrImage->IsLoaded();

			tString openFileKey = profile.InputBindings.FindModKeyText(Bindings::Operation::OpenFile);
			if (ImGui::MenuItem("Open File...", openFileKey.Chz()))
				openFilePressed = true;

			tString openDirKey = profile.InputBindings.FindModKeyText(Bindings::Operation::OpenDir);
			if (ImGui::MenuItem("Open Dir...", openDirKey.Chz()))
				openDirPressed = true;

			ImGui::Separator();

			tString saveKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Save);
			bool saveEnabled = CurrImage ? true : false;
			if (ImGui::MenuItem("Save...", saveKey.Chz(), false, saveEnabled))
				saveCurrentPressed = true;

			tString saveAsKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SaveAs);
			bool saveAsEnabled = CurrImage ? true : false;
			if (ImGui::MenuItem("Save As...", saveAsKey.Chz(), false, saveAsEnabled))
				saveAsPressed = true;

			tString saveAllKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SaveAll);
			bool saveAllEnabled = CurrImage ? true : false;
			if (ImGui::MenuItem("Save All...", saveAllKey.Chz(), false, saveAllEnabled))
				saveAllPressed = true;

			tString saveCSKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SaveContactSheet);
			bool saveCSEnabled = (Images.GetNumItems() > 1);
			if (ImGui::MenuItem("Save Contact Sheet...", saveCSKey.Chz(), false, saveCSEnabled))
				saveContactSheetPressed = true;

			tString saveMultiKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SaveMultiFrameImage);
			bool saveMultiEnabled = (Images.GetNumItems() > 1);
			if (ImGui::MenuItem("Save Multi-Frame...", saveMultiKey.Chz(), false, saveMultiEnabled))
				saveMultiFramePressed = true;

			tString saveFramesKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SaveExtractFrames);
			bool saveFramesEnabled = CurrImage && (CurrImage->GetNumFrames() > 1);
			if (ImGui::MenuItem("Save Extract Frames...", saveFramesKey.Chz(), false, saveFramesEnabled))
				saveExtractFramesPressed = true;

			ImGui::Separator();

			tString copyKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Copy);
			if (ImGui::MenuItem("Copy Image", copyKey.Chz(), false, imgAvail))
				Viewer::OnCopyToClipboard();

			tString pasteKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Paste);
			if (ImGui::MenuItem("Paste Image", pasteKey.Chz()))
				Viewer::OnPasteFromClipboard();

			tString importRawKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ImportRaw);
			if (ImGui::MenuItem("Import Raw...", importRawKey.Chz(), &profile.ShowImportRaw))
			{
				if (profile.ShowImportRaw)
					ImportRawWindowJustOpened = true;
				else
					ImportRawWindowJustClosed = true;
			}

			ImGui::Separator();

			// For all three profile menu items, changing to the currently activated is checked by ChangProfile
			// and does nothing. It is safe.
			bool mainProfile = Config::GetProfile() == Profile::Main;
			tString mainProfKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ProfileMain);
			if (ImGui::MenuItem("Main Profile", mainProfKey.Chz(), &mainProfile))
				ChangProfile(Profile::Main);

			bool basicProfile = Config::GetProfile() == Profile::Basic;
			tString basicProfKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ProfileBasic);
			if (ImGui::MenuItem("Basic Profile", basicProfKey.Chz(), &basicProfile))
				ChangProfile(Profile::Basic);

			bool kioskProfile = Config::GetProfile() == Profile::Kiosk;
			tString kioskProfKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ProfileKiosk);
			if (ImGui::MenuItem("Kiosk Profile", kioskProfKey.Chz(), &kioskProfile))
				ChangProfile(Profile::Kiosk);

			bool altProfile = Config::GetProfile() == Profile::Alt;
			tString altProfKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ProfileAlt);
			if (ImGui::MenuItem("Alt Profile", altProfKey.Chz(), &altProfile))
				ChangProfile(Profile::Alt);

			ImGui::Separator();

			tString bindingsKey = profile.InputBindings.FindModKeyText(Bindings::Operation::KeyBindings);
			if (ImGui::MenuItem("Key Bindings...", bindingsKey.Chz(), &profile.ShowBindingsWindow))
				if (profile.ShowBindingsWindow)
					BindingsWindowJustOpened = true;

			tString prefsKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Preferences);
			ImGui::MenuItem("Preferences...", prefsKey.Chz(), &profile.ShowPreferences);

			tString quitKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Quit);
			if (ImGui::MenuItem("Quit", quitKey.Chz()))
				glfwSetWindowShouldClose(Window, 1);

			ImGui::PopStyleVar();		// Pop F
			ImGui::EndMenu();
		}

		//
		// Edit Menu.
		//
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));		// Push G

			bool undoEnabled = CurrImage && CurrImage->IsUndoAvailable();
			tString undoDesc = undoEnabled ? CurrImage->GetUndoDesc() : tString();
			tString undoStr; tsPrintf(undoStr, "Undo %s", undoDesc.Chr());
			tString undoKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Undo);
			if (ImGui::MenuItem(undoStr.Chr(), undoKey.Chz(), false, undoEnabled))
				OnUndo();

			bool redoEnabled = CurrImage && CurrImage->IsRedoAvailable();
			tString redoDesc = redoEnabled ? CurrImage->GetRedoDesc() : tString();
			tString redoStr; tsPrintf(redoStr, "Redo %s", redoDesc.Chr());
			tString redoKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Redo);
			if (ImGui::MenuItem(redoStr.Chr(), redoKey.Chz(), false, redoEnabled))
				OnRedo();

			ImGui::Separator();

			bool imgAvail = CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled();
			tString flipVKey = profile.InputBindings.FindModKeyText(Bindings::Operation::FlipVertically);
			if (ImGui::MenuItem("Flip Vertically", flipVKey.Chz(), false, imgAvail ))
				OnFlipVert(true);

			tString flipHKey = profile.InputBindings.FindModKeyText(Bindings::Operation::FlipHorizontally);
			if (ImGui::MenuItem("Flip Horizontally", flipHKey.Chz(), false, imgAvail))
				OnFlipVert(false);

			tString rotACWKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Rotate90Anticlockwise);
			if (ImGui::MenuItem("Rotate Anti-Clockwise", rotACWKey.Chz(), false, imgAvail))
			{
				if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
				{
					Request_LosslessTrnsModal = LosslessTransformMode::Rot90ACW;
				}
				else
				{
					CurrImage->Unbind();
					CurrImage->Rotate90(true);
					CurrImage->Bind();
					Gutil::SetWindowTitle();
				}
			}

			tString rotCWKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Rotate90Clockwise);
			if (ImGui::MenuItem("Rotate Clockwise", rotCWKey.Chz(), false, imgAvail))
			{
				if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
				{
					Request_LosslessTrnsModal = LosslessTransformMode::Rot90CW;
				}
				else
				{
					CurrImage->Unbind();
					CurrImage->Rotate90(false);
					CurrImage->Bind();
					Gutil::SetWindowTitle();
				}
			}

			tString rotateImgKey = profile.InputBindings.FindModKeyText(Bindings::Operation::RotateImage);
			if (ImGui::MenuItem("Rotate Image...", rotateImgKey.Chz(), false, imgAvail))
				rotateImagePressed = true;

			ImGui::Separator();

			tString cropKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Crop);
			if (ImGui::MenuItem("Crop...", cropKey.Chz(), &CropMode, imgAvail))
			{
				if (CropMode)
					profile.Tile = false;
			}

			tString resizeImgKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ResizeImage);
			if (ImGui::MenuItem("Resize Image...", resizeImgKey.Chz(), false, imgAvail))
				resizeImagePressed = true;

			tString resizeCanvasKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ResizeCanvas);
			if (ImGui::MenuItem("Resize Canvas...", resizeCanvasKey.Chz(), false, imgAvail))
				resizeCanvasPressed = true;

			ImGui::Separator();

			tString editPixelKey = profile.InputBindings.FindModKeyText(Bindings::Operation::PixelEdit);
			ImGui::MenuItem("Edit Pixel", editPixelKey.Chz(), &profile.ShowPixelEditor, imgAvail);

			tString chanFiltKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ChannelFilter);
			ImGui::MenuItem("Channel Filter...", chanFiltKey.Chz(), &profile.ShowChannelFilter);

			tString levelsKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Levels);
			if (ImGui::MenuItem("Levels...", levelsKey.Chz(), false, imgAvail))
				levelsPressed = true;

			tString quantizeKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Quantize);
			if (ImGui::MenuItem("Quantize...", quantizeKey.Chz(), false, imgAvail))
				quantizePressed = true;

			tString propsKey = profile.InputBindings.FindModKeyText(Bindings::Operation::PropertyEdit);
			ImGui::MenuItem("Image Properties...", propsKey.Chz(), &profile.ShowPropsWindow);

			ImGui::PopStyleVar();		// Pop G
			ImGui::EndMenu();
		}

		//
		// View Menu.
		//
		if (ImGui::BeginMenu("View"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));	// Push H

			tString menuBarKey = profile.InputBindings.FindModKeyText(Bindings::Operation::MenuBar);
			ImGui::MenuItem("Menu Bar", menuBarKey.Chz(), &profile.ShowMenuBar, !CropMode);

			tString navBarKey = profile.InputBindings.FindModKeyText(Bindings::Operation::NavBar);
			ImGui::MenuItem("Nav Bar", navBarKey.Chz(), &profile.ShowNavBar, !CropMode);

			tString slideKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SlideshowTimer);
			ImGui::MenuItem("Slideshow Progress", slideKey.Chz(), &profile.SlideshowProgressArc, !CropMode);

			tString reshuffleKey = profile.InputBindings.FindModKeyText(Bindings::Operation::SlideshowReshuffle);
			ImGui::MenuItem("Slideshow Reshuffle", reshuffleKey.Chz(), &profile.SlideshowAutoReshuffle, !CropMode);

			tString detailsKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Details);
			ImGui::MenuItem("Image Details", detailsKey.Chz(), &profile.ShowImageDetails);

			tString metaDataKey = profile.InputBindings.FindModKeyText(Bindings::Operation::MetaData);
			ImGui::MenuItem("Image Meta Data", metaDataKey.Chz(), &profile.ShowImageMetaData);

			tString thumbKey = profile.InputBindings.FindModKeyText(Bindings::Operation::Thumbnails);
			ImGui::MenuItem("Thumbnail View", thumbKey.Chz(), &profile.ShowThumbnailView);

			ImGui::Separator();

			bool userMode = (zoomMode == Config::ProfileData::ZoomModeEnum::User);
			if (ImGui::MenuItem("Zoom User", nullptr, &userMode))
			{
				ResetPan();
				SetZoomMode(Config::ProfileData::ZoomModeEnum::User);
			}

			bool fitMode = (zoomMode == Config::ProfileData::ZoomModeEnum::Fit);
			tString zoomFitKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ZoomFit);
			if (ImGui::MenuItem("Zoom Fit", zoomFitKey.Chz(), &fitMode))
			{
				ResetPan();
				SetZoomMode(Config::ProfileData::ZoomModeEnum::Fit);
			}

			bool downscale = (zoomMode == Config::ProfileData::ZoomModeEnum::DownscaleOnly);
			tString zoomDSKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ZoomDownscaleOnly);
			if (ImGui::MenuItem("Zoom Downscale", zoomDSKey.Chz(), &downscale))
			{
				ResetPan();
				SetZoomMode(Config::ProfileData::ZoomModeEnum::DownscaleOnly);
			}

			bool oneToOne = (zoomMode == Config::ProfileData::ZoomModeEnum::OneToOne);
			tString zoomOneKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ZoomOneToOne);
			if (ImGui::MenuItem("Zoom 1:1", zoomOneKey.Chz(), &oneToOne))
			{
				SetZoomPercent(100.0f);
				ResetPan();
				SetZoomMode(Config::ProfileData::ZoomModeEnum::OneToOne);
			}

			const char* zoomItems[] = { "Zoom", "20%", "50%", "100%", "150%", "200%", "400%", "800%", "1200%", "1800%", "2500%"};
			float zoomVals[] = { -1.0f, 20.0f, 50.0f, 100.0f, 150.0f, 200.0f, 400.0f, 800.0f, 1200.0f, 1800.0f, 2500.0f };
			float zoomPercent = GetZoomPercent();
			tString currZoomStr;
			tsPrintf(currZoomStr, "%0.0f%%", zoomPercent);

			// We use the zoom combo only as a chooser. It always displays index 0 (Zoom) when not expanded.
			int zoomIdx = 0;
			ImGui::SetNextItemWidth(zoomComboWidth);
			if (ImGui::Combo(currZoomStr.Chr(), &zoomIdx, zoomItems, tNumElements(zoomItems)) && (zoomIdx > 0) && CurrImage && CurrImage->IsLoaded())
				ApplyZoomDelta(zoomVals[zoomIdx]-zoomPercent);

			tString zoomPerImageKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ZoomPerImage);
			ImGui::MenuItem("Zoom Per Image", zoomPerImageKey.Chz(), &profile.ZoomPerImage);

			tString panKey = profile.InputBindings.FindModKeyText(Bindings::Operation::ResetPan);
			if (ImGui::MenuItem("Reset Pan", panKey.Chz()))
				ResetPan();

			ImGui::PopStyleVar();		// Pop H
			ImGui::EndMenu();
		}

		//
		// Help Menu.
		//
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));		// Push I

			tString cheatKey = profile.InputBindings.FindModKeyText(Bindings::Operation::CheatSheet);
			ImGui::MenuItem("Cheat Sheet", cheatKey.Chz(), &profile.ShowCheatSheet);

			ImGui::MenuItem("About", nullptr, &profile.ShowAbout);

			ImGui::PopStyleVar();														// Pop I
			ImGui::EndMenu();
		}

		//
		// Toolbar.
		//
		bool imgAvail = CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled();

		// Colour Swatch.
		tColour4f floatCol(PixelColour);
		tVector4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, colourButtonSize))
			ImGui::OpenPopup("CopyColourAs");
		if (ImGui::BeginPopup("CopyColourAs"))
			ColourCopyAs();

		// Colour Channel Filter.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_ChannelFilter.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowChannelFilter ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	profile.ShowChannelFilter = !profile.ShowChannelFilter;
		Gutil::ToolTip("Colour Channel Filter");

		// Image Levels.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Levels.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
		{
			Request_LevelsModal = true;
		}
		Gutil::ToolTip("Image Levels");

		// Thumbnails View.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_ContentView.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowThumbnailView ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	profile.ShowThumbnailView = !profile.ShowThumbnailView;
		Gutil::ToolTip("Content Thumbnail View");

		// Tiled.
		bool tileAvail = (CurrImage && CurrImage->IsLoaded()) ? !CropMode : false;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Tile.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.Tile ? ColourPressedBG : ColourBG, tileAvail ? ColourEnabledTint : ColourDisabledTint) && tileAvail
		)
		{
			profile.Tile = !profile.Tile;
			if (!profile.Tile)
				ResetPan();
		}
		Gutil::ToolTip("Show Images Tiled");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		// Vertical Flip.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_FlipV.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)	OnFlipVert(true);
		Gutil::ToolTip("Flip Vertically");

		// Horizontal Flip.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_FlipH.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)	OnFlipVert(false);
		Gutil::ToolTip("Flip Horizontally");

		// Rotate Anticlockwise.
		ImGui::PushID("ToolRotACW");
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_RotCW_RotACW.Bind()), toolImageSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)	OnRotate90(false);
		ImGui::PopID();
		Gutil::ToolTip("Rotate 90 Anticlockwise");

		// Rotate clockwise.
		ImGui::PushID("ToolRotCW");
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_RotCW_RotACW.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)	OnRotate90(true);
		ImGui::PopID();
		Gutil::ToolTip("Rotate 90 Clockwise");

		// Rotate.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_RotateTheta.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)	OnRotate();
		Gutil::ToolTip("Rotate Theta");

		// Crop.
		bool cropAvail = CropMode || (CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled());
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Crop.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			CropMode ? ColourPressedBG : ColourBG, cropAvail ? ColourEnabledTint : ColourDisabledTint) && cropAvail
		)	OnCrop();
		Gutil::ToolTip("Crop");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		// Properties.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_PropEdit.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowPropsWindow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	OnPropertyEdit();
		Gutil::ToolTip("Image Properties");

		// Details.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_InfoOverlay.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowImageDetails ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	OnDetails();
		Gutil::ToolTip("Image Details Overlay");

		// Meta Data.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_MetaData.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowImageMetaData ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	OnMetaData();
		Gutil::ToolTip("Image Meta-Data Overlay");

		// Refresh File.
		bool refreshAvail = CurrImage ? true : false;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Refresh.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			ColourBG, refreshAvail ? ColourEnabledTint : ColourDisabledTint) && refreshAvail
		)	OnRefresh();
		Gutil::ToolTip("Refresh/Reload Current File");

		// Delete File.
		bool recycleAvail = CurrImage ? true : false;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Recycle.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			ColourBG, recycleAvail ? ColourEnabledTint : ColourDisabledTint) && recycleAvail
		)	OnDelete();
		Gutil::ToolTip("Delete Current File");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		// Help.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Help.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowCheatSheet ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	OnCheatSheet();
		Gutil::ToolTip("Help");

		// Preferences.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Prefs.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			profile.ShowPreferences ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	OnPreferences();
		Gutil::ToolTip("Preferences");

		menuBarHeight = int(ImGui::GetWindowSize().y);
		ImGui::EndMainMenuBar();
	}

	// Process any actions. Either from keyboard requests or from the menubar. The ones here are for ImGui windows
	// that require an update every frame. This could be reorganized so that when the operation is executed (in the
	// big keybindings switch) the call to open the popup is performed, but we'd still need the dialog updates here,
	// so this gives better encapsulation.
	//
	// We deal with all the modals in a single place. Modals are slightly special as we allow keyboard nav during
	// modals but not otherwise as it would interfere with the viewer's keyboard bindings. We start setting nav off.
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = 0;

	//
	// File requests.
	//
	DoOpenFileModal					(openFilePressed);
	DoOpenDirModal					(openDirPressed);

	DoSaveCurrentModal				(saveCurrentPressed);
	DoSaveAsModal					(saveAsPressed);

	DoSaveAllModal					(saveAllPressed);
	DoSaveContactSheetModal			(saveContactSheetPressed);
	DoSaveMultiFrameModal			(saveMultiFramePressed);
	DoSaveExtractFramesModal		(saveExtractFramesPressed);
	DoSnapMessageNoFileBrowseModal	(snapMessageNoFileBrowse);
	DoSnapMessageNoFrameTransModal	(snapMessageNoFrameTrans);

	DoDeleteFileModal				(deleteFilePressed);
	DoDeleteFileNoRecycleModal		(deleteFileNoRecycPressed);
	DoRenameModal					(renamePressed);

	//
	// Edit requests.
	//
	DoResizeImageModal				(resizeImagePressed);
	DoResizeCanvasModal				(resizeCanvasPressed);
	DoRotateImageModal				(rotateImagePressed);
	DoLevelsModal					(levelsPressed);
	DoQuantizeModal					(quantizePressed);
	DoLosslessTransformModal		(losslessTransformPressed);

	return menuBarHeight;
}


int Viewer::GetNavBarHeight()
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (!profile.ShowNavBar)
		return 0;

	int barHeight = Gutil::GetUIParamExtent(30, 72);
	return barHeight;
}


void Viewer::DoNavBar(int dispw, int disph, int barHeight)
{
	if (barHeight <= 0)
		return;

	float x = 0.0f;
	float y = float(disph - barHeight);
	float w = float(dispw);
	float h = float(barHeight);

	ImGui::SetNextWindowSize(tVector2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(tVector2(x, y), ImGuiCond_Always);
	float navVPad = Gutil::GetUIParamExtent(2.0f, 5.0f);
	float navHPad = Gutil::GetUIParamExtent(8.0f, 14.0f);

	// Push A
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(navHPad, navVPad));

	int flags =
		ImGuiWindowFlags_NoResize	| ImGuiWindowFlags_NoMove		| ImGuiWindowFlags_NoCollapse	|
		ImGuiWindowFlags_NoTitleBar	| ImGuiWindowFlags_NoScrollbar	;

	ImGui::Begin("NavBar", nullptr, flags);
	tVector2 upDirButtonSize = Gutil::GetUIParamScaled(tVector2(25.0f, 25.0f), 2.5f);
	if
	(
		ImGui::ImageButton
		(
			ImTextureID(Image_UpFolder.Bind()), upDirButtonSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f),
			1, Viewer::ColourBG, tVector4::one
		)
	)
	{
		tString upDir = tSystem::tGetUpDir(ImagesDir);

		#ifdef PLATFORM_WINDOWS
		// Windows has drive letters at the root. By checking if the upDir did not change we can
		// allow "/" to represent the root before all drive letters.
		if (upDir == ImagesDir)
			upDir = "/";
		#endif

		if (!upDir.IsEmpty())
		{
			ImageToLoad = upDir + "dummyfile.txt";
			PopulateImages();
			SetCurrentImage();
			Gutil::SetWindowTitle();
		}
	}

	float textYOffset = Gutil::GetUIParamExtent(6.0f, 15.0f);
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textYOffset);
	float textYPos = ImGui::GetCursorPosY();
	ImGui::Text("%s", ImagesDir.Chr());
	ImGui::SameLine();
	float navRight = ImGui::GetCursorPosX();

	if (ImagesSubDirs.NumItems() > 0)
	{
		float comboSize = Gutil::GetUIParamExtent(27.0f, 64.0f);
		if (ImGui::BeginCombo("##navcombo", nullptr, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoPreview, comboSize))
		{
			for (tStringItem* subDir = ImagesSubDirs.First(); subDir; subDir = subDir->Next())
			{
				bool isSelected = false;
				if (ImGui::Selectable(subDir->Chr(), isSelected))
				{
					// Selection made. This only runs once.
					#ifdef PLATFORM_WINDOWS
					if (ImagesDir == "/")
						ImagesDir.Clear();
					#endif
					ImageToLoad = ImagesDir + *subDir + "/" + "dummyfile.txt";
					PopulateImages();
					SetCurrentImage();
					Gutil::SetWindowTitle();
					break;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		textYPos = navVPad + Gutil::GetUIParamExtent(3.4f, 8.0f);
		navRight = ImGui::GetCursorPosX();
	}

	// Display the filename on the right.
	Config::ProfileData& profile = Config::GetProfileData();
	if
	(
		CurrImage && CurrImage->Filename.IsValid() &&
		(profile.ShowNavFilenameAlways || profile.FullscreenMode)
	)
	{
		tString fname = tGetFileName(CurrImage->Filename);
		tVector2 textSize = ImGui::CalcTextSize(fname.Chr());
		float textStartX = w - textSize.x - navHPad;
		
		// Only display if it's not going to overlap with the dir navigation on the left.
		if (textStartX > navRight)
		{
			ImGui::SetCursorPosY(textYPos);
			ImGui::SetCursorPosX(textStartX);
			ImGui::Text(fname.Chr());
		}
	}

	ImGui::End();

	// Pop A
	ImGui::PopStyleVar(3);
}


void Viewer::UpdateDesiredUISize()
{
	Config::ProfileData& profile = *Viewer::Config::Current;
	Config::ProfileData::UISizeEnum profSize = profile.GetUISize();
	switch (profSize)
	{
		case Config::ProfileData::UISizeEnum::Invalid:
			DesiredUISize = Config::ProfileData::UISizeEnum::Tiny;
			return;

		case Config::ProfileData::UISizeEnum::Auto:
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			if (!monitor)
			{
				DesiredUISize = Config::ProfileData::UISizeEnum::Tiny;
				return;
			}

			float xscale = 0.0f; float yscale = 0.0f;
			glfwGetMonitorContentScale(monitor, &xscale, &yscale);
			if ((xscale <= 0.0f) || (yscale <= 0.0f) || !tApproxEqual(xscale, yscale, 0.001f))
				DesiredUISize = Config::ProfileData::UISizeEnum::Tiny;
			else if (xscale < (0.75f + 1.00f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Nano;
			else if (xscale < (1.00f + 1.25f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Tiny;
			else if (xscale < (1.25f + 1.50f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Small;
			else if (xscale < (1.50f + 1.75f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Moderate;
			else if (xscale < (1.75f + 2.00f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Medium;
			else if (xscale < (2.00f + 2.25f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Large;
			else if (xscale < (2.25f + 2.50f)/2.0f)
				DesiredUISize = Config::ProfileData::UISizeEnum::Huge;
			else
				DesiredUISize = Config::ProfileData::UISizeEnum::Massive;

			return;
		}

		default:
			DesiredUISize = profSize;
			return;
	}
}


void Viewer::SetUISize(Viewer::Config::ProfileData::UISizeEnum uiSize)
{
	tAssert(uiSize >= Config::ProfileData::UISizeEnum::Smallest);
	tAssert(uiSize <= Config::ProfileData::UISizeEnum::Largest);
	int uiSizeInt = int(uiSize);
	ImGuiIO& io = ImGui::GetIO();

	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;
	builder.AddChar(0x2026);                               // Adds ellipsis.
	builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic()); // Default plus Cyrillic.
	builder.BuildRanges(&ranges);

	// The first step is to make sure ImGui has the font added. FontUISizeAdded stores either Invalid
	// meaning no fonts have been added, ALL meaning all font sizes (for all UISizes) have been added,
	// or a particular IU size meaning ONLY that size has been added. If it's invalid, we start by ONLY
	// adding a single font. This speeds things up if no further size changes are made. Otherwise, if
	// we don't have the font size we need, we load them all.
	if (FontUISizeAdded == Config::ProfileData::UISizeEnum::Invalid)
	{
		// Calling destroy is safe if no texture is currently bound.
		ImGui_ImplOpenGL2_DestroyFontsTexture();
		io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoFont_compressed_data_base85, MinFontPointSize + float(uiSizeInt)*FontStepPointSize, 0, ranges.Data);

		// This will call build on the font atlas for us.
		ImGui_ImplOpenGL2_CreateFontsTexture();
		FontUISizeAdded = uiSize;
	}
	if ((FontUISizeAdded != uiSize) && (FontUISizeAdded != Config::ProfileData::UISizeEnum::All))
	{
		ImGui_ImplOpenGL2_DestroyFontsTexture();
		io.Fonts->Clear();
		for (int uisize = 0; uisize < int(Viewer::Config::ProfileData::UISizeEnum::NumSizes); uisize++)
			io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoFont_compressed_data_base85, Viewer::MinFontPointSize + float(uisize)*Viewer::FontStepPointSize, 0, ranges.Data);
		ImGui_ImplOpenGL2_CreateFontsTexture();
		FontUISizeAdded = Config::ProfileData::UISizeEnum::All;
	}

	int fontIndex = -1;
	if (FontUISizeAdded == uiSize)
		fontIndex = 0;
	else if (FontUISizeAdded == Config::ProfileData::UISizeEnum::All)
		fontIndex = uiSizeInt;

	// Update the font.
	tAssert((fontIndex >= 0) && (fontIndex < io.Fonts->Fonts.Size));
	ImFont* font = io.Fonts->Fonts[fontIndex];
	io.FontDefault = font;

	// Update the style scale.
	float uiSizeNorm = float(uiSize) / float(Config::ProfileData::UISizeEnum::Largest);
	float uiSizeScale = tMath::tLini(uiSizeNorm, Viewer::MinUIScale, Viewer::MaxUIScale);
	ImGuiStyle scaledStyle;
	scaledStyle.ScaleAllSizes(uiSizeScale);
	ImGui::GetStyle() = scaledStyle;

	CurrentUISize = uiSize;
}


void Viewer::Update(GLFWwindow* window, double dt, bool dopoll)
{
	// Poll and handle events like inputs, window resize, etc. You can read the io.WantCaptureMouse,
	// io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	//
	// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	// When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	//
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those
	// two flags.
	if (dopoll)
		glfwPollEvents();

	Config::ProfileData& profile = Config::GetProfileData();
	if (Config::Global.TransparentWorkArea)
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	else
		glClearColor(ColourClear.x, ColourClear.y, ColourClear.z, ColourClear.w);
	glClear(GL_COLOR_BUFFER_BIT);

	// We deal with changing the UI size before ImGui_ImplOpenGL2_NewFrame. This is because modifying UI size
	// may need to add a new font texture atlas. Adding a font must happen outside of BeginFrame/EndFrame.
	// If one is added and bd->FontTexture is already set, ImGui_ImplOpenGL2_NewFrame will ignore trying to add
	// a new one. The UI size may change if a) the size was changed in the prefs, b) reset was pressed, or
	// c) inc/dec UISize operation was executed.
	if (CurrentUISize != DesiredUISize)
		SetUISize(DesiredUISize);

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	#ifdef CONFIG_DEBUG
	ImGuiStackSizes stackSizes;
	stackSizes.SetToCurrentState();
	#endif

	int dispw, disph;
	glfwGetFramebufferSize(window, &dispw, &disph);
	if ((dispw != DispW) || (disph != DispH))
	{
		DispW = dispw;
		DispH = disph;
		if (!GetPanX() && !GetPanY())
			ResetPan();
	}

	//
	// Step 1 - Draw and process the main (top) menu bar and remember its height.
	//
	int topUIHeight = DoMainMenuBar();

	//
	// Step 2 - Draw and process the bottom nav bar and remember its height.
	//
	int bottomUIHeight = GetNavBarHeight();
	DoNavBar(DispW, DispH, bottomUIHeight);

	int workAreaW = DispW;
	int workAreaH = DispH - bottomUIHeight - topUIHeight;
	float workAreaAspect = float(workAreaW)/float(workAreaH);

	glViewport(0, bottomUIHeight, workAreaW, workAreaH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, workAreaW, 0, workAreaH, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	float draww		= 1.0f;		float drawh		= 1.0f;
	float iw		= 1.0f;		float ih		= 1.0f;
	float left		= 0.0f;
	float right		= 0.0f;
	float top		= 0.0f;
	float bottom	= 0.0f;
	float uoff		= 0.0f;
	float voff		= 0.0f;
	float panX		= 0.0f;
	float panY		= 0.0f;

	static bool skipUpdatePlaying = false;
	double mouseXd, mouseYd;
	glfwGetCursorPos(window, &mouseXd, &mouseYd);

	// Make origin lower-left.
	float workH = float(DispH - bottomUIHeight);
	float mouseX = float(mouseXd);
	float mouseY = workH - float(mouseYd);
	int mouseXi = int(mouseX);
	int mouseYi = int(mouseY);
	Config::ProfileData::ZoomModeEnum zoomMode = GetZoomMode();
	bool imgAvail = CurrImage && CurrImage->IsLoaded();

	if (imgAvail)
	{
		if (!skipUpdatePlaying)
			CurrImage->UpdatePlaying(float(dt));

		int iwi = CurrImage->GetWidth();
		int ihi = CurrImage->GetHeight();
		iw = float(iwi);
		ih = float(ihi);
		float picAspect = iw/ih;

		draww = float(workAreaW);
		drawh = float(workAreaH);

		if (zoomMode == Config::ProfileData::ZoomModeEnum::DownscaleOnly)
		{
			SetZoomPercent(100.0f);
			float zoomh = draww / iw;
			float zoomv = drawh / ih;
			if ((iw > draww) || (ih > drawh))
				SetZoomPercent(100.0f * tMath::tMin(zoomh, zoomv));
		}
		else if (zoomMode == Config::ProfileData::ZoomModeEnum::Fit)
		{
			float zoomh = draww / iw;
			float zoomv = drawh / ih;
			SetZoomPercent(100.0f * tMath::tMin(zoomh, zoomv));
		}

		float w = iw * GetZoomPercent()/100.0f;
		float h = ih * GetZoomPercent()/100.0f;

		if (!profile.Tile)
		{
			if (Request_PanSnap != Anchor::Invalid)
			{
				// The panPercent is the percent of drawable width (or height) we want to not include the image
				// after snapping. The math for the offsets is easier to understand if you draw yourself a little
				// picture, but essentially it's drawableWidth - imageWidth is the margin, then we need to subtract
				// the margin we want (percent*drawableWidth) to get the offset.
				float panPercent = 0.25f;
				int panOffsetX = int( (draww-w)/2.0f - panPercent*draww );
				int panOffsetY = int( (drawh-h)/2.0f - panPercent*drawh );

				PanDragDownOffsetX = 0;
				switch (Request_PanSnap)
				{
					case Anchor::TL:	case Anchor::ML:	case Anchor::BL:	PanOffsetX = -panOffsetX; break;
					case Anchor::TM:	case Anchor::MM:	case Anchor::BM:	PanOffsetX = 0; break;
					case Anchor::TR:	case Anchor::MR:	case Anchor::BR:	PanOffsetX = panOffsetX; break;
				}

				PanDragDownOffsetY = 0;
				switch (Request_PanSnap)
				{
					case Anchor::TL:	case Anchor::TM:	case Anchor::TR:	PanOffsetY = panOffsetY; break;
					case Anchor::ML:	case Anchor::MM:	case Anchor::MR:	PanOffsetY = 0; break;
					case Anchor::BL:	case Anchor::BM:	case Anchor::BR:	PanOffsetY = -panOffsetY; break;
				}
				Request_PanSnap = Anchor::Invalid;
			}
			else
			{
				int panExtX = tMath::tMax(workAreaW/2, int(w)/2);
				int panExtY = tMath::tMax(workAreaH/2, int(h)/2);
				tMath::tiClamp(PanDragDownOffsetX, -panExtX-PanOffsetX, panExtX-PanOffsetX);
				tMath::tiClamp(PanDragDownOffsetY, -panExtY-PanOffsetY, panExtY-PanOffsetY);
			}
		}
		panX = float(GetPanX());
		panY = float(GetPanY());

		// w and h are the image width and height. draww and drawh are the drawable area width and height.
		left	= tMath::tRound(panX);
		right	= tMath::tRound(draww+panX);
		bottom	= tMath::tRound(panY);
		top		= tMath::tRound(drawh+panY);

		// Compute extents.
		float offsetW = tMath::tRound((draww - w) / 2.0f);
		left	+= offsetW;
		right	= left + w;		// Fix by Oddwarg. I had "right -= offsetW".

		float offsetH = tMath::tRound((drawh - h) / 2.0f);
		bottom	+= offsetH;
		top		= bottom + h;	// Fix by Oddwarg. I had "top -= offsetH".

		// Panning.
		if (RMBDown)
		{
			PanDragDownOffsetX = mouseXi - DragAnchorX;
			PanDragDownOffsetY = mouseYi - DragAnchorY;
		}

		// Draw background.
		glDisable(GL_TEXTURE_2D);
		if ((profile.BackgroundExtend || profile.Tile) && !CropMode)
			DrawBackground(0.0f, draww, 0.0f, drawh, draww, drawh);

		// There is no point drawing the background if the image is completely opaque.
		else if (!CurrImage->IsOpaque())
			DrawBackground(left, right, bottom, top, draww, drawh);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		CurrImage->Bind();
		glEnable(GL_TEXTURE_2D);

		if (RotateAnglePreview != 0.0f)
		{
			float origX = left + (right-left)/2.0f;
			float origY = bottom + (top-bottom)/2.0f;
			tMatrix4 rotMat;	tMakeRotateZ(rotMat, tMath::tDegToRad(RotateAnglePreview));
			tMatrix4 trnMatA;	tMakeTranslate(trnMatA, tVector3(-origX, -origY, 0.0f));
			tMatrix4 trnMatB;	tMakeTranslate(trnMatB, tVector3(origX, origY, 0.0f));
			rotMat = trnMatB * rotMat * trnMatA;
			glPushMatrix();
			glMultMatrixf(rotMat.E);
		}

		// Decide which colour channels to draw. OpenGL handles all this with swizzling.
		int swizzle[4] =
		{
			DrawChannel_R ? GL_RED : GL_ZERO,
			DrawChannel_G ? GL_GREEN : GL_ZERO,
			DrawChannel_B ? GL_BLUE : GL_ZERO,
			DrawChannel_A ? GL_ALPHA : GL_ONE
		};

		if (DrawChannel_AsIntensity)
		{
			if (DrawChannel_R) { swizzle[0] = GL_RED;	swizzle[1] = GL_RED;	swizzle[2] = GL_RED;	swizzle[3] = GL_ONE; }
			if (DrawChannel_G) { swizzle[0] = GL_GREEN;	swizzle[1] = GL_GREEN;	swizzle[2] = GL_GREEN;	swizzle[3] = GL_ONE; }
			if (DrawChannel_B) { swizzle[0] = GL_BLUE;	swizzle[1] = GL_BLUE;	swizzle[2] = GL_BLUE;	swizzle[3] = GL_ONE; }
			if (DrawChannel_A) { swizzle[0] = GL_ALPHA;	swizzle[1] = GL_ALPHA;	swizzle[2] = GL_ALPHA;	swizzle[3] = GL_ONE; }
		}

		bool swizzleModified = false;
		if (ShutterFXCountdown > 0.0f)
		{
			ShutterFXCountdown -= dt;
			int swizzleWhite[4] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleWhite);
			swizzleModified = true;
		}
		else if (!DrawChannel_R || !DrawChannel_G || !DrawChannel_B || !DrawChannel_A)
		{
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
			swizzleModified = true;
		}
	
		if (!profile.Tile)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(left,  bottom);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(left,  top);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(right, top);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(right, bottom);
			glEnd();
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glBegin(GL_QUADS);
			float repU = draww/(right-left);	float offU = (1.0f-repU)/2.0f;
			float repV = drawh/(top-bottom);	float offV = (1.0f-repV)/2.0f;
			float uoffp = -panX/w;
			float voffp = -panY/h;
			glTexCoord2f(offU + 0.0f + uoffp,	offV + 0.0f + voffp);	glVertex2f(0.0f,	0.0f);
			glTexCoord2f(offU + 0.0f + uoffp,	offV + repV + voffp);	glVertex2f(0.0f,	drawh);
			glTexCoord2f(offU + repU + uoffp,	offV + repV + voffp);	glVertex2f(draww,	drawh);
			glTexCoord2f(offU + repU + uoffp,	offV + 0.0f + voffp);	glVertex2f(draww,	0.0f);
			glEnd();
		}

		// Restore swizzle to normal if it was modified.
		if (swizzleModified)
		{
			int defaultSwizzle[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, defaultSwizzle);
		}

		if (RotateAnglePreview != 0.0f)
			glPopMatrix();

		// If mouse was cllcked to adjust cursor pos, CursorMouseX/Y will be >= 0.0f;
		if ((CursorMouseX >= 0.0f) && (CursorMouseX >= 0.0f))
		{
			tVector2 scrCursorPos(CursorMouseX, CursorMouseY);
			int preClampCursorX = CursorX;
			int preClampCursorY = CursorY;
			bool clamped = ConvertScreenPosToImagePos(CursorX, CursorY, scrCursorPos, tVector4(left, right, top, bottom), tVector2(uoff, voff));
			CursorMouseX = -1.0f; CursorMouseY = -1.0f;
			ReticleVisibleOnSelect = !clamped;
			if (clamped)
			{
				CursorX = preClampCursorX;
				CursorY = preClampCursorY;
			}
		}

		// If a request was made to move the cursor/reticle, process the request here,
		if (RequestCursorMove)
		{
			if (!CropMode)
			{
				ReticleVisibleOnSelect = true;
				switch (RequestCursorMove)
				{
					case CursorMove_Left:	CursorX--;	break;
					case CursorMove_Right:	CursorX++;	break;
					case CursorMove_Up:		CursorY++;	break;
					case CursorMove_Down:	CursorY--;	break;
				}
			}
			else
			{
				CropGizmo.MoveDirection(RequestCursorMove, tVector4(left, right, top, bottom), tVector2(uoff, voff), iwi, ihi);
			}
			RequestCursorMove = CursorMove_None;
		}

		// We always clamp. Image could have been cropped or otherwise resized.
		tiClamp(CursorX, 0, iwi - 1);
		tiClamp(CursorY, 0, ihi - 1);

		// Get the colour under the reticle.
		PixelColour = CurrImage->GetPixel(CursorX, CursorY);

		glDisable(GL_TEXTURE_2D);
		glColor4fv(tColour4f::white.E);

		// Show the cursor either as a square ouline or a reticle.
		bool reticleVisible = false;
		
		// Must not be cropping to have a chance at making visible.
		if (!CropMode)
		{
			switch (profile.GetReticleMode())
			{
				case Config::ProfileData::ReticleModeEnum::AlwaysVisible:
					reticleVisible = true;

				case Config::ProfileData::ReticleModeEnum::AlwaysHidden:
					break;

				case Config::ProfileData::ReticleModeEnum::OnSelect:
					reticleVisible = ReticleVisibleOnSelect;
					break;

				case Config::ProfileData::ReticleModeEnum::AutoHide:
					reticleVisible = (DisappearCountdown > 0.0);
					break;
			}
		}

		if (reticleVisible)
		{
			int intensity = (PixelColour.R + PixelColour.G + PixelColour.B) / 3;
			if ((intensity < 128) || (PixelColour.A < 128))
				glColor4ubv(tColour4b::white.E);
			else
				glColor4ubv(tColour4b::black.E);

			tVector4 lrtb(left, right, top, bottom);
			tVector2 uvoffset(uoff, voff);
			tVector2 scrPosBL;
			ConvertImagePosToScreenPos(scrPosBL, CursorX,   CursorY,   lrtb, uvoffset);
			tVector2 scrPosTR;
			ConvertImagePosToScreenPos(scrPosTR, CursorX+1, CursorY+1, lrtb, uvoffset);

			// Decide how to draw reticle based on how big pixels are. If zoomed in, it will be drawn as a square outline.
			const float scrSizeSquareReticle = 8.0f;
			if (tMath::tDistBetweenSq(scrPosBL, scrPosTR) > scrSizeSquareReticle*scrSizeSquareReticle)
			{
				// Draw as square outline.
				glBegin(GL_LINES);
				glVertex2f(scrPosBL.x-1,	scrPosBL.y-1);
				glVertex2f(scrPosTR.x,		scrPosBL.y);

				glVertex2f(scrPosTR.x,		scrPosBL.y);
				glVertex2f(scrPosTR.x,		scrPosTR.y);

				glVertex2f(scrPosTR.x,		scrPosTR.y);
				glVertex2f(scrPosBL.x,		scrPosTR.y);

				glVertex2f(scrPosBL.x,		scrPosTR.y);
				glVertex2f(scrPosBL.x-1,	scrPosBL.y-1);
				glEnd();
			}
			else
			{
				// Draw as reticle centered in the pixel position.
				float cw = float((Image_Reticle.GetWidth()) >> 1);
				float ch = float((Image_Reticle.GetHeight()) >> 1);
				tVector2 mid = (scrPosBL + scrPosTR) / 2.0f;
				float cx = mid.x;
				float cy = mid.y;

				glEnable(GL_TEXTURE_2D);
				Image_Reticle.Bind();
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(cx-cw, cy+ch);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(cx-cw, cy-ch);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(cx+cw, cy-ch);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(cx+cw, cy+ch);
				glEnd();
				glDisable(GL_TEXTURE_2D);
			}
		}

		glDisable(GL_TEXTURE_2D);
		glColor4fv(tColour4f::white.E);
		static bool lastCropMode = false;
		if (CropMode)
		{
			tVector4 lrtb(left, right, top, bottom);
			tVector2 uvoffset(uoff, voff);
			CropGizmo.Update(lrtb, tVector2(mouseX, mouseY), uvoffset);

			if (!lastCropMode)
			{
				// Lines are in image space.
				CropGizmo.SetLines(0, CurrImage->GetWidth()-1, CurrImage->GetHeight()-1, 0, lrtb, uvoffset);
			}

			if (Request_CropLineConstrain)
			{
				CropGizmo.ConstrainLines(0, CurrImage->GetWidth()-1, CurrImage->GetHeight()-1, 0, lrtb, uvoffset);
				Request_CropLineConstrain = false;
			}
		}
		lastCropMode = CropMode;
	}

	// Show the big demo window. You can browse its code to learn more about Dear ImGui.
	static bool showDemoWindow = false;
	//static bool showDemoWindow = true;
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	ImGuiWindowFlags flagsImgButton =
		ImGuiWindowFlags_NoTitleBar		|	ImGuiWindowFlags_NoScrollbar	|	ImGuiWindowFlags_NoMove			| ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse		|	ImGuiWindowFlags_NoNav			|	ImGuiWindowFlags_NoBackground	| ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (!ImGui::GetIO().WantCaptureMouse)
		DisappearCountdown -= dt;
	tVector2 mousePos(mouseX, mouseY);

	tVector2 prevNextArrowSize = Gutil::GetUIParamExtent(tVector2(18.0f, 72.0f), tVector2(32.0f, 128.0f));
	float prevNextArrowMargin = 10.0f;

	// Previous arrow.
	tVector2 rectCenterPrevArrow(0.0f, float(workAreaH)*0.5f);
	tARect2 hitAreaPrevArrow(rectCenterPrevArrow, prevNextArrowSize.y*2.0f);
	if
	(
		(profile.GetOnScreenControls() == Config::ProfileData::OnScreenEnum::Always) ||
		(
			(profile.GetOnScreenControls() == Config::ProfileData::OnScreenEnum::Auto) && !CropMode &&
			((DisappearCountdown > 0.0) || hitAreaPrevArrow.IsPointInside(mousePos)) &&
			((CurrImage != Images.First()) || (SlideshowPlaying && profile.SlideshowLooping))
		)
	)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(0.0f, 0.0f));
		ImGui::SetNextWindowPos(tVector2(prevNextArrowMargin, float(topUIHeight) + float(workAreaH)*0.5f - prevNextArrowSize.y/2.0f));
		ImGui::SetNextWindowSize(prevNextArrowSize, ImGuiCond_Always);
		ImGui::Begin("PrevArrow", nullptr, flagsImgButton);
		ImGui::PushID("MainPrevArrow");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_NextSide_PrevSide.Bind()), prevNextArrowSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 0,
			tVector4::zero, tVector4::one)
		)	OnNextImage(false);
		ImGui::PopID();
		ImGui::End();
		ImGui::PopStyleVar();
	}

	// Next arrow.
	tVector2 rectCenterNextArrow(float(workAreaW), float(workAreaH)*0.5f);
	tARect2 hitAreaNextArrow(rectCenterNextArrow, prevNextArrowSize.y*2.0f);
	if
	(
		(profile.GetOnScreenControls() == Config::ProfileData::OnScreenEnum::Always) ||
		(
			(profile.GetOnScreenControls() == Config::ProfileData::OnScreenEnum::Auto) && !CropMode &&
			((DisappearCountdown > 0.0) || hitAreaNextArrow.IsPointInside(mousePos)) &&
			((CurrImage != Images.Last()) || (SlideshowPlaying && profile.SlideshowLooping))
		)
	)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(0.0f, 0.0f));
		ImGui::SetNextWindowPos(tVector2(workAreaW - prevNextArrowSize.x - prevNextArrowMargin, float(topUIHeight) + float(workAreaH)*0.5f - prevNextArrowSize.y/2.0f));
		ImGui::SetNextWindowSize(prevNextArrowSize, ImGuiCond_Always);
		ImGui::Begin("NextArrow", nullptr, flagsImgButton);
		ImGui::PushID("MainNextArrow");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_NextSide_PrevSide.Bind()), prevNextArrowSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
			tVector4::zero, tVector4::one)
		)	OnNextImage(true);
		ImGui::PopID();
		ImGui::End();
		ImGui::PopStyleVar();
	}

	// Scrubber
	if
	(
		!CropMode && profile.ShowPropsWindow &&
		profile.ShowFrameScrubber && CurrImage && (CurrImage->GetNumFrames() > 1) && !CurrImage->IsAltPictureEnabled()
	)
	{
		float scrubOffset = Gutil::GetUIParamScaled(34.0f, 2.5f);
		ImGui::SetNextWindowPos(tVector2(0.0f, float(topUIHeight) + float(workAreaH) - scrubOffset));
		ImGui::SetNextWindowSize(tVector2(float(workAreaW), 0.0f), ImGuiCond_Always);
		ImGui::Begin("Scrubber", nullptr, flagsImgButton);

		ImGui::PushItemWidth(-1);	// Push E
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(7.0f, 2.0f));
		int frmNum = CurrImage->FrameNum + 1;
		if (ImGui::SliderInt("##ScrubberSlider", &frmNum, 1, CurrImage->GetNumFrames(), "%d", ImGuiSliderFlags_ClampOnInput))
		{
			tMath::tiClamp(frmNum, 1, CurrImage->GetNumFrames());
			CurrImage->FrameNum = frmNum-1;
		}
		skipUpdatePlaying = ImGui::IsItemActive();

		ImGui::PopStyleVar();		// Pop E
		ImGui::PopItemWidth();
		ImGui::End();
	}

	float mainButtonImgDim	= Gutil::GetUIParamScaled(26.0f, 2.5f);
	float mainButtonHSpace	= Gutil::GetUIParamScaled(8.0f, 2.5f);
	float mainButtonHDelta	= mainButtonImgDim + mainButtonHSpace;
	float hitAreaHeight		= Gutil::GetUIParamScaled(100.0f, 2.5f);

	float minHitRectX = (workAreaW>>1) - mainButtonHDelta*5.0f;
	float maxHitRectX = (workAreaW>>1) + mainButtonHDelta*5.0f;
	tVector2 rectMinControlButtons(minHitRectX, 0.0f);
	tVector2 rectMaxControlButtons(maxHitRectX, hitAreaHeight);
	tARect2 hitAreaControlButtons(rectMinControlButtons, rectMaxControlButtons);

	float buttonHeightOffset = Gutil::GetUIParamScaled(62.0f, 2.5f);
	if
	(
		(profile.GetOnScreenControls() == Config::ProfileData::OnScreenEnum::Always) ||
		(
			(profile.GetOnScreenControls() == Config::ProfileData::OnScreenEnum::Auto) && !CropMode &&
			((DisappearCountdown > 0.0) ||
			hitAreaControlButtons.IsPointInside(mousePos))
		)
	)
	{
		tVector2 mainButtonImgSize(mainButtonImgDim, mainButtonImgDim);
		tVector2 mainButtonSize(mainButtonImgDim, mainButtonImgDim);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(0.0f, 0.0f));

		// Center pan button.
		if (GetPanX() || GetPanY())
		{
			ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f - mainButtonHDelta*4.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
			ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
			ImGui::Begin("CenterPan", nullptr, flagsImgButton);
			if (ImGui::ImageButton
			(
				ImTextureID(Image_AnchorMM.Bind()), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
				tVector4::zero, tVector4::one)
			)	OnResetPan();
			ImGui::End();
		}

		// Looping button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f - mainButtonHDelta*3.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Repeat", nullptr, flagsImgButton);
		uint64 playModeImageID = profile.SlideshowLooping ? Image_PlayOnce.Bind() : Image_PlayLoop.Bind();
		if (ImGui::ImageButton
		(
			ImTextureID(playModeImageID), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
			tVector4::zero, tVector4::one)
		)	profile.SlideshowLooping = !profile.SlideshowLooping;
		ImGui::End();

		// Skip to beginning button.
		bool prevAvail = (CurrImage != Images.First()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f - mainButtonHDelta*2.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("SkipBegin", nullptr, flagsImgButton);
		ImGui::PushID("MainSkipBegin");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_SkipEnd_SkipBegin.Bind()), mainButtonImgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 0,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnLastImage(false);
		ImGui::PopID();
		ImGui::End();

		// Prev button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f - mainButtonHDelta*1.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Prev", nullptr, flagsImgButton);
		ImGui::PushID("MainPrev");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Next_Prev.Bind()), mainButtonImgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 0,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnNextImage(false);
		ImGui::PopID();
		ImGui::End();

		// Slideshow Play/Stop button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f + mainButtonHDelta*0.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Slideshow", nullptr, flagsImgButton);
		uint64 psImageID = SlideshowPlaying ? Image_Stop.Bind() : Image_Play_PlayRev.Bind();
		ImGui::PushID("MainPlayStop");
		if (ImGui::ImageButton
		(
			ImTextureID(psImageID), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
			tVector4::zero, tVector4::one)
		)
		{
			SlideshowPlaying = !SlideshowPlaying;
			SlideshowCountdown = profile.SlideshowPeriod;

			// If play pressed and we're on the last image and not looping, start at the beginning again.
			if (SlideshowPlaying && !profile.SlideshowLooping && (CurrImage == Images.Last()))
				OnLastImage(false);
		}
		ImGui::PopID();
		ImGui::End();

		// Next button.
		bool nextAvail = (CurrImage != Images.Last()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f + mainButtonHDelta*1.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Next", nullptr, flagsImgButton);
		ImGui::PushID("MainNext");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Next_Prev.Bind()), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnNextImage(true);
		ImGui::PopID();
		ImGui::End();

		// Skip to end button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f + mainButtonHDelta*2.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("SkipEnd", nullptr, flagsImgButton);
		ImGui::PushID("MainSkipEnd");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_SkipEnd_SkipBegin.Bind()), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnLastImage(true);
		ImGui::PopID();
		ImGui::End();

		// Fullscreen / Windowed button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f + mainButtonHDelta*3.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Fullscreen", nullptr, flagsImgButton);
		uint64 fsImageID = profile.FullscreenMode ? Image_Windowed.Bind() : Image_Fullscreen.Bind();
		if (ImGui::ImageButton
		(
			ImTextureID(fsImageID), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 0,
			tVector4::zero, tVector4::one)
		)	OnChangeScreenMode();
		ImGui::End();

		// Exit basic or kiosk profile.
		if ((Config::GetProfile() == Profile::Basic) || (Config::GetProfile() == Profile::Kiosk))
		{
			ImGui::SetNextWindowPos(tVector2((workAreaW>>1) - mainButtonImgDim/2.0f + mainButtonHDelta*4.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
			ImGui::SetNextWindowSize(tVector2(0.0f, mainButtonImgDim), ImGuiCond_Always);
			ImGui::Begin("ToMainProfile", nullptr, flagsImgButton);
			float escButtonWidth = Gutil::GetUIParamScaled(50.0f, 2.5f);
			if (ImGui::Button("ESC", tVector2(escButtonWidth, mainButtonImgDim)))
				ChangProfile(Profile::Main);
			ImGui::End();
		}

		ImGui::PopStyleVar();
	}

	// Slideshow progress arc.
	if (SlideshowPlaying && (profile.SlideshowPeriod >= 1.0f) && profile.SlideshowProgressArc)
	{
		float arcRadius = Gutil::GetUIParamScaled(8.0f, 2.5f);
		float arcHOffset = Gutil::GetUIParamScaled(97.0f, 2.5f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(0.0f, 0.0f));

		ImGui::SetNextWindowPos(tVector2(((workAreaW>>1) - arcRadius*2.0f), float(topUIHeight) + float(workAreaH) - arcHOffset));
		ImGui::SetNextWindowSize(tVector2(arcRadius*2.0f, arcRadius*2.0f), ImGuiCond_Always);
		ImGui::Begin("SlideProgress", nullptr, flagsImgButton | ImGuiWindowFlags_NoInputs);
		ImGui::SetCursorPos(tVector2(arcRadius*2.0f, arcRadius*2.0f - 1.0f));

		float percent = float(SlideshowCountdown / profile.SlideshowPeriod);
		Gutil::ProgressArc(arcRadius, percent, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), Viewer::ColourClear, arcRadius/2.0f);
		ImGui::End();

		ImGui::PopStyleVar();
	}

	// If any modal is open we allow keyboard navigation. For non-modal we do not as we need the keyboard
	// to control the viewer itself.
	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsPopupOpen(0u, ImGuiPopupFlags_AnyPopup))
		io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

	if (profile.ShowThumbnailView)
		ShowThumbnailViewDialog(&profile.ShowThumbnailView);

	if (profile.ShowPropsWindow)
		ShowPropertiesWindow(&profile.ShowPropsWindow);

	if (profile.ShowPixelEditor)
		ShowPixelEditorOverlay(&profile.ShowPixelEditor);

	if (profile.ShowPreferences)
		ShowPreferencesWindow(&profile.ShowPreferences);

	if (profile.ShowBindingsWindow)
	{
		Bindings::ShowBindingsWindow(&profile.ShowBindingsWindow, BindingsWindowJustOpened);
		BindingsWindowJustOpened = false;
	}

	if (profile.ShowImageMetaData)
		ShowImageMetaDataOverlay(&profile.ShowImageMetaData);

	if (profile.ShowCheatSheet)
		Bindings::ShowCheatSheetWindow(&profile.ShowCheatSheet);

	if (profile.ShowAbout)
		ShowAboutPopup(&profile.ShowAbout);

	if (profile.ShowOutputLog)
		ShowOutputLogPopup(&profile.ShowOutputLog);

	if (profile.ShowChannelFilter)
		ShowChannelFilterOverlay(&profile.ShowChannelFilter);

	static bool showingImport = false;
	bool handledClose = false;
	if (profile.ShowImportRaw)
	{
		handledClose = ShowImportRawOverlay(&profile.ShowImportRaw, ImportRawWindowJustOpened);
		ImportRawWindowJustOpened = false;
		showingImport = true;
	}
	if (!profile.ShowImportRaw && showingImport)
	{
		showingImport = false;
		if (!handledClose)
			CloseCancelImportRawOverlay();
	}

	if (profile.ShowImageDetails)
	{
		float zoomPercent = GetZoomPercent();
		ShowImageDetailsOverlay(&profile.ShowImageDetails, 0.0f, float(topUIHeight), float(dispw), float(disph - bottomUIHeight - topUIHeight), CursorX, CursorY, zoomPercent);
	}

	ShowCropPopup(tVector4(left, right, top, bottom), tVector2(uoff, voff));

	#ifdef CONFIG_DEBUG
	stackSizes.CompareWithCurrentState();
	#endif

	// This calls ImGui::EndFrame for us.
	ImGui::Render();
	glViewport(0, 0, dispw, disph);
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
	FrameNumber++;

	// We're done the frame. Is slideshow playing? If so, decrement the timer. Note: IsPopupOpen ignores str_id if ImGuiPopupFlags_AnyPopupId set.
	if (!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) && SlideshowPlaying)
	{
		SlideshowCountdown -= dt;
		if ((SlideshowCountdown <= 0.0f))
		{
			// If we are supposed to reshuffle at the end of every slideshow loop, we do so here.
			if
			(
				(profile.GetSortKey() == Config::ProfileData::SortKeyEnum::Shuffle) &&
				profile.SlideshowAutoReshuffle && profile.SlideshowLooping &&
				(CurrImage && !CurrImage->Next())
			)
			{
				for (Image* i = Images.First(); i; i = i->Next())
					i->RegenerateShuffleValue();
				SortImages(Config::ProfileData::SortKeyEnum::Shuffle, profile.SortAscending);

				// We set to last after the reshuffle because the OnNext below will push it to the first
				// one as needed. This way we always start from the first image after the reshuffle.
				CurrImage = Images.Last();
			}

			// We only support auto-playing 'forward' because you can always change the ascending flag in the thumbnail view
			// to go backwards. This keeps it simple.
			bool ok = OnNextImage(true);
			if (!ok)
				SlideshowPlaying = false;
			else
				SlideshowCountdown = profile.SlideshowPeriod;
		}
	}
}


bool Viewer::DeleteImageFile(const tString& imgFile, bool tryUseRecycleBin)
{
	// Try to find an image in the same dir that is valid.
	tString nextImgFile = CurrImage->Next() ? CurrImage->Next()->Filename : tString();
	if (nextImgFile.IsEmpty())
		nextImgFile = CurrImage->Prev() ? CurrImage->Prev()->Filename : tString();

	bool deleted = tSystem::tDeleteFile(imgFile, true, tryUseRecycleBin);
	if (!deleted && tryUseRecycleBin)
		deleted = tSystem::tDeleteFile(imgFile, true, false);
		
	if (deleted)
	{
		ImageToLoad = nextImgFile;		// We set this so if we lose and gain focus, we go back to the current image.
		PopulateImages();
		SetCurrentImage(nextImgFile);
	}

	return deleted;
}


bool Viewer::ChangeScreenMode(bool fullscreen, bool force)
{
	Config::ProfileData& profile = Config::GetProfileData();
	if (!force && (profile.FullscreenMode == fullscreen))
		return false;

	// If currently in windowed mode, remember our window geometry.
	if (!force && !profile.FullscreenMode)
	{
		glfwGetWindowPos(Viewer::Window, &Viewer::Config::Global.WindowX, &Viewer::Config::Global.WindowY);
		glfwGetWindowSize(Viewer::Window, &Viewer::Config::Global.WindowW, &Viewer::Config::Global.WindowH);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	if (fullscreen)
	{
		if (Config::Global.TransparentWorkArea)
		{
			glfwSetWindowSize(Viewer::Window, mode->width, mode->height);
			glfwSetWindowPos(Viewer::Window, 0, 0);
		}
		else
		{
			glfwSetWindowMonitor(Viewer::Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
	}
	else
	{
		if (Config::Global.TransparentWorkArea)
		{
			glfwSetWindowSize(Viewer::Window, Viewer::Config::Global.WindowW, Viewer::Config::Global.WindowH);
			glfwSetWindowPos(Viewer::Window, Viewer::Config::Global.WindowX, Viewer::Config::Global.WindowY);
		}
		else
		{
			glfwSetWindowMonitor(Viewer::Window, nullptr, Viewer::Config::Global.WindowX, Viewer::Config::Global.WindowY, Viewer::Config::Global.WindowW, Viewer::Config::Global.WindowH, mode->refreshRate);
		}
	}
	IgnoreNextCursorPosCallback = true;
	profile.FullscreenMode = fullscreen;
	return true;
}


void Viewer::ApplyZoomDelta(float zoomDelta)
{
	tAssert(CurrImage);
	float zoomOrig = GetZoomPercent();

	// Any manual zoom adjustment causes the zoom mode to switch to User.
	SetZoomMode(Config::ProfileData::ZoomModeEnum::User);
	float zoomPercent = zoomOrig + zoomDelta;

	// Make sure the zoom percent always hits 100 exactly.
	if (((zoomOrig < 100.0f) && (zoomPercent > 100.0f)) || ((zoomOrig > 100.0f) && (zoomPercent < 100.0f)))
		zoomPercent = 100.0f;

	tiClamp(zoomPercent, Config::ZoomMin, Config::ZoomMax);
	SetZoomPercent(zoomPercent);

	PanOffsetX += PanDragDownOffsetX; PanDragDownOffsetX = 0;
	PanOffsetY += PanDragDownOffsetY; PanDragDownOffsetY = 0;
	PanOffsetX = int(float(PanOffsetX)*zoomPercent/zoomOrig);
	PanOffsetY = int(float(PanOffsetY)*zoomPercent/zoomOrig);
}


void Viewer::ChangProfile(Profile profile)
{
	if (Config::GetProfile() == profile)
		return;

	Config::SetProfile(profile);

	// This is for the purists. Turns off unnecessary UI elements for the viewer to function only as a simple
	// viewer. Turns off the nav and menu bars, any dialogs (help, about, thumbnails, info, etc), sets the zoom
	// mode to downscale-only, makes the background match the border colour, sets the auto prop editor to false,
	// sets the slideshow/play to looping, and the slideshow duration to 8 seconds.
	//
	// Note that the settings mentioned above are for the _default_ settings of the basic profile. There is nothing
	// stopping the user from customizing the profile however they want. It is essentially a complete set of alternate
	// settings, the only difference is the default values are different than the main profile.
	Config::ProfileData& profileData = Config::GetProfileData();
	AutoPropertyWindow();
	ChangeScreenMode(profileData.FullscreenMode, true);
}


void Viewer::ZoomFit()
{
	ResetPan();
	SetZoomMode(Config::ProfileData::ZoomModeEnum::Fit);
}


void Viewer::ZoomDownscaleOnly()
{
	ResetPan();
	SetZoomMode(Config::ProfileData::ZoomModeEnum::DownscaleOnly);
}


void Viewer::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if ((action != GLFW_PRESS) && (action != GLFW_REPEAT))
		return;

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	// Don't let key repeats starve the update loop. Ignore repeats if there hasn't
	// been a frame between them.
	static uint64 lastRepeatFrameNum = 0;
	if (action == GLFW_REPEAT)
	{
		if (lastRepeatFrameNum == FrameNumber)
			return;
		lastRepeatFrameNum = FrameNumber;
	}

	// Convert key codes to support non-US keyboards. Since the glfwGetKeyName function works on
	// printable characters, it ends up converting the numpad keys KP_* to their printable counterparts.
	// ex. KEY_KP_9 -> KEY_9. This is why we skip the translation for these keys.
	if (!((key >= GLFW_KEY_KP_0) && (key <= GLFW_KEY_KP_EQUAL)))
	{
		const char* keyName = glfwGetKeyName(key, 0);
		if (keyName && *keyName)
			key = tStd::tChrupr(keyName[0]);
	}

	// Now we need to query the key-binding system to find out what operation is associated
	// with the received key. The current bindings are stored in the current profile.
	Config::ProfileData& profile = Config::GetProfileData();
	uint32 viewerModifiers = Bindings::TranslateModifiers(modifiers);
	Bindings::Operation operation = profile.InputBindings.GetOperation(key, viewerModifiers);

	// We allow pasting text if one of the file dialogs is open.
	if
	(
		(operation == Bindings::Operation::Paste) &&
		clip::has(clip::text_format()) &&
		(OpenFileDialog.IsPopupOpen() || OpenDirDialog.IsPopupOpen() || SaveAsDialog.IsPopupOpen())
	)
	{
		OnPasteFromClipboard();
		return;
	}

	// IsPopupOpen ignores str_id if ImGuiPopupFlags_AnyPopupId set.
	if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		return;

	switch (operation)
	{
		case Bindings::Operation::NextImage:			OnNextImage(true);				break;
		case Bindings::Operation::PrevImage:			OnNextImage(false);				break;
		case Bindings::Operation::SkipToLastImage:		OnLastImage(true);				break;
		case Bindings::Operation::SkipToFirstImage:		OnLastImage(false);				break;
		case Bindings::Operation::NextImageFrame:		OnNextImageFrame(true);			break;
		case Bindings::Operation::PrevImageFrame:		OnNextImageFrame(false);		break;
		case Bindings::Operation::PixelRight:			OnPixelMove(MoveDir::Right);	break;
		case Bindings::Operation::PixelLeft:			OnPixelMove(MoveDir::Left);		break;
		case Bindings::Operation::PixelDown:			OnPixelMove(MoveDir::Down);		break;
		case Bindings::Operation::PixelUp:				OnPixelMove(MoveDir::Up);		break;
		case Bindings::Operation::UISizeInc:			OnUISizeInc(true);				break;
		case Bindings::Operation::UISizeDec:			OnUISizeInc(false);				break;
		case Bindings::Operation::ZoomIn:				OnZoomIn();						break;
		case Bindings::Operation::ZoomOut:				OnZoomOut();					break;
		case Bindings::Operation::ZoomFit:				OnZoomFit();					break;
		case Bindings::Operation::ZoomDownscaleOnly:	OnZoomDownscaleOnly();			break;
		case Bindings::Operation::ZoomOneToOne:			OnZoomOneToOne();				break;
		case Bindings::Operation::ZoomPerImage:			OnZoomPerImageToggle();			break;
		case Bindings::Operation::ResetPan:				OnResetPan();					break;
		case Bindings::Operation::FlipVertically:		OnFlipVert(true);				break;
		case Bindings::Operation::FlipHorizontally:		OnFlipVert(false);				break;
		case Bindings::Operation::Rotate90Clockwise:	OnRotate90(true);				break;
		case Bindings::Operation::Rotate90Anticlockwise:OnRotate90(false);				break;
		case Bindings::Operation::RotateImage:			OnRotate();						break;
		case Bindings::Operation::Crop:					OnCrop();						break;
		case Bindings::Operation::ResizeImage:			OnResizeImage();				break;
		case Bindings::Operation::ResizeCanvas:			OnResizeCanvas();				break;
		case Bindings::Operation::PixelEdit:			OnPixelEdit();					break;
		case Bindings::Operation::PropertyEdit:			OnPropertyEdit();				break;
		case Bindings::Operation::ChannelFilter:		OnChannelFilter();				break;
		case Bindings::Operation::Levels:				OnLevels();						break;
		case Bindings::Operation::Quantize:				OnQuantize();					break;
		case Bindings::Operation::RedChannel:			OnRedChannel();					break;
		case Bindings::Operation::GreenChannel:			OnGreenChannel();				break;
		case Bindings::Operation::BlueChannel:			OnBlueChannel();				break;
		case Bindings::Operation::AlphaChannel:			OnAlphaChannel();				break;
		case Bindings::Operation::ChannelAsIntensity:	OnChannelIntensity();			break;
		case Bindings::Operation::Details:				OnDetails();					break;
		case Bindings::Operation::Tile:					OnTile();						break;
		case Bindings::Operation::Undo:					OnUndo();						break;
		case Bindings::Operation::Redo:					OnRedo();						break;
		case Bindings::Operation::Copy:					OnCopyToClipboard();			break;
		case Bindings::Operation::Paste:				OnPasteFromClipboard();			break;
		case Bindings::Operation::ImportRaw:			OnImportRaw();					break;
		case Bindings::Operation::Refresh:				OnRefresh();					break;
		case Bindings::Operation::RefreshDir:			OnRefreshDir();					break;
		case Bindings::Operation::Rename:				OnRename();						break;
		case Bindings::Operation::Delete:				OnDelete();						break;
		case Bindings::Operation::DeletePermanent:		OnDeletePermanent();			break;
		case Bindings::Operation::Save:					OnSave();						break;
		case Bindings::Operation::SaveAs:				OnSaveAs();						break;
		case Bindings::Operation::SaveAll:				OnSaveAll();					break;
		case Bindings::Operation::SaveContactSheet:		OnSaveContactSheet();			break;
		case Bindings::Operation::SaveMultiFrameImage:	OnSaveMultiFrameImage();		break;
		case Bindings::Operation::SaveExtractFrames:	OnSaveExtractFrames();			break;
		case Bindings::Operation::MenuBar:				OnMenuBar();					break;
		case Bindings::Operation::NavBar:				OnNavBar();						break;
		case Bindings::Operation::Thumbnails:			OnThumbnails();					break;
		case Bindings::Operation::FileBrowser:			OnFileBrowser();				break;
		case Bindings::Operation::SlideshowTimer:		OnSlideshowTimer();				break;
		case Bindings::Operation::SlideshowReshuffle:	OnSlideshowReshuffle();			break;
		case Bindings::Operation::CheatSheet:			OnCheatSheet();					break;
		case Bindings::Operation::DebugLog:				OnDebugLog();					break;
		case Bindings::Operation::ProfileMain:			OnProfileMain();				break;
		case Bindings::Operation::ProfileBasic:			OnProfileBasic();				break;
		case Bindings::Operation::ProfileKiosk:			OnProfileKiosk();				break;
		case Bindings::Operation::ProfileAlt:			OnProfileAlt();					break;
		case Bindings::Operation::Preferences:			OnPreferences();				break;
		case Bindings::Operation::KeyBindings:			OnKeyBindings();				break;
		case Bindings::Operation::Fullscreen:			OnChangeScreenMode();			break;
		case Bindings::Operation::Escape:				OnEscape();						break;
		case Bindings::Operation::EscapeSupportingQuit:	OnEscapeWithQuit();				break;
		case Bindings::Operation::Quit:					OnQuit();						break;
		case Bindings::Operation::OpenFile:				OnOpenFile();					break;
		case Bindings::Operation::OpenDir:				OnOpenDir();					break;
		case Bindings::Operation::MetaData:				OnMetaData();					break;
	}
}


void Viewer::MouseButtonCallback(GLFWwindow* window, int mouseButton, int press, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;

	double xposd, yposd;
	glfwGetCursorPos(window, &xposd, &yposd);
	float workH = float(DispH - GetNavBarHeight());

	// Make origin lower-left.
	float mouseX = float(xposd);
	float mouseY = workH - float(yposd);

	bool down = press ? true : false;
	switch (mouseButton)
	{
		// Left mouse button.
		case 0:
		{
			LMBDown = down;
			if (CropMode)
			{
				CropGizmo.MouseButton(LMBDown, tVector2(mouseX, mouseY));
			}
			else if (LMBDown)
			{
				CursorMouseX = mouseX;
				CursorMouseY = mouseY;
			}
			break;
		}

		// Right mouse button.
		case 1:
		{
			RMBDown = down;
			if (RMBDown)
			{
				DragAnchorX = int(mouseX);
				DragAnchorY = int(mouseY);
				PanOffsetX += PanDragDownOffsetX;
				PanOffsetY += PanDragDownOffsetY;
				PanDragDownOffsetX = 0;
				PanDragDownOffsetY = 0;
				ReticleVisibleOnSelect = true;
			}
			break;
		}
	}
}


void Viewer::CursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	
	if (IgnoreNextCursorPosCallback)
	{
		IgnoreNextCursorPosCallback = false;
		return;
	}
	DisappearCountdown = DisappearDuration;
}


void Viewer::ScrollWheelCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;

	SetZoomMode(Config::ProfileData::ZoomModeEnum::User);
	if (CurrImage && CurrImage->IsLoaded())
	{
		float percentChange = (y > 0.0) ? 0.1f : 1.0f-0.909090909f;
		float zoomDelta = GetZoomPercent() * percentChange * float(y);
		ApplyZoomDelta(zoomDelta);
	}
}


void Viewer::FileDropCallback(GLFWwindow* window, int count, const char** files)
{
	if (count < 1)
		return;

	tString file = tString(files[0]);
	ImageToLoad = file;
	PopulateImages();
	SetCurrentImage(file);
}


void Viewer::FocusCallback(GLFWwindow* window, int gotFocus)
{
	if (!gotFocus)
		return;

	// If we got focus, rescan the current folder to see if the hash is different.
	tList<tSystem::tFileInfo> files;
	ImagesDir = FindImagesInImageToLoadDir(files);
	PopulateImagesSubDirs();

	// In case the OS scale was modified.
	UpdateDesiredUISize();

	// We sort here so ComputeImagesHash always returns consistent values.
	files.Sort(Compare_AlphabeticalAscending, tListSortAlgorithm::Merge);
	tuint256 hash = ComputeImagesHash(files);
	Config::ProfileData& profile = Config::GetProfileData();

	// @todo There is a subtle bug here. If a file was replaced by the Viewer to exactly match what the file was
	// when the hash was computed (say from a discard in git), then the hash will not have been updated and it
	// will not detect a change.
	if (hash != ImagesHash)
	{
		tPrintf("Hash mismatch. Dir contents changed. Resynching.\n");
		PopulateImages();

		// It's ok if ImageToLoad is empty.
		if (profile.ShowImportRaw && ImportRaw::ImportedDstFile.IsValid())
			SetCurrentImage(ImportRaw::ImportedDstFile);
		else
			SetCurrentImage(ImageToLoad);
	}
	else
	{
		tPrintf("Hash match. Dir contents same.\n");
		if (profile.ShowImportRaw && ImportRaw::ImportedDstFile.IsValid())
			SetCurrentImage(ImportRaw::ImportedDstFile);
	}
}


void Viewer::IconifyCallback(GLFWwindow* window, int iconified)
{
	WindowIconified = iconified ? true : false;;
}


int Viewer::RemoveOldCacheFiles(const tString& cacheDir)
{
	Config::ProfileData& profile = Config::GetProfileData();

	tList<tSystem::tFileInfo> cacheFiles;
	tSystem::tFindFiles(cacheFiles, cacheDir, "bin");
	int numFiles = cacheFiles.NumItems();
	if (numFiles <= profile.MaxCacheFiles)
		return 0;

	cacheFiles.Sort(Compare_FileCreationTimeAscending);
	int targetCount = tClampMin(profile.MaxCacheFiles - 100, 0);

	int numToRemove = numFiles - targetCount;
	tAssert(numToRemove >= 0);
	int deletedCount = 0;
	while (numToRemove)
	{
		tSystem::tFileInfo* head = cacheFiles.Remove();
		if (tDeleteFile(head->FileName))
			deletedCount++;
		delete head;
		numToRemove--;
	}

	return deletedCount;
}


void Viewer::LoadAppImages(const tString& assetsDir)
{
	Image_Reticle			.Load(assetsDir + "Reticle.png");
	Image_Next_Prev			.Load(assetsDir + "Next_Prev.png");
	Image_NextSide_PrevSide	.Load(assetsDir + "NextSide_PrevSide.png");
	Image_File				.Load(assetsDir + "File.png");
	Image_Folder			.Load(assetsDir + "Folder.png");
	Image_FlipH				.Load(assetsDir + "FlipH.png");
	Image_FlipV				.Load(assetsDir + "FlipV.png");
	Image_RotCW_RotACW		.Load(assetsDir + "RotCW_RotACW.png");
	Image_RotateTheta		.Load(assetsDir + "RotTheta.png");
	Image_Fullscreen		.Load(assetsDir + "Fullscreen.png");
	Image_Windowed			.Load(assetsDir + "Windowed.png");
	Image_SkipEnd_SkipBegin	.Load(assetsDir + "SkipEnd_SkipBegin.png");
	Image_Refresh			.Load(assetsDir + "Refresh.png");
	Image_ShowHidden		.Load(assetsDir + "ShowHidden.png");
	Image_Recycle			.Load(assetsDir + "Recycle.png");
	Image_PropEdit			.Load(assetsDir + "PropEdit.png");
	Image_InfoOverlay		.Load(assetsDir + "InfoOverlay.png");
	Image_MetaData			.Load(assetsDir + "MetaData.png");
	Image_Help				.Load(assetsDir + "Help.png");
	Image_Prefs				.Load(assetsDir + "Settings.png");
	Image_Tile				.Load(assetsDir + "Tile.png");
	Image_Stop				.Load(assetsDir + "Stop.png");
	Image_Play_PlayRev		.Load(assetsDir + "Play_PlayRev.png");
	Image_PlayLoop			.Load(assetsDir + "PlayLoop.png");
	Image_PlayOnce			.Load(assetsDir + "PlayOnce.png");
	Image_ChannelFilter		.Load(assetsDir + "ChannelFilter.png");
	Image_Levels			.Load(assetsDir + "Levels.png");
	Image_ContentView		.Load(assetsDir + "ContentView.png");
	Image_UpFolder			.Load(assetsDir + "UpFolder.png");
	Image_Crop				.Load(assetsDir + "Crop.png");
	Image_AnchorBL			.Load(assetsDir + "AnchorBL.png");
	Image_AnchorBM			.Load(assetsDir + "AnchorBM.png");
	Image_AnchorML			.Load(assetsDir + "AnchorML.png");
	Image_AnchorMM			.Load(assetsDir + "AnchorMM.png");
	Image_DefaultThumbnail	.Load(assetsDir + "DefaultThumbnail.png");
}


void Viewer::UnloadAppImages()
{
	Image_Reticle			.Unload();
	Image_Next_Prev			.Unload();
	Image_NextSide_PrevSide	.Unload();
	Image_File				.Unload();
	Image_Folder			.Unload();
	Image_FlipH				.Unload();
	Image_FlipV				.Unload();
	Image_RotCW_RotACW		.Unload();
	Image_RotateTheta		.Unload();
	Image_Fullscreen		.Unload();
	Image_Windowed			.Unload();
	Image_SkipEnd_SkipBegin	.Unload();
	Image_Refresh			.Unload();
	Image_ShowHidden		.Unload();
	Image_Recycle			.Unload();
	Image_PropEdit			.Unload();
	Image_InfoOverlay		.Unload();
	Image_MetaData			.Unload();
	Image_Help				.Unload();
	Image_Prefs				.Unload();
	Image_Tile				.Unload();
	Image_Stop				.Unload();
	Image_Play_PlayRev		.Unload();
	Image_PlayLoop			.Unload();
	Image_PlayOnce			.Unload();
	Image_ChannelFilter		.Unload();
	Image_Levels			.Unload();
	Image_ContentView		.Unload();
	Image_UpFolder			.Unload();
	Image_Crop				.Unload();
	Image_AnchorBL			.Unload();
	Image_AnchorBM			.Unload();
	Image_AnchorML			.Unload();
	Image_AnchorMM			.Unload();
	Image_DefaultThumbnail	.Unload();
}


#ifdef TACENT_UTF16_API_CALLS
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
	#ifdef PLATFORM_WINDOWS
	setlocale(LC_ALL, ".UTF8");
	#endif

	tCmdLine::tParse(argc, argv);

	// To run in CLI mode you must set the cli option from the command line.
	// You can do this with --cli or -c
	if (Viewer::OptionCLI || Viewer::OptionHelp)
		return Command::Process();

	tSystem::tSetSupplementaryDebuggerOutput();
	tSystem::tSetStdoutRedirectCallback(Viewer::PrintRedirectCallback);

	if (Viewer::ParamImageFiles.IsPresent())
	{
		Viewer::ImageToLoad = Viewer::ParamImageFiles.Get();

		#ifdef PLATFORM_WINDOWS
		tString dest(MAX_PATH);
		int numchars = GetLongPathNameA(Viewer::ImageToLoad.Chr(), dest.Txt(), MAX_PATH);
		if (numchars > 0)
			Viewer::ImageToLoad = dest;
		#endif
	}

	// These three must get set. They depend on the platform and packaging.
	tString assetsDir;		// Must already exist and be populated with things like the icons that are needed for tacentview.
	tString configDir;		// Directory will be created if needed. Contains the per-user viewer config file.
	tString cacheDir;		// Directory will be created if needed. Contains cache information that is not required (but can) persist between releases.

	#if defined(PLATFORM_WINDOWS) || defined(PACKAGE_PORTABLE) || defined(PACKAGE_DEV)
	{
		// The portable layout is also what should be set while developing -- Everything relative
		// to the program executable with separate sub-directories for Assets, Config, and Cache.
		// This keeps portable/dev out of the way of any installed packages. Windows currently only
		// supports the portable layout.
		tString progDir	= tSystem::tGetProgramDir();
		assetsDir		= progDir + "Assets/";
		configDir		= progDir + "Config/";
		cacheDir		= progDir + "Cache/";
	}

	#elif defined(PLATFORM_LINUX)
		#if defined(PACKAGE_SNAP)
		{
			// SNAPs have specific environment variables that specify where to get and put stuff. We don't use XDG.
			// SNAP_USER_DATA is not shared between snap releases (what they call revisions). Used for the cache which can be regenerated.
			// SNAP_USER_COMMON is common to all releases/revisions of a snap. Used for the viewer config file.
			tString snapUserData = tSystem::tGetEnvVar("SNAP_USER_DATA") + "/";
			tString snapUserComm = tSystem::tGetEnvVar("SNAP_USER_COMMON") + "/";

			tString progDir	= tSystem::tGetProgramDir();
			assetsDir		= progDir		+ "Assets/";
			configDir		= snapUserComm	+ "Config/";
			cacheDir		= snapUserData	+ "Cache/";
		}
		#else
		{
			// Everything other than SNAP, including Nix/Deb, support XDG. We never use the default for tGetXDGDataHome
			// but will use the tGetXDGDataDirs default if both are unset. First we check data-home.
			tString dataDir;
			tString subAssetDir = "tacentview/Assets/";
			bool dataHomeWasSet = tSystem::tGetXDGDataHome(dataDir);
			if (dataHomeWasSet && tSystem::tDirExists(dataDir + subAssetDir))
			{
				assetsDir = dataDir + subAssetDir;
			}
			else
			{
				tList<tStringItem> dataDirs;
				tSystem::tGetXDGDataDirs(dataDirs);
				for (tStringItem* ddir = dataDirs.First(); ddir; ddir = ddir->Next())
				{
					tString contender = *ddir + subAssetDir;
					if (tSystem::tDirExists(contender))
					{
						assetsDir = contender;
						break;
					}
				}
			}
			if (assetsDir.IsEmpty())
				assetsDir = "/usr/share/" + subAssetDir;

			tString xdgConfigHome; tSystem::tGetXDGConfigHome(xdgConfigHome);
			tString xdgCacheHome;  tSystem::tGetXDGCacheHome(xdgCacheHome);

			configDir		= xdgConfigHome + "tacentview/";
			cacheDir		= xdgCacheHome + "tacentview/";
		}
		#endif
	#endif // Linux

	tAssert(assetsDir.IsValid());
	tAssert(configDir.IsValid());
	tAssert(cacheDir.IsValid());
	tPrintf("LocInfo: assetsDir : %s\n", assetsDir.Chr());
	tPrintf("LocInfo; configDir : %s\n", configDir.Chr());
	tPrintf("LocInfo: cacheDir  : %s\n", cacheDir.Chr());

	// assetDir is supposed to already exist and, well, have the assets in it.
	// The other two locations need to be created if they don't already exist.
	bool assetsDirExists = tSystem::tDirExists(assetsDir);
	bool configDirExists = tSystem::tDirExists(configDir);
	bool cacheDirExists  = tSystem::tDirExists(cacheDir);
	if (!configDirExists)
		configDirExists = tSystem::tCreateDirs(configDir);
	if (!cacheDirExists)
		cacheDirExists = tSystem::tCreateDirs(cacheDir);

	// If any don't exist we are done for. We check config and cache here and asset
	// later when we can display an error.
	if (!configDirExists)
	{
		tPrintf("Error: Config directory could not be created.\n");
		return Viewer::ErrorCode_GUI_FailConfigDirMissing;
	}
	if (!cacheDirExists)
	{
		tPrintf("Error: Cache directory could not be created.\n");
		return Viewer::ErrorCode_GUI_FailCacheDirMissing;
	}

	Viewer::Image::ThumbCacheDir = cacheDir;
	tString cfgFile = configDir + "Viewer.cfg";
	
	// Setup window
	glfwSetErrorCallback(Viewer::GlfwErrorCallback);
	if (!glfwInit())
		return Viewer::ErrorCode_GUI_FailGLFWInit;

	int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
	tPrintf("Exe %s\n", tSystem::tGetProgramPath().Chr());
	tPrintf("Tacent View V %d.%d.%d\n", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);
	tPrintf("Tacent Library V %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear ImGui V %s\n", IMGUI_VERSION);
	tPrintf("GLFW V %d.%d.%d\n", glfwMajor, glfwMinor, glfwRev);
	tPrintf("For CLI Mode: tacentview --cli --help\n");

	Viewer::Config::Load(cfgFile);
	Viewer::Profile overridProfile = Viewer::Profile::Invalid;
	Viewer::Profile originalProfile = Viewer::Config::GetProfile();
	if (Viewer::OptionProfile)
	{
		const tString& profStr = Viewer::OptionProfile.Arg1();
		switch (tHash::tHashString(profStr.Chr()))
		{
			case tHash::tHashCT("main"):	overridProfile = Viewer::Profile::Main;		break;
			case tHash::tHashCT("basic"):	overridProfile = Viewer::Profile::Basic;	break;
			case tHash::tHashCT("kiosk"):	overridProfile = Viewer::Profile::Kiosk;	break;
			case tHash::tHashCT("alt"):		overridProfile = Viewer::Profile::Alt;		break;
		}
	}
	if (overridProfile != Viewer::Profile::Invalid)
		Viewer::Config::SetProfile(overridProfile);

	// If no file from commandline, see if there is one set in the profile.
	if (Viewer::ImageToLoad.IsEmpty() && Viewer::Config::Global.LastOpenPath.IsValid())
		Viewer::ImageToLoad = Viewer::Config::Global.LastOpenPath;

	Viewer::PendingTransparentWorkArea = Viewer::Config::Global.TransparentWorkArea;

	// We start with window invisible. For windows DwmSetWindowAttribute won't redraw properly otherwise.
	// For all plats, we want to position the window before displaying it.
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	bool requestSnapMessageNoTrans = false;
	if (Viewer::Config::Global.TransparentWorkArea)
	{
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		#ifdef PACKAGE_SNAP
		requestSnapMessageNoTrans = true;
		#endif
	}

	#if defined(PLATFORM_LINUX)
	glfwWindowHintString(GLFW_X11_CLASS_NAME, "tacentview");
	#endif

	Viewer::PendingFrameBufferBPC = Viewer::Config::Global.FrameBufferBPC;
	int windowHintFramebufferBitsPerComponent = 0;
	switch (Viewer::Config::Global.FrameBufferBPC)
	{
		case int(Viewer::Config::GlobalData::FrameBufferBPCEnum::BPC_8):	windowHintFramebufferBitsPerComponent = 8;	break;
		case int(Viewer::Config::GlobalData::FrameBufferBPCEnum::BPC_10):	windowHintFramebufferBitsPerComponent = 10;	break;
		case int(Viewer::Config::GlobalData::FrameBufferBPCEnum::BPC_12):	windowHintFramebufferBitsPerComponent = 12;	break;
		case int(Viewer::Config::GlobalData::FrameBufferBPCEnum::BPC_16):	windowHintFramebufferBitsPerComponent = 16;	break;
	}
	if (windowHintFramebufferBitsPerComponent != 0)
	{
		glfwWindowHint(GLFW_RED_BITS, windowHintFramebufferBitsPerComponent);
		glfwWindowHint(GLFW_GREEN_BITS, windowHintFramebufferBitsPerComponent);
		glfwWindowHint(GLFW_BLUE_BITS, windowHintFramebufferBitsPerComponent);
		if (Viewer::Config::Global.TransparentWorkArea)
			glfwWindowHint(GLFW_ALPHA_BITS, windowHintFramebufferBitsPerComponent);
	}
	// @wip Try on monitor in office.

	// The title here seems to override the Linux hint above. When we create with the title string "tacentview",
	// glfw makes it the X11 WM_CLASS. This is needed so that the Ubuntu can map the same name in the .desktop file
	// to find things like the correct dock icon to display. The SetWindowTitle afterwards does not mod the WM_CLASS.
	Viewer::Window = glfwCreateWindow(Viewer::Config::Global.WindowW, Viewer::Config::Global.WindowH, "tacentview", nullptr, nullptr);
	if (!Viewer::Window)
	{
		// Attempt to display message about failing to create window.
		#ifdef PLATFORM_WINDOWS
		::MessageBoxA
		(
			nullptr,
			"Tacent View failed to launch because it could not create a GLFW window. This may be due to "
			"OpenGL being unavailable on your system or an incompatible graphics driver. Try using Mesa3D "
			"for Windows and placing the opengl32.dll in the same directory as the tacentview executable.",
			"Tacent View Message",
			MB_OK
		);
		glfwTerminate();

		#else
		glfwTerminate();
		system
		(
			"zenity --ellipsize --title=\"Warning\" --warning --text=\""
			"Tacent View failed to launch because it could not\n"
			"create a GLFW window.\""
		);

		tPrintf
		(
			"Tacent View failed to launch because could not "
			"create a GLFW window."
		);
		#endif

		return Viewer::ErrorCode_GUI_FailGLFWWindow;
	}

	if (assetsDirExists)
		Gutil::SetWindowIcon(assetsDir + "TacentView.ico");
	Gutil::SetWindowTitle();
	glfwSetWindowPos(Viewer::Window, Viewer::Config::Global.WindowX, Viewer::Config::Global.WindowY);

	#ifdef PLATFORM_WINDOWS
	// Make the window title bar show up in black.
	HWND hwnd = glfwGetWin32Window(Viewer::Window);
	const int DWMWA_USE_IMMERSIVE_DARK_MODE_A = 19;
	const int DWMWA_USE_IMMERSIVE_DARK_MODE_B = 20;
	BOOL isDarkMode = 1;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_A, &isDarkMode, sizeof(isDarkMode));
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_B, &isDarkMode, sizeof(isDarkMode));
	#endif

	if (!assetsDirExists)
	{
		#ifdef PLATFORM_WINDOWS
		::MessageBoxA
		(
			hwnd,
			"Tacent View failed to launch because it was run from a location "
			"that did not have the Assets directory in it. The executable should be in the "
			"same place as the Assets directory.",
			"Tacent View Message",
			MB_OK
		);

		glfwDestroyWindow(Viewer::Window);
		glfwTerminate();

		#else
		glfwDestroyWindow(Viewer::Window);
		glfwTerminate();
		system
		(
			"zenity --ellipsize --title=\"Warning\" --warning --text=\""
			"Tacent View failed to launch because it could not find\n"
			"the Assets directory. Check your XDG Data environemnt variables.\""
		);

		tPrintf
		(
			"Tacent View failed to launch because it could not find "
			"the Assets directory. Check your XDG Data environemnt variables."
		);
		#endif

		return Viewer::ErrorCode_GUI_FailAssetDirMissing;
	}

	glfwMakeContextCurrent(Viewer::Window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		tPrintf("Failed to initialize GLAD\n");
		glfwDestroyWindow(Viewer::Window);
		glfwTerminate();
		return Viewer::ErrorCode_GUI_FailGLADInit;
	}
	tPrintf("GLAD V %s\n", glGetString(GL_VERSION));

	glfwSwapInterval(1); // Enable vsync
	glfwSetWindowRefreshCallback(Viewer::Window, Viewer::WindowRefreshFun);
	glfwSetKeyCallback(Viewer::Window, Viewer::KeyCallback);
	glfwSetMouseButtonCallback(Viewer::Window, Viewer::MouseButtonCallback);
	glfwSetCursorPosCallback(Viewer::Window, Viewer::CursorPosCallback);
	glfwSetScrollCallback(Viewer::Window, Viewer::ScrollWheelCallback);
	glfwSetDropCallback(Viewer::Window, Viewer::FileDropCallback);
	glfwSetWindowFocusCallback(Viewer::Window, Viewer::FocusCallback);
	glfwSetWindowIconifyCallback(Viewer::Window, Viewer::IconifyCallback);

	// Setup Dear ImGui context.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags = 0;
	// io.NavActive = false;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// Setup Dear ImGui style.
	ImGui::StyleColorsDark();

	// Setup platform/renderer bindings.
	ImGui_ImplGlfw_InitForOpenGL(Viewer::Window, true);
	ImGui_ImplOpenGL2_Init();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Before we load the single font we currently need in the update loop, we need to know what the desired UI size is.
	Viewer::UpdateDesiredUISize();

	Viewer::LoadAppImages(assetsDir);
	Viewer::PopulateImages();

	// SetCurrentImage deals with ImageToLoad being empty.
	Viewer::SetCurrentImage(Viewer::ImageToLoad);

	if (Viewer::Config::Global.TransparentWorkArea)
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	else
		glClearColor(Viewer::ColourClear.x, Viewer::ColourClear.y, Viewer::ColourClear.z, Viewer::ColourClear.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int dispw, disph;
	glfwGetFramebufferSize(Viewer::Window, &dispw, &disph);
	glViewport(0, 0, dispw, disph);

	// Show the window. Can this just be the glfw call for all platforms?
	#ifdef PLATFORM_WINDOWS
	ShowWindow(hwnd, SW_SHOW);
	#elif defined(PLATFORM_LINUX)
	glfwShowWindow(Viewer::Window);
	#endif

	// I don't seem to be able to get Linux to v-sync.
	// glfwSwapInterval(1);
	glfwMakeContextCurrent(Viewer::Window);
	glfwSwapBuffers(Viewer::Window);

	Viewer::Config::ProfileData& profile = *Viewer::Config::Current;
	if (profile.FullscreenMode)
		Viewer::ChangeScreenMode(true, true);

	if (profile.SlideshowAutoStart)
	{
		Viewer::SlideshowPlaying = true;
		Viewer::SlideshowCountdown = profile.SlideshowPeriod;
		Viewer::DisappearCountdown = 0.0;
	}

	if (profile.ShowImportRaw)
		Viewer::ImportRawWindowJustOpened = true;

	int redBits		= 0;	glGetIntegerv(GL_RED_BITS,	&redBits);
	int greenBits	= 0;	glGetIntegerv(GL_GREEN_BITS,&greenBits);
	int blueBits	= 0;	glGetIntegerv(GL_BLUE_BITS,	&blueBits);
	int alphaBits	= 0;	glGetIntegerv(GL_ALPHA_BITS,&alphaBits);
	if (Viewer::Config::Global.TransparentWorkArea)
		tPrintf("Framebuffer BPC (RGBA): (%d,%d,%d,%d)\n", redBits, blueBits, greenBits, alphaBits);
	else
		tPrintf("Framebuffer BPC (RGB): (%d,%d,%d)\n", redBits, blueBits, greenBits);

	// Main loop.
	static double lastUpdateTime = glfwGetTime();
	while (!glfwWindowShouldClose(Viewer::Window) && !Viewer::Request_Quit)
	{
		double currUpdateTime = glfwGetTime();
		double elapsed = tMath::tMin(currUpdateTime - lastUpdateTime, 1.0/30.0);

		Viewer::Update(Viewer::Window, elapsed);

		// Modal dialogs only seem to work after the first Update. May be a ImGui bug?
		if (requestSnapMessageNoTrans)
		{
			Viewer::Request_SnapMessage_NoFrameTrans = true;
			requestSnapMessageNoTrans = false;
		}

		int sleepms = 0;

		// I don't seem to be able to get Linux to v-sync. This stops it using all the CPU.
		#ifdef PLATFORM_LINUX
		sleepms = 16;
		#endif

		if (Viewer::WindowIconified)
			sleepms = 100;
		if (sleepms)
			tSystem::tSleep(sleepms);

		lastUpdateTime = currUpdateTime;
	}

	if (Viewer::CurrImage)
		Viewer::Config::Global.LastOpenPath = Viewer::CurrImage->Filename;
	else if (Viewer::ImagesDir.IsValid())
		Viewer::Config::Global.LastOpenPath = Viewer::ImagesDir;

	// This is important. We need the destructors to run BEFORE we shutdown GLFW. Deconstructing the images may block for a bit while shutting
	// down worker threads. We could show a 'shutting down' popup here if we wanted -- if Image::ThumbnailNumThreadsRunning is > 0.
	Viewer::Images.Clear();
	Viewer::UnloadAppImages();

	// Get current window geometry and set in config file if we're not in fullscreen mode and not iconified.
	if (!profile.FullscreenMode && !Viewer::WindowIconified)
	{
		glfwGetWindowPos(Viewer::Window, &Viewer::Config::Global.WindowX, &Viewer::Config::Global.WindowY);

		// This is the client area size. Use glfwGetWindowFrameSize to include the title bar.
		glfwGetWindowSize(Viewer::Window, &Viewer::Config::Global.WindowW, &Viewer::Config::Global.WindowH);
	}

	// If we called with --profile we don't save it as current. Before saving the config we restore the original.
	if ((overridProfile != Viewer::Profile::Invalid) && (originalProfile != Viewer::Profile::Invalid))
		Viewer::Config::SetProfile(originalProfile);

	Viewer::Config::Global.TransparentWorkArea = Viewer::PendingTransparentWorkArea;
	Viewer::Config::Global.FrameBufferBPC = Viewer::PendingFrameBufferBPC;
	Viewer::Config::Save(cfgFile);

	// Cleanup.
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(Viewer::Window);
	glfwTerminate();

	// Before we go, lets clear out any old cache files.
	if (Viewer::DeleteAllCacheFilesOnExit)
		tSystem::tDeleteDir(Viewer::Image::ThumbCacheDir);
	else
		Viewer::RemoveOldCacheFiles(Viewer::Image::ThumbCacheDir);

	return Viewer::ErrorCode_Success;
}
