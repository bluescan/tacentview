// OpenSaveDialogs.cpp
//
// Modal dialogs open-file, open-dir, save-as and save-all.
//
// Copyright (c) 2019-2023 Tristan Grimmer.
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
#include "GuiUtil.h"
#include "OpenSaveDialogs.h"
#include "Image.h"
#include "TacentView.h"
#include "FileDialog.h"
#include "Quantize.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;
using namespace tFileDialog;
using namespace Gutil;


namespace Viewer
{
	void SaveAllImages(const tString& destDir, const tString& extension, float percent, int width, int height);
	void GetFilesNeedingOverwrite(const tString& destDir, tList<tStringItem>& overwriteFiles, const tString& extension);
	void AddSavedImageIfNecessary(const tString& savedFile);

	// This function saves the picture to the filename specified.
	bool SaveImageAs(Image&, const tString& outFile);
	bool SaveResizeImageAs(Image&, const tString& outFile, int width, int height, float scale = 1.0f, Config::ProfileData::SizeModeEnum = Config::ProfileData::SizeModeEnum::SetWidthAndHeight);
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
		ImageToLoad = chosenFile;
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
		ImageToLoad = chosenDir + "dummyfile.txt";
		PopulateImages();
		SetCurrentImage();
		SetWindowTitle();
	}
}


void Viewer::DoSaveCurrentModal(bool savePressed)
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

	float nextWinWidth = Gutil::GetUIParamScaled(300.0f, 2.5f);

	// The unused isOpenSaveOptions bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenSaveOptions = true;
	ImGui::SetNextWindowSize(tVector2(nextWinWidth, 0.0f));
	if (!ImGui::BeginPopupModal(label.Chr(), &isOpenSaveOptions, ImGuiWindowFlags_NoScrollbar))
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

	float nextWinWidth = Gutil::GetUIParamScaled(300.0f, 2.5f);

	// The unused isOpenSaveOptions bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isOpenSaveOptions = true;
	ImGui::SetNextWindowSize(tVector2(nextWinWidth, 0.0f));
	if (!ImGui::BeginPopupModal(label.Chr(), &isOpenSaveOptions, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	DoSavePopup();
	ImGui::EndPopup();
}


void Viewer::DoSavePopup()
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);
	Config::ProfileData& profile = Config::GetProfileData();

	// This gets the filetype from the filename. We then update the current profile.
	tFileType saveType = tGetFileType(SaveAsFile);
	profile.SaveFileType = tGetFileTypeName(saveType);

	DoSaveFiletypeOptions(saveType);

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);
	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();

	// The GetWindowContentRegionMax is OK here since width was fixed to a specific size before the Begin call.
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - buttonWidth);
	bool closeThisModal = false;

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("Save", tVector2(buttonWidth, 0.0f)))
	{
		if (tFileExists(SaveAsFile) && profile.ConfirmFileOverwrites)
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

				SortImages(profile.GetSortKey(), profile.SortAscending);
				SetCurrentImage(SaveAsFile);
			}
			closeThisModal = true;
		}
	}

	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	if (ImGui::BeginPopupModal("Overwrite File", &isOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
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

				SortImages(profile.GetSortKey(), profile.SortAscending);
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

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);

	// The GetWindowContentRegionMax is OK here since width was fixed to a specific size before the Begin call.
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - buttonWidth);
	bool closeThisModal = false;

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();

	if (Gutil::Button("OK", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();
}


tString Viewer::DoSubFolder()
{
	// Output sub-folder.
	Config::ProfileData& profile = Config::GetProfileData();
	char subFolder[256]; tMemset(subFolder, 0, 256);
	tStrncpy(subFolder, profile.SaveSubFolder.Chr(), 255);

	float itemWidth		= Gutil::GetUIParamScaled(160.0f, 2.5f);
	float buttonWidth	= Gutil::GetUIParamScaled(76.0f, 2.5f);

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputText("SubFolder", subFolder, 256);
	profile.SaveSubFolder.Set(subFolder);
	tString destDir = ImagesDir;
	if (!profile.SaveSubFolder.IsEmpty())
		destDir += profile.SaveSubFolder + "/";
	tString toolTipText;
	tsPrintf(toolTipText, "Save to %s", destDir.Chr());
	ShowToolTip(toolTipText.Chr());

	if (ImGui::Button("Default", tVector2(buttonWidth, 0.0f)))
		profile.SaveSubFolder.Set("Saved");
	ImGui::SameLine();
	if (ImGui::Button("Here", tVector2(buttonWidth, 0.0f)))
		profile.SaveSubFolder.Clear();

	return destDir;
}


tSystem::tFileType Viewer::DoSaveChooseFiletype()
{
	Config::ProfileData& profile = Config::GetProfileData();
	tString fileTypeName = profile.SaveFileType;
	tFileType fileType = tGetFileTypeFromName(fileTypeName);

	float itemWidth = Gutil::GetUIParamScaled(160.0f, 2.5f);
	ImGui::SetNextItemWidth(itemWidth);
	if (ImGui::BeginCombo("File Type", fileTypeName.Chr()))
	{
		for (tFileTypes::tFileTypeItem* item = FileTypes_Save.First(); item; item = item->Next())
		{
			tFileType ft = item->FileType;
			bool selected = (ft == fileType);

			tString ftName = tGetFileTypeName(ft);
			if (ImGui::Selectable(ftName.Chr(), &selected))
				profile.SaveFileType = ftName;

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ShowHelpMark("Output image format.\nFull (non-binary) alpha supported by tga, png, apng, bmp, tiff, qoi, and webp.\nAnimation supported by webp, gif, tiff, and apng.");

	return tGetFileTypeFromName(profile.SaveFileType);
}


void Viewer::DoSaveGifOptions(bool multiframeConfigValues)
{
	Config::ProfileData& profile = Config::GetProfileData();
	float itemWidth = Gutil::GetUIParamScaled(160.0f, 2.5f);

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::SliderInt("Bits per Pixel", &profile.SaveFileGifBPP, 1, 8, "%d");
	ImGui::SameLine();
	ShowHelpMark
	(
		"Determines how many colours the gif uses. From 2 to 256.\n"
		"Note that using transparency takes one colour away."
	);

	Viewer::DoQuantizeInterface
	(
		profile.SaveFileGifQuantMethod,
		profile.SaveFileGifFilterSize,
		profile.SaveFileGifDitherLevel,
		profile.SaveFileGifSampleFactor,
		itemWidth
	);

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Alpha Threshold", &profile.SaveFileGifAlphaThreshold);
	tiClamp(profile.SaveFileGifAlphaThreshold, -1, 255);
	ImGui::SameLine(); ShowHelpMark
	(
		"Gif only supports binary alpha. Set threshold -1 to auto-determine if the GIF\n"
		"should have alpha -- if the image is fully opaque the GIF will not have alpha and\n"
		"if the image has transparency, a threshold of 127 will be used. When threshold\n"
		"between 0 and 254 (both inclusive) it will use the threshold to determine if the\n"
		"pixel is transparent or not. A pixel alpha less-than or equal means the pixel\n"
		"will be transparent. Larger means opaque. If the threshold is set to 255 it will\n"
		"force the gif to be opaque regardless of the pixel alpha value."
	);

	ImGui::SetNextItemWidth(itemWidth);
	if (!multiframeConfigValues)
	{
		ImGui::InputInt("Duration Override", &profile.SaveFileGifDurOverride);
		tiClamp(profile.SaveFileGifDurOverride, -1, 1000);
		ImGui::SameLine(); ShowHelpMark("In 1/100 seconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
		if (Gutil::Button("1.0s"))  profile.SaveFileGifDurOverride = 100; ImGui::SameLine();
		if (Gutil::Button("0.5s"))  profile.SaveFileGifDurOverride = 50;  ImGui::SameLine();
		if (Gutil::Button("15fps")) profile.SaveFileGifDurOverride = 6;   ImGui::SameLine();
		if (Gutil::Button("30fps")) profile.SaveFileGifDurOverride = 3;
	}
	else
	{
		ImGui::InputInt("Frame Duration", &profile.SaveFileGifDurMultiFrame);
		tiClamp(profile.SaveFileGifDurMultiFrame, 0, 1000);
		ImGui::SameLine(); ShowHelpMark("In 1/100 seconds.");
		if (Gutil::Button("1.0s"))  profile.SaveFileGifDurMultiFrame = 100; ImGui::SameLine();
		if (Gutil::Button("0.5s"))  profile.SaveFileGifDurMultiFrame = 50;  ImGui::SameLine();
		if (Gutil::Button("15fps")) profile.SaveFileGifDurMultiFrame = 6;   ImGui::SameLine();
		if (Gutil::Button("30fps")) profile.SaveFileGifDurMultiFrame = 3;   ImGui::SameLine();
		if (Gutil::Button("50fps")) profile.SaveFileGifDurMultiFrame = 2;
	}

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::InputInt("Loop", &profile.SaveFileGifLoop);
	tiClampMin(profile.SaveFileGifLoop, 0);
	ImGui::SameLine();
	ShowHelpMark("How many times an animated gif will loop. 0 means forever.\nThis only applies to animated (multi-frame) images.");

	tString loopDesc = "forever";
	if (profile.SaveFileGifLoop != 0)
	{
		if (profile.SaveFileGifLoop == 1)
			tsPrintf(loopDesc, "1 time");
		else
			tsPrintf(loopDesc, "%d times", profile.SaveFileGifLoop);
	}
	tString desc;
	tsPrintf(desc, " %d-bit palette GIF. If animated, loops %s.\n", profile.SaveFileGifBPP, loopDesc.Chr());
	if (profile.SaveFileGifBPP == 1)
		tsaPrintf(desc, " 2-colour images are always opaque.\n");
	else if (profile.SaveFileGifAlphaThreshold < 0)
		tsaPrintf(desc, " Auto-alpha. If image transparent %d colours.\n", (1 << profile.SaveFileGifBPP) - 1);
	else if (profile.SaveFileGifAlphaThreshold >= 255)
		tsaPrintf(desc, " Opaque. Image will have %d colours.\n", (1 << profile.SaveFileGifBPP));
	else
		tsaPrintf(desc, " Binary-alpha. Image will have %d colours.\n", (1 << profile.SaveFileGifBPP) - 1);
	if ((profile.SaveFileGifQuantMethod == int(tQuantize::Method::Spatial)) && (profile.SaveFileGifBPP > 5))
		tsaPrintf(desc, " WARNING: Scolorq at large BPPs may take\n a long time for large images.");
	ImGui::Text(desc.Chr());

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);
	if (Gutil::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		profile.SaveFileGifBPP				= 8;
		profile.SaveFileGifQuantMethod		= int(tImage::tQuantize::Method::Wu);
		profile.SaveFileGifLoop				= 0;
		profile.SaveFileGifAlphaThreshold	= -1;
		profile.SaveFileGifDitherLevel		= 0.0f;
		profile.SaveFileGifFilterSize		= 1;
		profile.SaveFileGifSampleFactor		= 1;
		profile.SaveFileWebpDurOverride		= -1;
		profile.SaveFileGifDurOverride		= -1;
		profile.SaveFileGifDurMultiFrame	= 3;
	}
}


void Viewer::DoSaveFiletypeOptions(tFileType fileType)
{
	Config::ProfileData& profile = Config::GetProfileData();
	float itemWidth = Gutil::GetUIParamScaled(160.0f, 2.5f);

	ImGui::PushItemWidth(itemWidth);

	switch (fileType)
	{
		case tFileType::TGA:
		{
			const char* tgaModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::Combo("Bits Per Pixel", &profile.SaveFileTgaDepthMode , tgaModeItems, tNumElements(tgaModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");

			ImGui::Checkbox("RLE Compression", &profile.SaveFileTgaRLE);
			ImGui::SameLine();
			ShowHelpMark("Perform simple run-length compression.");
			break;
		}

		case tFileType::PNG:
		{
			const char* pngModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::Combo("Bits Per Pixel", &profile.SaveFilePngDepthMode , pngModeItems, tNumElements(pngModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");
			break;
		}

		case tFileType::JPG:
			ImGui::SliderInt("Quality", &profile.SaveFileJpegQuality, 1, 100, "%d");
			break;

		case tFileType::GIF:
			DoSaveGifOptions(false);
			break;

		case tFileType::WEBP:
			ImGui::Checkbox("Lossy", &profile.SaveFileWebpLossy);
			if (profile.SaveFileWebpLossy)
			{
				ImGui::SliderFloat("Quality", &profile.SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
				ImGui::SameLine(); ShowHelpMark("Lossy selected. This is the image quality percent.");
			}
			else
			{
				ImGui::SliderFloat("Compression", &profile.SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
				ImGui::SameLine(); ShowHelpMark("Non-lossy selected. This is the image compression strength.\nBigger is slower and yields smaller files.");
			}
			ImGui::SliderInt("Duration Override", &profile.SaveFileWebpDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowHelpMark("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
			if (Gutil::Button("1.0s"))  profile.SaveFileWebpDurOverride = 1000; ImGui::SameLine();
			if (Gutil::Button("0.5s"))  profile.SaveFileWebpDurOverride = 500;  ImGui::SameLine();
			if (Gutil::Button("30fps")) profile.SaveFileWebpDurOverride = 33;   ImGui::SameLine();
			if (Gutil::Button("60fps")) profile.SaveFileWebpDurOverride = 16;
			break;

		case tFileType::QOI:
		{
			const char* qoiModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::Combo("Bits Per Pixel", &profile.SaveFileQoiDepthMode , qoiModeItems, tNumElements(qoiModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");

			const char* qoiSpaceItems[] = { "Auto", "sRGB", "Linear" };
			ImGui::Combo("Colour Space", &profile.SaveFileQoiColourSpace , qoiSpaceItems, tNumElements(qoiSpaceItems));
			ImGui::SameLine();
			ShowHelpMark("Colour space to store in the saved file.\nAuto: Use current colour space as it was loaded.\nsRGB: The default for most images.\nLinear: For images used in lighting calculations.");
			break;
		}

		case tFileType::APNG:
			ImGui::SliderInt("Duration Override", &profile.SaveFileApngDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowHelpMark("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
			if (Gutil::Button("1.0s"))  profile.SaveFileApngDurOverride = 1000; ImGui::SameLine();
			if (Gutil::Button("0.5s"))  profile.SaveFileApngDurOverride = 500;  ImGui::SameLine();
			if (Gutil::Button("30fps")) profile.SaveFileApngDurOverride = 33;   ImGui::SameLine();
			if (Gutil::Button("60fps")) profile.SaveFileApngDurOverride = 16;
			break;

		case tFileType::BMP:
		{
			const char* bmpModeItems[] = { "Auto", "24 BPP", "32 BPP" };
			ImGui::Combo("Bits Per Pixel", &profile.SaveFileBmpDepthMode , bmpModeItems, tNumElements(bmpModeItems));
			ImGui::SameLine();
			ShowHelpMark("Auto: Decide based on opacity.\n24 BPP: Force 24 bits per pixel.\n32 BPP: Force 32 bits per pixel.");
			break;
		}

		case tFileType::TIFF:
			ImGui::SliderInt("Duration Override", &profile.SaveFileTiffDurOverride, -1, 10000, "%d");
			ImGui::SameLine(); ShowHelpMark("In milliseconds. If set to >= 0, overrides all frame durations.\nIf -1, uses the current value for the frame.");
			if (Gutil::Button("1.0s"))  profile.SaveFileTiffDurOverride = 1000; ImGui::SameLine();
			if (Gutil::Button("0.5s"))  profile.SaveFileTiffDurOverride = 500;  ImGui::SameLine();
			if (Gutil::Button("30fps")) profile.SaveFileTiffDurOverride = 33;   ImGui::SameLine();
			if (Gutil::Button("60fps")) profile.SaveFileTiffDurOverride = 16;
			break;
	}

	ImGui::PopItemWidth();
}


tString Viewer::DoSaveFiletypeMultiFrame()
{
	Config::ProfileData& profile = Config::GetProfileData();
	tString fileTypeName = profile.SaveFileTypeMultiFrame;
	tFileType fileType = tGetFileTypeFromName(fileTypeName);

	float itemWidth = Gutil::GetUIParamScaled(160.0f, 2.5f);

	ImGui::SetNextItemWidth(itemWidth);
	if (ImGui::BeginCombo("File Type", fileTypeName.Chr()))
	{
		for (tFileTypes::tFileTypeItem* item = FileTypes_SaveMultiFrame.First(); item; item = item->Next())
		{
			tFileType ft = item->FileType;
			bool selected = (ft == fileType);

			tString ftName = tGetFileTypeName(ft);
			if (ImGui::Selectable(ftName.Chr(), &selected))
				profile.SaveFileTypeMultiFrame = ftName;

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ShowHelpMark("Multi-frame output image format.");

	// There are different options depending on what type you are saving as.
	fileType = tGetFileTypeFromName(profile.SaveFileTypeMultiFrame);
	tString extension = tGetExtension(fileType);
	switch (fileType)
	{
		case tFileType::GIF:
			DoSaveGifOptions(true);
			break;

		case tFileType::WEBP:
			// @todo This should be using a standard options call like the GIF type.
			ImGui::Checkbox("Lossy", &profile.SaveFileWebpLossy);
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::SliderFloat("Quality / Compression", &profile.SaveFileWebpQualComp, 0.0f, 100.0f, "%.1f");
			ImGui::SameLine(); ShowToolTip("Image quality percent if lossy. Image compression strength if not lossy"); ImGui::NewLine();
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::SliderInt("Frame Duration", &profile.SaveFileWebpDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  profile.SaveFileWebpDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  profile.SaveFileWebpDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) profile.SaveFileWebpDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) profile.SaveFileWebpDurMultiFrame = 16;
			break;

		case tFileType::APNG:
			// @todo This should be using a standard options call like the GIF type.
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::SliderInt("Frame Duration", &profile.SaveFileApngDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  profile.SaveFileApngDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  profile.SaveFileApngDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) profile.SaveFileApngDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) profile.SaveFileApngDurMultiFrame = 16;
			break;

		case tFileType::TIFF:
			// @todo This should be using a standard options call like the GIF type.
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::SliderInt("Frame Duration", &profile.SaveFileTiffDurMultiFrame, 0, 10000, "%d");
			ImGui::SameLine(); ShowToolTip("In milliseconds."); ImGui::NewLine();
			if (ImGui::Button("1.0s"))  profile.SaveFileTiffDurMultiFrame = 1000; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  profile.SaveFileTiffDurMultiFrame = 500;  ImGui::SameLine();
			if (ImGui::Button("30fps")) profile.SaveFileTiffDurMultiFrame = 33;   ImGui::SameLine();
			if (ImGui::Button("60fps")) profile.SaveFileTiffDurMultiFrame = 16;
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
		imageLoaded = img.Load();
	if (!imageLoaded)
	{
		tPrintf("Failed to save image %s\n", outFile.Chr());
		return false;
	}

	Config::ProfileData& profile = Config::GetProfileData();
	tFileType fileType = tGetFileTypeFromName( profile.SaveFileType );
	bool success = img.Save(outFile, fileType);
	if (success)
		tPrintf("Saved image as %s\n", outFile.Chr());
	else
		tPrintf("Failed to save image %s\n", outFile.Chr());

	return success;
}


bool Viewer::SaveResizeImageAs(Image& img, const tString& outFile, int width, int height, float scale, Config::ProfileData::SizeModeEnum sizeMode)
{
	// We make sure to maintain the loaded/unloaded state. This function may be called many times in succession
	// so we don't want them all in memory at once by indiscriminantly loading them all.
	bool imageLoaded = img.IsLoaded();
	if (!imageLoaded)
		imageLoaded = img.Load();
	if (!imageLoaded)
	{
		tPrintf("Failed to save image %s\n", outFile.Chr());
		return false;
	}	

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
		case Config::ProfileData::SizeModeEnum::Percent:
			if (tMath::tApproxEqual(scale, 1.0f, 0.01f))
				break;
			outW = int( tRound(float(outW)*scale) );
			outH = int( tRound(float(outH)*scale) );
			break;

		case Config::ProfileData::SizeModeEnum::SetWidthAndHeight:
			outW = width;
			outH = height;
			break;

		case Config::ProfileData::SizeModeEnum::SetWidthRetainAspect:
			outW = width;
			outH = int( tRound(float(width) / aspect) );
			break;

		case Config::ProfileData::SizeModeEnum::SetHeightRetainAspect:
			outH = height;
			outW = int( tRound(float(height) * aspect) );
			break;
	};

	Config::ProfileData& profile = Config::GetProfileData();
	tMath::tiClampMin(outW, 4);
	tMath::tiClampMin(outH, 4);
	if ((outPic.GetWidth() != outW) || (outPic.GetHeight() != outH))
		outPic.Resample(outW, outH, tImage::tResampleFilter(profile.ResampleFilter), tImage::tResampleEdgeMode(profile.ResampleEdgeMode));

	tFileType saveFileType = tGetFileTypeFromName(profile.SaveFileType);
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
	if (!ImGui::BeginPopupModal("Save All", &isOpenSaveAll, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	ImGui::Text("Save all %d images to the image type you select.", Images.GetNumItems());

	ImGui::Separator();

	float buttonWidth	= Gutil::GetUIParamScaled(76.0f, 2.5f);
	float itemWidth		= Gutil::GetUIParamScaled(160.0f, 2.5f);

	ImGui::PushItemWidth(itemWidth);
	static int width = 512;
	static int height = 512;
	static float percent = 100.0f;
	const char* sizeModeNames[] = { "Percent of Original", "Set Width and Height", "Set Width - Keep Aspect", "Set Height - Keep Aspect" };
	Config::ProfileData& profile = Config::GetProfileData();
	ImGui::Combo("Size Mode", &profile.SaveAllSizeMode, sizeModeNames, tNumElements(sizeModeNames));
	ImGui::SameLine();
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
		"  Set Width - Keep Aspect\n"
		"  All images will have specified width. Always\n"
		"  uniform scale. Varying height.\n"
		"\n"
		"  Set Height - Keep Aspect\n"
		"  All images will have specified height. Always\n"
		"  uniform scale. Varying width.\n"
	);
	switch (profile.GetSaveAllSizeMode())
	{
		case Config::ProfileData::SizeModeEnum::Percent:
			ImGui::InputFloat("Percent", &percent, 1.0f, 10.0f, "%.1f");	ImGui::SameLine();	ShowHelpMark("Percent of original size.");
			break;

		case Config::ProfileData::SizeModeEnum::SetWidthAndHeight:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;

		case Config::ProfileData::SizeModeEnum::SetWidthRetainAspect:
			ImGui::InputInt("Width", &width);	ImGui::SameLine();	ShowHelpMark("Output width in pixels for all images.");
			break;

		case Config::ProfileData::SizeModeEnum::SetHeightRetainAspect:
			ImGui::InputInt("Height", &height);	ImGui::SameLine();	ShowHelpMark("Output height in pixels for all images.");
			break;
	};

	ImGui::Separator();
	if (!((profile.GetSaveAllSizeMode() == Config::ProfileData::SizeModeEnum::Percent) && (percent == 100.0f)))
	{
		ImGui::Combo("Filter", &profile.ResampleFilter, tResampleFilterNames, int(tResampleFilter::NumFilters), int(tResampleFilter::NumFilters));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");

		ImGui::Combo("Filter Edge Mode", &profile.ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
		ImGui::SameLine();
		ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");
	}
	tMath::tiClampMin(width, 4);
	tMath::tiClampMin(height, 4);
	ImGui::PopItemWidth();

	tFileType fileType = DoSaveChooseFiletype();
	DoSaveFiletypeOptions(fileType);
	tString extensionWithDot = tString(".") + tGetExtension(fileType);
	ImGui::Separator();
	tString destDir = DoSubFolder();

	ImGui::Separator();

	ImGui::NewLine();
	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	float saveAllOffset = Gutil::GetUIParamScaled(200.0f, 2.5f);
	ImGui::SetCursorPosX(saveAllOffset);

	// This needs to be static since this function is called for every frame the modal is open.
	static tList<tStringItem> overwriteFiles(tListMode::Static);
	bool closeThisModal = false;

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("Save All", tVector2(buttonWidth, 0.0f)))
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
			if (!overwriteFiles.IsEmpty() && profile.ConfirmFileOverwrites)
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
	if (ImGui::BeginPopupModal("Overwrite Multiple Files", &isOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
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
	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth		= Gutil::GetUIParamScaled(76.0f, 2.5f);
	float textWidth			= Gutil::GetUIParamScaled(366.0f, 2.5f);
	float overwriteOffset	= Gutil::GetUIParamScaled(310.0f, 2.5f);

	ImGui::Text("The Following Files");
	ImGui::Indent();
	int fnum = 0;
	const int maxToShow = 6;
	for (tStringItem* filename = overwriteFiles.First(); filename && (fnum < maxToShow); filename = filename->Next(), fnum++)
	{
		tString file		= tSystem::tGetFileName(*filename);
		tString fileCropped	= Gutil::CropStringToWidth(file, textWidth, true);
		ImGui::Text("%s", fileCropped.Chr());
		if (fileCropped != file)
			Gutil::ShowToolTip(file.Chr());
	}
	int remaining = overwriteFiles.GetNumItems() - fnum;
	if (remaining > 0)
		ImGui::Text("And %d more.", remaining);
	ImGui::Unindent();
	ImGui::Text("Already Exist In Folder");
	tString dir				= tSystem::tGetDir(*overwriteFiles.First());
	tString dirCropped		= Gutil::CropStringToWidth(dir, textWidth, true);
	ImGui::Indent();
		ImGui::Text("%s", dirCropped.Chr());
		if (dirCropped != dir)
			Gutil::ShowToolTip(dir.Chr());
	ImGui::Unindent();
	ImGui::Text("Overwrite Files?");
	ImGui::Separator();
	ImGui::Checkbox("Confirm file overwrites in the future?", &profile.ConfirmFileOverwrites);
	ImGui::NewLine();

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
	{
		pressedCancel = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	ImGui::SetCursorPosX(overwriteOffset);
	if (Gutil::Button("Overwrite", tVector2(buttonWidth, 0.0f)))
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
	Config::ProfileData& profile = Config::GetProfileData();

	bool anySaved = false;
	for (Image* image = Images.First(); image; image = image->Next())
	{
		tString baseName = tSystem::tGetFileBaseName(image->Filename);
		tString outFile = destDir + tString(baseName) + extension;

		bool ok = SaveResizeImageAs(*image, outFile, width, height, scale, profile.GetSaveAllSizeMode());
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
		SortImages(profile.GetSortKey(), profile.SortAscending);
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
	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth		= Gutil::GetUIParamScaled(76.0f, 2.5f);
	float textWidth			= Gutil::GetUIParamScaled(366.0f, 2.5f);
	float okOffset			= Gutil::GetUIParamScaled(310.0f, 2.5f);

	tString file			= tSystem::tGetFileName(outFile);
	tString fileCropped		= Gutil::CropStringToWidth(file, textWidth, true);
	tString dir				= tSystem::tGetDir(outFile);
	tString dirCropped		= Gutil::CropStringToWidth(dir, textWidth, true);

	ImGui::Text("Overwrite file");
	ImGui::Indent();
		ImGui::Text("%s", fileCropped.Chr());
		if (fileCropped != file)
			Gutil::ShowToolTip(file.Chr());
	ImGui::Unindent();

	ImGui::Text("In Folder");
	ImGui::Indent();
		ImGui::Text("%s", dirCropped.Chr());
		if (dirCropped != dir)
			Gutil::ShowToolTip(dir.Chr());
	ImGui::Unindent();
	ImGui::Separator();

	ImGui::Checkbox("Confirm file overwrites in the future?", &profile.ConfirmFileOverwrites);
	ImGui::NewLine();
	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
	{
		pressedCancel = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(okOffset);
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("OK", tVector2(buttonWidth, 0.0f)))
	{
		pressedOK = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


bool Viewer::SavePictureAs(tImage::tPicture& picture, const tString& outFile, tFileType fileType, bool steal)
{
	// @todo This function looks _very_ similar to the save call in the Image class.
	if (!picture.IsValid())
		return false;

	Config::ProfileData& profile = Config::GetProfileData();
	bool success = false;
	switch (fileType)
	{
		case tFileType::TGA:
		{
			tImageTGA tga(picture, steal);
			tImageTGA::tFormat saveFormat = tImageTGA::tFormat::Auto;
			switch (profile.SaveFileTgaDepthMode)
			{
				case 1: saveFormat = tImageTGA::tFormat::BPP24;		break;
				case 2: saveFormat = tImageTGA::tFormat::BPP32;		break;
			}
			tImageTGA::tFormat savedFmt = tga.Save(outFile, saveFormat, profile.SaveFileTgaRLE ? tImageTGA::tCompression::RLE : tImageTGA::tCompression::None);
			success = (savedFmt != tImageTGA::tFormat::Invalid);
			break;
		}

		case tFileType::PNG:
		{
			tImagePNG png(picture, steal);
			tImagePNG::tFormat saveFormat = tImagePNG::tFormat::Auto;
			switch (profile.SaveFilePngDepthMode)
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
			success = jpg.Save(outFile, profile.SaveFileJpegQuality);
			break;
		}

		case tFileType::GIF:
		{
			tImageGIF gif(picture, steal);
			tImageGIF::SaveParams params;
			params.Format					= tPixelFormat(int(tPixelFormat::FirstPalette) + profile.SaveFileGifBPP - 1);
			params.Method					= tQuantize::Method(profile.SaveFileGifQuantMethod);
			params.Loop						= profile.SaveFileGifLoop;
			params.AlphaThreshold			= profile.SaveFileGifAlphaThreshold;
			params.OverrideFrameDuration	= profile.SaveFileGifDurOverride;
			params.DitherLevel				= double(profile.SaveFileGifDitherLevel);
			params.FilterSize				= (profile.SaveFileGifFilterSize * 2) + 1;
			params.SampleFactor				= profile.SaveFileGifSampleFactor;
			success = gif.Save(outFile, params);
			break;
		}

		case tFileType::WEBP:
		{
			tImageWEBP webp(picture, steal);
			success = webp.Save(outFile, profile.SaveFileWebpLossy, profile.SaveFileWebpQualComp, profile.SaveFileWebpDurOverride);
			break;
		}

		case tFileType::QOI:
		{
			tImageQOI qoi(picture, steal);
			tImageQOI::tFormat saveFormat = tImageQOI::tFormat::Auto;
			switch (profile.SaveFileQoiDepthMode)
			{
				case 1: saveFormat = tImageQOI::tFormat::BPP24;		break;
				case 2: saveFormat = tImageQOI::tFormat::BPP32;		break;
			}

			tImageQOI::tSpace saveSpace = tImageQOI::tSpace::Auto;
			switch (profile.SaveFileQoiColourSpace)
			{
				case 1: saveSpace = tImageQOI::tSpace::sRGB;		break;
				case 2: saveSpace = tImageQOI::tSpace::Linear;		break;
			}

			tImageQOI::tFormat savedFormat = qoi.Save(outFile, saveFormat);
			success = (savedFormat != tImageQOI::tFormat::Invalid);
			break;
		}

		case tFileType::APNG:
		{
			tImageAPNG apng(picture, steal);

			tImageAPNG::SaveParams params;
			params.OverrideFrameDuration = profile.SaveFileApngDurOverride;
			tImageAPNG::tFormat savedFormat = apng.Save(outFile, params);
			success = (savedFormat != tImageAPNG::tFormat::Invalid);
			break;
		}

		case tFileType::BMP:
		{
			tImageBMP bmp(picture, steal);
			tImageBMP::tFormat saveFormat = tImageBMP::tFormat::Auto;
			switch (profile.SaveFileBmpDepthMode)
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
			tImageTIFF::SaveParams params;
			params.UseZLibCompression = profile.SaveFileTiffZLibDeflate;
			params.OverrideFrameDuration = profile.SaveFileTiffDurOverride;
			success = tiff.Save(outFile, params);
			break;
		}
	}

	return success;
}
