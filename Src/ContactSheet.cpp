// ContactSheet.cpp
//
// Dialog that generates contact sheets and processes alpha channel properly.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
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
#include "SaveDialogs.h"
#include "TacentView.h"
#include "Image.h"
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
}


void Viewer::DoContactSheetModalDialog(bool justOpened)
{
	static int frameWidth = 256;
	static int frameHeight = 256;
	static int numRows = 4;
	static int numCols = 4;
	static int finalWidth = 2048;
	static int finalHeight = 2048;
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	int picW = picture->GetWidth();
	int picH = picture->GetHeight();
	if (justOpened)
	{
		frameWidth = picW;
		frameHeight = picH;
		numRows = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
		numCols = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
	}

	int contactWidth = frameWidth * numCols;
	int contactHeight = frameHeight * numRows;
	if (justOpened)
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}

	ImGui::InputInt("Frame Width", &frameWidth);
	ImGui::SameLine();
	ShowHelpMark("Single frame width in pixels.");

	ImGui::InputInt("Frame Height", &frameHeight);
	ImGui::SameLine();
	ShowHelpMark("Single frame height in pixels.");

	ImGui::InputInt("Columns", &numCols);
	ImGui::SameLine();
	ShowHelpMark("Number of columns. Determines overall width.");
 
	ImGui::InputInt("Rows", &numRows);
	ImGui::SameLine();
	ShowHelpMark("Number of rows. Determines overall height.");

	if (ImGui::Button("Read From Image") && CurrImage)
	{
		frameWidth = picW;
		frameHeight = picH;
		numRows = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
		numCols = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
	}

	ImGui::Separator();
	ImGui::InputInt("Final Width", &finalWidth);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output sheet width in pixels.");

	ImGui::InputInt("Final Height", &finalHeight);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output sheet height in pixels.");

	if (ImGui::Button("Prev Pow2"))
	{
		finalWidth = tMath::tNextLowerPower2(contactWidth);
		finalHeight = tMath::tNextLowerPower2(contactHeight);
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Pow2"))
	{
		finalWidth = tMath::tNextHigherPower2(contactWidth);
		finalHeight = tMath::tNextHigherPower2(contactHeight);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}
	ImGui::Separator();

	// Matches tImage::tPicture::tFilter.
	const char* filterItems[] = { "NearestNeighbour", "Box", "Bilinear", "Bicubic", "Quadratic", "Hamming" };
	ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, tNumElements(filterItems));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	tString extension = DoSaveFiletype();
	ImGui::Separator();
	tString destDir = DoSubFolder();

	static char filename[128] = "ContactSheet";
	ImGui::InputText("Filename", filename, tNumElements(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	int numImg = Images.Count();
	tString genMsg;
	if (numImg >= 2)
		tsPrintf(genMsg, "Sheet Will Have %d Frames", numImg);
	else
		tsPrintf(genMsg, "More Than %d Images Needed", numImg);
	ImGui::Text(genMsg.Chars());

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();
	
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	tString outFile = destDir + tString(filename) + extension;
	bool closeThisModal = false;
	if (ImGui::Button("Generate", tVector2(100, 0)) && (numImg >= 2))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
			dirExists = tCreateDir(destDir);

		if (dirExists)
		{
			if (tFileExists(outFile) && Config.ConfirmFileOverwrites)
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
	if (ImGui::BeginPopupModal("Overwrite Contact Sheet File", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
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
	tImage::tPicture outPic(contactWidth, contactHeight);
	outPic.SetAll(tColouri(0, 0, 0, 0));

	// Do the work.
	int frameWidth = contactWidth / numCols;
	int frameHeight = contactHeight / numRows;
	int ix = 0;
	int iy = 0;
	int frame = 0;

	tPrintf("Loading all frames...\n");
	bool allOpaque = true;
	for (Image* img = Images.First(); img; img = img->Next())
	{
		if (!img->IsLoaded())
			img->Load();

		if (img->IsLoaded() && !img->IsOpaque())
			allOpaque = false;
	}

	Image* currImg = Images.First();
	while (currImg)
	{
		if (!currImg->IsLoaded())
		{
			currImg = currImg->Next();
			continue;
		}

		tPrintf("Processing frame %d : %s at (%d, %d).\n", frame, currImg->Filename.Chars(), ix, iy);
		frame++;
		tImage::tPicture* currPic = currImg->GetCurrentPic();

		tImage::tPicture resampled;
		if ((currImg->GetWidth() != frameWidth) || (currImg->GetHeight() != frameHeight))
		{
			resampled.Set(*currPic);
			resampled.Resample(frameWidth, frameHeight, tImage::tPicture::tFilter(Config.ResampleFilter));
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

	tImage::tPicture::tColourFormat colourFmt = allOpaque ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
	if ((finalWidth == contactWidth) && (finalHeight == contactHeight))
	{
		if (Config.SaveFileType == 0)
			outPic.SaveTGA(outFile, tImage::tImageTGA::tFormat::Auto, Config.SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
		else
			outPic.Save(outFile, colourFmt, Config.SaveFileJpgQuality);
	}
	else
	{
		tImage::tPicture finalResampled(outPic);
		finalResampled.Resample(finalWidth, finalHeight, tImage::tPicture::tFilter(Config.ResampleFilter));

		if (Config.SaveFileType == 0)
			finalResampled.SaveTGA(outFile, tImage::tImageTGA::tFormat::Auto, Config.SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
		else
			finalResampled.Save(outFile, colourFmt, Config.SaveFileJpgQuality);
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
