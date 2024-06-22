// ImportRaw.cpp
//
// Import contiguous raw pixel data from any file.
//
// Copyright (c) 2024 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tVector2.h>
#include <Math/tColour.h>
#include <System/tFile.h>
#include <Image/tPixelFormat.h>
#include <Image/tFrame.h>
#include <Image/tImageWEBP.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "GuiUtil.h"
#include "ImportRaw.h"
#include "Config.h"
#include "FileDialog.h"
#include "TacentView.h"
#include "Image.h"
using namespace tMath;
using namespace tFileDialog;
using namespace tImage;


namespace ImportRaw
{
	FileDialog SelectFileDialog(DialogMode::OpenFile);
	tString ImportFile;
	tString ImportedFile;

	bool CreateFrames(tList<tFrame>& frames, const tString& rawFile, int width, int height, int offset, bool mipmaps);
	tString MakeImportedFilename(tSystem::tFileType destType, const tString& rawFile);
	bool CreateImportedFile(tList<tFrame>& frames, const tString& filename);
}


void Viewer::ShowImportRawOverlay(bool* popen, bool justOpened)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::ImportRaw);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("Import Raw", popen, flags))
	{
		Config::ProfileData& profile = Config::GetProfileData();
		if (justOpened)
			ImportRaw::ImportedFile.Clear();

		if (Gutil::Button("Select File"))
			ImportRaw::SelectFileDialog.OpenPopup();

		FileDialog::DialogState state = ImportRaw::SelectFileDialog.DoPopup();

		// You will get an OK dialog state once after the DoPopup when OK is pressed.
		switch (state)
		{
			case FileDialog::DialogState::Cancel: break;
			case FileDialog::DialogState::Closed: break;
			case FileDialog::DialogState::OK:
				ImportRaw::ImportFile = ImportRaw::SelectFileDialog.GetResult();
				break;
			case FileDialog::DialogState::Open: break;
		}

		if (ImportRaw::ImportFile.IsValid())
		{
			tString importName = tSystem::tGetFileName(ImportRaw::ImportFile);
			ImGui::SameLine();
			ImGui::Text(importName.Chr());
		}

		// FileType of destination image.
		tString createTypeName = profile.ImportRawFileType;
		tSystem::tFileType createType = tSystem::tGetFileTypeFromName(createTypeName);
		tSystem::tFileType newType = createType;
		bool filetypeChanged = false;
		if (ImGui::BeginCombo("Create Type", createTypeName.Chr()))
		{
			for (tSystem::tFileTypes::tFileTypeItem* item = FileTypes_ImportRaw.First(); item; item = item->Next())
			{
				tSystem::tFileType ft = item->FileType;
				bool selected = (ft == createType);

				tString ftName = tGetFileTypeName(ft);
				if (ImGui::Selectable(ftName.Chr(), &selected))
				{
					profile.ImportRawFileType = ftName;
					newType = tSystem::tGetFileTypeFromName(ftName);
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}				
			ImGui::EndCombo();
		}
		if (createType != newType)
		{
			createType = newType;
			// @todo If it changed and an existing file exists, delete it and clear the ImportedFile.
		}
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"When an image is imported from raw data it creates a new image of this type.\n"
			"Valid types are ones that are lossless or support lossless encoding like webp.\n"
			"Created images support alpha channel. If no alpha it saves the image without it.\n"
			"If the creation type supports multiple frames, the option to import mipmaps will\n"
			"be available. Not all formats support this but WEBP, APNG, and TIF do."
		);

		if (ImGui::InputInt("Data Offset##ImportRaw", &profile.ImportRawDataOffset))
			tiClamp(profile.ImportRawDataOffset, 0, Viewer::Image::MaxDim-1);

		if (ImGui::InputInt("Width##ImportRaw", &profile.ImportRawWidth))
			tiClamp(profile.ImportRawWidth, 1, Viewer::Image::MaxDim);

		if (ImGui::InputInt("Height##ImportRaw", &profile.ImportRawHeight))
			tiClamp(profile.ImportRawHeight, 1, Viewer::Image::MaxDim);

		bool fileTypeSupportsMipmaps = (createType == tSystem::tFileType::APNG) || (createType == tSystem::tFileType::TIFF) || (createType == tSystem::tFileType::WEBP);
		if (fileTypeSupportsMipmaps)
			ImGui::Checkbox("Mipmaps##ImportRaw", &profile.ImportRawMipmaps);
		bool importMipmaps = fileTypeSupportsMipmaps ? profile.ImportRawMipmaps : false;

		static bool liveUpdates = false;
		ImGui::Checkbox("Live Updates", &liveUpdates);
		ImGui::SameLine();
		Gutil::HelpMark("Automatically re-import on any change.");

		Gutil::Separator();
		tImage::tPixelFormat fmt = profile.GetImportRawPixelFormat();
		int currPacked = tImage::tIsPackedFormat(fmt) ? int(fmt) - int(tImage::tPixelFormat::FirstPacked) : -1;
		int currBlock  = tImage::tIsBCFormat(fmt)     ? int(fmt) - int(tImage::tPixelFormat::FirstBC)     : -1;
		int currPVR    = tImage::tIsPVRFormat(fmt)    ? int(fmt) - int(tImage::tPixelFormat::FirstPVR)    : -1;
		int currASTC   = tImage::tIsASTCFormat(fmt)   ? int(fmt) - int(tImage::tPixelFormat::FirstASTC)   : -1;
		const int maxDropdownFormats = 14;
		ImGui::Text("Pixel Format:");
		if (Gutil::Combo("Packed", &currPacked, tImage::PixelFormatNames_Packed, tImage::PixelFormatDescs_Packed, int(tImage::tPixelFormat::NumPackedFormats), tMin(int(tImage::tPixelFormat::NumPackedFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPacked) + currPacked;
		if (currPacked != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_Packed[currPacked]);

		if (Gutil::Combo("BC", &currBlock, tImage::PixelFormatNames_Block, tImage::PixelFormatDescs_Block, int(tImage::tPixelFormat::NumBCFormats), tMin(int(tImage::tPixelFormat::NumBCFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstBC) + currBlock;
		if (currBlock != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_Block[currBlock]);

		if (Gutil::Combo("PVR", &currPVR, tImage::PixelFormatNames_PVR, tImage::PixelFormatDescs_PVR, int(tImage::tPixelFormat::NumPVRFormats), tMin(int(tImage::tPixelFormat::NumPVRFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPVR) + currPVR;
		if (currPVR != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_PVR[currPVR]);

		if (Gutil::Combo("ASTC", &currASTC, tImage::PixelFormatNames_ASTC, tImage::PixelFormatDescs_ASTC, int(tImage::tPixelFormat::NumASTCFormats), tMin(int(tImage::tPixelFormat::NumASTCFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstASTC) + currASTC;
		if (currASTC != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_ASTC[currASTC]);

		if (ImportRaw::ImportFile.IsValid())
		{
			if (Gutil::Button("Import"))
			{
				tList<tFrame> frames;
				bool ok = ImportRaw::CreateFrames(frames, ImportRaw::ImportFile, profile.ImportRawWidth, profile.ImportRawHeight, profile.ImportRawDataOffset, profile.ImportRawMipmaps);
				if (ok)
				{
					tString importedFilename = ImportRaw::ImportFile;
					if (importedFilename.IsEmpty())
						importedFilename = ImportRaw::MakeImportedFilename(createType, ImportRaw::ImportFile);
					if (importedFilename.IsEmpty())
					{
						// Update message.
					}
					else
					{
						bool exists = ImportRaw::CreateImportedFile(frames, importedFilename);
						if (exists)
						{
							ImportRaw::ImportedFile = importedFilename;
							
							// @todo Should not create a new one if not needed.
							Image* newImg = new Image(importedFilename);
							Images.Append(newImg);
							ImagesLoadTimeSorted.Append(newImg);
							SortImages(profile.GetSortKey(), profile.SortAscending);
							SetCurrentImage(importedFilename);
						}
					}
				}
				else
				{
					// Set Message Text.
				}
				// Create the frames from ImportFile.
				// if no frames (not enough data), print warning. Do nothing.
				// if frames,
				// 		string ImportedFileTry = MakeImportedFilename(profile.ImportRawFileType);
				//		ImportedFile = DoCreateImportedFile(frames, ImportedFileTry); // OverWrite the file if necessary.
			}
		}

		// @wip ImportRawPixelFormat
		// @wip ImportRawColourProfile
	}

	ImGui::End();
}


bool ImportRaw::CreateFrames(tList<tFrame>& frames, const tString& rawFile, int width, int height, int offset, bool mipmaps)
{
	if (rawFile.IsEmpty() || (width <= 0) || (height <= 0) || (offset < 0) || !tSystem::tFileExists(rawFile))
		return false;

	static int cycle = 0;
	tPixel4b colour = tPixel4b::black;
	switch (cycle)
	{
		case 0:		colour = tPixel4b::red;		break;
		case 1:		colour = tPixel4b::green;	break;
		case 2:		colour = tPixel4b::blue;	break;
	}
	cycle = (cycle+1) % 3;
	tPixel4b* pixels = new tPixel4b[width*height];
	for (int p = 0; p < width*height; p++)
		pixels[p] = colour;

	tFrame* frame = new tFrame;
	frame->StealFrom(pixels, width, height);
	frames.Append(frame);
	return true;
}


tString ImportRaw::MakeImportedFilename(tSystem::tFileType destType, const tString& rawFile)
{
	tString dir = tSystem::tGetDir(rawFile);
	tString base = tSystem::tGetFileBaseName(rawFile);
	tString filename;
	int attempt = 0;
	do
	{

		tsPrintf
		(
			filename, "%s%s_Imported_%02d.%s",
			Viewer::ImagesDir.Chr(),
			base.Chr(),
			attempt,
			tGetExtension(destType).Chr()
		);
		attempt++;
	} while (tSystem::tFileExists(filename) && (attempt < 100));

	return filename;
}


bool ImportRaw::CreateImportedFile(tList<tFrame>& frames, const tString& filename)
{
	// OverWrite the file if necessary.
	tSystem::tFileType type = tSystem::tGetFileTypeFromExtension( tSystem::tGetFileExtension(filename) );

	switch (type)
	{
		case tSystem::tFileType::WEBP:
		{
			tImage::tImageWEBP webp;
			webp.Set(frames, true);
//			webp.Set(dstData, width, height, true);
			bool saved = webp.Save(filename, false);
			return saved;
		}
	}

	return false;
}
