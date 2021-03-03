// FileDialog.cpp
//
// Dialog that allows selection of a file or directory. May be used for opening a file/directory or saving to a file.
//
// Copyright (c) 2021 Tristan Grimmer.
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
#include "FileDialog.h"
#include "TacentView.h"
#include "Image.h"
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;


FileDialog::FileDialog(DialogMode mode) :
	Mode(mode)
{
}


void FileDialog::OpenPopup()
{
	ImGui::OpenPopup("Open File");
}


FileDialog::DialogResult FileDialog::DoPopup()
{
	// The unused isOpenFile bool is just so we get a close button in ImGui. 
	bool isOpenFile = true;
	if (!ImGui::BeginPopupModal("Open File", &isOpenFile, ImGuiWindowFlags_AlwaysAutoResize))
		return DialogResult::Closed;

	DialogResult result = DialogResult::Open;
	Result.Clear();
	ImGui::Text("OPEN FILE");
	ImGui::Separator();
	if (ImGui::Button("OK"))
	{
		if (Mode == DialogMode::OpenFile)
			Result = "C:/testfile.txt";
		else if (Mode == DialogMode::OpenDir)
			Result = "C:/";
		result = DialogResult::OK;
	}

	if (ImGui::Button("Cancel"))
		result = DialogResult::Cancel;
	ImGui::Text("OPEN FILE");

	if (result != DialogResult::Open)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
	return result;
}


tString FileDialog::GetResult()
{
	return Result;
}
