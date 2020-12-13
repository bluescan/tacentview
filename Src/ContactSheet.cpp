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
		numRows = int(tCeiling(tSqrt(float(Images.Count()))));
		numCols = int(tCeiling(tSqrt(float(Images.Count()))));
	}

	int contactWidth = frameWidth * numCols;
	int contactHeight = frameHeight * numRows;
	if (justOpened)
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}

	static char lo[32];
	static char hi[32];

	ImGui::InputInt("Frame Width", &frameWidth);
	tiClampMin(frameWidth, 4);
	int loP2W = tNextLowerPower2(frameWidth);	tiClampMin(loP2W, 4);	tsPrintf(lo, "fw%d", loP2W);
	ImGui::SameLine(); if (ImGui::Button(lo)) frameWidth = loP2W;
	int hiP2W = tNextHigherPower2(frameWidth);							tsPrintf(hi, "fw%d", hiP2W);
	ImGui::SameLine(); if (ImGui::Button(hi)) frameWidth = hiP2W;
	ImGui::SameLine(); ShowHelpMark("Single frame width in pixels.");

	ImGui::InputInt("Frame Height", &frameHeight);
	tiClampMin(frameHeight, 4);
	int loP2H = tNextLowerPower2(frameHeight);	tiClampMin(loP2H, 4);	tsPrintf(lo, "fh%d", loP2H);
	ImGui::SameLine(); if (ImGui::Button(lo)) frameHeight = loP2H;
	int hiP2H = tNextHigherPower2(frameHeight);							tsPrintf(hi, "fh%d", hiP2H);
	ImGui::SameLine(); if (ImGui::Button(hi)) frameHeight = hiP2H;
	ImGui::SameLine(); ShowHelpMark("Single frame height in pixels.");

	ImGui::InputInt("Columns", &numCols);
	ImGui::SameLine();
	ShowHelpMark("Number of columns.");
 
	ImGui::InputInt("Rows", &numRows);
	ImGui::SameLine();
	ShowHelpMark("Number of rows.");

	if (ImGui::Button("Reset From Image") && CurrImage)
	{
		frameWidth = picW;
		frameHeight = picH;
		numRows = int(tCeiling(tSqrt(float(Images.Count()))));
		numCols = int(tCeiling(tSqrt(float(Images.Count()))));
	}
	ImGui::Separator();

	ImGui::InputInt("Final Width", &finalWidth);
	tiClampMin(finalWidth, 4);
	loP2W = tNextLowerPower2(finalWidth);	tiClampMin(loP2W, 4);	tsPrintf(lo, "w%d", loP2W);
	ImGui::SameLine(); if (ImGui::Button(lo)) finalWidth = loP2W;
	hiP2W = tNextHigherPower2(finalWidth);							tsPrintf(hi, "w%d", hiP2W);
	ImGui::SameLine(); if (ImGui::Button(hi)) finalWidth = hiP2W;
	ImGui::SameLine(); ShowHelpMark("Final scaled output sheet width in pixels.");

	ImGui::InputInt("Final Height", &finalHeight);
	tiClampMin(finalHeight, 4);
	loP2H = tNextLowerPower2(finalHeight);	tiClampMin(loP2H, 4);	tsPrintf(lo, "h%d", loP2H);
	ImGui::SameLine(); if (ImGui::Button(lo)) finalHeight = loP2H;
	hiP2H = tNextHigherPower2(finalHeight);							tsPrintf(hi, "h%d", hiP2H);
	ImGui::SameLine(); if (ImGui::Button(hi)) finalHeight = hiP2H;
	ImGui::SameLine(); ShowHelpMark("Final scaled output sheet height in pixels.");

	if (ImGui::Button("Reset"))
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}
	ImGui::Separator();

	ImGui::Combo("Filter", &Config.ResampleFilter, tResampleFilterNames, tNumElements(tResampleFilterNames), tNumElements(tResampleFilterNames));
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
		tsPrintf(genMsg, " Sheet will have %d frames with %d images.", numRows*numCols, tMin(numImg, numRows*numCols));
	else
		tsPrintf(genMsg, " Warning: At least 2 images are needed.");
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
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

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
			resampled.Resample(frameWidth, frameHeight, tImage::tResampleFilter(Config.ResampleFilter));
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
	tImage::tImageTGA::tFormat tgaFmt = allOpaque ? tImage::tImageTGA::tFormat::Bit24 : tImage::tImageTGA::tFormat::Bit32;
	if ((finalWidth == contactWidth) && (finalHeight == contactHeight))
	{
		if (Config.SaveFileType == 0)
			outPic.SaveTGA(outFile, tgaFmt, Config.SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
		else
			outPic.Save(outFile, colourFmt, Config.SaveFileJpegQuality);
	}
	else
	{
		tImage::tPicture finalResampled(outPic);
		finalResampled.Resample(finalWidth, finalHeight, tImage::tResampleFilter(Config.ResampleFilter));

		if (Config.SaveFileType == 0)
			finalResampled.SaveTGA(outFile, tgaFmt, Config.SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
		else
			finalResampled.Save(outFile, colourFmt, Config.SaveFileJpegQuality);
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
