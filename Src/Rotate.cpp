// Rotate.cpp
//
// Modal for rotating an image.
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
#include "Rotate.h"
#include "Image.h"
#include "TacentView.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;


void Viewer::DoRotateImageModal(bool justOpened)
{
	if (justOpened)
		Viewer::ResetPan();

	ImGui::InputFloat("Edit Angle", &RotateAnglePreview, 0.01f, 0.1f, "%.3f");
	ImGui::SliderFloat("Angle", &RotateAnglePreview, -180.0f, 180.0f);
	ImGui::DragFloat("Fine Tune Drag", &RotateAnglePreview, 0.01f);
	ImGui::NewLine();

	ImGui::Combo("Up Filter", &Config.ResampleFilterRotateUp, tResampleFilterNames, tNumElements(tResampleFilterNames), tNumElements(tResampleFilterNames));
	ImGui::SameLine();
	ShowHelpMark
	(
		"Filtering method used during up-sampling.\n"
		"If set to None no resampling, preserves colours,\n"
		"nearest neighbour, fast, for pixel art."
	);

	if (Config.ResampleFilterRotateUp != int(tResampleFilter(tResampleFilter::None)))
	{
		ImGui::Combo("Down Filter", &Config.ResampleFilterRotateDown, tResampleFilterNames, tNumElements(tResampleFilterNames), tNumElements(tResampleFilterNames));
		ImGui::SameLine();
		ShowHelpMark
		(
			"Filtering method used during down-sampling.\n"
			"Box works well. If set to None uses an an alternate\n"
			"down-sample that produces sharper images."
		);
	}

	static const char* modeNames[] = { "Fill", "Crop", "Crop Resize" };
	ImGui::Combo("Mode", &Config.RotateMode, modeNames, tNumElements(modeNames), tNumElements(modeNames));
	ImGui::SameLine();
	ShowHelpMark
	(
		"Fill will result in a slightly larger image that uses the fill colour where necessary.\n"
		"Crop will result in a slightly smaller image after rotation but it will be full.\n"
		"Crop Resize is the same as Crop and then resamples to the original size with upfilter.\n"
		"For both crop modes, the aspect ratio is preserved for rotations that are within +-45\n"
		"degrees on either side. If the rotation is mostly vertical, the reciprical aspect is used."
	);

	if (Config.RotateMode == int(Settings::RotMode::Fill))
	{
		tColourf floatCol(Config.CropFillColour);
		ImGui::ColorEdit4("Fill Colour", floatCol.E, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf);
		Config.CropFillColour.Set(floatCol);
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset", tVector2(100, 0)))
		RotateAnglePreview = 0.0f;

	if (ImGui::Button("Cancel", tVector2(100, 0)))
	{
		RotateAnglePreview = 0.0f;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Rotate", tVector2(100, 0)))
	{
		tPicture* picture = CurrImage->GetCurrentPic(); tAssert(picture);
		int origW = picture->GetWidth();
		int origH = picture->GetHeight();

		CurrImage->Unbind();
		CurrImage->Rotate
		(
			tDegToRad(RotateAnglePreview), Config.CropFillColour,
			tResampleFilter(Config.ResampleFilterRotateUp),
			tResampleFilter(Config.ResampleFilterRotateDown)
		);

		if ((Config.RotateMode == int(Settings::RotMode::Crop)) || (Config.RotateMode == int(Settings::RotMode::CropResize)))
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

		if (Config.RotateMode == int(Settings::RotMode::CropResize))
		{
			// The crop is done. Now resample.
			tResampleFilter filter = (Config.ResampleFilterRotateUp != int(tResampleFilter::None)) ? tResampleFilter(Config.ResampleFilterRotateUp) : tResampleFilter::Nearest;
			CurrImage->Resample(origW, origH, filter, tResampleEdgeMode::Clamp);
		}

		CurrImage->Bind();
		RotateAnglePreview = 0.0f;
		Viewer::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


