// Dialogs.cpp
//
// Various dialogs and helpers including a log window, info overlay, cheatsheet, help window, about window, and prefs.
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

#include <Foundation/tVersion.h>
#include <Math/tVector2.h>
#include <Math/tColour.h>
#include <Image/tImageHDR.h>
#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <../../Contrib/OpenEXR/config/OpenEXRConfig.h>
#include <../../Contrib/OpenEXR/zlib/zlib.h>
#include "imgui.h"
#include "Dialogs.h"
#include "Settings.h"
#include "TacitImage.h"
#include "TacitTexView.h"
using namespace tMath;


void TexView::ShowImageDetailsOverlay(bool* popen, float x, float y, float w, float h, int cursorX, int cursorY, float zoom)
{
	// This overlay function is pretty much taken from the DearImGui demo code.
	const float margin = 6.0f;

	tVector2 windowPos = tVector2
	(
		x + ((Config.OverlayCorner & 1) ? w - margin : margin),
		y + ((Config.OverlayCorner & 2) ? h - margin : margin)
	);
	tVector2 windowPivot = tVector2
	(
		(Config.OverlayCorner & 1) ? 1.0f : 0.0f,
		(Config.OverlayCorner & 2) ? 1.0f : 0.0f
	);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("ImageDetails", popen, flags))
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX()+30);
		ImGui::Text("Image   Details         ");

		ShowToolTip("Right-Click to Change Anchor");
		ImGui::Separator();

		if (CurrImage)
		{
			tColourf floatCol(PixelColour);
			tVector4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
			if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, tVector2(15,15)))
				ImGui::OpenPopup("CopyColourOverlayAs");

			if (ImGui::BeginPopup("CopyColourOverlayAs"))
				ColourCopyAs();

			ImGui::SameLine(); ImGui::Text("(%d, %d, %d, %d)", PixelColour.R, PixelColour.G, PixelColour.B, PixelColour.A);

			TacitImage::ImgInfo& info = CurrImage->Info;
			int bpp = tImage::tGetBitsPerPixel(info.SrcPixelFormat);
			if (info.IsValid())
			{
				ImGui::Text("Size: %dx%d", CurrImage->GetWidth(), CurrImage->GetHeight());
				ImGui::Text("Format: %s", tImage::tGetPixelFormatName(info.SrcPixelFormat));
				if (bpp > 0)
					ImGui::Text("Bits Per Pixel: %d", bpp);
				else
					ImGui::Text("Bits Per Pixel: --");
				ImGui::Text("Opaque: %s", info.Opaque ? "true" : "false");
				ImGui::Text("Parts: %d", CurrImage->GetNumParts());
				tString sizeStr; tsPrintf(sizeStr, "File Size: %'d", info.FileSizeBytes);
				ImGui::Text(sizeStr.Chars());
				ImGui::Text("Cursor: (%d, %d)", cursorX, cursorY);
				ImGui::Text("Zoom: %.0f%%", zoom);
			}
		}
		ImGui::Text("Images In Folder: %d", Images.GetNumItems());

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left",		nullptr, Config.OverlayCorner == 0)) Config.OverlayCorner = 0;
			if (ImGui::MenuItem("Top-right",	nullptr, Config.OverlayCorner == 1)) Config.OverlayCorner = 1;
			if (ImGui::MenuItem("Bottom-left",  nullptr, Config.OverlayCorner == 2)) Config.OverlayCorner = 2;
			if (ImGui::MenuItem("Bottom-right", nullptr, Config.OverlayCorner == 3)) Config.OverlayCorner = 3;
			if (popen && ImGui::MenuItem("Close")) *popen = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}


void TexView::ShowCheatSheetPopup(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(4);
	ImGui::SetNextWindowBgAlpha(0.80f);
	ImGui::SetNextWindowSize(tVector2(300.0f, 400.0f));
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin("Cheat Sheet", popen, flags))
	{
		float col = ImGui::GetCursorPosX() + 80.0f;
		ImGui::Text("Left Arrow");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Previous Image");
		ImGui::Text("Right Arrow");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image");
		ImGui::Text("Ctrl-Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Skip to First Image");
		ImGui::Text("Ctrl-Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Skip to Last Image");
		ImGui::Text("Alt-Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Previous Image Frame");
		ImGui::Text("Alt-Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image Frame");
		ImGui::Text("Space");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image");
		ImGui::Text("Ctrl +");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom In");
		ImGui::Text("Ctrl -");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Out");
		ImGui::Text("F1");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Cheat Sheet");
		ImGui::Text("F5");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Refresh/Reload Image");
		ImGui::Text("F11");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Fullscreen");
		ImGui::Text("Alt-Enter");   ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Fullscreen");
		ImGui::Text("Esc");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Exit Fullscreen");
		ImGui::Text("Tab");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Open Explorer Window");
		ImGui::Text("Delete");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Delete Current Image");
		ImGui::Text("Shift-Delete");ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Delete Current Image Permanently");
		ImGui::Text("LMB-Click");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Set Colour Reticle Pos");
		ImGui::Text("RMB-Drag");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Pan Image");
		ImGui::Text("Alt-F4");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Quit");
		ImGui::Text("Ctrl-S");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save As...");
		ImGui::Text("Alt-S");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save All...");

		ImGui::Text("Ctrl <");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Flip Vertically");
		ImGui::Text("Ctrl >");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Flip Horizontally");
		ImGui::Text("<");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Rotate Anti-Clockwise");
		ImGui::Text(">");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Rotate Clockwise");
		ImGui::Text("/");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Crop");

		ImGui::Text("R");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Refresh/Reload Image");
		ImGui::Text("I");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Image Details");
		ImGui::Text("T");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Tile");
		ImGui::Text("N");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Nav Bar");
		ImGui::Text("F");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Fit");
		ImGui::Text("L");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Show Debug Log");
		ImGui::Text("D");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Downscale Only");
		ImGui::Text("Z");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom 1:1 Pixels");
		ImGui::Text("C");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Contact Sheet...");
		ImGui::Text("P");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Preferences...");
		ImGui::Text("V");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Content Thumbnail View...");
	}
	ImGui::End();
}


void TexView::ShowAboutPopup(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(3);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("About", popen, flags))
	{
		int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
		glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
		ImGui::Text("Tacit Viewer V %d.%d.%d by Tristan Grimmer", TexView::MajorVersion, TexView::MinorVersion, TexView::Revision);
		ImGui::Separator();
		ImGui::Text("The following amazing and liberally licenced frameworks are used by this tool.");
		ImGui::Text("Dear ImGui V %s", IMGUI_VERSION);
		#ifdef PLATFORM_WINDOWS
		ImGui::Text("GLEW V %s", glewGetString(GLEW_VERSION));
		#else
		ImGui::Text("GLAD V %s", glad_glGetString(GL_VERSION));
		#endif
		ImGui::Text("GLFW V %d.%d.%d", glfwMajor, glfwMinor, glfwRev);
		ImGui::Text("Tacent Library V %d.%d.%d", tVersion::Major, tVersion::Minor, tVersion::Revision);
		ImGui::Text("CxImage");
		ImGui::Text("nVidia Texture Tools");
		ImGui::Text("Ionicons");
		ImGui::Text("Roboto Google Font");
		ImGui::Text("Radiance Software");
		ImGui::Text("OpenEXR V %s", OPENEXR_VERSION_STRING);
		ImGui::Text("ZLib V %s", ZLIB_VERSION);
		ImGui::Text("Gif Load");
	}
	ImGui::End();
}


void TexView::ShowPropertyEditorWindow(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = GetDialogOrigin(0);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Image Property Editor", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	if (!CurrImage)
	{
		ImGui::Text("No Images in Folder");
		ImGui::End();
		return;
	}

	bool fileTypeSectionDisplayed = false;
	switch (CurrImage->Filetype)
	{
		case tSystem::tFileType::HDR:
		{
			ImGui::Text("Radiance HDR");
			ImGui::Indent();
			ImGui::PushItemWidth(110);

			ImGui::InputFloat("Gamma Correction", &CurrImage->LoadParams.GammaValue, 0.01f, 0.1f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0] if you reload this Radiance hdr file. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams.GammaValue, 0.6f, 3.0f);

			ImGui::InputInt("Exposure Adj", &CurrImage->LoadParams.HDR_Exposure); ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10, 10] if you reload this Radiance hdr file.");
			tMath::tiClamp(CurrImage->LoadParams.HDR_Exposure, -10, 10);

			ImGui::PopItemWidth();
			if (ImGui::Button("Reset"))
				CurrImage->ResetLoadParams();
			ImGui::SameLine();

			if (ImGui::Button("Reload"))
			{
				CurrImage->Unload();
				CurrImage->Load();
			}
			ImGui::SameLine();

			if (ImGui::Button("Reload All"))
			{
				tImage::tPicture::LoadParams params = CurrImage->LoadParams;
				for (TacitImage* img = Images.First(); img; img = img->Next())
				{
					if (img->Filetype != tSystem::tFileType::HDR)
						continue;
					img->Unload();
					img->LoadParams = params;
					img->Load();
				}
			}

			ImGui::Unindent();
			fileTypeSectionDisplayed = true;
			break;
		}

		case tSystem::tFileType::EXR:
		{
			ImGui::Separator();
			ImGui::Text("Open EXR");
			ImGui::Indent();
			ImGui::PushItemWidth(110);

			ImGui::InputFloat("Gamma", &CurrImage->LoadParams.GammaValue, 0.01f, 0.1f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0] if you reload this exr file. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams.GammaValue, 0.6f, 3.0f);

			ImGui::InputFloat("Exposure", &CurrImage->LoadParams.EXR_Exposure, 0.01f, 0.1f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10.0, 10.0] if you reload this exr file.");
			tMath::tiClamp(CurrImage->LoadParams.EXR_Exposure, -10.0f, 10.0f);

			ImGui::InputFloat("Defog", &CurrImage->LoadParams.EXR_Defog, 0.001f, 0.01f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Remove fog strength [0.0, 0.1] if you reload this exr file. Try to keep under 0.01");
			tMath::tiClamp(CurrImage->LoadParams.EXR_Defog, 0.0f, 0.1f);

			ImGui::InputFloat("Knee Low", &CurrImage->LoadParams.EXR_KneeLow, 0.01f, 0.1f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Lower bound knee taper [-3.0, 3.0] if you reload this exr file.");
			tMath::tiClamp(CurrImage->LoadParams.EXR_KneeLow, -3.0f, 3.0f);

			ImGui::InputFloat("Knee High", &CurrImage->LoadParams.EXR_KneeHigh, 0.01f, 0.1f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Upper bound knee taper [3.5, 7.5] if you reload this exr file.");
			tMath::tiClamp(CurrImage->LoadParams.EXR_KneeHigh, 3.5f, 7.5f);

			ImGui::PopItemWidth();
			if (ImGui::Button("Reset"))
				CurrImage->ResetLoadParams();
			ImGui::SameLine();

			if (ImGui::Button("Reload"))
			{
				CurrImage->Unload();
				CurrImage->Load();
			}
			ImGui::SameLine();

			if (ImGui::Button("Reload All"))
			{
				tImage::tPicture::LoadParams params = CurrImage->LoadParams;
				for (TacitImage* img = Images.First(); img; img = img->Next())
				{
					if (img->Filetype != tSystem::tFileType::EXR)
						continue;
					img->Unload();
					img->LoadParams = params;
					img->Load();
				}
			}

			ImGui::Unindent();
			fileTypeSectionDisplayed = true;
			break;
		}
	}

	int numParts = CurrImage->GetNumParts();
	if ((numParts <= 1) && !fileTypeSectionDisplayed)
		ImGui::Text("No Editable Image Properties Available");

	if (numParts > 1)
	{
		if (fileTypeSectionDisplayed)
			ImGui::Separator();

		ImGui::Text("%d-Part Image", CurrImage->GetNumParts());
		ImGui::Indent();
		ImGui::PushItemWidth(110);

		int oneBasedPartNum = CurrImage->PartNum + 1;
		if (ImGui::InputInt("Part", &oneBasedPartNum))
		{
			CurrImage->PartNum = oneBasedPartNum - 1;
			tMath::tiClamp(CurrImage->PartNum, 0, CurrImage->GetNumParts()-1);
		}
		ImGui::SameLine(); ShowHelpMark("Which image in a multipart file to display.");

		ImGui::Checkbox("Override Frame Duration", &CurrImage->PartDurationOverrideEnabled);
		if (CurrImage->PartDurationOverrideEnabled)
		{
			ImGui::InputFloat("Frame Duration", &CurrImage->PartDurationOverride, 0.01f, 0.1f, "%.4f");
			tMath::tiClamp(CurrImage->PartDurationOverride, 0.0f, 60.0f);

			if (ImGui::Button("1.0s")) CurrImage->PartDurationOverride = 1.0f; ImGui::SameLine();
			if (ImGui::Button("0.5s")) CurrImage->PartDurationOverride = 0.5f; ImGui::SameLine();
			if (ImGui::Button("30fps")) CurrImage->PartDurationOverride = 1.0f/30.0f; ImGui::SameLine();
			if (ImGui::Button("60fps")) CurrImage->PartDurationOverride = 1.0f/60.0f;
		}

		ImGui::PopItemWidth();
		ImGui::Unindent();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		uint64 loopImageID = CurrImage->PartPlayLooping ? PlayOnceImage.Bind() : PlayLoopImage.Bind();
		if (ImGui::ImageButton(ImTextureID(loopImageID), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2, ColourBG, ColourEnabledTint))
			CurrImage->PartPlayLooping = !CurrImage->PartPlayLooping;
		ImGui::SameLine();

		bool prevEnabled = !CurrImage->PartPlaying && (CurrImage->PartNum > 0);
		if (ImGui::ImageButton
		(
			ImTextureID(SkipBeginImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, prevEnabled ? ColourEnabledTint : ColourDisabledTint) && prevEnabled
		)	CurrImage->PartNum = 0;
		ImGui::SameLine();

		if (ImGui::ImageButton
		(
			ImTextureID(PrevImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, prevEnabled ? ColourEnabledTint : ColourDisabledTint) && prevEnabled
		)	CurrImage->PartNum = tClampMin(CurrImage->PartNum-1, 0);
		ImGui::SameLine();

		bool playRevEnabled = !(CurrImage->PartPlaying && !CurrImage->PartPlayRev);
		uint64 playRevImageID = !CurrImage->PartPlaying ? PlayRevImage.Bind() : StopRevImage.Bind();
		if (ImGui::ImageButton
		(
			ImTextureID(playRevImageID), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, playRevEnabled ? ColourEnabledTint : ColourDisabledTint) && playRevEnabled
		)
		{
			CurrImage->PartPlayRev = true;
			if (CurrImage->PartPlaying) CurrImage->Stop(); else CurrImage->Play();
		}
		ImGui::SameLine();

		bool playFwdEnabled = !(CurrImage->PartPlaying && CurrImage->PartPlayRev);
		uint64 playImageID = !CurrImage->PartPlaying ? PlayImage.Bind() : StopImage.Bind();
		if (ImGui::ImageButton
		(
			ImTextureID(playImageID), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, playFwdEnabled ? ColourEnabledTint : ColourDisabledTint) && playFwdEnabled
		)
		{
			CurrImage->PartPlayRev = false;
			if (CurrImage->PartPlaying) CurrImage->Stop(); else CurrImage->Play();
		}
		ImGui::SameLine();

		bool nextEnabled = !CurrImage->PartPlaying && (CurrImage->PartNum < (numParts-1));
		if (ImGui::ImageButton
		(
			ImTextureID(NextImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, nextEnabled ? ColourEnabledTint : ColourDisabledTint) && nextEnabled
		)	CurrImage->PartNum = tClampMax(CurrImage->PartNum+1, numParts-1);
		ImGui::SameLine();

		if (ImGui::ImageButton
		(
			ImTextureID(SkipEndImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, nextEnabled ? ColourEnabledTint : ColourDisabledTint) && nextEnabled
		)	CurrImage->PartNum = numParts-1;
		ImGui::SameLine();
	}

	ImGui::End();
}


void TexView::ShowPreferencesWindow(bool* popen)
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

	ImGui::Text("Background");
	ImGui::Indent();
	ImGui::Checkbox("Extend", &Config.BackgroundExtend);
	const char* backgroundItems[] = { "None", "Checkerboard", "Black", "Grey", "White" };
	ImGui::PushItemWidth(110);
	ImGui::Combo("Style", &Config.BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
	ImGui::PopItemWidth();
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("Slideshow");
	ImGui::Indent();
	ImGui::PushItemWidth(110);
	ImGui::InputDouble("Frame Duration (s)", &Config.SlidehowFrameDuration, 0.001f, 1.0f, "%.3f");
	ImGui::PopItemWidth();
	if (ImGui::Button("Reset Duration"))
		Config.SlidehowFrameDuration = 1.0/30.0;
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("Gamma");
	ImGui::Indent();
	ImGui::PushItemWidth(110);
	ImGui::InputFloat("Monitor Gamma", &Config.MonitorGamma, 0.01f, 0.1f, "%.3f");
	ImGui::PopItemWidth();
	if (ImGui::Button("Reset Gamma"))
		Config.MonitorGamma = tMath::DefaultGamma;
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("System");
	ImGui::Indent();
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
	ImGui::PopItemWidth();
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("Interface");
	ImGui::Indent();
	ImGui::Checkbox("Confirm Deletes", &Config.ConfirmDeletes);
	ImGui::Checkbox("Confirm File Overwrites", &Config.ConfirmFileOverwrites);
	ImGui::Checkbox("Auto Propery Window", &Config.AutoPropertyWindow);
	ImGui::Checkbox("Auto Play GIFs", &Config.AutoPlayAnimatedGIFs);
	
	ImGui::Unindent();
	ImGui::Separator();
	if (ImGui::Button("Reset UI Settings"))
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		Config.ResetUISettings(mode->width, mode->height);
		ChangeScreenMode(false, true);
	}
	ShowToolTip("Resets window dimensions/position, nav bar, content view, tiling, background, details, etc.");
	ImGui::SameLine();
	if (ImGui::Button("Reset Behaviour Settings"))
	{
		Config.ResetBehaviourSettings();
	}
	ShowToolTip("Resets sort order, resample filter, confirmations, preferred file type, cache size, etc.");
	ImGui::Unindent();

	ImGui::End();
}


void TexView::ColourCopyAs()
{
	tColourf floatCol(PixelColour);
	ImGui::Text("Copy As...");
	int ri = PixelColour.R; int gi = PixelColour.G; int bi = PixelColour.B; int ai = PixelColour.A;
	float rf = floatCol.R; float gf = floatCol.G; float bf = floatCol.B; float af = floatCol.A;
	tString cpyTxt;

	tsPrintf(cpyTxt, "%d %d %d %d", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "%d %d %d", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "(%d, %d, %d, %d)", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "(%d, %d, %d)", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "#%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "#%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "0x%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "%.3f, %.3f, %.3f, %.3f", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "%.3ff, %.3ff, %.3ff, %.3ff", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "(%.3f, %.3f, %.3f, %.3f)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	tsPrintf(cpyTxt, "(%.3ff, %.3ff, %.3ff, %.3ff)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chars()))
		ImGui::SetClipboardText(cpyTxt.Chars());
	ImGui::EndPopup();
}


void TexView::DoDeleteFileModal()
{
	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.Chars()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chars()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();
	ImGui::Checkbox("Confirm file deletions in the future?", &Config.ConfirmDeletes);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100, 0)))
	{
		DeleteImageFile(fullname, true);
		ImGui::CloseCurrentPopup();
	}
	ImGui::SetItemDefaultFocus();
	ImGui::EndPopup();
}


void TexView::DoDeleteFileNoRecycleModal()
{
	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.Chars()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chars()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("This operation cannot be undone. The file\nwill be deleted permanently.");
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100, 0)))
	{
		DeleteImageFile(fullname, false);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


// Parts of this class are a modification of the one that ships with Dear ImGui. The DearImGui
// licence (MIT) may be found in the txt file Licence_DearImGui_MIT.txt in the Data folder.
void TexView::NavLogBar::ClearLog()
{
	LogBuf.clear();
	LogLineOffsets.clear();
	LogLineOffsets.push_back(0);
}


void TexView::NavLogBar::AddLog(const char* fmt, ...)
{
	int oldSize = LogBuf.size();
	va_list args;
	va_start(args, fmt);
	LogBuf.appendfv(fmt, args);
	va_end(args);

	for (int newSize = LogBuf.size(); oldSize < newSize; oldSize++)
		if (LogBuf[oldSize] == '\n')
			LogLineOffsets.push_back(oldSize + 1);

	LogScrollToBottom = true;
}


void TexView::NavLogBar::Draw()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);

	if
	(
		ImGui::ImageButton(ImTextureID(UpFolderImage.Bind()), tVector2(18,18), tVector2(0,1), tVector2(1,0), 1,
		TexView::ColourBG, tVector4(1.00f, 1.00f, 1.00f, 1.00f))
	)
	{
		tString upDir = tSystem::tGetUpDir(ImagesDir);
		if (!upDir.IsEmpty())
		{
			ImageFileParam.Param = upDir + "dummyfile.txt";
			PopulateImages();
			SetCurrentImage();
			SetWindowTitle();
		}
	}
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("%s", ImagesDir.Chars());

	if (ImagesSubDirs.NumItems() > 0)
	{
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.0f);
		if (ImGui::BeginCombo("##combo", nullptr, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoPreview))
		{
			for (tStringItem* subDir = ImagesSubDirs.First(); subDir; subDir = subDir->Next())
			{
				bool isSelected = false;
				if (ImGui::Selectable(subDir->Chars(), isSelected))
				{
					// Selection made. This only runs once.
					ImageFileParam.Param = ImagesDir + *subDir + "/" + "dummyfile.txt";
					PopulateImages();
					SetCurrentImage();
					SetWindowTitle();
					break;
				}
			}
			ImGui::EndCombo();
		}
	}

	if (ShowLog)
		DrawLog();
}


void TexView::NavLogBar::DrawLog()
{
	if (ImGui::Button("Clear"))
		ClearLog();

	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	LogFilter.Draw("Filter", -100.0f);
	ImGui::Separator();
	ImGui::BeginChild("scrolling", tVector2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (copy)
		ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(0, 0));
	const char* buf = LogBuf.begin();
	const char* bufEnd = LogBuf.end();
	if (LogFilter.IsActive())
	{
		for (int lineNo = 0; lineNo < LogLineOffsets.Size; lineNo++)
		{
			const char* lineStart = buf + LogLineOffsets[lineNo];
			const char* lineEnd = (lineNo + 1 < LogLineOffsets.Size) ? (buf + LogLineOffsets[lineNo + 1] - 1) : bufEnd;
			if (LogFilter.PassFilter(lineStart, lineEnd))
				ImGui::TextUnformatted(lineStart, lineEnd);
		}
	}
	else
	{
		// The simplest way to display the entire buffer is with ImGui::TextUnformatted(buf, buf_end); TextUnformatted
		// has specialization for large blobs of text and will fast-forward to skip non-visible lines. Here we instead
		// demonstrate using the clipper to only process lines that are within the visible area. If you have tens of
		// thousands of items and their processing cost is non-negligible, coarse clipping them on your side is
		// recommended.
		//
		// ImGuiListClipper requires a) random access into your data, and b) items all being the same height, both of
		// which we can handle since we an array pointing to the beginning of each line of text. When using the filter
		// (in the block of code above) we don't have random access into the data to display anymore, which is why we
		// don't use the clipper. Storing or skimming through the search result would make it possible and would be
		// recommended if you want to search through tens of thousands of entries.
		ImGuiListClipper clipper;
		clipper.Begin(LogLineOffsets.Size);
		while (clipper.Step())
		{
			for (int lineNo = clipper.DisplayStart; lineNo < clipper.DisplayEnd; lineNo++)
			{
				const char* lineStart = buf + LogLineOffsets[lineNo];
				const char* lineEnd = (lineNo + 1 < LogLineOffsets.Size) ? (buf + LogLineOffsets[lineNo + 1] - 1) : bufEnd;
				ImGui::TextUnformatted(lineStart, lineEnd);
			}
		}
		clipper.End();
	}
	ImGui::PopStyleVar();

	if (LogScrollToBottom)
		ImGui::SetScrollHereY(1.0f);

	LogScrollToBottom = false;
	ImGui::EndChild();
}
