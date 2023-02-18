// OpenSaveDialogs.cpp
//
// Modal dialogs open-file, open-dir, save-as and save-all.
//
// Copyright (c) 2019-2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Image/tImageTGA.h>
#include <Image/tImagePNG.h>
#include <Image/tImageJPG.h>
#include <Image/tImageGIF.h>
#include <Image/tImageWEBP.h>
#include <Image/tImageQOI.h>
#include <Image/tImageAPNG.h>
#include <Image/tImageBMP.h>
#include <Image/tImageTIFF.h>
#include "imgui.h"
#include "OpenSaveDialogs.h"
#include "Image.h"
#include "TacentView.h"
#include "FileDialog.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;
using namespace tFileDialog;


namespace Viewer
{
	void SaveAllImages(const tString& destDir, const tString& extension, float percent, int width, int height);
	void GetFilesNeedingOverwrite(const tString& destDir, tList<tStringItem>& overwriteFiles, const tString& extension);
	void AddSavedImageIfNecessary(const tString& savedFile);

	// This function saves the picture to the filename specified.
	bool SaveImageAs(Image&, const tString& outFile);
	bool SaveResizeImageAs(Image&, const tString& outFile, int width, int height, float scale = 1.0f, Config::ProfileSettings::SizeModeEnum = Config::ProfileSettings::SizeModeEnum::SetWidthAndHeight);
	void DoSavePopup();
	void DoSaveUnsupportedTypePopup();

	void DoSaveGifOptions(bool multiframeConfigValues);
	tString SaveAsFile;
}


void Viewer::DoOpenFileModal(bool openFilePressed)
{
	if (openFilePressed)
		OpenFileDialog.OpenPopup(ImagesDir);

	FileDialog::DialogState state = OpenFileDialog.DoPopup();
	if (state == FileDialog::DialogState::OK)
	{
		tString chosenFile = OpenFileDialog.GetResult();
		tPrintf("Opening file: %s\n", chosenFile.Chr());
		CurrImageFile = chosenFile;
		PopulateImages();
		SetCurrentImage(chosenFile);
		SetWindowTitle();
	}
}


void Viewer::DoOpenDirModal(bool openDirPressed)
{
	if (openDirPressed)
		OpenDirDialog.OpenPopup(ImagesDir);

	FileDialog::DialogState state = OpenDirDialog.DoPopup();
	if (state == FileDialog::DialogState::OK)
	{
		tString chosenDir = OpenDirDialog.GetResult();
		CurrImageFile = chosenDir + "dummyfile.txt";
		PopulateImages();
		SetCurrentImage();
		SetWindowTitle();
	}
}


void Viewer::DoSaveModal(bool savePressed)
{
	static tString label;
	if (!CurrImage)
		return;

	if (savePressed)
	{
		SaveAsFile = CurrImage->Filename;
		tString typeNameUpper(tGetFileTypeName(tGetFileType(SaveAsFile)));
		typeNameUpper.ToUpper();
		tsPrintf(label, "Save %s Options", typeNameUpper.Chr());
		ImGui::OpenPopup(label.Chr());
	}

	float nextWinWidth;
	switch (Viewer::Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:
			nextWinWidth = 300.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
			nextWinWidth = 325.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:
			nextWinWidth = 350.0f;
			break;
	}

	// The unused isOpenSaveOptions bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenSaveOptions = true;
	ImGui::SetNextWindowSize(tVector2(nextWinWidth, 0.0f));
	if (!ImGui::BeginPopupModal(label.Chr(), &isOpenSaveOptions, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tFileType saveType = tGetFileType(SaveAsFile);
	if (FileTypes_Save.Contains(saveType))
		DoSavePopup();
	else
		DoSaveUnsupportedTypePopup();

	ImGui::EndPopup();
}


void Viewer::DoSaveAsModal(bool saveAsPressed)
{
	static tString label;
	if (!CurrImage)
		return;

	if (saveAsPressed)
	{
		tString baseName;
		baseName = tSystem::tGetFileBaseName(CurrImage->Filename);
		SaveAsDialog.OpenPopup(ImagesDir, baseName);
	}
	FileDialog::DialogState state = SaveAsDialog.DoPopup();
	if (state == FileDialog::DialogState::OK)
	{
		SaveAsFile = SaveAsDialog.GetResult();
		tString typeNameUpper(tGetFileTypeName(tGetFileType(SaveAsFile)));
		typeNameUpper.ToUpper();
		tsPrintf(label, "Save As %s Options", typeNameUpper.Chr());
		ImGui::OpenPopup(label.Chr());
	}

	float nextWinWidth;
	switch (Viewer::Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:
			nextWinWidth = 300.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
			nextWinWidth = 325.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:
			nextWinWidth = 350.0f;
			break;
	}

	// The unused isOpenSaveOptions bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenSaveOptions = true;
	ImGui::SetNextWindowSize(tVector2(nextWinWidth, 0.0f));
	if (!ImGui::BeginPopupModal(label.Chr(), &isOpenSaveOptions, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	DoSavePopup();
	ImGui::EndPopup();
}


void Viewer::DoSavePopup()
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);

	// This gets the filetype from the filename. We then update the current config.
	tFileType saveType = tGetFileType(SaveAsFile);
	Config::Current->SaveFileType = tGetFileTypeName(saveType);

	DoSaveFiletypeOptions(saveType);

	ImGui::NewLine();
	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();
	
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	bool closeThisModal = false;

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("Save", tVector2(100.0f, 0.0f)))
	{
		if (tFileExists(SaveAsFile) && Config::Current->ConfirmFileOverwrites)
		{
			ImGui::OpenPopup("Overwrite File");
		}
		else
		{
			bool ok = SaveImageAs(*CurrImage, SaveAsFile);
			if (ok)
			{
				// This gets a bit tricky. Image A may be saved as the same name as image B also in the list. We need to search for it.
				// If it's not found, we need to add it to the list iff it was saved to the current folder.
				Image* foundImage = FindImage(SaveAsFile);
				if (foundImage)
				{
					foundImage->Unload(true);
					foundImage->ClearDirty();
					foundImage->RequestInvalidateThumbnail();
				}
				else
					AddSavedImageIfNecessary(SaveAsFile);

				SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);
				SetCurrentImage(SaveAsFile);
			}
			closeThisModal = true;
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Overwrite File", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool pressedOK = false, pressedCancel = false;
		DoOverwriteFileModal(SaveAsFile, pressedOK, pressedCancel);
		if (pressedOK)
		{
			bool ok = SaveImageAs(*CurrImage, SaveAsFile);
			if (ok)
			{
				Image* foundImage = FindImage(SaveAsFile);
				if (foundImage)
				{
					foundImage->Unload(true);
					foundImage->ClearDirty();
					foundImage->RequestInvalidateThumbnail();
				}
				else
					AddSavedImageIfNecessary(SaveAsFile);

				SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);
				SetCurrentImage(SaveAsFile);
			}
		}
		if (pressedOK || pressedCancel)
			closeThisModal = true;
	}

	if (closeThisModal)
		ImGui::CloseCurrentPopup();
}


void Viewer::DoSaveUnsupportedTypePopup()
{
	tString saveTypeName = tGetFileTypeName(tGetFileType(SaveAsFile));
	ImGui::Text("Saving of %s files is not supported.", saveTypeName.Chr());

	tString support;
	tsPrintf(support, "Tacent View supports:\n\n");

	int ln = 1;
	for (tFileTypes::tFileTypeItem* i = FileTypes_Save.First(); i; i = i->Next())
	{
		tString line;
		tsPrintf(line, "%s ", tGetFileTypeName(i->FileType).Chr());
		if ((ln++ % 5) == 0)
			line += "\n";
		support += line;
	}

	ImGui::Text("%s", support.Chr());
	ImGui::NewLine();
	
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	bool closeThisModal = false;

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();

	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();
}


tString Viewer::DoSubFolder()
{
	// Output sub-folder
	char subFolder[256]; tMemset(subFolder, 0, 256);
	tStrncpy(subFolder, Config::Current->SaveSubFolder.Chr(), 255);
	ImGui::InputText("SubFolder", subFolder, 256);
	Config::Current->SaveSubFolder.Set(subFolder);
	tString destDir = ImagesDir;
	if (!Config::Current->SaveSubFolder.IsEmpty())
		destDir += Config::Current->SaveSubFolder + "/";
	tString toolTipText;
	tsPrintf(toolTipText, "Save to %s", destDir.Chr());
	ShowToolTip(toolTipText.Chr());
	ImGui::SameLine();
	if (ImGui::Button("Default"))
		Config::Current->SaveSubFolder.Set("Saved");
	ImGui::SameLine();
	if (ImGui::Button("Here"))
		Config::Current->SaveSubFolder.Clear();

	return destDir;
}


tSystem::tFileType Viewer::DoSaveChooseFiletype()
{
	tString fileTypeName = Config::Current->SaveFileType;
	tFileType fileType = tGetFileTypeFromName(fileTypeName);
	if (ImGui::BeginCombo("File Type", fileTypeName.Chr()))
	{
		for (tFileTypes::tFileTypeItem* item = FileTypes_Save.First(); item; item = item->Next())
		{
			tFileType ft = item->FileType;
			bool selected = (ft == fileType);

			tString ftName = tGetFileTypeName(ft);
			if (ImGui::Selectable(ftName.Chr(), &selected))
				Config::Current->SaveFileType = ftName;

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ShowHelpMark("Output image format.\nFull (non-binary) alpha supported by tga, png, apng, bmp, tiff, qoi, and webp.\nAnimation supported by webp, gif, tiff, and apng.");

	return tGetFileTypeFromName(Config::Current->SaveFileType);
}


void Viewer::DoSaveGifOptions(bool multiframeConfigValues)
{
	const int itemWidth = 160;
	if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
	ImGui::SliderInt("Bits per Pixel", &Config::Current->SaveFileGifBPP, 1, 8, "%d");
	ImGui::SameLine();
	ShowHelpMark
	(
		"Determines how many colours the gif uses. From 2 to 256.\n"
		"Note that using transparency takes one colour away."
	);
	
	if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
	const char* methodItems[] = { "Fixed Palette", "Spatial Scolorq", "Neu Quant", "Wu Bipartition" };
	ImGui::Combo("Quantize", &Config::Current->SaveFileGifQuantMethod , methodItems, tNumElements(methodItems));
	ImGui::SameLine();
	ShowHelpMark
	(
		"The colour quantization method is used to create a high-quality palette,\n\n"

		"Fixed Palette: Lowest quality since it doesn't inspect the image pixels. It\n"
		"is useful for 1-bit palettes as it guarantees black and white for this size.\n\n"

		"Spatial Scolorq: High quality but slow. Good for 5-bit palettes and smaller.\n"
		"This is the only quantization method supporting dither.\n\n"

		"Neu Quant: Defacto high-quality quantizer. Neural-network based.\n"
		"Good for 6-bit palettes and larger.\n\n"

		"Wu Bipartition: The default quantizer. Fast and looks very good at 5-bit and up."
	);

	if (Config::Current->SaveFileGifQuantMethod == int(tQuantize::Method::Spatial))
	{
		if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
		const char* filterSizeItems[] = { "Low", "Med", "High" };
		ImGui::Combo("Quantize Filter Size", &Config::Current->SaveFileGifFilterSize , filterSizeItems, tNumElements(filterSizeItems));
		ImGui::SameLine();
		ShowHelpMark("Filter size for the scolorq/spatial quantizer. Low -> 1, Med -> 3, High -> 5");

		if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
 		ImGui::SliderFloat("Quantize Dither", &Config::Current->SaveFileGifDitherLevel, 0.0f, 2.0f, "%.1f");
		ImGui::SameLine();
		ShowHelpMark
		(
			"Dither level for the scolorq/spatial quantizer. 0 means auto-determine a good value for\n"
			"the current image based on its dimensions. Greater than zero means manually set the\n"
			"amount. A dither value of 0.1 results in no dithering. 2.0 results in significant dithering."
		);
	}

	if (Config::Current->SaveFileGifQuantMethod == int(tQuantize::Method::Neu))
	{
		if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
		ImGui::SliderInt("Quantize Factor", &Config::Current->SaveFileGifSampleFactor, 1, 10, "%d");
		ImGui::SameLine();
		ShowHelpMark
		(
			"Sample factor for the Neu quantizer. 1 is highest quality and slowest. All pixels are visited\n"
			"during the learning step. 10 means only 1/10th of the image is considered during learning."
		);
	}

	if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Alpha Threshold", &Config::Current->SaveFileGifAlphaThreshold);
	tiClamp(Config::Current->SaveFileGifAlphaThreshold, -1, 255);
	ImGui::SameLine(); ShowHelpMark
	(
		"When set to -1 this will force the gif to be opaque. When between 0 and 255 it will use\n"
		"this value to determine if the pixel is transparent or not. A pixel alpha less-than or\n"
		"equal means the pixel will be transparent. Larger means opaque. Gif only supports binary alpha."
	);

	if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
	if (!multiframeConfigValues)
	{
		ImGui::InputInt("Duration Override", &Config::Current->SaveFileGifDurOverride);
		tiClamp(Config::Current->SaveFileGifAlphaThreshold, -1, 1000);
		ImGui::SameLine(); ShowHelpMark("In 1/100 seconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
		if (Viewer::Button("1.0s"))  Config::Current->SaveFileGifDurOverride = 100; ImGui::SameLine();
		if (Viewer::Button("0.5s"))  Config::Current->SaveFileGifDurOverride = 50;  ImGui::SameLine();
		if (Viewer::Button("15fps")) Config::Current->SaveFileGifDurOverride = 6;   ImGui::SameLine();
		if (Viewer::Button("30fps")) Config::Current->SaveFileGifDurOverride = 3;
	}
	else
	{
		ImGui::InputInt("Frame Duration", &Config::Current->SaveFileGifDurMultiFrame);
		tiClamp(Config::Current->SaveFileGifDurMultiFrame, 0, 1000);
		ImGui::SameLine(); ShowHelpMark("In 1/100 seconds.");
		if (Viewer::Button("1.0s"))  Config::Current->SaveFileGifDurMultiFrame = 100; ImGui::SameLine();
		if (Viewer::Button("0.5s"))  Config::Current->SaveFileGifDurMultiFrame = 50;  ImGui::SameLine();
		if (Viewer::Button("15fps")) Config::Current->SaveFileGifDurMultiFrame = 6;   ImGui::SameLine();
		if (Viewer::Button("30fps")) Config::Current->SaveFileGifDurMultiFrame = 3;   ImGui::SameLine();
		if (Viewer::Button("50fps")) Config::Current->SaveFileGifDurMultiFrame = 2;
	}

	if (!multiframeConfigValues) ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Loop", &Config::Current->SaveFileGifLoop);
	tiClampMin(Config::Current->SaveFileGifLoop, 0);
	ImGui::SameLine();
	ShowHelpMark("How many times an animated gif will loop. 0 means forever.\nThis only applies to animated (multi-frame) images.");

	bool isOpaque = (Config::Current->SaveFileGifAlphaThreshold == -1);
	tString loopDesc = "forever";
	if (Config::Current->SaveFileGifLoop != 0)
	{
		if (Config::Current->SaveFileGifLoop == 1)
			tsPrintf(loopDesc, "1 time");
		else
			tsPrintf(loopDesc, "%d times", Config::Current->SaveFileGifLoop);
	}
	tString desc; tsPrintf
	(
		desc, " A %d colour %s GIF. If\n animated, loops %s.",
		(1 << Config::Current->SaveFileGifBPP) - (isOpaque ? 0 : 1),
		isOpaque ? "opaque" : "transparent",
		loopDesc.Chr()
	);
	if ((Config::Current->SaveFileGifQuantMethod == int(tQuantize::Method::Spatial)) && (Config::Current->SaveFileGifBPP > 5))
		desc += "\n\n WARNING: Spatial quantization\n at large BPPs may take a long\n time for large images.";

	ImGui::Text(desc.Chr());
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

	if (Viewer::Button("Reset", tVector2(100.0f, 0.0f)))
	{
		Config::Current->SaveFileGifBPP				= 8;
		Config::Current->SaveFileGifQuantMethod		= int(tImage::tQuantize::Method::Wu);
		Config::Current->SaveFileGifLoop			= 0;
		Config::Current->SaveFileGifAlphaThreshold	= -1;
		Config::Current->SaveFileGifDitherLevel		= 0.0f;
		Config::Current->SaveFileGifFilterSize		= 1;
		Config::Current->SaveFileGifSampleFactor	= 1;
		Config::Current->SaveFileWebpDurOverride	= -1;
		Config::Current->SaveFileGifDurOverride		= -1;
		Config::Current->SaveFileGifDurMultiFrame	= 3;
	}
}


void Viewer::DoSaveFiletypeOptions(tFileType fileType)
{
	switch (fileType)
	{
		case tFileType::TGA:
			ImGui::Checkbox("RLE Compression", &Config::Current->SaveFileTargaRLE);
			break;

		case tFileType::PNG:
		{
			const char* pngModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::SetNextItemWidth(80);
			ImGui::Combo("Bits Per Pixel", &Config::Current->SaveFilePngDepthMode , pngModeItems, tNumElements(pngModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");
			break;
		}

		case tFileType::JPG:
			ImGui::SetNextItemWidth(166);
			ImGui::SliderInt("Quality", &Config::Current->SaveFileJpegQuality, 1, 100, "%d");
			break;

		case tFileType::GIF:
			DoSaveGifOptions(false);
			break;

		case tFileType::WEBP:
			ImGui::Checkbox("Lossy", &Config::Current->SaveFileWebpLossy);
			ImGui::SetNextItemWidth(166);
			if (Config::Current->SaveFileWebpLossy)
			{
				ImGui::SliderFloat("Quality", &Config::Current->SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
				ImGui::SameLine(); ShowHelpMark("Lossy selected. This is the image quality percent.");
			}
			else
			{
				ImGui::SliderFloat("Compression", &Config::Current->SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
				ImGui::SameLine(); ShowHelpMark("Non-lossy selected. This is the image compression strength.\nBigger is slower and yields smaller files.");
			}
			ImGui::SetNextItemWidth(166);
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileWebpDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowHelpMark("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
			if (Viewer::Button("1.0s"))  Config::Current->SaveFileWebpDurOverride = 1000; ImGui::SameLine();
			if (Viewer::Button("0.5s"))  Config::Current->SaveFileWebpDurOverride = 500;  ImGui::SameLine();
			if (Viewer::Button("30fps")) Config::Current->SaveFileWebpDurOverride = 33;   ImGui::SameLine();
			if (Viewer::Button("60fps")) Config::Current->SaveFileWebpDurOverride = 16;
			break;

		case tFileType::QOI:
		{
			const char* qoiModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::SetNextItemWidth(80);
			ImGui::Combo("Bits Per Pixel", &Config::Current->SaveFileQoiDepthMode , qoiModeItems, tNumElements(qoiModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");

			const char* qoiSpaceItems[] = { "sRGB", "Linear" };
			ImGui::SetNextItemWidth(80);
			ImGui::Combo("Colour Space", &Config::Current->SaveFileQoiColourSpace , qoiSpaceItems, tNumElements(qoiSpaceItems));
			ImGui::SameLine();
			ShowHelpMark("Colour space stored in file. sRGB: The default for most images. Linear: Used when doing lighting calculations.");

			break;
		}

		case tFileType::APNG:
			ImGui::SetNextItemWidth(166);
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileApngDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowHelpMark("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
			if (Viewer::Button("1.0s"))  Config::Current->SaveFileApngDurOverride = 1000; ImGui::SameLine();
			if (Viewer::Button("0.5s"))  Config::Current->SaveFileApngDurOverride = 500;  ImGui::SameLine();
			if (Viewer::Button("30fps")) Config::Current->SaveFileApngDurOverride = 33;   ImGui::SameLine();
			if (Viewer::Button("60fps")) Config::Current->SaveFileApngDurOverride = 16;
			break;

		case tFileType::BMP:
		{
			const char* bmpModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::SetNextItemWidth(80);
			ImGui::Combo("Bits Per Pixel", &Config::Current->SaveFileBmpDepthMode , bmpModeItems, tNumElements(bmpModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");
			break;
		}

		case tFileType::TIFF:
			ImGui::SetNextItemWidth(166);
			ImGui::SliderInt("Duration Override", &Config::Current->SaveFileTiffDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowHelpMark("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
			if (Viewer::Button("1.0s"))  Config::Current->SaveFileTiffDurOverride = 1000; ImGui::SameLine();
			if (Viewer::Button("0.5s"))  Config::Current->SaveFileTiffDurOverride = 500;  ImGui::SameLine();
			if (Viewer::Button("30fps")) Config::Current->SaveFileTiffDurOverride = 33;   ImGui::SameLine();
			if (Viewer::Button("60fps")) Config::Current->SaveFileTiffDurOverride = 16;
			break;
	}
}


tString Viewer::DoSaveFiletypeMultiFrame()
{
	tString fileTypeName = Config::Current->SaveFileTypeMultiFrame;
	tFileType fileType = tGetFileTypeFromName(fileTypeName);
	if (ImGui::BeginCombo("File Type", fileTypeName.Chr()))
	{
		for (tFileTypes::tFileTypeItem* item = FileTypes_SaveMultiFrame.First(); item; item = item->Next())
		{
			tFileType ft = item->FileType;
			bool selected = (ft == fileType);

			tString ftName = tGetFileTypeName(ft);
			if (ImGui::Selectable(ftName.Chr(), &selected))
				Config::Current->SaveFileTypeMultiFrame = ftName;

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ShowHelpMark("Multi-frame output image format.");

	// There are different options depending on what type you are saving as.
	fileType = tGetFileTypeFromName(Config::Current->SaveFileTypeMultiFrame);
	tString extension = tGetExtension(fileType);
	switch (fileType)
	{
		case tFileType::GIF:
			DoSaveGifOptions(true);
			break;

		case tFileType::WEBP:
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

		case tFileType::APNG:
			ImGui::SliderInt("Frame Duration", &Config::Current->SaveFileApngDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  Config::Current->SaveFileApngDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  Config::Current->SaveFileApngDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) Config::Current->SaveFileApngDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) Config::Current->SaveFileApngDurMultiFrame = 16;
			break;

		case tFileType::TIFF:
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
	tFileType fileType = tGetFileTypeFromName( Config::Current->SaveFileType );

	bool success = SaveImageAs(img, outFile, fileType);
	if (success)
		tPrintf("Saved image as %s\n", outFile.Chr());
	else
		tPrintf("Failed to save image %s\n", outFile.Chr());

	return success;
}


bool Viewer::SaveResizeImageAs(Image& img, const tString& outFile, int width, int height, float scale, Config::ProfileSettings::SizeModeEnum sizeMode)
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
		case Config::ProfileSettings::SizeModeEnum::Percent:
			if (tMath::tApproxEqual(scale, 1.0f, 0.01f))
				break;
			outW = int( tRound(float(outW)*scale) );
			outH = int( tRound(float(outH)*scale) );
			break;

		case Config::ProfileSettings::SizeModeEnum::SetWidthAndHeight:
			outW = width;
			outH = height;
			break;

		case Config::ProfileSettings::SizeModeEnum::SetWidthRetainAspect:
			outW = width;
			outH = int( tRound(float(width) / aspect) );
			break;

		case Config::ProfileSettings::SizeModeEnum::SetHeightRetainAspect:
			outH = height;
			outW = int( tRound(float(height) * aspect) );
			break;
	};
	tMath::tiClampMin(outW, 4);
	tMath::tiClampMin(outH, 4);

	if ((outPic.GetWidth() != outW) || (outPic.GetHeight() != outH))
		outPic.Resample(outW, outH, tImage::tResampleFilter(Config::Current->ResampleFilter), tImage::tResampleEdgeMode(Config::Current->ResampleEdgeMode));

	tFileType saveFileType = tGetFileTypeFromName(Config::Current->SaveFileType);
	bool success = SavePictureAs(outPic, outFile, saveFileType, true);
	if (success)
		tPrintf("Saved image as %s\n", outFile.Chr());
	else
		tPrintf("Failed to save image %s\n", outFile.Chr());

	return success;
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
	switch (Config::Current->GetSaveAllSizeMode())
	{
		case Config::ProfileSettings::SizeModeEnum::Percent:
			ImGui::InputFloat("Percent", &percent, 1.0f, 10.0f, "%.1f");	ImGui::SameLine();	ShowHelpMark("Percent of original size.");
			break;

		case Config::ProfileSettings::SizeModeEnum::SetWidthAndHeight:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;

		case Config::ProfileSettings::SizeModeEnum::SetWidthRetainAspect:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			break;

		case Config::ProfileSettings::SizeModeEnum::SetHeightRetainAspect:
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;
	};

	ImGui::Separator();
	if (!((Config::Current->GetSaveAllSizeMode() == Config::ProfileSettings::SizeModeEnum::Percent) && (percent == 100.0f)))
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

	tFileType fileType = DoSaveChooseFiletype();
	DoSaveFiletypeOptions(fileType);
	tString extensionWithDot = tString(".") + tGetExtension(fileType);
	ImGui::Separator();
	tString destDir = DoSubFolder();

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	// This needs to be static since DoSaveModal is called for every frame the modal is open.
	static tList<tStringItem> overwriteFiles(tListMode::Static);
	bool closeThisModal = false;
	if (ImGui::Button("Save All", tVector2(100.0f, 0.0f)))
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
			GetFilesNeedingOverwrite(destDir, overwriteFiles, extensionWithDot);
			if (!overwriteFiles.IsEmpty() && Config::Current->ConfirmFileOverwrites)
			{
				ImGui::OpenPopup("Overwrite Multiple Files");
			}
			else
			{
				SaveAllImages(destDir, extensionWithDot, percent, width, height);
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
			SaveAllImages(destDir, extensionWithDot, percent, width, height);

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
		ImGui::Text("%s", file.Chr());
	}
	int remaining = overwriteFiles.GetNumItems() - fnum;
	if (remaining > 0)
		ImGui::Text("And %d more.", remaining);
	ImGui::Unindent();
	ImGui::Text("Already Exist In Folder");
	ImGui::Indent(); ImGui::Text("%s", dir.Chr()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Text("Overwrite Files?");
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config::Current->ConfirmFileOverwrites);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
	{
		pressedCancel = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Overwrite", tVector2(100.0f, 0.0f)))
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

		bool ok = SaveResizeImageAs(*image, outFile, width, height, scale, Config::Current->GetSaveAllSizeMode());
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
		SortImages(Config::Current->GetSortKey(), Config::Current->SortAscending);
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
		ImGui::Indent(); ImGui::Text("%s", file.Chr()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chr()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config::Current->ConfirmFileOverwrites);
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
	{
		pressedCancel = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
	{
		pressedOK = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


bool Viewer::SaveImageAs(Image& img, const tString& outFile, tFileType fileType)
{
	bool success = false;
	switch (fileType)
	{
		case tFileType::TGA:
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;
			tImageTGA tga(*picture, true);

			tImageTGA::tFormat savedFmt = tga.Save(outFile, tImageTGA::tFormat::Auto, Config::Current->SaveFileTargaRLE ? tImageTGA::tCompression::RLE : tImageTGA::tCompression::None);
			success = (savedFmt != tImageTGA::tFormat::Invalid);
			break;
		}

		case tFileType::PNG:
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;

			tImagePNG png(*picture, false);
			tImagePNG::tFormat saveFormat = tImagePNG::tFormat::Auto;
			switch (Config::Current->SaveFilePngDepthMode)
			{
				case 1: saveFormat = tImagePNG::tFormat::BPP24;		break;
				case 2: saveFormat = tImagePNG::tFormat::BPP32;		break;
			}
			tImagePNG::tFormat savedFmt = png.Save(outFile, saveFormat);
			success = (savedFmt != tImagePNG::tFormat::Invalid);
			break;
		}

		case tFileType::JPG:
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;

			tImageJPG jpg(*picture, false);
			success = jpg.Save(outFile, Config::Current->SaveFileJpegQuality);
			break;
		}

		case tFileType::GIF:
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
			tImageGIF::SaveParams params;
			params.Format = tPixelFormat(int(tPixelFormat::FirstPalette) + Config::Current->SaveFileGifBPP - 1);
			params.Method = tQuantize::Method(Config::Current->SaveFileGifQuantMethod);
			params.Loop = Config::Current->SaveFileGifLoop;
			params.AlphaThreshold = Config::Current->SaveFileGifAlphaThreshold;
			params.OverrideFrameDuration = Config::Current->SaveFileGifDurOverride;
			params.DitherLevel = double(Config::Current->SaveFileGifDitherLevel);
			params.FilterSize = (Config::Current->SaveFileGifFilterSize * 2) + 1;
			params.SampleFactor = Config::Current->SaveFileGifSampleFactor;
			success = gif.Save(outFile, params);
			break;
		}

		case tFileType::WEBP:
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

		case tFileType::QOI:
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;

			tImageQOI qoi(*picture, false);
			tImageQOI::tFormat saveFormat = tImageQOI::tFormat::Auto;
			switch (Config::Current->SaveFileQoiDepthMode)
			{
				case 1: saveFormat = tImageQOI::tFormat::BPP24;		break;
				case 2: saveFormat = tImageQOI::tFormat::BPP32;		break;
			}

			tImageQOI::tSpace saveSpace = tImageQOI::tSpace::sRGB;
			switch (Config::Current->SaveFileQoiColourSpace)
			{
				case 0: saveSpace = tImageQOI::tSpace::sRGB;		break;
				case 1: saveSpace = tImageQOI::tSpace::Linear;		break;
			}
			qoi.SetColourSpace(saveSpace);

			tImageQOI::tFormat savedFormat = qoi.Save(outFile, saveFormat);
			success = (savedFormat != tImageQOI::tFormat::Invalid);
			break;
		}

		case tFileType::APNG:
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

		case tFileType::BMP:
		{
			tPicture* picture = img.GetCurrentPic();
			if (!picture || !picture->IsValid())
				return false;

			tImageBMP bmp(*picture, false);
			tImageBMP::tFormat saveFormat = tImageBMP::tFormat::Auto;
			switch (Config::Current->SaveFileBmpDepthMode)
			{
				case 1: saveFormat = tImageBMP::tFormat::BPP24;		break;
				case 2: saveFormat = tImageBMP::tFormat::BPP32;		break;
			}
			tImageBMP::tFormat savedFormat = bmp.Save(outFile, saveFormat);
			success = (savedFormat != tImageBMP::tFormat::Invalid);
			break;
		}

		case tFileType::TIFF:
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
	}

	return success;
}


bool Viewer::SavePictureAs(tImage::tPicture& picture, const tString& outFile, tFileType fileType, bool steal)
{
	if (!picture.IsValid())
		return false;

	bool success = false;
	switch (fileType)
	{
		case tFileType::TGA:
		{
			tImageTGA tga(picture, steal);
			tImageTGA::tFormat savedFmt = tga.Save(outFile, tImageTGA::tFormat::Auto, Config::Current->SaveFileTargaRLE ? tImageTGA::tCompression::RLE : tImageTGA::tCompression::None);
			success = (savedFmt != tImageTGA::tFormat::Invalid);
			break;
		}

		case tFileType::PNG:
		{
			tImagePNG png(picture, steal);
			tImagePNG::tFormat saveFormat = tImagePNG::tFormat::Auto;
			switch (Config::Current->SaveFilePngDepthMode)
			{
				case 1: saveFormat = tImagePNG::tFormat::BPP24;		break;
				case 2: saveFormat = tImagePNG::tFormat::BPP32;		break;
			}
			tImagePNG::tFormat savedFmt = png.Save(outFile, saveFormat);
			success = (savedFmt != tImagePNG::tFormat::Invalid);
			break;
		}

		case tFileType::JPG:
		{
			tImageJPG jpg(picture, steal);
			success = jpg.Save(outFile, Config::Current->SaveFileJpegQuality);
			break;
		}

		case tFileType::GIF:
		{
			tImageGIF gif(picture, steal);
			tImageGIF::SaveParams params;
			params.Format = tPixelFormat(int(tPixelFormat::FirstPalette) + Config::Current->SaveFileGifBPP - 1);
			params.Method = tQuantize::Method(Config::Current->SaveFileGifQuantMethod);
			params.Loop = Config::Current->SaveFileGifLoop;
			params.AlphaThreshold = Config::Current->SaveFileGifAlphaThreshold;
			params.OverrideFrameDuration = Config::Current->SaveFileGifDurOverride;
			params.DitherLevel = double(Config::Current->SaveFileGifDitherLevel);
			params.FilterSize = (Config::Current->SaveFileGifFilterSize * 2) + 1;
			params.SampleFactor = Config::Current->SaveFileGifSampleFactor;
			success = gif.Save(outFile, params);
			break;
		}

		case tFileType::WEBP:
		{
			tImageWEBP webp(picture, steal);
			success = webp.Save(outFile, Config::Current->SaveFileWebpLossy, Config::Current->SaveFileWebpQualComp, Config::Current->SaveFileWebpDurOverride);
			break;
		}

		case tFileType::QOI:
		{
			tImageQOI qoi(picture, steal);
			tImageQOI::tFormat saveFormat = tImageQOI::tFormat::Auto;
			switch (Config::Current->SaveFileQoiDepthMode)
			{
				case 1: saveFormat = tImageQOI::tFormat::BPP24;		break;
				case 2: saveFormat = tImageQOI::tFormat::BPP32;		break;
			}

			tImageQOI::tSpace saveSpace = tImageQOI::tSpace::sRGB;
			switch (Config::Current->SaveFileQoiColourSpace)
			{
				case 0: saveSpace = tImageQOI::tSpace::sRGB;		break;
				case 1: saveSpace = tImageQOI::tSpace::Linear;		break;
			}
			qoi.SetColourSpace(saveSpace);

			tImageQOI::tFormat savedFormat = qoi.Save(outFile, saveFormat);
			success = (savedFormat != tImageQOI::tFormat::Invalid);
			break;
		}

		case tFileType::APNG:
		{
			tImageAPNG apng(picture, steal);
			success = apng.Save(outFile, Config::Current->SaveFileApngDurOverride);
			break;
		}

		case tFileType::BMP:
		{
			tImageBMP bmp(picture, steal);
			tImageBMP::tFormat saveFormat = tImageBMP::tFormat::Auto;
			switch (Config::Current->SaveFileBmpDepthMode)
			{
				case 1: saveFormat = tImageBMP::tFormat::BPP24;		break;
				case 2: saveFormat = tImageBMP::tFormat::BPP32;		break;
			}
			tImageBMP::tFormat savedFormat = bmp.Save(outFile, saveFormat);
			success = (savedFormat != tImageBMP::tFormat::Invalid);
			break;
		}

		case tFileType::TIFF:
		{
			tImageTIFF tiff(picture, steal);
			success = tiff.Save(outFile, Config::Current->SaveFileTiffZLibDeflate, Config::Current->SaveFileTiffDurOverride);
			break;
		}
	}

	return success;
}
