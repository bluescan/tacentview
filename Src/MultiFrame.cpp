// MultiFrame.cpp
//
// Dialog that generates multiframe images from all image files in the directory.
//
// Copyright (c) 2021-2023 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tVector2.h>
#include <Math/tInterval.h>
#include <Image/tImageGIF.h>
#include <Image/tImageWEBP.h>
#include <Image/tImageAPNG.h>
#include <Image/tImageTIFF.h>
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

	void SaveExtractedFrames(const tString& destDir, const tString& baseName, tFileType, tIntervalSet frames);
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


void Viewer::DoSaveMultiFrameModal(bool saveMultiFramePressed)
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
	const int itemWidth = 160;

	// If just opened, loop through all the images and choose the largest width and height.
	if (saveMultiFramePressed)
		ComputeMaxWidthHeight(outWidth, outHeight);

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Width", &outWidth);
	tiClampMin(outWidth, 4);
	int loP2W = tNextLowerPower2(outWidth);		tiClampMin(loP2W, 4);	tsPrintf(lo, "w%d", loP2W);
	ImGui::SameLine(); if (ImGui::Button(lo)) outWidth = loP2W;
	int hiP2W = tNextHigherPower2(outWidth);							tsPrintf(hi, "w%d", hiP2W);
	ImGui::SameLine(); if (ImGui::Button(hi)) outWidth = hiP2W;
	ImGui::SameLine(); ShowHelpMark("Output width in pixels.");

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Height", &outHeight);
	tiClampMin(outHeight, 4);
	int loP2H = tNextLowerPower2(outHeight);	tiClampMin(loP2H, 4);	tsPrintf(lo, "h%d", loP2H);
	ImGui::SameLine(); if (ImGui::Button(lo)) outHeight = loP2H;
	int hiP2H = tNextHigherPower2(outHeight);							tsPrintf(hi, "h%d", hiP2H);
	ImGui::SameLine(); if (ImGui::Button(hi)) outHeight = hiP2H;
	ImGui::SameLine(); ShowHelpMark("Output height in pixels.");

	if (ImGui::Button("Reset From Images") && CurrImage)
		ComputeMaxWidthHeight(outWidth, outHeight);

	Config::ProfileSettings& config = *Config::Current;

	// @todo This is not a cheap call. No need to do it every frame, only when dims change above.
	if (!AllDimensionsMatch(outWidth, outHeight))
	{
		ImGui::Combo("Filter", &config.ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");

		ImGui::Combo("Filter Edge Mode", &config.ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
		ImGui::SameLine();
		ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");
	}

	ImGui::Separator();

	tString extension = DoSaveFiletypeMultiFrame();

	ImGui::Separator();

	tString destDir = DoSubFolder();

	static char filename[128] = "MultiFrame";
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputText("Filename", filename, tNumElements(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	int numImg = Images.Count();
	tString genMsg;
	tsPrintf(genMsg, "Image %s.%s will have %d frames.", filename, extension.Chr(), numImg);
	ImGui::Text(genMsg.Chr());

	ImGui::NewLine();
	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	tString outFile = destDir + tString(filename) + "." + extension;
	bool closeThisModal = false;
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Generate", tVector2(100.0f, 0.0f)) && (numImg >= 2))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

		if (dirExists)
		{
			if (tFileExists(outFile) && config.ConfirmFileOverwrites)
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
	Config::ProfileSettings& config = *Config::Current;
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
			resampled.Resample(outWidth, outHeight, tImage::tResampleFilter(config.ResampleFilter), tImage::tResampleEdgeMode(config.ResampleEdgeMode));

		tFrame* frame = new tFrame(resampled.StealPixels(), outWidth, outHeight, currPic->Duration);
		frames.Append(frame);
	}

	bool success = false;
	tFileType fileType = tGetFileTypeFromName( config.SaveFileTypeMultiFrame );
	switch (fileType)
	{
		case tFileType::GIF:
		{
			tImageGIF gif(frames, true);
			tImageGIF::SaveParams params;
			params.Format					= tPixelFormat(int(tPixelFormat::FirstPalette) + config.SaveFileGifBPP - 1);
			params.Method					= tQuantize::Method(config.SaveFileGifQuantMethod);
			params.Loop						= config.SaveFileGifLoop;
			params.AlphaThreshold			= config.SaveFileGifAlphaThreshold;
			params.OverrideFrameDuration	= config.SaveFileGifDurMultiFrame;
			params.DitherLevel				= double(config.SaveFileGifDitherLevel);
			params.FilterSize				= (config.SaveFileGifFilterSize * 2) + 1;
			params.SampleFactor				= config.SaveFileGifSampleFactor;
			success = gif.Save(outFile, params);
			break;
		}

		case tFileType::WEBP:
		{
			tImageWEBP webp(frames, true);
			success = webp.Save(outFile, config.SaveFileWebpLossy, config.SaveFileWebpQualComp, config.SaveFileWebpDurMultiFrame);
			break;
		}

		case tFileType::APNG:
		{
			tImageAPNG apng(frames, true);
			tImageAPNG::SaveParams params;
			params.OverrideFrameDuration = config.SaveFileApngDurMultiFrame;
			tImageAPNG::tFormat savedFormat = apng.Save(outFile, params);
			success = (savedFormat != tImageAPNG::tFormat::Invalid);
			break;
		}

		case tFileType::TIFF:
		{
			tImageTIFF tiff(frames, true);
			tImageTIFF::SaveParams params;
			params.UseZLibCompression = config.SaveFileTiffZLibDeflate;
			params.OverrideFrameDuration = config.SaveFileTiffDurMultiFrame;
			success = tiff.Save(outFile, params);
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


tString Viewer::GetFrameFilename(int frameNum, const tString& dir, const tString& baseName, tFileType fileType)
{
	tString frameFile;
	tString extension = tSystem::tGetExtension(fileType);
	tsPrintf(frameFile, "%s%s_%03d.%s", dir.Chr(), baseName.Chr(), frameNum, extension.Chr());
	return frameFile;
}


void Viewer::SaveExtractedFrames(const tString& destDir, const tString& baseName, tFileType fileType, tIntervalSet frameSet)
{
	tAssert(CurrImage);
	int frameNum = 0;
	for (tImage::tPicture* framePic = CurrImage->GetFirstPic(); framePic; framePic = framePic->Next(), frameNum++)
	{
		if (!frameSet.Contains(frameNum))
			continue;

		tString frameFile = GetFrameFilename(frameNum, destDir, baseName, fileType);
		Viewer::SavePictureAs(*framePic, frameFile, fileType, false);
	}
}


void Viewer::DoSaveExtractFramesModal(bool saveExtractFramesPressed)
{
	if (saveExtractFramesPressed)
		ImGui::OpenPopup("Extract Frames");

	// The unused isOpenExtractFrames bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenExtractFrames = true;
	if (!ImGui::BeginPopupModal("Extract Frames", &isOpenExtractFrames, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	int numFrames = CurrImage->GetNumFrames();

	// These two statics are in lock-step for efficiency.
	static tIntervalSet frameSet;
	static char framesToExtract[128] = "all";
	if (saveExtractFramesPressed)
	{
		frameSet.Clear();
		frameSet.Add( tInterval(0, numFrames-1) );
		tString rangedStr = frameSet.Get();
		tStd::tStrncpy(framesToExtract, rangedStr.Chr(), 128);

		// Strncpy is scary. It may not write the terminating null!
		framesToExtract[127] = '\0';
	}

	tString srcFileName = tGetFileName(CurrImage->Filename);
	tString genMsg;
	tsPrintf
	(
		genMsg,
		"Extracts specified frames and saves them to a folder.\n"
		"%d frames will be extraxcted from\n"
		"%s\n"
		"Frames to extract: %s\n",
		frameSet.Count(), srcFileName.Chr(), frameSet.MakeInclusive().Get(tIntervalRep::Normal, tIntervalSetRep::Union).Chr()
	);
	ImGui::Text(genMsg.Chr());

	ImGui::Separator();
	tFileType fileType = DoSaveChooseFiletype();
	DoSaveFiletypeOptions(fileType);

	ImGui::Separator();
	tString destDir = DoSubFolder();
	static char outBaseName[128] = "ExtractedFrame";
	if (saveExtractFramesPressed)
	{
		tString currBaseName = tGetFileBaseName(CurrImage->Filename);
		if (!currBaseName.IsEmpty())
		{
			tStd::tStrncpy(outBaseName, currBaseName.Chr(), 128);
			outBaseName[127] = '\0';
		}
	}
	const int itemWidth = 160;
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputText("Base Filename", outBaseName, tNumElements(outBaseName));
	tString baseHelp;
	tString extension = tGetExtension(fileType);
	tsPrintf(baseHelp, "The output base filename without extension.\nSaved files will be:\n%s_001.%s,\n%s_002.%s,\netc.", outBaseName, extension.Chr(), outBaseName, extension.Chr());
	ImGui::SameLine(); ShowHelpMark(baseHelp.Chr());

	ImGui::SetNextItemWidth(itemWidth);
	if (ImGui::InputText("Frames", framesToExtract, tNumElements(framesToExtract)))
	{
		frameSet.Set( tString(framesToExtract) );
		if (!frameSet.IsValid())
		{
			frameSet.Clear();
			frameSet.Add( tInterval(0, numFrames-1) );
			tString rangedStr = frameSet.Get();
			tStd::tStrncpy(framesToExtract, rangedStr.Chr(), 128);

			// Strncpy is scary. It may not write the terminating null!
			framesToExtract[127] = '\0';
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("All##Frames"))
	{
		frameSet.Clear();
		frameSet.Add( tInterval(0, numFrames-1) );
		tString rangedStr = frameSet.Get();
		tStd::tStrncpy(framesToExtract, rangedStr.Chr(), 128);
		framesToExtract[127] = '\0';
	}
	ImGui::SameLine();
	ShowHelpMark
	(
		"Frames to extract. Frame numbers start at 0.\n"
		"Specify frames in interval-set notation:\n"
		"[] means inclusive. () means exclusive.\n"
		"Specify multiple intervals with a union (U) or bar (|)\n"
		"Example: [0,2]U[6,8) extracts 5 frames { 0, 1, 2, 6, 7 }"
	);

	ImGui::NewLine();
	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	// This needs to be static since DoSaveModal is called for every frame the modal is open.
	static tList<tStringItem> overwriteFiles(tListMode::Static);
	bool closeThisModal = false;

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();		
	if (Viewer::Button("Extract", tVector2(100.0f, 0.0f)))
	{
		Config::ProfileSettings& config = *Config::Current;
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

		if (dirExists)
		{
			// Generate all the filenames we will be saving _before_ we extract.
			// This allows us to make sure, if Config->AllowExtractOverwites is false.
			// that there aren't existing files.
			tList<tStringItem> extractedFilenames;
			for (int frameNum = 0; frameNum < numFrames; frameNum++)
			{
				if (!frameSet.Contains(frameNum))
					continue;

				tString frameFile = GetFrameFilename(frameNum, destDir, tString(outBaseName), fileType);
				extractedFilenames.Append(new tStringItem(frameFile));
			}

			if (config.ConfirmFileOverwrites)
			{
				overwriteFiles.Empty();
				for (tStringItem* exFile = extractedFilenames.First(); exFile; exFile = exFile->Next())
					if (tSystem::tFileExists(*exFile))
						overwriteFiles.Append(new tStringItem(*exFile));

				if (!overwriteFiles.IsEmpty())
				{
					ImGui::OpenPopup("Extract Overwrite Popup");
				}
				else
				{
					SaveExtractedFrames(destDir, tString(outBaseName), fileType, frameSet);
					closeThisModal = true;
				}
			}
			else
			{
				SaveExtractedFrames(destDir, tString(outBaseName), fileType, frameSet);
				closeThisModal = true;
			}
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Extract Overwrite Popup", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool pressedOK = false, pressedCancel = false;
		Viewer::DoOverwriteMultipleFilesModal(overwriteFiles, pressedOK, pressedCancel);
		if (pressedOK)
			SaveExtractedFrames(destDir, tString(outBaseName), fileType, frameSet);

		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}
