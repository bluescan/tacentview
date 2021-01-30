// Preferences.h
//
// Priferences window.
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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Preferences.h"
#include "Settings.h"
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

	if (!ImGui::Begin("Preferences", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	bool tab = false;
	if (ImGui::BeginTabBar("PreferencesTabBar", ImGuiTabBarFlags_None))
	{
		tab = ImGui::BeginTabItem("Background", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			ImGui::NewLine();
			ImGui::Checkbox("Transparent Work Area", &PendingTransparentWorkArea);
			if (PendingTransparentWorkArea != Config.TransparentWorkArea)
			{
				ImGui::SameLine();
				ImGui::Text("(Needs Restart)");
			}

			ImGui::Checkbox("Extend", &Config.BackgroundExtend);
			if (!Config.TransparentWorkArea)
			{
				const char* backgroundItems[] = { "None", "Checkerboard", "Black", "Grey", "White" };
				ImGui::PushItemWidth(110);
				ImGui::Combo("Style", &Config.BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
				ImGui::PopItemWidth();
			}
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Slideshow", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			ImGui::NewLine();
			ImGui::PushItemWidth(110);
			if (ImGui::InputDouble("Period (s)", &Config.SlideshowPeriod, 0.001f, 1.0f, "%.3f"))
			{
				tiClampMin(Config.SlideshowPeriod, 1.0/60.0);
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::PopItemWidth();
			if (ImGui::Button("8 s"))
			{
				Config.SlideshowPeriod = 8.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("4 s"))
			{
				Config.SlideshowPeriod = 4.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("1 s"))
			{
				Config.SlideshowPeriod = 1.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("10 fps"))
			{
				Config.SlideshowPeriod = 1.0/10.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("30 fps"))
			{
				Config.SlideshowPeriod = 1.0/30.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("60 fps"))
			{
				Config.SlideshowPeriod = 1.0/60.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
			}
			ImGui::Checkbox("Countdown Indicator", &Config.SlideshowProgressArc);
			if (ImGui::Button("Reset"))
			{
				Config.SlideshowPeriod = 8.0;
				Viewer::SlideshowCountdown = Config.SlideshowPeriod;
				Config.SlideshowProgressArc = true;
			}
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("System", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			ImGui::NewLine();
			ImGui::PushItemWidth(110);
			ImGui::InputInt("Max Mem (MB)", &Config.MaxImageMemMB); ImGui::SameLine();
			ShowHelpMark("Approx memory use limit of this app. Minimum 256 MB.");
			tMath::tiClampMin(Config.MaxImageMemMB, 256);
			ImGui::InputInt("Max Cache Files", &Config.MaxCacheFiles); ImGui::SameLine();
			ShowHelpMark("Maximum number of cache files that may be created. Minimum 200.");
			tMath::tiClampMin(Config.MaxCacheFiles, 200);
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
			ImGui::InputInt("Max Undo Steps", &Config.MaxUndoSteps); ImGui::SameLine();
			ShowHelpMark("Maximum number of Ctrl-Z undo steps.");
			tMath::tiClamp(Config.MaxUndoSteps, 1, 32);

			ImGui::Checkbox("Strict Loading", &Config.StrictLoading); ImGui::SameLine();
			ShowHelpMark("Some image files are ill-formed. If strict is true no attempt to display them is made.");

			ImGui::Checkbox("Detect APNG Inside PNG", &Config.DetectAPNGInsidePNG); ImGui::SameLine();
			ShowHelpMark("Some png image files are really apng files. If detecton is true these png files will be displayed animated.");

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::InputFloat("Monitor Gamma", &Config.MonitorGamma, 0.01f, 0.1f, "%.3f");
			ImGui::PopItemWidth();
			if (ImGui::Button("Reset Gamma"))
				Config.MonitorGamma = tMath::DefaultGamma;
	
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Interface", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			ImGui::NewLine();
			ImGui::Checkbox("Confirm Deletes", &Config.ConfirmDeletes);
			ImGui::Checkbox("Confirm File Overwrites", &Config.ConfirmFileOverwrites);
			ImGui::Checkbox("Auto Propery Window", &Config.AutoPropertyWindow);
			ImGui::Checkbox("Auto Play Anims", &Config.AutoPlayAnimatedImages);
			
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Reset Behaviour", tVector2(100, 0)))
	{
		Config.ResetBehaviourSettings();
	}
	ShowToolTip("Resets sort order, resample filters, confirmations, preferred file type, cache size, etc.");

	if (ImGui::Button("Reset UI", tVector2(100, 0)))
	{
		Config.ResetUISettings();
		PendingTransparentWorkArea = false;
		ChangeScreenMode(false, true);
	}
	ShowToolTip("Resets window dimensions/position, nav bar, content view, basic mode, tiling, background, details, etc.");

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("Close", tVector2(100, 0)))
	{
		if (popen)
			*popen = false;
	}
	ImGui::End();
}
