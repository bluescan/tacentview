// Settings.cpp
//
// Viewer settings stored as human-readable symbolic expressions.
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

#include <System/tFile.h>
#include <System/tScript.h>
#include <Math/tFundamentals.h>
#include "Settings.h"
#include "Image.h"
using namespace tMath;
#define ReadItem(name) case tHashCT(#name): name = e.Arg1(); break
#define	WriteItem(name) writer.Comp(#name, name)


namespace Viewer
{
	Settings Config;
}


void Viewer::Settings::Reset(int screenW, int screenH)
{
	ResetUISettings(screenW, screenH);
	ResetBehaviourSettings();
}


void Viewer::Settings::ResetBehaviourSettings()
{
	SortKey						= 0;
	SortAscending				= true;
	ResampleFilter				= 2;
	ConfirmDeletes				= true;
	ConfirmFileOverwrites		= true;
	SlideshowLooping			= false;
	SlidehowFrameDuration		= 1.0/30.0;
	SaveSubFolder				.Clear();
	SaveFileType				= 0;
	SaveFileTargaRLE			= false;
	SaveFileJpgQuality			= 90.0f;
	SaveAllSizeMode				= 0;
	MaxImageMemMB				= 1024;
	MaxCacheFiles				= 7000;
	AutoPropertyWindow			= true;
	AutoPlayAnimatedImages		= true;
	MonitorGamma				= tMath::DefaultGamma;
}


void Viewer::Settings::ResetUISettings(int screenW, int screenH)
{
	WindowW						= 1280;
	WindowH						= 720;
	WindowX						= (screenW - WindowW) >> 1;
	WindowY						= (screenH - WindowH) >> 1;

	ShowNavBar					= true;
	ShowImageDetails			= true;
	ContentViewShow				= false;
	ThumbnailWidth				= 128.0f;
	OverlayCorner				= 3;
	Tile						= false;
	BackgroundStyle				= 1;
	BackgroundExtend			= false;
}


void Viewer::Settings::Load(const tString& filename, int screenW, int screenH)
{
	if (!tSystem::tFileExists(filename))
	{
		Reset(screenW, screenH);
	}
	else
	{
		tScriptReader reader(filename);
		for (tExpr e = reader.First(); e.IsValid(); e = e.Next())
		{
			switch (e.Command().Hash())
			{
				ReadItem(WindowX);
				ReadItem(WindowY);
				ReadItem(WindowW);
				ReadItem(WindowH);
				ReadItem(ShowNavBar);
				ReadItem(ShowImageDetails);
				ReadItem(ContentViewShow);
				ReadItem(ThumbnailWidth);
				ReadItem(SortKey);
				ReadItem(SortAscending);
				ReadItem(OverlayCorner);
				ReadItem(Tile);
				ReadItem(BackgroundStyle);
				ReadItem(BackgroundExtend);
				ReadItem(ResampleFilter);
				ReadItem(ConfirmDeletes);
				ReadItem(ConfirmFileOverwrites);
				ReadItem(SlideshowLooping);
				ReadItem(SlidehowFrameDuration);
				ReadItem(SaveSubFolder);
				ReadItem(SaveFileType);
				ReadItem(SaveFileTargaRLE);
				ReadItem(SaveFileJpgQuality);
				ReadItem(SaveAllSizeMode);
				ReadItem(MaxImageMemMB);
				ReadItem(MaxCacheFiles);
				ReadItem(AutoPropertyWindow);
				ReadItem(AutoPlayAnimatedImages);
				ReadItem(MonitorGamma);
			}
		}
	}

	tiClamp(ResampleFilter, 0, 5);
	tiClamp(BackgroundStyle, 0, 4);
	tiClamp(WindowW, 640, screenW);
	tiClamp(WindowH, 360, screenH);
	tiClamp(WindowX, 0, screenW - WindowW);
	tiClamp(WindowY, 0, screenH - WindowH);
	tiClamp(OverlayCorner, 0, 3);
	tiClamp(SaveFileType, 0, 4);
	tiClamp(ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth));
	tiClamp(SortKey, 0, 3);
	tiClampMin(MaxImageMemMB, 256);
	tiClampMin(MaxCacheFiles, 200);
	tiClamp(SaveAllSizeMode, 0, 3);
	tiClamp(SaveFileJpgQuality, 0.0f, 100.0f);
}


bool Viewer::Settings::Save(const tString& filename)
{
	tScriptWriter writer(filename);
	writer.Rem("Tacent View Configuration File");
	writer.CR();

	WriteItem(WindowX);
	WriteItem(WindowY);
	WriteItem(WindowW);
	WriteItem(WindowH);
	WriteItem(ShowNavBar);
	WriteItem(ShowImageDetails);
	WriteItem(ContentViewShow);
	WriteItem(ThumbnailWidth);
	WriteItem(SortKey);
	WriteItem(SortAscending);
	WriteItem(OverlayCorner);
	WriteItem(Tile);
	WriteItem(BackgroundExtend);
	WriteItem(BackgroundStyle);
	WriteItem(ResampleFilter);
	WriteItem(ConfirmDeletes);
	WriteItem(ConfirmFileOverwrites);
	WriteItem(SlideshowLooping);
	WriteItem(SlidehowFrameDuration);
	WriteItem(SaveSubFolder);
	WriteItem(SaveFileType);
	WriteItem(SaveFileTargaRLE);
	WriteItem(SaveFileJpgQuality);
	WriteItem(SaveAllSizeMode);
	WriteItem(MaxImageMemMB);
	WriteItem(MaxCacheFiles);
	WriteItem(AutoPropertyWindow);
	WriteItem(AutoPlayAnimatedImages);
	WriteItem(MonitorGamma);

	return true;
}
