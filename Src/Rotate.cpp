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
		Viewer::ZoomDownscaleOnly();

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

	static const char* modeNames[] = { "Crop", "Fill" };
	ImGui::Combo("Mode", &Config.RotateMode, modeNames, tNumElements(modeNames), tNumElements(modeNames));
	ImGui::SameLine();
	ShowHelpMark
	(
		"Crop will result in a slightly smaller image after rotation but it will be full.\n"
		"Fill will result in a slightly larger image that uses the fill colour where necessary."
	);

	if (Config.RotateMode == 1)
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
		CurrImage->Unbind();
		CurrImage->Rotate
		(
			tDegToRad(RotateAnglePreview), Config.CropFillColour,
			tResampleFilter(Config.ResampleFilterRotateUp),
			tResampleFilter(Config.ResampleFilterRotateDown)
		);
		CurrImage->Bind();
		RotateAnglePreview = 0.0f;
		Viewer::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


