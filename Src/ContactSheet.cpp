// ContactSheet.cpp
//
// Dialog that generates contact sheets and processes alpha channel properly.
//
// Copyright (c) 2019-2024 Tristan Grimmer.
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
#include "ContactSheet.h"
#include "OpenSaveDialogs.h"
#include "TacentView.h"
#include "GuiUtil.h"
#include "Image.h"
namespace Viewer { extern void DoFillColourInterface(const char* = nullptr, bool = false); }
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;


namespace Viewer
{
	void SaveContactSheetTo
	(
		const tString& outFile,
		int contactWidth, int contactHeight,
		int numCols, int numRows,
		int finalWidth, int finalHeight
	);

	bool AnyImageNeedsResize(int frameWidth, int frameHeight);
}


bool Viewer::AnyImageNeedsResize(int frameWidth, int frameHeight)
{
	bool anyImageNeedsResize = false;
	for (Image* checkImg = Images.First(); checkImg; checkImg = checkImg->Next())
	{
		if (!checkImg->IsLoaded())
			checkImg->Load();
		if (!checkImg->IsLoaded())
			continue;
		
		if ((checkImg->GetWidth() != frameWidth) || (checkImg->GetHeight() != frameHeight))
		{
			anyImageNeedsResize = true;
			break;
		}
	}
	return anyImageNeedsResize;
}


void Viewer::DoSaveContactSheetModal(bool saveContactSheetPressed)
{
	if (saveContactSheetPressed)
		ImGui::OpenPopup("Contact Sheet");

	// The unused isOpenContactSheet bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenContactSheet = true;
	if (!ImGui::BeginPopupModal("Contact Sheet", &isOpenContactSheet, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	const ImGuiStyle& style = ImGui::GetStyle();
	static int frameWidth = 256;
	static int frameHeight = 256;
	static bool anyImageNeedsResize = false;
	static int numRows = 4;
	static int numCols = 4;
	static int finalWidth = 2048;
	static int finalHeight = 2048;
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	int picW = picture->GetWidth();
	int picH = picture->GetHeight();
	if (saveContactSheetPressed)
	{
		frameWidth = picW;
		frameHeight = picH;
		numRows = int(tCeiling(tSqrt(float(Images.Count()))));
		numCols = int(tCeiling(tSqrt(float(Images.Count()))));
		anyImageNeedsResize = AnyImageNeedsResize(frameWidth, frameHeight);
	}

	int contactWidth = frameWidth * numCols;
	int contactHeight = frameHeight * numRows;
	if (saveContactSheetPressed)
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}

	static char lo[32];
	static char hi[32];

	float itemWidth		= Gutil::GetUIParamScaled(160.0f, 2.5f);
	float powButWidth	= Gutil::GetUIParamScaled(60.0f, 2.5f);
	float powButOffset	= Gutil::GetUIParamScaled(252.0f, 2.5f);
	float buttonWidth	= Gutil::GetUIParamScaled(76.0f, 2.5f);

	tVector2 powSize(powButWidth, 0.0f);

	ImGui::SetNextItemWidth(itemWidth);
	if (ImGui::InputInt("Frame Width", &frameWidth))
		anyImageNeedsResize = AnyImageNeedsResize(frameWidth, frameHeight);
	tiClampMin(frameWidth, 4);
	int loP2W = tNextLowerPower2(frameWidth);
	tiClampMin(loP2W, 4);
	tsPrintf(lo, "%d##framewidthlo", loP2W);
	ImGui::SameLine();
	ImGui::SetCursorPosX(powButOffset);
	if (ImGui::Button(lo, powSize))
		frameWidth = loP2W;
	int hiP2W = tNextHigherPower2(frameWidth);
	tsPrintf(hi, "%d##framewidthhi", hiP2W);
	ImGui::SameLine();
	if (ImGui::Button(hi, powSize))
		frameWidth = hiP2W;
	ImGui::SameLine();
	Gutil::HelpMark("Single frame width in pixels.");

	ImGui::SetNextItemWidth(itemWidth);
	if (ImGui::InputInt("Frame Height", &frameHeight))
		anyImageNeedsResize = AnyImageNeedsResize(frameWidth, frameHeight);
	tiClampMin(frameHeight, 4);
	int loP2H = tNextLowerPower2(frameHeight);
	tiClampMin(loP2H, 4);
	tsPrintf(lo, "%d##frameheightlo", loP2H);
	ImGui::SameLine();
	ImGui::SetCursorPosX(powButOffset);
	if (ImGui::Button(lo, powSize))
		frameHeight = loP2H;
	int hiP2H = tNextHigherPower2(frameHeight);
	tsPrintf(hi, "%d##frameheighthi", hiP2H);
	ImGui::SameLine();
	if (ImGui::Button(hi, powSize))
		frameHeight = hiP2H;
	ImGui::SameLine();
	Gutil::HelpMark("Single frame height in pixels.");

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Columns", &numCols);
	ImGui::SameLine();
	Gutil::HelpMark("Number of columns.");
 
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Rows", &numRows);
	ImGui::SameLine();
	Gutil::HelpMark("Number of rows.");

	int numImg = Images.Count();
	if ((numImg >= 2) && (numCols*numRows >= 2) && (numCols*numRows > numImg))
		DoFillColourInterface("Empty pages in the contact sheet\nwill be filled with this colour.", true);

	if (ImGui::Button("Reset From Image") && CurrImage)
	{
		frameWidth = picW;
		frameHeight = picH;
		numRows = int(tCeiling(tSqrt(float(Images.Count()))));
		numCols = int(tCeiling(tSqrt(float(Images.Count()))));
	}
	ImGui::Separator();

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Final Width", &finalWidth);
	tiClampMin(finalWidth, 4);
	loP2W = tNextLowerPower2(finalWidth);
	tiClampMin(loP2W, 4);
	tsPrintf(lo, "%d##widthlo", loP2W);
	ImGui::SameLine();
	ImGui::SetCursorPosX(powButOffset);
	if (ImGui::Button(lo, powSize))
		finalWidth = loP2W;
	hiP2W = tNextHigherPower2(finalWidth);
	tsPrintf(hi, "%d##widthhi", hiP2W);
	ImGui::SameLine();
	if (ImGui::Button(hi, powSize))
		finalWidth = hiP2W;
	ImGui::SameLine();
	Gutil::HelpMark("Final scaled output sheet width in pixels.");

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Final Height", &finalHeight);
	tiClampMin(finalHeight, 4);
	loP2H = tNextLowerPower2(finalHeight);
	tiClampMin(loP2H, 4);
	tsPrintf(lo, "%d##heightlo", loP2H);
	ImGui::SameLine();
	ImGui::SetCursorPosX(powButOffset);
	if (ImGui::Button(lo, powSize))
		finalHeight = loP2H;
	hiP2H = tNextHigherPower2(finalHeight);
	tsPrintf(hi, "%d##heighthi", hiP2H);
	ImGui::SameLine();
	if (ImGui::Button(hi, powSize))
		finalHeight = hiP2H;
	ImGui::SameLine();
	Gutil::HelpMark("Final scaled output sheet height in pixels.");

	if (ImGui::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}
	ImGui::Separator();

	// Only display filter option if filtering will be used.
	bool needFinalResize = ((finalWidth != contactWidth) || (finalHeight != contactHeight));
	Config::ProfileData& profile = Config::GetProfileData();
	if (anyImageNeedsResize || needFinalResize)
	{
		if (anyImageNeedsResize)
		{
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("Frame Filter", &profile.ResampleFilterContactFrame, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
			ImGui::SameLine();
			Gutil::HelpMark("Filtering method to use when resizing input frame images.");

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("Frame Filter Edge Mode", &profile.ResampleEdgeModeContactFrame, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
			ImGui::SameLine();
			Gutil::HelpMark("How frame filter chooses pixels along image edges. Use wrap for tiled textures.");
		}
		if (needFinalResize)
		{
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("Final Filter", &profile.ResampleFilterContactFinal, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
			ImGui::SameLine();
			Gutil::HelpMark("Filtering method to use when resizing output image.");

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("Final Filter Edge Mode", &profile.ResampleEdgeModeContactFinal, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
			ImGui::SameLine();
			Gutil::HelpMark("How output filter chooses pixels along image edges. Use wrap for tiled textures.");
		}

		ImGui::Separator();
	}

	tFileType fileType = DoSaveChooseFiletype();
	DoSaveFiletypeOptions(fileType);
	tString extensionWithDot = tString(".") + tGetExtension(fileType);
	ImGui::Separator();
	tString destDir = DoSubFolder();

	static char filename[128] = "ContactSheet";
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputText("Filename", filename, tNumElements(filename));
	ImGui::SameLine(); Gutil::HelpMark("The output filename without extension.");

	tString genMsg;
	if (numImg >= 2)
		tsPrintf(genMsg, " Sheet will have %d frames with %d images.", numRows*numCols, tMin(numImg, numRows*numCols));
	else
		tsPrintf(genMsg, " Warning: At least 2 images are needed.");
	ImGui::Text(genMsg.Chr());

	ImGui::NewLine();
	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();
	
	tString outFile = destDir + tString(filename) + extensionWithDot;
	bool closeThisModal = false;

	float genButOffset	= Gutil::GetUIParamScaled(324.0f, 2.5f);
	ImGui::SetCursorPosX(genButOffset);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("Generate", tVector2(buttonWidth, 0.0f)) && (numImg >= 2))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

		if (dirExists)
		{
			if (tFileExists(outFile) && profile.ConfirmFileOverwrites)
			{
				ImGui::OpenPopup("Overwrite Contact Sheet File");
			}
			else
			{
				SaveContactSheetTo(outFile, contactWidth, contactHeight, numCols, numRows, finalWidth, finalHeight);
				closeThisModal = true;
			}
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui.
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Overwrite Contact Sheet File", &isOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
	{
		bool pressedOK = false, pressedCancel = false;
		DoOverwriteFileModal(outFile, pressedOK, pressedCancel);
		if (pressedOK)
			SaveContactSheetTo(outFile, contactWidth, contactHeight, numCols, numRows, finalWidth, finalHeight);
		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::SaveContactSheetTo
(
	const tString& outFile,
	int contactWidth, int contactHeight,
	int numCols, int numRows,
	int finalWidth, int finalHeight
)
{
	Config::ProfileData& profile = Config::GetProfileData();

	tImage::tPicture outPic(contactWidth, contactHeight);
	outPic.SetAll(profile.FillColourContact);

	// Do the work.
	int frameWidth = contactWidth / numCols;
	int frameHeight = contactHeight / numRows;
	int ix = 0;
	int iy = 0;
	int frame = 0;

	tPrintf("Loading all frames...\n");
	for (Image* img = Images.First(); img; img = img->Next())
	{
		if (!img->IsLoaded())
			img->Load();
	}

	Image* currImg = Images.First();
	while (currImg)
	{
		if (!currImg->IsLoaded())
		{
			currImg = currImg->Next();
			continue;
		}

		tPrintf("Processing frame %d : %s at (%d, %d).\n", frame, currImg->Filename.Chr(), ix, iy);
		frame++;
		tImage::tPicture* currPic = currImg->GetCurrentPic();

		tImage::tPicture resampled;
		if ((currImg->GetWidth() != frameWidth) || (currImg->GetHeight() != frameHeight))
		{
			resampled.Set(*currPic);
			resampled.Resample(frameWidth, frameHeight, tImage::tResampleFilter(profile.ResampleFilterContactFrame), tImage::tResampleEdgeMode(profile.ResampleEdgeModeContactFrame));
		}
		else
		{
			tPrintf("No resizing of [%s] needed.\n", tSystem::tGetFileBaseName(currImg->Filename).Chr());
		}

		// Copy resampled frame into place.
		for (int y = 0; y < frameHeight; y++)
			for (int x = 0; x < frameWidth; x++)
				outPic.SetPixel
				(
					x + (ix*frameWidth),
					y + ((numRows-1-iy)*frameHeight),
					resampled.IsValid() ? resampled.GetPixel(x, y) : currPic->GetPixel(x, y)
				);

		currImg = currImg->Next();

		ix++;
		if (ix >= numCols)
		{
			ix = 0;
			iy++;
			if (iy >= numRows)
				break;
		}
	}

	tFileType saveFileType = tGetFileTypeFromName(profile.SaveFileType);
	if ((finalWidth == contactWidth) && (finalHeight == contactHeight))
	{
		tPrintf("No resizing of output [%s] image needed.\n", tSystem::tGetFileBaseName(outFile).Chr());
		SavePictureAs(outPic, outFile, saveFileType, true);
	}
	else
	{
		tImage::tPicture finalResampled(outPic);
		finalResampled.Resample(finalWidth, finalHeight, tImage::tResampleFilter(profile.ResampleFilterContactFinal), tImage::tResampleEdgeMode(profile.ResampleEdgeModeContactFinal));
		SavePictureAs(finalResampled, outFile, saveFileType, true);
	}

	// If we saved to the same dir we are currently viewing, reload
	// and set the current image to the generated one.
	if (ImagesDir.IsEqualCI( tGetDir(outFile) ))
	{
		Images.Clear();
		PopulateImages();
		SetCurrentImage(outFile);
	}
}
