// Rotate.cpp
//
// Modal for rotating an image.
//
// Copyright (c) 2020 Tristan Grimmer.
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
	if (ImGui::Button("Reset", tVector2(100, 0)))
		RotateAnglePreview = 0.0f;

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
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
		CurrImage->Rotate(tDegToRad(RotateAnglePreview));
		CurrImage->Bind();
		RotateAnglePreview = 0.0f;
		Viewer::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


