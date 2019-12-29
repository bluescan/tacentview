// Settings.cpp
//
// Viewer settings stored as human-readable symbolic expressions.
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

#include <System/tFile.h>
#include <System/tScript.h>
#include <Math/tFundamentals.h>
#include "Settings.h"
using namespace tMath;


void Settings::Load(const tString& filename, int screenW, int screenH)
{
	if (!tSystem::tFileExists(filename))
	{
		WindowW = 1280;
		WindowH = 720;
		WindowX = (screenW - WindowW) >> 1;
		WindowY = (screenH - WindowH) >> 1;
		OverlayShow = false;
		OverlayCorner = 3;
	}
	else
	{
		tScriptReader reader(filename);
		for (tExpr e = reader.First(); e.IsValid(); e = e.Next())
		{
			switch (e.Command().Hash())
			{
				case tHashCT("WindowX"):
					WindowX = e.Arg1();
					break;

				case tHashCT("WindowY"):
					WindowY = e.Arg1();
					break;

				case tHashCT("WindowW"):
					WindowW = e.Arg1();
					break;

				case tHashCT("WindowH"):
					WindowH = e.Arg1();
					break;

				case tHashCT("OverlayShow"):
					OverlayShow = e.Arg1();
					break;

				case tHashCT("OverlayCorner"):
					OverlayCorner = e.Arg1();
					break;
			}
		}
	}

	tClamp(WindowW, 640, screenW);
	tClamp(WindowH, 360, screenH);
	tClamp(WindowX, 0, screenW - WindowW);
	tClamp(WindowY, 0, screenH - WindowH);
	tClamp(OverlayCorner, 0, 3);
}


bool Settings::Save(const tString& filename)
{
	tScriptWriter writer(filename);
	writer.Rem("Tacit Texture Viewer Configuration File");
	writer.CR();

	writer.Comp("WindowX", WindowX);
	writer.Comp("WindowY", WindowY);
	writer.Comp("WindowW", WindowW);
	writer.Comp("WindowH", WindowH);
	writer.Comp("OverlayShow", OverlayShow);
	writer.Comp("OverlayCorner", OverlayCorner);

	return true;
}
