// OpenSaveDialogs.h
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

#pragma once
#include <Foundation/tList.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include <Image/tPicture.h>


namespace Viewer
{
	class Image;
	void DoOpenFileModal(bool openFilePressed);
	void DoOpenDirModal(bool openDirPressed);

	void DoSaveModal(bool savePressed);
	void DoSaveAsModal(bool savePressed);
	void DoSaveAllModal(bool saveAllPressed);

	void DoOverwriteFileModal(const tString& outFile, bool& pressedOK, bool& pressedCancel);
	void DoOverwriteMultipleFilesModal(const tList<tStringItem>& overwriteFiles, bool& pressedOK, bool& pressedCancel);

	// Returns destination dir.
	tString DoSubFolder();
	tSystem::tFileType DoSaveChooseFiletype();
	void DoSaveFiletypeOptions(tSystem::tFileType);

	// Returns extension.
	tString DoSaveFiletypeMultiFrame();

	// Not all fileTypes are supported for save. Reads config for each types save parameters. If you set steal to true
	// you are indicating you don't mind if picture is modified and the pixels are stolen from it. It's more efficient
	// to do this if possible. Returns success.
	bool SavePictureAs(tImage::tPicture& picture, const tString& outFile, tSystem::tFileType fileType, bool steal);

	// Not all fileTypes are supported for save. Reads config for each types save parameters. When saving a
	// Viewer::Image the pixels are never stolen from it. Handles single and multi-frame images. Returns success.
	bool SaveImageAs(Viewer::Image& image, const tString& outFile, tSystem::tFileType fileType);
}
