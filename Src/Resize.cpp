// Resize.cpp
//
// Dialog for resizing an image.
//
// Copyright (c) 2020-2023 Tristan Grimmer.
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

	// These are used when resizing canvas.
	void DoResizeAnchorInterface();
	void DoFillColourInterface(const char* tootTipText = nullptr, bool contactSheetFillColour = false);
	void DoResizeCrop(int srcW, int srcH, int dstW, int dstH);

	void DoResizeCanvasAnchorTab(bool firstOpen);
	void DoResizeCanvasRemoveBordersTab(bool firstOpen);
	void DoResizeCanvasAspectTab(bool firstOpen);
}


void Viewer::DoResizeWidthHeightInterface(int srcW, int srcH, int& dstW, int& dstH)
{
	float aspect = float(srcW) / float(srcH);
	static bool lockAspect = true;

	Config::ProfileData& profile	= Config::GetProfileData();
	float dimWidth					= profile.GetUIParamScaled(90.0f, 2.5f);
	float dimOffset					= profile.GetUIParamScaled(140.0f, 2.5f);
	float powButtonWidth			= profile.GetUIParamScaled(44.0f, 2.5f);

	static char lo[32];
	static char hi[32];

	ImGui::PushItemWidth(dimWidth);
	if (ImGui::InputInt("Width", &dstW) && lockAspect)
		dstH = int( float(dstW) / aspect );
	ImGui::PopItemWidth();
	tiClamp(dstW, 4, 65536); tiClamp(dstH, 4, 65536);
	int loP2W = tNextLowerPower2(dstW);		tiClampMin(loP2W, 4);	tsPrintf(lo, "%d##Wlo", loP2W);
	int hiP2W = tNextHigherPower2(dstW);							tsPrintf(hi, "%d##Whi", hiP2W);
	ImGui::SetCursorPosX(dimOffset);
	ImGui::SameLine();
	ImGui::SetCursorPosX(dimOffset);
	if (ImGui::Button(lo, tVector2(powButtonWidth, 0.0f)))			{ dstW = loP2W; if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine();
	if (ImGui::Button(hi, tVector2(powButtonWidth, 0.0f)))			{ dstW = hiP2W; if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output width in pixels.\nIf dimensions match current no scaling.");

	if (ImGui::Checkbox("Lock Aspect", &lockAspect) && lockAspect)
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::PushItemWidth(dimWidth);
	if (ImGui::InputInt("Height", &dstH) && lockAspect)
		dstW = int( float(dstH) * aspect );
	tiClamp(dstW, 4, 65536); tiClamp(dstH, 4, 65536);
	ImGui::SetCursorPosX(dimOffset);
	int loP2H = tNextLowerPower2(dstH);		tiClampMin(loP2H, 4);	tsPrintf(lo, "%d##Hlo", loP2H);
	int hiP2H = tNextHigherPower2(dstH);							tsPrintf(hi, "%d##Hhi", hiP2H);
	ImGui::SameLine();
	ImGui::SetCursorPosX(dimOffset);
	if (ImGui::Button(lo, tVector2(powButtonWidth, 0.0f)))			{ dstH = loP2H; if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine();
	if (ImGui::Button(hi, tVector2(powButtonWidth, 0.0f)))			{ dstH = hiP2H; if (lockAspect) dstW = int( float(dstH) * aspect ); }

	ImGui::SameLine(); ShowHelpMark("Final output height in pixels.\nIf dimensions match current no scaling.");
}


void Viewer::DoResizeFilterInterface(int srcW, int srcH, int dstW, int dstH)
{
	if ((dstW == srcW) && (dstH == srcH))
		return;

	Config::ProfileData& profile = Config::GetProfileData();
	float comboWidth = profile.GetUIParamScaled(168.0f, 2.5f);

	ImGui::SetNextItemWidth(comboWidth);
	ImGui::Combo("Filter", &profile.ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	ImGui::SetNextItemWidth(comboWidth);
	ImGui::Combo("Edges", &profile.ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
	ImGui::SameLine();
	ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");	
}


void Viewer::DoResizeAnchorInterface()
{
	Config::ProfileData& profile = Config::GetProfileData();
	static const char* longNames[3*3] = { "Top-Left", "Top-Middle", "Top-Right", "Middle-Left", "Middle", "Middle-Right", "Bottom-Left", "Bottom-Middle", "Bottom-Right" };

	float ancTextPos	= profile.GetUIParamScaled(72.0f, 2.5f);
	ImGui::NewLine();
	ImGui::SetCursorPosX(ancTextPos);
	ImGui::Text("Anchor: %s", (profile.CropAnchor == -1) ? "Cursor Position" : longNames[profile.CropAnchor]);
	ImGui::SameLine();
	ShowHelpMark("Choose an anchor below. To use the cursor position, deselect the current anchor.");

	// Anchors.
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2::zero);
	float ancLeft		= profile.GetUIParamScaled(92.0f, 2.5f);
	float ancImgSize	= profile.GetUIParamScaled(24.0f, 2.5f);
	float ancTopMargin	= profile.GetUIParamScaled(7.0f, 2.5f);
	float ancSpacing	= profile.GetUIParamScaled(2.0f, 2.5f);
	tVector2 imgSize	(ancImgSize, ancImgSize);

	// Top Row
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ancTopMargin);
	ImGui::SetCursorPosX(ancLeft);

	bool selTL = (profile.CropAnchor == int(Anchor::TL));
	ImGui::PushID("TL");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selTL ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selTL ? -1 : int(Anchor::TL);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ancSpacing);
	bool selTM = (profile.CropAnchor == int(Anchor::TM));
	ImGui::PushID("TM");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selTM ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selTM ? -1 : int(Anchor::TM);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ancSpacing);
	bool selTR = (profile.CropAnchor == int(Anchor::TR));
	ImGui::PushID("TR");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, selTR ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selTR ? -1 : int(Anchor::TR);
	ImGui::PopID();

	// Middle Row
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ancSpacing);
	ImGui::SetCursorPosX(ancLeft);

	bool selML = (profile.CropAnchor == int(Anchor::ML));
	ImGui::PushID("ML");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selML ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selML ? -1 : int(Anchor::ML);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ancSpacing);
	bool selMM = (profile.CropAnchor == int(Anchor::MM));
	ImGui::PushID("MM");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorMM.Bind()), imgSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, selMM ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selMM ? -1 : int(Anchor::MM);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ancSpacing);
	bool selMR = (profile.CropAnchor == int(Anchor::MR));
	ImGui::PushID("MR");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), imgSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, selMR ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selMR ? -1 : int(Anchor::MR);
	ImGui::PopID();

	// Bottom Row
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ancSpacing);
	ImGui::SetCursorPosX(ancLeft);

	bool selBL = (profile.CropAnchor == int(Anchor::BL));
	ImGui::PushID("BL");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, selBL ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selBL ? -1 : int(Anchor::BL);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ancSpacing);
	bool selBM = (profile.CropAnchor == int(Anchor::BM));
	ImGui::PushID("BM");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), imgSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, selBM ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selBM ? -1 : int(Anchor::BM);
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ancSpacing);
	bool selBR = (profile.CropAnchor == int(Anchor::BR));
	ImGui::PushID("BR");
	if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), imgSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG, selBR ? ColourEnabledTint : ColourDisabledTint))
		profile.CropAnchor = selBR ? -1 : int(Anchor::BR);
	ImGui::PopID();

	ImGui::PopStyleVar();
}


void Viewer::DoResizeCrop(int srcW, int srcH, int dstW, int dstH)
{
	if ((dstW != srcW) || (dstH != srcH))
	{
		Config::ProfileData& profile = Config::GetProfileData();
		CurrImage->Unbind();
		if (profile.CropAnchor == -1)
		{
			int originX = (Viewer::CursorX * (srcW - dstW)) / srcW;
			int originY = (Viewer::CursorY * (srcH - dstH)) / srcH;
			CurrImage->Crop(dstW, dstH, originX, originY, profile.FillColour);
		}
		else
		{
			CurrImage->Crop(dstW, dstH, tPicture::Anchor(profile.CropAnchor), profile.FillColour);
		}
		CurrImage->Bind();
		Viewer::SetWindowTitle();
		Viewer::ZoomDownscaleOnly();
	}
}


void Viewer::DoFillColourInterface(const char* toolTipText, bool contactSheetFillColour)
{
	Config::ProfileData& profile = Config::GetProfileData();
	tColourf floatCol(contactSheetFillColour ? profile.FillColourContact : profile.FillColour);
	ImGui::ColorEdit4
	(
		"Fill##Colour", floatCol.E,
		ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs
	);

	tColour4i* fillColour = contactSheetFillColour ? &profile.FillColourContact : &profile.FillColour;
	fillColour->Set(floatCol);
	if (toolTipText)
		ShowToolTip(toolTipText);

	float buttonWidth = profile.GetUIParamScaled(63.0f, 2.5f);

	ImGui::SameLine();
	tPicture* picture = CurrImage ? CurrImage->GetCurrentPic() : nullptr;
	if (ImGui::Button("Origin", tVector2(buttonWidth, 0.0f)) && picture)
		fillColour->Set(picture->GetPixel(0, 0));
	ShowToolTip("Pick the colour from pixel (0, 0) in the current image.");

	ImGui::SameLine();
	if (ImGui::Button("Cursor", tVector2(buttonWidth, 0.0f)))
		fillColour->Set(Viewer::PixelColour);
	ShowToolTip("Pick the colour from the cursor pixel in the current image.");

	ImGui::SameLine();
	if (ImGui::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		if (contactSheetFillColour)
			fillColour->Set(tColour4i::transparent);
		else
			fillColour->Set(tColour4i::black);
	}
	if (contactSheetFillColour)
		ShowToolTip("Reset the fill colour to transparent black.");
	else
		ShowToolTip("Reset the fill colour to black.");
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

	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth = profile.GetUIParamScaled(78.0f, 2.5f);

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

	if (Viewer::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::SameLine();
	if (Viewer::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Resize", tVector2(buttonWidth, 0.0f)))
	{
		if ((dstW != srcW) || (dstH != srcH))
		{
			CurrImage->Unbind();
			CurrImage->Resample(dstW, dstH, tImage::tResampleFilter(profile.ResampleFilter), tImage::tResampleEdgeMode(profile.ResampleEdgeMode));
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

	static bool firstOpenAnchor = false;
	static bool firstOpenBorder = false;
	static bool firstOpenAspect = false;
	if (resizeCanvasPressed)
	{
		firstOpenAnchor = true;
		firstOpenBorder = true;
		firstOpenAspect = true;
	}

	// There are 3 resize canvas modes: Anchor, Border, Aspect. Each gets its own tab.
	bool tab = false;
	if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
	{
		tab = ImGui::BeginTabItem("Anchor");
		ShowToolTip("Choose an anchor and new dimensions.");
		if (tab)
		{
			DoResizeCanvasAnchorTab(firstOpenAnchor);
			firstOpenAnchor = false;
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Remove Borders");
		ShowToolTip("Remove same-coloured border from image.");
		if (tab)
		{
			DoResizeCanvasRemoveBordersTab(firstOpenBorder);
			firstOpenBorder = false;
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Aspect");
		ShowToolTip("Choose the new aspect ratio.");
		if (tab)
		{
			DoResizeCanvasAspectTab(firstOpenAspect);
			firstOpenAspect = false;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::EndPopup();
}


void Viewer::DoResizeCanvasAnchorTab(bool firstOpen)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);

	int srcW					= picture->GetWidth();
	int srcH					= picture->GetHeight();
	static int dstW				= 512;
	static int dstH				= 512;
	if (firstOpen)
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

	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth = profile.GetUIParamScaled(78.0f, 2.5f);

	if (Viewer::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		profile.CropAnchor		= 4;
		profile.FillColour		= tColouri::black;
		dstW					= srcW;
		dstH					= srcH;
	}

	ImGui::SameLine();
	if (Viewer::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Resize", tVector2(buttonWidth, 0.0f)))
	{
		DoResizeCrop(srcW, srcH, dstW, dstH);
		ImGui::CloseCurrentPopup();
	}
}


void Viewer::DoResizeCanvasRemoveBordersTab(bool firstOpen)
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

	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth = profile.GetUIParamScaled(78.0f, 2.5f);

	if (Viewer::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		profile.FillColour.Set(Viewer::PixelColour);
		channelR = true;
		channelG = true;
		channelB = true;
		channelA = true;
	}

	ImGui::SameLine();
	if (Viewer::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Remove", tVector2(buttonWidth, 0.0f)))
	{
		uint32 channels =
			(channelR ? tCompBit_R : 0) |
			(channelG ? tCompBit_G : 0) |
			(channelB ? tCompBit_B : 0) |
			(channelA ? tCompBit_A : 0);

		CurrImage->Unbind();
		CurrImage->Deborder(profile.FillColour, channels);
		CurrImage->Bind();
		Viewer::SetWindowTitle();
		Viewer::ZoomDownscaleOnly();
		ImGui::CloseCurrentPopup();
	}
}


void Viewer::DoResizeCanvasAspectTab(bool firstOpen)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	int srcW = picture->GetWidth();
	int srcH = picture->GetHeight();
	Config::ProfileData& profile = Config::GetProfileData();
	float comboWidth = profile.GetUIParamScaled(108.0f, 2.5f);
	float inputWidth = profile.GetUIParamScaled(26.0f, 2.5f);

	ImGui::NewLine();
	ImGui::PushItemWidth(comboWidth);
	ImGui::Combo
	(
		"Aspect",
		&profile.ResizeAspectRatio,
		&tImage::tAspectRatioNames[1],
		int(tImage::tAspectRatio::NumRatios),
		int(tImage::tAspectRatio::NumRatios)/2
	);
	ImGui::PopItemWidth();

	if (profile.GetResizeAspectRatio() == tAspectRatio::User)
	{
		ImGui::SameLine();
		ImGui::PushItemWidth(inputWidth);
		ImGui::InputInt("##Num", &profile.ResizeAspectUserNum, 0, 0);
		ImGui::SameLine(); ImGui::Text(":"); ImGui::SameLine();
		ImGui::InputInt("##Den", &profile.ResizeAspectUserDen, 0, 0);
		ImGui::PopItemWidth();
		tiClamp(profile.ResizeAspectUserNum, 1, 99); tiClamp(profile.ResizeAspectUserDen, 1, 99);
	}
	else
	{
		ImGui::SameLine();
		ShowHelpMark("Aspect ratio for resizing.\nUser means enter the aspect ratio manually.\nFor the print presets the L means Landscape.");
	}

	const char* resizeAspectModes[] = { "Crop", "Letterbox" };
	ImGui::SetNextItemWidth(comboWidth);
	ImGui::Combo("Mode", &profile.ResizeAspectMode, resizeAspectModes, tNumElements(resizeAspectModes), tNumElements(resizeAspectModes));

	ImGui::SameLine();
	ShowHelpMark("Crop mode cuts off sides resulting in a filled image.\nLetterbox mode adds coloured borders resulting in whole image being visible.");

	if (profile.ResizeAspectMode == 1)
		DoFillColourInterface();

	DoResizeAnchorInterface();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	float buttonWidth = profile.GetUIParamScaled(78.0f, 2.5f);

	if (Viewer::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		profile.CropAnchor				= 4;
		profile.FillColour				= tColouri::black;
		profile.ResizeAspectRatio		= int(tAspectRatio::Screen_16_9) - 1;
		profile.ResizeAspectUserNum		= 16;
		profile.ResizeAspectUserDen		= 9;
		profile.ResizeAspectMode		= 0;
	}

	ImGui::SameLine();
	if (Viewer::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Resize", tVector2(buttonWidth, 0.0f)))
	{
		int dstH = srcH;
		int dstW = srcW;
		float srcAspect = float(srcW)/float(srcH);
		float dstAspect = profile.GetResizeAspectRatioFloat();
		if (profile.ResizeAspectMode == 0)
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
