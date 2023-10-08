// ThumbnailView.cpp
//
// Dialog that displays folder contents as thumbnails.
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

#include <System/tTime.h>
#include <Math/tVector2.h>
#include "imgui.h"
#include "ThumbnailView.h"
#include "TacentView.h"
#include "GuiUtil.h"
#include "Image.h"
using namespace tMath;
using namespace Gutil;


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


void Viewer::DoSortParameters(bool singleLine)
{
	Config::ProfileData& profile = Config::GetProfileData();
	float sortComboWidth = Gutil::GetUIParamScaled(120.0f, 2.5f);

	const char* sortItems[] =
	{
		"Name", "ModTime", "Size", "Type", "Area", "Width", "Height", "Latitude*",
		"Longitude*", "Altitude*", "Roll*", "Pitch*", "Yaw*", "Speed*", "Shutter Speed*",
		"Exposure Time*", "F-Stop*", "ISO*", "Aperture*", "Orientation*", "Brightness*",
		"Flash*", "Focal Length*", "Time Taken*", "Time Modified*", "Camera Make*", "Description*",
		"Shuffle"
	};

	tStaticAssert(tNumElements(sortItems) == int(Config::ProfileData::SortKeyEnum::NumKeys));
	tString label = singleLine ? "##Sort" : "Sort";
	ImGui::SetNextItemWidth(sortComboWidth);
	if (ImGui::Combo(label.Chr(), &profile.SortKey, sortItems, tNumElements(sortItems), tNumElements(sortItems)/2))
		SortImages(profile.GetSortKey(), profile.SortAscending);
	ShowToolTip("Specifies what property to sort by. An asterisk (*) means\nthe property is stored in image meta-data and may not be\npresent in all images. Shuffle means random order.");

	if (profile.GetSortKey() == Config::ProfileData::SortKeyEnum::Shuffle)
	{
		ImGui::SameLine();
		if (ImGui::Button(" Reshuffle "))
		{
			for (Image* i = Images.First(); i; i = i->Next())
				i->RegenerateShuffleValue();
			SortImages(Config::ProfileData::SortKeyEnum::Shuffle, profile.SortAscending);
		}
	}

	if (singleLine)
		ImGui::SameLine();
	if (ImGui::Checkbox("Ascending", &profile.SortAscending))
		SortImages(profile.GetSortKey(), profile.SortAscending);
}


void Viewer::ShowThumbnailViewDialog(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar;
	tVector2 windowPos = GetDialogOrigin(DialogID::ThumbnailView);

	Config::ProfileData& profile = Config::GetProfileData();
	tVector2 initialSize = Gutil::GetUIParamScaled(tVector2(586.0f, 480.0f), 2.5f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Thumbnail View", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	float barHeight				= Gutil::GetUIParamScaled(4.0f, 2.5f);
	float viewOptionsHeight		= Gutil::GetUIParamScaled(66.0f, 2.5f) + barHeight;
	float optionsHeight			= Gutil::GetUIParamScaled(20.0f, 2.5f) + barHeight;
	float progressTextOffset	= Gutil::GetUIParamScaled(460.0f, 2.5f);
	float thumbItemInfoHeight	= Gutil::GetUIParamScaled(32.0f, 2.5f);
	float minSpacing			= Gutil::GetUIParamScaled(4.0f, 2.5f);

	ImGuiWindowFlags thumbWindowFlags = 0;
	ImGui::BeginChild("Thumbnails", tVector2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight()-viewOptionsHeight), false, thumbWindowFlags);

	ImGuiStyle& style = ImGui::GetStyle();
	float visibleW = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

	float numPerRowF = ImGui::GetWindowContentRegionMax().x / (profile.ThumbnailWidth + minSpacing);
	int numPerRow = tMath::tClampMin(int(numPerRowF), 1);
	float extra = ImGui::GetWindowContentRegionMax().x - (float(numPerRow) * (profile.ThumbnailWidth + minSpacing));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(minSpacing + extra/float(numPerRow), minSpacing));
	tVector2 thumbButtonSize(profile.ThumbnailWidth, profile.ThumbnailWidth*9.0f/16.0f);
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
		bool visible = ImGui::BeginChild("ThumbItem", thumbButtonSize+tVector2(0.0f, thumbItemInfoHeight), false, ImGuiWindowFlags_NoDecoration);
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

	float viewOptionsMargin = Gutil::GetUIParamScaled(10.0f, 2.5f);
	float optOffset = (ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - optionsHeight) / 2.0f;

	ImGui::SetCursorPos(tVector2(viewOptionsMargin, ImGui::GetCursorPosY() + optOffset));
	ImGui::BeginChild("ViewOptions", tVector2(ImGui::GetWindowWidth()-viewOptionsMargin*2.0f, optionsHeight), false, viewOptionsWindowFlags);

	float sizeSliderWidth = Gutil::GetUIParamScaled(200.0f, 2.5f);
	ImGui::PushItemWidth(sizeSliderWidth);
	ImGui::SliderFloat("Size", &profile.ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth), "%.0f");
	tiClampMin(profile.ThumbnailWidth, float(Image::ThumbMinDispWidth));
	ImGui::SameLine();
	ImGui::PopItemWidth();

	DoSortParameters(true);

	// If we are sorting by a thumbnail cached key, resort if necessary.
	Config::ProfileData::SortKeyEnum sortKey = profile.GetSortKey();
	if (Config::ProfileData::IsCachedSortKey(sortKey))
	{
		if (numThumbsWhenSorted != numGeneratedThumbs)
		{
			SortImages(sortKey, profile.SortAscending);
			numThumbsWhenSorted = numGeneratedThumbs;
		}
	}

	if (numGeneratedThumbs < Images.GetNumItems())
	{
		tString progText;
		tsPrintf(progText, "%d/%d", numGeneratedThumbs, Images.GetNumItems());
		tVector2 textSize = ImGui::CalcTextSize(progText.Chr());
		float rightx = ImGui::GetWindowContentRegionMax().x - 8.0f;
		float textx = rightx - textSize.x;

		if (textx > progressTextOffset)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(textx);
			ImGui::Text(progText.Chr());
		}
		ImGui::ProgressBar(float(numGeneratedThumbs)/float(Images.GetNumItems()), tVector2(rightx, barHeight), "");
	}

	ImGui::EndChild();
	ImGui::End();
}
