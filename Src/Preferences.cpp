// Preferences.h
//
// Priferences window.
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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Preferences.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "Version.cmake.h"
using namespace tMath;


void Viewer::ShowPreferencesWindow(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = GetDialogOrigin(2);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	tString title;
	tsPrintf(title, "Preferences (%s Profile)", (Config::GetProfile() == Config::Profile::Basic) ? "Basic" : "Main");
	if (!ImGui::Begin(title.Chars(), popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	bool tab = false;
	Config::Category category = Config::Category::Everything;
	if (ImGui::BeginTabBar("PreferencesTabBar", ImGuiTabBarFlags_None))
	{
		tab = ImGui::BeginTabItem("Display", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category::Display;
			ImGui::NewLine();
			ImGui::Checkbox("Transparent Work Area", &PendingTransparentWorkArea);
			#ifndef PACKAGE_SNAP
			if (PendingTransparentWorkArea != Config::Current->TransparentWorkArea)
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

			ImGui::Checkbox("Fixed Aspect Work Area", &Config::Current->FixedAspectWorkArea);

			ImGui::Checkbox("Background Extend", &Config::Current->BackgroundExtend);
			if (!Config::Current->TransparentWorkArea)
			{
				const char* backgroundItems[] = { "None", "Checkerboard", "Black", "Grey", "White" };
				ImGui::PushItemWidth(110);
				ImGui::Combo("Background Style", &Config::Current->BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
				ImGui::PopItemWidth();
			}
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Slideshow", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category::Slideshow;
			ImGui::NewLine();
			ImGui::PushItemWidth(110);
			if (ImGui::InputDouble("Period (s)", &Config::Current->SlideshowPeriod, 0.001f, 1.0f, "%.3f"))
			{
				tiClampMin(Config::Current->SlideshowPeriod, 1.0/60.0);
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::PopItemWidth();
			if (ImGui::Button("8 s"))
			{
				Config::Current->SlideshowPeriod = 8.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("4 s"))
			{
				Config::Current->SlideshowPeriod = 4.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("1 s"))
			{
				Config::Current->SlideshowPeriod = 1.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("10 fps"))
			{
				Config::Current->SlideshowPeriod = 1.0/10.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("30 fps"))
			{
				Config::Current->SlideshowPeriod = 1.0/30.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("60 fps"))
			{
				Config::Current->SlideshowPeriod = 1.0/60.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
			}
			ImGui::Checkbox("Countdown Indicator", &Config::Current->SlideshowProgressArc);
			if (ImGui::Button("Reset"))
			{
				Config::Current->SlideshowPeriod = 8.0;
				Viewer::SlideshowCountdown = Config::Current->SlideshowPeriod;
				Config::Current->SlideshowProgressArc = true;
			}
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("System", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category::System;
			ImGui::NewLine();
			ImGui::PushItemWidth(110);
			ImGui::InputInt("Max Mem (MB)", &Config::Current->MaxImageMemMB); ImGui::SameLine();
			ShowHelpMark("Approx memory use limit of this app. Minimum 256 MB.");
			tMath::tiClampMin(Config::Current->MaxImageMemMB, 256);
			ImGui::InputInt("Max Cache Files", &Config::Current->MaxCacheFiles); ImGui::SameLine();
			ShowHelpMark("Maximum number of cache files that may be created. Minimum 200.");
			tMath::tiClampMin(Config::Current->MaxCacheFiles, 200);
			if (!DeleteAllCacheFilesOnExit)
			{
				if (ImGui::Button("Clear Cache"))
					DeleteAllCacheFilesOnExit = true;
			}
			else
			{
				if (ImGui::Button("Cancel"))
					DeleteAllCacheFilesOnExit = false;
				ImGui::SameLine(); ImGui::Text("Cache will be cleared on exit.");
			}

			ImGui::PushItemWidth(110);
			ImGui::InputInt("Max Undo Steps", &Config::Current->MaxUndoSteps); ImGui::SameLine();
			ShowHelpMark("Maximum number of Ctrl-Z undo steps.");
			tMath::tiClamp(Config::Current->MaxUndoSteps, 1, 32);

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Checkbox("Strict Loading", &Config::Current->StrictLoading); ImGui::SameLine();
			ShowHelpMark("Some image files are ill-formed. If strict is true no attempt to display them is made.");

			ImGui::Checkbox("Detect APNG Inside PNG", &Config::Current->DetectAPNGInsidePNG); ImGui::SameLine();
			ShowHelpMark("Some png image files are really apng files. If detecton is true these png files will be displayed animated.");

			ImGui::Checkbox("Mipmap Chaining", &Config::Current->MipmapChaining); ImGui::SameLine();
			ShowHelpMark("Chaining generates mipmaps faster. No chaining gives slightly\nbetter results at cost of large generation time.");

			ImGui::Combo("Mipmap Filter", &Config::Current->MipmapFilter, tImage::tResampleFilterNames, int(tImage::tResampleFilter::NumFilters), int(tImage::tResampleFilter::NumFilters));
			ImGui::SameLine();
			ShowHelpMark("Filtering method to use when generating minification mipmaps.\nUse None for no mipmapping.");

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::InputFloat("Monitor Gamma", &Config::Current->MonitorGamma, 0.01f, 0.1f, "%.3f");
			ImGui::PopItemWidth();
			if (ImGui::Button("Reset Gamma"))
				Config::Current->MonitorGamma = tMath::DefaultGamma;
	
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Interface", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category::Interface;
			ImGui::NewLine();
			ImGui::Checkbox("Confirm Deletes", &Config::Current->ConfirmDeletes);
			ImGui::Checkbox("Confirm File Overwrites", &Config::Current->ConfirmFileOverwrites);
			ImGui::Checkbox("Auto Propery Window", &Config::Current->AutoPropertyWindow);
			ImGui::Checkbox("Auto Play Anims", &Config::Current->AutoPlayAnimatedImages);
			ImGui::Checkbox("Esc Key Can Quit", &Config::Current->EscCanQuit);

			const char* zoomModes[] = { "Keep", "Fit", "Downscale", "OneToOne" };
			ImGui::PushItemWidth(86);
			ImGui::Combo("Default Zoom Mode", &Config::Current->DefaultZoomMode, zoomModes, tNumElements(zoomModes));
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark
			(
				"Controls what zoom mode to use when displaying a new image.\n"
				"Keep: No change to zoom mode when switching images.\n"
				"Fit: Image is zoomed to fit display area no matter its size.\n"
				"Downscale: Shows it at 100% zoom unless image is too big and needs downscaling.\n"
				"OneToOne: One image pixel takes up one screen pixel."
			);
			
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset Profile", tVector2(100, 0)))
	{
		Config::ResetProfile();
		PendingTransparentWorkArea = false;
		ChangeScreenMode(false, true);
	}
	ShowToolTip("Resets the current profile to defaults.");

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Reset Cat", tVector2(100, 0)))
	{
		Config::ResetProfile(category);
		PendingTransparentWorkArea = false;
		ChangeScreenMode(false, true);
	}
	ShowToolTip("Resets the current category/tab for the current profile.");

	if (ImGui::Button("Reset All", tVector2(100, 0)))
	{
		Config::ResetAll();
		Config::SetProfile(Config::Profile::Main);
		PendingTransparentWorkArea = false;
		ChangeScreenMode(false, true);
	}
	ShowToolTip("Resets all profiles to their default settings\nand switches to the main profile.");

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("Close", tVector2(100, 0)))
	{
		if (popen)
			*popen = false;
	}
	ImGui::End();
}
