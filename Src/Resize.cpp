// Resize.cpp
//
// Dialog for resizing an image.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
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
	void DoResizeWidthHeightInterface(int& srcW, int& srcH, int& dstW, int& dstH, bool& justOpened);
	void DoResizeSampleFilterInterface(int srcW, int srcH, int dstW, int dstH);
	void DoResizeAnchorInterface();
	void DoResizeCrop(int srcW, int srcH, int dstW, int dstH);
}


void Viewer::DoResizeWidthHeightInterface(int& srcW, int& srcH, int& dstW, int& dstH, bool& justOpened)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	srcW = picture->GetWidth();
	srcH = picture->GetHeight();

	if (justOpened)
	{
		dstW = srcW;
		dstH = srcH;
	}

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


void Viewer::DoResizeSampleFilterInterface(int srcW, int srcH, int dstW, int dstH)
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

	ImGui::Text("\nAnchor: %s", (Config.CropAnchor == -1) ? "Cursor Position" : longNames[Config.CropAnchor]);
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
			CurrImage->Crop(dstW, dstH, originX, originY, Config.CropFillColour);
		}
		else
		{
			CurrImage->Crop(dstW, dstH, tPicture::Anchor(Config.CropAnchor), Config.CropFillColour);
		}
		CurrImage->Bind();
		Viewer::SetWindowTitle();
		Viewer::ZoomDownscaleOnly();
	}
}


void Viewer::DoResizeImageModal(bool justOpened)
{
	int srcW, srcH;
	static int dstW = 512;
	static int dstH = 512;
	DoResizeWidthHeightInterface(srcW, srcH, dstW, dstH, justOpened);
	DoResizeSampleFilterInterface(srcW, srcH, dstW, dstH);

	ImGui::NewLine();
	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
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
	int srcW, srcH;
	static int dstW = 512;
	static int dstH = 512;

	DoResizeWidthHeightInterface(srcW, srcH, dstW, dstH, justOpened);
	DoResizeSampleFilterInterface(srcW, srcH, dstW, dstH);
	DoResizeAnchorInterface();

	if ((dstW > srcW) || (dstH > srcH))
	{
		tColourf floatCol(Config.CropFillColour);
		ImGui::ColorEdit4("Fill Colour", floatCol.E, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf);
		Config.CropFillColour.Set(floatCol);
	}

	ImGui::NewLine();
	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		Config.CropAnchor		= 4;
		Config.CropFillColour	= tColouri::black;
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Resize", tVector2(100, 0)))
	{
		DoResizeCrop(srcW, srcH, dstW, dstH);
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


void Viewer::DoResizeAspectModal(bool justOpened)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	int srcW = picture->GetWidth();
	int srcH = picture->GetHeight();

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

	const char* resizeAspectModes[] = { "Crop Mode", "Letterbox Mode" };
	ImGui::Combo("Resize Mode", &Config.ResizeAspectMode, resizeAspectModes, tNumElements(resizeAspectModes), tNumElements(resizeAspectModes));
	ImGui::SameLine();
	ShowHelpMark("Crop mode cuts off sides resulting in a filled image.\nLetterbox mode adds coloured borders resulting in whole image being visible.");

	DoResizeAnchorInterface();
	if (Config.ResizeAspectMode == 1)
	{
		tColourf floatCol(Config.CropFillColour);
		ImGui::ColorEdit4("Fill Colour", floatCol.E, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf);
		Config.CropFillColour.Set(floatCol);
	}

	ImGui::NewLine();
	if (ImGui::Button("Reset", tVector2(100, 0)))
	{
		Config.CropAnchor		= 4;
		Config.CropFillColour	= tColouri::black;
		Config.ResizeAspectNum	= 16;
		Config.ResizeAspectDen	= 9;
		Config.ResizeAspectMode	= 0;
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
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

	ImGui::EndPopup();
}
