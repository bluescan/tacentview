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

//#include <Foundation/tVersion.cmake.h>
#include <Math/tVector2.h>
//#include <Math/tColour.h>
//#include <Image/tImageJPG.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "GuiUtil.h"
#include "ImportRaw.h"
//#include "Config.h"
//#include "Image.h"
//#include "TacentView.h"
//#include "Preferences.h"
//#include "Version.cmake.h"
using namespace tMath;
using namespace Gutil;


namespace Viewer
{
//
}


void Viewer::ShowImportRawOverlay(bool* popen)
{
	#if 0
	// @wip
	tVector2 windowPos = GetDialogOrigin(DialogID::ImportRaw);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("Import Raw", popen, flags))
	{
		ImGui::Text("Select File");
		static bool testBool = false;
		if (ImGui::Checkbox("Channel Intensity", &testBool))
		{
		}

		Gutil::Separator();
	}

	ImGui::End();
	#endif
}
