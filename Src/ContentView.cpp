// ContentView.cpp
//
// Dialog that displays folder contents as thumbnails.
//
// Copyright (c) 2020, 2021, 2022 Tristan Grimmer.
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


namespace Viewer
{
	tString MakeImageTooltipString(Image*, const tString& filename);
}


tString Viewer::MakeImageTooltipString(Viewer::Image* image, const tString& filename)
{
	tString tooltip;
	if (!image)
		return tooltip;

	if (image->Cached_PrimaryWidth && image->Cached_PrimaryHeight)
		tsPrintf
		(
			tooltip, "%s\n%s\n%'d Bytes\nW:%'d\nH:%'d\nArea:%'d",
			filename.Chr(),
			tSystem::tConvertTimeToString(tSystem::tConvertTimeToLocal(image->FileModTime)).Chr(),
			image->FileSizeB,
			image->Cached_PrimaryWidth,
			image->Cached_PrimaryHeight,
			image->Cached_PrimaryArea
		);
	else
		tsPrintf
		(
			tooltip, "%s\n%s\n%'d Bytes",
			filename.Chr(),
			tSystem::tConvertTimeToString(tSystem::tConvertTimeToLocal(image->FileModTime)).Chr(),
			image->FileSizeB
		);


	return tooltip;
}


void Viewer::ShowContentViewDialog(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar;
	tVector2 windowPos = GetDialogOrigin(0);
	
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
	float numPerRowF = ImGui::GetWindowContentRegionMax().x / (Config::Current->ThumbnailWidth + minSpacing);
	int numPerRow = tMath::tClampMin(int(numPerRowF), 1);
	float extra = ImGui::GetWindowContentRegionMax().x - (float(numPerRow) * (Config::Current->ThumbnailWidth + minSpacing));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(minSpacing + extra/float(numPerRow), minSpacing));
	tVector2 thumbButtonSize(Config::Current->ThumbnailWidth, Config::Current->ThumbnailWidth*9.0f/16.0f); // 64 36, 32 18,
	int thumbNum = 0;
	int numGeneratedThumbs = 0;
	static int numThumbsWhenSorted = 0;
	for (Image* i = Images.First(); i; i = i->Next(), thumbNum++)
	{
		tVector2 cursor = ImGui::GetCursorPos();
		if ((thumbNum % numPerRow) == 0)
			ImGui::SetCursorPos(tVector2(0.5f*extra/float(numPerRow), cursor.y));

		ImGui::PushID(thumbNum);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2::zero);
		bool isCurr = (i == CurrImage);

		// It's ok to call bind even if a request has not been made yet. Takes no time.
		// Calling bind also frees up the worker threads when requests are fulfilled.
		uint64 thumbnailTexID = i->BindThumbnail();
		if (thumbnailTexID)
			numGeneratedThumbs++;

		// Unlike other widgets, BeginChild ALWAYS needs a corresponding EndChild, even if it's invisible.
		bool visible = ImGui::BeginChild("ThumbItem", thumbButtonSize+tVector2(0.0f, 32.0f), false, ImGuiWindowFlags_NoDecoration);
		int maxNonVisibleThumbThreads = 3;
		if (visible)
		{
			// Give priority to creating thumbnails for visible widgets. Later on, if no threads are active
			// we request non-visible ones.
			i->RequestThumbnail();
			if (!thumbnailTexID)
				thumbnailTexID = Image_DefaultThumbnail.Bind();
			ImGui::PushStyleColor(ImGuiCol_Button, ColourClear);
			if
			(
				thumbnailTexID &&
				ImGui::ImageButton(ImTextureID(thumbnailTexID), thumbButtonSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 0,
				ColourBG, ColourEnabledTint)
			)
			{
				CurrImage = i;
				LoadCurrImage();
			}
			ImGui::PopStyleColor();

			tString filename = tSystem::tGetFileName(i->Filename);
			ImGui::Text(filename.Chr());
			
			tString ttStr = Viewer::MakeImageTooltipString(i, filename);
			ShowToolTip(ttStr.Chr());

			// We use a separator to indicate the current item.
			if (isCurr)
				ImGui::Separator(2.0f);
		}

		// Not visible. If we're not doing much, request non-visible thumbnail generation. For the
		// offscreen ones we only do maxNonVisibleThumbThreads at a time.
		else if (Image::GetThumbnailNumThreadsRunning() < maxNonVisibleThumbThreads)
			i->RequestThumbnail();

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
	ImGui::SliderFloat("Size", &Config::Current->ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth), "%.0f");
	ImGui::SameLine();
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(100);
	const char* sortItems[] = { "Name", "Date", "Size", "Type", "Area", "Width", "Height" };
	if (ImGui::Combo("Sort", &Config::Current->SortKey, sortItems, tNumElements(sortItems)))
		SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);
	ImGui::SameLine();
	if (ImGui::Checkbox("Ascending", &Config::Current->SortAscending))
		SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);

	// If we are sorting by a thumbnail cached key, resort if necessary.
	Config::ProfileSettings::SortKeyEnum sortKey = Config::Current->GetSortKey();
	if ((sortKey == Config::ProfileSettings::SortKeyEnum::ImageArea) || (sortKey == Config::ProfileSettings::SortKeyEnum::ImageWidth) || (sortKey == Config::ProfileSettings::SortKeyEnum::ImageHeight))
	{
		if (numThumbsWhenSorted != numGeneratedThumbs)
		{
			SortImages(sortKey, Config::Current->SortAscending);
			numThumbsWhenSorted = numGeneratedThumbs;
		}
	}

	if (numGeneratedThumbs < Images.GetNumItems())
	{
		tString progText;
		tsPrintf(progText, "%d/%d", numGeneratedThumbs, Images.GetNumItems());
		tVector2 textSize = ImGui::CalcTextSize(progText.Chr());
		float rightx = ImGui::GetWindowContentRegionMax().x - 4.0f;
		float textx = rightx - textSize.x;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY()+1.0f);
		if (textx > 470)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(textx);
			ImGui::Text(progText.Chr());
			ImGui::ProgressBar(float(numGeneratedThumbs)/float(Images.GetNumItems()), tVector2(rightx, 0));
		}
	}

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::End();
}
