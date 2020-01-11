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
	ImGuiWindowFlags windowFlags = 0; // ImGuiWindowFlags_AlwaysAutoResize;
	ImVec2 windowPos = ImVec2(PopupMargin*4.0f, TopUIHeight + PopupMargin*4.0f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Content View", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	int cols = 0;
	for (TacitImage* i = Images.First(); i; i = i->Next())
	{
//		TacitImage* i = CurrImage;
		i->RequestThumbnail();
		uint64 thumbnailTexID = i->BindThumbnail();
		if (!thumbnailTexID)
			thumbnailTexID = DefaultThumbnailImage.Bind();

		if
		(
			thumbnailTexID &&
//			ImGui::ImageButton(ImTextureID(thumbnailTexID), ImVec2(240,135), ImVec2(0,1), ImVec2(1,0), -1,
//			ImGui::ImageButton(ImTextureID(thumbnailTexID), ImVec2(200,100), ImVec2(0,1), ImVec2(1,0), 0)
			ImGui::ImageButton(ImTextureID(thumbnailTexID), ImVec2(128,72), ImVec2(0,1), ImVec2(1,0), 0,
			ColourBG, ImVec4(1.00f, 1.00f, 1.00f, 1.00f))
		)
		{
			// WIP. Goto image.
		}

		if (++cols % 5)
			ImGui::SameLine();
	}

	ImGui::End();
}
