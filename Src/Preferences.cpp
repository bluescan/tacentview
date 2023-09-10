// Preferences.h
//
// Preferences window.
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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Preferences.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "ThumbnailView.h"
#include "Version.cmake.h"
using namespace tMath;


void Viewer::ShowPreferencesWindow(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = GetDialogOrigin(DialogID::Preferences);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	tString title;
	tsPrintf(title, "Preferences (%s Profile)", Config::GetProfileName());
	if (!ImGui::Begin(title.Chr(), popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	Config::ProfileSettings& config = *Config::Current;
	float buttonOffset, comboWidth;
	switch (config.GetUISize())
	{
		case Viewer::Config::ProfileSettings::UISizeEnum::Nano:
			buttonOffset	= 141.0f;
			comboWidth		= 110.0f;
			break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Tiny:
			buttonOffset	= 170.0f;
			comboWidth		= 132.0f;
			break;
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:
			buttonOffset	= 191.0f;
			comboWidth		= 146.0f;
			break;
	}

	bool tab = false;
	uint32 category = Config::Category_None;
	if (ImGui::BeginTabBar("PreferencesTabBar", ImGuiTabBarFlags_None))
	{
		tab = ImGui::BeginTabItem("Display", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_Display;
			ImGui::NewLine();
			ImGui::Checkbox("Transparent Work Area", &PendingTransparentWorkArea);
			#ifndef PACKAGE_SNAP
			if (PendingTransparentWorkArea != Config::Global.TransparentWorkArea)
			{
				ImGui::SameLine();
				ImGui::Text("(Needs Restart)");
			}
			#else
			if (PendingTransparentWorkArea)
			{
				ImGui::SameLine();
				ImGui::Text("(No Snap Support)");
			}
			#endif

			ImGui::Checkbox("Background Extend", &config.BackgroundExtend);
			if (!Config::Global.TransparentWorkArea)
			{
				const char* backgroundItems[] = { "None", "Checker", "Solid" };
				ImGui::PushItemWidth(comboWidth);
				ImGui::Combo("Background Style", &config.BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
				ImGui::PopItemWidth();

				if (config.GetBackgroundStyle() == Config::ProfileSettings::BackgroundStyleEnum::SolidColour)
				{
					tColourf floatCol(config.BackgroundColour);
					if (ImGui::ColorEdit3("Solid Colour", floatCol.E, ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar))
					{
						config.BackgroundColour.Set(floatCol);
						config.BackgroundColour.A = 0xFF;
					}

					int preset = 0;
					if (config.BackgroundColour == tColouri::black)
						preset = 1;
					else if (config.BackgroundColour == tColouri::lightgrey)
						preset = 2;
					else if (config.BackgroundColour == tColouri::white)
						preset = 3;

					ImGui::SameLine();
					const char* presetColours[] = { "Custom", "Black", "Grey", "White" };
					ImGui::PushItemWidth(comboWidth*0.64f);
					if (ImGui::Combo("Preset", &preset, presetColours, tNumElements(presetColours)))
					{
						switch (preset)
						{
							case 1:		config.BackgroundColour = tColouri::black;		break;
							case 2:		config.BackgroundColour = tColouri::lightgrey;	break;
							case 3:		config.BackgroundColour = tColouri::white;		break;
						}
					}
					ImGui::PopItemWidth();
				}

				if (config.GetBackgroundStyle() == Config::ProfileSettings::BackgroundStyleEnum::Checkerboard)
				{
					ImGui::PushItemWidth(comboWidth);
					ImGui::InputInt("Checker Size", &config.BackgroundCheckerboxSize);
					ImGui::PopItemWidth();
					tMath::tiClamp(config.BackgroundCheckerboxSize, 2, 256);
				}
			}

			// Reticle mode.
			const char* reticleModeItems[] = { "Always Hidden", "Always Visible", "On Select", "Auto Hide" };
			ImGui::PushItemWidth(comboWidth);
			ImGui::Combo("Reticle Mode", &config.ReticleMode, reticleModeItems, tNumElements(reticleModeItems));
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark
			(
				"Controls when the cursor reticle is visible.\n"
				"Always Hidden: Never display reticle. Driving blind.\n"
				"Always Visible: Never hide the reticle.\n"
				"On Select: Visible when click mouse. Hides when switch image or click outside image.\n"
				"Auto Hide: Hides after inactivity timeout."
			);
			#ifdef ALLOW_ALL_UI_SIZES
			const char* uiSizeItems[] = { "Nano", "Tiny", "Small", "Moderate", "Medium", "Large", "Huge", "Massive" };
			#else
			const char* uiSizeItems[] = { "Nano", "Tiny", "Small" };
			#endif
			tStaticAssert(tNumElements(uiSizeItems) == int(Config::ProfileSettings::UISizeEnum::NumSizes));

			ImGui::PushItemWidth(comboWidth);
			ImGui::Combo("UI Size", &config.UISize, uiSizeItems, tNumElements(uiSizeItems));
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Overall size of UI widgets and font.");

			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Slideshow", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_Slideshow;
			ImGui::NewLine();
			ImGui::PushItemWidth(110);
			if (ImGui::InputDouble("Period (s)", &config.SlideshowPeriod, 0.001f, 1.0f, "%.3f"))
			{
				tiClampMin(config.SlideshowPeriod, 1.0/60.0);
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}
			ImGui::PopItemWidth();
			if (ImGui::Button("8 s"))
			{
				config.SlideshowPeriod = 8.0;
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("4 s"))
			{
				config.SlideshowPeriod = 4.0;
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("1 s"))
			{
				config.SlideshowPeriod = 1.0;
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("10fps"))
			{
				config.SlideshowPeriod = 1.0/10.0;
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("30fps"))
			{
				config.SlideshowPeriod = 1.0/30.0;
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("60fps"))
			{
				config.SlideshowPeriod = 1.0/60.0;
				Viewer::SlideshowCountdown = config.SlideshowPeriod;
			}

			ImGui::Checkbox("Countdown Indicator", &config.SlideshowProgressArc);
			ImGui::SameLine();
			ShowHelpMark("Display a time remaining indicator when slideshow active.");

			ImGui::Checkbox("Auto Start", &config.SlideshowAutoStart);
			ImGui::SameLine();
			ShowHelpMark("Should slideshow start automatically on launch.");

			ImGui::Checkbox("Looping", &config.SlideshowLooping);
			ImGui::SameLine();
			ShowHelpMark("Should slideshow loop after completion.");

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
			ImGui::Separator();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

			Viewer::DoSortParameters(false);
			ImGui::Checkbox("Auto Reshuffle", &config.SlideshowAutoReshuffle);
			ImGui::SameLine();
			ShowHelpMark("If sort set to shuffle, reshuffle automatically after every loop.");

			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("System", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_System;
			ImGui::NewLine();

			ImGui::PushItemWidth(110);
			ImGui::InputInt("Max Undo Steps", &config.MaxUndoSteps); ImGui::SameLine();
			ShowHelpMark("Maximum number of undo steps.");
			tMath::tiClamp(config.MaxUndoSteps, 1, 32);

			ImGui::PushItemWidth(110);
			ImGui::InputInt("Max Mem (MB)", &config.MaxImageMemMB); ImGui::SameLine();
			ShowHelpMark("Approx memory use limit of this app. Minimum 256 MB.");
			tMath::tiClampMin(config.MaxImageMemMB, 256);

			ImGui::InputInt("Max Cache Files", &config.MaxCacheFiles); ImGui::SameLine();
			ShowHelpMark("Maximum number of cache files that may be created. Minimum 200.");
			tMath::tiClampMin(config.MaxCacheFiles, 200);
			if (!DeleteAllCacheFilesOnExit)
			{
				if (ImGui::Button("Clear Cache On Exit"))
					DeleteAllCacheFilesOnExit = true;
				ImGui::SameLine(); ShowHelpMark("Cache will be cleared on exit.");
			}
			else
			{
				if (ImGui::Button("Cancel Clear Cache"))
					DeleteAllCacheFilesOnExit = false;
				ImGui::SameLine(); ShowHelpMark("Cache will no longer be cleared on exit.");
			}

			if (ImGui::Button("Reset Bookmarks"))
				tFileDialog::Reset();
			ImGui::SameLine(); ShowHelpMark("Reset File Dialog Bookmarks.");

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
			ImGui::Separator();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

			ImGui::InputFloat("Gamma##Monitor", &config.MonitorGamma, 0.01f, 0.1f, "%.3f");
			ImGui::SameLine();
			ShowHelpMark("Some image property windows allow gamma correction and the gamma to be specified (eg. HDR DDS files).\nThis setting allows you to set a custom value for what the gamma will be reset to in those dialogs.\nResetting this tab always chooses the industry-standard gamm of 2.2");
			ImGui::PopItemWidth();

			ImGui::Checkbox("Strict Loading", &config.StrictLoading); ImGui::SameLine();
			ShowHelpMark
			(
				"Some image files are ill-formed. If strict is true these files are not loaded.\n"
				"Ill-formed jpg and dds files have been found in the wild that are ill-formed\n"
				"but still loadable. If strict is false, these files will still load."
			);

			// If the orient loading value changes we need to reload any images that have the Orientation tag set in their meta-data.
			// If the current image ends up not being unloaded, the 'Load' call exits immediately, so it's fast (i.e. it knows).
			if (ImGui::Checkbox("Exif Orient Loading", &config.ExifOrientLoading))
			{
				for (Image* i = Images.First(); i; i = i->Next())
					if (i->Cached_MetaData.IsValid() && i->Cached_MetaData[tImage::tMetaTag::Orientation].IsSet())
						i->Unload(true);

				CurrImage->Load();
			}
			ImGui::SameLine();
			ShowHelpMark("If Exif meta-data contains camera orientation information this will take it into account\nwhen loading and display the image the correctly oriented/flipped. Mostly affects jpg files.");

			ImGui::Checkbox("Detect APNG Inside PNG", &config.DetectAPNGInsidePNG); ImGui::SameLine();
			ShowHelpMark("Some png image files are really apng files. If detecton is true these png files will be displayed animated.");

			ImGui::Checkbox("Mipmap Chaining", &config.MipmapChaining); ImGui::SameLine();
			ShowHelpMark("Chaining generates mipmaps faster. No chaining gives slightly\nbetter results at cost of large generation time.");

			ImGui::PushItemWidth(comboWidth*1.22f);
			ImGui::Combo("Mip Filter", &config.MipmapFilter, tImage::tResampleFilterNames, 1+int(tImage::tResampleFilter::NumFilters), 1+int(tImage::tResampleFilter::NumFilters));
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Filtering method to use when generating minification mipmaps.\nUse None for no mipmapping.");

			tString pasteTypeName = config.ClipboardPasteFileType;
			tSystem::tFileType pasteType = tSystem::tGetFileTypeFromName(pasteTypeName);
			ImGui::PushItemWidth(comboWidth*1.22f);
			if (ImGui::BeginCombo("Paste Type", pasteTypeName.Chr()))
			{
				for (tSystem::tFileTypes::tFileTypeItem* item = FileTypes_ClipboardPaste.First(); item; item = item->Next())
				{
					tSystem::tFileType ft = item->FileType;
					bool selected = (ft == pasteType);

					tString ftName = tGetFileTypeName(ft);
					if (ImGui::Selectable(ftName.Chr(), &selected))
						config.ClipboardPasteFileType = ftName;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}				
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark
			(
				"When an image is pasted from the clipboard it creates a new image of this type.\n"
				"Valid types are ones that are lossless or support lossless encoding like webp.\n"
				"Pasted images support alpha channel. If no alpha it saves the image without it." 
			);
	
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Interface", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_Interface;
			ImGui::NewLine();
			ImGui::Checkbox("Confirm Deletes", &config.ConfirmDeletes);
			ImGui::Checkbox("Confirm File Overwrites", &config.ConfirmFileOverwrites);
			ImGui::Checkbox("Auto Property Window", &config.AutoPropertyWindow);
			ImGui::Checkbox("Auto Play Anims", &config.AutoPlayAnimatedImages);

			ImGui::Checkbox("Zoom Per Image", &config.ZoomPerImage);
			ImGui::EndTabItem();

			if (!config.ZoomPerImage)
			{
				const char* zoomModes[] = { "User", "Fit", "Downscale", "OneToOne" };
				ImGui::PushItemWidth(comboWidth*0.9f);
				int zoomMode = int(GetZoomMode());
				if (ImGui::Combo("Zoom Mode", &zoomMode, zoomModes, tNumElements(zoomModes)))
				{
					switch (Config::ProfileSettings::ZoomModeEnum(zoomMode))
					{
						case Config::ProfileSettings::ZoomModeEnum::Fit:
						case Config::ProfileSettings::ZoomModeEnum::DownscaleOnly:
							ResetPan();
							break;
						case Config::ProfileSettings::ZoomModeEnum::OneToOne:
							SetZoomPercent(100.0f);
							ResetPan();
							break;
					}
					SetZoomMode( Config::ProfileSettings::ZoomModeEnum(zoomMode) );
				}
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ShowHelpMark
				(
					"Controls what zoom to use when displaying images.\n"
					"User: User-specified. This mode is automatically turned on when zooming in/out.\n"
					"Fit: Image is zoomed to fit display area no matter its size.\n"
					"Downscale: Shows it at 100% zoom unless image is too big and needs downscaling.\n"
					"  This is the default. It keeps the full image always visible.\n"
					"OneToOne: One image pixel takes up one screen pixel."
				);

				float zoom = Viewer::GetZoomPercent();
				ImGui::PushItemWidth(comboWidth*0.9f);
				if (ImGui::InputFloat("Zoom Percent", &zoom, 0.01f, 0.1f, "%.3f"))
					Viewer::SetZoomPercent(zoom);
				ImGui::PopItemWidth();
			}
		}
		ImGui::EndTabBar();
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
	ImGui::Separator();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

	if (ImGui::Button("Reset Profile", tVector2(100.0f, 0.0f)))
	{
		Config::ResetProfile(Config::Category_AllNoBindings);
		SlideshowCountdown = config.SlideshowPeriod;
	}
	ShowToolTip
	(
		"Resets the current profile (excluding key-bindings) to defaults.\n"
		"Key-bindings may be reset from the Key Bindings window."
	);

	ImGui::SameLine();
	ImGui::SetCursorPosX(buttonOffset);
	if (ImGui::Button("Reset Tab", tVector2(100.0f, 0.0f)))
	{
		Config::ResetProfile(category);
		SlideshowCountdown = config.SlideshowPeriod;
	}
	ShowToolTip("Resets the current tab/category for the current profile (what you see above).");

	if (ImGui::Button("Reset All", tVector2(100.0f, 0.0f)))
	{
		Config::ResetAllProfiles(Config::Category_AllNoBindings);
		Config::Global.Reset();
		Config::SetProfile(Profile::Main);

		// If the global reset turns transparent work area off we can always safely clear the pending.
		if (!Config::Global.TransparentWorkArea)
			PendingTransparentWorkArea = false;
		SlideshowCountdown = config.SlideshowPeriod;
		ChangeScreenMode(config.FullscreenMode, true);
	}
	ShowToolTip
	(
		"Resets all profiles (excluding key bindings) to their default settings and switches\n"
		"to the main profile. Keybindings may be reset from the Key Bindings window."
	);

	ImGui::SameLine();
	ImGui::SetCursorPosX(buttonOffset);
	if (ImGui::Button("Close", tVector2(100.0f, 0.0f)))
	{
		if (popen)
			*popen = false;
	}
	ImGui::End();
}
