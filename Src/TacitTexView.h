// TacitTexView.h
//
// A texture viewer for various formats.
//
// Copyright (c) 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <System/tCommand.h>
#include "Settings.h"
class TacitImage;
class tColouri;


namespace TexView
{
	extern int MajorVersion;
	extern int MinorVersion;
	extern int Revision;

	extern Settings Config;
	extern TacitImage* CurrImage;
	extern tList<TacitImage> Images;
	extern tCommand::tParam ImageFileParam;
	extern tColouri PixelColour;

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	void ShowHelpMark(const char* desc);
	void ShowToolTip(const char* desc);
	void FindTextureFiles();
	void SetCurrentImage(const tString& currFilename = tString());
}
