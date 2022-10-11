// TacentView.h
//
// A texture viewer for various formats.
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
#include <Math/tVector2.h>
#include <Math/tVector4.h>
#include <System/tCmdLine.h>
#include "Config.h"
#include "FileDialog.h"
namespace Viewer { class Image; }


namespace Viewer
{
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
	extern tFileDialog::FileDialog OpenFileDialog;
	extern tFileDialog::FileDialog OpenDirDialog;
	extern tFileDialog::FileDialog SaveAsDialog;

	extern Image* CurrImage;
	extern tString ImagesDir;
	extern tList<tStringItem> ImagesSubDirs;
	extern tList<Viewer::Image> Images;
	extern tItList<Viewer::Image> ImagesLoadTimeSorted;
	extern tCmdLine::tParam ImageFileParam;
	extern tColouri PixelColour;
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

	extern Config::ProfileSettings::ZoomMode CurrZoomMode;
	extern bool CropMode;
	extern Anchor Request_PanSnap;
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
	extern float RotateAnglePreview;

	extern const tMath::tVector4 ColourEnabledTint;
	extern const tMath::tVector4 ColourDisabledTint;
	extern const tMath::tVector4 ColourBG;
	extern const tMath::tVector4 ColourPressedBG;
	extern const tMath::tVector4 ColourClear;

	const int TopUIHeight			= 26;
	const float DialogOrigin		= 12.0f;
	const float DialogDelta			= 19.0f;
	const double DisappearDuration	= 4.0;

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	void ShowHelpMark(const char* desc, bool autoWrap = true);
	void ShowToolTip(const char* desc, bool autoWrap = true);
	void PopulateImages();
	void PopulateImagesSubDirs();
	Image* FindImage(const tString& filename);
	void SetCurrentImage(const tString& currFilename = tString());
	void LoadCurrImage();
	bool ChangeScreenMode(bool fullscreeen, bool force = false);
	void SortImages(Config::ProfileSettings::SortKeyEnum, bool ascending);
	bool DeleteImageFile(const tString& imgFile, bool tryUseRecycleBin);
	void SetWindowTitle();
	void ZoomFit();
	void ZoomDownscaleOnly();
	void ResetPan(bool resetX = true, bool resetY = true);
	int GetPanX();
	int GetPanY();
	tMath::tVector2 GetDialogOrigin(float index);

	void Undo();
	void Redo();

	void ConvertScreenPosToImagePos
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
