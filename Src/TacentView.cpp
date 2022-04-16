// TacentView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2018, 2019, 2020, 2021, 2022 Tristan Grimmer.
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
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.

#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <Foundation/tVersion.cmake.h>
#include <Foundation/tHash.h>
#include <System/tCmdLine.h>
#include <Image/tPicture.h>
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
#include "Preferences.h"
#include "Properties.h"
#include "ContactSheet.h"
#include "MultiFrame.h"
#include "ContentView.h"
#include "Crop.h"
#include "Resize.h"
#include "Rotate.h"
#include "OpenSaveDialogs.h"
#include "Config.h"
#include "InputBindings.h"
#include "Version.cmake.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;


namespace Viewer
{
	tCmdLine::tParam ImageFileParam(1, "ImageFile", "File to open.");
	NavLogBar NavBar;
	tString ImagesDir;
	tList<tStringItem> ImagesSubDirs;
	tList<Image> Images;
	tItList<Image> ImagesLoadTimeSorted(tListMode::External);		// We don't need static here cuz the list is only used after main().
	tuint256 ImagesHash												= 0;
	Image* CurrImage												= nullptr;
	
	void LoadAppImages(const tString& dataDir);
	void UnloadAppImages();
	Image ReticleImage;
	Image PrevImage;
	Image NextImage;
	Image PrevArrowImage;
	Image NextArrowImage;
	Image FlipHImage;
	Image FlipVImage;
	Image RotateACWImage;
	Image RotateCWImage;
	Image RotateThetaImage;
	Image FullscreenImage;
	Image WindowedImage;
	Image SkipBeginImage;
	Image SkipEndImage;
	Image RefreshImage;
	Image RecycleImage;
	Image PropEditImage;
	Image InfoOverlayImage;
	Image HelpImage;
	Image PrefsImage;
	Image TileImage;
	Image StopImage;
	Image StopRevImage;
	Image PlayImage;
	Image PlayRevImage;
	Image PlayLoopImage;
	Image PlayOnceImage;
	Image ChannelFilterImage;
	Image ContentViewImage;
	Image UpFolderImage;
	Image CropImage;
	Image DefaultThumbnailImage;

	GLFWwindow* Window							= nullptr;
	double DisappearCountdown					= DisappearDuration;
	double SlideshowCountdown					= 0.0;
	float ReticleToMouseDist					= 75.0f;
	bool SlideshowPlaying						= false;
	bool FullscreenMode							= false;
	bool WindowIconified						= false;
	bool ShowCheatSheet							= false;
	bool ShowAbout								= false;
	
	#ifdef ENABLE_FILE_DIALOG_SUPPORT
	bool Request_OpenFileModal					= false;
	bool Request_OpenDirModal					= false;
	#endif

	bool Request_SaveAsModal					= false;
	bool Request_SaveAllModal					= false;
	bool Request_ResizeImageModal				= false;
	bool Request_ResizeCanvasModal				= false;
	bool Request_RotateImageModal				= false;
	bool Request_ContactSheetModal				= false;
	bool Request_MultiFrameModal				= false;
	bool Request_DeleteFileModal				= false;
	bool Request_DeleteFileNoRecycleModal		= false;
	bool Request_RenameModal					= false;
	bool Request_SnapMessage_NoFileBrowse		= false;
	bool Request_SnapMessage_NoFrameTrans		= false;
	bool Request_Quit							= false;
	bool PrefsWindow							= false;
	bool PropsWindow							= false;
	bool BindingsWindow							= false;
	bool BindingsWindowJustOpened				= false;
	bool CropMode								= false;
	bool LMBDown								= false;
	bool RMBDown								= false;
	bool DeleteAllCacheFilesOnExit				= false;
	bool PendingTransparentWorkArea				= false;
	bool DrawChannel_AsIntensity				= false;
	bool DrawChannel_R							= true;
	bool DrawChannel_G							= true;
	bool DrawChannel_B							= true;
	bool DrawChannel_A							= true;
	int DragAnchorX								= 0;
	int DragAnchorY								= 0;
	int CursorX									= 0;
	int CursorY									= 0;
	float RotateAnglePreview					= 0.0f;

	Config::Settings::ZoomMode CurrZoomMode		= Config::Settings::ZoomMode::DownscaleOnly;
	float ZoomPercent							= 100.0f;

	int Dispw									= 1;
	int Disph									= 1;
	int PanOffsetX								= 0;
	int PanOffsetY								= 0;
	int PanDragDownOffsetX						= 0;
	int PanDragDownOffsetY						= 0;
	float ReticleX								= -1.0f;
	float ReticleY								= -1.0f;
	tColouri PixelColour						= tColouri::black;

	const tVector4 ColourEnabledTint			= tVector4(1.00f, 1.00f, 1.00f, 1.00f);
	const tVector4 ColourDisabledTint			= tVector4(0.54f, 0.54f, 0.54f, 1.00f);
	const tVector4 ColourBG						= tVector4(0.00f, 0.00f, 0.00f, 0.00f);
	const tVector4 ColourPressedBG				= tVector4(0.21f, 0.45f, 0.21f, 1.00f);
	const tVector4 ColourClear					= tVector4(0.10f, 0.10f, 0.12f, 1.00f);

	const int MenuBarHeight						= 30;
	const float ZoomMin							= 10.0f;
	const float ZoomMax							= 2500.0f;
	uint64 FrameNumber							= 0;
	tVector2 ToolImageSize						(24.0f, 24.0f);

	void DrawBackground(float l, float r, float b, float t);
	void DrawNavBar(float x, float y, float w, float h);
	int GetNavBarHeight();
	void PrintRedirectCallback(const char* text, int numChars);
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }
	void SetWindowIcon(const tString& icoFile);

	// When compare functions are used to sort, they result in ascending order if they return a < b.
	bool Compare_AlphabeticalAscending(const tSystem::tFileInfo& a, const tSystem::tFileInfo& b)						{ return tStricmp(a.FileName.Chars(), b.FileName.Chars()) < 0; }
	bool Compare_FileCreationTimeAscending(const tSystem::tFileInfo& a, const tSystem::tFileInfo& b)					{ return a.CreationTime < b.CreationTime; }
	bool Compare_ImageLoadTimeAscending	(const Image& a, const Image& b)												{ return a.GetLoadedTime() < b.GetLoadedTime(); }
	bool Compare_ImageFileNameAscending	(const Image& a, const Image& b)												{ return tStricmp(a.Filename.Chars(), b.Filename.Chars()) < 0; }
	bool Compare_ImageFileNameDescending(const Image& a, const Image& b)												{ return tStricmp(a.Filename.Chars(), b.Filename.Chars()) > 0; }
	bool Compare_ImageFileTypeAscending	(const Image& a, const Image& b)												{ return int(a.Filetype) < int(b.Filetype); }
	bool Compare_ImageFileTypeDescending(const Image& a, const Image& b)												{ return int(a.Filetype) > int(b.Filetype); }
	bool Compare_ImageModTimeAscending	(const Image& a, const Image& b)												{ return a.FileModTime < b.FileModTime; }
	bool Compare_ImageModTimeDescending	(const Image& a, const Image& b)												{ return a.FileModTime > b.FileModTime; }
	bool Compare_ImageFileSizeAscending	(const Image& a, const Image& b)												{ return a.FileSizeB < b.FileSizeB; }
	bool Compare_ImageFileSizeDescending(const Image& a, const Image& b)												{ return a.FileSizeB > b.FileSizeB; }
	bool Compare_ImageAreaAscending		(const Image& a, const Image& b)												{ return a.CachePrimaryArea < b.CachePrimaryArea; }
	bool Compare_ImageAreaDescending	(const Image& a, const Image& b)												{ return a.CachePrimaryArea > b.CachePrimaryArea; }
	bool Compare_ImageWidthAscending	(const Image& a, const Image& b)												{ return a.CachePrimaryWidth < b.CachePrimaryWidth; }
	bool Compare_ImageWidthDescending	(const Image& a, const Image& b)												{ return a.CachePrimaryWidth > b.CachePrimaryWidth; }
	bool Compare_ImageHeightAscending	(const Image& a, const Image& b)												{ return a.CachePrimaryHeight < b.CachePrimaryHeight; }
	bool Compare_ImageHeightDescending	(const Image& a, const Image& b)												{ return a.CachePrimaryHeight > b.CachePrimaryHeight; }
	typedef bool ImageCompareFn			(const Image&, const Image&);

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

	enum CursorMove
	{
		CursorMove_None,
		CursorMove_Left,
		CursorMove_Right,
		CursorMove_Up,
		CursorMove_Down
	};
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

	void ChangeProfile(Config::Profile);
}


void Viewer::PrintRedirectCallback(const char* text, int numChars)
{
	NavBar.AddLog("%s", text);
	
	#ifdef PLATFORM_LINUX
	// We have a terminal in Linux so use it.
	printf("%s", text);
	#endif
}


tVector2 Viewer::GetDialogOrigin(float index)
{
	if (index == 2)
		return tVector2(DialogOrigin + DialogDelta, DialogOrigin + TopUIHeight + 275.0f);

	else if (index == 5)
		return tVector2(DialogOrigin + 416.0f, DialogOrigin + TopUIHeight + DialogDelta*1.0f);

	else if (index == 6)
		return tVector2(DialogOrigin + 300.0f, DialogOrigin + TopUIHeight + DialogDelta*1.0f);

	else if (index == 7)
		return tVector2(DialogOrigin + 388.0f, TopUIHeight + 12.0f);

	return tVector2(DialogOrigin + DialogDelta*float(index), DialogOrigin + TopUIHeight + DialogDelta*float(index));
}


int Viewer::GetNavBarHeight()
{
	if (FullscreenMode || !Config::Current->ShowNavBar)
		return 0;

	return NavBar.GetShowLog() ? 150 : 24;
}


void Viewer::DrawNavBar(float x, float y, float w, float h)
{
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(tVector2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(tVector2(x, y), ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(1, 1));
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
	if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
		imagesDir = tSystem::tGetDir(ImageFileParam.Get());

	tPrintf("Finding image files in %s\n", imagesDir.Chars());
	tSystem::tExtensions extensions;
	Image::GetCanLoad(extensions);
	tSystem::tFindFilesFast(foundFiles, imagesDir, extensions);

	return imagesDir;
}


tuint256 Viewer::ComputeImagesHash(const tList<tSystem::tFileInfo>& files)
{
	tuint256 hash = 0;
	for (tSystem::tFileInfo* item = files.First(); item; item = item->Next())
		hash = tHash::tHashString256(item->FileName.Chars(), hash);

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

	SortImages(Config::Settings::SortKeyEnum(Config::Current->SortKey), Config::Current->SortAscending);
	CurrImage = nullptr;
}


void Viewer::SortImages(Config::Settings::SortKeyEnum key, bool ascending)
{
	ImageCompareFn* sortFn;
	switch (key)
	{
		case Config::Settings::SortKeyEnum::FileName:
			sortFn = ascending ? Compare_ImageFileNameAscending : &Compare_ImageFileNameDescending;
			break;

		case Config::Settings::SortKeyEnum::FileModTime:
			sortFn = ascending ? Compare_ImageModTimeAscending : Compare_ImageModTimeDescending;
			break;

		case Config::Settings::SortKeyEnum::FileSize:
			sortFn = ascending ? Compare_ImageFileSizeAscending : Compare_ImageFileSizeDescending;
			break;

		case Config::Settings::SortKeyEnum::FileType:
			sortFn = ascending ? Compare_ImageFileTypeAscending : Compare_ImageFileTypeDescending;
			break;

		case Config::Settings::SortKeyEnum::ImageArea:
			sortFn = ascending ? Compare_ImageAreaAscending : Compare_ImageAreaDescending;
			break;

		case Config::Settings::SortKeyEnum::ImageWidth:
			sortFn = ascending ? Compare_ImageWidthAscending : Compare_ImageWidthDescending;
			break;

		case Config::Settings::SortKeyEnum::ImageHeight:
			sortFn = ascending ? Compare_ImageHeightAscending : Compare_ImageHeightDescending;
			break;
	}

	Images.Sort(sortFn);
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

		if (tStricmp(siName.Chars(), imgName.Chars()) == 0)
		{
			CurrImage = si;
			break;
		}
	}

	if (!CurrImage)
	{
		CurrImage = Images.First();
		if (!currFilename.IsEmpty())
			tPrintf("Could not display [%s].\n", tSystem::tGetFileName(currFilename).Chars());
		if (CurrImage && !CurrImage->Filename.IsEmpty())
			tPrintf("Displaying [%s] instead.\n", tSystem::tGetFileName(CurrImage->Filename).Chars());
	}

	if (CurrImage)
	{
		CurrZoomMode = Config::Settings::ZoomMode::DownscaleOnly;
		LoadCurrImage();
	}
}


void Viewer::AutoPropertyWindow()
{
	if (Config::Current->AutoPropertyWindow)
		PropsWindow = (CurrImage->TypeSupportsProperties() || (CurrImage->GetNumFrames() > 1));

	if (SlideshowPlaying)
		PropsWindow = false;
}


void Viewer::LoadCurrImage()
{
	tAssert(CurrImage);
	bool imgJustLoaded = false;
	if (!CurrImage->IsLoaded())
		imgJustLoaded = CurrImage->Load();

	if (Config::Current->DefaultZoomMode != int(Config::Settings::ZoomMode::User))
	{
		CurrZoomMode = Config::Settings::ZoomMode(Config::Current->DefaultZoomMode);

		if (CurrZoomMode == Config::Settings::ZoomMode::OneToOne)
			ZoomPercent = 100.0f;
	}

	AutoPropertyWindow();
	if
	(
		Config::Current->AutoPlayAnimatedImages && (CurrImage->GetNumFrames() > 1) &&
		(
			(CurrImage->Filetype == tFileType::GIF) ||
			(CurrImage->Filetype == tFileType::WEBP) ||
			(CurrImage->Filetype == tFileType::APNG) ||
			(CurrImage->Filetype == tFileType::PNG) ||		// PNGs that have APNGs inside (more than one frame), also autoplay.
			(CurrImage->Filetype == tFileType::TIFF)
		)
	)
	{
		CurrImage->FramePlayLooping = true;
		CurrImage->FramePlayRev = false;
		CurrImage->Play();
	}

	SetWindowTitle();
	ResetPan();

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
					tPrintf("Unloading %s freeing %d Bytes\n", tSystem::tGetFileName(i->Filename).Chars(), i->Info.MemSizeBytes);
					usedMem -= i->Info.MemSizeBytes;
					i->Unload();
					if (usedMem < allowedMem)
						break;
				}
			}
			tPrintf("Used mem %|64dB out of max %|64dB.\n", usedMem, allowedMem);
		}
	}
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
	if (!CurrImage || (CurrImage->GetNumFrames() <= 1))
		return;

	CurrImage->Stop();
	CurrImage->FrameNum = tClampMin(CurrImage->FrameNum-1, 0);
}


void Viewer::OnNextImageFrame()
{
	if (!CurrImage || (CurrImage->GetNumFrames() <= 1))
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


void Viewer::ShowHelpMark(const char* desc)
{
	ImGui::TextDisabled("[?]");
	if (!ImGui::IsItemHovered())
		return;

	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}


void Viewer::ShowToolTip(const char* desc)
{
	if (!ImGui::IsItemHovered())
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(3.0f, 3.0f));
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
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

	glfwSetWindowTitle(Window, title.Chars());
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


void Viewer::DrawBackground(float l, float r, float b, float t)
{
	if (Config::Current->TransparentWorkArea)
		return;

	switch (Config::Current->BackgroundStyle)
	{
		case int(Config::Settings::BGStyle::None):
			return;

		case int(Config::Settings::BGStyle::Checkerboard):
		{
			// Semitransparent checkerboard background.
			int x = 0;
			int y = 0;
			bool lineStartToggle = false;
			float checkSize = 16.0f;
			float bgH = t - b;
			float bgW = r - l;
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

					glBegin(GL_QUADS);
					glVertex2f(lc, bc);
					glVertex2f(lc, tc);
					glVertex2f(rc, tc);
					glVertex2f(rc, bc);
					glEnd();

					x++;
				}
				x = 0;
				y++;
				lineStartToggle = !lineStartToggle;
			}
			break;
		}

		case int(Config::Settings::BGStyle::SolidColour):
		{
			glColor4ubv(Config::Current->BackgroundColour.E);
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


void Viewer::ConvertScreenPosToImagePos
(
	int& imgX, int& imgY,
	const tVector2& scrPos, const tVector4& lrtb,
	const tVector2& uvMarg, const tVector2& uvOff
)
{
	float picX = scrPos.x - lrtb.L;
	//float picY = (scrPos.y - 1) - lrtb.B;
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

	float imagew = float(CurrImage->GetWidth());
	float imageh = float(CurrImage->GetHeight());

	float imposX = imagew * tMath::tLisc(normX, 0.0f + uvMarg.u + uvOff.u, 1.0f - uvMarg.u + uvOff.u);
	float imposY = imageh * tMath::tLisc(normY, 0.0f + uvMarg.v + uvOff.v, 1.0f - uvMarg.v + uvOff.v);

	imgX = int(imposX);
	imgY = int(imposY);
	if (!Config::Current->Tile)
	{
		tMath::tiClamp(imgX, 0, CurrImage->GetWidth() - 1);
		tMath::tiClamp(imgY, 0, CurrImage->GetHeight() - 1);
	}
	else
	{
		imgX %= CurrImage->GetWidth();
		if (imgX < 0) imgX += CurrImage->GetWidth();
		imgY %= CurrImage->GetHeight();
		if (imgY < 0) imgY += CurrImage->GetHeight();
	}
}


void Viewer::ConvertImagePosToScreenPos
(
	tVector2& scrPos,
	int imposX, int imposY, const tVector4& lrtb,
	const tVector2& uvMarg, const tVector2& uvOff
)
{
	tMath::tiClamp(imposX, 0, CurrImage->GetWidth());
	tMath::tiClamp(imposY, 0, CurrImage->GetHeight());
	float imgX = float(imposX);
	float imgY = float(imposY);

	float imagew = float(CurrImage->GetWidth());
	float imageh = float(CurrImage->GetHeight());

	float umin = 0.0f + uvMarg.u + uvOff.u;
	float umax = 1.0f - uvMarg.u + uvOff.u;
	float u = (imgX/imagew - umin) / (umax-umin);

	float vmin = 0.0f + uvMarg.v + uvOff.v;
	float vmax = 1.0f - uvMarg.v + uvOff.v;
	float v = (imgY/imageh - vmin) / (vmax-vmin);

	float picX = u * (lrtb.R-lrtb.L);
	float picY = v * (lrtb.T-lrtb.B);

	scrPos.x = tCeiling(picX + lrtb.L);
	//scrPos.y = picY + 1.0f + lrtb.B;
	scrPos.y = tCeiling(picY + lrtb.B);
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

	if (Config::Current->TransparentWorkArea)
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	else
		glClearColor(ColourClear.x, ColourClear.y, ColourClear.z, ColourClear.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int bottomUIHeight	= GetNavBarHeight();
	int topUIHeight		= (FullscreenMode || !Config::Current->ShowMenuBar) ? 0 : MenuBarHeight;

	ImGui_ImplOpenGL2_NewFrame();		
	ImGui_ImplGlfw_NewFrame();
	int dispw, disph;
	glfwGetFramebufferSize(window, &dispw, &disph);
	if ((dispw != Dispw) || (disph != Disph))
	{
		Dispw = dispw;
		Disph = disph;
		if ((PanOffsetX+PanDragDownOffsetX == 0) && (PanOffsetY+PanDragDownOffsetY == 0))
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
	float hmargin	= 0.0f;		float vmargin	= 0.0f;

	float left		= 0.0f;
	float right		= 0.0f;
	float top		= 0.0f;
	float bottom	= 0.0f;
	float uoff		= 0.0f;
	float voff		= 0.0f;
	float umarg		= 0.0f;
	float vmarg		= 0.0f;
	static bool skipUpdatePlaying = false;

	double mouseXd, mouseYd;
	glfwGetCursorPos(window, &mouseXd, &mouseYd);

	// Make origin lower-left.
	float workH = float(Disph - GetNavBarHeight());
	float mouseX = float(mouseXd);
	float mouseY = workH - float(mouseYd);

	int mouseXi = int(mouseX);
	int mouseYi = int(mouseY);

	if (CurrImage)
	{
		if (!skipUpdatePlaying)
			CurrImage->UpdatePlaying(float(dt));

		iw = float(CurrImage->GetWidth());
		ih = float(CurrImage->GetHeight());
		float picAspect = iw/ih;

		float cropExtraMargin = CropMode ? 5.0f : 0.0f;
		if (Config::Current->FixedAspectWorkArea)
		{
			if (workAreaAspect > picAspect)
			{
				drawh = float(workAreaH) - cropExtraMargin*2.0f;
				draww = picAspect * drawh;
				hmargin = (workAreaW - draww) * 0.5f;
				vmargin = cropExtraMargin;
			}
			else
			{
				draww = float(workAreaW) - cropExtraMargin*2.0f;
				drawh = draww / picAspect;
				vmargin = (workAreaH - drawh) * 0.5f;
				hmargin = cropExtraMargin;
			}
		}
		else
		{
			draww = float(workAreaW) - cropExtraMargin*2.0f;
			drawh = float(workAreaH) - cropExtraMargin*2.0f;
			vmargin = cropExtraMargin;
			hmargin = cropExtraMargin;
		}

		// w and h are the image width and height. draww and drawh are the drawable area width and height.
		left	= tMath::tRound(hmargin);
		right	= tMath::tRound(hmargin+draww);
		bottom	= tMath::tRound(vmargin);
		top		= tMath::tRound(vmargin+drawh);

		if (CurrZoomMode == Config::Settings::ZoomMode::DownscaleOnly)
		{
			if (Config::Current->FixedAspectWorkArea)
			{
				ZoomPercent = 100.0f;
				if (draww < iw)
					ZoomPercent = 100.0f * draww / iw;
			}
			else
			{
				ZoomPercent = 100.0f;
				float zoomh = draww / iw;
				float zoomv = drawh / ih;
				if ((iw > draww) || (ih > drawh))
					ZoomPercent = 100.0f * tMath::tMin(zoomh, zoomv);
			}
		}
		else if (CurrZoomMode == Config::Settings::ZoomMode::Fit)
		{
			if (Config::Current->FixedAspectWorkArea)
			{
				ZoomPercent = 100.0f * draww / iw;
			}
			else
			{
				float zoomh = draww / iw;
				float zoomv = drawh / ih;
				ZoomPercent = 100.0f * tMath::tMin(zoomh, zoomv);
			}
		}

		float w = iw * ZoomPercent/100.0f;
		float h = ih * ZoomPercent/100.0f;

		// If the image is smaller than the drawable area we draw a quad of the correct size with full 0..1 range in the uvs.
		// Setting the UV margins is then what allows the fixed-aspect work area to work properly.
		if (Config::Current->FixedAspectWorkArea && (w > draww))
		{
			float propw = draww / w;
			umarg = (1.0f - propw)/2.0f;
			float proph = drawh / h;
			vmarg = (1.0f - proph)/2.0f;
		}
		else
		{
			// Compute extents.
			float offsetW = tMath::tRound((draww - w) / 2.0f);
			left	+= offsetW;
			right	= left + w;		// Fix by Oddwarg. I had "right -= offsetW".

			float offsetH = tMath::tRound((drawh - h) / 2.0f);
			bottom	+= offsetH;
			top		= bottom + h;	// Fix by Oddwarg. I had "top -= offsetH".
		}

		// Modify the UVs here to magnify.
		if ((draww < w) || Config::Current->Tile)
		{
			if (RMBDown)
				PanDragDownOffsetX = mouseXi - DragAnchorX;

			if (!Config::Current->Tile)
				tMath::tiClamp(PanDragDownOffsetX, int(-(w-draww)/2.0f) - PanOffsetX, int((w-draww)/2.0f) - PanOffsetX);
		}

		if ((drawh < h) || Config::Current->Tile)
		{
			if (RMBDown)
				PanDragDownOffsetY = mouseYi - DragAnchorY;

			if (!Config::Current->Tile)
				tMath::tiClamp(PanDragDownOffsetY, int(-(h-drawh)/2.0f) - PanOffsetY, int((h-drawh)/2.0f) - PanOffsetY);
		}

		if ((draww > w) && !Config::Current->Tile)
			ResetPan(true, false);

		if ((drawh > h) && !Config::Current->Tile)
			ResetPan(false, true);

		uoff = -float(PanOffsetX+PanDragDownOffsetX)/w;
		voff = -float(PanOffsetY+PanDragDownOffsetY)/h;

		// Draw background.
		glDisable(GL_TEXTURE_2D);
		if ((Config::Current->BackgroundExtend || Config::Current->Tile) && !CropMode)
			DrawBackground(hmargin, hmargin+draww, vmargin, vmargin+drawh);
		else
			DrawBackground(left, right, bottom, top);

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

		bool modifiedSwizzle = !DrawChannel_R || !DrawChannel_G || !DrawChannel_B || !DrawChannel_A;
		if (modifiedSwizzle)
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	
		glBegin(GL_QUADS);
		if (!Config::Current->Tile)
		{
			glTexCoord2f(0.0f + umarg + uoff, 0.0f + vmarg + voff); glVertex2f(left,  bottom);
			glTexCoord2f(0.0f + umarg + uoff, 1.0f - vmarg + voff); glVertex2f(left,  top);
			glTexCoord2f(1.0f - umarg + uoff, 1.0f - vmarg + voff); glVertex2f(right, top);
			glTexCoord2f(1.0f - umarg + uoff, 0.0f + vmarg + voff); glVertex2f(right, bottom);
		}
		else
		{
			float repU = draww/(right-left);	float offU = (1.0f-repU)/2.0f;
			float repV = drawh/(top-bottom);	float offV = (1.0f-repV)/2.0f;
			glTexCoord2f(offU + 0.0f + umarg + uoff,	offV + 0.0f + vmarg + voff);	glVertex2f(hmargin,			vmargin);
			glTexCoord2f(offU + 0.0f + umarg + uoff,	offV + repV - vmarg + voff);	glVertex2f(hmargin,			vmargin+drawh);
			glTexCoord2f(offU + repU - umarg + uoff,	offV + repV - vmarg + voff);	glVertex2f(hmargin+draww,	vmargin+drawh);
			glTexCoord2f(offU + repU - umarg + uoff,	offV + 0.0f + vmarg + voff);	glVertex2f(hmargin+draww,	vmargin);
		}
		glEnd();

		// Restore swizzle to normal if it was modified.
		if (modifiedSwizzle)
		{
			int defaultSwizzle[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, defaultSwizzle);
		}

		if (RotateAnglePreview != 0.0f)
	 		glPopMatrix();

		// If a request was made to move the cursor/reticle, process the request here,
		if (RequestCursorMove)
		{
			switch (RequestCursorMove)
			{
				case CursorMove_Left:	CursorX--;	break;
				case CursorMove_Right:	CursorX++;	break;
				case CursorMove_Up:		CursorY++;	break;
				case CursorMove_Down:	CursorY--;	break;
			}
			tiClamp(CursorX, 0, CurrImage->GetWidth() - 1);
			tiClamp(CursorY, 0, CurrImage->GetHeight() - 1);
			tVector2 reticle;
			ConvertImagePosToScreenPos(reticle, CursorX, CursorY, tVector4(left, right, top, bottom), tVector2(umarg, vmarg), tVector2(uoff, voff));
			ReticleX = reticle.x;
			ReticleY = reticle.y;
			RequestCursorMove = CursorMove_None;
		}

		// Get the colour under the reticle.
		tVector2 scrCursorPos(ReticleX, ReticleY);
		ConvertScreenPosToImagePos
		(
			CursorX, CursorY, scrCursorPos, tVector4(left, right, top, bottom),
			tVector2(umarg, vmarg), tVector2(uoff, voff)
		);

		PixelColour = CurrImage->GetPixel(CursorX, CursorY);

		// Show the reticle.
		glDisable(GL_TEXTURE_2D);
		glColor4fv(tColour::white.E);

		tVector2 mousePos(mouseX, mouseY);
		tVector2 reticPos(ReticleX, ReticleY);
		float retMouseDistSq = tMath::tDistBetweenSq(mousePos, reticPos);
		if
		(
			// Must not be cropping.
			!CropMode &&

			// Must have a colour inspector visible (menu bar and details both have one).
			((Config::Current->ShowMenuBar && !FullscreenMode) || Config::Current->ShowImageDetails) &&

			// And any of the following: a) details is on, b) disappear countdown not finished, or c) mouse is close.
			(
				Config::Current->ShowImageDetails || (DisappearCountdown > 0.0) ||

				// Continue to draw the reticle if mouse is close enough (even if timer expired).
				(retMouseDistSq < ReticleToMouseDist*ReticleToMouseDist)
			)
		)
		{
			tColouri hsv = PixelColour;
			hsv.RGBToHSV();
			if (hsv.V > 150)
				glColor4ubv(tColouri::black.E);
			else
				glColor4ubv(tColouri::white.E);

			if (ZoomPercent >= 500.0f)
			{
				// Draw the reticle as a box.
				tVector2 scrPosBL;
				ConvertImagePosToScreenPos
				(
					scrPosBL, CursorX, CursorY, tVector4(left, right, top, bottom),
					tVector2(umarg, vmarg), tVector2(uoff, voff)
				);
				tVector2 scrPosTR;
				ConvertImagePosToScreenPos
				(
					scrPosTR, CursorX+1, CursorY+1, tVector4(left, right, top, bottom),
					tVector2(umarg, vmarg), tVector2(uoff, voff)
				);

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
				// Draw the reticle.
				float cw = float((ReticleImage.GetWidth()) >> 1);
				float ch = float((ReticleImage.GetHeight()) >> 1);
				float cx = ReticleX;
				float cy = ReticleY;
				glEnable(GL_TEXTURE_2D);
				ReticleImage.Bind();
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
			if (!lastCropMode)
				CropGizmo.SetLines(tVector4(left, right, top, bottom));

			CropGizmo.UpdateDraw
			(
				tVector4(left, right, top, bottom), tVector2(mouseX, mouseY),
				tVector2(umarg, vmarg), tVector2(uoff, voff)
			);
		}
		lastCropMode = CropMode;
	}

	ImGui::NewFrame();
	
	// Show the big demo window. You can browse its code to learn more about Dear ImGui.
	static bool showDemoWindow = false;
	//showDemoWindow = true;
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

	tVector2 rectCenterPrevArrow(0.0f, float(workAreaH)*0.5f);
	tARect2 hitAreaPrevArrow(rectCenterPrevArrow, 160.0f);
	if
	(
		!CropMode &&
		((DisappearCountdown > 0.0) || hitAreaPrevArrow.IsPointInside(mousePos)) &&
		((CurrImage != Images.First()) || (SlideshowPlaying && Config::Current->SlideshowLooping))
	)
	{
		// Previous arrow.
		ImGui::SetNextWindowPos(tVector2(0.0f, float(topUIHeight) + float(workAreaH)*0.5f - 33.0f));
		ImGui::SetNextWindowSize(tVector2(16, 70), ImGuiCond_Always);
		ImGui::Begin("PrevArrow", nullptr, flagsImgButton);
		ImGui::SetCursorPos(tVector2(6, 2));
		if (ImGui::ImageButton(ImTextureID(PrevArrowImage.Bind()), tVector2(15,56), tVector2(0,0), tVector2(1,1), 3, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			OnPrevious();
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
		ImGui::SetNextWindowPos(tVector2(workAreaW - 33.0f, float(topUIHeight) + float(workAreaH) * 0.5f - 33.0f));
		ImGui::SetNextWindowSize(tVector2(16, 70), ImGuiCond_Always);
		ImGui::Begin("NextArrow", nullptr, flagsImgButton);
		ImGui::SetCursorPos(tVector2(6, 2));
		if (ImGui::ImageButton(ImTextureID(NextArrowImage.Bind()), tVector2(15,56), tVector2(0,0), tVector2(1,1), 3, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			OnNext();
		ImGui::End();
	}

	// Scrubber
	if
	(
		!CropMode && PropsWindow &&
		Config::Current->ShowFrameScrubber && CurrImage && (CurrImage->GetNumFrames() > 1) && !CurrImage->IsAltPictureEnabled()
	)
	{
		ImGui::SetNextWindowPos(tVector2(0.0f, float(topUIHeight) + float(workAreaH) - 34.0f));
		ImGui::SetNextWindowSize(tVector2(float(workAreaW), 5.0f), ImGuiCond_Always);
		ImGui::Begin("Scrubber", nullptr, flagsImgButton);
		ImGui::PushItemWidth(-1);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(7.0f, 2.0f));
		int frmNum = CurrImage->FrameNum + 1;
		if (ImGui::SliderInt("", &frmNum, 1, CurrImage->GetNumFrames(), "%d", ImGuiSliderFlags_ClampOnInput))
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
		// Looping button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-120.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Repeat", nullptr, flagsImgButton);
		uint64 playModeImageID = Config::Current->SlideshowLooping ? PlayOnceImage.Bind() : PlayLoopImage.Bind();
		if (ImGui::ImageButton(ImTextureID(playModeImageID), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			Config::Current->SlideshowLooping = !Config::Current->SlideshowLooping;
		ImGui::End();

		// Skip to beginning button.
		bool prevAvail = (CurrImage != Images.First()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-80.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("SkipBegin", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(SkipBeginImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnSkipBegin();
		ImGui::End();

		// Prev button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-40.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Prev", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(PrevImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnPrevious();
		ImGui::End();

		// Slideshow Play/Stop button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+0.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Slideshow", nullptr, flagsImgButton);
		uint64 psImageID = SlideshowPlaying ? StopImage.Bind() : PlayImage.Bind();
		if (ImGui::ImageButton(ImTextureID(psImageID), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
		{
			SlideshowPlaying = !SlideshowPlaying;
			SlideshowCountdown = Config::Current->SlideshowPeriod;
		}
		ImGui::End();

		// Next button.
		bool nextAvail = (CurrImage != Images.Last()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+40.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Next", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(NextImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnNext();
		ImGui::End();

		// Skip to end button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+80.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("SkipEnd", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(SkipEndImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnSkipEnd();
		ImGui::End();

		// Fullscreen / Windowed button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+120.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Fullscreen", nullptr, flagsImgButton);
		uint64 fsImageID = FullscreenMode ? WindowedImage.Bind() : FullscreenImage.Bind();
		if (ImGui::ImageButton(ImTextureID(fsImageID), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			ChangeScreenMode(!FullscreenMode);
		ImGui::End();

		// Exit basic profile.
		if (Config::GetProfile() == Config::Profile::Basic)
		{
			ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+160.0f, float(topUIHeight) + float(workAreaH) - buttonHeightOffset));
			ImGui::SetNextWindowSize(tVector2(120, 40), ImGuiCond_Always);
			ImGui::Begin("ExitBasic", nullptr, flagsImgButton);
			if (ImGui::Button("ESC", tVector2(50,28)))
				ChangeProfile(Config::Profile::Main);
			ImGui::End();
		}
	}

	ImGui::SetNextWindowPos(tVector2(0, 0));

	if (!FullscreenMode && Config::Current->ShowMenuBar)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,8));
		ImGui::BeginMainMenuBar();

		//
		// File Menu.
		//
		#ifdef ENABLE_FILE_DIALOG_SUPPORT
		bool openFilePressed			= Request_OpenFileModal;			Request_OpenFileModal				= false;
		bool openDirPressed				= Request_OpenDirModal;				Request_OpenDirModal				= false;
		#endif

		bool saveAsPressed				= Request_SaveAsModal;				Request_SaveAsModal					= false;
		bool saveAllPressed				= Request_SaveAllModal;				Request_SaveAllModal				= false;
		bool saveContactSheetPressed	= Request_ContactSheetModal;		Request_ContactSheetModal			= false;
		bool saveMultiFramePressed		= Request_MultiFrameModal;			Request_MultiFrameModal				= false;
		bool snapMessageNoFileBrowse	= Request_SnapMessage_NoFileBrowse;	Request_SnapMessage_NoFileBrowse	= false;
		bool snapMessageNoFrameTrans	= Request_SnapMessage_NoFrameTrans;	Request_SnapMessage_NoFrameTrans	= false;
		if (ImGui::BeginMenu("File"))
		{
			// Show file menu items...
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

			#ifdef ENABLE_FILE_DIALOG_SUPPORT
			if (ImGui::MenuItem("Open File...", "Ctrl-O"))
				openFilePressed = true;

			if (ImGui::MenuItem("Open Dir...", "Alt-O"))
				openDirPressed = true;
			#endif

			if (ImGui::MenuItem("Save As...", "Ctrl-S") && CurrImage)
				saveAsPressed = true;

			if (ImGui::MenuItem("Save All...", "Alt-S") && CurrImage)
				saveAllPressed = true;

			if (ImGui::MenuItem("Save Contact Sheet...", "C") && (Images.GetNumItems() > 1))
				saveContactSheetPressed = true;

			if (ImGui::MenuItem("Save Multi-Frame...", "Ctrl-M") && (Images.GetNumItems() > 1))
				saveMultiFramePressed = true;

			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Alt-F4"))
				glfwSetWindowShouldClose(Window, 1);

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

		#ifdef ENABLE_FILE_DIALOG_SUPPORT
		DoOpenFileModal(openFilePressed);
		DoOpenDirModal(openDirPressed);
		#endif

		DoSaveAsModal(saveAsPressed);
		DoSaveAllModal(saveAllPressed);
		DoContactSheetModal(saveContactSheetPressed);
		DoMultiFrameModal(saveMultiFramePressed);
		DoSnapMessageNoFileBrowseModal(snapMessageNoFileBrowse);
		DoSnapMessageNoFrameTransModal(snapMessageNoFrameTrans);
		ImGui::PopStyleVar();

		//
		// Edit Menu.
		//
		bool resizeImagePressed		= Request_ResizeImageModal;		Request_ResizeImageModal = false;
		bool resizeCanvasPressed	= Request_ResizeCanvasModal;	Request_ResizeCanvasModal = false;
		bool rotateImagePressed		= Request_RotateImageModal;		Request_RotateImageModal = false;
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

			bool undoEnabled = CurrImage && CurrImage->IsUndoAvailable();
			tString undoDesc = undoEnabled ? CurrImage->GetUndoDesc() : tString();
			tString undoStr; tsPrintf(undoStr, "Undo %s", undoDesc.Chars());
			if (ImGui::MenuItem(undoStr, "Ctrl-Z", false, undoEnabled))
				Undo();

			bool redoEnabled = CurrImage && CurrImage->IsRedoAvailable();
			tString redoDesc = redoEnabled ? CurrImage->GetRedoDesc() : tString();
			tString redoStr; tsPrintf(redoStr, "Redo %s", redoDesc.Chars());
			if (ImGui::MenuItem(redoStr, "Ctrl-Y", false, redoEnabled))
				Redo();

			if (ImGui::MenuItem("Flip Vertically", "Ctrl <", false, CurrImage && !CurrImage->IsAltPictureEnabled()))
			{
				CurrImage->Unbind();
				CurrImage->Flip(false);
				CurrImage->Bind();
				SetWindowTitle();
			}

			if (ImGui::MenuItem("Flip Horizontally", "Ctrl >", false, CurrImage && !CurrImage->IsAltPictureEnabled()))
			{
				CurrImage->Unbind();
				CurrImage->Flip(true);
				CurrImage->Bind();
				SetWindowTitle();
			}

			if (ImGui::MenuItem("Rotate Anti-Clockwise", "<", false, CurrImage && !CurrImage->IsAltPictureEnabled()))
			{
				CurrImage->Unbind();
				CurrImage->Rotate90(true);
				CurrImage->Bind();
				SetWindowTitle();
			}

			if (ImGui::MenuItem("Rotate Clockwise", ">", false, CurrImage && !CurrImage->IsAltPictureEnabled()))
			{
				CurrImage->Unbind();
				CurrImage->Rotate90(false);
				CurrImage->Bind();
				SetWindowTitle();
			}

			ImGui::MenuItem("Crop...", "/", &CropMode);

			if (ImGui::MenuItem("Resize Image...", "Alt-R") && CurrImage)
				resizeImagePressed = true;

			if (ImGui::MenuItem("Resize Canvas...", "Ctrl-R") && CurrImage)
				resizeCanvasPressed = true;

			if (ImGui::MenuItem("Rotate Image...", "R") && CurrImage)
				rotateImagePressed = true;

			ImGui::MenuItem("Edit Pixel", "A", &Config::Current->ShowPixelEditor);
			ImGui::MenuItem("Channel Filter...", "~", &Config::Current->ShowChannelFilter);
			ImGui::MenuItem("Image Properties...", "E", &PropsWindow);

			ImGui::Separator();

			bool mainProfile = Config::GetProfile() == Config::Profile::Main;
			if (ImGui::MenuItem("Main Profile", mainProfile ? nullptr : "B", &mainProfile))
				ChangeProfile(mainProfile ? Config::Profile::Main : Config::Profile::Basic);

			bool basicProfile = Config::GetProfile() == Config::Profile::Basic;
			if (ImGui::MenuItem("Basic Profile", basicProfile ? nullptr : "B", &basicProfile))
				ChangeProfile(basicProfile ? Config::Profile::Basic : Config::Profile::Main);

			ImGui::MenuItem("Preferences...", "P", &PrefsWindow);

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));
		DoResizeImageModal(resizeImagePressed);
		DoResizeCanvasModal(resizeCanvasPressed);
		DoRotateImageModal(rotateImagePressed);
		ImGui::PopStyleVar();

		//
		// View Menu.
		//
		if (ImGui::BeginMenu("View"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));
			ImGui::MenuItem("Menu Bar", "M", &Config::Current->ShowMenuBar, !CropMode);
			ImGui::MenuItem("Nav Bar", "N", &Config::Current->ShowNavBar, !CropMode);
			ImGui::MenuItem("Slideshow Progress", "S", &Config::Current->SlideshowProgressArc, !CropMode);
			bool basicSettings = (Config::GetProfile() == Config::Profile::Basic);
			if (ImGui::MenuItem("Basic View Mode", "B", &basicSettings, !CropMode))
				ChangeProfile(basicSettings ? Config::Profile::Basic : Config::Profile::Main);

			ImGui::MenuItem("Image Details", "I", &Config::Current->ShowImageDetails);
			ImGui::MenuItem("Content View", "V", &Config::Current->ContentViewShow);

			ImGui::Separator();

			bool userMode = CurrZoomMode == Config::Settings::ZoomMode::User;
			if (ImGui::MenuItem("Zoom User", "", &userMode))
			{
				ResetPan();
				CurrZoomMode = Config::Settings::ZoomMode::User;
			}

			bool fitMode = CurrZoomMode == Config::Settings::ZoomMode::Fit;
			if (ImGui::MenuItem("Zoom Fit", "F", &fitMode))
			{
				ResetPan();
				CurrZoomMode = Config::Settings::ZoomMode::Fit;
			}

			bool downscale = CurrZoomMode == Config::Settings::ZoomMode::DownscaleOnly;
			if (ImGui::MenuItem("Zoom Downscale", "D", &downscale))
			{
				ResetPan();
				CurrZoomMode = Config::Settings::ZoomMode::DownscaleOnly;
			}

			bool oneToOne = CurrZoomMode == Config::Settings::ZoomMode::OneToOne;
			if (ImGui::MenuItem("Zoom 1:1", "Z", &oneToOne))
			{
				ZoomPercent = 100.0f;
				ResetPan();
				CurrZoomMode = Config::Settings::ZoomMode::OneToOne;
			}

			ImGui::PushItemWidth(60);
			const char* zoomItems[] = { "Zoom", "20%", "50%", "100%", "150%", "200%", "400%", "800%", "1200%", "1800%", "2500%"};
			float zoomVals[] = { -1.0f, 20.0f, 50.0f, 100.0f, 150.0f, 200.0f, 400.0f, 800.0f, 1200.0f, 1800.0f, 2500.0f };
			tString currZoomStr;
			tsPrintf(currZoomStr, "%0.0f%%", ZoomPercent);
			int zoomIdx = 0;
			if (ImGui::Combo(currZoomStr.Chars(), &zoomIdx, zoomItems, tNumElements(zoomItems)) && (zoomIdx > 0))
				ApplyZoomDelta(zoomVals[zoomIdx]-ZoomPercent);
			ImGui::PopItemWidth();

			ImGui::Separator();
			if (ImGui::Button("Reset Pan"))
				ResetPan();

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		//
		// Help Menu.
		//
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));
			ImGui::MenuItem("Cheat Sheet", "F1", &ShowCheatSheet);
			if (ImGui::MenuItem("Key Bindings...", "K", &BindingsWindow))
				if (BindingsWindow)
					BindingsWindowJustOpened = true;
			ImGui::MenuItem("About", "", &ShowAbout);
			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		//
		// Toolbar.
		//
		tColourf floatCol(PixelColour);
		tVector4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, tVector2(26,26)))
			ImGui::OpenPopup("CopyColourAs");

		if (ImGui::BeginPopup("CopyColourAs"))
			ColourCopyAs();

		if (ImGui::ImageButton
		(
			ImTextureID(ChannelFilterImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			Config::Current->ShowChannelFilter ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowChannelFilter = !Config::Current->ShowChannelFilter;
		ShowToolTip("Colour Channel Filter");

		if (ImGui::ImageButton
		(
			ImTextureID(ContentViewImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			Config::Current->ContentViewShow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ContentViewShow = !Config::Current->ContentViewShow;
		ShowToolTip("Content Thumbnail View");

		bool tileAvail = CurrImage ? !CropMode : false;
		if (ImGui::ImageButton
		(
			ImTextureID(TileImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			Config::Current->Tile ? ColourPressedBG : ColourBG, tileAvail ? ColourEnabledTint : ColourDisabledTint) && tileAvail
		)
		{
			Config::Current->Tile = !Config::Current->Tile;
			if (!Config::Current->Tile)
				ResetPan();
		}
		ShowToolTip("Show Images Tiled");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		bool transAvail = CurrImage ? !CurrImage->IsAltPictureEnabled() : false;
		if (ImGui::ImageButton
		(
			ImTextureID(FlipVImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1, ColourBG,
			transAvail ? ColourEnabledTint : ColourDisabledTint) && transAvail
		)
		{
			CurrImage->Unbind();
			CurrImage->Flip(false);
			CurrImage->Bind();
			SetWindowTitle();
		}
		ShowToolTip("Flip Vertically");

		if (ImGui::ImageButton
		(
			ImTextureID(FlipHImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1, ColourBG,
			transAvail ? ColourEnabledTint : ColourDisabledTint) && transAvail
		)
		{
			CurrImage->Unbind();
			CurrImage->Flip(true);
			CurrImage->Bind();
			SetWindowTitle();
		}
		ShowToolTip("Flip Horizontally");

		if (ImGui::ImageButton
		(
			ImTextureID(RotateACWImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1, ColourBG,
			transAvail ? ColourEnabledTint : ColourDisabledTint) && transAvail
		)
		{
			CurrImage->Unbind();
			CurrImage->Rotate90(true);
			CurrImage->Bind();
			SetWindowTitle();
		}
		ShowToolTip("Rotate 90 Anticlockwise");

		if (ImGui::ImageButton
		(
			ImTextureID(RotateCWImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1, ColourBG,
			transAvail ? ColourEnabledTint : ColourDisabledTint) && transAvail
		)
		{
			CurrImage->Unbind();
			CurrImage->Rotate90(false);
			CurrImage->Bind();
			SetWindowTitle();
		}
		ShowToolTip("Rotate 90 Clockwise");

		if (ImGui::ImageButton
		(
			ImTextureID(RotateThetaImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1, ColourBG,
			transAvail ? ColourEnabledTint : ColourDisabledTint) && transAvail
		)
		{
			Request_RotateImageModal = true;
		}
		ShowToolTip("Rotate Theta");

		bool cropAvail = CurrImage && transAvail && !Config::Current->Tile;
		if (ImGui::ImageButton
		(
			ImTextureID(CropImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			CropMode ? ColourPressedBG : ColourBG, cropAvail ? ColourEnabledTint : ColourDisabledTint) && cropAvail
		)	CropMode = !CropMode;
		ShowToolTip("Crop");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		if (ImGui::ImageButton
		(
			ImTextureID(PropEditImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			PropsWindow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	PropsWindow = !PropsWindow;
		ShowToolTip("Image Properties");

		if (ImGui::ImageButton
		(
			ImTextureID(InfoOverlayImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			Config::Current->ShowImageDetails ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config::Current->ShowImageDetails = !Config::Current->ShowImageDetails;
		ShowToolTip("Information Overlay");

		bool refreshAvail = CurrImage ? true : false;
		if (ImGui::ImageButton
		(
			ImTextureID(RefreshImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
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
		if (ImGui::ImageButton
		(
			ImTextureID(RecycleImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			ColourBG, recycleAvail ? ColourEnabledTint : ColourDisabledTint) && recycleAvail
		)	Request_DeleteFileModal = true;
		ShowToolTip("Delete Current File");

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.0f);

		if (ImGui::ImageButton
		(
			ImTextureID(HelpImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			ShowCheatSheet ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	ShowCheatSheet = !ShowCheatSheet;
		ShowToolTip("Help");

		if (ImGui::ImageButton
		(
			ImTextureID(PrefsImage.Bind()), ToolImageSize, tVector2(0, 1), tVector2(1, 0), 1,
			PrefsWindow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	PrefsWindow = !PrefsWindow;
		ShowToolTip("Preferences");

		ImGui::EndMainMenuBar();
		ImGui::PopStyleVar();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

	if (PrefsWindow)
		ShowPreferencesWindow(&PrefsWindow);

	if (PropsWindow)
		ShowPropertiesWindow(&PropsWindow);

	if (BindingsWindow)
	{
		Bindings::ShowBindingsWindow(&BindingsWindow, BindingsWindowJustOpened);
		BindingsWindowJustOpened = false;
	}

	ImGui::PopStyleVar();

	if (!FullscreenMode && Config::Current->ShowNavBar)
		DrawNavBar(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

	// We allow the overlay and cheatsheet in fullscreen.
	if (Config::Current->ShowImageDetails)
		ShowImageDetailsOverlay(&Config::Current->ShowImageDetails, 0.0f, float(topUIHeight), float(dispw), float(disph - bottomUIHeight - topUIHeight), CursorX, CursorY, ZoomPercent);

	if (Config::Current->ShowPixelEditor)
		ShowPixelEditorOverlay(&Config::Current->ShowPixelEditor);

	if (Config::Current->ShowChannelFilter)
		ShowChannelFilterOverlay(&Config::Current->ShowChannelFilter);

	if (Config::Current->ContentViewShow)
		ShowContentViewDialog(&Config::Current->ContentViewShow);

	if (ShowCheatSheet)
		Bindings::ShowCheatSheetWindow(&ShowCheatSheet);

	if (ShowAbout)
		ShowAboutPopup(&ShowAbout);

	ShowCropPopup(tVector4(left, right, top, bottom), tVector2(umarg, vmarg), tVector2(uoff, voff));

	if (Request_DeleteFileModal)
	{
		Request_DeleteFileModal = false;
		if (!Config::Current->ConfirmDeletes)
			DeleteImageFile(CurrImage->Filename, true);
		else
			ImGui::OpenPopup("Delete File");
	}
	// The unused isOpenDeleteFile bool is just so we get a close button in ImGui.
	bool isOpenDeleteFile = true;
	if (ImGui::BeginPopupModal("Delete File", &isOpenDeleteFile, ImGuiWindowFlags_AlwaysAutoResize))
		DoDeleteFileModal();

	if (Request_DeleteFileNoRecycleModal)
	{
		Request_DeleteFileNoRecycleModal = false;
		ImGui::OpenPopup("Delete File Permanently");
	}
	// The unused isOpenPerm bool is just so we get a close button in ImGui.
	bool isOpenPerm = true;
	if (ImGui::BeginPopupModal("Delete File Permanently", &isOpenPerm, ImGuiWindowFlags_AlwaysAutoResize))
		DoDeleteFileNoRecycleModal();

	bool renameJustOpened = false;
	if (Request_RenameModal)
	{
		renameJustOpened = true;
		Request_RenameModal = false;
	}

	if (renameJustOpened)
		ImGui::OpenPopup("Rename File");	
	// The unused isOpenRen bool is just so we get a close button in ImGui.
	bool isOpenRen = true;
	if (ImGui::BeginPopupModal("Rename File", &isOpenRen, ImGuiWindowFlags_AlwaysAutoResize))
		DoRenameModal(renameJustOpened);

	ImGui::Render();
	glViewport(0, 0, dispw, disph);
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
	FrameNumber++;

	// We're done the frame. Is slideshow playing.
	if (!ImGui::IsAnyPopupOpen() && SlideshowPlaying)
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
		ImageFileParam.Param = nextImgFile;		// We set this so if we lose and gain focus, we go back to the current image.
		PopulateImages();
		SetCurrentImage(nextImgFile);
	}

	return deleted;
}


bool Viewer::ChangeScreenMode(bool fullscreen, bool force)
{
	if (!force && (FullscreenMode == fullscreen))
		return false;

	// If currently in windowed mode, remember our window geometry.
	if (!force && !FullscreenMode)
	{
		glfwGetWindowPos(Viewer::Window, &Viewer::Config::Current->WindowX, &Viewer::Config::Current->WindowY);
		glfwGetWindowSize(Viewer::Window, &Viewer::Config::Current->WindowW, &Viewer::Config::Current->WindowH);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	if (fullscreen)
	{
		if (Config::Current->TransparentWorkArea)
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
		if (Config::Current->TransparentWorkArea)
		{
			glfwSetWindowSize(Viewer::Window, Viewer::Config::Current->WindowW, Viewer::Config::Current->WindowH);
			glfwSetWindowPos(Viewer::Window, Viewer::Config::Current->WindowX, Viewer::Config::Current->WindowY);
		}
		else
		{
			glfwSetWindowMonitor(Viewer::Window, nullptr, Viewer::Config::Current->WindowX, Viewer::Config::Current->WindowY, Viewer::Config::Current->WindowW, Viewer::Config::Current->WindowH, mode->refreshRate);
		}
	}

	FullscreenMode = fullscreen;
	return true;
}


void Viewer::ApplyZoomDelta(float zoomDelta)
{
	CurrZoomMode = Config::Settings::ZoomMode::User;
	float zoomOrig = ZoomPercent;
	ZoomPercent += zoomDelta;
	if (((zoomOrig < 100.0f) && (ZoomPercent > 100.0f)) || ((zoomOrig > 100.0f) && (ZoomPercent < 100.0f)))
		ZoomPercent = 100.0f;

	tMath::tiClamp(ZoomPercent, ZoomMin, ZoomMax);

	PanOffsetX += PanDragDownOffsetX; PanDragDownOffsetX = 0;
	PanOffsetY += PanDragDownOffsetY; PanDragDownOffsetY = 0;
	PanOffsetX = int(float(PanOffsetX)*ZoomPercent/zoomOrig);
	PanOffsetY = int(float(PanOffsetY)*ZoomPercent/zoomOrig);
}


void Viewer::ChangeProfile(Config::Profile profile)
{
	if (Config::GetProfile() == profile)
		return;

	Config::SetProfile(profile);

	// This is for the purists. Turns off unnecessary UI elements for the viewer to function only as a simple
	// viewer. Turns off the nav and menu bars, any dialogs (help, about, thumbnails, info, etc), sets the zoom
	// mode to downscale-only, makes the background match the border colour, sets the auto prop editor to false,
	// sets the slideshow/play to looping, and the slideshow duration to 8 seconds.
	// Note that the settings mentioned above are for the _default_ settings of the basic profile. There is nothing
	// stopping the user from customizing the profile however they want. It is essentially a complete set of alternate
	// settings, the only difference is the default values are different than the main profile.
	if (profile == Config::Profile::Basic)
	{
		// These ones are runtime only.
		PropsWindow								= false;
		BindingsWindow							= false;
		ShowCheatSheet							= false;
		ShowAbout								= false;
	}

	AutoPropertyWindow();
}


void Viewer::ZoomFit()
{
	ResetPan();
	CurrZoomMode = Config::Settings::ZoomMode::Fit;
}


void Viewer::ZoomDownscaleOnly()
{
	ResetPan();
	CurrZoomMode = Config::Settings::ZoomMode::DownscaleOnly;
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


void Viewer::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if ((action != GLFW_PRESS) && (action != GLFW_REPEAT))
		return;

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput || ImGui::IsAnyPopupOpen())
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

	// Convert key codes to support non-US keyboards.
	const char* keyName = glfwGetKeyName(key, 0);
	if (keyName && *keyName)
		key = tStd::tChrupr(keyName[0]);

	// Now we need to query the key-binding system to find out what operation is associated
	// with the received key. The current bindings are stored in the current config.
	uint32 viewerModifiers = Bindings::TranslateModifiers(modifiers);
	Bindings::Operation operation = Config::Current->InputBindings.GetOperation(key, viewerModifiers);
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
			OnNextImageFrame();
			break;

		case Bindings::Operation::PrevImageFrame:
			OnPrevImageFrame();
			break;

		case Bindings::Operation::OnePixelRight:
			if (CurrImage) RequestCursorMove = CursorMove_Right;
			break;

		case Bindings::Operation::OnePixelLeft:
			if (CurrImage) RequestCursorMove = CursorMove_Left;
			break;

		case Bindings::Operation::OnePixelDown:
			if (CurrImage) RequestCursorMove = CursorMove_Down;
			break;

		case Bindings::Operation::OnePixelUp:
			if (CurrImage) RequestCursorMove = CursorMove_Up;
			break;

		case Bindings::Operation::ZoomIn:
			ApplyZoomDelta(tMath::tRound(ZoomPercent*0.1f));
			break;

		case Bindings::Operation::ZoomOut:
			ApplyZoomDelta(tMath::tRound(ZoomPercent*(0.909090909f - 1.0f)));
			break;

		case Bindings::Operation::ToggleCheatSheet:
			ShowCheatSheet = !ShowCheatSheet;
			break;

		case Bindings::Operation::RenameFile:
			if (CurrImage) Request_RenameModal = true;
			break;

		case Bindings::Operation::RefreshReloadImage:
			if (CurrImage)
			{
				CurrImage->Unbind();
				CurrImage->Unload(true);
				CurrImage->Load();
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case Bindings::Operation::ToggleFullscreen:
			ChangeScreenMode(!FullscreenMode);
			break;

		case Bindings::Operation::Escape:
			if (FullscreenMode)
				ChangeScreenMode(false);
			else if (Config::GetProfile() == Config::Profile::Basic)
				ChangeProfile(Config::Profile::Main);
			break;

		case Bindings::Operation::EscapeSupportingQuit:
			if (FullscreenMode)
				ChangeScreenMode(false);
			else if (Config::GetProfile() == Config::Profile::Basic)
				ChangeProfile(Config::Profile::Main);
			else
				Viewer::Request_Quit = true;				
			break;

		case Bindings::Operation::OpenFileBrowser:
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

		case Bindings::Operation::Delete:
			if (CurrImage) Request_DeleteFileModal = true;
			break;

		case Bindings::Operation::DeletePermanent:
			if (CurrImage) Request_DeleteFileNoRecycleModal = true;
			break;

		case Bindings::Operation::Quit:
			Viewer::Request_Quit = true;				
			break;

		case Bindings::Operation::FlipVertically:
		case Bindings::Operation::FlipHorizontally:
			if (CurrImage && !CurrImage->IsAltPictureEnabled())
			{
				CurrImage->Unbind();
				CurrImage->Flip(operation == Bindings::Operation::FlipHorizontally);
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case Bindings::Operation::Rotate90Anticlockwise:
		case Bindings::Operation::Rotate90Clockwise:
			if (CurrImage && !CurrImage->IsAltPictureEnabled())
			{
				CurrImage->Unbind();
				CurrImage->Rotate90(operation == Bindings::Operation::Rotate90Anticlockwise);
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case Bindings::Operation::Crop:
			CropMode = !CropMode;
			break;

		case Bindings::Operation::PropertyEditor:
			PropsWindow = !PropsWindow;
			break;

		case Bindings::Operation::AdjustPixelColour:
			Viewer::Config::Current->ShowPixelEditor = !Viewer::Config::Current->ShowPixelEditor;
			break;

		case Bindings::Operation::ResizeImage:
			if (CurrImage) Request_ResizeImageModal = true;
			break;

		case Bindings::Operation::ResizeCanvas:
			if (CurrImage) Request_ResizeCanvasModal = true;
			break;

		case Bindings::Operation::RotateImage:
			if (CurrImage) Request_RotateImageModal = true;
			break;

		case Bindings::Operation::ToggleImageDetails:
			Viewer::Config::Current->ShowImageDetails = !Viewer::Config::Current->ShowImageDetails;
			break;

		case Bindings::Operation::ToggleTile:
			Config::Current->Tile = !Config::Current->Tile;
			if (!Config::Current->Tile)
				ResetPan();
			break;

		case Bindings::Operation::ToggleMenuBar:
			if (!CropMode) Config::Current->ShowMenuBar = !Config::Current->ShowMenuBar;
			break;

		case Bindings::Operation::SaveMultiFrameImage:
			if (Images.GetNumItems() > 1) Request_MultiFrameModal = true;
			break;

		case Bindings::Operation::ToggleNavBar:
			if (!CropMode) Config::Current->ShowNavBar = !Config::Current->ShowNavBar;
			break;

		case Bindings::Operation::ToggleSlideshowCountdown:
			Config::Current->SlideshowProgressArc = !Config::Current->SlideshowProgressArc;
			break;

		case Bindings::Operation::SaveAs:
			if (CurrImage) Request_SaveAsModal = true;
			break;

		case Bindings::Operation::SaveAll:
			if (CurrImage) Request_SaveAllModal = true;
			break;

		case Bindings::Operation::ToggleBasicMode:
			if (!CropMode) ChangeProfile((Config::GetProfile() == Config::Profile::Basic) ? Config::Profile::Main : Config::Profile::Basic);
			break;

		case Bindings::Operation::ToggleDebugLog:
			NavBar.SetShowLog( !NavBar.GetShowLog() );
			if (NavBar.GetShowLog() && !Config::Current->ShowNavBar)
				Config::Current->ShowNavBar = true;
			break;

		case Bindings::Operation::ZoomFit:
			ResetPan();
			CurrZoomMode = Config::Settings::ZoomMode::Fit;
			break;

		case Bindings::Operation::ZoomDownscaleOnly:
			ResetPan();
			CurrZoomMode = Config::Settings::ZoomMode::DownscaleOnly;
			break;

		case Bindings::Operation::ZoomOneToOne:
			ZoomPercent = 100.0f;
			ResetPan();
			CurrZoomMode = Config::Settings::ZoomMode::OneToOne;
			break;

		case Bindings::Operation::ContactSheet:
			if (Images.GetNumItems() > 1) Request_ContactSheetModal = true;
			break;

		case Bindings::Operation::Preferences:
			PrefsWindow = !PrefsWindow;
			break;

		case Bindings::Operation::ContentThumbnailView:
			Viewer::Config::Current->ContentViewShow = !Viewer::Config::Current->ContentViewShow;
			break;

		case Bindings::Operation::ToggleKeyBindings:
			BindingsWindow = !BindingsWindow;
			if (BindingsWindow) BindingsWindowJustOpened = true;
			break;

		case Bindings::Operation::ToggleChannelFilter:
			Config::Current->ShowChannelFilter = !Config::Current->ShowChannelFilter;
			break;

		case Bindings::Operation::ToggleRedChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = true; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = false; }
			else
				DrawChannel_R = !DrawChannel_R;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::ToggleGreenChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = false; DrawChannel_G = true; DrawChannel_B = false; DrawChannel_A = false; }
			else
				DrawChannel_G = !DrawChannel_G;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::ToggleBlueChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = false; DrawChannel_G = false; DrawChannel_B = true; DrawChannel_A = false; }
			else
				DrawChannel_B = !DrawChannel_B;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::ToggleAlphaChannel:
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = false; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = true; }
			else
				DrawChannel_A = !DrawChannel_A;
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::ToggleChannelAsIntensity:
			DrawChannel_AsIntensity = !DrawChannel_AsIntensity;
			if (DrawChannel_AsIntensity)
				{ DrawChannel_R = true; DrawChannel_G = false; DrawChannel_B = false; DrawChannel_A = false; }
			else
				{ DrawChannel_R = true; DrawChannel_G = true; DrawChannel_B = true; DrawChannel_A = true; }
			Config::Current->ShowChannelFilter = true;	
			break;

		case Bindings::Operation::Undo:
			if (CurrImage && CurrImage->IsUndoAvailable()) Undo();
			break;

		case Bindings::Operation::Redo:
			if (CurrImage && CurrImage->IsRedoAvailable()) Redo();
			break;

		#ifdef ENABLE_FILE_DIALOG_SUPPORT
		case Bindings::Operation::OpenFile:
			Request_OpenFileModal = true;
			break;

		case Bindings::Operation::OpenDir:
			Request_OpenDirModal = true;
			break;
		#endif
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
				ReticleX = mouseX;
				ReticleY = mouseY;
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

	CurrZoomMode = Config::Settings::ZoomMode::User;
	float percentChange = (y > 0.0) ? 0.1f : 1.0f-0.909090909f;
	float zoomDelta = ZoomPercent * percentChange * float(y);
	ApplyZoomDelta(zoomDelta);
}


void Viewer::FileDropCallback(GLFWwindow* window, int count, const char** files)
{
	if (count < 1)
		return;

	tString file = tString(files[0]);
	ImageFileParam.Param = file;
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

	if (hash != ImagesHash)
	{
		tPrintf("Hash mismatch. Dir contents changed. Resynching.\n");
		PopulateImages();
		if (ImageFileParam.IsPresent())
			SetCurrentImage(Viewer::ImageFileParam.Get());
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
	tSystem::tFindFilesFast(cacheFiles, cacheDir, "bin");
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
	ReticleImage			.Load(dataDir + "Reticle.png");
	PrevImage				.Load(dataDir + "Prev.png");
	NextImage				.Load(dataDir + "Next.png");
	PrevArrowImage			.Load(dataDir + "PrevArrow.png");
	NextArrowImage			.Load(dataDir + "NextArrow.png");
	FlipHImage				.Load(dataDir + "FlipH.png");
	FlipVImage				.Load(dataDir + "FlipV.png");
	RotateACWImage			.Load(dataDir + "RotACW.png");
	RotateCWImage			.Load(dataDir + "RotCW.png");
	RotateThetaImage		.Load(dataDir + "RotTheta.png");
	FullscreenImage			.Load(dataDir + "Fullscreen.png");
	WindowedImage			.Load(dataDir + "Windowed.png");
	SkipBeginImage			.Load(dataDir + "SkipBegin.png");
	SkipEndImage			.Load(dataDir + "SkipEnd.png");
	RefreshImage			.Load(dataDir + "Refresh.png");
	RecycleImage			.Load(dataDir + "Recycle.png");
	PropEditImage			.Load(dataDir + "PropEdit.png");
	InfoOverlayImage		.Load(dataDir + "InfoOverlay.png");
	HelpImage				.Load(dataDir + "Help.png");
	PrefsImage				.Load(dataDir + "Settings.png");
	TileImage				.Load(dataDir + "Tile.png");
	StopImage				.Load(dataDir + "Stop.png");
	StopRevImage			.Load(dataDir + "Stop.png");
	PlayImage				.Load(dataDir + "Play.png");
	PlayRevImage			.Load(dataDir + "PlayRev.png");
	PlayLoopImage			.Load(dataDir + "PlayLoop.png");
	PlayOnceImage			.Load(dataDir + "PlayOnce.png");
	ChannelFilterImage		.Load(dataDir + "ChannelFilter.png");
	ContentViewImage		.Load(dataDir + "ContentView.png");
	UpFolderImage			.Load(dataDir + "UpFolder.png");
	CropImage				.Load(dataDir + "Crop.png");
	DefaultThumbnailImage	.Load(dataDir + "DefaultThumbnail.png");
}


void Viewer::UnloadAppImages()
{
	ReticleImage			.Unload();
	PrevImage				.Unload();
	NextImage				.Unload();
	PrevArrowImage			.Unload();
	NextArrowImage			.Unload();
	FlipHImage				.Unload();
	FlipVImage				.Unload();
	RotateACWImage			.Unload();
	RotateCWImage			.Unload();
	RotateThetaImage		.Unload();
	FullscreenImage			.Unload();
	WindowedImage			.Unload();
	SkipBeginImage			.Unload();
	SkipEndImage			.Unload();
	RefreshImage			.Unload();
	RecycleImage			.Unload();
	PropEditImage			.Unload();
	PrefsImage				.Unload();
	HelpImage				.Unload();
	InfoOverlayImage		.Unload();
	TileImage				.Unload();
	StopImage				.Unload();
	StopRevImage			.Unload();
	PlayImage				.Unload();
	PlayRevImage			.Unload();
	PlayLoopImage			.Unload();
	PlayOnceImage			.Unload();
	ChannelFilterImage		.Unload();
	ContentViewImage		.Unload();
	UpFolderImage			.Unload();
	CropImage				.Unload();
	DefaultThumbnailImage	.Unload();
}


int main(int argc, char** argv)
{
	tSystem::tSetSupplementaryDebuggerOutput();
	tSystem::tSetStdoutRedirectCallback(Viewer::PrintRedirectCallback);
	tCmdLine::tParse(argc, argv);

	#ifdef PLATFORM_WINDOWS
	if (Viewer::ImageFileParam.IsPresent())
	{
		tString dest(MAX_PATH);
		int numchars = GetLongPathNameA(Viewer::ImageFileParam.Param.ConstText(), dest.Text(), MAX_PATH);
		if (numchars > 0)
			Viewer::ImageFileParam.Param = dest;

		tPrintf("LongPath:%s\n", dest.ConstText());
	}
	#endif

	#ifdef PACKAGE_SNAP
	// SNAP_USER_DATA is common to all revisions and is backed up. Used for viewer user-configuration file.
	// SNAP_USER_COMMON is common to all revisions of a snap and is not backed up. Used for viewer cache.
	tString snapUserData = tSystem::tGetEnvVar("SNAP_USER_DATA") + "/";
	tString snapUserCommon = tSystem::tGetEnvVar("SNAP_USER_COMMON") + "/";
	tString ldLibraryPath = tSystem::tGetEnvVar("LD_LIBRARY_PATH") + "/";
	tPrintf("SNAP_USER_DATA   : %s\n", snapUserData.Chars());
	tPrintf("SNAP_USER_COMMON : %s\n", snapUserCommon.Chars());
	tPrintf("LD_LIBRARY_PATH  : %s\n", ldLibraryPath.Chars());
	#endif

	// Setup window
	glfwSetErrorCallback(Viewer::GlfwErrorCallback);
	if (!glfwInit())
		return 1;

	int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
	tPrintf("Exe %s\n", tSystem::tGetProgramPath().Chars());
	tPrintf("Tacent View V %d.%d.%d\n", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);
	tPrintf("Tacent Library V %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear ImGui V %s\n", IMGUI_VERSION);
	tPrintf("GLFW V %d.%d.%d\n", glfwMajor, glfwMinor, glfwRev);

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
	Viewer::PendingTransparentWorkArea = Viewer::Config::Current->TransparentWorkArea;

	// We start with window invisible. For windows DwmSetWindowAttribute won't redraw properly otherwise.
	// For all plats, we want to position the window before displaying it.
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	bool requestSnapMessageNoTrans = false;
	if (Viewer::Config::Current->TransparentWorkArea)
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
	Viewer::Window = glfwCreateWindow(Viewer::Config::Current->WindowW, Viewer::Config::Current->WindowH, "tacentview", nullptr, nullptr);
	if (!Viewer::Window)
		return 1;

	Viewer::SetWindowIcon(dataDir + "TacentView.ico");
	Viewer::SetWindowTitle();
	glfwSetWindowPos(Viewer::Window, Viewer::Config::Current->WindowX, Viewer::Config::Current->WindowY);

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

	tString fontFile = dataDir + "Roboto-Medium.ttf";
	io.Fonts->AddFontFromFileTTF(fontFile.Chars(), 14.0f);

	Viewer::LoadAppImages(dataDir);
	Viewer::PopulateImages();
	if (Viewer::ImageFileParam.IsPresent())
		Viewer::SetCurrentImage(Viewer::ImageFileParam.Get());
	else
		Viewer::SetCurrentImage();

	if (Viewer::Config::Current->TransparentWorkArea)
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

	// Main loop.
	static double lastUpdateTime = glfwGetTime();
	while (!glfwWindowShouldClose(Viewer::Window) && !Viewer::Request_Quit)
	{
		double currUpdateTime = glfwGetTime();
		Viewer::Update(Viewer::Window, currUpdateTime - lastUpdateTime);

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

	// This is important. We need the destructors to run BEFORE we shutdown GLFW. Deconstructing the images may block for a bit while shutting
	// down worker threads. We could show a 'shutting down' popup here if we wanted -- if Image::ThumbnailNumThreadsRunning is > 0.
	Viewer::Images.Clear();	
	Viewer::UnloadAppImages();

	// Get current window geometry and set in config file if we're not in fullscreen mode and not iconified.
	if (!Viewer::FullscreenMode && !Viewer::WindowIconified)
	{
		glfwGetWindowPos(Viewer::Window, &Viewer::Config::Current->WindowX, &Viewer::Config::Current->WindowY);
		glfwGetWindowSize(Viewer::Window, &Viewer::Config::Current->WindowW, &Viewer::Config::Current->WindowH);
	}

	Viewer::Config::Current->TransparentWorkArea = Viewer::PendingTransparentWorkArea;
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
