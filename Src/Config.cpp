// Config.cpp
//
// Viewer settings stored as human-readable symbolic expressions.
//
// Copyright (c) 2019, 2020, 2021, 2022 Tristan Grimmer.
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
#include "Config.h"
#include "Image.h"
using namespace tMath;
#define ReadItem(name) case tHash::tHashCT(#name): name = e.Arg1(); break
#define	WriteItem(name) writer.Comp(#name, name)
#define	WriteLast(name) writer.Coms(#name, name)
namespace Viewer {


namespace Config
{
	const int ConfigFileVersion = 2;

	GlobalSettings Global;
	Settings MainSettings;
	Settings BasicSettings;
	Settings* Current = &MainSettings;
}


const char* Config::ProfileNames[int(Profile::NumProfiles)] =
{
	"Main",
	"Basic"
};


const char* Config::ProfileNamesLong[int(Profile::NumProfiles)] =
{
	"Main Profile",
	"Basic Profile"
};


void Config::SetProfile(Profile profile)
{
	if (profile == Profile(Global.CurrentProfile))
		return;

	switch (profile)
	{
		case Profile::Main:		Current = &MainSettings;	break;
		case Profile::Basic:	Current = &BasicSettings;	break;
	}
	Global.CurrentProfile = int(profile);
}


Config::Profile Config::GetProfile()			{ return Profile(Global.CurrentProfile); }
const char* Config::GetProfileName()			{ return ProfileNames[int(GetProfile())]; }


void Config::ResetProfile(uint32 categories)
{
	Current->Reset(Profile(Global.CurrentProfile), categories);
}


void Config::ResetAllProfiles(uint32 categories)
{
	MainSettings.Reset(Profile::Main, categories);
	BasicSettings.Reset(Profile::Basic, categories);
}


void Config::ResetAll()
{
	ResetAllProfiles(Category_All);
}


void Config::Load(const tString& filename)
{
	if (!tSystem::tFileExists(filename))
	{
		Global.Reset();
		ResetAll();
		Current = &MainSettings;
		Global.CurrentProfile = int(Profile::Main);
		return;
	}

	tScriptReader reader(filename);
	for (tExpr e = reader.First(); e.IsValid(); e = e.Next())
	{
		switch (e.Item0().Hash())
		{
			case tHash::tHashCT("Global"):
				Global.Load(e);
				break;

			case tHash::tHashCT("MainProfile"):
				MainSettings.Load(e);
				break;
			
			case tHash::tHashCT("BasicProfile"):
				BasicSettings.Load(e);
				break;
		}
	}

	// I think it makes sense to restore the currently selected profile when the config file is loaded.
	// This means if you were in Basic profile when you close, you will be in basic when you start the app again.
	switch (Profile(Global.CurrentProfile))
	{
		case Profile::Main:		Current = &MainSettings;	break;
		case Profile::Basic:	Current = &BasicSettings;	break;
	}
}


void Config::Save(const tString& filename)
{
	tScriptWriter writer(filename);
	writer.Rem("Tacent View Configuration File");
	writer.CR();

	Global.ConfigVersion = ConfigFileVersion;
	Global.Save(writer);
	writer.CR();
	writer.CR();

	MainSettings.Profile = "MainProfile";
	MainSettings.Save(writer);
	writer.CR();
	writer.CR();

	BasicSettings.Profile = "BasicProfile";
	BasicSettings.Save(writer);
	writer.CR();
}


void Config::GlobalSettings::Save(tScriptWriter& writer)
{
	writer.Begin();
	writer.Indent();
	writer.CR();
	writer.WriteAtom("Global");
	writer.CR();

	WriteItem(ConfigVersion);
	WriteLast(CurrentProfile);

	writer.Dedent();
	writer.CR();
	writer.End();
}


void Config::GlobalSettings::Load(tExpression expr)
{
	for (tExpr e = expr.Item1(); e.IsValid(); e = e.Next())
	{
		switch (e.Command().Hash())
		{
			ReadItem(ConfigVersion);
			ReadItem(CurrentProfile);
		}
	}
}


void Config::GlobalSettings::Reset()
{
	ConfigVersion = ConfigFileVersion;
	CurrentProfile = int(Profile::Main);
}


void Config::Settings::Reset(Config::Profile profile, uint32 categories)
{
	if (categories & Category_Unspecified)
	{
		GLFWmonitor* monitor		= glfwGetPrimaryMonitor();
		const GLFWvidmode* mode		= monitor ? glfwGetVideoMode(monitor) : nullptr;
		int screenW					= mode ? mode->width  : 1280;
		int screenH					= mode ? mode->height : 720;
		WindowW						= 1280;
		WindowH						= 720;						//	Sub 37 if want 720 inc title bar;
		WindowX						= (screenW - WindowW) >> 1;
		WindowY						= (screenH - WindowH) >> 1;

		ShowMenuBar					= (profile == Profile::Basic) ? false : true;
		ShowNavBar					= (profile == Profile::Basic) ? false : true;
		ShowImageDetails			= (profile == Profile::Basic) ? false : true;
		ShowPixelEditor				= false;
		ShowChannelFilter			= false;
		ShowFrameScrubber			= (profile == Profile::Basic) ? false : true;
		ContentViewShow				= false;
		ThumbnailWidth				= 128.0f;
		SortKey						= 0;
		SortAscending				= true;
		OverlayCorner				= 1;
		Tile						= false;
		ResampleFilter				= int(tImage::tResampleFilter::Bilinear);
		ResampleEdgeMode			= int(tImage::tResampleEdgeMode::Clamp);
		ResampleFilterRotateUp		= int(tImage::tResampleFilter::Bilinear);
		ResampleFilterRotateDown	= int(tImage::tResampleFilter::None);
		RotateMode					= int(RotMode::Fill);

		SlideshowLooping			= (profile == Profile::Basic) ? true : false;
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
	}

	if (categories & Category_Display)
	{
		BackgroundStyle				= (profile == Profile::Basic) ? int(BGStyle::None) : int(BGStyle::Checkerboard);
		BackgroundColour			= tColouri::black;
		BackgroundExtend			= false;
		TransparentWorkArea			= false;
		FixedAspectWorkArea			= false;
	}

	if (categories & Category_Slideshow)
	{
		SlideshowProgressArc		= true;
		SlideshowPeriod				= (profile == Profile::Basic) ? 8.0 : 4.0;			// Values as small as 1.0/60.0 also work.
	}

	if (categories & Category_System)
	{
		MaxImageMemMB				= 2048;
		MaxCacheFiles				= 8192;
		MaxUndoSteps				= 16;
		StrictLoading				= false;
		DetectAPNGInsidePNG			= true;
		MipmapFilter				= int(tImage::tResampleFilter::Bilinear);
		MipmapChaining				= true;
		MonitorGamma				= tMath::DefaultGamma;
	}

	if (categories & Category_Interface)
	{
		ConfirmDeletes				= true;
		ConfirmFileOverwrites		= true;
		AutoPropertyWindow			= (profile == Profile::Basic) ? false : true;
		AutoPlayAnimatedImages		= true;
		DefaultZoomMode				= int(ZoomMode::DownscaleOnly);
	}

	if (categories & Category_Bindings)
	{
		InputBindings				.Reset();
	}
}


void Config::Settings::Load(tExpression expr)
{
	for (tExpr e = expr.Item1(); e.IsValid(); e = e.Next())
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
			ReadItem(ShowChannelFilter);
			ReadItem(ShowFrameScrubber);
			ReadItem(ContentViewShow);
			ReadItem(ThumbnailWidth);
			ReadItem(SortKey);
			ReadItem(SortAscending);
			ReadItem(OverlayCorner);
			ReadItem(Tile);
			ReadItem(BackgroundStyle);
			ReadItem(BackgroundColour);
			ReadItem(BackgroundExtend);
			ReadItem(TransparentWorkArea);
			ReadItem(FixedAspectWorkArea);
			ReadItem(ResampleFilter);
			ReadItem(ResampleEdgeMode);
			ReadItem(ResampleFilterRotateUp);
			ReadItem(ResampleFilterRotateDown);
			ReadItem(RotateMode);
			ReadItem(DefaultZoomMode);
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

			case tHash::tHashCT("KeyBindings"):
				InputBindings.Read(e);
				break;

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

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
	int screenW = mode ? mode->width  : 1280;
	int screenH = mode ? mode->height : 720;

	tiClamp		(ResampleFilter, 0, int(tImage::tResampleFilter::NumFilters)-1);		// No None allowed.
	tiClamp		(ResampleEdgeMode, 0, int(tImage::tResampleEdgeMode::NumEdgeModes)-1);
	tiClamp		(ResampleFilterRotateUp, 0, int(tImage::tResampleFilter::NumFilters));	// None allowed.
	tiClamp		(ResampleFilterRotateDown, 0, int(tImage::tResampleFilter::NumFilters));// None allowed.
	tiClamp		(RotateMode, 0, int(RotMode::NumModes)-1);
	tiClamp		(DefaultZoomMode, 0, int(ZoomMode::NumModes)-1);
	tiClampMin	(SlideshowPeriod, 1.0/60.0);
	tiClamp		(BackgroundStyle, 0, int(BGStyle::NumStyles)-1);
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


bool Config::Settings::Save(tScriptWriter& writer)
{
	writer.Begin();
	writer.Indent();
	writer.CR();
	writer.WriteAtom(Profile);
	writer.CR();

	WriteItem(WindowX);
	WriteItem(WindowY);
	WriteItem(WindowW);
	WriteItem(WindowH);
	WriteItem(ShowMenuBar);
	WriteItem(ShowNavBar);
	WriteItem(ShowImageDetails);
	WriteItem(ShowPixelEditor);
	WriteItem(ShowChannelFilter);
	WriteItem(ShowFrameScrubber);
	WriteItem(ContentViewShow);
	WriteItem(ThumbnailWidth);
	WriteItem(SortKey);
	WriteItem(SortAscending);
	WriteItem(OverlayCorner);
	WriteItem(Tile);
	WriteItem(BackgroundStyle);
	WriteItem(BackgroundColour);
	WriteItem(BackgroundExtend);
	WriteItem(TransparentWorkArea);
	WriteItem(FixedAspectWorkArea);
	WriteItem(ResampleFilter);
	WriteItem(ResampleEdgeMode);
	WriteItem(ResampleFilterRotateUp);
	WriteItem(ResampleFilterRotateDown);
	WriteItem(RotateMode);
	WriteItem(DefaultZoomMode);
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

	InputBindings.Write(writer);

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

	writer.Dedent();
	writer.CR();
	writer.End();

	return true;
}


}
