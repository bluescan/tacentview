// Rotate.cpp
//
// Modals for rotating an image.
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
#include "Rotate.h"
#include "Image.h"
#include "TacentView.h"
#include "GuiUtil.h"
#include "Config.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;
using namespace Gutil;
namespace Viewer { extern void DoFillColourInterface(const char* = nullptr, bool = false); }


void Viewer::DoRotateImageModal(bool rotateImagePressed)
{
	if (rotateImagePressed)
		ImGui::OpenPopup("Rotate Image");

	bool isOpenRotateImage = true;
	if (!ImGui::BeginPopupModal("Rotate Image", &isOpenRotateImage, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
	{
		RotateAnglePreview = 0.0f;
		return;
	}

	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);

	if (rotateImagePressed)
		Viewer::ResetPan();

	ImGui::InputFloat("Edit Angle", &RotateAnglePreview, 0.01f, 0.1f, "%.3f");
	ImGui::SliderFloat("Angle", &RotateAnglePreview, -180.0f, 180.0f);
	ImGui::DragFloat("Fine Tune Drag", &RotateAnglePreview, 0.01f);
	ImGui::NewLine();

	ImGui::Combo("Up Filter", &profile.ResampleFilterRotateUp, tResampleFilterNames, tNumElements(tResampleFilterNames), tNumElements(tResampleFilterNames));
	ImGui::SameLine();
	ShowHelpMark
	(
		"Filtering method used during up-sampling.\n"
		"If set to None no resampling, preserves colours,\n"
		"nearest neighbour, fast, for pixel art."
	);

	if (profile.ResampleFilterRotateUp != int(tResampleFilter(tResampleFilter::None)))
	{
		ImGui::Combo("Down Filter", &profile.ResampleFilterRotateDown, tResampleFilterNames, tNumElements(tResampleFilterNames), tNumElements(tResampleFilterNames));
		ImGui::SameLine();
		ShowHelpMark
		(
			"Filtering method used during down-sampling.\n"
			"Box works well. If set to None uses an an alternate\n"
			"down-sample that produces sharper images."
		);
	}

	static const char* modeNames[] = { "Fill", "Crop", "Crop Resize" };
	ImGui::Combo("Mode", &profile.RotateMode, modeNames, tNumElements(modeNames), tNumElements(modeNames));
	ImGui::SameLine();
	ShowHelpMark
	(
		"Fill will result in a slightly larger image that uses the fill colour where necessary.\n"
		"Crop will result in a slightly smaller image after rotation but it will be full.\n"
		"Crop Resize is the same as Crop and then resamples to the original size with upfilter.\n"
		"For both crop modes, the aspect ratio is preserved for rotations that are within +-45\n"
		"degrees on either side. If the rotation is mostly vertical, the reciprical aspect is used."
	);

	if (profile.GetRotateMode() == Config::ProfileData::RotateModeEnum::Fill)
		DoFillColourInterface();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (Gutil::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		RotateAnglePreview = 0.0f;
		profile.SetRotateMode(Config::ProfileData::RotateModeEnum::Fill);
	}

	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
	{
		RotateAnglePreview = 0.0f;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	float rotOffset = Gutil::GetUIParamScaled(254.0f, 2.5f);
	ImGui::SetCursorPosX(rotOffset);
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();

	if (Gutil::Button("Rotate", tVector2(buttonWidth, 0.0f)))
	{
		tPicture* picture = CurrImage->GetCurrentPic();
		if (!picture)
		{
			RotateAnglePreview = 0.0f;
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return;
		}
		int origW = picture->GetWidth();
		int origH = picture->GetHeight();

		CurrImage->Unbind();
		CurrImage->Rotate
		(
			tDegToRad(RotateAnglePreview), profile.FillColour,
			tResampleFilter(profile.ResampleFilterRotateUp),
			tResampleFilter(profile.ResampleFilterRotateDown)
		);

		if ((profile.GetRotateMode() == Config::ProfileData::RotateModeEnum::Crop) || (profile.GetRotateMode() == Config::ProfileData::RotateModeEnum::CropResize))
		{
			// If one of the crop modes is selected we need to crop the edges. Since rectangles are made of lines and there
			// is symmetry and we can compute the reduced size by subtracting the original size from the rotated size.
			int rotW = picture->GetWidth();
			int rotH = picture->GetHeight();
			bool aspectFlip = ((origW > origH) && (rotW < rotH)) || ((origW < origH) && (rotW > rotH));
			if (aspectFlip)
				tSwap(origW, origH);

			int dx = rotW - origW;
			int dy = rotH - origH;
			int newW = origW - dx;
			int newH = origH - dy;

			if (dx > origW/2)
			{
				newW = origW - origW/2;
				newH = (newW*origH)/origW;
			}
			else if (dy > origH/2)
			{
				newH = origH - origH/2;
				newW = (newH*origW)/origH;
			}

			// The above code has been tested with a 1x1 input and (newH,newW) result correcty as (1,1). 
			CurrImage->Crop(newW, newH, tPicture::Anchor::MiddleMiddle);
		}

		if (profile.GetRotateMode() == Config::ProfileData::RotateModeEnum::CropResize)
		{
			// The crop is done. Now resample.
			tResampleFilter filter = (profile.ResampleFilterRotateUp != int(tResampleFilter::None)) ? tResampleFilter(profile.ResampleFilterRotateUp) : tResampleFilter::Nearest;
			CurrImage->Resample(origW, origH, filter, tResampleEdgeMode::Clamp);
		}

		CurrImage->Bind();
		RotateAnglePreview = 0.0f;
		Gutil::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


void Viewer::DoLosslessTransformModal(LosslessTransformMode mode)
{
	static const char* transName = nullptr;
	static LosslessTransformMode currMode = LosslessTransformMode::None;
	static bool isPerfect = false;

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);

	if (mode != LosslessTransformMode::None)
	{
		currMode = mode;
		switch (mode)
		{
			case LosslessTransformMode::Rot90ACW:	transName = "Anti-clockwise 90° Rotation";	break;
			case LosslessTransformMode::Rot90CW:	transName = "Clockwise 90° Rotation";		break;
			case LosslessTransformMode::FlipH:		transName = "Horizontal Flip";				break;
			case LosslessTransformMode::FlipV:		transName = "Vertical Flip";				break;
		}
		if (CurrImage->Filetype == tSystem::tFileType::JPG)
		{
			// @todo This isn't ideal as we are loading the jpg only to find out if a perfect
			// lossless transformation is possible. Later the user may press the lossless button
			// in which case we need to load the jpg again.
			isPerfect = false;
			tImage::tImageJPG jpg;
			tImage::tImageJPG::LoadParams params;
			params.Flags = tImage::tImageJPG::LoadFlag_NoDecompress;
			jpg.Load(CurrImage->Filename, params);
			if (jpg.IsValid())
			{
				switch (mode)
				{
					case LosslessTransformMode::Rot90ACW:	isPerfect = jpg.CanDoPerfectLosslessTransform(tImage::tImageJPG::Transform::Rotate90ACW);	break;
					case LosslessTransformMode::Rot90CW:	isPerfect = jpg.CanDoPerfectLosslessTransform(tImage::tImageJPG::Transform::Rotate90CW);	break;
					case LosslessTransformMode::FlipH:		isPerfect = jpg.CanDoPerfectLosslessTransform(tImage::tImageJPG::Transform::FlipH);			break;
					case LosslessTransformMode::FlipV:		isPerfect = jpg.CanDoPerfectLosslessTransform(tImage::tImageJPG::Transform::FlipV);			break;
				}
			}
		}
		else
		{
			isPerfect = true;
		}
		ImGui::OpenPopup("Lossless Transform");
	}

	// The unused isOpenRen bool is just so we get a close button in ImGui.
	bool isOpenRen = true;
	if (!ImGui::BeginPopupModal("Lossless Transform", &isOpenRen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	ImGui::Text
	(
		"Tacent View supports lossless %ss.\n"
		"\n"
		"If you select 'Normal' the transform will be performed\n"
		"but if you save to a lossy format it will need to re-compress.\n"
		"\n"
		"If you press 'Lossless' the transform will be applied directly\n"
		"to the file without decompressing. The image will be saved\n"
		"and reloaded. There is no undo but also no loss of quality.\n\n",
		transName
	);

	if (!isPerfect)
	{
		ImGui::Text
		(
			"Note: For this file a perfect (area-preserving) transformation\n"
			"is not possible because the width or height is not a multiple of\n"
			"the JPeg MCU block-size. If you press 'Lossless' you will still\n"
			"get a lossless image afterwards, but the right or bottom edge\n"
			"will be slightly trimmed.\n\n"
		);
	}

	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();

	float normLossOffset = Gutil::GetUIParamScaled(180.0f, 2.5f);
	ImGui::SetCursorPosX(normLossOffset);
	if (Gutil::Button("Normal", tVector2(buttonWidth, 0.0f)))
	{
		CurrImage->Unbind();
		switch (currMode)
		{
			case LosslessTransformMode::Rot90ACW:	CurrImage->Rotate90(true);	break;
			case LosslessTransformMode::Rot90CW:	CurrImage->Rotate90(false);	break;
			case LosslessTransformMode::FlipH:		CurrImage->Flip(true);		break;
			case LosslessTransformMode::FlipV:		CurrImage->Flip(false);		break;
		}
		CurrImage->Bind();
		SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("Lossless", tVector2(buttonWidth, 0.0f)))
	{
		bool fileSaved = false;
		switch (CurrImage->Filetype)
		{
			case tSystem::tFileType::JPG:
			{
				tImage::tImageJPG jpg;
				tImage::tImageJPG::LoadParams params;
				params.Flags = tImage::tImageJPG::LoadFlag_NoDecompress;
				jpg.Load(CurrImage->Filename, params);
				if (!jpg.IsValid())
					break;
				switch (currMode)
				{
					case LosslessTransformMode::Rot90ACW:	jpg.LosslessTransform(tImage::tImageJPG::Transform::Rotate90ACW);	break;
					case LosslessTransformMode::Rot90CW:	jpg.LosslessTransform(tImage::tImageJPG::Transform::Rotate90CW);	break;
					case LosslessTransformMode::FlipH:		jpg.LosslessTransform(tImage::tImageJPG::Transform::FlipH);			break;
					case LosslessTransformMode::FlipV:		jpg.LosslessTransform(tImage::tImageJPG::Transform::FlipV);			break;
				}
				fileSaved = jpg.Save(CurrImage->Filename);
				break;
			}
		}
		if (fileSaved)
		{
			CurrImage->Unbind();
			CurrImage->Unload(true);
			CurrImage->Load();
			CurrImage->Bind();
			SetWindowTitle();
		}
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
