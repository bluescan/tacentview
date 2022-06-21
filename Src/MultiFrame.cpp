// MultiFrame.cpp
//
// Dialog that generates multiframe images from all image files in the directory.
//
// Copyright (c) 2021, 2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tVector2.h>
#include "imgui.h"
#include "MultiFrame.h"
#include "OpenSaveDialogs.h"
#include "TacentView.h"
#include "Image.h"
#include "Config.h"
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;


namespace Viewer
{
	void SaveMultiFrameTo(const tString& outFile, int finalWidth, int finalHeight);
	void ComputeMaxWidthHeight(int& outWidth, int& outHeight);
	bool AllDimensionsMatch(int width, int height);
}


void Viewer::ComputeMaxWidthHeight(int& outWidth, int& outHeight)
{
	outWidth = 0; outHeight = 0;
	for (Image* img = Images.First(); img; img = img->Next())
	{
		if (!img->IsLoaded())
			img->Load();

		if (!img->IsLoaded())
			continue;

		tPicture* currPic = img->GetCurrentPic();
		if (currPic)
		{
			if (currPic->GetWidth() > outWidth)		outWidth = currPic->GetWidth();
			if (currPic->GetHeight() > outHeight)	outHeight = currPic->GetHeight();
		}
	}
}


bool Viewer::AllDimensionsMatch(int width, int height)
{
	for (Image* img = Images.First(); img; img = img->Next())
	{
		if (!img->IsLoaded())
			img->Load();

		if (!img->IsLoaded())
			continue;

		tPicture* currPic = img->GetCurrentPic();
		if (currPic)
		{
			if ((currPic->GetWidth() != width) || (currPic->GetHeight() != height))
				return false;
		}
	}

	return true;
}


void Viewer::DoMultiFrameModal(bool saveMultiFramePressed)
{
	if (saveMultiFramePressed)
		ImGui::OpenPopup("Multi Frame");

	// The unused isOpenMultiFrame bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenMultiFrame = true;
	if (!ImGui::BeginPopupModal("Multi Frame", &isOpenMultiFrame, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	static int outWidth = 256;
	static int outHeight = 256;
	static char lo[32];
	static char hi[32];

	// If just opened, loop through all the images and choose the largest width and height.
	if (saveMultiFramePressed)
		ComputeMaxWidthHeight(outWidth, outHeight);

	ImGui::InputInt("Width", &outWidth);
	tiClampMin(outWidth, 4);
	int loP2W = tNextLowerPower2(outWidth);		tiClampMin(loP2W, 4);	tsPrintf(lo, "w%d", loP2W);
	ImGui::SameLine(); if (ImGui::Button(lo)) outWidth = loP2W;
	int hiP2W = tNextHigherPower2(outWidth);							tsPrintf(hi, "w%d", hiP2W);
	ImGui::SameLine(); if (ImGui::Button(hi)) outWidth = hiP2W;
	ImGui::SameLine(); ShowHelpMark("Output width in pixels.");

	ImGui::InputInt("Height", &outHeight);
	tiClampMin(outHeight, 4);
	int loP2H = tNextLowerPower2(outHeight);	tiClampMin(loP2H, 4);	tsPrintf(lo, "h%d", loP2H);
	ImGui::SameLine(); if (ImGui::Button(lo)) outHeight = loP2H;
	int hiP2H = tNextHigherPower2(outHeight);							tsPrintf(hi, "h%d", hiP2H);
	ImGui::SameLine(); if (ImGui::Button(hi)) outHeight = hiP2H;
	ImGui::SameLine(); ShowHelpMark("Output height in pixels.");

	if (ImGui::Button("Reset From Images") && CurrImage)
		ComputeMaxWidthHeight(outWidth, outHeight);

	// @todo This is not a cheap call. No need to do it every frame, only when dims change above.
	if (!AllDimensionsMatch(outWidth, outHeight))
	{
		ImGui::Combo("Filter", &Config::Current->ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");

		ImGui::Combo("Filter Edge Mode", &Config::Current->ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
		ImGui::SameLine();
		ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");
	}

	ImGui::Separator();

	tString extension = DoSaveFiletypeMultiFrame();

	ImGui::Separator();

	tString destDir = DoSubFolder();

	static char filename[128] = "MultiFrame";
	ImGui::InputText("Filename", filename, tNumElements(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	int numImg = Images.Count();
	tString genMsg;
	tsPrintf(genMsg, "Image %s%s will have %d frames.", filename, extension.Chs(), numImg);
	ImGui::Text(genMsg.Chs());

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	tString outFile = destDir + tString(filename) + extension;
	bool closeThisModal = false;
	if (ImGui::Button("Generate", tVector2(100.0f, 0.0f)) && (numImg >= 2))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

		if (dirExists)
		{
			if (tFileExists(outFile) && Config::Current->ConfirmFileOverwrites)
			{
				ImGui::OpenPopup("Overwrite Image File");
			}
			else
			{
				SaveMultiFrameTo(outFile, outWidth, outHeight);
				closeThisModal = true;
			}
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Overwrite image File", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool pressedOK = false, pressedCancel = false;
		DoOverwriteFileModal(outFile, pressedOK, pressedCancel);
		if (pressedOK)
			SaveMultiFrameTo(outFile, outWidth, outHeight);
		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::SaveMultiFrameTo(const tString& outFile, int outWidth, int outHeight)
{
	tList<tFrame> frames;
	for (Image* img = Images.First(); img; img = img->Next())
	{
		if (!img->IsLoaded())
			img->Load();

		if (!img->IsLoaded())
			continue;

		tImage::tPicture* currPic = img->GetCurrentPic();
		if (!currPic)
			continue;

		tImage::tPicture resampled(*currPic);
		if ((resampled.GetWidth() != outWidth) || (resampled.GetHeight() != outHeight))
			resampled.Resample(outWidth, outHeight, tImage::tResampleFilter(Config::Current->ResampleFilter), tImage::tResampleEdgeMode(Config::Current->ResampleEdgeMode));

		tFrame* frame = new tFrame(resampled.StealPixels(), outWidth, outHeight, currPic->Duration);
		frames.Append(frame);
	}

	bool success = false;
	switch (Config::Current->SaveFileTypeMultiFrame)
	{
		case 0:			// WEBP
		{
			tImageWEBP webp(frames, true);
			success = webp.Save(outFile, Config::Current->SaveFileWebpLossy, Config::Current->SaveFileWebpQualComp, Config::Current->SaveFileWebpDurMultiFrame);
			break;
		}

		case 1:			// GIF
		{
			tImageGIF gif(frames, true);
			success = gif.Save(outFile, Config::Current->SaveFileGifDurMultiFrame);
			break;
		}

		case 2:			// APNG
		{
			tImageAPNG apng(frames, true);
			success = apng.Save(outFile, Config::Current->SaveFileApngDurMultiFrame);
			break;
		}

		case 3:			// TIFF
		{
			tImageTIFF tiff(frames, true);
			success = tiff.Save(outFile, Config::Current->SaveFileTiffZLibDeflate, Config::Current->SaveFileTiffDurMultiFrame);
			break;
		}
	}

	if (!success)
		return;

	// If we saved to the same dir we are currently viewing, reload
	// and set the current image to the generated one.
	if (ImagesDir.IsEqualCI( tGetDir(outFile) ))
	{
		Images.Clear();
		PopulateImages();
		SetCurrentImage(outFile);
	}
}
