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

#include <Foundation/tFundamentals.h>
#include <System/tFile.h>
#include <System/tScript.h>
#include <Image/tResample.h>
#include "Settings.h"
#include "Image.h"
using namespace tMath;
#define ReadItem(name) case tHash::tHashCT(#name): name = e.Arg1(); break
#define	WriteItem(name) writer.Comp(#name, name)


namespace Viewer
{
	Settings Config;
}


void Viewer::Settings::Reset(int screenW, int screenH)
{
	ResetUISettings(screenW, screenH);
	ResetBehaviourSettings();
	TransparentWorkArea			= false;
}


void Viewer::Settings::ResetBehaviourSettings()
{
	SortKey						= 0;
	SortAscending				= true;
	ResampleFilter				= 2;			// Bilinear.
	ResampleEdgeMode			= 0;			// Clamp.
	ConfirmDeletes				= true;
	ConfirmFileOverwrites		= true;
	SlideshowLooping			= false;
	SlideshowProgressArc		= true;
	SlidehowFrameDuration		= 8.0;			// Values as small as 1.0/30.0 also work.
	SaveSubFolder				.Clear();
	SaveFileType				= 0;
	SaveFileTargaRLE			= false;
	SaveFileJpegQuality			= 95;
	SaveAllSizeMode				= 0;
	ResizeAspectNum				= 16;
	ResizeAspectDen				= 9;
	ResizeAspectMode			= 0;
	ResizeAspectFillColour		= tColouri::black;
	MaxImageMemMB				= 1024;
	MaxCacheFiles				= 7000;
	StrictLoading				= false;
	DetectAPNGInsidePNG			= true;
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

	ShowMenuBar					= true;
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
				ReadItem(ShowMenuBar);
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
				ReadItem(TransparentWorkArea);
				ReadItem(ResampleFilter);
				ReadItem(ResampleEdgeMode);
				ReadItem(ConfirmDeletes);
				ReadItem(ConfirmFileOverwrites);
				ReadItem(SlideshowLooping);
				ReadItem(SlideshowProgressArc);
				ReadItem(SlidehowFrameDuration);
				ReadItem(SaveSubFolder);
				ReadItem(SaveFileType);
				ReadItem(SaveFileTargaRLE);
				ReadItem(SaveFileJpegQuality);
				ReadItem(SaveAllSizeMode);
				ReadItem(ResizeAspectNum);
				ReadItem(ResizeAspectDen);
				ReadItem(ResizeAspectMode);
				ReadItem(ResizeAspectFillColour);
				ReadItem(MaxImageMemMB);
				ReadItem(MaxCacheFiles);
				ReadItem(StrictLoading);
				ReadItem(DetectAPNGInsidePNG);
				ReadItem(AutoPropertyWindow);
				ReadItem(AutoPlayAnimatedImages);
				ReadItem(MonitorGamma);
			}
		}
	}

	tiClamp(ResampleFilter, 0, int(tImage::tResampleFilter::NumFilters)-1);
	tiClamp(ResampleEdgeMode, 0, int(tImage::tResampleEdgeMode::NumEdgeModes)-1);
	tiClamp(BackgroundStyle, 0, 4);
	tiClamp(WindowW, 640, screenW);
	tiClamp(WindowH, 360, screenH);
	tiClamp(WindowX, 0, screenW - WindowW);
	tiClamp(WindowY, 0, screenH - WindowH);
	tiClamp(OverlayCorner, 0, 3);
	tiClamp(SaveFileType, 0, 3);
	tiClamp(ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth));
	tiClamp(SortKey, 0, 3);
	tiClampMin(ResizeAspectNum, 1);
	tiClampMin(ResizeAspectDen, 1);
	tiClamp(ResizeAspectMode, 0, 1);
	tiClampMin(MaxImageMemMB, 256);
	tiClampMin(MaxCacheFiles, 200);
	tiClamp(SaveAllSizeMode, 0, 3);
	tiClamp(SaveFileJpegQuality, 1, 100);
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
	WriteItem(ShowMenuBar);
	WriteItem(ShowNavBar);
	WriteItem(ShowImageDetails);
	WriteItem(ContentViewShow);
	WriteItem(ThumbnailWidth);
	WriteItem(SortKey);
	WriteItem(SortAscending);
	WriteItem(OverlayCorner);
	WriteItem(Tile);
	WriteItem(BackgroundStyle);
	WriteItem(BackgroundExtend);
	WriteItem(TransparentWorkArea);
	WriteItem(ResampleFilter);
	WriteItem(ResampleEdgeMode);
	WriteItem(ConfirmDeletes);
	WriteItem(ConfirmFileOverwrites);
	WriteItem(SlideshowLooping);
	WriteItem(SlideshowProgressArc);
	WriteItem(SlidehowFrameDuration);
	WriteItem(SaveSubFolder);
	WriteItem(SaveFileType);
	WriteItem(SaveFileTargaRLE);
	WriteItem(SaveFileJpegQuality);
	WriteItem(SaveAllSizeMode);
	WriteItem(ResizeAspectNum);
	WriteItem(ResizeAspectDen);
	WriteItem(ResizeAspectMode);
	WriteItem(ResizeAspectFillColour);
	WriteItem(MaxImageMemMB);
	WriteItem(MaxCacheFiles);
	WriteItem(StrictLoading);
	WriteItem(DetectAPNGInsidePNG);
	WriteItem(AutoPropertyWindow);
	WriteItem(AutoPlayAnimatedImages);
	WriteItem(MonitorGamma);

	return true;
}
