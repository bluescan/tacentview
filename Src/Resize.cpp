// Resize.cpp
//
// Dialog for resizing an image.
//
// Copyright (c) 2020, 2021 Tristan Grimmer.
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

	ImGui::Combo("Filter", &Config.ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	ImGui::Combo("Filter Edge Mode", &Config.ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
	ImGui::SameLine();
	ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");	
}


void Viewer::DoResizeAnchorInterface()
{
	static const char* shortNames[3*3] = { "TL", "TM", "TR", "ML", "MM", "MR", "BL", "BM", "BR" };
	static const char* longNames[3*3] = { "Top-Left", "Top-Middle", "Top-Right", "Middle-Left", "Middle", "Middle-Right", "Bottom-Left", "Bottom-Middle", "Bottom-Right" };

	ImGui::NewLine();
	ImGui::Text("Anchor: %s", (Config.CropAnchor == -1) ? "Cursor Position" : longNames[Config.CropAnchor]);
	ImGui::SameLine();
	ShowHelpMark("Choose an anchor below. To use the cursor position, deselect the current anchor.");

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, tVector2(0.5f, 0.5f));
	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 3; x++)
		{
			int index = 3*y + x;
			char name[4];
			tsPrintf(name, "%s", shortNames[index]);
			if (x > 0)
				ImGui::SameLine();

			bool selected = (Config.CropAnchor == index);
			if (ImGui::Selectable(name, &selected, ImGuiSelectableFlags_DontClosePopups, ImVec2(22, 22)))
				Config.CropAnchor = !selected ? -1 : index;
		}
	}
	ImGui::PopStyleVar();
}


void Viewer::DoResizeCrop(int srcW, int srcH, int dstW, int dstH)
{
	if ((dstW != srcW) || (dstH != srcH))
	{
		CurrImage->Unbind();
		if (Config.CropAnchor == -1)
		{
			int originX = (Viewer::CursorX * (srcW - dstW)) / srcW;
			int originY = (Viewer::CursorY * (srcH - dstH)) / srcH;
			CurrImage->Crop(dstW, dstH, originX, originY, Config.FillColour);
		}
		else
		{
			CurrImage->Crop(dstW, dstH, tPicture::Anchor(Config.CropAnchor), Config.FillColour);
		}
		CurrImage->Bind();
		Viewer::SetWindowTitle();
		Viewer::ZoomDownscaleOnly();
	}
}


void Viewer::DoFillColourInterface(const char* toolTipText)
{
	tColourf floatCol(Config.FillColour);
	ImGui::ColorEdit4
	(
		"Fill Colour", floatCol.E,
		ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs
	);
	Config.FillColour.Set(floatCol);
	if (toolTipText)
		ShowToolTip(toolTipText);

	ImGui::SameLine();
	tPicture* picture = CurrImage ? CurrImage->GetCurrentPic() : nullptr;
	if (ImGui::Button("Origin", tVector2(63, 0)) && picture)
		Config.FillColour = picture->GetPixel(0, 0);
	ShowToolTip("Pick the colour from pixel (0, 0) in the current image.");

	ImGui::SameLine();
	if (ImGui::Button("Cursor", tVector2(63, 0)))
		Config.FillColour.Set(Viewer::PixelColour);
	ShowToolTip("Pick the colour from the cursor pixel in the current image.");
}


//
// Below are the top-level modals.
//


void Viewer::DoResizeImageModal(bool justOpened)
{
	tAssert(CurrImage);		tPicture* picture = CurrImage->GetCurrentPic();		tAssert(picture);
	int srcW				= picture->GetWidth();
	int srcH				= picture->GetHeight();
	static int dstW			= 512;
	static int dstH			= 512;
	if (justOpened)			{ dstW = srcW; dstH = srcH; }

	DoResizeWidthHeightInterface(srcW, srcH, dstW, dstH);
	DoResizeFilterInterface(srcW, srcH, dstW, dstH);

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		dstW = srcW;
		dstH = srcH;
	}
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100, 0)))
	{
		if ((dstW != srcW) || (dstH != srcH))
		{
			CurrImage->Unbind();
			CurrImage->Resample(dstW, dstH, tImage::tResampleFilter(Config.ResampleFilter), tImage::tResampleEdgeMode(Config.ResampleEdgeMode));
			CurrImage->Bind();
			Viewer::SetWindowTitle();
			Viewer::ZoomDownscaleOnly();
		}
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


void Viewer::DoResizeCanvasModal(bool justOpened)
{
	static bool justOpenedAnchor = false;
	static bool justOpenedBorder = false;
	static bool justOpenedAspect = false;
	if (justOpened)
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

	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		Config.CropAnchor		= 4;
		Config.FillColour		= tColouri::black;
		dstW = srcW;
		dstH = srcH;
	}

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100, 0)))
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

	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		Config.FillColour.Set(Viewer::PixelColour);
		channelR = true;
		channelG = true;
		channelB = true;
		channelA = true;
	}

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Crop Borders", tVector2(100, 0)))
	{
		uint32 channels =
			(channelR ? tMath::ColourChannel_R : 0) |
			(channelG ? tMath::ColourChannel_G : 0) |
			(channelB ? tMath::ColourChannel_B : 0) |
			(channelA ? tMath::ColourChannel_A : 0);

		CurrImage->Unbind();
		CurrImage->Crop(Config.FillColour, channels);
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
	ImGui::InputInt("Num", &Config.ResizeAspectNum);
	ImGui::InputInt("Den", &Config.ResizeAspectDen);
	ImGui::PopItemWidth();
	tiClampMin(Config.ResizeAspectNum, 1); tiClampMin(Config.ResizeAspectDen, 1);

	int presetIndex = 0;
	if      ((Config.ResizeAspectNum == 2 ) && (Config.ResizeAspectDen == 1 ))	presetIndex = 1;
	else if ((Config.ResizeAspectNum == 16) && (Config.ResizeAspectDen == 9 ))	presetIndex = 2;
	else if ((Config.ResizeAspectNum == 16) && (Config.ResizeAspectDen == 10))	presetIndex = 3;
	else if ((Config.ResizeAspectNum == 4 ) && (Config.ResizeAspectDen == 3 ))	presetIndex = 4;
	else if ((Config.ResizeAspectNum == 1 ) && (Config.ResizeAspectDen == 1 ))	presetIndex = 5;
	else if ((Config.ResizeAspectNum == 3 ) && (Config.ResizeAspectDen == 4 ))	presetIndex = 6;
	else if ((Config.ResizeAspectNum == 10) && (Config.ResizeAspectDen == 16))	presetIndex = 7;
	else if ((Config.ResizeAspectNum == 9 ) && (Config.ResizeAspectDen == 16))	presetIndex = 8;
	else if ((Config.ResizeAspectNum == 1 ) && (Config.ResizeAspectDen == 2 ))	presetIndex = 9;
	const char* presetAspects[] = { "Custom", "2:1", "16:9", "16:10", "4:3", "1:1", "3:4", "10:16", "9:16", "1:2" };
	if (ImGui::Combo("Aspect", &presetIndex, presetAspects, tNumElements(presetAspects), tNumElements(presetAspects)))
	{
		switch (presetIndex)
		{
			case 0:																	break;
			case 1:	Config.ResizeAspectNum = 2;		Config.ResizeAspectDen = 1;		break;
			case 2:	Config.ResizeAspectNum = 16;	Config.ResizeAspectDen = 9;		break;
			case 3:	Config.ResizeAspectNum = 16;	Config.ResizeAspectDen = 10;	break;
			case 4:	Config.ResizeAspectNum = 4;		Config.ResizeAspectDen = 3;		break;
			case 5:	Config.ResizeAspectNum = 1;		Config.ResizeAspectDen = 1;		break;
			case 6:	Config.ResizeAspectNum = 3;		Config.ResizeAspectDen = 4;		break;
			case 7:	Config.ResizeAspectNum = 10;	Config.ResizeAspectDen = 16;	break;
			case 8:	Config.ResizeAspectNum = 9;		Config.ResizeAspectDen = 16;	break;
			case 9:	Config.ResizeAspectNum = 1;		Config.ResizeAspectDen = 2;		break;
		}
	}

	const char* resizeAspectModes[] = { "Crop", "Letterbox" };
	ImGui::Combo("Mode", &Config.ResizeAspectMode, resizeAspectModes, tNumElements(resizeAspectModes), tNumElements(resizeAspectModes));
	ImGui::SameLine();
	ShowHelpMark("Crop mode cuts off sides resulting in a filled image.\nLetterbox mode adds coloured borders resulting in whole image being visible.");

	if (Config.ResizeAspectMode == 1)
		DoFillColourInterface();

	DoResizeAnchorInterface();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		Config.CropAnchor		= 4;
		Config.FillColour		= tColouri::black;
		Config.ResizeAspectNum	= 16;
		Config.ResizeAspectDen	= 9;
		Config.ResizeAspectMode	= 0;
	}

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100, 0)))
	{
		int dstH = srcH;
		int dstW = srcW;
		float srcAspect = float(srcW)/float(srcH);
		float dstAspect = float(Config.ResizeAspectNum)/float(Config.ResizeAspectDen);
		if (Config.ResizeAspectMode == 0)
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
