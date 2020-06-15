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

#include <System/tTime.h>
#include <Math/tVector2.h>
#include "imgui.h"
#include "ContentView.h"
#include "TacentView.h"
#include "Image.h"
using namespace tMath;


void Viewer::ShowContentViewDialog(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar;
	tVector2 windowPos = GetDialogOrigin(1);
	
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(tVector2(640, 374), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Content View", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	ImGuiWindowFlags thumbWindowFlags = 0;
	ImGui::BeginChild("Thumbnails", tVector2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight()-61.0f), false, thumbWindowFlags);
	
	ImGuiStyle& style = ImGui::GetStyle();
	float visibleW = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

	float minSpacing = 4.0f;
	float numPerRowF = ImGui::GetWindowContentRegionMax().x / (Config.ThumbnailWidth + minSpacing);
	int numPerRow = tMath::tClampMin(int(numPerRowF), 1);
	float extra = ImGui::GetWindowContentRegionMax().x - (float(numPerRow) * (Config.ThumbnailWidth + minSpacing));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(minSpacing + extra/float(numPerRow), minSpacing));
	tVector2 thumbButtonSize(Config.ThumbnailWidth, Config.ThumbnailWidth*9.0f/16.0f); // 64 36, 32 18,
	int thumbNum = 0;
	for (Image* i = Images.First(); i; i = i->Next(), thumbNum++)
	{
		tVector2 cursor = ImGui::GetCursorPos();
		if ((thumbNum % numPerRow) == 0)
			ImGui::SetCursorPos(tVector2(0.5f*extra/float(numPerRow), cursor.y));

		ImGui::PushID(thumbNum);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2::zero);
		bool isCurr = (i == CurrImage);

		// Unlike other widgets, BeginChild ALWAYS needs a corresponding EndChild, even if it's invisible.
		bool visible = ImGui::BeginChild("ThumbItem", thumbButtonSize+tVector2(0.0, 32.0f), false, ImGuiWindowFlags_NoDecoration);
		if (visible)
		{
			i->RequestThumbnail();
			uint64 thumbnailTexID = i->BindThumbnail();
			if (!thumbnailTexID)
				thumbnailTexID = DefaultThumbnailImage.Bind();
			if
			(
				thumbnailTexID &&
				ImGui::ImageButton(ImTextureID(thumbnailTexID), thumbButtonSize, tVector2(0,1), tVector2(1,0), 0,
				ColourBG, ColourEnabledTint)
			)
			{
				CurrImage = i;
				LoadCurrImage();
			}

			tString filename = tSystem::tGetFileName(i->Filename);
			ImGui::Text(filename.Chars());

			tString ttStr;
			tsPrintf(ttStr, "%s\n%s\n%'d Bytes", 
				filename.Chars(),
				tSystem::tConvertTimeToString(tSystem::tConvertTimeToLocal(i->FileModTime)).Chars(), i->FileSizeB);
			ShowToolTip(ttStr.Chars());

			// We use a separator to indicate the current item.
			if (isCurr)
				ImGui::Separator(2.0f);
		}
		else
		{
			// We need to keep calling bind even if the image is not visible. It frees up the worker threads.
			if (i->IsThumbnailWorkerActive())
				i->BindThumbnail();
			else
				i->UnrequestThumbnail();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();

		if ((thumbNum+1) % numPerRow)
			ImGui::SameLine();

		ImGui::PopID();
	}
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGuiWindowFlags viewOptionsWindowFlags = ImGuiWindowFlags_NoScrollbar;
	ImGui::BeginChild("ViewOptions", tVector2(ImGui::GetWindowContentRegionWidth(), 40), false, viewOptionsWindowFlags);
	ImGui::SetCursorPos(tVector2(0.0f, 3.0f));

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Size", &Config.ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth), "%.0f");
	ImGui::SameLine();
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(100);
	const char* sortItems[] = { "Alphabetical", "Date", "Size", "Type" };
	if (ImGui::Combo("Sort", &Config.SortKey, sortItems, tNumElements(sortItems)))
		SortImages(Settings::SortKeyEnum(Config.SortKey), Config.SortAscending);
	ImGui::SameLine();
	if (ImGui::Checkbox("Ascending", &Config.SortAscending))
		SortImages(Settings::SortKeyEnum(Config.SortKey), Config.SortAscending);

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::End();
}
