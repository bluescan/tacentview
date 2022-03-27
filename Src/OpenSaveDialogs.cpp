// OpenSaveDialogs.cpp
//
// Modal dialogs open-file, open-dir, save-as and save-all.
//
// Copyright (c) 2019, 2020, 2021, 2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "imgui.h"
#include "OpenSaveDialogs.h"
#include "Image.h"
#include "TacentView.h"
#include "FileDialog.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;
#ifdef ENABLE_FILE_DIALOG_SUPPORT
using namespace tInterface;
#endif


namespace Viewer
{
	void SaveAllImages(const tString& destDir, const tString& extension, float percent, int width, int height);
	void GetFilesNeedingOverwrite(const tString& destDir, tList<tStringItem>& overwriteFiles, const tString& extension);
	void AddSavedImageIfNecessary(const tString& savedFile);

	// This function saves the picture to the filename specified.
	bool SaveImageAs(Image&, const tString& outFile);
	bool SaveResizeImageAs(Image&, const tString& outFile, int width, int height, float scale = 1.0f, Config::Settings::SizeMode = Config::Settings::SizeMode::SetWidthAndHeight);
}


// Once we have the file dialog working, use ENABLE_FILE_DIALOG_SUPPORT to enable it.
// We will want to have the following types supported:
#ifdef ENABLE_FILE_DIALOG_SUPPORT
FileDialog OpenFileDialog(FileDialog::DialogMode::OpenFile);
FileDialog OpenDirDialog(FileDialog::DialogMode::OpenDir);
FileDialog SaveFileDialog(FileDialog::DialogMode::SaveFile);


void Viewer::DoOpenFileModal(bool openFilePressed)
{
	if (openFilePressed)
		OpenFileDialog.OpenPopup();

	FileDialog::DialogResult result = OpenFileDialog.DoPopup();
	if (result == FileDialog::DialogResult::OK)
	{
		tString chosenFile = OpenFileDialog.GetResult();
		tPrintf("Opening file: %s\n", chosenFile.Chars());
		ImageFileParam.Param = chosenFile;
		PopulateImages();
		SetCurrentImage(chosenFile);
		SetWindowTitle();
	}
}


void Viewer::DoOpenDirModal(bool openDirPressed)
{
	if (openDirPressed)
		OpenDirDialog.OpenPopup();

	FileDialog::DialogResult result = OpenDirDialog.DoPopup();
	if (result == FileDialog::DialogResult::OK)
	{
		tString chosenDir = OpenDirDialog.GetResult();
		tPrintf("Opening dir: %s\n", chosenDir.Chars());
		ImageFileParam.Param = chosenDir + "dummyfile.txt";
		PopulateImages();
		SetCurrentImage();
		SetWindowTitle();
	}
}
#endif // ENABLE_FILE_DIALOG_SUPPORT


tString Viewer::DoSubFolder()
{
	// Output sub-folder
	char subFolder[256]; tMemset(subFolder, 0, 256);
	tStrncpy(subFolder, Config::Current->SaveSubFolder.Chars(), 255);
	ImGui::InputText("SubFolder", subFolder, 256);
	Config::Current->SaveSubFolder.Set(subFolder);
	tString destDir = ImagesDir;
	if (!Config::Current->SaveSubFolder.IsEmpty())
		destDir += Config::Current->SaveSubFolder + "/";
	tString toolTipText;
	tsPrintf(toolTipText, "Save to %s", destDir.Chars());
	ShowToolTip(toolTipText.Chars());
	ImGui::SameLine();
	if (ImGui::Button("Default"))
		Config::Current->SaveSubFolder.Set("Saved");
	ImGui::SameLine();
	if (ImGui::Button("Here"))
		Config::Current->SaveSubFolder.Clear();

	return destDir;
}


tString Viewer::DoSaveFiletype()
{
	const char* fileTypeItems[] = { "tga", "png", "bmp", "jpg", "webp", "gif", "apng", "tiff" };
	ImGui::Combo("File Type", &Config::Current->SaveFileType, fileTypeItems, tNumElements(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Output image format.\nAlpha supported by tga, png, apng, bmp, tiff, and webp.\nAnimation supported by webp, gif, tiff, and apng.");

	tString extension = ".tga";
	switch (Config::Current->SaveFileType)
	{
		case 0: extension = ".tga";  break;
		case 1: extension = ".png";  break;
		case 2: extension = ".bmp";  break;
		case 3: extension = ".jpg";  break;
		case 4: extension = ".webp"; break;
		case 5: extension = ".gif";  break;
		case 6: extension = ".apng"; break;
		case 7: extension = ".tiff"; break;
	}

	// There are different options depending on what type you are saving as.
	switch (Config::Current->SaveFileType)
	{
		case 0:
			ImGui::Checkbox("RLE Compression", &Config::Current->SaveFileTargaRLE);
			break;

		case 3:
			ImGui::SliderInt("Quality", &Config::Current->SaveFileJpegQuality, 1, 100, "%d");
			break;

		case 4:
			ImGui::Checkbox("Lossy", &Config::Current->SaveFileWebpLossy);
			ImGui::SliderFloat("Quality / Compression", &Config::Current->SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
			ImGui::SameLine(); ShowToolTip("Image quality percent if lossy. Image compression strength if not lossy"); ImGui::NewLine();
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileWebpDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileWebpDurOverride = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileWebpDurOverride = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileWebpDurOverride = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileWebpDurOverride = 16;
			break;

		case 5:
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileGifDurOverride, -1, 1000, "%d");
			ImGui::SameLine(); ShowToolTip("In 1/100 seconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileGifDurOverride = 100; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileGifDurOverride = 50;  ImGui::SameLine();
			if (ImGui::Button("15fps")) Config::Current->SaveFileGifDurOverride = 6;   ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileGifDurOverride = 3;
			break;

		case 6:
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileApngDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileApngDurOverride = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileApngDurOverride = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileApngDurOverride = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileApngDurOverride = 16;
			break;

		case 7:
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileTiffDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileTiffDurOverride = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileTiffDurOverride = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileTiffDurOverride = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileTiffDurOverride = 16;
			break;
	}

	return extension;
}


tString Viewer::DoSaveFiletypeMultiFrame()
{
	const char* fileTypeItems[] = { "webp", "gif", "apng", "tiff" };
	ImGui::Combo("File Type", &Config::Current->SaveFileTypeMultiFrame, fileTypeItems, tNumElements(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Multi-frame output image format.");

	// There are different options depending on what type you are saving as.
	tString extension = ".webp";
	switch (Config::Current->SaveFileTypeMultiFrame)
	{
		case 0:
			extension = ".webp";
			ImGui::Checkbox("Lossy", &Config::Current->SaveFileWebpLossy);
			ImGui::SliderFloat("Quality / Compression", &Config::Current->SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
			ImGui::SameLine(); ShowToolTip("Image quality percent if lossy. Image compression strength if not lossy"); ImGui::NewLine();
			ImGui::SliderInt("Frame Duration", &Config::Current->SaveFileWebpDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileWebpDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileWebpDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileWebpDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileWebpDurMultiFrame = 16;
			break;

		case 1:
			extension = ".gif";
			ImGui::SliderInt("Frame Duration", &Config::Current->SaveFileGifDurMultiFrame, 0, 1000, "%d");
			ImGui::SameLine(); ShowToolTip("In 1/100 seconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileGifDurMultiFrame = 100; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileGifDurMultiFrame = 50;  ImGui::SameLine();
			if (ImGui::Button("15fps")) Config::Current->SaveFileGifDurMultiFrame = 6;   ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileGifDurMultiFrame = 3;   ImGui::SameLine();
			if (ImGui::Button("50fps")) Config::Current->SaveFileGifDurMultiFrame = 2;
			break;

		case 2:
			extension = ".apng";
			ImGui::SliderInt("Frame Duration", &Config::Current->SaveFileApngDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileApngDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileApngDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileApngDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileApngDurMultiFrame = 16;
			break;

		case 3:
			extension = ".tiff";
			ImGui::SliderInt("Frame Duration", &Config::Current->SaveFileTiffDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileTiffDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileTiffDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileTiffDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileTiffDurMultiFrame = 16;
			break;
	}

	return extension;
}


bool Viewer::SaveImageAs(Image& img, const tString& outFile)
{
	// We make sure to maintain the loaded/unloaded state. This function may be called many times in succession
	// so we don't want them all in memory at once by indiscriminantly loading them all.
	bool imageLoaded = img.IsLoaded();
	if (!imageLoaded)
		img.Load();

	bool success = false;
	switch (Config::Current->SaveFileType)
	{
		case 0:		// TGA
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;
			success = picture->SaveTGA(outFile, tImage::tImageTGA::tFormat::Auto, Config::Current->SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
			break;
		}

		case 4:		// WEBP
		{
			tList<tFrame> frames;
			tList<tImage::tPicture>& pics = img.GetPictures();
			for (tPicture* picture = pics.First(); picture; picture = picture->Next())
			{
				frames.Append
				(
					new tFrame
					(
						picture->GetPixelPointer(),
						picture->GetWidth(),
						picture->GetHeight(),
						picture->Duration
					)
				);
			}

			tImageWEBP webp(frames, true);
			success = webp.Save(outFile, Config::Current->SaveFileWebpLossy, Config::Current->SaveFileWebpQualComp, Config::Current->SaveFileWebpDurOverride);
			break;
		}

		case 5:		// GIF
		{
			tList<tFrame> frames;
			tList<tImage::tPicture>& pics = img.GetPictures();
			for (tPicture* picture = pics.First(); picture; picture = picture->Next())
			{
				frames.Append
				(
					new tFrame
					(
						picture->GetPixelPointer(),
						picture->GetWidth(),
						picture->GetHeight(),
						picture->Duration
					)
				);
			}

			tImageGIF gif(frames, true);
			success = gif.Save(outFile, Config::Current->SaveFileGifDurOverride);
			break;
		}

		case 6:		// APNG
		{
			tList<tFrame> frames;
			tList<tImage::tPicture>& pics = img.GetPictures();
			for (tPicture* picture = pics.First(); picture; picture = picture->Next())
			{
				frames.Append
				(
					new tFrame
					(
						picture->GetPixelPointer(),
						picture->GetWidth(),
						picture->GetHeight(),
						picture->Duration
					)
				);
			}

			tImageAPNG apng(frames, true);
			success = apng.Save(outFile, Config::Current->SaveFileApngDurOverride);
			break;
		}

		case 7:		// TIFF
		{
			tList<tFrame> frames;
			tList<tImage::tPicture>& pics = img.GetPictures();
			for (tPicture* picture = pics.First(); picture; picture = picture->Next())
			{
				frames.Append
				(
					new tFrame
					(
						picture->GetPixelPointer(),
						picture->GetWidth(),
						picture->GetHeight(),
						picture->Duration
					)
				);
			}

			tImageTIFF tiff(frames, true);
			success = tiff.Save(outFile, Config::Current->SaveFileTiffZLibDeflate, Config::Current->SaveFileTiffDurOverride);
			break;
		}

		default:	// BMP, PNG, JPG etc.
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;
			tImage::tPicture::tColourFormat colourFmt = picture->IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
			success = picture->Save(outFile, colourFmt, Config::Current->SaveFileJpegQuality);
			break;
		}
	}

	if (success)
		tPrintf("Saved image as %s\n", outFile.Chars());
	else
		tPrintf("Failed to save image %s\n", outFile.Chars());

	return success;
}


bool Viewer::SaveResizeImageAs(Image& img, const tString& outFile, int width, int height, float scale, Config::Settings::SizeMode sizeMode)
{
	// We make sure to maintain the loaded/unloaded state. This function may be called many times in succession
	// so we don't want them all in memory at once by indiscriminantly loading them all.
	bool imageLoaded = img.IsLoaded();
	if (!imageLoaded)
		img.Load();

	tPicture* currPic = img.GetCurrentPic();
	if (!currPic)
		return false;

	// Make a temp copy we can safely resize.
	tImage::tPicture outPic;
	outPic.Set(*currPic);

	// Restore loadedness.
	if (!imageLoaded)
		img.Unload();

	int outW = outPic.GetWidth();
	int outH = outPic.GetHeight();
	float aspect = float(outW) / float(outH);

	switch (sizeMode)
	{
		case Config::Settings::SizeMode::Percent:
			if (tMath::tApproxEqual(scale, 1.0f, 0.01f))
				break;
			outW = int( tRound(float(outW)*scale) );
			outH = int( tRound(float(outH)*scale) );
			break;

		case Config::Settings::SizeMode::SetWidthAndHeight:
			outW = width;
			outH = height;
			break;

		case Config::Settings::SizeMode::SetWidthRetainAspect:
			outW = width;
			outH = int( tRound(float(width) / aspect) );
			break;

		case Config::Settings::SizeMode::SetHeightRetainAspect:
			outH = height;
			outW = int( tRound(float(height) * aspect) );
			break;
	};
	tMath::tiClampMin(outW, 4);
	tMath::tiClampMin(outH, 4);

	if ((outPic.GetWidth() != outW) || (outPic.GetHeight() != outH))
		outPic.Resample(outW, outH, tImage::tResampleFilter(Config::Current->ResampleFilter), tImage::tResampleEdgeMode(Config::Current->ResampleEdgeMode));

	bool success = false;
	tImage::tPicture::tColourFormat colourFmt = outPic.IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
	if (Config::Current->SaveFileType == 0)
		success = outPic.SaveTGA(outFile, tImage::tImageTGA::tFormat::Auto, Config::Current->SaveFileTargaRLE ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None);
	else
		success = outPic.Save(outFile, colourFmt, Config::Current->SaveFileJpegQuality);

	if (success)
		tPrintf("Saved image as %s\n", outFile.Chars());
	else
		tPrintf("Failed to save image %s\n", outFile.Chars());

	return success;
}


void Viewer::DoSaveAsModal(bool saveAsPressed)
{
	if (saveAsPressed)
		ImGui::OpenPopup("Save As");

	// The unused isOpenSaveAs bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenSaveAs = true;
	if (!ImGui::BeginPopupModal("Save As", &isOpenSaveAs, ImGuiWindowFlags_AlwaysAutoResize))
		return;
	
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);

	tString extension = DoSaveFiletype();
	ImGui::Separator();
	tString destDir = DoSubFolder();

	static char filename[128] = "Filename";
	if (saveAsPressed)
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
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

		if (dirExists)
		{
			if (tFileExists(outFile) && Config::Current->ConfirmFileOverwrites)
			{
				ImGui::OpenPopup("Overwrite File");
			}
			else
			{
				bool ok = SaveImageAs(*CurrImage, outFile);
				if (ok)
				{
					// This gets a bit tricky. Image A may be saved as the same name as image B also in the list. We need to search for it.
					// If it's not found, we need to add it to the list iff it was saved to the current folder.
					Image* foundImage = FindImage(outFile);
					if (foundImage)
					{
						foundImage->Unload(true);
						foundImage->ClearDirty();
						foundImage->RequestInvalidateThumbnail();
					}
					else
						AddSavedImageIfNecessary(outFile);

					SortImages(Config::Settings::SortKeyEnum(Config::Current->SortKey), Config::Current->SortAscending);
					SetCurrentImage(outFile);
				}
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
		{
			bool ok = SaveImageAs(*CurrImage, outFile);
			if (ok)
			{
				Image* foundImage = FindImage(outFile);
				if (foundImage)
				{
					foundImage->Unload(true);
					foundImage->ClearDirty();
					foundImage->RequestInvalidateThumbnail();
				}
				else
					AddSavedImageIfNecessary(outFile);

				SortImages(Config::Settings::SortKeyEnum(Config::Current->SortKey), Config::Current->SortAscending);
				SetCurrentImage(outFile);
			}
		}
		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::DoSaveAllModal(bool saveAllPressed)
{
	if (saveAllPressed)
		ImGui::OpenPopup("Save All");

	// The unused isOpenSaveAll bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenSaveAll = true;
	if (!ImGui::BeginPopupModal("Save All", &isOpenSaveAll, ImGuiWindowFlags_AlwaysAutoResize))
		return;

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
	ImGui::Combo("Size Mode", &Config::Current->SaveAllSizeMode, sizeModeNames, tNumElements(sizeModeNames));
	switch (Config::Settings::SizeMode(Config::Current->SaveAllSizeMode))
	{
		case Config::Settings::SizeMode::Percent:
			ImGui::InputFloat("Percent", &percent, 1.0f, 10.0f, "%.1f");	ImGui::SameLine();	ShowHelpMark("Percent of original size.");
			break;

		case Config::Settings::SizeMode::SetWidthAndHeight:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;

		case Config::Settings::SizeMode::SetWidthRetainAspect:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			break;

		case Config::Settings::SizeMode::SetHeightRetainAspect:
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;
	};

	ImGui::Separator();
	if (!((Config::Settings::SizeMode(Config::Current->SaveAllSizeMode) == Config::Settings::SizeMode::Percent) && (percent == 100.0f)))
	{
		ImGui::Combo("Filter", &Config::Current->ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");

		ImGui::Combo("Filter Edge Mode", &Config::Current->ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
		ImGui::SameLine();
		ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");
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

	// This needs to be static since DoSaveModal is called for every frame the modal is open.
	static tList<tStringItem> overwriteFiles(tListMode::Static);
	bool closeThisModal = false;
	if (ImGui::Button("Save All", tVector2(100, 0)))
	{
		bool dirExists = tDirExists(destDir);
		if (!dirExists)
		{
			dirExists = tCreateDir(destDir);
			PopulateImagesSubDirs();
		}

		if (dirExists)
		{
			overwriteFiles.Empty();
			GetFilesNeedingOverwrite(destDir, overwriteFiles, extension);
			if (!overwriteFiles.IsEmpty() && Config::Current->ConfirmFileOverwrites)
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


void Viewer::GetFilesNeedingOverwrite(const tString& destDir, tList<tStringItem>& overwriteFiles, const tString& extension)
{
	for (Image* image = Images.First(); image; image = image->Next())
	{
		tString baseName = tSystem::tGetFileBaseName(image->Filename);
		tString outFile = destDir + tString(baseName) + extension;

		// Only add unique items to the list.
		if (tSystem::tFileExists(outFile) && !overwriteFiles.Contains(outFile))
			overwriteFiles.Append(new tStringItem(outFile));
	}
}


void Viewer::DoOverwriteMultipleFilesModal(const tList<tStringItem>& overwriteFiles, bool& pressedOK, bool& pressedCancel)
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
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config::Current->ConfirmFileOverwrites);
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


void Viewer::SaveAllImages(const tString& destDir, const tString& extension, float percent, int width, int height)
{
	float scale = percent/100.0f;
	tString currFile = CurrImage ? CurrImage->Filename : tString();

	bool anySaved = false;
	for (Image* image = Images.First(); image; image = image->Next())
	{
		tString baseName = tSystem::tGetFileBaseName(image->Filename);
		tString outFile = destDir + tString(baseName) + extension;

		bool ok = SaveResizeImageAs(*image, outFile, width, height, scale, Config::Settings::SizeMode(Config::Current->SaveAllSizeMode));
		if (ok)
		{
			Image* foundImage = FindImage(outFile);
			if (foundImage)
			{
				foundImage->Unload(true);
				foundImage->ClearDirty();
				foundImage->RequestInvalidateThumbnail();
			}
			else
				AddSavedImageIfNecessary(outFile);
			anySaved = true;
		}
	}

	// If we saved to the same dir we are currently viewing we need to reload and set the current image again.
	if (anySaved)
	{
		SortImages(Config::Settings::SortKeyEnum(Config::Current->SortKey), Config::Current->SortAscending);
		SetCurrentImage(currFile);
	}
}


void Viewer::AddSavedImageIfNecessary(const tString& savedFile)
{
	#ifdef PLATFORM_LINUX
	if (ImagesDir.IsEqual(tGetDir(savedFile)))
	#else
	if (ImagesDir.IsEqualCI(tGetDir(savedFile)))
	#endif
	{
		// Add to list. It's still unloaded.
		Image* newImg = new Image(savedFile);
		Images.Append(newImg);
		ImagesLoadTimeSorted.Append(newImg);
	}
}


void Viewer::DoOverwriteFileModal(const tString& outFile, bool& pressedOK, bool& pressedCancel)
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
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config::Current->ConfirmFileOverwrites);
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
