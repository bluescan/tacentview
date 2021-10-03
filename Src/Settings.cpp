// Settings.cpp
//
// Viewer settings stored as human-readable symbolic expressions.
//
// Copyright (c) 2019, 2020, 2021 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WINDOWS
#include <dwmapi.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.

#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

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


void Viewer::Settings::Reset()
{
	ResetUISettings();
	ResetBehaviourSettings();
	TransparentWorkArea			= false;
}


void Viewer::Settings::ResetBehaviourSettings()
{
	SortKey						= 0;
	SortAscending				= true;
	ResampleFilter				= int(tImage::tResampleFilter::Bilinear);
	ResampleEdgeMode			= int(tImage::tResampleEdgeMode::Clamp);
	ResampleFilterRotateUp		= int(tImage::tResampleFilter::Bilinear);
	ResampleFilterRotateDown	= int(tImage::tResampleFilter::None);
	RotateMode					= int(RotMode::Fill);
	ConfirmDeletes				= true;
	ConfirmFileOverwrites		= true;
	SlideshowLooping			= false;
	SlideshowProgressArc		= true;
	SlideshowPeriod				= 8.0;			// Values as small as 1.0/60.0 also work.
	SaveSubFolder				.Clear();

	SaveFileType				= 0;
	SaveFileTypeMultiFrame		= 0;
	SaveFileTargaRLE			= false;
	SaveFileJpegQuality			= 95;
	SaveFileWebpLossy			= false;
	SaveFileWebpQualComp		= 90.0f;
	SaveFileTiffZLibDeflate		= true;

	SaveFileWebpDurOverride		= -1;
	SaveFileGifDurOverride		= -1;
	SaveFileApngDurOverride		= -1;
	SaveFileTiffDurOverride		= -1;

	SaveFileWebpDurMultiFrame	= 33;
	SaveFileGifDurMultiFrame	= 3;
	SaveFileApngDurMultiFrame	= 33;
	SaveFileTiffDurMultiFrame	= 33;

	SaveAllSizeMode				= 0;
	CropAnchor					= 4;
	FillColour					= tColouri::black;
	ResizeAspectNum				= 16;
	ResizeAspectDen				= 9;
	ResizeAspectMode			= 0;
	MaxImageMemMB				= 1024;
	MaxCacheFiles				= 7000;
	MaxUndoSteps				= 16;
	StrictLoading				= false;
	DetectAPNGInsidePNG			= true;
	MipmapFilter				= int(tImage::tResampleFilter::Bilinear);
	MipmapChaining				= true;
	AutoPlayAnimatedImages		= true;
	MonitorGamma				= tMath::DefaultGamma;
}


void Viewer::Settings::ResetUISettings()
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
	int screenW = mode ? mode->width  : 1280;
	int screenH = mode ? mode->height : 720;

	WindowW						= 1280;
	WindowH						= 720;
	WindowX						= (screenW - WindowW) >> 1;
	WindowY						= (screenH - WindowH) >> 1;

	ShowMenuBar					= true;
	ShowNavBar					= true;
	ShowImageDetails			= true;
	ShowPixelEditor				= false;
	ShowFrameScrubber			= true;
	AutoPropertyWindow			= true;
	ContentViewShow				= false;
	ThumbnailWidth				= 128.0f;
	OverlayCorner				= 1;
	Tile						= false;
	BackgroundStyle				= 1;
	BackgroundExtend			= false;
}


void Viewer::Settings::Load(const tString& filename)
{
	if (!tSystem::tFileExists(filename))
	{
		Reset();
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
				ReadItem(ShowPixelEditor);
				ReadItem(ShowFrameScrubber);
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
				ReadItem(ResampleFilterRotateUp);
				ReadItem(ResampleFilterRotateDown);
				ReadItem(RotateMode);
				ReadItem(ConfirmDeletes);
				ReadItem(ConfirmFileOverwrites);
				ReadItem(SlideshowLooping);
				ReadItem(SlideshowProgressArc);
				ReadItem(SlideshowPeriod);

				ReadItem(SaveSubFolder);
				ReadItem(SaveFileType);
				ReadItem(SaveFileTypeMultiFrame);
				ReadItem(SaveFileTargaRLE);
				ReadItem(SaveFileJpegQuality);
				ReadItem(SaveFileWebpLossy);
				ReadItem(SaveFileWebpQualComp);
				ReadItem(SaveFileTiffZLibDeflate);

				ReadItem(SaveFileWebpDurOverride);
				ReadItem(SaveFileGifDurOverride);
				ReadItem(SaveFileApngDurOverride);
				ReadItem(SaveFileTiffDurOverride);

				ReadItem(SaveFileWebpDurMultiFrame);
				ReadItem(SaveFileGifDurMultiFrame);
				ReadItem(SaveFileApngDurMultiFrame);
				ReadItem(SaveFileTiffDurMultiFrame);

				ReadItem(SaveAllSizeMode);
				ReadItem(CropAnchor);
				ReadItem(FillColour);
				ReadItem(ResizeAspectNum);
				ReadItem(ResizeAspectDen);
				ReadItem(ResizeAspectMode);
				ReadItem(MaxImageMemMB);
				ReadItem(MaxCacheFiles);
				ReadItem(MaxUndoSteps);
				ReadItem(StrictLoading);
				ReadItem(DetectAPNGInsidePNG);
				ReadItem(MipmapFilter);
				ReadItem(MipmapChaining);
				ReadItem(AutoPropertyWindow);
				ReadItem(AutoPlayAnimatedImages);
				ReadItem(MonitorGamma);
			}
		}
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
	int screenW = mode ? mode->width  : 1280;
	int screenH = mode ? mode->height : 720;

	tiClamp		(ResampleFilter, 0, int(tImage::tResampleFilter::NumFilters)-1);		// No None allowed.
	tiClamp		(ResampleEdgeMode, 0, int(tImage::tResampleEdgeMode::NumEdgeModes)-1);
	tiClamp		(ResampleFilterRotateUp, 0, int(tImage::tResampleFilter::NumFilters));	// None allowed.
	tiClamp		(ResampleFilterRotateDown, 0, int(tImage::tResampleFilter::NumFilters));// None allowed.
	tiClamp		(RotateMode, 0, int(RotMode::NumModes)-1);
	tiClampMin	(SlideshowPeriod, 1.0/60.0);
	tiClamp		(BackgroundStyle, 0, 4);
	tiClamp		(WindowW, 640, screenW);
	tiClamp		(WindowH, 360, screenH);
	tiClamp		(WindowX, 0, screenW - WindowW);
	tiClamp		(WindowY, 0, screenH - WindowH);
	tiClamp		(OverlayCorner, 0, 3);
	tiClamp		(SaveFileType, 0, 7);
	tiClamp		(SaveFileTypeMultiFrame, 0, 3);
	tiClamp		(ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth));
	tiClamp		(SortKey, 0, 6);
	tiClamp		(CropAnchor, -1, 9);
	tiClampMin	(ResizeAspectNum, 1);
	tiClampMin	(ResizeAspectDen, 1);
	tiClamp		(ResizeAspectMode, 0, 1);
	tiClampMin	(MaxImageMemMB, 256);
	tiClampMin	(MaxCacheFiles, 200);	
	tiClamp		(MaxUndoSteps, 1, 32);
	tiClamp		(MipmapFilter, 0, int(tImage::tResampleFilter::NumFilters));	// None allowed.
	tiClamp		(SaveAllSizeMode, 0, 3);
	tiClamp		(SaveFileJpegQuality, 1, 100);
	tiClamp		(SaveFileWebpQualComp, 0.0f, 100.0f);

	tiClampMin	(SaveFileWebpDurOverride, -1);
	tiClampMin	(SaveFileGifDurOverride, -1);
	tiClamp		(SaveFileApngDurOverride, -1, 65535);
	tiClampMin	(SaveFileTiffDurOverride, -1);

	tiClampMin	(SaveFileWebpDurMultiFrame, 0);
	tiClampMin	(SaveFileGifDurMultiFrame, 0);
	tiClamp		(SaveFileApngDurMultiFrame, 0, 65535);
	tiClampMin	(SaveFileTiffDurMultiFrame, 0);
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
	WriteItem(ShowPixelEditor);
	WriteItem(ShowFrameScrubber);
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
	WriteItem(ResampleFilterRotateUp);
	WriteItem(ResampleFilterRotateDown);
	WriteItem(RotateMode);
	WriteItem(ConfirmDeletes);
	WriteItem(ConfirmFileOverwrites);
	WriteItem(SlideshowLooping);
	WriteItem(SlideshowProgressArc);
	WriteItem(SlideshowPeriod);

	WriteItem(SaveSubFolder);
	WriteItem(SaveFileType);
	WriteItem(SaveFileTypeMultiFrame);
	WriteItem(SaveFileTargaRLE);
	WriteItem(SaveFileJpegQuality);
	WriteItem(SaveFileWebpLossy);
	WriteItem(SaveFileWebpQualComp);
	WriteItem(SaveFileTiffZLibDeflate);

	WriteItem(SaveFileWebpDurOverride);
	WriteItem(SaveFileGifDurOverride);
	WriteItem(SaveFileApngDurOverride);
	WriteItem(SaveFileTiffDurOverride);

	WriteItem(SaveFileWebpDurMultiFrame);
	WriteItem(SaveFileGifDurMultiFrame);
	WriteItem(SaveFileApngDurMultiFrame);
	WriteItem(SaveFileTiffDurMultiFrame);

	WriteItem(SaveAllSizeMode);
	WriteItem(CropAnchor);
	WriteItem(FillColour);
	WriteItem(ResizeAspectNum);
	WriteItem(ResizeAspectDen);
	WriteItem(ResizeAspectMode);
	WriteItem(MaxImageMemMB);
	WriteItem(MaxCacheFiles);
	WriteItem(MaxUndoSteps);
	WriteItem(StrictLoading);
	WriteItem(DetectAPNGInsidePNG);
	WriteItem(MipmapFilter);
	WriteItem(MipmapChaining);
	WriteItem(AutoPropertyWindow);
	WriteItem(AutoPlayAnimatedImages);
	WriteItem(MonitorGamma);

	return true;
}
