// TacitTexView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2018, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <dwmapi.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Foundation/tVersion.h>
#include <System/tCommand.h>
#include <Image/tPicture.h>
#include <Image/tImageHDR.h>
#include <System/tFile.h>
#include <System/tTime.h>
#include <System/tScript.h>
#include <System/tMachine.h>
#include <Math/tHash.h>
#include <Math/tVector2.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "TacitTexView.h"
#include "TacitImage.h"
#include "Dialogs.h"
#include "ContactSheet.h"
#include "ContentView.h"
#include "Crop.h"
#include "SaveDialogs.h"
#include "Settings.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;


namespace TexView
{
	tCommand::tParam ImageFileParam(1, "ImageFile", "File to open.");
	int MajorVersion							= 1;
	int MinorVersion							= 0;
	int Revision								= 7;
	NavLogBar NavBar;
	tString ImagesDir;
	tList<tStringItem> ImagesSubDirs;
	tList<TacitImage> Images;
	tItList<TacitImage> ImagesLoadTimeSorted	(false);
	tuint256 ImagesHash							= 0;
	TacitImage* CurrImage						= nullptr;
	TacitImage ReticleImage;
	TacitImage PrevImage;
	TacitImage NextImage;
	TacitImage PrevArrowImage;
	TacitImage NextArrowImage;
	TacitImage FlipHImage;
	TacitImage FlipVImage;
	TacitImage RotateACWImage;
	TacitImage RotateCWImage;
	TacitImage FullscreenImage;
	TacitImage WindowedImage;
	TacitImage SkipBeginImage;
	TacitImage SkipEndImage;
	TacitImage MipmapsImage;
	TacitImage CubemapImage;
	TacitImage RefreshImage;
	TacitImage RecycleImage;
	TacitImage PropEditImage;
	TacitImage InfoOverlayImage;
	TacitImage TileImage;
	TacitImage StopImage;
	TacitImage StopRevImage;
	TacitImage PlayImage;
	TacitImage PlayRevImage;
	TacitImage PlayLoopImage;
	TacitImage PlayOnceImage;
	TacitImage ContentViewImage;
	TacitImage UpFolderImage;
	TacitImage CropImage;
	TacitImage DefaultThumbnailImage;

	GLFWwindow* Window							= nullptr;
	double DisappearCountdown					= DisappearDuration;
	double SlideshowCountdown					= 0.0;
	bool SlideshowPlaying						= false;
	bool FullscreenMode							= false;
	bool WindowIconified						= false;
	bool ShowCheatSheet							= false;
	bool ShowAbout								= false;
	bool Request_SaveAsModal					= false;
	bool Request_SaveAllModal					= false;
	bool Request_ContactSheetModal				= false;
	bool Request_DeleteFileModal				= false;
	bool Request_DeleteFileNoRecycleModal		= false;
	bool PrefsWindow							= false;
	bool PropEditorWindow						= false;
	bool CropMode								= false;
	bool LMBDown								= false;
	bool RMBDown								= false;
	bool DeleteAllCacheFilesOnExit				= false;

	int DragAnchorX								= 0;
	int DragAnchorY								= 0;

	enum class ZoomMode
	{
		User,
		Fit,
		Downscale,
		OneToOne
	};
	ZoomMode CurrZoomMode						= ZoomMode::Downscale;

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
	const tVector4 ColourDisabledTint			= tVector4(0.36f, 0.36f, 0.48f, 1.00f);
	const tVector4 ColourBG						= tVector4(0.00f, 0.00f, 0.00f, 0.00f);
	const tVector4 ColourPressedBG				= tVector4(0.26f, 0.59f, 0.98f, 1.00f);
	const tVector4 ColourClear					= tVector4(0.10f, 0.10f, 0.12f, 1.00f);

	const float ZoomMin							= 10.0f;
	const float ZoomMax							= 2500.0f;
	uint64 FrameNumber							= 0;

	void DrawBackground(float bgX, float bgY, float bgW, float bgH);
	void DrawNavBar(float x, float y, float w, float h);
	int GetNavBarHeight();
	void PrintRedirectCallback(const char* text, int numChars)															{ NavBar.AddLog("%s", text); }
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }

	// When compare functions are used to sort, they result in ascending order if they return a < b.
	bool Compare_AlphabeticalAscending(const tStringItem& a, const tStringItem& b)										{ return tStricmp(a.Chars(), b.Chars()) < 0; }
	bool Compare_FileCreationTimeAscending(const tStringItem& a, const tStringItem& b)
	{
		tFileInfo ia; tGetFileInfo(ia, a);
		tFileInfo ib; tGetFileInfo(ib, b);
		return ia.CreationTime < ib.CreationTime;
	}
	bool Compare_ImageLoadTimeAscending(const TacitImage& a, const TacitImage& b)										{ return a.GetLoadedTime() < b.GetLoadedTime(); }
	bool Compare_ImageFileNameAscending(const TacitImage& a, const TacitImage& b)										{ return tStricmp(a.Filename.Chars(), b.Filename.Chars()) < 0; }
	bool Compare_ImageFileNameDescending(const TacitImage& a, const TacitImage& b)										{ return tStricmp(a.Filename.Chars(), b.Filename.Chars()) > 0; }
	bool Compare_ImageFileTypeAscending(const TacitImage& a, const TacitImage& b)										{ return int(a.Filetype) < int(b.Filetype); }
	bool Compare_ImageFileTypeDescending(const TacitImage& a, const TacitImage& b)										{ return int(a.Filetype) > int(b.Filetype); }
	bool Compare_ImageModTimeAscending(const TacitImage& a, const TacitImage& b)										{ return a.FileModTime < b.FileModTime; }
	bool Compare_ImageModTimeDescending(const TacitImage& a, const TacitImage& b)										{ return a.FileModTime > b.FileModTime; }
	bool Compare_ImageFileSizeAscending(const TacitImage& a, const TacitImage& b)										{ return a.FileSizeB < b.FileSizeB; }
	bool Compare_ImageFileSizeDescending(const TacitImage& a, const TacitImage& b)										{ return a.FileSizeB > b.FileSizeB; }
	typedef bool ImageCompareFn(const TacitImage&, const TacitImage&);

	void PopulateImagesSubDirs();
	bool OnPrevious(bool circ = false);
	bool OnNext(bool circ = false);
	void OnPreviousPart();
	void OnNextPart();
	bool OnSkipBegin();
	bool OnSkipEnd();
	void ResetPan(bool resetX = true, bool resetY = true);
	void ApplyZoomDelta(float zoomDelta, float roundTo, bool correctPan);
	tString FindImageFilesInCurrentFolder(tList<tStringItem>& foundFiles);					// Returns the image folder.
	tuint256 ComputeImagesHash(const tList<tStringItem>& files);
	int RemoveOldCacheFiles(const tString& cacheDir);						// Returns num removed.

	void Update(GLFWwindow* window, double dt, bool dopoll = true);
	void WindowRefreshFun(GLFWwindow* window)																			{ Update(window, 0.0, false); }
	void KeyCallback(GLFWwindow*, int key, int scancode, int action, int modifiers);
	void MouseButtonCallback(GLFWwindow*, int mouseButton, int x, int y);
	void CursorPosCallback(GLFWwindow*, double x, double y);
	void ScrollWheelCallback(GLFWwindow*, double x, double y);
	void FileDropCallback(GLFWwindow*, int count, const char** paths);
	void FocusCallback(GLFWwindow*, int gotFocus);
	void IconifyCallback(GLFWwindow*, int iconified);
}


tVector2 TexView::GetDialogOrigin(float index)
{
	return tVector2(DialogOrigin + DialogDelta*float(index), DialogOrigin + TopUIHeight + DialogDelta*float(index));
}


int TexView::GetNavBarHeight()
{
	if (FullscreenMode || !Config.ShowNavBar)
		return 0;

	return NavBar.GetShowLog() ? 150 : 24;
}


void TexView::DrawNavBar(float x, float y, float w, float h)
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


tString TexView::FindImageFilesInCurrentFolder(tList<tStringItem>& foundFiles)
{
	tString imagesDir = tSystem::tGetCurrentDir();
	if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
		imagesDir = tSystem::tGetDir(ImageFileParam.Get());

	tPrintf("Looking for image files in %s\n", imagesDir.Chars());
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.jpg");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.gif");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tga");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.png");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tif");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tiff");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.bmp");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.dds");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.hdr");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.rgbe");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.exr");

	return imagesDir;
}


tuint256 TexView::ComputeImagesHash(const tList<tStringItem>& files)
{
	tuint256 hash = 0;
	for (tStringItem* item = files.First(); item; item = item->Next())
		hash = tMath::tHashString256(item->Chars(), hash);

	return hash;
}


void TexView::PopulateImagesSubDirs()
{
	ImagesSubDirs.Clear();

	tList<tStringItem> foundDirs;
	tFindDirs(foundDirs, ImagesDir, false);
	for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
	{
		tString relPath = tGetRelativePath(ImagesDir, *dir);
		relPath = tGetSimplifiedPath(relPath);
		if (relPath[relPath.Length()-1] == '/')
			relPath.ExtractSuffix(1);

		ImagesSubDirs.Append(new tStringItem(relPath));
	}
}


void TexView::PopulateImages()
{
	Images.Clear();
	ImagesLoadTimeSorted.Clear();

	tList<tStringItem> foundFiles;
	ImagesDir = FindImageFilesInCurrentFolder(foundFiles);
	PopulateImagesSubDirs();

	// We sort here so ComputeImagesHash always returns consistent values.
	foundFiles.Sort(Compare_AlphabeticalAscending, tListSortAlgorithm::Merge);
	ImagesHash = ComputeImagesHash(foundFiles);

	for (tStringItem* filename = foundFiles.First(); filename; filename = filename->Next())
	{
		// It is important we don't call Load after newing. We save memory by not having all images loaded.
		TacitImage* newImg = new TacitImage(*filename);
		Images.Append(newImg);
		ImagesLoadTimeSorted.Append(newImg);
	}

	SortImages(Settings::SortKeyEnum(Config.SortKey), Config.SortAscending);
	CurrImage = nullptr;
}


void TexView::SortImages(Settings::SortKeyEnum key, bool ascending)
{
	ImageCompareFn* sortFn;
	switch (key)
	{
		case Settings::SortKeyEnum::Alphabetical:
			sortFn = ascending ? Compare_ImageFileNameAscending : &Compare_ImageFileNameDescending;
			break;

		case Settings::SortKeyEnum::FileModTime:
			sortFn = ascending ? Compare_ImageModTimeAscending : Compare_ImageModTimeDescending;
			break;

		case Settings::SortKeyEnum::FileSize:
			sortFn = ascending ? Compare_ImageFileSizeAscending : Compare_ImageFileSizeDescending;
			break;

		case Settings::SortKeyEnum::FileType:
			sortFn = ascending ? Compare_ImageFileTypeAscending : Compare_ImageFileTypeDescending;
			break;
	}

	Images.Sort(sortFn);
}


TacitImage* TexView::FindImage(const tString& filename)
{
	TacitImage* tacitImage = nullptr;
	for (TacitImage* si = Images.First(); si; si = si->Next())
	{
		if (si->Filename.IsEqualCI(filename))
		{
			tacitImage = si;
			break;
		}
	}

	return tacitImage;
}


void TexView::SetCurrentImage(const tString& currFilename)
{
	for (TacitImage* si = Images.First(); si; si = si->Next())
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
		CurrZoomMode = ZoomMode::Downscale;
		LoadCurrImage();
	}
}


void TexView::LoadCurrImage()
{
	tAssert(CurrImage);
	bool imgJustLoaded = false;
	if (!CurrImage->IsLoaded())
		imgJustLoaded = CurrImage->Load();

	if (Config.AutoPropertyWindow)
		PropEditorWindow = (CurrImage->TypeSupportsProperties() || (CurrImage->GetNumParts() > 1));

	if (Config.AutoPlayAnimatedGIFs && (CurrImage->Filetype == tFileType::GIF) && (CurrImage->GetNumParts() > 1))
	{
		CurrImage->PartPlayLooping = true;
		CurrImage->PartPlayRev = false;
		CurrImage->Play();
	}

	SetWindowTitle();
	ResetPan();

	// We only need to consider unloading an image when a new one is loaded... in this function.
	// We currently do not allow unloading when in slideshow and the frame duration is small.
	bool slideshowSmallDuration = SlideshowPlaying && (Config.SlidehowFrameDuration < 0.5f);
	if (imgJustLoaded && !slideshowSmallDuration)
	{
		ImagesLoadTimeSorted.Sort(Compare_ImageLoadTimeAscending);

		int64 usedMem = 0;
		for (tItList<TacitImage>::Iter iter = ImagesLoadTimeSorted.First(); iter; iter++)
			usedMem += int64((*iter).Info.MemSizeBytes);

		int64 allowedMem = int64(Config.MaxImageMemMB) * 1024 * 1024;
		if (usedMem > allowedMem)
		{
			tPrintf("Used image mem (%|64d) bigger than max (%|64d). Unloading.\n", usedMem, allowedMem);
			for (tItList<TacitImage>::Iter iter = ImagesLoadTimeSorted.First(); iter; iter++)
			{
				TacitImage* i = iter.GetObject();

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


bool TexView::OnPrevious(bool circ)
{
	if (!CurrImage || (!circ && !CurrImage->Prev()))
		return false;

	CurrImage = circ ? Images.PrevCirc(CurrImage) : CurrImage->Prev();
	LoadCurrImage();
	return true;
}


bool TexView::OnNext(bool circ)
{
	if (!CurrImage || (!circ && !CurrImage->Next()))
		return false;

	CurrImage = circ ? Images.NextCirc(CurrImage) : CurrImage->Next();
	LoadCurrImage();
	return true;
}


void TexView::OnPreviousPart()
{
	if (!CurrImage || (CurrImage->GetNumParts() <= 1))
		return;

	CurrImage->PartNum = tClampMin(CurrImage->PartNum-1, 0);
}


void TexView::OnNextPart()
{
	if (!CurrImage || (CurrImage->GetNumParts() <= 1))
		return;

	CurrImage->PartNum = tClampMax(CurrImage->PartNum+1, CurrImage->GetNumParts()-1);
}


bool TexView::OnSkipBegin()
{
	if (!CurrImage || !Images.First())
		return false;

	CurrImage = Images.First();
	LoadCurrImage();
	return true;
}


bool TexView::OnSkipEnd()
{
	if (!CurrImage || !Images.Last())
		return false;

	CurrImage = Images.Last();
	LoadCurrImage();
	return true;
}


void TexView::ShowHelpMark(const char* desc)
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


void TexView::ShowToolTip(const char* desc)
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


void TexView::SetWindowTitle()
{
	if (!Window)
		return;

	tString title = "Tacit Viewer";
	if (CurrImage && !CurrImage->Filename.IsEmpty())
	{
		title = title + " - " + tGetFileName(CurrImage->Filename);
		if (CurrImage->IsDirty())
			title += "*";
	}

	glfwSetWindowTitle(Window, title.Chars());
}


void TexView::ResetPan(bool resetX, bool resetY)
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


void TexView::DrawBackground(float bgX, float bgY, float bgW, float bgH)
{
	switch (Config.BackgroundStyle)
	{
		case Settings::BGStyle::None:
			return;

		case Settings::BGStyle::Checkerboard:
		{
			// Semitransparent checkerboard background.
			int x = 0;
			int y = 0;
			bool lineStartToggle = false;
			float checkSize = 16.0f;
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

					float l = tMath::tRound(bgX+x*checkSize);
					float r = tMath::tRound(bgX+x*checkSize+cw);
					float b = tMath::tRound(bgY+y*checkSize);
					float t = tMath::tRound(bgY+y*checkSize+ch);

					glBegin(GL_QUADS);
					glVertex2f(l, b);
					glVertex2f(l, t);
					glVertex2f(r, t);
					glVertex2f(r, b);
					glEnd();

					x++;
				}
				x = 0;
				y++;
				lineStartToggle = !lineStartToggle;
			}
			break;
		}

		case Settings::BGStyle::Black:
		case Settings::BGStyle::Grey:
		case Settings::BGStyle::White:
		{
			switch (Config.BackgroundStyle)
			{
				case Settings::BGStyle::Black:	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);		break;
				case Settings::BGStyle::Grey:	glColor4f(0.25f, 0.25f, 0.3f, 1.0f);	break;
				case Settings::BGStyle::White:	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);		break;
			}
			float l = tMath::tRound(bgX);
			float r = tMath::tRound(bgX+bgW);
			float b = tMath::tRound(bgY);
			float t = tMath::tRound(bgY+bgH);

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


void TexView::ConvertScreenPosToImagePos
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
	if (Config.Tile)
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
	if (!Config.Tile)
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


void TexView::ConvertImagePosToScreenPos
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


void TexView::Update(GLFWwindow* window, double dt, bool dopoll)
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

	glClearColor(ColourClear.x, ColourClear.y, ColourClear.z, ColourClear.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int bottomUIHeight	= GetNavBarHeight();
	int topUIHeight		= FullscreenMode ? 0 : 26;

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
	float draww = 1.0f;		float drawh = 1.0f;
	float iw = 1.0f;		float ih = 1.0f;
	float hmargin = 0.0f;	float vmargin = 0.0f;
	static int imgx = 0;	static int imgy = 0;

	float uvUOff = 0.0f;
	float uvVOff = 0.0f;
	float l = 0.0f;
	float r = 0.0f;
	float b = 0.0f;
	float t = 0.0f;
	float uvUMarg = 0.0f;
	float uvVMarg = 0.0f;

	if (CurrImage)
	{
		CurrImage->UpdatePlaying(float(dt));

		iw = float(CurrImage->GetWidth());
		ih = float(CurrImage->GetHeight());
		float picAspect = iw/ih;

		float cropExtraMargin = CropMode ? 5.0f : 0.0f;
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

		// w and h are the image width and height. draww and drawh are the drawable area width and height.
		l = tMath::tRound(hmargin);
		r = tMath::tRound(hmargin+draww);
		b = tMath::tRound(vmargin);
		t = tMath::tRound(vmargin+drawh);

		if (CurrZoomMode == ZoomMode::Downscale)
		{
			ZoomPercent = 100.0f;
			if (draww < iw)
				ZoomPercent = 100.0f * draww / iw;
		}
		else if (CurrZoomMode == ZoomMode::Fit)
		{
			ZoomPercent = 100.0f * draww / iw;
		}

		float w = iw * ZoomPercent/100.0f;
		float h = ih * ZoomPercent/100.0f;

		// If the image is smaller than the drawable area we draw a quad of the correct size with full 0..1 range in the uvs.
		if (w < draww)
		{
			float offsetW = tMath::tRound((draww - w) / 2.0f);
			l += offsetW;
			r -= offsetW;
			float offsetH = tMath::tRound((drawh - h) / 2.0f);
			b += offsetH;
			t -= offsetH;
		}
		else
		{
			float propw = draww / w;
			uvUMarg = (1.0f - propw)/2.0f;
			float proph = drawh / h;
			uvVMarg = (1.0f - proph)/2.0f;
		}

		double mouseXd, mouseYd;
		glfwGetCursorPos(window, &mouseXd, &mouseYd);

		// Make origin lower-left.
		float workH = float(Disph - GetNavBarHeight());
		float mouseX = float(mouseXd);
		float mouseY = workH - float(mouseYd);

		int mouseXi = int(mouseX);
		int mouseYi = int(mouseY);

		// Modify the UVs here to magnify.
		if ((draww < w) || Config.Tile)
		{
			if (RMBDown)
				PanDragDownOffsetX = mouseXi - DragAnchorX;

			if (!Config.Tile)
				tMath::tiClamp(PanDragDownOffsetX, int(-(w-draww)/2.0f) - PanOffsetX, int((w-draww)/2.0f) - PanOffsetX);
		}

		if ((drawh < h) || Config.Tile)
		{
			if (RMBDown)
				PanDragDownOffsetY = mouseYi - DragAnchorY;

			if (!Config.Tile)
				tMath::tiClamp(PanDragDownOffsetY, int(-(h-drawh)/2.0f) - PanOffsetY, int((h-drawh)/2.0f) - PanOffsetY);
		}

		if ((draww > w) && !Config.Tile)
			ResetPan(true, false);

		if ((drawh > h) && !Config.Tile)
			ResetPan(false, true);

		uvUOff = -float(PanOffsetX+PanDragDownOffsetX)/w;
		uvVOff = -float(PanOffsetY+PanDragDownOffsetY)/h;

		// Draw background.
		glDisable(GL_TEXTURE_2D);
		if ((Config.BackgroundExtend || Config.Tile) && !CropMode)
			DrawBackground(hmargin, vmargin, draww, drawh);
		else
			DrawBackground(l, b, r-l, t-b);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		CurrImage->Bind();
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		if (!Config.Tile)
		{
			glTexCoord2f(0.0f + uvUMarg + uvUOff, 0.0f + uvVMarg + uvVOff); glVertex2f(l, b);
			glTexCoord2f(0.0f + uvUMarg + uvUOff, 1.0f - uvVMarg + uvVOff); glVertex2f(l, t);
			glTexCoord2f(1.0f - uvUMarg + uvUOff, 1.0f - uvVMarg + uvVOff); glVertex2f(r, t);
			glTexCoord2f(1.0f - uvUMarg + uvUOff, 0.0f + uvVMarg + uvVOff); glVertex2f(r, b);
		}
		else
		{
			float repU = draww/(r-l);	float offU = (1.0f-repU)/2.0f;
			float repV = drawh/(t-b);	float offV = (1.0f-repV)/2.0f;
			glTexCoord2f(offU + 0.0f + uvUMarg + uvUOff,	offV + 0.0f + uvVMarg + uvVOff);	glVertex2f(hmargin,			vmargin);
			glTexCoord2f(offU + 0.0f + uvUMarg + uvUOff,	offV + repV - uvVMarg + uvVOff);	glVertex2f(hmargin,			vmargin+drawh);
			glTexCoord2f(offU + repU - uvUMarg + uvUOff,	offV + repV - uvVMarg + uvVOff);	glVertex2f(hmargin+draww,	vmargin+drawh);
			glTexCoord2f(offU + repU - uvUMarg + uvUOff,	offV + 0.0f + uvVMarg + uvVOff);	glVertex2f(hmargin+draww,	vmargin);
		}
		glEnd();

		// Get the colour under the reticle.
		tVector2 scrCursorPos(ReticleX, ReticleY);
		ConvertScreenPosToImagePos
		(
			imgx, imgy, scrCursorPos, tVector4(l, r, t, b),
			tVector2(uvUMarg, uvVMarg), tVector2(uvUOff, uvVOff)
		);

		PixelColour = CurrImage->GetPixel(imgx, imgy);

		// Show the reticle.
		glDisable(GL_TEXTURE_2D);
		glColor4fv(tColour::white.E);
		if (!CropMode && (Config.ShowImageDetails || (DisappearCountdown > 0.0)))
		{
			tVector2 scrPosBL;
			ConvertImagePosToScreenPos
			(
				scrPosBL, imgx, imgy, tVector4(l, r, t, b),
				tVector2(uvUMarg, uvVMarg), tVector2(uvUOff, uvVOff)
			);
			tVector2 scrPosTR;
			ConvertImagePosToScreenPos
			(
				scrPosTR, imgx+1, imgy+1, tVector4(l, r, t, b),
				tVector2(uvUMarg, uvVMarg), tVector2(uvUOff, uvVOff)
			);
			tColouri hsv = PixelColour;
			hsv.RGBToHSV();
			if (hsv.V > 150)
				glColor4ubv(tColouri::black.E);
			else
				glColor4ubv(tColouri::white.E);

			if (ZoomPercent >= 500.0f)
			{
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
				CropGizmo.SetLines(tVector4(l,r,t,b));

			CropGizmo.UpdateDraw
			(
				tVector4(l,r,t,b), tVector2(mouseX, mouseY),
				tVector2(uvUMarg, uvVMarg), tVector2(uvUOff, uvVOff)
			);
		}
		lastCropMode = CropMode;
	}

	ImGui::NewFrame();
	
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

	if ((DisappearCountdown > 0.0) && !CropMode)
	{
		// Previous arrow.
		if ((CurrImage != Images.First()) || SlideshowPlaying)
		{
			ImGui::SetNextWindowPos(tVector2(0.0f, float(topUIHeight) + float(workAreaH)*0.5f - 33.0f));
			ImGui::SetNextWindowSize(tVector2(16, 70), ImGuiCond_Always);
			ImGui::Begin("PrevArrow", nullptr, flagsImgButton);
			ImGui::SetCursorPos(tVector2(6, 2));
			if (ImGui::ImageButton(ImTextureID(PrevArrowImage.Bind()), tVector2(15,56), tVector2(0,0), tVector2(1,1), 3, tVector4(0,0,0,0), tVector4(1,1,1,1)))
				OnPrevious();
			ImGui::End();
		}

		// Next arrow.
		if ((CurrImage != Images.Last()) || SlideshowPlaying)
		{
			ImGui::SetNextWindowPos(tVector2(workAreaW - 33.0f, float(topUIHeight) + float(workAreaH) * 0.5f - 33.0f));
			ImGui::SetNextWindowSize(tVector2(16, 70), ImGuiCond_Always);
			ImGui::Begin("NextArrow", nullptr, flagsImgButton);
			ImGui::SetCursorPos(tVector2(6, 2));
			if (ImGui::ImageButton(ImTextureID(NextArrowImage.Bind()), tVector2(15,56), tVector2(0,0), tVector2(1,1), 3, tVector4(0,0,0,0), tVector4(1,1,1,1)))
				OnNext();
			ImGui::End();
		}

		// Looping button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-120.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Repeat", nullptr, flagsImgButton);
		uint64 playModeImageID = Config.SlideshowLooping ? PlayOnceImage.Bind() : PlayLoopImage.Bind();
		if (ImGui::ImageButton(ImTextureID(playModeImageID), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			Config.SlideshowLooping = !Config.SlideshowLooping;
		ImGui::End();

		// Skip to beginning button.
		bool prevAvail = (CurrImage != Images.First()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-80.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("SkipBegin", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(SkipBeginImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnSkipBegin();
		ImGui::End();

		// Prev button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f-40.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Prev", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(PrevImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, prevAvail ? ColourEnabledTint : ColourDisabledTint) && prevAvail
		)	OnPrevious();
		ImGui::End();

		// Slideshow/Stop button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+0.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Slideshow", nullptr, flagsImgButton);
		uint64 psImageID = SlideshowPlaying ? StopImage.Bind() : PlayImage.Bind();
		if (ImGui::ImageButton(ImTextureID(psImageID), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			SlideshowPlaying = !SlideshowPlaying;
		ImGui::End();

		// Next button.
		bool nextAvail = (CurrImage != Images.Last()) || SlideshowPlaying;
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+40.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Next", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(NextImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnNext();
		ImGui::End();

		// Skip to end button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+80.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("SkipEnd", nullptr, flagsImgButton);
		if (ImGui::ImageButton
		(
			ImTextureID(SkipEndImage.Bind()), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2,
			ColourBG, nextAvail ? ColourEnabledTint : ColourDisabledTint) && nextAvail
		)	OnSkipEnd();
		ImGui::End();

		// Fullscreen / Windowed button.
		ImGui::SetNextWindowPos(tVector2((workAreaW>>1)-22.0f+120.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(tVector2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Fullscreen", nullptr, flagsImgButton);
		uint64 fsImageID = FullscreenMode ? WindowedImage.Bind() : FullscreenImage.Bind();
		if (ImGui::ImageButton(ImTextureID(fsImageID), tVector2(24,24), tVector2(0,0), tVector2(1,1), 2, tVector4(0,0,0,0), tVector4(1,1,1,1)))
			ChangeScreenMode(!FullscreenMode);
		ImGui::End();
	}

	ImGui::SetNextWindowPos(tVector2(0, 0));

	if (!FullscreenMode)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,6));
		ImGui::BeginMainMenuBar();

		//
		// File Menu.
		//
		bool saveAsPressed = Request_SaveAsModal;
		bool saveAllPressed = Request_SaveAllModal;
		bool saveContactSheetPressed = Request_ContactSheetModal;
		Request_SaveAsModal = false;
		Request_SaveAllModal = false;
		Request_ContactSheetModal = false;
		if (ImGui::BeginMenu("File"))
		{
			// Show file menu items...
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

			if (ImGui::MenuItem("Save As...", "Ctrl-S") && CurrImage)
				saveAsPressed = true;

			if (ImGui::MenuItem("Save All...", "Alt-S") && CurrImage)
				saveAllPressed = true;

			if (ImGui::MenuItem("Save Contact Sheet...", "C") && (Images.GetNumItems() > 1))
				saveContactSheetPressed = true;

			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Alt-F4"))
				glfwSetWindowShouldClose(Window, 1);

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

		if (saveAsPressed)
			ImGui::OpenPopup("Save As");
		// The unused isOpenSaveAs bool is just so we get a close button in ImGui. 
		bool isOpenSaveAs = true;
		if (ImGui::BeginPopupModal("Save As", &isOpenSaveAs, ImGuiWindowFlags_AlwaysAutoResize))
			DoSaveAsModalDialog(saveAsPressed);

		if (saveAllPressed)
			ImGui::OpenPopup("Save All");
		// The unused isOpenSaveAll bool is just so we get a close button in ImGui. 
		bool isOpenSaveAll = true;
		if (ImGui::BeginPopupModal("Save All", &isOpenSaveAll, ImGuiWindowFlags_AlwaysAutoResize))
			DoSaveAllModalDialog(saveAllPressed);

		if (saveContactSheetPressed)
			ImGui::OpenPopup("Contact Sheet");
		// The unused isOpenContactSheet bool is just so we get a close button in ImGui. 
		bool isOpenContactSheet = true;
		if (ImGui::BeginPopupModal("Contact Sheet", &isOpenContactSheet, ImGuiWindowFlags_AlwaysAutoResize))
			DoContactSheetModalDialog(saveContactSheetPressed);

		ImGui::PopStyleVar();

		//
		// Edit Menu.
		//
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

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

			ImGui::Separator();

			ImGui::MenuItem("Property Editor...", "E", &PropEditorWindow);
			ImGui::MenuItem("Preferences...", "P", &PrefsWindow);

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));

		if (PrefsWindow)
			ShowPreferencesWindow(&PrefsWindow);

		if (PropEditorWindow)
			ShowPropertyEditorWindow(&PropEditorWindow);

		ImGui::PopStyleVar();

		//
		// View Menu.
		//
		if (ImGui::BeginMenu("View"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(4,3));
			ImGui::MenuItem("Nav Bar", "N", &Config.ShowNavBar);
			ImGui::MenuItem("Image Details", "I", &Config.ShowImageDetails);
			ImGui::MenuItem("Content View", "V", &Config.ContentViewShow);

			ImGui::Separator();

			bool userMode = CurrZoomMode == ZoomMode::User;
			if (ImGui::MenuItem("Zoom User", "", &userMode))
			{
				ResetPan();
				CurrZoomMode = ZoomMode::User;
			}

			bool fitMode = CurrZoomMode == ZoomMode::Fit;
			if (ImGui::MenuItem("Zoom Fit", "F", &fitMode))
			{
				ResetPan();
				CurrZoomMode = ZoomMode::Fit;
			}

			bool downscale = CurrZoomMode == ZoomMode::Downscale;
			if (ImGui::MenuItem("Zoom Downscale", "D", &downscale))
			{
				ResetPan();
				CurrZoomMode = ZoomMode::Downscale;
			}

			bool oneToOne = CurrZoomMode == ZoomMode::OneToOne;
			if (ImGui::MenuItem("Zoom 1:1", "Z", &oneToOne))
			{
				ZoomPercent = 100.0f;
				ResetPan();
				CurrZoomMode = ZoomMode::OneToOne;
			}

			ImGui::PushItemWidth(60);
			const char* zoomItems[] = { "Zoom", "20%", "50%", "100%", "150%", "200%", "400%", "800%", "1200%", "1800%", "2500%"};
			float zoomVals[] = { -1.0f, 20.0f, 50.0f, 100.0f, 150.0f, 200.0f, 400.0f, 800.0f, 1200.0f, 1800.0f, 2500.0f };
			tString currZoomStr;
			tsPrintf(currZoomStr, "%0.0f%%", ZoomPercent);
			int zoomIdx = 0;
			if (ImGui::Combo(currZoomStr.Chars(), &zoomIdx, zoomItems, tNumElements(zoomItems)) && (zoomIdx > 0))
				ApplyZoomDelta( zoomVals[zoomIdx]-ZoomPercent, 1.0f, true);
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
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, tVector2(20,20)))
			ImGui::OpenPopup("CopyColourAs");

		if (ImGui::BeginPopup("CopyColourAs"))
			ColourCopyAs();

		bool transAvail = CurrImage ? !CurrImage->IsAltPictureEnabled() : false;
		if (ImGui::ImageButton
		(
			ImTextureID(FlipVImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2, ColourBG,
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
			ImTextureID(FlipHImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2, ColourBG,
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
			ImTextureID(RotateACWImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2, ColourBG,
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
			ImTextureID(RotateCWImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2, ColourBG,
			transAvail ? ColourEnabledTint : ColourDisabledTint) && transAvail
		)
		{
			CurrImage->Unbind();
			CurrImage->Rotate90(false);
			CurrImage->Bind();
			SetWindowTitle();
		}
		ShowToolTip("Rotate 90 Clockwise");

		bool cropAvail = CurrImage && transAvail && !Config.Tile;
		if (ImGui::ImageButton
		(
			ImTextureID(CropImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			CropMode ? ColourPressedBG : ColourBG, cropAvail ? ColourEnabledTint : ColourDisabledTint) && cropAvail
		)	CropMode = !CropMode;
		ShowToolTip("Crop");

		bool altMipmapsPicAvail = CurrImage ? CurrImage->IsAltMipmapsPictureAvail() && !CropMode : false;
		bool altMipmapsPicEnabl = altMipmapsPicAvail && CurrImage->IsAltPictureEnabled();
		if (ImGui::ImageButton
		(
			ImTextureID(MipmapsImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			altMipmapsPicEnabl ? ColourPressedBG : ColourBG, altMipmapsPicAvail ? ColourEnabledTint : ColourDisabledTint) && altMipmapsPicAvail
		)
		{
			CurrImage->EnableAltPicture(!altMipmapsPicEnabl);
			CurrImage->Bind();
		}
		ShowToolTip("Display Mipmaps\nDDS files may include mipmaps.");

		bool altCubemapPicAvail = CurrImage ? CurrImage->IsAltCubemapPictureAvail() && !CropMode : false;
		bool altCubemapPicEnabl = altCubemapPicAvail && CurrImage->IsAltPictureEnabled();
		if (ImGui::ImageButton
		(
			ImTextureID(CubemapImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			altCubemapPicEnabl ? ColourPressedBG : ColourBG, altCubemapPicAvail ? ColourEnabledTint : ColourDisabledTint) && altCubemapPicAvail
		)
		{
			CurrImage->EnableAltPicture(!altCubemapPicEnabl);
			CurrImage->Bind();
		}
		ShowToolTip("Display Cubemap\nDDS files may be cubemaps.");

		bool tileAvail = CurrImage ? !CropMode : false;
		if (ImGui::ImageButton
		(
			ImTextureID(TileImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			Config.Tile ? ColourPressedBG : ColourBG, tileAvail ? ColourEnabledTint : ColourDisabledTint) && tileAvail
		)
		{
			Config.Tile = !Config.Tile;
			if (!Config.Tile)
				ResetPan();
		}
		ShowToolTip("Show Images Tiled");

		bool refreshAvail = CurrImage ? true : false;
		if (ImGui::ImageButton
		(
			ImTextureID(RefreshImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
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
			ImTextureID(RecycleImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, recycleAvail ? ColourEnabledTint : ColourDisabledTint) && recycleAvail
		)	Request_DeleteFileModal = true;
		ShowToolTip("Delete Current File");

		if (ImGui::ImageButton
		(
			ImTextureID(ContentViewImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			Config.ContentViewShow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config.ContentViewShow = !Config.ContentViewShow;
		ShowToolTip("Content Thumbnail View");

		if (ImGui::ImageButton
		(
			ImTextureID(PropEditImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			PropEditorWindow ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	PropEditorWindow = !PropEditorWindow;
		ShowToolTip("Image Property Editor");

		if (ImGui::ImageButton
		(
			ImTextureID(InfoOverlayImage.Bind()), tVector2(17, 17), tVector2(0, 1), tVector2(1, 0), 2,
			Config.ShowImageDetails ? ColourPressedBG : ColourBG, ColourEnabledTint)
		)	Config.ShowImageDetails = !Config.ShowImageDetails;
		ShowToolTip("Information Overlay");

		ImGui::EndMainMenuBar();
		ImGui::PopStyleVar();
	}

	if (!FullscreenMode && Config.ShowNavBar)
		DrawNavBar(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

	// We allow the overlay and cheatsheet in fullscreen.
	if (Config.ShowImageDetails)
		ShowImageDetailsOverlay(&Config.ShowImageDetails, 0.0f, float(topUIHeight), float(dispw), float(disph - bottomUIHeight - topUIHeight), imgx, imgy, ZoomPercent);

	if (Config.ContentViewShow)
		ShowContentViewDialog(&Config.ContentViewShow);

	if (ShowCheatSheet)
		ShowCheatSheetPopup(&ShowCheatSheet);

	if (ShowAbout)
		ShowAboutPopup(&ShowAbout);

	ShowCropPopup(tVector4(l, r, t, b), tVector2(uvUMarg, uvVMarg), tVector2(uvUOff, uvVOff));

	if (Request_DeleteFileModal)
	{
		Request_DeleteFileModal = false;
		if (!Config.ConfirmDeletes)
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
			bool ok = OnNext(Config.SlideshowLooping);
			if (!ok)
				SlideshowPlaying = false;
			else
				SlideshowCountdown = Config.SlidehowFrameDuration;
		}
	}
}


bool TexView::DeleteImageFile(const tString& imgFile, bool tryUseRecycleBin)
{
	tString nextImgFile = CurrImage->Next() ? CurrImage->Next()->Filename : tString();

	bool deleted = tSystem::tDeleteFile(imgFile, true, tryUseRecycleBin);
	if (deleted)
	{
		ImageFileParam.Param = nextImgFile;		// We set this so if we lose and gain focus, we go back to the current image.
		PopulateImages();
		SetCurrentImage(nextImgFile);
	}

	return deleted;
}


bool TexView::ChangeScreenMode(bool fullscreen, bool force)
{
	if (!force && (FullscreenMode == fullscreen))
		return false;

	// If currently in windowed mode, remember our window geometry.
	if (!force && !FullscreenMode)
	{
		glfwGetWindowPos(TexView::Window, &TexView::Config.WindowX, &TexView::Config.WindowY);
		glfwGetWindowSize(TexView::Window, &TexView::Config.WindowW, &TexView::Config.WindowH);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	if (fullscreen)
		glfwSetWindowMonitor(TexView::Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	else
		glfwSetWindowMonitor(TexView::Window, nullptr, TexView::Config.WindowX, TexView::Config.WindowY, TexView::Config.WindowW, TexView::Config.WindowH, mode->refreshRate);

	FullscreenMode = fullscreen;
	return true;
}


void TexView::ApplyZoomDelta(float zoomDelta, float roundTo, bool correctPan)
{
	CurrZoomMode = ZoomMode::User;
	float zoomOrig = ZoomPercent;
	ZoomPercent += zoomDelta;
	if (((zoomOrig < 100.0f) && (ZoomPercent > 100.0f)) || ((zoomOrig > 100.0f) && (ZoomPercent < 100.0f)))
		ZoomPercent = 100.0f;

	tMath::tiClamp(ZoomPercent, ZoomMin, ZoomMax);

	if (correctPan)
	{
		PanOffsetX += PanDragDownOffsetX; PanDragDownOffsetX = 0;
		PanOffsetY += PanDragDownOffsetY; PanDragDownOffsetY = 0;
		PanOffsetX = int(float(PanOffsetX)*ZoomPercent/zoomOrig);
		PanOffsetY = int(float(PanOffsetY)*ZoomPercent/zoomOrig);
	}
}


void TexView::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
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

	switch (key)
	{
		case GLFW_KEY_LEFT:
			if (modifiers == GLFW_MOD_CONTROL)
				OnSkipBegin();
			else if (modifiers == GLFW_MOD_ALT)
				OnPreviousPart();
			else
				OnPrevious();
			break;

		case GLFW_KEY_RIGHT:
			if (modifiers == GLFW_MOD_CONTROL)
				OnSkipEnd();
			else if (modifiers == GLFW_MOD_ALT)
				OnNextPart();
			else
				OnNext();
			break;

		case GLFW_KEY_SPACE:
			OnNext();
			break;

		case GLFW_KEY_EQUAL:
			// Ctrl +
			if (modifiers == GLFW_MOD_CONTROL)
				ApplyZoomDelta(tMath::tRound(ZoomPercent*0.1f), 1.0f, true);
			break;

		case GLFW_KEY_MINUS:
			// Ctrl -
			if (modifiers == GLFW_MOD_CONTROL)
				ApplyZoomDelta(tMath::tRound(ZoomPercent*(0.909090909f - 1.0f)), 1.0f, true);
			break;

		case GLFW_KEY_ENTER:
			if (modifiers == GLFW_MOD_ALT)
				ChangeScreenMode(!FullscreenMode);
			break;

		case GLFW_KEY_ESCAPE:
			ChangeScreenMode(false);
			break;

		case GLFW_KEY_DELETE:
			if (CurrImage)
			{
				if (modifiers == GLFW_MOD_SHIFT)
					Request_DeleteFileNoRecycleModal = true;
				else
					Request_DeleteFileModal = true;
			}
			break;

		case GLFW_KEY_TAB:
			if (CurrImage)
				tSystem::tOpenSystemFileExplorer(CurrImage->Filename);
			break;

		case GLFW_KEY_COMMA:
			if (CurrImage && !CurrImage->IsAltPictureEnabled())
			{
				CurrImage->Unbind();
				if (modifiers == GLFW_MOD_CONTROL)
					CurrImage->Flip(false);
				else
					CurrImage->Rotate90(true);
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case GLFW_KEY_PERIOD:
			if (CurrImage && !CurrImage->IsAltPictureEnabled())
			{
				CurrImage->Unbind();
				if (modifiers == GLFW_MOD_CONTROL)
					CurrImage->Flip(true);
				else
					CurrImage->Rotate90(false);
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case GLFW_KEY_SLASH:
			CropMode = !CropMode;
			break;

		case GLFW_KEY_F1:
			ShowCheatSheet = !ShowCheatSheet;
			break;

		case GLFW_KEY_F11:
			ChangeScreenMode(!FullscreenMode);
			break;

		case GLFW_KEY_F5:
		case GLFW_KEY_R:
			if (CurrImage)
			{
				CurrImage->Unbind();
				CurrImage->Unload(true);
				CurrImage->Load();
				CurrImage->Bind();
				SetWindowTitle();
			}
			break;

		case GLFW_KEY_T:
			Config.Tile = !Config.Tile;
			if (!Config.Tile)
				ResetPan();
			break;

		case GLFW_KEY_N:
			Config.ShowNavBar = !Config.ShowNavBar;
			break;

		case GLFW_KEY_I:
			TexView::Config.ShowImageDetails = !TexView::Config.ShowImageDetails;
			break;

		case GLFW_KEY_V:
			TexView::Config.ContentViewShow = !TexView::Config.ContentViewShow;
			break;

		case GLFW_KEY_L:
			NavBar.SetShowLog( !NavBar.GetShowLog() );
			if (NavBar.GetShowLog() && !Config.ShowNavBar)
				Config.ShowNavBar = true;
			break;

		case GLFW_KEY_F:
			ResetPan();
			CurrZoomMode = ZoomMode::Fit;
			break;

		case GLFW_KEY_D:
			ResetPan();
			CurrZoomMode = ZoomMode::Downscale;
			break;

		case GLFW_KEY_Z:
			ZoomPercent = 100.0f;
			ResetPan();
			CurrZoomMode = ZoomMode::OneToOne;
			break;

		case GLFW_KEY_S:
			if (CurrImage)
			{
				if (modifiers == GLFW_MOD_CONTROL)
					Request_SaveAsModal = true;
				else if (modifiers == GLFW_MOD_ALT)
					Request_SaveAllModal = true;
			}
			break;

		case GLFW_KEY_C:
			if (Images.GetNumItems() > 1)
				Request_ContactSheetModal = true;
			break;

		case GLFW_KEY_P:
			PrefsWindow = !PrefsWindow;
			break;

		case GLFW_KEY_E:
			PropEditorWindow = !PropEditorWindow;
			break;
	}
}


void TexView::MouseButtonCallback(GLFWwindow* window, int mouseButton, int press, int mods)
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


void TexView::CursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;
}


void TexView::ScrollWheelCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DisappearCountdown = DisappearDuration;

	CurrZoomMode = ZoomMode::User;
	float percentChange = (y > 0.0) ? 0.1f : 1.0f-0.909090909f;
	float zoomDelta = ZoomPercent * percentChange * float(y);
	ApplyZoomDelta(zoomDelta, 10.0f, true);
}


void TexView::FileDropCallback(GLFWwindow* window, int count, const char** files)
{
	if (count < 1)
		return;

	tString file = tString(files[0]);
	ImageFileParam.Param = file;
	PopulateImages();
	SetCurrentImage(file);
}


void TexView::FocusCallback(GLFWwindow* window, int gotFocus)
{
	if (!gotFocus)
		return;

	// If we got focus, rescan the current folder to see if the hash is different.
	tList<tStringItem> files;
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
			SetCurrentImage(TexView::ImageFileParam.Get());
		else
			SetCurrentImage();
	}
	else
	{
		tPrintf("Hash match. Dir contents same. Doing nothing.\n");
	}
}


void TexView::IconifyCallback(GLFWwindow* window, int iconified)
{
	WindowIconified = iconified;
}


int TexView::RemoveOldCacheFiles(const tString& cacheDir)
{
	tList<tStringItem> cacheFiles;
	tSystem::tFindFilesInDir(cacheFiles, cacheDir, "*.bin");
	int numFiles = cacheFiles.NumItems();
	if (numFiles <= Config.MaxCacheFiles)
		return 0;

	cacheFiles.Sort(Compare_FileCreationTimeAscending);
	int targetCount = tClampMin(Config.MaxCacheFiles - 100, 0);

	int numToRemove = numFiles - targetCount;
	tAssert(numToRemove >= 0);
	int deletedCount = 0;
	while (numToRemove)
	{
		tStringItem* head = cacheFiles.Remove();
		if (tDeleteFile(*head))
			deletedCount++;
		delete head;
		numToRemove--;
	}

	return deletedCount;
}


int main(int argc, char** argv)
{
	tSystem::tSetStdoutRedirectCallback(TexView::PrintRedirectCallback);
	tCommand::tParse(argc, argv);

	// Setup window
	glfwSetErrorCallback(TexView::GlfwErrorCallback);
	if (!glfwInit())
		return 1;

	int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);

	tPrintf("Tacit Viewer V %d.%d.%d\n", TexView::MajorVersion, TexView::MinorVersion, TexView::Revision);
	tPrintf("Tacent Library V %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear ImGui V %s\n", IMGUI_VERSION);
	tPrintf("GLEW V %s\n", glewGetString(GLEW_VERSION));
	tPrintf("GLFW V %d.%d.%d\n", glfwMajor, glfwMinor, glfwRev);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	tString dataDir = tSystem::tGetProgramDir() + "Data/";
	TacitImage::ThumbCacheDir = dataDir + "Cache/";
	if (!tSystem::tDirExists(TacitImage::ThumbCacheDir))
		tSystem::tCreateDir(TacitImage::ThumbCacheDir);

	tString cfgFile = dataDir + "Settings.cfg";
	TexView::Config.Load(cfgFile, mode->width, mode->height);

	// We start with window invisible as DwmSetWindowAttribute won't redraw properly otherwise.
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	TexView::Window = glfwCreateWindow(TexView::Config.WindowW, TexView::Config.WindowH, "Tacit Viewer", nullptr, nullptr);
	if (!TexView::Window)
		return 1;

	glfwSetWindowPos(TexView::Window, TexView::Config.WindowX, TexView::Config.WindowY);

	// Make the window title bar show up in black.
	HWND hwnd = glfwGetWin32Window(TexView::Window);
	const int DWMWA_USE_IMMERSIVE_DARK_MODE = 19;
	BOOL isDarkMode = 1;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &isDarkMode, sizeof(isDarkMode));
	if (!tSystem::tDirExists(dataDir))
	{
		::MessageBoxA
		(
			hwnd,
			"Tacit Texture Viewer failed to launch because it was run from a location "
			"that did not have the Data directory in it. The executable should be in the "
			"same place as the Data directory.",
			"Viewer Message",
			MB_OK
		);

		glfwDestroyWindow(TexView::Window);
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(TexView::Window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetWindowRefreshCallback(TexView::Window, TexView::WindowRefreshFun);
	glfwSetKeyCallback(TexView::Window, TexView::KeyCallback);
	glfwSetMouseButtonCallback(TexView::Window, TexView::MouseButtonCallback);
	glfwSetCursorPosCallback(TexView::Window, TexView::CursorPosCallback);
	glfwSetScrollCallback(TexView::Window, TexView::ScrollWheelCallback);
	glfwSetDropCallback(TexView::Window, TexView::FileDropCallback);
	glfwSetWindowFocusCallback(TexView::Window, TexView::FocusCallback);
	glfwSetWindowIconifyCallback(TexView::Window, TexView::IconifyCallback);

	GLenum err = glewInit();
	if (err != GLEW_OK)
		return err;

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
	ImGui_ImplGlfw_InitForOpenGL(TexView::Window, true);
	ImGui_ImplOpenGL2_Init();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tString fontFile = dataDir + "Roboto-Medium.ttf";
	io.Fonts->AddFontFromFileTTF(fontFile.Chars(), 14.0f);

	TexView::ReticleImage			.Load(dataDir + "Reticle.png");
	TexView::PrevImage				.Load(dataDir + "Prev.png");
	TexView::NextImage				.Load(dataDir + "Next.png");
	TexView::PrevArrowImage			.Load(dataDir + "PrevArrow.png");
	TexView::NextArrowImage			.Load(dataDir + "NextArrow.png");
	TexView::FlipHImage				.Load(dataDir + "FlipH.png");
	TexView::FlipVImage				.Load(dataDir + "FlipV.png");
	TexView::RotateACWImage			.Load(dataDir + "RotACW.png");
	TexView::RotateCWImage			.Load(dataDir + "RotCW.png");
	TexView::FullscreenImage		.Load(dataDir + "Fullscreen.png");
	TexView::WindowedImage			.Load(dataDir + "Windowed.png");
	TexView::SkipBeginImage			.Load(dataDir + "SkipBegin.png");
	TexView::SkipEndImage			.Load(dataDir + "SkipEnd.png");
	TexView::MipmapsImage			.Load(dataDir + "Mipmaps.png");
	TexView::CubemapImage			.Load(dataDir + "Cubemap.png");
	TexView::RefreshImage			.Load(dataDir + "Refresh.png");
	TexView::RecycleImage			.Load(dataDir + "Recycle.png");
	TexView::PropEditImage			.Load(dataDir + "PropEdit.png");
	TexView::InfoOverlayImage		.Load(dataDir + "InfoOverlay.png");
	TexView::TileImage				.Load(dataDir + "Tile.png");
	TexView::StopImage				.Load(dataDir + "Stop.png");
	TexView::StopRevImage			.Load(dataDir + "Stop.png");
	TexView::PlayImage				.Load(dataDir + "Play.png");
	TexView::PlayRevImage			.Load(dataDir + "PlayRev.png");
	TexView::PlayLoopImage			.Load(dataDir + "PlayLoop.png");
	TexView::PlayOnceImage			.Load(dataDir + "PlayOnce.png");
	TexView::ContentViewImage		.Load(dataDir + "ContentView.png");
	TexView::UpFolderImage			.Load(dataDir + "UpFolder.png");
	TexView::CropImage				.Load(dataDir + "Crop.png");
	TexView::DefaultThumbnailImage	.Load(dataDir + "DefaultThumbnail.png");

	TexView::PopulateImages();
	if (TexView::ImageFileParam.IsPresent())
		TexView::SetCurrentImage(TexView::ImageFileParam.Get());
	else
		TexView::SetCurrentImage();

	glClearColor(TexView::ColourClear.x, TexView::ColourClear.y, TexView::ColourClear.z, TexView::ColourClear.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int dispw, disph;
	glfwGetFramebufferSize(TexView::Window, &dispw, &disph);
	glViewport(0, 0, dispw, disph);

	ShowWindow(hwnd, SW_SHOW);
	glfwMakeContextCurrent(TexView::Window);
	glfwSwapBuffers(TexView::Window);

	// Main loop.
	static double lastUpdateTime = glfwGetTime();
	while (!glfwWindowShouldClose(TexView::Window))
	{
		double currUpdateTime = glfwGetTime();
		TexView::Update(TexView::Window, currUpdateTime - lastUpdateTime);
		lastUpdateTime = currUpdateTime;
	}

	// This is important. We need the destructors to run BEFORE we shutdown GLFW. Deconstructing the images may block for a bit while shutting
	// down worker threads. We could show a 'shutting down' popup here if we wanted -- if TacitImage::ThumbnailNumThreadsRunning is > 0.
	TexView::Images.Clear();

	// Get current window geometry and set in config file if we're not in fullscreen mode or iconified.
	if (!TexView::FullscreenMode && !TexView::WindowIconified)
	{
		glfwGetWindowPos(TexView::Window, &TexView::Config.WindowX, &TexView::Config.WindowY);
		glfwGetWindowSize(TexView::Window, &TexView::Config.WindowW, &TexView::Config.WindowH);
	}
	TexView::Config.Save(cfgFile);

	// Cleanup.
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(TexView::Window);
	glfwTerminate();

	// Before we go, lets clear out any old cache files.
	if (TexView::DeleteAllCacheFilesOnExit)
		tSystem::tDeleteDir(TacitImage::ThumbCacheDir);
	else
		TexView::RemoveOldCacheFiles(TacitImage::ThumbCacheDir);
	return 0;
}
