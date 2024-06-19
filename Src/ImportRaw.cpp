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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "GuiUtil.h"
#include "ImportRaw.h"
#include "Config.h"
#include "FileDialog.h"
using namespace tMath;
using namespace Gutil;
using namespace tFileDialog;


namespace ImportRaw
{
	FileDialog SelectFileDialog(DialogMode::OpenFile);
	tString ImportFile;
}


void Viewer::ShowImportRawOverlay(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(DialogID::ImportRaw);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("Import Raw", popen, flags))
	{
		Config::ProfileData& profile = Config::GetProfileData();

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

			Gutil::Button("Import");
		}

		if (ImGui::InputInt("Width##ImportRaw", &profile.ImportRawWidth))
			tiClamp(profile.ImportRawWidth, 1, 65536);

		if (ImGui::InputInt("Height##ImportRaw", &profile.ImportRawHeight))
			tiClamp(profile.ImportRawHeight, 1, 65536);

		ImGui::Checkbox("Mipmaps##ImportRaw", &profile.ImportRawMipmaps);

		if (ImGui::InputInt("Data Offset##ImportRaw", &profile.ImportRawDataOffset))
			tiClamp(profile.ImportRawDataOffset, 0, 65535);

		Gutil::Separator();
		tImage::tPixelFormat fmt = profile.GetImportRawPixelFormat();
		int currPacked = tImage::tIsPackedFormat(fmt) ? int(fmt) - int(tImage::tPixelFormat::FirstPacked) : -1;
		int currBlock  = tImage::tIsBCFormat(fmt)     ? int(fmt) - int(tImage::tPixelFormat::FirstBC)     : -1;
		int currPVR    = tImage::tIsPVRFormat(fmt)    ? int(fmt) - int(tImage::tPixelFormat::FirstPVR)    : -1;
		int currASTC   = tImage::tIsASTCFormat(fmt)   ? int(fmt) - int(tImage::tPixelFormat::FirstASTC)   : -1;
		const int maxDropdownFormats = 14;

		if (Gutil::Combo("Packed", &currPacked, tImage::PixelFormatNames_Packed, tImage::PixelFormatNames_Packed, int(tImage::tPixelFormat::NumPackedFormats), tMin(int(tImage::tPixelFormat::NumPackedFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPacked) + currPacked;

		if (ImGui::Combo("Block", &currBlock, tImage::PixelFormatNames_Block, int(tImage::tPixelFormat::NumBCFormats), tMin(int(tImage::tPixelFormat::NumBCFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstBC) + currBlock;

		if (ImGui::Combo("PVR", &currPVR, tImage::PixelFormatNames_PVR, int(tImage::tPixelFormat::NumPVRFormats), tMin(int(tImage::tPixelFormat::NumPVRFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPVR) + currPVR;

		if (ImGui::Combo("ASTC", &currASTC, tImage::PixelFormatNames_ASTC, int(tImage::tPixelFormat::NumASTCFormats), tMin(int(tImage::tPixelFormat::NumASTCFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstASTC) + currASTC;

		// @wip ImportRawPixelFormat
		// @wip ImportRawColourProfile
	}

	ImGui::End();
}
