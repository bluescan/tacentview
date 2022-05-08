// OpenSaveDialogs.h
//
// Modal dialogs open-file, open-dir, save-as and save-all.
//
// Copyright (c) 2019-2021 Tristan Grimmer.
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


namespace Viewer
{
	void DoOpenFileModal(bool openFilePressed);
	void DoOpenDirModal(bool openDirPressed);

	void DoSaveAsModal(bool saveAsPressed);
	void DoSaveAllModal(bool saveAllPressed);

	void DoOverwriteFileModal(const tString& outFile, bool& pressedOK, bool& pressedCancel);
	void DoOverwriteMultipleFilesModal(const tList<tStringItem>& overwriteFiles, bool& pressedOK, bool& pressedCancel);

	tString DoSubFolder();				// Returns destination dir.
	tString DoSaveFiletype();			// Returns extension.
	tString DoSaveFiletypeMultiFrame();	// Returns extension.
}
