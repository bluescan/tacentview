// Resize.cpp
//
// Dialog for resizing an image.
//
// Copyright (c) 2020, 2021, 2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "imgui.h"
#include "Resize.h"
#include "Image.h"
#include "TacentView.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;


namespace Viewer
{
	void DoResizeWidthHeightInterface(int srcW, int srcH, int& dstW, int& dstH);
	void DoResizeFilterInterface(int srcW, int srcH, int dstW, int dstH);
	void DoResizeAnchorInterface();
	void DoFillColourInterface(const char* tootTipText = nullptr);
	void DoResizeCrop(int srcW, int srcH, int dstW, int dstH);

	void DoResizeCanvasAnchorTab(bool justOpened);
	void DoResizeCanvasRemoveBordersTab(bool justOpened);
	void DoResizeCanvasAspectTab(bool justOpened);
}


void Viewer::DoResizeWidthHeightInterface(int srcW, int srcH, int& dstW, int& dstH)
{
	float aspect = float(srcW) / float(srcH);
	static bool lockAspect = true;

	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Width", &dstW) && lockAspect)
		dstH = int( float(dstW) / aspect );
	ImGui::PopItemWidth();
	tiClampMin(dstW, 4); tiClampMin(dstH, 4);

	static char lo[32];
	static char hi[32];
	int loP2W = tNextLowerPower2(dstW);		tiClampMin(loP2W, 4);	tsPrintf(lo, "w%d", loP2W);
	int hiP2W = tNextHigherPower2(dstW);							tsPrintf(hi, "w%d", hiP2W);
	ImGui::SameLine(); if (ImGui::Button(lo))
		{ dstW = loP2W; if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); if (ImGui::Button(hi))
		{ dstW = hiP2W; if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output width in pixels.\nIf dimensions match current no scaling.");

	if (ImGui::Checkbox("Lock Aspect", &lockAspect) && lockAspect)
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Height", &dstH) && lockAspect)
		dstW = int( float(dstH) * aspect );
	ImGui::PopItemWidth();
	tiClampMin(dstW, 4); tiClampMin(dstH, 4);

	int loP2H = tNextLowerPower2(dstH);		tiClampMin(loP2H, 4);	tsPrintf(lo, "h%d", loP2H);
	int hiP2H = tNextHigherPower2(dstH);							tsPrintf(hi, "h%d", hiP2H);
	ImGui::SameLine(); if (ImGui::Button(lo))
		{ dstH = loP2H; if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine(); if (ImGui::Button(hi))
		{ dstH = hiP2H; if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output height in pixels.\nIf dimensions match current no scaling.");
}


void Viewer::DoResizeFilterInterface(int srcW, int srcH, int dstW, int dstH)
{
	if ((dstW == srcW) && (dstH == srcH))
		return;

	ImGui::Combo("Filter", &Config::Current->ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	ImGui::Combo("Filter Edge Mode", &Config::Current->ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
	ImGui::SameLine();
	ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");	
}


void Viewer::DoResizeAnchorInterface()
{
	static const char* longNames[3*3] = { "Top-Left", "Top-Middle", "Top-Right", "Middle-Left", "Middle", "Middle-Right", "Bottom-Left", "Bottom-Middle", "Bottom-Right" };

	ImGui::NewLine();
	ImGui::SetCursorPosX(58.0f);
	ImGui::Text("Anchor: %s", (Config::Current->CropAnchor == -1) ? "Cursor Position" : longNames[Config::Current->CropAnchor]);
	ImGui::SameLine();
	ShowHelpMark("Choose an anchor below. To use the cursor position, deselect the current anchor.");

	tVector2 imgSize(24.0f, 24.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);
	float ancLeft = 98.0f;
	float ancSpace = 6.0f;

	// Top Row
	ImGui::SetCursorPosX(ancLeft);

	bool selTL = (Config::Current->CropAnchor == int(Anchor::TL));
	ImGui::PushID("TL");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selTL ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selTL ? -1 : int(Anchor::TL);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ancSpace);
	bool selTM = (Config::Current->CropAnchor == int(Anchor::TM));
	ImGui::PushID("TM");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selTM ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selTM ? -1 : int(Anchor::TM);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ancSpace);
	bool selTR = (Config::Current->CropAnchor == int(Anchor::TR));
	ImGui::PushID("TR");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, selTR ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selTR ? -1 : int(Anchor::TR);
	ImGui::PopID();

	// Middle Row
	ImGui::SetCursorPosX(ancLeft);

	bool selML = (Config::Current->CropAnchor == int(Anchor::ML));
	ImGui::PushID("ML");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selML ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selML ? -1 : int(Anchor::ML);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ancSpace);
	bool selMM = (Config::Current->CropAnchor == int(Anchor::MM));
	ImGui::PushID("MM");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorMM.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selMM ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selMM ? -1 : int(Anchor::MM);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ancSpace);
	bool selMR = (Config::Current->CropAnchor == int(Anchor::MR));
	ImGui::PushID("MR");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), imgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, selMR ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selMR ? -1 : int(Anchor::MR);
	ImGui::PopID();

	// Bottom Row
	ImGui::SetCursorPosX(ancLeft);

	bool selBL = (Config::Current->CropAnchor == int(Anchor::BL));
	ImGui::PushID("BL");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, selBL ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selBL ? -1 : int(Anchor::BL);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ancSpace);
	bool selBM = (Config::Current->CropAnchor == int(Anchor::BM));
	ImGui::PushID("BM");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), imgSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, selBM ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selBM ? -1 : int(Anchor::BM);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ancSpace);
	bool selBR = (Config::Current->CropAnchor == int(Anchor::BR));
	ImGui::PushID("BR");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG, selBR ? ColourEnabledTint : ColourDisabledTint))
		Config::Current->CropAnchor = selBR ? -1 : int(Anchor::BR);
	ImGui::PopID();
}


void Viewer::DoResizeCrop(int srcW, int srcH, int dstW, int dstH)
{
	if ((dstW != srcW) || (dstH != srcH))
	{
		CurrImage->Unbind();
		if (Config::Current->CropAnchor == -1)
		{
			int originX = (Viewer::CursorX * (srcW - dstW)) / srcW;
			int originY = (Viewer::CursorY * (srcH - dstH)) / srcH;
			CurrImage->Crop(dstW, dstH, originX, originY, Config::Current->FillColour);
		}
		else
		{
			CurrImage->Crop(dstW, dstH, tPicture::Anchor(Config::Current->CropAnchor), Config::Current->FillColour);
		}
		CurrImage->Bind();
		Viewer::SetWindowTitle();
		Viewer::ZoomDownscaleOnly();
	}
}


void Viewer::DoFillColourInterface(const char* toolTipText)
{
	tColourf floatCol(Config::Current->FillColour);
	ImGui::ColorEdit4
	(
		"Fill Colour", floatCol.E,
		ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs
	);
	Config::Current->FillColour.Set(floatCol);
	if (toolTipText)
		ShowToolTip(toolTipText);

	ImGui::SameLine();
	tPicture* picture = CurrImage ? CurrImage->GetCurrentPic() : nullptr;
	if (ImGui::Button("Origin", tVector2(63.0f, 0.0f)) && picture)
		Config::Current->FillColour = picture->GetPixel(0, 0);
	ShowToolTip("Pick the colour from pixel (0, 0) in the current image.");

	ImGui::SameLine();
	if (ImGui::Button("Cursor", tVector2(63.0f, 0.0f)))
		Config::Current->FillColour.Set(Viewer::PixelColour);
	ShowToolTip("Pick the colour from the cursor pixel in the current image.");
}


//
// Below are the top-level modals.
//


void Viewer::DoResizeImageModal(bool resizeImagePressed)
{
	if (resizeImagePressed)
		ImGui::OpenPopup("Resize Image");

	bool isOpenResizeImage = true;
	if (!ImGui::BeginPopupModal("Resize Image", &isOpenResizeImage, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tAssert(CurrImage);		tPicture* picture = CurrImage->GetCurrentPic();		tAssert(picture);
	int srcW				= picture->GetWidth();
	int srcH				= picture->GetHeight();
	static int dstW			= 512;
	static int dstH			= 512;
	if (resizeImagePressed)	{ dstW = srcW; dstH = srcH; }

	DoResizeWidthHeightInterface(srcW, srcH, dstW, dstH);
	DoResizeFilterInterface(srcW, srcH, dstW, dstH);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100.0f, 0.0f)))
	{
		dstW = srcW;
		dstH = srcH;
	}
	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100.0f, 0.0f)))
	{
		if ((dstW != srcW) || (dstH != srcH))
		{
			CurrImage->Unbind();
			CurrImage->Resample(dstW, dstH, tImage::tResampleFilter(Config::Current->ResampleFilter), tImage::tResampleEdgeMode(Config::Current->ResampleEdgeMode));
			CurrImage->Bind();
			Viewer::SetWindowTitle();
			Viewer::ZoomDownscaleOnly();
		}
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


void Viewer::DoResizeCanvasModal(bool resizeCanvasPressed)
{
	if (resizeCanvasPressed)
		ImGui::OpenPopup("Resize Canvas");
	bool isOpenResizeCanvas = true;
	if (!ImGui::BeginPopupModal("Resize Canvas", &isOpenResizeCanvas, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	static bool justOpenedAnchor = false;
	static bool justOpenedBorder = false;
	static bool justOpenedAspect = false;
	if (resizeCanvasPressed)
	{
		justOpenedAnchor = true;
		justOpenedBorder = true;
		justOpenedAspect = true;
	}

	// There are 3 resize canvas modes: Anchor, Border, Aspect. Each gets its own tab.
	bool tab = false;
	if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
	{
		tab = ImGui::BeginTabItem("Anchor");
		ShowToolTip("Choose an anchor and new dimensions.");
		if (tab)
		{
			DoResizeCanvasAnchorTab(justOpenedAnchor);
			justOpenedAnchor = false;
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Remove Borders");
		ShowToolTip("Remove same-coloured border from image.");
		if (tab)
		{
			DoResizeCanvasRemoveBordersTab(justOpenedBorder);
			justOpenedBorder = false;
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Aspect");
		ShowToolTip("Choose the new aspect ratio.");
		if (tab)
		{
			DoResizeCanvasAspectTab(justOpenedAspect);
			justOpenedAspect = false;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::EndPopup();
}


void Viewer::DoResizeCanvasAnchorTab(bool justOpened)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);

	int srcW					= picture->GetWidth();
	int srcH					= picture->GetHeight();
	static int dstW				= 512;
	static int dstH				= 512;
	if (justOpened)
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::NewLine();
	DoResizeWidthHeightInterface(srcW, srcH, dstW, dstH);
	if ((dstW > srcW) || (dstH > srcH))
		DoFillColourInterface();

	DoResizeAnchorInterface();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100.0f, 0.0f)))
	{
		Config::Current->CropAnchor		= 4;
		Config::Current->FillColour		= tColouri::black;
		dstW							= srcW;
		dstH							= srcH;
	}

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100.0f, 0.0f)))
	{
		DoResizeCrop(srcW, srcH, dstW, dstH);
		ImGui::CloseCurrentPopup();
	}
}


void Viewer::DoResizeCanvasRemoveBordersTab(bool justOpened)
{
	static bool channelR		= true;
	static bool channelG		= true;
	static bool channelB		= true;
	static bool channelA		= true;
	ImGui::NewLine();

	// You cannot have all channels off.
	if (ImGui::Checkbox("R", &channelR) && !channelR && !channelG && !channelB && !channelA)
		channelR = true;
	ImGui::SameLine();
	if (ImGui::Checkbox("G", &channelG) && !channelR && !channelG && !channelB && !channelA)
		channelG = true;
	ImGui::SameLine();
	if (ImGui::Checkbox("B", &channelB) && !channelR && !channelG && !channelB && !channelA)
		channelB = true;
	ImGui::SameLine();
	if (ImGui::Checkbox("A", &channelA) && !channelR && !channelG && !channelB && !channelA)
		channelA = true;
	ImGui::SameLine();
	ImGui::Text("Channels");
	ShowToolTip("These channels are checked for border colour match.\nAt least one must be selected.");

	DoFillColourInterface("If border matches this colour it will be cropped.");

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100.0f, 0.0f)))
	{
		Config::Current->FillColour.Set(Viewer::PixelColour);
		channelR = true;
		channelG = true;
		channelB = true;
		channelA = true;
	}

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Crop Borders", tVector2(100.0f, 0.0f)))
	{
		uint32 channels =
			(channelR ? tMath::ColourChannel_R : 0) |
			(channelG ? tMath::ColourChannel_G : 0) |
			(channelB ? tMath::ColourChannel_B : 0) |
			(channelA ? tMath::ColourChannel_A : 0);

		CurrImage->Unbind();
		CurrImage->Crop(Config::Current->FillColour, channels);
		CurrImage->Bind();
		Viewer::SetWindowTitle();
		Viewer::ZoomDownscaleOnly();
		ImGui::CloseCurrentPopup();
	}
}


void Viewer::DoResizeCanvasAspectTab(bool justOpened)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	int srcW = picture->GetWidth();
	int srcH = picture->GetHeight();

	ImGui::NewLine();
	ImGui::PushItemWidth(100);
	ImGui::InputInt("Num", &Config::Current->ResizeAspectNum);
	ImGui::InputInt("Den", &Config::Current->ResizeAspectDen);
	ImGui::PopItemWidth();
	tiClampMin(Config::Current->ResizeAspectNum, 1); tiClampMin(Config::Current->ResizeAspectDen, 1);

	int presetIndex = 0;
	if      ((Config::Current->ResizeAspectNum == 2 ) && (Config::Current->ResizeAspectDen == 1 ))	presetIndex = 1;
	else if ((Config::Current->ResizeAspectNum == 16) && (Config::Current->ResizeAspectDen == 9 ))	presetIndex = 2;
	else if ((Config::Current->ResizeAspectNum == 16) && (Config::Current->ResizeAspectDen == 10))	presetIndex = 3;
	else if ((Config::Current->ResizeAspectNum == 3 ) && (Config::Current->ResizeAspectDen == 2 ))	presetIndex = 4;
	else if ((Config::Current->ResizeAspectNum == 4 ) && (Config::Current->ResizeAspectDen == 3 ))	presetIndex = 5;
	else if ((Config::Current->ResizeAspectNum == 1 ) && (Config::Current->ResizeAspectDen == 1 ))	presetIndex = 6;
	else if ((Config::Current->ResizeAspectNum == 3 ) && (Config::Current->ResizeAspectDen == 4 ))	presetIndex = 7;
	else if ((Config::Current->ResizeAspectNum == 2 ) && (Config::Current->ResizeAspectDen == 3 ))	presetIndex = 8;
	else if ((Config::Current->ResizeAspectNum == 10) && (Config::Current->ResizeAspectDen == 16))	presetIndex = 9;
	else if ((Config::Current->ResizeAspectNum == 9 ) && (Config::Current->ResizeAspectDen == 16))	presetIndex = 10;
	else if ((Config::Current->ResizeAspectNum == 1 ) && (Config::Current->ResizeAspectDen == 2 ))	presetIndex = 11;
	const char* presetAspects[] = { "Custom", "2:1", "16:9", "16:10", "3:2", "4:3", "1:1", "3:4", "2:3", "10:16", "9:16", "1:2" };
	if (ImGui::Combo("Aspect", &presetIndex, presetAspects, tNumElements(presetAspects), tNumElements(presetAspects)))
	{
		switch (presetIndex)
		{
			case 0:																						break;
			case 1:		Config::Current->ResizeAspectNum = 2;	Config::Current->ResizeAspectDen = 1;	break;
			case 2:		Config::Current->ResizeAspectNum = 16;	Config::Current->ResizeAspectDen = 9;	break;
			case 3:		Config::Current->ResizeAspectNum = 16;	Config::Current->ResizeAspectDen = 10;	break;
			case 4:		Config::Current->ResizeAspectNum = 3;	Config::Current->ResizeAspectDen = 2;	break;
			case 5:		Config::Current->ResizeAspectNum = 4;	Config::Current->ResizeAspectDen = 3;	break;
			case 6:		Config::Current->ResizeAspectNum = 1;	Config::Current->ResizeAspectDen = 1;	break;
			case 7:		Config::Current->ResizeAspectNum = 3;	Config::Current->ResizeAspectDen = 4;	break;
			case 8:		Config::Current->ResizeAspectNum = 2;	Config::Current->ResizeAspectDen = 3;	break;
			case 9:		Config::Current->ResizeAspectNum = 10;	Config::Current->ResizeAspectDen = 16;	break;
			case 10:	Config::Current->ResizeAspectNum = 9;	Config::Current->ResizeAspectDen = 16;	break;
			case 11:	Config::Current->ResizeAspectNum = 1;	Config::Current->ResizeAspectDen = 2;	break;
		}
	}

	const char* resizeAspectModes[] = { "Crop", "Letterbox" };
	ImGui::Combo("Mode", &Config::Current->ResizeAspectMode, resizeAspectModes, tNumElements(resizeAspectModes), tNumElements(resizeAspectModes));
	ImGui::SameLine();
	ShowHelpMark("Crop mode cuts off sides resulting in a filled image.\nLetterbox mode adds coloured borders resulting in whole image being visible.");

	if (Config::Current->ResizeAspectMode == 1)
		DoFillColourInterface();

	DoResizeAnchorInterface();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100.0f, 0.0f)))
	{
		Config::Current->CropAnchor		= 4;
		Config::Current->FillColour		= tColouri::black;
		Config::Current->ResizeAspectNum	= 16;
		Config::Current->ResizeAspectDen	= 9;
		Config::Current->ResizeAspectMode	= 0;
	}

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100.0f, 0.0f)))
	{
		int dstH = srcH;
		int dstW = srcW;
		float srcAspect = float(srcW)/float(srcH);
		float dstAspect = float(Config::Current->ResizeAspectNum)/float(Config::Current->ResizeAspectDen);
		if (Config::Current->ResizeAspectMode == 0)
		{
			// Crop Mode.
			if (dstAspect > srcAspect)
				dstH = tFloatToInt(float(dstW) / dstAspect);
			else if (dstAspect < srcAspect)
				dstW = tFloatToInt(float(dstH) * dstAspect);
		}
		else
		{
			// Letterbox Mode.
			if (dstAspect > srcAspect)
				dstW = tFloatToInt(float(dstH) * dstAspect);
			else if (dstAspect < srcAspect)
				dstH = tFloatToInt(float(dstW) / dstAspect);
		}

		DoResizeCrop(srcW, srcH, dstW, dstH);
		ImGui::CloseCurrentPopup();
	}
}
