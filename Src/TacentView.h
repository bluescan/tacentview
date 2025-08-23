// TacentView.h
//
// A texture viewer for various formats.
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
#include <Math/tVector2.h>
#include <Math/tVector4.h>
#include <System/tCmdLine.h>
#include "Config.h"
#include "FileDialog.h"
namespace Viewer { class Image; }
struct GLFWwindow;


namespace Viewer
{
	enum ErrorCode
	{
		ErrorCode_Success					= 0,
		ErrorCode_GUI_FailGLFWInit			= 10,
		ErrorCode_GUI_FailGLFWWindow		= 20,
		ErrorCode_GUI_FailGLADInit			= 30,
		ErrorCode_GUI_FailAssetDirMissing	= 40,
		ErrorCode_GUI_FailConfigDirMissing	= 50,
		ErrorCode_GUI_FailCacheDirMissing	= 60,

		ErrorCode_CLI_FailUnknown			= 100,
		ErrorCode_CLI_FailImageLoad			= 110,
		ErrorCode_CLI_FailImageProcess		= 120,
		ErrorCode_CLI_FailEarlyExit			= 130,
		ErrorCode_CLI_FailImageSave			= 140,
	};

	enum class Anchor
	{
		Invalid			= -1,
		TopLeft,		TL = TopLeft,
		TopMiddle,		TM = TopMiddle,
		TopRight,		TR = TopRight,
		MiddleLeft,		ML = MiddleLeft,
		Middle,			MM = Middle,
		MiddleRight,	MR = MiddleRight,
		BottomLeft,		BL = BottomLeft,
		BottomMiddle,	BM = BottomMiddle,
		BottomRight,	BR = BottomRight,
		NumAnchors
	};

	enum CursorMove
	{
		CursorMove_None,
		CursorMove_Left,
		CursorMove_Right,
		CursorMove_Up,
		CursorMove_Down
	};

	extern tSystem::tFileTypes FileTypes_Load;
	extern tSystem::tFileTypes FileTypes_Save;
	extern tSystem::tFileTypes FileTypes_MultiFrame;
	extern tSystem::tFileTypes FileTypes_SaveMultiFrame;
	extern tSystem::tFileTypes FileTypes_ClipboardPaste;
	extern tSystem::tFileTypes FileTypes_ImportRaw;
	extern tFileDialog::FileDialog OpenFileDialog;
	extern tFileDialog::FileDialog OpenDirDialog;
	extern tFileDialog::FileDialog SaveAsDialog;

	extern Image* CurrImage;
	extern tString ImageToLoad;
	extern tString ImagesDir;
	extern tList<tStringItem> ImagesSubDirs;
	extern tList<Viewer::Image> Images;
	extern tItList<Viewer::Image> ImagesLoadTimeSorted;
	extern tColour4b PixelColour;
	extern Viewer::Image Image_DefaultThumbnail;
	extern Viewer::Image Image_File;
	extern Viewer::Image Image_Folder;
	extern Viewer::Image Image_UpFolder;
	extern Viewer::Image Image_RotateTheta;
	extern Viewer::Image Image_Play_PlayRev;
	extern Viewer::Image Image_Stop;
	extern Viewer::Image Image_PlayLoop;
	extern Viewer::Image Image_PlayOnce;
	extern Viewer::Image Image_Next_Prev;
	extern Viewer::Image Image_SkipEnd_SkipBegin;
	extern Viewer::Image Image_Refresh;
	extern Viewer::Image Image_ShowHidden;
	extern Viewer::Image Image_AnchorBL;
	extern Viewer::Image Image_AnchorBM;
	extern Viewer::Image Image_AnchorML;
	extern Viewer::Image Image_AnchorMM;

	extern bool CropMode;
	extern Anchor Request_PanSnap;
	extern bool Request_CropLineConstrain;

	extern bool DeleteAllCacheFilesOnExit;
	extern bool PendingTransparentWorkArea;
	extern bool DrawChannel_AsIntensity;
	extern bool DrawChannel_R;
	extern bool DrawChannel_G;
	extern bool DrawChannel_B;
	extern bool DrawChannel_A;
	extern double SlideshowCountdown;
	extern int CursorX;
	extern int CursorY;
	extern int PendingFrameBufferBPC;
	extern float RotateAnglePreview;

	extern const tMath::tVector4 ColourEnabledTint;
	extern const tMath::tVector4 ColourDisabledTint;
	extern const tMath::tVector4 ColourBG;
	extern const tMath::tVector4 ColourPressedBG;
	extern const tMath::tVector4 ColourClear;

	extern Viewer::Config::ProfileData::UISizeEnum CurrentUISize;
	extern GLFWwindow* Window;

	const double DisappearDuration	= 4.0;

	void PopulateImages();
	void PopulateImagesSubDirs();
	Image* FindImage(const tString& filename);
	bool SetCurrentImage(const tString& currFilename = tString(), bool forceReload = false);	// Returns true if current image was in the list of images.
	void LoadCurrImage(bool forceReload = false);
	bool ChangeScreenMode(bool fullscreeen, bool force = false);
	void SortImages(Config::ProfileData::SortKeyEnum, bool ascending);
	bool DeleteImageFile(const tString& imgFile, bool tryUseRecycleBin);
	
	Config::ProfileData::ZoomModeEnum GetZoomMode();				// Reads the ZoomModePerImage setting to see where to get the zoom mode.
	void SetZoomMode(Config::ProfileData::ZoomModeEnum);			// Reads the ZoomModePerImage setting to see where to set the zoom mode.
	float GetZoomPercent();											// Reads the ZoomModePerImage setting to see where to get the zoom percent.
	void SetZoomPercent(float);										// Reads the ZoomModePerImage setting to see where to set the zoom percent.
	void ZoomFit();
	void ZoomDownscaleOnly();

	void ResetPan(bool resetX = true, bool resetY = true);
	int GetPanX();
	int GetPanY();

	// This reads the UI size from the current profile. If the profile is set to auto it queries the OS scale settings.
	// The performance profile of the OS query is unknown so it is assumed to be an expensive call. This function should
	// not be called every frame. After it determines the desired UISize it sets the DesiredUISzie variable. It is
	// always set to a valid (non-auto) UI size after this call.
	void UpdateDesiredUISize();

	void OnUndo();
	void OnRedo();

	// Returns true if clamping was necessary.
	bool ConvertScreenPosToImagePos
	(
		int& imgX, int& imgY, const tMath::tVector2& scrPos,
		const tMath::tVector4& lrtb, const tMath::tVector2& uvOff
	);
	void ConvertImagePosToScreenPos
	(
		tMath::tVector2& scrPos, int imgX, int imgY, const tMath::tVector4& lrtb,
		const tMath::tVector2& uvOff, bool centerPixel = false
	);
}

// Implementation below this line.
