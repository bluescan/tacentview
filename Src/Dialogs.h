// Dialogs.h
//
// Various dialogs and helpers including a log window, info overlay, cheatsheet, help window, about window, and prefs.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once


namespace TexView
{
	void ShowInfoOverlay(bool* popen, float x, float y, float w, float h, int cursorX, int cursorY, float zoom);
	void ShowCheatSheetPopup(bool* popen);
	void ShowAboutPopup(bool* popen);
	void ShowPreferencesDialog(bool* popen);
	void ColourCopyAs();
	void DoDeleteFileModal();
	void DoDeleteFileNoRecycleModal();

	// This class is a modification of the one that ships with Dear ImGui. I ahve included the orginal licence below.
	//
	// Copyright (c) 2014-2019 Omar Cornut
	// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
	// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
	// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
	// permit persons to whom the Software is furnished to do so, subject to the following conditions:
	//
	// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
	// Software.
	//
	// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	struct ViewerLog
	{
		ViewerLog()																											: ScrollToBottom(true) { Clear(); }

		void Clear();
		void AddLog(const char* fmt, ...) IM_FMTARGS(2);
		void Draw(const char* title, bool* popen = nullptr);

		ImGuiTextBuffer Buf;
		ImGuiTextFilter Filter;

		// Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines.
		ImVector<int> LineOffsets;
		bool ScrollToBottom;
	};
}
