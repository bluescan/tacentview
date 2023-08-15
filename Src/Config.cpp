// Config.cpp
//
// Viewer settings stored as human-readable symbolic expressions.
//
// Copyright (c) 2019-2023 Tristan Grimmer.
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
#include <Image/tQuantize.h>
#include "Config.h"
#include "Image.h"
#include "FileDialog.h"
using namespace tMath;
#define ReadItem(name) case tHash::tHashCT(#name): name = e.Arg1(); break
#define	WriteItem(name) writer.Comp(#name, name)
#define	WriteLast(name) writer.Coms(#name, name)
namespace Viewer {


namespace Config
{
	const int ConfigFileVersion = 3;

	GlobalSettings Global;
	ProfileSettings MainProfileSettings(Profile::Main);
	ProfileSettings BasicProfileSettings(Profile::Basic);
	ProfileSettings KioskProfileSettings(Profile::Kiosk);
	ProfileSettings* Current = &MainProfileSettings;
}


void Config::SetProfile(Profile profile)
{
	if (profile == Profile(Global.CurrentProfile))
		return;

	switch (profile)
	{
		case Profile::Main:		Current = &MainProfileSettings;		break;
		case Profile::Basic:	Current = &BasicProfileSettings;	break;
		case Profile::Kiosk:	Current = &KioskProfileSettings;	break;
	}
	Global.CurrentProfile = int(profile);
}


Viewer::Profile Config::GetProfile()
{
	return Profile(Global.CurrentProfile);
}


const char* Config::GetProfileName()
{
	return GetProfileName(GetProfile());
}


void Config::ResetProfile(uint32 categories)
{
	Current->Reset(Profile(Global.CurrentProfile), categories);
}


void Config::ResetAllProfiles(uint32 categories)
{
	MainProfileSettings.Reset(Profile::Main, categories);
	BasicProfileSettings.Reset(Profile::Basic, categories);
	KioskProfileSettings.Reset(Profile::Kiosk, categories);
}


void Config::ResetAllProfiles()
{
	ResetAllProfiles(Category_All);
}


void Config::Save(const tString& filename)
{
	tExprWriter writer(filename);
	writer.Rem("Tacent View Configuration File");
	writer.CR();

	// We always save in most recent / current version.
	Global.ConfigVersion = ConfigFileVersion;
	Global.Save(writer);
	writer.CR();
	writer.CR();

	MainProfileSettings.Name = "MainProfile";
	MainProfileSettings.Save(writer);
	writer.CR();
	writer.CR();

	BasicProfileSettings.Name = "BasicProfile";
	BasicProfileSettings.Save(writer);
	writer.CR();
	writer.CR();

	KioskProfileSettings.Name = "KioskProfile";
	KioskProfileSettings.Save(writer);
	writer.CR();

	// Save the file dialog settings.
	tFileDialog::Save(writer, "FileDialog");
}


void Config::Load(const tString& filename)
{
	if (!tSystem::tFileExists(filename))
	{
		Global.Reset();
		ResetAllProfiles();
		Current = &MainProfileSettings;
		tFileDialog::Reset();
		return;
	}

	bool loadedGlobal		= false;
	bool loadedMainProfile	= false;
	bool loadedBasicProfile	= false;
	bool loadedKioskProfile	= false;
	bool loadedFileDialog	= false;
	tExprReader reader(filename);
	for (tExpr e = reader.First(); e.IsValid(); e = e.Next())
	{
		switch (e.Item0().Hash())
		{
			case tHash::tHashCT("Global"):
				Global.Load(e);
				loadedGlobal = true;
				break;

			case tHash::tHashCT("MainProfile"):
				MainProfileSettings.Load(e);
				loadedMainProfile = true;
				break;

			case tHash::tHashCT("BasicProfile"):
				BasicProfileSettings.Load(e);
				loadedBasicProfile = true;
				break;

			case tHash::tHashCT("KioskProfile"):
				KioskProfileSettings.Load(e);
				loadedKioskProfile = true;
				break;

			case tHash::tHashCT("FileDialog"):
				tFileDialog::Load(e, "FileDialog");
				loadedFileDialog = true;
				break;
		}
	}

	// If the config existed but any particular top-level expression was missing, we reset it.
	if (!loadedGlobal)
		Global.Reset();

	// These are correct. Basically the loaded bools will only be false if the information
	// wasn't present in the config file at all. If it's not there, reset the values.
	if (!loadedMainProfile)
		MainProfileSettings.Reset(Viewer::Profile::Main, Category_All);

	if (!loadedBasicProfile)
		BasicProfileSettings.Reset(Viewer::Profile::Basic, Category_All);

	if (!loadedKioskProfile)
		KioskProfileSettings.Reset(Viewer::Profile::Kiosk, Category_All);

	if (!loadedFileDialog)
		tFileDialog::Reset();

	// At this point the cfg file exists and has been loaded. However, even without a version number increase we want
	// to be able to support new operations than may have been added and have the key-bindings assigned. This is
	// possible in a generic way if the new operations bindings do not conflict with user-specified bindings. That is,
	// if the new operations default bindings do not have the key already reassigned to something else, we should
	// assign them here.
	bool onlyIfUnassigned = true;
	MainProfileSettings.InputBindings.Reset(Viewer::Profile::Main, onlyIfUnassigned);
	BasicProfileSettings.InputBindings.Reset(Viewer::Profile::Basic, onlyIfUnassigned);
	KioskProfileSettings.InputBindings.Reset(Viewer::Profile::Kiosk, onlyIfUnassigned);

	// Add stuff here if you care about what version you loaded from.
	if (Global.ConfigVersion <= 2)
	{
		Global.Reset();
		ResetAllProfiles();
	}

	// I think it makes sense to restore the currently selected profile when the config file is loaded.
	// This means if you were in Basic profile when you close, you will be in basic when you start the app again.
	switch (Profile(Global.CurrentProfile))
	{
		case Profile::Main:		Current = &MainProfileSettings;		break;
		case Profile::Basic:	Current = &BasicProfileSettings;	break;
		case Profile::Kiosk:	Current = &KioskProfileSettings;	break;
		default:				Current = &MainProfileSettings;		break;
	}
}


void Config::GlobalSettings::Save(tExprWriter& writer)
{
	writer.Begin();
	writer.Indent();
	writer.CR();
	writer.WriteAtom("Global");
	writer.CR();

	WriteItem(ConfigVersion);
	WriteItem(CurrentProfile);
	WriteItem(WindowX);
	WriteItem(WindowY);
	WriteItem(WindowW);
	WriteItem(WindowH);
	WriteItem(TransparentWorkArea);
	WriteItem(LastOpenPath);
	WriteLast(FullscreenMode);

	writer.Dedent();
	writer.CR();
	writer.End();
}


void Config::GlobalSettings::GetScreenSize(int& screenW, int& screenH)
{
	GLFWmonitor* monitor		= glfwGetPrimaryMonitor();
	const GLFWvidmode* mode		= monitor ? glfwGetVideoMode(monitor) : nullptr;
	screenW						= mode ? mode->width  : 1920;
	screenH						= mode ? mode->height : 1080;
}


void Config::GlobalSettings::Load(tExpression expr)
{
	for (tExpr e = expr.Item1(); e.IsValid(); e = e.Next())
	{
		switch (e.Command().Hash())
		{
			ReadItem(ConfigVersion);
			ReadItem(CurrentProfile);
			ReadItem(WindowX);
			ReadItem(WindowY);
			ReadItem(WindowW);
			ReadItem(WindowH);
			ReadItem(TransparentWorkArea);
			ReadItem(LastOpenPath);
			ReadItem(FullscreenMode);
		}
	}

	#ifdef PLATFORM_LINUX
	// No need to clamp to screen dimensions on Linux. It deals with making sure window is visible.
	tiClampMin(WindowW, 640);
	tiClampMin(WindowH, 360);
	#endif

	#ifdef PLATFORM_WINDOWS
	int screenW, screenH;
	GetScreenSize(screenW, screenH);
	tiClamp		(WindowW, 640, screenW);
	tiClamp		(WindowH, 360, screenH);
	tiClamp		(WindowX, 0, screenW - WindowW);
	tiClamp		(WindowY, 0, screenH - WindowH);
	#endif
}


void Config::GlobalSettings::Reset()
{
	ConfigVersion = ConfigFileVersion;
	CurrentProfile = int(Profile::Main);

	int screenW, screenH;
	GetScreenSize(screenW, screenH);
	WindowW						= 1280;
	WindowH						= 720;						//	Sub 37 if want 720 inc title bar;
	WindowX						= (screenW - WindowW) >> 1;
	WindowY						= (screenH - WindowH) >> 1;
	TransparentWorkArea			= false;
	LastOpenPath				.Clear();
	FullscreenMode				= false;
}


void Config::ProfileSettings::Reset(Viewer::Profile profile, uint32 categories)
{
	if (categories & Category_Unspecified)
	{
		ShowMenuBar					= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? false : true;
		ShowNavBar					= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? false : true;
		ShowImageDetails			= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? false : true;
		ShowImageMetaData			= false;
		ShowPixelEditor				= false;
		ShowPreferences				= false;
		ShowChannelFilter			= false;
		ShowFrameScrubber			= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? false : true;
		ShowThumbnailView			= false;
		ShowPropsWindow				= false;
		ShowBindingsWindow			= false;
		ShowCheatSheet				= false;
		ShowAbout					= false;

		ThumbnailWidth				= 128.0f;
		OverlayCorner				= 1;
		Tile						= false;
		ResampleFilter				= int(tImage::tResampleFilter::Bilinear);
		ResampleEdgeMode			= int(tImage::tResampleEdgeMode::Clamp);
		ResampleFilterContactFrame	= int(tImage::tResampleFilter::Bilinear);
		ResampleEdgeModeContactFrame= int(tImage::tResampleEdgeMode::Clamp);
		ResampleFilterContactFinal	= int(tImage::tResampleFilter::Bilinear);
		ResampleEdgeModeContactFinal= int(tImage::tResampleEdgeMode::Clamp);
		ResampleFilterRotateUp		= int(tImage::tResampleFilter::Bilinear);
		ResampleFilterRotateDown	= int(tImage::tResampleFilter::None);
		RotateMode					= int(RotateModeEnum::Fill);

		SaveSubFolder				.Clear();
		SaveFileType				.Set(tSystem::tGetFileTypeName(tSystem::tFileType::TGA));
		SaveFileTypeMultiFrame		.Set(tSystem::tGetFileTypeName(tSystem::tFileType::GIF));
		SaveFileTargaRLE			= false;
		SaveFilePngDepthMode		= 0;
		SaveFileQoiDepthMode		= 0;
		SaveFileQoiColourSpace		= 0;
		SaveFileBmpDepthMode		= 0;
		SaveFileJpegQuality			= 95;
		SaveFileWebpLossy			= false;
		SaveFileWebpQualComp		= 90.0f;
		SaveFileTiffZLibDeflate		= true;
		SaveFileGifBPP				= 8;
		SaveFileGifQuantMethod		= int(tImage::tQuantize::Method::Wu);
		SaveFileGifLoop				= 0;
		SaveFileGifAlphaThreshold	= -1;
		SaveFileGifDitherLevel		= 0.0f;
		SaveFileGifFilterSize		= 1;
		SaveFileGifSampleFactor		= 1;
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
		CropAspectRatio				= int(tImage::tAspectRatio::Free);
		CropAspectUserNum			= 16;
		CropAspectUserDen			= 9;
		FillColour					= tColouri::black;
		ResizeAspectRatio			= int(tImage::tAspectRatio::Screen_16_9) - 1;
		ResizeAspectUserNum			= 16;
		ResizeAspectUserDen			= 9;
		ResizeAspectMode			= 0;
	}

	if (categories & Category_Display)
	{
		BackgroundStyle				= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? int(BackgroundStyleEnum::None) : int(BackgroundStyleEnum::Checkerboard);
		BackgroundCheckerboxSize	= 16;
		BackgroundColour			= tColouri::black;
		BackgroundExtend			= false;
		switch (profile)
		{
			default:
			case Profile::Main:		ReticleMode = int(ReticleModeEnum::OnSelect);		break;
			case Profile::Basic:	ReticleMode = int(ReticleModeEnum::AutoHide);		break;
			case Profile::Kiosk:	ReticleMode = int(ReticleModeEnum::AlwaysHidden);	break;
		}
		UISize						= (profile == Profile::Kiosk) ? int(UISizeEnum::Large) : int(UISizeEnum::Medium);
	}

	if (categories & Category_Slideshow)
	{
		SlideshowAutoStart			= (profile == Profile::Kiosk) ? true : false;
		SlideshowLooping			= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? true : false;
		SlideshowProgressArc		= true;
		switch (profile)
		{
			default:				
			case Profile::Main:		SlideshowPeriod = 4.0f;		break;
			case Profile::Basic:	SlideshowPeriod = 8.0f;		break;
			case Profile::Kiosk:	SlideshowPeriod = 16.0f;	break;
		}

		SortKey						= (profile == Profile::Kiosk) ? int(SortKeyEnum::Shuffle) : int(SortKeyEnum::FileName);
		SortAscending				= true;
		SlideshowAutoReshuffle		= true;
	}

	if (categories & Category_System)
	{
		MaxImageMemMB				= 2048;
		MaxCacheFiles				= 8192;
		MaxUndoSteps				= 16;
		StrictLoading				= false;
		ExifOrientLoading			= true;
		DetectAPNGInsidePNG			= true;
		MipmapFilter				= int(tImage::tResampleFilter::Bilinear);
		MipmapChaining				= true;
		ClipboardPasteFileType		.Set(tSystem::tGetFileTypeName(tSystem::tFileType::PNG));
		MonitorGamma				= tMath::DefaultGamma;
	}

	if (categories & Category_Interface)
	{
		ConfirmDeletes				= true;
		ConfirmFileOverwrites		= true;
		AutoPropertyWindow			= (profile == Profile::Basic) || (profile == Profile::Kiosk) ? false : true;
		AutoPlayAnimatedImages		= true;
		ZoomMode					= (profile == Profile::Kiosk) ? int(ZoomModeEnum::Fit) : int(ZoomModeEnum::DownscaleOnly);
		ZoomPercent					= 100.0f;
		ZoomPerImage				= (profile == Profile::Kiosk) ? false : true;
	}

	if (categories & Category_Bindings)
	{
		InputBindings				.Reset(profile);
	}
}


void Config::ProfileSettings::Load(tExpression expr)
{
	for (tExpr e = expr.Item1(); e.IsValid(); e = e.Next())
	{
		switch (e.Command().Hash())
		{
			ReadItem(ShowMenuBar);
			ReadItem(ShowNavBar);
			ReadItem(ShowImageDetails);
			ReadItem(ShowImageMetaData);
			ReadItem(ShowPixelEditor);
			ReadItem(ShowPreferences);
			ReadItem(ShowChannelFilter);
			ReadItem(ShowFrameScrubber);
			ReadItem(ShowThumbnailView);
			ReadItem(ShowPropsWindow);
			ReadItem(ShowBindingsWindow);
			ReadItem(ShowCheatSheet);
			ReadItem(ShowAbout);
			ReadItem(ThumbnailWidth);
			ReadItem(SortKey);
			ReadItem(SortAscending);
			ReadItem(OverlayCorner);
			ReadItem(Tile);
			ReadItem(BackgroundStyle);
			ReadItem(BackgroundCheckerboxSize);
			ReadItem(BackgroundColour);
			ReadItem(BackgroundExtend);
			ReadItem(ReticleMode);
			ReadItem(UISize);
			ReadItem(ResampleFilter);
			ReadItem(ResampleEdgeMode);
			ReadItem(ResampleFilterContactFrame);
			ReadItem(ResampleEdgeModeContactFrame);
			ReadItem(ResampleFilterContactFinal);
			ReadItem(ResampleEdgeModeContactFinal);
			ReadItem(ResampleFilterRotateUp);
			ReadItem(ResampleFilterRotateDown);
			ReadItem(RotateMode);
			ReadItem(ZoomMode);
			ReadItem(ZoomPercent);
			ReadItem(ZoomPerImage);
			ReadItem(ConfirmDeletes);
			ReadItem(ConfirmFileOverwrites);
			ReadItem(SlideshowAutoStart);
			ReadItem(SlideshowLooping);
			ReadItem(SlideshowProgressArc);
			ReadItem(SlideshowAutoReshuffle);
			ReadItem(SlideshowPeriod);
			ReadItem(ClipboardPasteFileType);

			ReadItem(SaveSubFolder);
			ReadItem(SaveFileType);
			ReadItem(SaveFileTypeMultiFrame);
			ReadItem(SaveFileTargaRLE);
			ReadItem(SaveFilePngDepthMode);
			ReadItem(SaveFileQoiDepthMode);
			ReadItem(SaveFileQoiColourSpace);
			ReadItem(SaveFileBmpDepthMode);
			ReadItem(SaveFileJpegQuality);
			ReadItem(SaveFileWebpLossy);
			ReadItem(SaveFileWebpQualComp);
			ReadItem(SaveFileTiffZLibDeflate);

			ReadItem(SaveFileGifBPP);
			ReadItem(SaveFileGifQuantMethod);
			ReadItem(SaveFileGifLoop);
			ReadItem(SaveFileGifAlphaThreshold);
			ReadItem(SaveFileGifDitherLevel);
			ReadItem(SaveFileGifFilterSize);
			ReadItem(SaveFileGifSampleFactor);

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
			ReadItem(CropAspectRatio);
			ReadItem(CropAspectUserNum);
			ReadItem(CropAspectUserDen);
			ReadItem(FillColour);
			ReadItem(ResizeAspectRatio);
			ReadItem(ResizeAspectUserNum);
			ReadItem(ResizeAspectUserDen);
			ReadItem(ResizeAspectMode);
			ReadItem(MaxImageMemMB);
			ReadItem(MaxCacheFiles);
			ReadItem(MaxUndoSteps);
			ReadItem(StrictLoading);
			ReadItem(ExifOrientLoading);
			ReadItem(DetectAPNGInsidePNG);
			ReadItem(MipmapFilter);
			ReadItem(MipmapChaining);
			ReadItem(AutoPropertyWindow);
			ReadItem(AutoPlayAnimatedImages);
			ReadItem(MonitorGamma);
		}
	}

	tiClamp		(ResampleFilter, 0, int(tImage::tResampleFilter::NumFilters)-1);					// No None allowed.
	tiClamp		(ResampleEdgeMode, 0, int(tImage::tResampleEdgeMode::NumEdgeModes)-1);				// No none allowed.
	tiClamp		(ResampleFilterContactFrame, 0, int(tImage::tResampleFilter::NumFilters)-1);		// No None allowed.
	tiClamp		(ResampleEdgeModeContactFrame, 0, int(tImage::tResampleEdgeMode::NumEdgeModes)-1);	// No None allowed.
	tiClamp		(ResampleFilterContactFinal, 0, int(tImage::tResampleFilter::NumFilters)-1);		// No None allowed.
	tiClamp		(ResampleEdgeModeContactFinal, 0, int(tImage::tResampleEdgeMode::NumEdgeModes)-1);	// No None allowed.
	tiClamp		(ResampleFilterRotateUp, 0, int(tImage::tResampleFilter::NumFilters));				// None allowed, so 1 bigger.
	tiClamp		(ResampleFilterRotateDown, 0, int(tImage::tResampleFilter::NumFilters));			// None allowed, so 1 bigger.
	tiClamp		(RotateMode, 0, int(RotateModeEnum::NumModes)-1);
	tiClamp		(ZoomMode, 0, int(ZoomModeEnum::NumModes)-1);
	tiClamp		(ZoomPercent, Config::ZoomMin, Config::ZoomMax);
	tiClampMin	(SlideshowPeriod, 1.0/60.0);
	tiClamp		(BackgroundStyle, 0, int(BackgroundStyleEnum::NumStyles)-1);
	tiClamp		(BackgroundCheckerboxSize, 2, 256);
	tiClamp		(ReticleMode, 0, int(ReticleModeEnum::NumModes)-1);
	tiClamp		(UISize, 0, int(UISizeEnum::NumModes)-1);
	tiClamp		(OverlayCorner, 0, 3);
	tiClamp		(ThumbnailWidth, float(Image::ThumbMinDispWidth), float(Image::ThumbWidth));
	tiClamp		(SortKey, 0, int(SortKeyEnum::NumKeys)-1);
	tiClamp		(CropAnchor, -1, 9);
	tiClamp		(CropAspectRatio, int(tImage::tAspectRatio::Free), int(tImage::tAspectRatio::User));
	tiClamp		(CropAspectUserNum, 1, 99);
	tiClamp		(CropAspectUserDen, 1, 99);
	tiClamp		(ResizeAspectRatio, 0, int(tImage::tAspectRatio::User)-1);
	tiClamp		(ResizeAspectUserNum, 1, 99);
	tiClamp		(ResizeAspectUserDen, 1, 99);
	tiClamp		(ResizeAspectMode, 0, 1);
	tiClampMin	(MaxImageMemMB, 256);
	tiClampMin	(MaxCacheFiles, 200);	
	tiClamp		(MaxUndoSteps, 1, 32);
	tiClamp		(MipmapFilter, 0, int(tImage::tResampleFilter::NumFilters));						// None allowed.
	tiClamp		(SaveAllSizeMode, 0, int(SizeModeEnum::NumModes)-1);
	tiClamp		(SaveFileJpegQuality, 1, 100);
	tiClamp		(SaveFileWebpQualComp, 0.0f, 100.0f);
	tiClamp		(SaveFileGifBPP, 1, 8);
	tiClamp		(SaveFileGifQuantMethod, 0, int(tImage::tQuantize::Method::NumMethods)-1);
	tiClampMin	(SaveFileGifLoop, 0);
	tiClamp		(SaveFileGifAlphaThreshold, -1, 255);
	tiClampMin	(SaveFileGifDitherLevel, 0.0f);
	tiClamp		(SaveFileGifFilterSize, 0, 2);
	tiClamp		(SaveFileGifSampleFactor, 1, 10);

	tiClampMin	(SaveFileWebpDurOverride, -1);
	tiClampMin	(SaveFileGifDurOverride, -1);
	tiClamp		(SaveFileApngDurOverride, -1, 65535);
	tiClampMin	(SaveFileTiffDurOverride, -1);

	tiClampMin	(SaveFileWebpDurMultiFrame, 0);
	tiClampMin	(SaveFileGifDurMultiFrame, 0);
	tiClamp		(SaveFileApngDurMultiFrame, 0, 65535);
	tiClampMin	(SaveFileTiffDurMultiFrame, 0);
}


bool Config::ProfileSettings::Save(tExprWriter& writer) const
{
	writer.Begin();
	writer.Indent();
	writer.CR();
	writer.WriteAtom(Name);
	writer.CR();

	WriteItem(ShowMenuBar);
	WriteItem(ShowNavBar);
	WriteItem(ShowImageDetails);
	WriteItem(ShowImageMetaData);
	WriteItem(ShowPixelEditor);
	WriteItem(ShowPreferences);
	WriteItem(ShowChannelFilter);
	WriteItem(ShowFrameScrubber);
	WriteItem(ShowThumbnailView);
	WriteItem(ShowPropsWindow);
	WriteItem(ShowBindingsWindow);
	WriteItem(ShowCheatSheet);
	WriteItem(ShowAbout);
	WriteItem(ThumbnailWidth);
	WriteItem(SortKey);
	WriteItem(SortAscending);
	WriteItem(OverlayCorner);
	WriteItem(Tile);
	WriteItem(BackgroundStyle);
	WriteItem(BackgroundCheckerboxSize);
	WriteItem(BackgroundColour);
	WriteItem(BackgroundExtend);
	WriteItem(ReticleMode);
	WriteItem(UISize);
	WriteItem(ResampleFilter);
	WriteItem(ResampleEdgeMode);
	WriteItem(ResampleFilterContactFrame);
	WriteItem(ResampleEdgeModeContactFrame);
	WriteItem(ResampleFilterContactFinal);
	WriteItem(ResampleEdgeModeContactFinal);
	WriteItem(ResampleFilterRotateUp);
	WriteItem(ResampleFilterRotateDown);
	WriteItem(RotateMode);
	WriteItem(ZoomMode);
	WriteItem(ZoomPercent);
	WriteItem(ZoomPerImage);
	WriteItem(ConfirmDeletes);
	WriteItem(ConfirmFileOverwrites);
	WriteItem(SlideshowAutoStart);
	WriteItem(SlideshowLooping);
	WriteItem(SlideshowProgressArc);
	WriteItem(SlideshowAutoReshuffle);
	WriteItem(SlideshowPeriod);
	WriteItem(ClipboardPasteFileType);

	WriteItem(SaveSubFolder);
	WriteItem(SaveFileType);
	WriteItem(SaveFileTypeMultiFrame);
	WriteItem(SaveFileTargaRLE);
	WriteItem(SaveFilePngDepthMode);
	WriteItem(SaveFileQoiDepthMode);
	WriteItem(SaveFileQoiColourSpace);
	WriteItem(SaveFileBmpDepthMode);
	WriteItem(SaveFileJpegQuality);
	WriteItem(SaveFileWebpLossy);
	WriteItem(SaveFileWebpQualComp);
	WriteItem(SaveFileTiffZLibDeflate);

	WriteItem(SaveFileGifBPP);
	WriteItem(SaveFileGifQuantMethod);
	WriteItem(SaveFileGifLoop);
	WriteItem(SaveFileGifAlphaThreshold);
	WriteItem(SaveFileGifDitherLevel);
	WriteItem(SaveFileGifFilterSize);
	WriteItem(SaveFileGifSampleFactor);

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
	WriteItem(CropAspectRatio);
	WriteItem(CropAspectUserNum);
	WriteItem(CropAspectUserDen);
	WriteItem(FillColour);
	WriteItem(ResizeAspectRatio);
	WriteItem(ResizeAspectUserNum);
	WriteItem(ResizeAspectUserDen);
	WriteItem(ResizeAspectMode);
	WriteItem(MaxImageMemMB);
	WriteItem(MaxCacheFiles);
	WriteItem(MaxUndoSteps);
	WriteItem(StrictLoading);
	WriteItem(ExifOrientLoading);
	WriteItem(DetectAPNGInsidePNG);
	WriteItem(MipmapFilter);
	WriteItem(MipmapChaining);
	WriteItem(AutoPropertyWindow);
	WriteItem(AutoPlayAnimatedImages);
	WriteLast(MonitorGamma);

	writer.Dedent();
	writer.CR();
	writer.End();

	return true;
}


}
