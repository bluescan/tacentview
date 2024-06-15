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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "GuiUtil.h"
#include "ImportRaw.h"
#include "Config.h"
using namespace tMath;
using namespace Gutil;


namespace Viewer
{
	// @wip
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

		// @wip Select File

		if (ImGui::InputInt("Width##ImportRaw", &profile.ImportRawWidth))
			tiClamp(profile.ImportRawWidth, 1, 65536);

		if (ImGui::InputInt("Height##ImportRaw", &profile.ImportRawHeight))
			tiClamp(profile.ImportRawHeight, 1, 65536);

		ImGui::Checkbox("Mipmaps##ImportRaw", &profile.ImportRawMipmaps);

		Gutil::Separator();
		if (ImGui::InputInt("Data Offset##ImportRaw", &profile.ImportRawDataOffset))
			tiClamp(profile.ImportRawDataOffset, 0, 65535);

		// @wip ImportRawPixelFormat
		// @wip ImportRawColourProfile
	}

	ImGui::End();
}
