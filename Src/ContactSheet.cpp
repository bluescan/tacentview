// ContactSheet.cpp
//
// Dialog that generates contact sheets and processes alpha channel properly.
//
// Copyright (c) 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "imgui.h"
#include "ContactSheet.h"
#include "TacitTexView.h"
#include "TacitImage.h"


void TexView::ShowContactSheetDialog(bool* popen, bool justOpened)
{
	ImGuiWindowFlags windowFlags = 0;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(370, 280), ImGuiCond_FirstUseEver);

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Contact Sheet Generator", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	static int frameWidth = 256;
	static int frameHeight = 256;
	static int numRows = 4;
	static int numCols = 4;
	static int finalWidth = 2048;
	static int finalHeight = 2048;
	if (justOpened && CurrImage)
	{
		frameWidth = CurrImage->GetWidth();
		frameHeight = CurrImage->GetHeight();
		numRows = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
		numCols = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
	}

	int contactWidth = frameWidth * numCols;
	int contactHeight = frameHeight * numRows;
	if (justOpened && CurrImage)
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

	ImGui::Separator();
	ImGui::InputInt("Final Width", &finalWidth);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output sheet height in pixels.");

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
	ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, IM_ARRAYSIZE(filterItems));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	const char* fileTypeItems[] = { "TGA", "PNG", "BMP", "JPG", "GIF" };
	static int itemCurrent = 0;
	ImGui::Combo("File Type", &itemCurrent, fileTypeItems, IM_ARRAYSIZE(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Output image format. JPG and GIF do not support alpha channel.");

	tString extension = ".tga";
	switch (itemCurrent)
	{
		case 0: extension = ".tga"; break;
		case 1: extension = ".png"; break;
		case 2: extension = ".bmp"; break;
		case 3: extension = ".jpg"; break;
		case 4: extension = ".gif"; break;
	}

	static char filename[128] = "ContactSheet";
	ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	if (ImGui::Button("Generate"))
	{
		tString imagesDir = tSystem::tGetCurrentDir();
		if (TexView::ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(TexView::ImageFileParam.Get()))
			imagesDir = tSystem::tGetDir(TexView::ImageFileParam.Get());
		tString outFile = imagesDir + tString(filename) + extension;

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
		for (TacitImage* img = Images.First(); img; img = img->Next())
		{
			if (!img->IsLoaded())
				img->Load();

			if (img->IsLoaded() && !img->IsOpaque())
				allOpaque = false;
		}

		TacitImage* currImg = Images.First();
		while (currImg)
		{
			if (!currImg->IsLoaded())
			{
				currImg = currImg->Next();
				continue;
			}

			if (tSystem::tGetFileBaseName(currImg->Filename) == tSystem::tGetFileBaseName(outFile))
			{
				currImg = currImg->Next();
				continue;
			}

			tPrintf("Processing frame %d : %s at (%d, %d).\n", frame, currImg->Filename.ConstText(), ix, iy);
			frame++;
			tImage::tPicture* currPic = currImg->GetPrimaryPicture();

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
			outPic.Save(outFile, colourFmt);
		}
		else
		{
			tImage::tPicture finalResampled(outPic);
			finalResampled.Resample(finalWidth, finalHeight, tImage::tPicture::tFilter(Config.ResampleFilter));
			finalResampled.Save(outFile, colourFmt);
		}
		Images.Clear();
		PopulateImages();
		SetCurrentImage(outFile);
	}

	ImGui::End();
}
