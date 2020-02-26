// SaveDialogs.cpp
//
// Modal dialogs save-as and save-all.
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

#include "imgui.h"
#include "SaveDialogs.h"
#include "TacitImage.h"
#include "TacitTexView.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;


namespace TexView
{
	void SaveImageTo(tPicture* picture, const tString& outFile, int finalWidth, int finalHeight);
	void SaveAllImages(const tString& destDir, const tString& extension, float percent, int width, int height);
	void GetFilesNeedingOverwrite(const tString& destDir, tListZ<tStringItem>& overwriteFiles, const tString& extension);
}


tString TexView::DoSubFolder()
{
	// Output sub-folder
	char subFolder[256]; tMemset(subFolder, 0, 256);
	tStrncpy(subFolder, Config.SaveSubFolder.Chars(), 255);
	ImGui::InputText("Folder", subFolder, 256);
	Config.SaveSubFolder.Set(subFolder);
	tString destDir = ImagesDir;
	if (!Config.SaveSubFolder.IsEmpty())
		destDir += Config.SaveSubFolder + "/";
	tString toolTipText;
	tsPrintf(toolTipText, "Save to %s", destDir.Chars());
	ShowToolTip(toolTipText.Chars());
	ImGui::SameLine();
	if (ImGui::Button("Default"))
		Config.SaveSubFolder.Set("Saved");
	ImGui::SameLine();
	if (ImGui::Button("This"))
		Config.SaveSubFolder.Clear();

	return destDir;
}


tString TexView::DoSaveFiletype()
{
	const char* fileTypeItems[] = { "tga", "png", "bmp", "jpg", "gif" };
	ImGui::Combo("File Type", &Config.SaveFileType, fileTypeItems, tNumElements(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Output image format. TGA, PNG, and BMP support an alpha channel.");

	tString extension = ".tga";
	switch (Config.SaveFileType)
	{
		case 0: extension = ".tga"; break;
		case 1: extension = ".png"; break;
		case 2: extension = ".bmp"; break;
		case 3: extension = ".jpg"; break;
		case 4: extension = ".gif"; break;
	}

	if (Config.SaveFileType == 0)
		ImGui::Checkbox("RLE Compression", &Config.SaveFileTargaRLE);
	else if (Config.SaveFileType == 3)
		ImGui::SliderFloat("Quality", &Config.SaveFileJpgQuality, 0.0f, 100.0f, "%.1f");

	return extension;
}


void TexView::DoSaveAsModalDialog(bool justOpened)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);

	static int dstW = 512;
	static int dstH = 512;
	int srcW = picture->GetWidth();
	int srcH = picture->GetHeight();

	if (justOpened)
	{
		dstW = picture->GetWidth();
		dstH = picture->GetHeight();
	}

	float aspect = float(srcW) / float(srcH);
	static bool lockAspect = true;

	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Width", &dstW) && lockAspect)
		dstH = int( float(dstW) / aspect );
	ImGui::PopItemWidth();
	ImGui::SameLine(); if (ImGui::Button("Pow2 W-"))
		{ dstW = tMath::tNextLowerPower2(srcW); if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); if (ImGui::Button("Pow2 W+"))
		{ dstW = tMath::tNextHigherPower2(srcW); if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output width in pixels.\nIf dimensions match current no scaling.");

	if (ImGui::Checkbox("Lock Aspect", &lockAspect) && lockAspect)
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Height", &dstH) && lockAspect)
		dstW = int( float(dstH) * aspect );
	ImGui::PopItemWidth();
	ImGui::SameLine(); if (ImGui::Button("Pow2 H-"))
		{ dstH = tMath::tNextLowerPower2(srcH); if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine(); if (ImGui::Button("Pow2 H+"))
		{ dstH = tMath::tNextHigherPower2(srcH); if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output height in pixels.\nIf dimensions match current no scaling.");

	if (ImGui::Button("Reset"))
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::Separator();

	if ((dstW != srcW) || (dstH != srcH))
	{
		// Matches tImage::tPicture::tFilter.
		const char* filterItems[] = { "NearestNeighbour", "Box", "Bilinear", "Bicubic", "Quadratic", "Hamming" };
		ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, tNumElements(filterItems));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");
	}

	tString extension = DoSaveFiletype();
	ImGui::Separator();
	tString destDir = DoSubFolder();

	static char filename[128] = "Filename";
	if (justOpened)
	{
		tString baseName = tSystem::tGetFileBaseName(CurrImage->Filename);
		tStrcpy(filename, baseName.Chars());
	}
	ImGui::InputText("Filename", filename, tNumElements(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();
	
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	tString outFile = destDir + tString(filename) + extension;
	bool closeThisModal = false;
	if (ImGui::Button("Save", tVector2(100, 0)))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
			dirExists = tCreateDir(destDir);

		if (dirExists)
		{
			if (tFileExists(outFile) && Config.ConfirmFileOverwrites)
			{
				ImGui::OpenPopup("Overwrite File");
			}
			else
			{
				SaveImageTo(picture, outFile, dstW, dstH);
				closeThisModal = true;
			}
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Overwrite File", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool pressedOK = false, pressedCancel = false;
		DoOverwriteFileModal(outFile, pressedOK, pressedCancel);
		if (pressedOK)
 			SaveImageTo(picture, outFile, dstW, dstH);
		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void TexView::DoSaveAllModalDialog(bool justOpened)
{
	ImGui::Text("Save all %d images to the image type you select.", Images.GetNumItems()); ImGui::SameLine();
	ShowHelpMark
	(
		"Images may be resized based on the Size Mode:\n"
		"\n"
		"  Percent of Original\n"
		"  Use 100% for no scaling/resampling. Less\n"
		"  than 100% downscales. Greater than upscales.\n"
		"\n"
		"  Set Width and Height\n"
		"  Scales all images to specified width and\n"
		"  height, possibly non-uniformly.\n"
		"\n"
		"  Set Width - Retain Aspect\n"
		"  All images will have specified width. Always\n"
		"  uniform scale. Varying height.\n"
		"\n"
		"  Set Height - Retain Aspect\n"
		"  All images will have specified height. Always\n"
		"  uniform scale. Varying width.\n"
	);

	ImGui::Separator();

	static int width = 512;
	static int height = 512;
	static float percent = 100.0f;
	const char* sizeModeNames[] = { "Percent of Original", "Set Width and Height", "Set Width - Retain Aspect", "Set Height - Retain Aspect" };
	ImGui::Combo("Size Mode", &Config.SaveAllSizeMode, sizeModeNames, tNumElements(sizeModeNames));
	switch (Settings::SizeMode(Config.SaveAllSizeMode))
	{
		case Settings::SizeMode::Percent:
			ImGui::InputFloat("Percent", &percent, 1.0f, 10.0f, "%.1f");	ImGui::SameLine();	ShowHelpMark("Percent of original size.");
			break;

		case Settings::SizeMode::SetWidthAndHeight:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;

		case Settings::SizeMode::SetWidthRetainAspect:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			break;

		case Settings::SizeMode::SetHeightRetainAspect:
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;
	};

	ImGui::Separator();
	if (!((Settings::SizeMode(Config.SaveAllSizeMode) == Settings::SizeMode::Percent) && (percent == 100.0f)))
	{
		// Matches tImage::tPicture::tFilter.
		const char* filterItems[] = { "NearestNeighbour", "Box", "Bilinear", "Bicubic", "Quadratic", "Hamming" };
		ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, tNumElements(filterItems));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");
	}
	tMath::tiClampMin(width, 4);
	tMath::tiClampMin(height, 4);

	tString extension = DoSaveFiletype();
	ImGui::Separator();
	tString destDir = DoSubFolder();

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	static tListZ<tStringItem> overwriteFiles;
	bool closeThisModal = false;
	if (ImGui::Button("Save All", tVector2(100, 0)))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
			dirExists = tCreateDir(destDir);

		if (dirExists)
		{
			overwriteFiles.Empty();
			GetFilesNeedingOverwrite(destDir, overwriteFiles, extension);
			if (!overwriteFiles.IsEmpty() && Config.ConfirmFileOverwrites)
			{
				ImGui::OpenPopup("Overwrite Multiple Files");
			}
			else
			{
				SaveAllImages(destDir, extension, percent, width, height);
				closeThisModal = true;
			}
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Overwrite Multiple Files", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool pressedOK = false, pressedCancel = false;
		DoOverwriteMultipleFilesModal(overwriteFiles, pressedOK, pressedCancel);
		if (pressedOK)
			SaveAllImages(destDir, extension, percent, width, height);

		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
	{
		overwriteFiles.Empty();
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void TexView::GetFilesNeedingOverwrite(const tString& destDir, tListZ<tStringItem>& overwriteFiles, const tString& extension)
{
	for (TacitImage* image = Images.First(); image; image = image->Next())
	{
		tString baseName = tSystem::tGetFileBaseName(image->Filename);
		tString outFile = destDir + tString(baseName) + extension;

		// Only add unique items to the list.
		if (tSystem::tFileExists(outFile) && !overwriteFiles.Contains(outFile))
			overwriteFiles.Append(new tStringItem(outFile));
	}
}


void TexView::DoOverwriteMultipleFilesModal(const tListZ<tStringItem>& overwriteFiles, bool& pressedOK, bool& pressedCancel)
{
	tAssert(!overwriteFiles.IsEmpty());
	tString dir = tSystem::tGetDir(*overwriteFiles.First());
	ImGui::Text("The Following Files");
	ImGui::Indent();
	int fnum = 0;
	const int maxToShow = 6;
	for (tStringItem* filename = overwriteFiles.First(); filename && (fnum < maxToShow); filename = filename->Next(), fnum++)
	{
		tString file = tSystem::tGetFileName(*filename);
		ImGui::Text("%s", file.Chars());
	}
	int remaining = overwriteFiles.GetNumItems() - fnum;
	if (remaining > 0)
		ImGui::Text("And %d more.", remaining);
	ImGui::Unindent();
	ImGui::Text("Already Exist In Folder");
	ImGui::Indent(); ImGui::Text("%s", dir.Chars()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Text("Overwrite Files?");
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config.ConfirmFileOverwrites);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
	{
		pressedCancel = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Overwrite", tVector2(100, 0)))
	{
		pressedOK = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void TexView::SaveAllImages(const tString& destDir, const tString& extension, float percent, int width, int height)
{
	float scale = percent/100.0f;
	tString currFile = CurrImage ? CurrImage->Filename : tString();

	for (TacitImage* image = Images.First(); image; image = image->Next())
	{
		tString baseName = tSystem::tGetFileBaseName(image->Filename);
		tString outFile = destDir + tString(baseName) + extension;

		// We make sure to maintain the loaded/unloaded state of all images. This function
		// can process many files, so we don't want them all in memory at once by indiscriminantly
		// loading them all.
		bool imageLoaded = image->IsLoaded();
		if (!imageLoaded)
			image->Load();
		tImage::tPicture outPic;
		outPic.Set(*image->GetCurrentPic());
		if (!imageLoaded)
			image->Unload();

		int outW = outPic.GetWidth();
		int outH = outPic.GetHeight();
		float aspect = float(outW) / float(outH);

		switch (Settings::SizeMode(Config.SaveAllSizeMode))
		{
			case Settings::SizeMode::Percent:
				if (tMath::tApproxEqual(scale, 1.0f, 0.01f))
					break;
				outW = int( tRound(float(outW)*scale) );
				outH = int( tRound(float(outH)*scale) );
				break;

			case Settings::SizeMode::SetWidthAndHeight:
				outW = width;
				outH = height;
				break;

			case Settings::SizeMode::SetWidthRetainAspect:
				outW = width;
				outH = int( tRound(float(width) / aspect) );
				break;

			case Settings::SizeMode::SetHeightRetainAspect:
				outH = height;
				outW = int( tRound(float(height) * aspect) );
				break;
		};
		tMath::tiClampMin(outW, 4);
		tMath::tiClampMin(outH, 4);

		if ((outPic.GetWidth() != outW) || (outPic.GetHeight() != outH))
			outPic.Resample(outW, outH, tImage::tPicture::tFilter(Config.ResampleFilter));

		bool success = false;
		tImage::tPicture::tColourFormat colourFmt = outPic.IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
		if (Config.SaveFileType == 0)
			success = outPic.SaveTGA(outFile, tImage::tImageTGA::tFormat::Auto, Config.SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
		else
			success = outPic.Save(outFile, colourFmt, Config.SaveFileJpgQuality);

		if (success)
			tPrintf("Saved image as %s\n", outFile.Chars());
		else
			tPrintf("Failed to save image %s\n", outFile.Chars());
	}

	// If we saved to the same dir we are currently viewing we need to
	// reload and set the current image again.
	if (ImagesDir.IsEqualCI(destDir))
	{
		Images.Clear();
		PopulateImages();
		SetCurrentImage(currFile);
	}
}


void TexView::SaveImageTo(tPicture* picture, const tString& outFile, int width, int height)
{
	tAssert(picture);

	// We need to make a copy in case we need to resample.
	tImage::tPicture outPic( *picture );

	if ((outPic.GetWidth() != width) || (outPic.GetHeight() != height))
		outPic.Resample(width, height, tImage::tPicture::tFilter(Config.ResampleFilter));

	bool success = false;
	tImage::tPicture::tColourFormat colourFmt = outPic.IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
	if (Config.SaveFileType == 0)
		success = outPic.SaveTGA(outFile, tImage::tImageTGA::tFormat::Auto, Config.SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
	else
		success = outPic.Save(outFile, colourFmt, Config.SaveFileJpgQuality);
	if (success)
		tPrintf("Saved image as : %s\n", outFile.Chars());
	else
		tPrintf("Failed to save image %s\n", outFile.Chars());

	// If we saved to the same dir we are currently viewing, reload
	// and set the current image to the generated one.
	if (ImagesDir.IsEqualCI( tGetDir(outFile) ))
	{
		Images.Clear();
		PopulateImages();
		SetCurrentImage(outFile);
	}
}


void TexView::DoOverwriteFileModal(const tString& outFile, bool& pressedOK, bool& pressedCancel)
{
	tString file = tSystem::tGetFileName(outFile);
	tString dir = tSystem::tGetDir(outFile);
	ImGui::Text("Overwrite file");
		ImGui::Indent(); ImGui::Text("%s", file.Chars()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chars()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config.ConfirmFileOverwrites);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
	{
		pressedCancel = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("OK", tVector2(100, 0)))
	{
		pressedOK = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}
