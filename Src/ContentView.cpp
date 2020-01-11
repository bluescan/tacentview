// ContentView.cpp
//
// Dialog that displays folder contents as thumbnails.
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
#include "ContentView.h"
#include "TacitTexView.h"
#include "TacitImage.h"


void TexView::ShowContentViewDialog(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar;
	ImVec2 windowPos = GetDialogOrigin(0);
	
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(640, 366), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Content View", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	ImGuiWindowFlags thumbWindowFlags = 0;
	ImGui::BeginChild("Thumbnails", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight()-61.0f), false, thumbWindowFlags);
	
	ImGuiStyle& style = ImGui::GetStyle();
	float visibleW = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	static float thumbWidth = 72.0f;

	float minSpacing = 4.0f;
	float numPerRowF = ImGui::GetWindowContentRegionMax().x / (thumbWidth + minSpacing);
	int numPerRow = tMath::tGetClampMin(int(numPerRowF), 1);
	float extra = ImGui::GetWindowContentRegionMax().x - (float(numPerRow) * (thumbWidth + minSpacing));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(minSpacing + extra/float(numPerRow), minSpacing));
	ImVec2 thumbButtonSize(thumbWidth, thumbWidth*9.0f/16.0f); // 64 36, 32 18,
	int thumbNum = 0;
	for (TacitImage* i = Images.First(); i; i = i->Next(), thumbNum++)
	{
		ImVec2 cursor = ImGui::GetCursorPos();
		if ((thumbNum % numPerRow) == 0)
			ImGui::SetCursorPos(ImVec2(0.5f*extra/float(numPerRow), cursor.y));

		i->RequestThumbnail();
		uint64 thumbnailTexID = i->BindThumbnail();
		if (!thumbnailTexID)
			thumbnailTexID = DefaultThumbnailImage.Bind();

		ImGui::PushID(thumbNum);

		if
		(
			thumbnailTexID &&
			ImGui::ImageButton(ImTextureID(thumbnailTexID), thumbButtonSize, ImVec2(0,1), ImVec2(1,0), 0,
			ColourBG, ImVec4(1.00f, 1.00f, 1.00f, 1.00f))
		)
		{
			CurrImage = i;
			LoadCurrImage();
		}
		tString filename = tSystem::tGetFileName(i->Filename);
		ShowToolTip(filename.ConstText());

		if ((thumbNum+1) % numPerRow)
			ImGui::SameLine();

		ImGui::PopID();
	}
	ImGui::PopStyleVar(1);
	ImGui::EndChild();

	ImGuiWindowFlags viewOptionsWindowFlags = ImGuiWindowFlags_NoScrollbar;
	ImGui::BeginChild("ViewOptions", ImVec2(ImGui::GetWindowContentRegionWidth(), 40), false, viewOptionsWindowFlags);
	ImGui::SetCursorPos(ImVec2(0.0f, 3.0f));
	ImGui::SliderFloat("Thumbnail Size", &thumbWidth, 64.0f, 240.0f, "");
	ImGui::EndChild();

	ImGui::End();
}
