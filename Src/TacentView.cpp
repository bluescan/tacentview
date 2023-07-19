// TacentView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2018-2023 Tristan Grimmer.
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
#include "imgui_internal.h"			// For ProgressArc.
#include "TacentView.h"
#include "Image.h"
#include "Dialogs.h"
#include "Details.h"
#include "Preferences.h"
#include "Properties.h"
#include "ContactSheet.h"
#include "MultiFrame.h"
#include "ContentView.h"
#include "Crop.h"
#include "Quantize.h"
#include "Resize.h"
#include "Rotate.h"
#include "OpenSaveDialogs.h"
#include "Config.h"
#include "InputBindings.h"
#include "Command.h"
#include "Version.cmake.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;


namespace Viewer
{
	tCmdLine::tParam  ParamImageFiles	("Files to open",												"ImageFiles",			0,	true	);
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

	// This is the list of lossy formats where the viewer supports lossless transformations.
	tFileTypes FileTypes_LosslessTransform
	(
		tFileType::JPG,
		tFileType::EOL
	);

	tFileDialog::FileDialog OpenFileDialog(tFileDialog::DialogMode::OpenFile, Viewer::FileTypes_Load);
	tFileDialog::FileDialog OpenDirDialog(tFileDialog::DialogMode::OpenDir);
	tFileDialog::FileDialog SaveAsDialog(tFileDialog::DialogMode::SaveFile, Viewer::FileTypes_Save);

	NavLogBar NavBar;
	tString ImagesDir;
	tList<tStringItem> ImagesSubDirs;
	tList<Image> Images;
	tItList<Image> ImagesLoadTimeSorted(tListMode::External);		// We don't need static here cuz the list is only used after main().
	tuint256 ImagesHash												= 0;
	Image* CurrImage												= nullptr;
	tString CurrImageFile;

	void LoadAppImages(const tString& dataDir);
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
	int CurrentFontIndex							= -1;
	double DisappearCountdown						= DisappearDuration;
	double SlideshowCountdown						= 0.0;
	bool SlideshowPlaying							= false;
	bool ReticleVisibleOnSelect						= false;
	bool WindowIconified							= false;

	bool Request_OpenFileModal						= false;
	bool Request_OpenDirModal						= false;
	bool Request_SaveModal							= false;
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
	int DragAnchorX									= 0;
	int DragAnchorY									= 0;

	// CursorX/Y are the real position. CursorMouseX/Y are updated when mouse clicked and update CursorX/Y.
	int CursorX										= 0;
	int CursorY										= 0;
	float CursorMouseX								= -1.0f;
	float CursorMouseY								= -1.0f;
	float RotateAnglePreview						= 0.0f;

	int Dispw										= 1;
	int Disph										= 1;
	int PanOffsetX									= 0;
	int PanOffsetY									= 0;
	int PanDragDownOffsetX							= 0;
	int PanDragDownOffsetY							= 0;
	tColouri PixelColour							= tColouri::black;

	const tVector4 ColourEnabledTint				= tVector4(1.00f, 1.00f, 1.00f, 1.00f);
	const tVector4 ColourDisabledTint				= tVector4(0.54f, 0.54f, 0.54f, 1.00f);
	const tVector4 ColourBG							= tVector4(0.00f, 0.00f, 0.00f, 0.00f);
	const tVector4 ColourPressedBG					= tVector4(0.21f, 0.45f, 0.21f, 1.00f);
	const tVector4 ColourClear						= tVector4(0.10f, 0.10f, 0.12f, 1.00f);

	uint64 FrameNumber								= 0;
	tVector2 ToolImageSizeSmall						(24.0f, 24.0f);
	tVector2 ToolImageSizeMed						(26.0f, 26.0f);
	tVector2 ToolImageSizeLarge						(28.0f, 28.0f);
	tVector2 ColourButtonSizeSmall					(26.0f, 26.0f);
	tVector2 ColourButtonSizeMed					(28.0f, 28.0f);
	tVector2 ColourButtonSizeLarge					(30.0f, 30.0f);

	void DrawBackground(float l, float r, float b, float t, float drawW, float drawH);
	void DrawNavBar(float x, float y, float w, float h);
	int GetNavBarHeight();
	int GetMenuBarHeight();
	void PrintRedirectCallback(const char* text, int numChars);
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }
	void SetWindowIcon(const tString& icoFile);
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
		ImageCompareFunctionObject(Config::ProfileSettings::SortKeyEnum key, bool ascending) : Key(key), Ascending(ascending) { }
		Config::ProfileSettings::SortKeyEnum Key;
		bool Ascending;

		// This is what makes it a magical function object.
		bool operator() (const Image& a, const Image& b) const;
	};

	bool OnPrevious();
	bool OnNext();
	void OnPrevImageFrame();
	void OnNextImageFrame();
	bool OnSkipBegin();
	bool OnSkipEnd();
	void ApplyZoomDelta(float zoomDelta);
	void AutoPropertyWindow();

	tString FindImageFilesInCurrentFolder(tList<tSystem::tFileInfo>& foundFiles);	// Returns the image folder.
	tuint256 ComputeImagesHash(const tList<tSystem::tFileInfo>& files);
	int RemoveOldCacheFiles(const tString& cacheDir);								// Returns num removed.

	CursorMove RequestCursorMove = CursorMove_None;

	void Update(GLFWwindow* window, double dt, bool dopoll = true);
	void WindowRefreshFun(GLFWwindow* window)																			{ Update(window, 0.0, false); }
	void KeyCallback(GLFWwindow*, int key, int scancode, int action, int modifiers);
	void MouseButtonCallback(GLFWwindow*, int mouseButton, int x, int y);
	void CursorPosCallback(GLFWwindow*, double x, double y);
	void ScrollWheelCallback(GLFWwindow*, double x, double y);
	void FileDropCallback(GLFWwindow*, int count, const char** paths);
	void FocusCallback(GLFWwindow*, int gotFocus);
	void IconifyCallback(GLFWwindow*, int iconified);
	void ProgressArc(float radius, float percent, const ImVec4& colour, const ImVec4& colourbg, float thickness = 4.0f, int segments = 32);
	void ChangeProfile(Viewer::Profile);
	bool CopyImage();
	bool PasteImage();
}


bool Viewer::ImageCompareFunctionObject::operator() (const Image& a, const Image& b) const
{
	switch (Key)
	{
		default:
		case Config::ProfileSettings::SortKeyEnum::FileName:
		{
			const char8_t* A = a.Filename.Chars();
			const char8_t* B = b.Filename.Chars();
			return Ascending ? (tStricmp(A, B) < 0) : (tStricmp(A, B) > 0);
		}

		case Config::ProfileSettings::SortKeyEnum::FileModTime:
		{
			time_t A = a.FileModTime;
			time_t B = b.FileModTime;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::FileSize:
		{
			uint64 A = a.FileSizeB;
			uint64 B = b.FileSizeB;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::FileType:
		{
			const char* A = tGetExtension(a.Filetype).Chr();
			const char* B = tGetExtension(b.Filetype).Chr();
			return Ascending ? (tStricmp(A, B) < 0) : (tStricmp(A, B) > 0);
		}

		case Config::ProfileSettings::SortKeyEnum::ImageArea:
		{
			int A = a.Cached_PrimaryArea;
			int B = b.Cached_PrimaryArea;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::ImageWidth:
		{
			int A = a.Cached_PrimaryWidth;
			int B = b.Cached_PrimaryWidth;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::ImageHeight:
		{
			int A = a.Cached_PrimaryHeight;
			int B = b.Cached_PrimaryHeight;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::MetaLatitude:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::LatitudeDD].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::LatitudeDD].Float : -100.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::LatitudeDD].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::LatitudeDD].Float : -100.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::MetaLongitude:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::LongitudeDD].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::LongitudeDD].Float : -200.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::LongitudeDD].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::LongitudeDD].Float : -200.0f;
			return Ascending ? (A < B) : (A > B);
		}

		case Config::ProfileSettings::SortKeyEnum::MetaAltitude:
		{
			float A = a.Cached_MetaData[tImage::tMetaTag::Altitude].IsSet() ? a.Cached_MetaData[tImage::tMetaTag::Altitude].Float : -1000.0f;
			float B = b.Cached_MetaData[tImage::tMetaTag::Altitude].IsSet() ? b.Cached_MetaData[tImage::tMetaTag::Altitude].Float : -1000.0f;
			return Ascending ? (A < B) : (A > B);
		}
	}

	return true;
}


bool Viewer::Button(const char* label, const tMath::tVector2& size)
{
    return ImGui::Button(label, size) || (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter));
}


Viewer::Config::ProfileSettings::ZoomModeEnum Viewer::GetZoomMode()
{
	if (Config::Current->ZoomPerImage && CurrImage)
		return CurrImage->ZoomMode;
	else
		return Config::Current->GetZoomMode();
}


void Viewer::SetZoomMode(Config::ProfileSettings::ZoomModeEnum mode)
{
	if (Config::Current->ZoomPerImage && CurrImage)
		CurrImage->ZoomMode = mode;
	else
		Config::Current->SetZoomMode(mode);
}


float Viewer::GetZoomPercent()
{
	if (Config::Current->ZoomPerImage && CurrImage)
		return CurrImage->ZoomPercent;
	else
		return Config::Current->ZoomPercent;
}


void Viewer::SetZoomPercent(float zoom)
{
	if (Config::Current->ZoomPerImage && CurrImage)
		CurrImage->ZoomPercent = zoom;
	else
		Config::Current->ZoomPercent = zoom;
}


void Viewer::PrintRedirectCallback(const char* text, int numChars)
{
	NavBar.AddLog("%s", text);
	
	#ifdef PLATFORM_LINUX
	// We have a terminal in Linux so use it.
	printf("%s", text);
	#endif
}


tVector2 Viewer::GetDialogOrigin(DialogID dialogID)
{
	int hindex = int(dialogID) % 4;
	int vindex = int(dialogID) / 4;

	float topOffset, leftOffset, heightDelta;
	switch (Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:
			topOffset	= 64.0f;
			leftOffset	= 30.0f;
			heightDelta = 22.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
			topOffset	= 66.0f;
			leftOffset	= 32.0f;
			heightDelta = 24.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:
			topOffset	= 68.0f;
			leftOffset	= 34.0f;
			heightDelta = 26.0f;
			break;
	}

	float widthDelta = 200.0f;
	float x = leftOffset + widthDelta*float(hindex);
	float y = topOffset + heightDelta*float(vindex);
	return tVector2(x, y);
}


int Viewer::GetNavBarHeight()
{
	if (!Config::Current->ShowNavBar)
		return 0;

	return NavBar.GetShowLog() ? 150 : 30;
}


int Viewer::GetMenuBarHeight()
{
	if (!Config::Current->ShowMenuBar)
		return 0;

	switch (Config::Current->GetUISize())
	{
		case Config::ProfileSettings::UISizeEnum::Small:		return 30;
		case Config::ProfileSettings::UISizeEnum::Medium:		return 32;
		case Config::ProfileSettings::UISizeEnum::Large:		return 34;
	}
	return 30;
}


void Viewer::DrawNavBar(float x, float y, float w, float h)
{
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(tVector2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(tVector2(x, y), ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("NavBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
	NavBar.Draw();
	ImGui::End();

	ImGui::PopStyleVar(3);
}


tString Viewer::FindImageFilesInCurrentFolder(tList<tSystem::tFileInfo>& foundFiles)
{
	tString imagesDir = tSystem::tGetCurrentDir();
	if (!CurrImageFile.IsEmpty() && tSystem::tIsAbsolutePath(CurrImageFile))
		imagesDir = tSystem::tGetDir(CurrImageFile);

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
	}

	return hash;
}


void Viewer::PopulateImagesSubDirs()
{
	ImagesSubDirs.Clear();

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
	ImagesDir = FindImageFilesInCurrentFolder(foundFiles);
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

	SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);
	CurrImage = nullptr;
}


void Viewer::SortImages(Config::ProfileSettings::SortKeyEnum key, bool ascending)
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


void Viewer::SetCurrentImage(const tString& currFilename)
{
	for (Image* si = Images.First(); si; si = si->Next())
	{
		tString siName = tSystem::tGetFileName(si->Filename);
		tString imgName = tSystem::tGetFileName(currFilename);

		// We want a case-sensitive compare on Linux.
		#ifdef PLATFORM_LINUX
		if (tStrcmp(siName.Chars(), imgName.Chars()) == 0)
		#else
		if (tStricmp(siName.Chars(), imgName.Chars()) == 0)
		#endif
		{
			CurrImage = si;
			break;
		}
	}

	if (!CurrImage)
	{
		CurrImage = Images.First();
		if (!currFilename.IsEmpty())
			tPrintf("Could not display [%s].\n", tSystem::tGetFileName(currFilename).Chr());
		if (CurrImage && !CurrImage->Filename.IsEmpty())
			tPrintf("Displaying [%s] instead.\n", tSystem::tGetFileName(CurrImage->Filename).Chr());
	}

	if (CurrImage)
		LoadCurrImage();
}


void Viewer::AutoPropertyWindow()
{
	if (Config::Current->AutoPropertyWindow)
		Config::Current->ShowPropsWindow = (CurrImage->TypeSupportsProperties() || (CurrImage->GetNumFrames() > 1));

	if (SlideshowPlaying)
		Config::Current->ShowPropsWindow = false;
}


void Viewer::LoadCurrImage()
{
	tAssert(CurrImage);
	bool imgJustLoaded = false;
	if (!CurrImage->IsLoaded())
		imgJustLoaded = CurrImage->Load();

	AutoPropertyWindow();
	SetWindowTitle();
	if (!CurrImage->IsLoaded())
	{
		tPrintf("Warning: Failed to load [%s]\n", tGetFileName(CurrImage->Filename).Chr());
		return;
	}

	if
	(
		Config::Current->AutoPlayAnimatedImages &&
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
	bool slideshowSmallDuration = SlideshowPlaying && (Config::Current->SlideshowPeriod < 0.5f);
	if (imgJustLoaded && !slideshowSmallDuration)
	{
		ImagesLoadTimeSorted.Sort(Compare_ImageLoadTimeAscending);

		int64 usedMem = 0;
		for (tItList<Image>::Iter iter = ImagesLoadTimeSorted.First(); iter; iter++)
			usedMem += int64((*iter).Info.MemSizeBytes);

		int64 allowedMem = int64(Config::Current->MaxImageMemMB) * 1024 * 1024;
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


bool Viewer::OnPrevious()
{
	bool circ = SlideshowPlaying && Config::Current->SlideshowLooping;
	if (!CurrImage || (!circ && !CurrImage->Prev()))
		return false;

	if (SlideshowPlaying)
		SlideshowCountdown = Config::Current->SlideshowPeriod;

	CurrImage = circ ? Images.PrevCirc(CurrImage) : CurrImage->Prev();
	LoadCurrImage();
	return true;
}


bool Viewer::OnNext()
{
	bool circ = SlideshowPlaying && Config::Current->SlideshowLooping;
	if (!CurrImage || (!circ && !CurrImage->Next()))
		return false;

	if (SlideshowPlaying)
		SlideshowCountdown = Config::Current->SlideshowPeriod;

	CurrImage = circ ? Images.NextCirc(CurrImage) : CurrImage->Next();
	LoadCurrImage();
	return true;
}


void Viewer::OnPrevImageFrame()
{
	tAssert(CurrImage);
	if (CurrImage->GetNumFrames() <= 1)
		return;

	CurrImage->Stop();
	CurrImage->FrameNum = tClampMin(CurrImage->FrameNum-1, 0);
}


void Viewer::OnNextImageFrame()
{
	tAssert(CurrImage);
	if (CurrImage->GetNumFrames() <= 1)
		return;

	CurrImage->Stop();
	CurrImage->FrameNum = tClampMax(CurrImage->FrameNum+1, CurrImage->GetNumFrames()-1);
}


bool Viewer::OnSkipBegin()
{
	if (!CurrImage || !Images.First())
		return false;

	CurrImage = Images.First();
	LoadCurrImage();
	return true;
}


bool Viewer::OnSkipEnd()
{
	if (!CurrImage || !Images.Last())
		return false;

	CurrImage = Images.Last();
	LoadCurrImage();
	return true;
}


void Viewer::ShowHelpMark(const char* desc, bool autoWrap)
{
	ImGui::TextDisabled("[?]");
	if (!ImGui::IsItemHovered() || !desc)
		return;

	ImGui::BeginTooltip();
	if (autoWrap)
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	if (autoWrap)
		ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}


void Viewer::ShowToolTip(const char* desc, bool autoWrap)
{
	if (!ImGui::IsItemHovered() || !desc)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(3.0f, 3.0f));
	ImGui::BeginTooltip();
	if (autoWrap)
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	if (autoWrap)
		ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
	ImGui::PopStyleVar();
}


void Viewer::SetWindowTitle()
{
	if (!Window)
		return;

	tString title = "Tacent View";
	if (CurrImage && !CurrImage->Filename.IsEmpty())
	{
		title = title + " - " + tGetFileName(CurrImage->Filename);
		if (CurrImage->IsDirty())
			title += "*";
	}

	glfwSetWindowTitle(Window, title.Chr());
}


void Viewer::SetWindowIcon(const tString& icoFile)
{
	// Some window manager in Linux show an app icon (like KDE) while some don't by default (Gnome).
	// For windows, the icon is set as an exe resource, so no need to call this for that platform.
	#ifdef PLATFORM_LINUX
	tImage::tImageICO icon(icoFile);
	if (!icon.IsValid())
		return;

	const int maxImages = 16;
	GLFWimage* imageTable[maxImages];
	GLFWimage images[maxImages];
	int numImages = tMath::tMin(icon.GetNumFrames(), maxImages);
	for (int i = 0; i < numImages; i++)
	{
		imageTable[i] = &images[i];
		tImage::tFrame* frame = icon.GetFrame(i);
		frame->ReverseRows();
		images[i].width = frame->Width;
		images[i].height = frame->Height;
		images[i].pixels = (uint8*)frame->Pixels;
	}

	// This copies the pixel data out so we can let the tImageICO clean itself up afterwards afterwards.
	glfwSetWindowIcon(Viewer::Window, numImages, *imageTable);
	#endif
}


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

	Config::ProfileSettings::BackgroundStyleEnum style = Config::Current->GetBackgroundStyle();
	bool overrideBG = CurrImage && CurrImage->OverrideBackgroundColour;
	if (overrideBG)
		style = Config::ProfileSettings::BackgroundStyleEnum::SolidColour;

	switch (style)
	{
		case Config::ProfileSettings::BackgroundStyleEnum::None:
			return;

		case Config::ProfileSettings::BackgroundStyleEnum::Checkerboard:
		{
			// Semitransparent checkerboard background.
			float checkSize = float(Config::Current->BackgroundCheckerboxSize);

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

		case Config::ProfileSettings::BackgroundStyleEnum::SolidColour:
		{
			tColour4i bgCol = Config::Current->BackgroundColour;
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
	float picX = scrPos.x - lrtb.L;
	float picY = (scrPos.y) - lrtb.B;
	float normX = picX / (lrtb.R-lrtb.L);
	float normY = picY / (lrtb.T-lrtb.B);
	if (Config::Current->Tile)
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

	float imposX = imagew * tMath::tLisc(normX, 0.0f + uvOff.u, 1.0f + uvOff.u);
	float imposY = imageh * tMath::tLisc(normY, 0.0f + uvOff.v, 1.0f + uvOff.v);

	imgX = int(imposX);
	imgY = int(imposY);
	bool clamped = false;
	if (!Config::Current->Tile)
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


void Viewer::ProgressArc(float radius, float percent, const ImVec4& colour, const ImVec4& colourbg, float thickness, int segments)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	tiSaturate(percent);
	if (percent <= 0.0f)
		return;

    const ImVec2 pos = window->DC.CursorPos;
	window->DrawList->PathArcTo(pos, radius, IM_PI/2.0f-0.10f, IM_PI/2.0f + percent*IM_PI*2.0f +0.10f, segments-1);
    window->DrawList->PathStroke(ImGui::GetColorU32(colourbg), false, thickness+1.5f);

	window->DrawList->PathArcTo(pos, radius, IM_PI/2.0f, IM_PI/2.0f + percent*IM_PI*2.0f, segments-1);
    window->DrawList->PathStroke(ImGui::GetColorU32(colour), false, thickness);
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

	if (Config::Global.TransparentWorkArea)
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	else
		glClearColor(ColourClear.x, ColourClear.y, ColourClear.z, ColourClear.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int bottomUIHeight	= GetNavBarHeight();
	int topUIHeight		= GetMenuBarHeight();

	ImGui_ImplOpenGL2_NewFrame();		
	ImGui_ImplGlfw_NewFrame();

	int dispw, disph;
	glfwGetFramebufferSize(window, &dispw, &disph);
	if ((dispw != Dispw) || (disph != Disph))
	{
		Dispw = dispw;
		Disph = disph;
		if (!GetPanX() && !GetPanY())
			ResetPan();
	}

	int workAreaW = Dispw;
	int workAreaH = Disph - bottomUIHeight - topUIHeight;
	float workAreaAspect = float(workAreaW)/float(workAreaH);

	glViewport(0, bottomUIHeight, workAreaW, workAreaH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, workAreaW, 0, workAreaH, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	float draww		= 1.0f;		float drawh		= 1.0f;
	float iw 		= 1.0f;		float ih		= 1.0f;
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
	float workH = float(Disph - GetNavBarHeight());
	float mouseX = float(mouseXd);
	float mouseY = workH - float(mouseYd);
	int mouseXi = int(mouseX);
	int mouseYi = int(mouseY);
	Config::ProfileSettings::ZoomModeEnum zoomMode = GetZoomMode();
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

		if (zoomMode == Config::ProfileSettings::ZoomModeEnum::DownscaleOnly)
		{
			SetZoomPercent(100.0f);
			float zoomh = draww / iw;
			float zoomv = drawh / ih;
			if ((iw > draww) || (ih > drawh))
				SetZoomPercent(100.0f * tMath::tMin(zoomh, zoomv));
		}
		else if (zoomMode == Config::ProfileSettings::ZoomModeEnum::Fit)
		{
			float zoomh = draww / iw;
			float zoomv = drawh / ih;
			SetZoomPercent(100.0f * tMath::tMin(zoomh, zoomv));
		}

		float w = iw * GetZoomPercent()/100.0f;
		float h = ih * GetZoomPercent()/100.0f;

		if (!Config::Current->Tile)
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
		if ((Config::Current->BackgroundExtend || Config::Current->Tile) && !CropMode)
			DrawBackground(0.0f, draww, 0.0f, drawh, draww, drawh);

		// There is no point drawing the background at all if the image is completely opaque.
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
	
		if (!Config::Current->Tile)
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
			bool clamped = ConvertScreenPosToImagePos(CursorX, CursorY, scrCursorPos, tVector4(left, right, top, bottom), tVector2(uoff, voff));
			CursorMouseX = -1.0f; CursorMouseY = -1.0f;
			ReticleVisibleOnSelect = !clamped;
		}

		// If a request was made to move the cursor/reticle, process the request here,
		if (RequestCursorMove)
		{
			if (!CropMode)
			{
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
		glColor4fv(tColour::white.E);

		// Show the cursor either as a square ouline or a reticle.
		bool reticleVisible = false;
		
		// Must not be cropping to have a chance at making visible.
		if (!CropMode)
		{
			switch (Config::Current->GetReticleMode())
			{
				case Config::ProfileSettings::ReticleModeEnum::AlwaysVisible:
					reticleVisible = true;

				case Config::ProfileSettings::ReticleModeEnum::AlwaysHidden:
					break;

				case Config::ProfileSettings::ReticleModeEnum::OnSelect:
					reticleVisible = ReticleVisibleOnSelect;
					break;

				case Config::ProfileSettings::ReticleModeEnum::AutoHide:
					reticleVisible = (DisappearCountdown > 0.0);
					break;
			}
		}

		if (reticleVisible)
		{
			int intensity = (PixelColour.R + PixelColour.G + PixelColour.B) / 3;
			if ((intensity < 128) || (PixelColour.A < 128))
				glColor4ubv(tColouri::white.E);
			else
				glColor4ubv(tColouri::black.E);

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
		glColor4fv(tColour::white.E);
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

	ImGui::NewFrame();

	// Did the font change? This may happen if a) the font was changed in the prefs, b) reset was pressed, or c) inc/dec UISize
	// operation was executed. Note that fontCurrent may be null if ImGui hasn't updated it from last time so instead we track
	// what was submitted with CurrentFontIndex.
	if (CurrentFontIndex != Config::Current->UISize)
	{
		ImGuiIO& io = ImGui::GetIO();
		tAssert(Config::Current->UISize < io.Fonts->Fonts.Size);
		ImFont* font = io.Fonts->Fonts[Config::Current->UISize];
		ImGui::PushID((void*)font);
		io.FontDefault = font;
		ImGui::PopID();
		CurrentFontIndex = Config::Current->UISize;
	}

	// Show the big demo window. You can browse its code to learn more about Dear ImGui.
	static bool showDemoWindow = false;
	//static bool showDemoWindow = true;
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	ImGuiWindowFlags flagsImgButton =
		ImGuiWindowFlags_NoTitleBar		|	ImGuiWindowFlags_NoScrollbar	|	ImGuiWindowFlags_NoMove			| ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse		|	ImGuiWindowFlags_NoNav			|	ImGuiWindowFlags_NoBackground	| ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (SlideshowPlaying && (Config::Current->SlideshowPeriod >= 1.0f) && Config::Current->SlideshowProgressArc)
	{
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+7.0f, float(topUIHeight) + float(workAreaH) - 93.0f));
		ImGui::Begin("SlideProgress", nullptr, flagsImgButton | ImGuiWindowFlags_NoInputs);
		ImGui::SetCursorPos(tVector2(15, 14));

		float percent = float(SlideshowCountdown / Config::Current->SlideshowPeriod);
		ProgressArc(8.0f, percent, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), Viewer::ColourClear);
		ImGui::End();
	}

	if (!ImGui::GetIO().WantCaptureMouse)
		DisappearCountdown -= dt;
	tVector2 mousePos(mouseX, mouseY);

	tVector2 mainArrowSize;
	switch (Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:
			mainArrowSize.Set(16.0f, 56.0f);
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
			mainArrowSize.Set(18.0f, 63.0f);
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:
			mainArrowSize.Set(20.0f, 70.0f);
			break;
	}

	tVector2 rectCenterPrevArrow(0.0f, float(workAreaH)*0.5f);
	tVector2 mainArrowWindowSize = mainArrowSize + tVector2(18.0f, 14.0f);
	tARect2 hitAreaPrevArrow(rectCenterPrevArrow, 160.0f);
	if
	(
		!CropMode &&
		((DisappearCountdown > 0.0) || hitAreaPrevArrow.IsPointInside(mousePos)) &&
		((CurrImage != Images.First()) || (SlideshowPlaying && Config::Current->SlideshowLooping))
	)
	{
		// Previous arrow.
		ImGui::SetNextWindowPos(tVector2(-4.0f, float(topUIHeight) + float(workAreaH)*0.5f - mainArrowWindowSize.y/2.0f - 4.0f));
		ImGui::SetNextWindowSize(mainArrowWindowSize, ImGuiCond_Always);
		ImGui::Begin("PrevArrow", nullptr, flagsImgButton);
		// Originally was doing the SetCursor instead of the -4.
		// ImGui::SetCursorPos(tVector2(6, 2));
		ImGui::PushID("MainPrevArrow");
		if (ImGui::ImageButton(ImTextureID(Image_NextSide_PrevSide.Bind()), mainArrowSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 3, tVector4(0.0f, 0.0f, 0.0f, 0.0f), tVector4(1.0f, 1.0f, 1.0f, 1.0f)))
			OnPrevious();
		ImGui::PopID();
		ImGui::End();
	}

	tVector2 rectCenterNextArrow(float(workAreaW), float(workAreaH)*0.5f);
	tARect2 hitAreaNextArrow(rectCenterNextArrow, 160.0f);
	if
	(
		!CropMode &&
		((DisappearCountdown > 0.0) || hitAreaNextArrow.IsPointInside(mousePos)) &&
		((CurrImage != Images.Last()) || (SlideshowPlaying && Config::Current->SlideshowLooping))
	)
	{
		// Next arrow.
		ImGui::SetNextWindowPos(tVector2(workAreaW - mainArrowWindowSize.x, float(topUIHeight) + float(workAreaH) * 0.5f - mainArrowWindowSize.y/2.0f - 4.0f));
		ImGui::SetNextWindowSize(mainArrowWindowSize, ImGuiCond_Always);
		ImGui::Begin("NextArrow", nullptr, flagsImgButton);
		ImGui::PushID("MainNextArrow");
		if (ImGui::ImageButton(ImTextureID(Image_NextSide_PrevSide.Bind()), mainArrowSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 3, tVector4(0.0f, 0.0f, 0.0f, 0.0f), tVector4(1.0f, 1.0f, 1.0f, 1.0f)))
			OnNext();
		ImGui::PopID();
		ImGui::End();
	}

	// Scrubber
	if
	(
		!CropMode && Config::Current->ShowPropsWindow &&
		Config::Current->ShowFrameScrubber && CurrImage && (CurrImage->GetNumFrames() > 1) && !CurrImage->IsAltPictureEnabled()
	)
	{
		ImGui::SetNextWindowPos(tVector2(0.0f, float(topUIHeight) + float(workAreaH) - 34.0f));
		ImGui::SetNextWindowSize(tVector2(float(workAreaW), 5.0f), ImGuiCond_Always);
		ImGui::Begin("Scrubber", nullptr, flagsImgButton);
		ImGui::PushItemWidth(-1);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(7.0f, 2.0f));
		int frmNum = CurrImage->FrameNum + 1;
		if (ImGui::SliderInt("##ScrubberSlider", &frmNum, 1, CurrImage->GetNumFrames(), "%d", ImGuiSliderFlags_ClampOnInput))
		{
			tMath::tiClamp(frmNum, 1, CurrImage->GetNumFrames());
			CurrImage->FrameNum = frmNum-1;
		}
		skipUpdatePlaying = ImGui::IsItemActive();
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::End();
	}

	tVector2 rectMinControlButtons(float(workAreaW)/2.0f-200.0f, 0.0f);
	tVector2 rectMaxControlButtons(float(workAreaW)/2.0f+200.0f, 90.0f);
	tARect2 hitAreaControlButtons(rectMinControlButtons, rectMaxControlButtons);
	float buttonHeightOffset = 69.0f;
	if
	(
		!CropMode &&
		((DisappearCountdown > 0.0) || hitAreaControlButtons.IsPointInside(mousePos))
	)
	{
		float mainButtonImgDim, mainButtonDim, escButtonHeight;
		switch (Config::Current->GetUISize())
		{
			default:
			case Viewer::Config::ProfileSettings::UISizeEnum::Small:
				mainButtonImgDim	= 24.0f;
				mainButtonDim		= 40.0f;
				escButtonHeight		= 28.0f;
				break;
			case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
				mainButtonImgDim	= 26.0f;
				mainButtonDim		= 42.0f;
				escButtonHeight		= 30.0f;
				break;
			case Viewer::Config::ProfileSettings::UISizeEnum::Large:
				mainButtonImgDim	= 28.0f;
				mainButtonDim		= 44.0f;
				escButtonHeight		= 32.0f;
				break;
		}
		tVector2 mainButtonImgSize(mainButtonImgDim, mainButtonImgDim);
		tVector2 mainButtonSize(mainButtonDim, mainButtonDim);

		// Center pan button.
		if (GetPanX() || GetPanY())
		{
			ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-mainButtonDim*4.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
			ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
			ImGui::Begin("CenterPan", nullptr, flagsImgButton);
			if (ImGui::ImageButton(ImTextureID(Image_AnchorMM.Bind()), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 2, tVector4(0.0f, 0.0f, 0.0f, 0.0f), tVector4(1.0f, 1.0f, 1.0f, 1.0f)))
				ResetPan();
			ImGui::End();
		}

		// Looping button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-mainButtonDim*3.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Repeat", nullptr, flagsImgButton);
		uint64 playModeImageID = Config::Current->SlideshowLooping ? Image_PlayOnce.Bind() : Image_PlayLoop.Bind();
		if (ImGui::ImageButton(ImTextureID(playModeImageID), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 2, tVector4(0.0f, 0.0f, 0.0f, 0.0f), tVector4(1.0f, 1.0f, 1.0f, 1.0f)))
			Config::Current->SlideshowLooping = !Config::Current->SlideshowLooping;
		ImGui::End();

		// Skip to beginning button.
		bool prevAvail = (CurrImage != Images.First()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-mainButtonDim*2.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("SkipBegin", nullptr, flagsImgButton);
		ImGui::PushID("MainSkipBegin");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_SkipEnd_SkipBegin.Bind()), mainButtonImgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 2,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnSkipBegin();
		ImGui::PopID();
		ImGui::End();

		// Prev button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-mainButtonDim*1.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Prev", nullptr, flagsImgButton);
		ImGui::PushID("MainPrev");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Next_Prev.Bind()), mainButtonImgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 2,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnPrevious();
		ImGui::PopID();
		ImGui::End();

		// Slideshow Play/Stop button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+mainButtonDim*0.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Slideshow", nullptr, flagsImgButton);
		uint64 psImageID = SlideshowPlaying ? Image_Stop.Bind() : Image_Play_PlayRev.Bind();
		ImGui::PushID("MainPlayStop");
		if (ImGui::ImageButton(ImTextureID(psImageID), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
		{
			SlideshowPlaying = !SlideshowPlaying;
			SlideshowCountdown = Config::Current->SlideshowPeriod;
		}
		ImGui::PopID();
		ImGui::End();

		// Next button.
		bool nextAvail = (CurrImage != Images.Last()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+mainButtonDim*1.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Next", nullptr, flagsImgButton);
		ImGui::PushID("MainNext");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Next_Prev.Bind()), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 2,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnNext();
		ImGui::PopID();
		ImGui::End();

		// Skip to end button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+mainButtonDim*2.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("SkipEnd", nullptr, flagsImgButton);
		ImGui::PushID("MainSkipEnd");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_SkipEnd_SkipBegin.Bind()), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 2,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnSkipEnd();
		ImGui::PopID();
		ImGui::End();

		// Fullscreen / Windowed button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+mainButtonDim*3.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(mainButtonSize, ImGuiCond_Always);
		ImGui::Begin("Fullscreen", nullptr, flagsImgButton);
		uint64 fsImageID = Config::Global.FullscreenMode ? Image_Windowed.Bind() : Image_Fullscreen.Bind();
		if (ImGui::ImageButton(ImTextureID(fsImageID), mainButtonImgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 2, tVector4(0,0,0,0), tVector4(1.0f, 1.0f, 1.0f, 1.0f)))
			ChangeScreenMode(!Config::Global.FullscreenMode);
		ImGui::End();

		// Exit basic profile.
		if (Config::GetProfile() == Profile::Basic)
		{
			ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+mainButtonDim*4.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
			ImGui::SetNextWindowSize(tVector2(120.0f, mainButtonDim), ImGuiCond_Always);
			ImGui::Begin("ExitBasic", nullptr, flagsImgButton);
			if (ImGui::Button("ESC", tVector2(50.0f, escButtonHeight)))
				ChangeProfile(Profile::Main);
			ImGui::End();
		}
	}

	// File requests.
	bool openFilePressed			= Request_OpenFileModal;			Request_OpenFileModal				= false;
	bool openDirPressed				= Request_OpenDirModal;				Request_OpenDirModal				= false;
	bool savePressed				= Request_SaveModal;				Request_SaveModal					= false;
	bool saveAsPressed				= Request_SaveAsModal;				Request_SaveAsModal					= false;
	bool saveAllPressed				= Request_SaveAllModal;				Request_SaveAllModal				= false;
	bool saveContactSheetPressed	= Request_ContactSheetModal;		Request_ContactSheetModal			= false;
	bool saveMultiFramePressed		= Request_MultiFrameModal;			Request_MultiFrameModal				= false;
	bool saveExtractFramesPressed 	= Request_ExtractFramesModal;		Request_ExtractFramesModal			= false;
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

	if (Config::Current->ShowMenuBar)
	{
		ImGui::SetNextWindowPos(tVector2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 8.0f));
		ImGui::BeginMainMenuBar();

		//
		// File Menu.
		//
		if (ImGui::BeginMenu("File"))
		{
			// Show file menu items...
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));

			tString openFileKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::OpenFile);
			if (ImGui::MenuItem("Open File...", openFileKey.Chz()))
				openFilePressed = true;

			tString openDirKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::OpenDir);
			if (ImGui::MenuItem("Open Dir...", openDirKey.Chz()))
				openDirPressed = true;

			ImGui::Separator();

			tString saveKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Save);
			bool saveEnabled = CurrImage ? true : false;
			if (ImGui::MenuItem("Save...", saveKey.Chz(), false, saveEnabled))
				savePressed = true;

			tString saveAsKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::SaveAs);
			bool saveAsEnabled = CurrImage ? true : false;
			if (ImGui::MenuItem("Save As...", saveAsKey.Chz(), false, saveAsEnabled))
				saveAsPressed = true;

			tString saveAllKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::SaveAll);
			bool saveAllEnabled = CurrImage ? true : false;
			if (ImGui::MenuItem("Save All...", saveAllKey.Chz(), false, saveAllEnabled))
				saveAllPressed = true;

			tString saveCSKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::SaveContactSheet);
			bool saveCSEnabled = (Images.GetNumItems() > 1);
			if (ImGui::MenuItem("Save Contact Sheet...", saveCSKey.Chz(), false, saveCSEnabled))
				saveContactSheetPressed = true;

			tString saveMultiKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::SaveMultiFrameImage);
			bool saveMultiEnabled = (Images.GetNumItems() > 1);
			if (ImGui::MenuItem("Save Multi-Frame...", saveMultiKey.Chz(), false, saveMultiEnabled))
				saveMultiFramePressed = true;

			tString saveFramesKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::SaveExtractFrames);
			bool saveFramesEnabled = CurrImage && (CurrImage->GetNumFrames() > 1);
			if (ImGui::MenuItem("Save Extract Frames...", saveFramesKey.Chz(), false, saveFramesEnabled))
				saveExtractFramesPressed = true;

			ImGui::Separator();

			tString copyKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Copy);
			if (ImGui::MenuItem("Copy Image", copyKey.Chz(), false, imgAvail))
				Viewer::CopyImage();

			tString pasteKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Paste);
			if (ImGui::MenuItem("Paste Image", pasteKey.Chz()))
				Viewer::PasteImage();

			ImGui::Separator();

			bool mainProfile = Config::GetProfile() == Profile::Main;
			tString mainProfKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Profile);
			if (ImGui::MenuItem("Main Profile", mainProfile ? nullptr : mainProfKey.Chz(), &mainProfile))
				ChangeProfile(mainProfile ? Profile::Main : Profile::Basic);

			bool basicProfile = Config::GetProfile() == Profile::Basic;
			tString basicProfKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Profile);
			if (ImGui::MenuItem("Basic Profile", basicProfile ? nullptr : basicProfKey.Chz(), &basicProfile))
				ChangeProfile(basicProfile ? Profile::Basic : Profile::Main);

			ImGui::Separator();

			tString bindingsKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::KeyBindings);
			if (ImGui::MenuItem("Key Bindings...", bindingsKey.Chz(), &Config::Current->ShowBindingsWindow))
				if (Config::Current->ShowBindingsWindow)
					BindingsWindowJustOpened = true;

			tString prefsKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Preferences);
			ImGui::MenuItem("Preferences...", prefsKey.Chz(), &Config::Current->ShowPreferences);

			tString quitKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Quit);
			if (ImGui::MenuItem("Quit", quitKey.Chz()))
				glfwSetWindowShouldClose(Window, 1);

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		//
		// Edit Menu.
		//
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));

			bool imgAvail = CurrImage && CurrImage->IsLoaded() && !CurrImage->IsAltPictureEnabled();
			bool undoEnabled = CurrImage && CurrImage->IsUndoAvailable();
			tString undoDesc = undoEnabled ? CurrImage->GetUndoDesc() : tString();
			tString undoStr; tsPrintf(undoStr, "Undo %s", undoDesc.Chr());
			tString undoKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Undo);
			if (ImGui::MenuItem(undoStr.Chr(), undoKey.Chz(), false, undoEnabled))
				Undo();

			bool redoEnabled = CurrImage && CurrImage->IsRedoAvailable();
			tString redoDesc = redoEnabled ? CurrImage->GetRedoDesc() : tString();
			tString redoStr; tsPrintf(redoStr, "Redo %s", redoDesc.Chr());
			tString redoKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Redo);
			if (ImGui::MenuItem(redoStr.Chr(), redoKey.Chz(), false, redoEnabled))
				Redo();

			ImGui::Separator();

			tString flipVKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::FlipVertically);
			if (ImGui::MenuItem("Flip Vertically", flipVKey.Chz(), false, imgAvail ))
			{
				if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
				{
					Request_LosslessTrnsModal = LosslessTransformMode::FlipV;
				}
				else
				{
					CurrImage->Unbind();
					CurrImage->Flip(false);
					CurrImage->Bind();
					SetWindowTitle();
				}
			}

			tString flipHKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::FlipHorizontally);
			if (ImGui::MenuItem("Flip Horizontally", flipHKey.Chz(), false, imgAvail))
			{
				if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
				{
					Request_LosslessTrnsModal = LosslessTransformMode::FlipH;
				}
				else
				{
					CurrImage->Unbind();
					CurrImage->Flip(true);
					CurrImage->Bind();
					SetWindowTitle();
				}
			}

			tString rotACWKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Rotate90Anticlockwise);
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
					SetWindowTitle();
				}
			}

			tString rotCWKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Rotate90Clockwise);
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
					SetWindowTitle();
				}
			}

			tString rotateImgKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::RotateImage);
			if (ImGui::MenuItem("Rotate Image...", rotateImgKey.Chz(), false, imgAvail))
				rotateImagePressed = true;

			ImGui::Separator();

			tString cropKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Crop);
			if (ImGui::MenuItem("Crop...", cropKey.Chz(), &CropMode, imgAvail))
			{
				if (CropMode)
					Config::Current->Tile = false;
			}

			tString resizeImgKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ResizeImage);
			if (ImGui::MenuItem("Resize Image...", resizeImgKey.Chz(), false, imgAvail))
				resizeImagePressed = true;

			tString resizeCanvasKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ResizeCanvas);
			if (ImGui::MenuItem("Resize Canvas...", resizeCanvasKey.Chz(), false, imgAvail))
				resizeCanvasPressed = true;

			ImGui::Separator();

			tString editPixelKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::PixelEdit);
			ImGui::MenuItem("Edit Pixel", editPixelKey.Chz(), &Config::Current->ShowPixelEditor, imgAvail);

			tString chanFiltKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ChannelFilter);
			ImGui::MenuItem("Channel Filter...", chanFiltKey.Chz(), &Config::Current->ShowChannelFilter);

			tString levelsKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Levels);
			if (ImGui::MenuItem("Levels...", levelsKey.Chz(), false, imgAvail))
				levelsPressed = true;

			tString quantizeKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Quantize);
			if (ImGui::MenuItem("Quantize...", quantizeKey.Chz(), false, imgAvail))
				quantizePressed = true;

			tString propsKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::PropertyEdit);
			ImGui::MenuItem("Image Properties...", propsKey.Chz(), &Config::Current->ShowPropsWindow);

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		//
		// View Menu.
		//
		if (ImGui::BeginMenu("View"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));

			tString menuBarKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::MenuBar);
			ImGui::MenuItem("Menu Bar", menuBarKey.Chz(), &Config::Current->ShowMenuBar, !CropMode);

			tString navBarKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::NavBar);
			ImGui::MenuItem("Nav Bar", navBarKey.Chz(), &Config::Current->ShowNavBar, !CropMode);

			tString slideKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::SlideshowTimer);
			ImGui::MenuItem("Slideshow Progress", slideKey.Chz(), &Config::Current->SlideshowProgressArc, !CropMode);

			bool basicSettings = (Config::GetProfile() == Profile::Basic);
			tString modeKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Profile);
			if (ImGui::MenuItem("Basic Profile", modeKey.Chz(), &basicSettings, !CropMode))
				ChangeProfile(basicSettings ? Profile::Basic : Profile::Main);

			tString detailsKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Details);
			ImGui::MenuItem("Image Details", detailsKey.Chz(), &Config::Current->ShowImageDetails);

			tString metaDataKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::MetaData);
			ImGui::MenuItem("Image Meta Data", metaDataKey.Chz(), &Config::Current->ShowImageMetaData);

			tString thumbKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::Thumbnails);
			ImGui::MenuItem("Thumbnail View", thumbKey.Chz(), &Config::Current->ShowContentView);

			ImGui::Separator();

			bool userMode = (zoomMode == Config::ProfileSettings::ZoomModeEnum::User);
			if (ImGui::MenuItem("Zoom User", nullptr, &userMode))
			{
				ResetPan();
				SetZoomMode(Config::ProfileSettings::ZoomModeEnum::User);
			}

			bool fitMode = (zoomMode == Config::ProfileSettings::ZoomModeEnum::Fit);
			tString zoomFitKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ZoomFit);
			if (ImGui::MenuItem("Zoom Fit", zoomFitKey.Chz(), &fitMode))
			{
				ResetPan();
				SetZoomMode(Config::ProfileSettings::ZoomModeEnum::Fit);
			}

			bool downscale = (zoomMode == Config::ProfileSettings::ZoomModeEnum::DownscaleOnly);
			tString zoomDSKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ZoomDownscaleOnly);
			if (ImGui::MenuItem("Zoom Downscale", zoomDSKey.Chz(), &downscale))
			{
				ResetPan();
				SetZoomMode(Config::ProfileSettings::ZoomModeEnum::DownscaleOnly);
			}

			bool oneToOne = (zoomMode == Config::ProfileSettings::ZoomModeEnum::OneToOne);
			tString zoomOneKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ZoomOneToOne);
			if (ImGui::MenuItem("Zoom 1:1", zoomOneKey.Chz(), &oneToOne))
			{
				SetZoomPercent(100.0f);
				ResetPan();
				SetZoomMode(Config::ProfileSettings::ZoomModeEnum::OneToOne);
			}

			ImGui::PushItemWidth(70);
			const char* zoomItems[] = { "Zoom", "20%", "50%", "100%", "150%", "200%", "400%", "800%", "1200%", "1800%", "2500%"};
			float zoomVals[] = { -1.0f, 20.0f, 50.0f, 100.0f, 150.0f, 200.0f, 400.0f, 800.0f, 1200.0f, 1800.0f, 2500.0f };
			float zoomPercent = GetZoomPercent();
			tString currZoomStr;
			tsPrintf(currZoomStr, "%0.0f%%", zoomPercent);

			// We use the zoom combo only as a chooser. It always displays index 0 (Zoom) when not expanded.
			int zoomIdx = 0;
			if (ImGui::Combo(currZoomStr.Chr(), &zoomIdx, zoomItems, tNumElements(zoomItems)) && (zoomIdx > 0) && CurrImage && CurrImage->IsLoaded())
				ApplyZoomDelta(zoomVals[zoomIdx]-zoomPercent);
			ImGui::PopItemWidth();

			tString zoomPerImageKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ZoomPerImage);
			ImGui::MenuItem("Zoom Per Image", zoomPerImageKey.Chz(), &Config::Current->ZoomPerImage);

			tString panKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::ResetPan);
			if (ImGui::MenuItem("Reset Pan", panKey.Chz()))
				ResetPan();

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		//
		// Help Menu.
		//
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));

			tString cheatKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::CheatSheet);
			ImGui::MenuItem("Cheat Sheet", cheatKey.Chz(), &Config::Current->ShowCheatSheet);

			ImGui::MenuItem("About", nullptr, &Config::Current->ShowAbout);
			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		//
		// Toolbar.
		//
		bool imgAvail = (CurrImage && CurrImage->IsLoaded()) ? !CurrImage->IsAltPictureEnabled() : false;
		tVector2 colourButtonSize;
		switch (Config::Current->GetUISize())
		{
			case Config::ProfileSettings::UISizeEnum::Small:	colourButtonSize = ColourButtonSizeSmall;	break;
			case Config::ProfileSettings::UISizeEnum::Medium:	colourButtonSize = ColourButtonSizeMed;		break;
			case Config::ProfileSettings::UISizeEnum::Large:	colourButtonSize = ColourButtonSizeLarge;	break;
		}
		tColourf floatCol(PixelColour);
		tVector4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, colourButtonSize))
			ImGui::OpenPopup("CopyColourAs");

		if (ImGui::BeginPopup("CopyColourAs"))
			ColourCopyAs();

		tVector2 toolImageSize;
		switch (Config::Current->GetUISize())
		{
			case Config::ProfileSettings::UISizeEnum::Small:	toolImageSize = ToolImageSizeSmall;		break;
			case Config::ProfileSettings::UISizeEnum::Medium:	toolImageSize = ToolImageSizeMed;		break;
			case Config::ProfileSettings::UISizeEnum::Large:	toolImageSize = ToolImageSizeLarge;		break;
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_ChannelFilter.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowChannelFilter ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowChannelFilter = !Config::Current->ShowChannelFilter;
		ShowToolTip("Colour Channel Filter");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Levels.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
		{
			Request_LevelsModal = true;
		}
		ShowToolTip("Image Levels");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_ContentView.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowContentView ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowContentView = !Config::Current->ShowContentView;
		ShowToolTip("Content Thumbnail View");

		bool tileAvail = (CurrImage && CurrImage->IsLoaded()) ? !CropMode : false;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Tile.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->Tile ? ColourPressedBG : ColourBG, tileAvail ? ColourEnabledTint : ColourDisabledTint) && tileAvail
		)
		{
			Config::Current->Tile = !Config::Current->Tile;
			if (!Config::Current->Tile)
				ResetPan();
		}
		ShowToolTip("Show Images Tiled");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_FlipV.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
		{
			if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
			{
				Request_LosslessTrnsModal = LosslessTransformMode::FlipV;
			}
			else
			{
				CurrImage->Unbind();
				CurrImage->Flip(false);
				CurrImage->Bind();
				SetWindowTitle();
			}
		}
		ShowToolTip("Flip Vertically");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_FlipH.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
		{
			if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
			{
				Request_LosslessTrnsModal = LosslessTransformMode::FlipH;
			}
			else
			{
				CurrImage->Unbind();
				CurrImage->Flip(true);
				CurrImage->Bind();
				SetWindowTitle();
			}
		}
		ShowToolTip("Flip Horizontally");

		ImGui::PushID("ToolRotACW");
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_RotCW_RotACW.Bind()), toolImageSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
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
				SetWindowTitle();
			}
		}
		ImGui::PopID();
		ShowToolTip("Rotate 90 Anticlockwise");

		ImGui::PushID("ToolRotCW");
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_RotCW_RotACW.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
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
				SetWindowTitle();
			}
		}
		ImGui::PopID();
		ShowToolTip("Rotate 90 Clockwise");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_RotateTheta.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG,
			imgAvail ? ColourEnabledTint : ColourDisabledTint) && imgAvail
		)
		{
			Request_RotateImageModal = true;
		}
		ShowToolTip("Rotate Theta");

		bool cropAvail = CurrImage && imgAvail && !Config::Current->Tile;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Crop.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			CropMode ? ColourPressedBG : ColourBG, cropAvail ? ColourEnabledTint : ColourDisabledTint) && cropAvail
		)
		{
			CropMode = !CropMode;
			if (CropMode)
				Config::Current->Tile = false;
		}
		ShowToolTip("Crop");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_PropEdit.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowPropsWindow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowPropsWindow = !Config::Current->ShowPropsWindow;
		ShowToolTip("Image Properties");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_InfoOverlay.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowImageDetails ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowImageDetails = !Config::Current->ShowImageDetails;
		ShowToolTip("Image Details Overlay");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_MetaData.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowImageMetaData ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowImageMetaData = !Config::Current->ShowImageMetaData;
		ShowToolTip("Image Meta-Data Overlay");

		bool refreshAvail = CurrImage ? true : false;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Refresh.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			ColourBG, refreshAvail ? ColourEnabledTint : ColourDisabledTint) && refreshAvail
		)
		{
			CurrImage->Unbind();
			CurrImage->Unload(true);
			CurrImage->Load();
			CurrImage->Bind();
			SetWindowTitle();
		}
		ShowToolTip("Refresh/Reload Current File");

		bool recycleAvail = CurrImage ? true : false;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Recycle.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			ColourBG, recycleAvail ? ColourEnabledTint : ColourDisabledTint) && recycleAvail
		)	Request_DeleteFileModal = true;
		ShowToolTip("Delete Current File");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Help.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowCheatSheet ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowCheatSheet = !Config::Current->ShowCheatSheet;
		ShowToolTip("Help");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Prefs.Bind()), toolImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
			Config::Current->ShowPreferences ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowPreferences = !Config::Current->ShowPreferences;
		ShowToolTip("Preferences");

		ImGui::EndMainMenuBar();
		ImGui::PopStyleVar();
	}

	// Process any actions. Either from keyboard requests or from the menubar. The ones here are for ImGui windows
	// that require an update every frame. This could be reorganized so that when the operation is executed (in the
	// big keybindings switch) the call to open the popup is performed, but we'd still need the dialog updates here,
	// so this gives better encapsulation.
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4.0f, 3.0f));

	// We deal with all the modals in a single place. Modals are slightly special as we allow keyboard nav during
	// modals but not otherwise as it would interfere with the viewer's keyboard bindings. We start setting nav off.
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = 0;

	//
	// File requests.
	//
	DoOpenFileModal					(openFilePressed);
	DoOpenDirModal					(openDirPressed);

	DoSaveModal						(savePressed);
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

	// If any modal is open we allow keyboard navigation. For non-modal we do not as we need the keyboard
	// to control the viewer itself.
	if (ImGui::IsPopupOpen(0u, ImGuiPopupFlags_AnyPopup))
		io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

	if (Config::Current->ShowContentView)
		ShowContentViewDialog(&Config::Current->ShowContentView);

	if (Config::Current->ShowPropsWindow)
		ShowPropertiesWindow(&Config::Current->ShowPropsWindow);

	if (Config::Current->ShowPixelEditor)
		ShowPixelEditorOverlay(&Config::Current->ShowPixelEditor);

	if (Config::Current->ShowPreferences)
		ShowPreferencesWindow(&Config::Current->ShowPreferences);

	if (Config::Current->ShowBindingsWindow)
	{
		Bindings::ShowBindingsWindow(&Config::Current->ShowBindingsWindow, BindingsWindowJustOpened);
		BindingsWindowJustOpened = false;
	}

	if (Config::Current->ShowImageMetaData)
		ShowImageMetaDataOverlay(&Config::Current->ShowImageMetaData);

	if (Config::Current->ShowCheatSheet)
		Bindings::ShowCheatSheetWindow(&Config::Current->ShowCheatSheet);

	if (Config::Current->ShowAbout)
		ShowAboutPopup(&Config::Current->ShowAbout);

	if (Config::Current->ShowChannelFilter)
		ShowChannelFilterOverlay(&Config::Current->ShowChannelFilter);

	ImGui::PopStyleVar();

	if (Config::Current->ShowNavBar)
		DrawNavBar(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

	if (Config::Current->ShowImageDetails)
	{
		float zoomPercent = GetZoomPercent();
		ShowImageDetailsOverlay(&Config::Current->ShowImageDetails, 0.0f, float(topUIHeight), float(dispw), float(disph - bottomUIHeight - topUIHeight), CursorX, CursorY, zoomPercent);
	}

	ShowCropPopup(tVector4(left, right, top, bottom), tVector2(uoff, voff));

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
			bool ok = OnNext();
			if (!ok)
				SlideshowPlaying = false;
			else
				SlideshowCountdown = Config::Current->SlideshowPeriod;
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
		CurrImageFile = nextImgFile;		// We set this so if we lose and gain focus, we go back to the current image.
		PopulateImages();
		SetCurrentImage(nextImgFile);
	}

	return deleted;
}


bool Viewer::ChangeScreenMode(bool fullscreen, bool force)
{
	if (!force && (Config::Global.FullscreenMode == fullscreen))
		return false;

	// If currently in windowed mode, remember our window geometry.
	if (!force && !Config::Global.FullscreenMode)
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

	Config::Global.FullscreenMode = fullscreen;
	return true;
}


void Viewer::ApplyZoomDelta(float zoomDelta)
{
	tAssert(CurrImage);
	float zoomOrig = GetZoomPercent();

	// Any manual zoom adjustment causes the zoom mode to switch to User.
	SetZoomMode(Config::ProfileSettings::ZoomModeEnum::User);
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


void Viewer::ChangeProfile(Profile profile)
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

	AutoPropertyWindow();
}


void Viewer::ZoomFit()
{
	ResetPan();
	SetZoomMode(Config::ProfileSettings::ZoomModeEnum::Fit);
}


void Viewer::ZoomDownscaleOnly()
{
	ResetPan();
	SetZoomMode(Config::ProfileSettings::ZoomModeEnum::DownscaleOnly);
}


void Viewer::Undo()
{
	tAssert(CurrImage && CurrImage->IsUndoAvailable());
	CurrImage->Unbind();
	CurrImage->Undo();
	CurrImage->Bind();
	SetWindowTitle();
}


void Viewer::Redo()
{
	tAssert(CurrImage && CurrImage->IsRedoAvailable());
	CurrImage->Unbind();
	CurrImage->Redo();
	CurrImage->Bind();
	SetWindowTitle();
}


bool Viewer::CopyImage()
{
	tAssert(CurrImage);
	tImage::tPicture* pic = CurrImage->GetCurrentPic();
	if (!pic)
		pic = CurrImage->GetPrimaryPic();
	if (!pic || !pic->IsValid())
		return false;

	// We need to give the data to the clip system with first row at top.
	tImage::tPicture pict(*pic);
	pict.Flip(false);

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


bool Viewer::PasteImage()
{
	using namespace tImage;

	// The viewer currently requires an image file when you paste (so the filetype is known etc). Essentially what we
	// do is create a new image file (in a lossless/saveable image format as specified by the user config), and then
	// add a new Image based on it to the list, and finally set the current image to it. The filename needs to be unique
	// for the current directory.
	bool ok = false;
	ok = clip::has(clip::image_format());
	if (!ok)
		return false;

	clip::image img;
    ok = clip::get_image(img);
	if (!ok)
		return false;

	//
	// Step 1. Get the data in the necessary tPixel RGBA format with rows starting at bottom.
	//
	const clip::image_spec& spec = img.spec();
	int width = spec.width;
	int height = spec.height;
	uint32* srcData = (uint32*)img.data();
	tPixel* dstData = new tPixel[width*height];

	int bytesPerRow = width*sizeof(tPixel);
	for (int y = height-1; y >= 0; y--)
		tStd::tMemcpy((uint8*)dstData + ((height-1)-y)*bytesPerRow, (uint8*)srcData + y*bytesPerRow, bytesPerRow);

	for (int p = 0; p < width*height; p++)
	{
		uint32 orig = dstData[p].BP;
		uint32 newc =
			(( orig & spec.red_mask  ) >> spec.red_shift  )        |
			(((orig & spec.green_mask) >> spec.green_shift) << 8)  |
			(((orig & spec.blue_mask ) >> spec.blue_shift ) << 16) |
			(((orig & spec.alpha_mask) >> spec.alpha_shift) << 24);
		dstData[p].BP = newc;
	}

	// We are done with the img. We might as well clean it up now instead of waiting for scope to end.
	img.reset();

	//
	// Step 2. Determine filename.
	//
	tFileType pasteType = tGetFileTypeFromName(Config::Current->ClipboardPasteFileType);
	tString filename;
	tsPrintf
	(
		filename, "%sClipboardImage_%s.%s",
		Viewer::ImagesDir.Chr(),
		tConvertTimeToString(tGetTimeLocal(), tTimeFormat::Filename).Chr(),
		tGetExtension(pasteType).Chr()
	);

	//
	// Step 3. Save the file in the correct format losslessly. The dstData is given to the tImage. It will delete it.
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
	// Step 4. Make image current. Add to images list, sort, and make current.
	//
	Image* newImg = new Image(filename);
	Images.Append(newImg);
	ImagesLoadTimeSorted.Append(newImg);
	SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);
	SetCurrentImage(filename);

	// In the clip sample code the clipboard is clear()-ed. I don't think we'd want to do that.
	return true;
}


void Viewer::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if ((action != GLFW_PRESS) && (action != GLFW_REPEAT))
		return;

	ImGuiIO& io = ImGui::GetIO();
	// Note: IsPopupOpen ignores str_id if ImGuiPopupFlags_AnyPopupId set.
	if (io.WantTextInput || ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
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
	// with the received key. The current bindings are stored in the current config.
	uint32 viewerModifiers = Bindings::TranslateModifiers(modifiers);
	Bindings::Operation operation = Config::Current->InputBindings.GetOperation(key, viewerModifiers);
	bool imgAvail = CurrImage && CurrImage->IsLoaded();
	switch (operation)
	{
		case Bindings::Operation::NextImage:
			OnNext();
			break;

		case Bindings::Operation::PrevImage:
			OnPrevious();
			break;

		case Bindings::Operation::SkipToLastImage:
			OnSkipEnd();
			break;

		case Bindings::Operation::SkipToFirstImage:
			OnSkipBegin();
			break;

		case Bindings::Operation::NextImageFrame:
			if (imgAvail) OnNextImageFrame();
			break;

		case Bindings::Operation::PrevImageFrame:
			if (imgAvail) OnPrevImageFrame();
			break;

		case Bindings::Operation::PixelRight:
			if (imgAvail) RequestCursorMove = CursorMove_Right;
			break;

		case Bindings::Operation::PixelLeft:
			if (imgAvail) RequestCursorMove = CursorMove_Left;
			break;

		case Bindings::Operation::PixelDown:
			if (imgAvail) RequestCursorMove = CursorMove_Down;
			break;

		case Bindings::Operation::PixelUp:
			if (imgAvail) RequestCursorMove = CursorMove_Up;
			break;

		case Bindings::Operation::UISizeInc:
			Config::Current->UISize++;
			tMath::tiClampMax(Config::Current->UISize, int(Config::ProfileSettings::UISizeEnum::NumModes)-1);
			break;

		case Bindings::Operation::UISizeDec:
			Config::Current->UISize--;
			tMath::tiClampMin(Config::Current->UISize, 0);
			break;

		case Bindings::Operation::ZoomIn:
			if (imgAvail) ApplyZoomDelta(tMath::tRound(GetZoomPercent()*0.1f));
			break;

		case Bindings::Operation::ZoomOut:
			if (imgAvail) ApplyZoomDelta(tMath::tRound(GetZoomPercent()*(0.909090909f - 1.0f)));
			break;

		case Bindings::Operation::ZoomFit:
			ResetPan();
			SetZoomMode(Config::ProfileSettings::ZoomModeEnum::Fit);
			break;

		case Bindings::Operation::ZoomDownscaleOnly:
			ResetPan();
			SetZoomMode(Config::ProfileSettings::ZoomModeEnum::DownscaleOnly);
			break;

		case Bindings::Operation::ZoomOneToOne:
			SetZoomPercent(100.0f);
			ResetPan();
			SetZoomMode(Config::ProfileSettings::ZoomModeEnum::OneToOne);
			break;

		case Bindings::Operation::ZoomPerImage:
			Config::Current->ZoomPerImage = !Config::Current->ZoomPerImage;
			break;

		case Bindings::Operation::ResetPan:
			ResetPan();
			break;

		case Bindings::Operation::FlipVertically:
		case Bindings::Operation::FlipHorizontally:
			if (imgAvail && !CurrImage->IsAltPictureEnabled())
			{
				if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
				{
					Request_LosslessTrnsModal =
						(operation == Bindings::Operation::FlipVertically) ?
						LosslessTransformMode::FlipV :
						LosslessTransformMode::FlipH;
				}
				else
				{
					CurrImage->Unbind();
					CurrImage->Flip(operation == Bindings::Operation::FlipHorizontally);
					CurrImage->Bind();
					SetWindowTitle();
				}
			}
			break;

		case Bindings::Operation::Rotate90Anticlockwise:
		case Bindings::Operation::Rotate90Clockwise:
			if (imgAvail && !CurrImage->IsAltPictureEnabled())
			{
				if (FileTypes_LosslessTransform.Contains(CurrImage->Filetype))
				{
					Request_LosslessTrnsModal =
						(operation == Bindings::Operation::Rotate90Anticlockwise) ?
						LosslessTransformMode::Rot90ACW :
						LosslessTransformMode::Rot90CW;
				}
				else
				{
					CurrImage->Unbind();
					CurrImage->Rotate90(operation == Bindings::Operation::Rotate90Anticlockwise);
					CurrImage->Bind();
					SetWindowTitle();
				}
			}
			break;

		case Bindings::Operation::RotateImage:
			if (imgAvail) Request_RotateImageModal = true;
			break;

		case Bindings::Operation::Crop:
			if (!CropMode && !imgAvail)
				break;
			CropMode = !CropMode;
			if (CropMode)
				Config::Current->Tile = false;
			break;

		case Bindings::Operation::ResizeImage:
			if (imgAvail) Request_ResizeImageModal = true;
			break;

		case Bindings::Operation::ResizeCanvas:
			if (imgAvail) Request_ResizeCanvasModal = true;
			break;

		case Bindings::Operation::PixelEdit:
			Viewer::Config::Current->ShowPixelEditor = !Viewer::Config::Current->ShowPixelEditor;
			break;

		case Bindings::Operation::PropertyEdit:
			Config::Current->ShowPropsWindow = !Config::Current->ShowPropsWindow;
			break;

		case Bindings::Operation::ChannelFilter:
			Config::Current->ShowChannelFilter = !Config::Current->ShowChannelFilter;
			break;

		case Bindings::Operation::Levels:
			// @todo Should these be checking that no alt image like imgAvail?
			if (imgAvail) Request_LevelsModal = true;
			break;

		case Bindings::Operation::Quantize:
			if (imgAvail) Request_QuantizeModal = true;
			break;

		case Bindings::Operation::RedChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = true; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = false; }
			else
				DrawChannel_R = !DrawChannel_R;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::GreenChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = false; DrawChannel_G = true; DrawChannel_B = false; DrawChannel_A = false; }
			else
				DrawChannel_G = !DrawChannel_G;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::BlueChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = false; DrawChannel_G = false; DrawChannel_B = true; DrawChannel_A = false; }
			else
				DrawChannel_B = !DrawChannel_B;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::AlphaChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = false; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = true; }
			else
				DrawChannel_A = !DrawChannel_A;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::ChannelAsIntensity:
			DrawChannel_AsIntensity = !DrawChannel_AsIntensity;
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = true; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = false; }
			else
				{ DrawChannel_R = true; DrawChannel_G = true; DrawChannel_B = true; DrawChannel_A = true; }
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::Details:
			Viewer::Config::Current->ShowImageDetails = !Viewer::Config::Current->ShowImageDetails;
			break;

		case Bindings::Operation::Tile:
			if (CropMode)
				break;
			Config::Current->Tile = !Config::Current->Tile;
			if (!Config::Current->Tile)
				ResetPan();
			break;

		case Bindings::Operation::Undo:
			if (imgAvail && CurrImage->IsUndoAvailable()) Undo();
			break;

		case Bindings::Operation::Redo:
			if (imgAvail && CurrImage->IsRedoAvailable()) Redo();
			break;

		case Bindings::Operation::Copy:
			if (imgAvail) CopyImage();
			break;

		case Bindings::Operation::Paste:
			PasteImage();
			break;

		case Bindings::Operation::Refresh:
			if (CurrImage)
			{
				CurrImage->Unbind();
				CurrImage->Unload(true);
				CurrImage->Load();
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case Bindings::Operation::Rename:
			if (CurrImage) Request_RenameModal = true;
			break;

		case Bindings::Operation::Delete:
			if (CurrImage) Request_DeleteFileModal = true;
			break;

		case Bindings::Operation::DeletePermanent:
			if (CurrImage) Request_DeleteFileNoRecycleModal = true;
			break;

		case Bindings::Operation::Save:
			if (imgAvail) Request_SaveModal = true;
			break;

		case Bindings::Operation::SaveAs:
			if (imgAvail) Request_SaveAsModal = true;
			break;

		case Bindings::Operation::SaveAll:
			if (CurrImage) Request_SaveAllModal = true;
			break;

		case Bindings::Operation::SaveContactSheet:
			if (Images.GetNumItems() > 1) Request_ContactSheetModal = true;
			break;

		case Bindings::Operation::SaveMultiFrameImage:
			if (Images.GetNumItems() > 1) Request_MultiFrameModal = true;
			break;

		case Bindings::Operation::SaveExtractFrames:
			if (CurrImage && (CurrImage->GetNumFrames() > 1)) Request_ExtractFramesModal = true;
			break;

		case Bindings::Operation::MenuBar:
			if (!CropMode) Config::Current->ShowMenuBar = !Config::Current->ShowMenuBar;
			break;

		case Bindings::Operation::NavBar:
			if (!CropMode) Config::Current->ShowNavBar = !Config::Current->ShowNavBar;
			break;

		case Bindings::Operation::Thumbnails:
			Viewer::Config::Current->ShowContentView = !Viewer::Config::Current->ShowContentView;
			break;

		case Bindings::Operation::FileBrowser:
		{
			#ifdef PACKAGE_SNAP
			static int messageCount = 2;
			if (messageCount-- > 0)
				Request_SnapMessage_NoFileBrowse = true;
			#else
			if (CurrImage) tSystem::tOpenSystemFileExplorer(CurrImage->Filename);
			#endif
			break;
		}

		case Bindings::Operation::SlideshowTimer:
			Config::Current->SlideshowProgressArc = !Config::Current->SlideshowProgressArc;
			break;

		case Bindings::Operation::CheatSheet:
			Config::Current->ShowCheatSheet = !Config::Current->ShowCheatSheet;
			break;

		case Bindings::Operation::DebugLog:
			NavBar.SetShowLog( !NavBar.GetShowLog() );
			if (NavBar.GetShowLog() && !Config::Current->ShowNavBar)
				Config::Current->ShowNavBar = true;
			break;

		case Bindings::Operation::Profile:
			if (!CropMode) ChangeProfile((Config::GetProfile() == Profile::Basic) ? Profile::Main : Profile::Basic);
			break;

		case Bindings::Operation::Preferences:
			Config::Current->ShowPreferences = !Config::Current->ShowPreferences;
			break;

		case Bindings::Operation::KeyBindings:
			Config::Current->ShowBindingsWindow = !Config::Current->ShowBindingsWindow;
			if (Config::Current->ShowBindingsWindow) BindingsWindowJustOpened = true;
			break;

		case Bindings::Operation::Fullscreen:
			ChangeScreenMode(!Config::Global.FullscreenMode);
			break;

		case Bindings::Operation::Escape:
			if (Config::Global.FullscreenMode)
				ChangeScreenMode(false);
			else if (Config::GetProfile() == Profile::Basic)
				ChangeProfile(Profile::Main);
			break;

		case Bindings::Operation::EscapeSupportingQuit:
			if (Config::Global.FullscreenMode)
				ChangeScreenMode(false);
			else if (Config::GetProfile() == Profile::Basic)
				ChangeProfile(Profile::Main);
			else
				Viewer::Request_Quit = true;				
			break;

		case Bindings::Operation::Quit:
			Viewer::Request_Quit = true;				
			break;

		case Bindings::Operation::OpenFile:
			Request_OpenFileModal = true;
			break;

		case Bindings::Operation::OpenDir:
			Request_OpenDirModal = true;
			break;

		case Bindings::Operation::MetaData:
			Viewer::Config::Current->ShowImageMetaData = !Viewer::Config::Current->ShowImageMetaData;
			break;
	}
}


void Viewer::MouseButtonCallback(GLFWwindow* window, int mouseButton, int press, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;

	double xposd, yposd;
	glfwGetCursorPos(window, &xposd, &yposd);
	float workH = float(Disph - GetNavBarHeight());

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
			}
			break;
		}
	}
}


void Viewer::CursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;
}


void Viewer::ScrollWheelCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;

	SetZoomMode(Config::ProfileSettings::ZoomModeEnum::User);
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
	CurrImageFile = file;
	PopulateImages();
	SetCurrentImage(file);
}


void Viewer::FocusCallback(GLFWwindow* window, int gotFocus)
{
	if (!gotFocus)
		return;

	// If we got focus, rescan the current folder to see if the hash is different.
	tList<tSystem::tFileInfo> files;
	ImagesDir = FindImageFilesInCurrentFolder(files);
	PopulateImagesSubDirs();

	// We sort here so ComputeImagesHash always returns consistent values.
	files.Sort(Compare_AlphabeticalAscending, tListSortAlgorithm::Merge);
	tuint256 hash = ComputeImagesHash(files);

	// @todo There is a subtle bug here. If a file was replaced by the Viewer to exactly match what the file was
	// when the hash was computed (say from a discard in git), then the hash will not have been updated and it
	// will not detect a change.
	if (hash != ImagesHash)
	{
		tPrintf("Hash mismatch. Dir contents changed. Resynching.\n");
		PopulateImages();
		if (!CurrImageFile.IsEmpty())
			SetCurrentImage(CurrImageFile);
		else
			SetCurrentImage();
	}
	else
	{
		tPrintf("Hash match. Dir contents same. Doing nothing.\n");
	}
}


void Viewer::IconifyCallback(GLFWwindow* window, int iconified)
{
	WindowIconified = iconified;
}


int Viewer::RemoveOldCacheFiles(const tString& cacheDir)
{
	tList<tSystem::tFileInfo> cacheFiles;
	tSystem::tFindFiles(cacheFiles, cacheDir, "bin");
	int numFiles = cacheFiles.NumItems();
	if (numFiles <= Config::Current->MaxCacheFiles)
		return 0;

	cacheFiles.Sort(Compare_FileCreationTimeAscending);
	int targetCount = tClampMin(Config::Current->MaxCacheFiles - 100, 0);

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


void Viewer::LoadAppImages(const tString& dataDir)
{
	Image_Reticle			.Load(dataDir + "Reticle.png");
	Image_Next_Prev			.Load(dataDir + "Next_Prev.png");
	Image_NextSide_PrevSide	.Load(dataDir + "NextSide_PrevSide.png");
	Image_File				.Load(dataDir + "File.png");
	Image_Folder			.Load(dataDir + "Folder.png");
	Image_FlipH				.Load(dataDir + "FlipH.png");
	Image_FlipV				.Load(dataDir + "FlipV.png");
	Image_RotCW_RotACW		.Load(dataDir + "RotCW_RotACW.png");
	Image_RotateTheta		.Load(dataDir + "RotTheta.png");
	Image_Fullscreen		.Load(dataDir + "Fullscreen.png");
	Image_Windowed			.Load(dataDir + "Windowed.png");
	Image_SkipEnd_SkipBegin	.Load(dataDir + "SkipEnd_SkipBegin.png");
	Image_Refresh			.Load(dataDir + "Refresh.png");
	Image_ShowHidden		.Load(dataDir + "ShowHidden.png");
	Image_Recycle			.Load(dataDir + "Recycle.png");
	Image_PropEdit			.Load(dataDir + "PropEdit.png");
	Image_InfoOverlay		.Load(dataDir + "InfoOverlay.png");
	Image_MetaData			.Load(dataDir + "MetaData.png");
	Image_Help				.Load(dataDir + "Help.png");
	Image_Prefs				.Load(dataDir + "Settings.png");
	Image_Tile				.Load(dataDir + "Tile.png");
	Image_Stop				.Load(dataDir + "Stop.png");
	Image_Play_PlayRev		.Load(dataDir + "Play_PlayRev.png");
	Image_PlayLoop			.Load(dataDir + "PlayLoop.png");
	Image_PlayOnce			.Load(dataDir + "PlayOnce.png");
	Image_ChannelFilter		.Load(dataDir + "ChannelFilter.png");
	Image_Levels			.Load(dataDir + "Levels.png");
	Image_ContentView		.Load(dataDir + "ContentView.png");
	Image_UpFolder			.Load(dataDir + "UpFolder.png");
	Image_Crop				.Load(dataDir + "Crop.png");
	Image_AnchorBL			.Load(dataDir + "AnchorBL.png");
	Image_AnchorBM			.Load(dataDir + "AnchorBM.png");
	Image_AnchorML			.Load(dataDir + "AnchorML.png");
	Image_AnchorMM			.Load(dataDir + "AnchorMM.png");
	Image_DefaultThumbnail	.Load(dataDir + "DefaultThumbnail.png");
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


#include "RobotoFontBase85.cpp"


int main(int argc, char** argv)
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
		Viewer::CurrImageFile = Viewer::ParamImageFiles.Get();

		#ifdef PLATFORM_WINDOWS
		tString dest(MAX_PATH);
		int numchars = GetLongPathNameA(Viewer::CurrImageFile.Chr(), dest.Txt(), MAX_PATH);
		if (numchars > 0)
			Viewer::CurrImageFile = dest;
		#endif
	}

	#ifdef PACKAGE_SNAP
	// SNAP_USER_DATA is common to all revisions and is backed up. Used for viewer user-configuration file.
	// SNAP_USER_COMMON is common to all revisions of a snap and is not backed up. Used for viewer cache.
	tString snapUserData = tSystem::tGetEnvVar("SNAP_USER_DATA") + "/";
	tString snapUserCommon = tSystem::tGetEnvVar("SNAP_USER_COMMON") + "/";
	tString ldLibraryPath = tSystem::tGetEnvVar("LD_LIBRARY_PATH") + "/";
	tPrintf("SNAP_USER_DATA   : %s\n", snapUserData.Chr());
	tPrintf("SNAP_USER_COMMON : %s\n", snapUserCommon.Chr());
	tPrintf("LD_LIBRARY_PATH  : %s\n", ldLibraryPath.Chr());
	#endif

	// Setup window
	glfwSetErrorCallback(Viewer::GlfwErrorCallback);
	if (!glfwInit())
		return 1;

	int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
	tPrintf("Exe %s\n", tSystem::tGetProgramPath().Chr());
	tPrintf("Tacent View V %d.%d.%d\n", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);
	tPrintf("Tacent Library V %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear ImGui V %s\n", IMGUI_VERSION);
	tPrintf("GLFW V %d.%d.%d\n", glfwMajor, glfwMinor, glfwRev);
	tPrintf("For CLI Mode: tacentview.exe --cli --help\n");

	#ifdef PLATFORM_WINDOWS
	tString dataDir = tSystem::tGetProgramDir() + "Data/";
	Viewer::Image::ThumbCacheDir = dataDir + "Cache/";
	tString cfgFile = dataDir + "Viewer.cfg";

	#elif defined(PLATFORM_LINUX)
		#ifdef PACKAGE_SNAP
		tString progDir = tSystem::tGetProgramDir();
		tString dataDir = progDir + "Data/";

		tString cfgFile = snapUserData + "Viewer.cfg";
		Viewer::Image::ThumbCacheDir = snapUserCommon + "Cache/";

		#else
		tString progDir = tSystem::tGetProgramDir();
		bool isDev = (progDir != "/usr/bin/") ? true : false;
		tString dataDir = isDev ? (progDir + "Data/") : "/usr/share/tacentview/Data/";
		tString localAppDir = isDev ? dataDir : tSystem::tGetHomeDir() + ".tacentview/";
		if (!tSystem::tDirExists(localAppDir))
			tSystem::tCreateDir(localAppDir);	
		Viewer::Image::ThumbCacheDir = localAppDir + "Cache/";
		tString cfgFile = localAppDir + "Viewer.cfg";
		#endif
	#endif

	if (!tSystem::tDirExists(Viewer::Image::ThumbCacheDir))
		tSystem::tCreateDir(Viewer::Image::ThumbCacheDir);
	
	Viewer::Config::Load(cfgFile);

	// If no file from commandline, see if there is one set in the config.
	if (Viewer::CurrImageFile.IsEmpty() && Viewer::Config::Global.LastOpenPath.IsValid())
		Viewer::CurrImageFile = Viewer::Config::Global.LastOpenPath;

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

	// The title here seems to override the Linux hint above. When we create with the title string "tacentview",
	// glfw makes it the X11 WM_CLASS. This is needed so that the Ubuntu can map the same name in the .desktop file
	// to find things like the correct dock icon to display. The SetWindowTitle afterwards does not mod the WM_CLASS.
	Viewer::Window = glfwCreateWindow(Viewer::Config::Global.WindowW, Viewer::Config::Global.WindowH, "tacentview", nullptr, nullptr);
	if (!Viewer::Window)
		return 1;

	Viewer::SetWindowIcon(dataDir + "TacentView.ico");
	Viewer::SetWindowTitle();
	glfwSetWindowPos(Viewer::Window, Viewer::Config::Global.WindowX, Viewer::Config::Global.WindowY);

	#ifdef PLATFORM_WINDOWS
	// Make the window title bar show up in black.
	HWND hwnd = glfwGetWin32Window(Viewer::Window);
	const int DWMWA_USE_IMMERSIVE_DARK_MODE_A = 19;
	const int DWMWA_USE_IMMERSIVE_DARK_MODE_B = 20;
	BOOL isDarkMode = 1;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_A, &isDarkMode, sizeof(isDarkMode));
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_B, &isDarkMode, sizeof(isDarkMode));

	if (!tSystem::tDirExists(dataDir))
	{
		::MessageBoxA
		(
			hwnd,
			"Tacent Texture Viewer failed to launch because it was run from a location "
			"that did not have the Data directory in it. The executable should be in the "
			"same place as the Data directory.",
			"Viewer Message",
			MB_OK
		);

		glfwDestroyWindow(Viewer::Window);
		glfwTerminate();
		return 1;
	}
	#else
	if (!tSystem::tDirExists(dataDir))
	{
		glfwDestroyWindow(Viewer::Window);
		glfwTerminate();
		system
		(
			"zenity --ellipsize --title=\"Warning\" --warning --text=\""
			"Tacent Texture Viewer failed to launch because it was run from a\n"
			"location that did not have access to the Data directory.\""
		);

		tPrintf
		(
			"Tacent Texture Viewer failed to launch because it was run from a location "
			"that did not have the Data directory in it. The executable should be in the "
			"same place as the Data directory."
		);

		return 15;
	}
	#endif

	glfwMakeContextCurrent(Viewer::Window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		tPrintf("Failed to initialize GLAD\n");
		return 10;
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

	// Fonts must be added from smallest to largest and number of adds needs to match UIMode::NumModes.
	// I'm leaving the original AddFont calls here because they work and I haven't profiled which gives faster
	// bootup times. I imagine add font from memory is faster since 3 fewer files to open, but the exe load
	// for the in-memory method will be a bit slower plus it has to decode. In any case, pretty sure it will
	// still be faster.
	#define USE_IN_MEMORY_FONT_LOAD
	#ifdef USE_IN_MEMORY_FONT_LOAD
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoFont_compressed_data_base85, 14.0f);
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoFont_compressed_data_base85, 16.0f);
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoFont_compressed_data_base85, 18.0f);
	#else
	tString fontFile = dataDir + "Roboto-Medium.ttf";
	io.Fonts->AddFontFromFileTTF(fontFile.Chr(), 14.0f);
	io.Fonts->AddFontFromFileTTF(fontFile.Chr(), 16.0f);
	io.Fonts->AddFontFromFileTTF(fontFile.Chr(), 18.0f);
	#endif

	tiClamp(Viewer::Config::Current->UISize, 0, io.Fonts->Fonts.Size - 1);
	ImFont* font = io.Fonts->Fonts[Viewer::Config::Current->UISize];
	io.FontDefault = font;
	Viewer::CurrentFontIndex = Viewer::Config::Current->UISize;

	Viewer::LoadAppImages(dataDir);
	Viewer::PopulateImages();
	if (!Viewer::CurrImageFile.IsEmpty())
		Viewer::SetCurrentImage(Viewer::CurrImageFile);
	else
		Viewer::SetCurrentImage();

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

	if (Viewer::Config::Global.FullscreenMode)
		Viewer::ChangeScreenMode(true, true);

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

		// I don't seem to be able to get Linux to v-sync. This stops it using all the CPU.
		#ifdef PLATFORM_LINUX
		tSystem::tSleep(16);
		#endif

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
	if (!Viewer::Config::Global.FullscreenMode && !Viewer::WindowIconified)
	{
		glfwGetWindowPos(Viewer::Window, &Viewer::Config::Global.WindowX, &Viewer::Config::Global.WindowY);
		glfwGetWindowSize(Viewer::Window, &Viewer::Config::Global.WindowW, &Viewer::Config::Global.WindowH);
	}

	Viewer::Config::Global.TransparentWorkArea = Viewer::PendingTransparentWorkArea;
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
	return 0;
}
