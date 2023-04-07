// Dialogs.cpp
//
// Various dialogs and helpers including a log window, info overlay, help window, and about window.
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

#include <Foundation/tVersion.cmake.h>
#include <Math/tVector2.h>
#include <Math/tColour.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Dialogs.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "Version.cmake.h"
using namespace tMath;


namespace Viewer
{
	float HistogramCallbackBridge(void* data, int index);
	struct HistogramCallback
	{
		HistogramCallback(Image::AdjChan channels, bool logarithmic, tImage::tPicture* picture) : Channels(channels), Logarithmic(logarithmic), Picture(picture) { }
		float GetCount(int index) const;
		Image::AdjChan Channels;
		bool Logarithmic;
		tImage::tPicture* Picture;
	};
}


void Viewer::ShowPixelEditorOverlay(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(DialogID::PixelEditor);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	static bool live = true;
	static tColourf floatCol = tColourf::black;
	static tColourf floatColReset = tColourf::black;
	if (ImGui::Begin("Edit Pixel", popen, flags))
	{
		static int lastCursorX = -1;
		static int lastCursorY = -1;
		static Image* lastImage = nullptr;
		if ((lastCursorX != Viewer::CursorX) || (lastCursorY != Viewer::CursorY) || (lastImage != CurrImage))
		{
			lastCursorX = Viewer::CursorX;
			lastCursorY = Viewer::CursorY;
			lastImage = CurrImage;
			if (live)
				floatCol.Set(Viewer::PixelColour);
			floatColReset.Set(Viewer::PixelColour);
		}

		tColourf origCol = floatCol;
		if ((ImGui::ColorPicker4("Colour", floatCol.E, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf), floatColReset.E) && (floatCol != origCol))
		{
			if (live)
			{
				CurrImage->Unbind();
				tColouri col; col.Set(floatCol);

				// Don't push undo steps if we're dragging around the cursor.
				CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, false, true);
				CurrImage->Bind();
				Viewer::SetWindowTitle();
			}
		}

		tVector4 resetVecCol(floatColReset.E);
		bool resetPressed2 = ImGui::ColorButton("Reset Colour", resetVecCol, 0);
		ImGui::SameLine();
		bool resetPressed = ImGui::Button("Reset", tVector2(100.0f, 0.0f));
		ImGui::SameLine();
		if (resetPressed || resetPressed2)
		{
			if (live)
			{
				CurrImage->Unbind();
				tColouri col; col.Set(floatColReset);
				CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true, true);
				CurrImage->Bind();
				Viewer::SetWindowTitle();
			}
			else
			{
				floatCol = floatColReset;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Live", &live);
		ImGui::SameLine();
		if (!live && ImGui::Button("Apply", tVector2(100.0f, 0.0f)))
		{
			CurrImage->Unbind();
			tColouri col; col.Set(floatCol);
			CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
	}

	// If we closed the dialog and we're live, set the colour one more time but push to the undo stack.
	if (popen && (*popen == false) && live)
	{
		CurrImage->Unbind();
		tColouri col;
		col.Set(floatColReset);
		CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, false, true);
		col.Set(floatCol);
		CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true);
		CurrImage->Bind();
		Viewer::SetWindowTitle();
	}
	ImGui::End();
}


void Viewer::ShowChannelFilterOverlay(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(DialogID::ChannelFilter);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Channel Filter", popen, flags))
	{
		ImGui::Text("Display");
		if (ImGui::Checkbox("Channel Intensity", &Viewer::DrawChannel_AsIntensity))
		{
			if (Viewer::DrawChannel_AsIntensity)
			{
				Viewer::DrawChannel_R = true;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = false;
			}
			else
			{
				Viewer::DrawChannel_R = true;
				Viewer::DrawChannel_G = true;
				Viewer::DrawChannel_B = true;
				Viewer::DrawChannel_A = true;
			}
		}
		if (Viewer::DrawChannel_AsIntensity)
		{
			if (ImGui::RadioButton("Red", Viewer::DrawChannel_R))
			{
				Viewer::DrawChannel_R = true;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = false;
			}
			if (ImGui::RadioButton("Green", Viewer::DrawChannel_G))
			{
				Viewer::DrawChannel_R = false;
				Viewer::DrawChannel_G = true;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = false;
			}
			if (ImGui::RadioButton("Blue", Viewer::DrawChannel_B))
			{
				Viewer::DrawChannel_R = false;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = true;
				Viewer::DrawChannel_A = false;
			}
			if (ImGui::RadioButton("Alpha", Viewer::DrawChannel_A))
			{
				Viewer::DrawChannel_R = false;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = true;
			}
		}
		else
		{
			ImGui::Checkbox("Red", &Viewer::DrawChannel_R);
			ImGui::Checkbox("Green", &Viewer::DrawChannel_G);
			ImGui::Checkbox("Blue", &Viewer::DrawChannel_B);
			ImGui::Checkbox("Alpha", &Viewer::DrawChannel_A);
			ImGui::SameLine(); ShowHelpMark
			(
				"Alpha is interprested as blending opacity. When this channel is set to false full alpha is used and"
				"image is drawn opaque. When true it blends whatever colour channels are selected with the current background."
			);
		}

		tColourf floatCol(Config::Current->BackgroundColour);
		if (ImGui::ColorEdit3("##Background", floatCol.E, ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar))
		{
			Config::Current->BackgroundColour.Set(floatCol);
			Config::Current->BackgroundColour.A = 0xFF;
		}

		ImGui::SameLine();
		const char* backgroundItems[] = { "None", "Checker", "Solid" };
		ImGui::PushItemWidth(83);
		ImGui::Combo("##Background Style", &Config::Current->BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ShowHelpMark("Background colour and style.\nThe blend-background button uses the colour regardless of style.");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		ImGui::Text("Modify");

		float buttonWidth = 134.0f;
		if (ImGui::Button("Blend Background", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			CurrImage->AlphaBlendColour(Config::Current->BackgroundColour, true);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
		ImGui::SameLine();
		ShowHelpMark("Blend background colour into RGB of image based on alpha. Sets alphas to full when done.");

		comp_t channels = (Viewer::DrawChannel_R ? tCompBit_R : 0) | (Viewer::DrawChannel_G ? tCompBit_G : 0) | (Viewer::DrawChannel_B ? tCompBit_B : 0) | (Viewer::DrawChannel_A ? tCompBit_A : 0);
		if (ImGui::Button("Max Selected", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			tColouri full(255, 255, 255, 255);
			CurrImage->SetAllPixels(full, channels);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
		ImGui::SameLine();
		ShowHelpMark("Sets selected channel(s) to their maximum value (255).");

		if (ImGui::Button("Zero Selected", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			tColouri zero(0, 0, 0, 0);
			CurrImage->SetAllPixels(zero, channels);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
		ImGui::SameLine();
		ShowHelpMark("Sets selected channel(s) to zero.");
	}

	ImGui::End();
}


float Viewer::HistogramCallbackBridge(void* data, int index)
{
	HistogramCallback* callback = (HistogramCallback*)data;
	if (!callback)
		return 0.0f;
	return callback->GetCount(index);
}


float Viewer::HistogramCallback::GetCount(int index) const
{
	if (!Picture || (index >= tImage::tPicture::NumGroups))
		return 0.0f;
	float* table = nullptr;
	switch (Channels)
	{
		case Image::AdjChan::RGB:	table = Picture->HistogramI;	break;
		case Image::AdjChan::R:		table = Picture->HistogramR;	break;
		case Image::AdjChan::G:		table = Picture->HistogramG;	break;
		case Image::AdjChan::B:		table = Picture->HistogramB;	break;
		case Image::AdjChan::A:		table = Picture->HistogramA;	break;
	}
	float count = table[index];
	return (Logarithmic && (count > 0)) ? tMath::tLog(count) : count;
}


void Viewer::DoLevelsModal(bool levelsPressed)
{
	static bool popupOpen = false;
	enum class TabEnum { Levels, Contrast, Brightness };
	static TabEnum currTab = TabEnum::Levels;
	static bool okPressed = false;

	static float brightness = 0.5f;
	static float contrast = 0.5f;
	static float levelsBlack = 0.0f;
	static float levelsMid = 0.5f;
	static float levelsWhite = 1.0f;
	static float levelsOutBlack = 0.0f;
	static float levelsOutWhite = 1.0f;
	static bool allFrames = true;
	static bool powerMidGamma = true;
	static bool autoMidPoint = false;
	static bool logarithmicHisto = true;
	static int channels = int(Image::AdjChan::RGB);

	if (levelsPressed)
	{
		ImGui::OpenPopup("Adjust Levels");
		popupOpen = true;

		// This gets called whenever the levels dialog gets opened.
		CurrImage->AdjustmentBegin();
		CurrImage->AdjustGetDefaults(brightness, contrast, levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite);
		okPressed = false;
	}

	bool isOpenLevels = true;
	if (!ImGui::BeginPopupModal("Adjust Levels", &isOpenLevels, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (popupOpen)
		{
			// This gets called whenever the levels dialog gets closed.
			if (!okPressed)
			{
				CurrImage->Unbind();
				CurrImage->AdjustRestoreOriginal();
				CurrImage->Bind();
			}
			CurrImage->AdjustmentEnd();
		}
		popupOpen = false;
		return;
	}

	//
	// UI size parameters.
	//
	float okOffset = 140.0f;
	switch (Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:	okOffset = 135.0f;	break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:	okOffset = 146.0f;	break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:	okOffset = 158.0f;	break;
	}

	const char* channelItems[] = { "RGB", "Red", "Green", "Blue", "Alpha" };
	if (ImGui::BeginTabBar("LevelsTabBar", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Levels", nullptr, ImGuiTabItemFlags_NoTooltip))
		{
			//
			// Just switched to this tab?
			//
			if (currTab != TabEnum::Levels)
			{
				CurrImage->AdjustGetDefaults(brightness, contrast, levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite);
				CurrImage->Unbind();
				CurrImage->AdjustRestoreOriginal();
				CurrImage->Bind();
				currTab = TabEnum::Levels;
			}
			ImGui::NewLine();
			bool modified = false;

			//
			// All frames or just current?
			//
			if (CurrImage->GetNumFrames() > 1)
			{
				if (ImGui::Checkbox("All Frames", &allFrames))
				{
					// We need to undo anything that may have either applied to all or one frame. No need to Unbind/Bind as modified true deals with that.
					CurrImage->AdjustRestoreOriginal();
					modified = true;
				}
				ImGui::SameLine(); ShowHelpMark("If image is animated or otherwise has more than one frame\nsetting this to false allows only the single current frames to be adjusted.\nMake sure the image is stopped on the frame you want before opening the levels dialog.");
			}

			//
			// Mid-tones algorithm.
			//
			modified = ImGui::Checkbox("Continuous Mid Gamma", &powerMidGamma)			|| modified;
			ImGui::SameLine();
			ShowHelpMark
			(
				"Continuous-midpoint-gamma lets you decide between 2 algorithms to determine the curve on the mid-tone gamma.\n"
				"\n"
				"If false it tries to mimic Photoshop where there is a C1 discontinuity at gamma = 1.\n"
				"The gamma range is [0.01, 9.99] where 1.0 is linear. This approximates PS.\n"
				"\n"
				"If true uses a continuous base-10 power curve that smoothly transitions the full range.\n"
				"The gamma range is [0.1, 10.0] where 1.0 is linear. This approximates GIMP."
			);

			//
			// Auto-set mid-point between black/white point?
			//
			modified = ImGui::Checkbox("Auto Mid Point", &autoMidPoint)					|| modified;
			ImGui::SameLine(); ShowHelpMark("If true forces the midpoint to be half way between the black and white points.");

			//
			// Logarithmic histogram?
			//
			ImGui::Checkbox("Logarithmic Histogram", &logarithmicHisto);
			ImGui::SameLine(); ShowHelpMark("Logarithmic scale is useful when you have a 'clumpy' intensity distribution.\nTurning this on uses the natural logarithm to scale the histogram counts.");
			ImGui::NewLine();

			//
			// The histogram itself.
			//
			float max = 0.0f;
			tVector4 colour = tVector4::one;
			tImage::tPicture* pic = CurrImage->GetCurrentPic();
			if (pic)
			{
				switch (channels)
				{
					case int(Image::AdjChan::RGB):	max = pic->MaxICount;	colour.Set(0.9f, 0.9f, 0.9f, 1.8f);		break;
					case int(Image::AdjChan::R):	max = pic->MaxRCount;	colour.Set(1.0f, 0.2f, 0.2f, 1.0f);		break;
					case int(Image::AdjChan::G):	max = pic->MaxGCount;	colour.Set(0.2f, 1.0f, 0.2f, 1.0f);		break;
					case int(Image::AdjChan::B):	max = pic->MaxBCount;	colour.Set(0.3f, 0.3f, 1.0f, 1.0f);		break;
					case int(Image::AdjChan::A):	max = pic->MaxACount;	colour.Set(0.6f, 0.6f, 0.6f, 1.0f);		break;
				}
			}
			tString histName;  tsPrintf(histName,  "%s Intensity", channelItems[channels]);
			tString histLabel; tsPrintf(histLabel, "Max %d\n\nHistogram", int(max));
			HistogramCallback histoCB(Image::AdjChan(channels), logarithmicHisto, pic);
			if (logarithmicHisto)
				max = tMath::tLog(max);

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, colour);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2);

			// Why the +1 to NumGroups? It may be a an out-by-1 mistake in PlotHistogram. I can't hover the last group with my mouse without the +1.
			ImGui::PlotHistogram(histLabel.Chr(), HistogramCallbackBridge, &histoCB, tImage::tPicture::NumGroups+1, 0, histName.Chr(), 0.0f, max, ImVec2(256, 80));
			ImGui::PopStyleColor();

			//
			// Black/mid/white point sliders.
			//
			ImGui::PushItemWidth(258);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(6.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 8.0);
			modified = ImGui::SliderFloat("Black Point", &levelsBlack, 0.0f, 1.0f)		|| modified;
			if (!autoMidPoint)
				modified = ImGui::SliderFloat("Mid Point", &levelsMid, 0.0f, 1.0f)		|| modified;
			modified = ImGui::SliderFloat("White Point", &levelsWhite, 0.0f, 1.0f)		|| modified;
			modified = ImGui::SliderFloat("Black Out", &levelsOutBlack, 0.0f, 1.0f)		|| modified;
			modified = ImGui::SliderFloat("White Out", &levelsOutWhite, 0.0f, 1.0f)		|| modified;
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			//
			// Channels combo.
			//
			modified = ImGui::Combo("Channel##Levels", &channels, channelItems, tNumElements(channelItems)) || modified;
			ImGui::SameLine(); ShowHelpMark("Which channel(s) to apply adjustments to.");
			ImGui::PopItemWidth();

			//
			// Adjust the image.
			//
			if (modified)
			{
				// Constrain.
				if (autoMidPoint)
				{
					tiClampMin(levelsWhite, levelsBlack);
					levelsMid = (levelsWhite+levelsBlack)/2.0f;
				}
				else
				{
					tiClampMin(levelsMid, levelsBlack);
					tiClampMin(levelsWhite, levelsMid);
				}
				tiClampMin(levelsOutWhite, levelsOutBlack);

				CurrImage->Unbind();
				CurrImage->AdjustLevels(levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite, powerMidGamma, Image::AdjChan(channels), allFrames);
				CurrImage->Bind();
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Contrast", nullptr, ImGuiTabItemFlags_NoTooltip))
		{
			//
			// Just switched to this tab?
			//
			if (currTab != TabEnum::Contrast)
			{
				CurrImage->AdjustGetDefaults(brightness, contrast, levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite);
				CurrImage->Unbind();
				CurrImage->AdjustRestoreOriginal();
				CurrImage->Bind();
				currTab = TabEnum::Contrast;
			}
			ImGui::NewLine();
			bool modified = false;

			//
			// All frames or just current?
			//
			if (CurrImage->GetNumFrames() > 1)
			{
				if (ImGui::Checkbox("All Frames", &allFrames))
				{
					// We need to undo anything that may have either applied to all or one frame. No need to Unbind/Bind as modified true deals with that.
					CurrImage->AdjustRestoreOriginal();
					modified = true;
				}
				ImGui::SameLine(); ShowHelpMark("If image is animated or otherwise has more than one frame\nsetting this to false allows only the single current frames to be adjusted.\nMake sure the image is stopped on the frame you want before opening the levels dialog.");
			}

			//
			// Contrast slider.
			//
			ImGui::PushItemWidth(258);
			modified = ImGui::SliderFloat("Contrast", &contrast, 0.0f, 1.0f) || modified;

			//
			// Channels combo.
			//
			modified = ImGui::Combo("Channel##Contrast", &channels, channelItems, tNumElements(channelItems)) || modified;
			ImGui::SameLine(); ShowHelpMark("Which channel(s) to apply adjustments to.");
			ImGui::PopItemWidth();

			//
			// Adjust the image.
			//
			if (modified)
			{
				CurrImage->Unbind();
				CurrImage->AdjustContrast(contrast, Image::AdjChan(channels));
				CurrImage->Bind();
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Brightness", nullptr, ImGuiTabItemFlags_NoTooltip))
		{
			//
			// Just switched to this tab?
			//
			if (currTab != TabEnum::Brightness)
			{
				CurrImage->AdjustGetDefaults(brightness, contrast, levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite);
				CurrImage->Unbind();
				CurrImage->AdjustRestoreOriginal();
				CurrImage->Bind();
				currTab = TabEnum::Brightness;
			}
			ImGui::NewLine();
			bool modified = false;

			//
			// All frames or just current?
			//
			if (CurrImage->GetNumFrames() > 1)
			{
				if (ImGui::Checkbox("All Frames", &allFrames))
				{
					// We need to undo anything that may have either applied to all or one frame. No need to Unbind/Bind as modified true deals with that.
					CurrImage->AdjustRestoreOriginal();
					modified = true;
				}
				ImGui::SameLine(); ShowHelpMark("If image is animated or otherwise has more than one frame\nsetting this to false allows only the single current frames to be adjusted.\nMake sure the image is stopped on the frame you want before opening the levels dialog.");
			}

			//
			// Brightness slider.
			//
			ImGui::PushItemWidth(258);
			modified = ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f) || modified;

			//
			// Channels combo.
			//
			modified = ImGui::Combo("Channel##Brightness", &channels, channelItems, tNumElements(channelItems)) || modified;
			ImGui::SameLine(); ShowHelpMark("Which channel(s) to apply adjustments to.");
			ImGui::PopItemWidth();

			//
			// Adjust the image.
			//
			if (modified)
			{
				CurrImage->Unbind();
				CurrImage->AdjustBrightness(brightness, Image::AdjChan(channels));
				CurrImage->Bind();
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	if (Viewer::Button("Reset", tVector2(100.0f, 0.0f)))
	{
		CurrImage->AdjustGetDefaults(brightness, contrast, levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite);
		powerMidGamma = true;
		autoMidPoint = false;
		logarithmicHisto = true;
		channels = int(Image::AdjChan::RGB);

		CurrImage->Unbind();
		CurrImage->AdjustRestoreOriginal();
		CurrImage->Bind();
	}

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
	{
		Viewer::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + okOffset);
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
	{
		okPressed = true;
		Viewer::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}


void Viewer::ShowAboutPopup(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(DialogID::About);
	tVector2 windowSize;
	switch (Config::Current->GetUISize())
	{
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:	windowSize.Set(240, 500);	break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Medium:	windowSize.Set(280, 501);	break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Large:	windowSize.Set(320, 515);	break;
	}

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize;
	if (ImGui::Begin("About", popen, flags))
	{
		int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
		glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
		ImGui::Text("Tacent View V %d.%d.%d by Tristan Grimmer", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);

		tString platform	= tGetPlatformName( tGetPlatform() );
		tString architec	= tGetArchitectureNameLong( tGetArchitecture() );
		tString config 		= tGetConfigurationName( tGetConfiguration() );
		tString package		= "Native";
		#if defined(PACKAGE_SNAP)
			package = "Snap";
		#elif defined(PACKAGE_DEB)
			package = "Deb";
		#elif defined(PACKAGE_ZIP)
			package = "Zip";
		#endif

		ImGui::Text("Platform: %s", platform.Chr());
		ImGui::Text("Architecture: %s", architec.Chr());
		ImGui::Text("Configuration: %s", config.Chr());
		ImGui::Text("Package: %s", package.Chr());

		#ifdef TACENT_UTF16_API_CALLS
		ImGui::Text("API Mode: UTF-16");
		#else
		ImGui::Text("API Mode: UTF-8");
		#endif

		ImGui::Separator();
		ImGui::Text("Tacent Library V %d.%d.%d", tVersion::Major, tVersion::Minor, tVersion::Revision);
		ImGui::Text("Dear ImGui V %s", IMGUI_VERSION);

		// This way of getting the version is 'dynamic'. It will, for example, print mesa compatibility mode if it's being used.
		ImGui::Text("GLAD V %s", glad_glGetString(GL_VERSION));
		// This is the more 'static' way. Currently unused.
		// ImGui::Text("GLAD V %d.%d", GLVersion.major, GLVersion.minor);

		ImGui::Text("GLFW V %d.%d.%d", glfwMajor, glfwMinor, glfwRev);
		ImGui::Text("Ico Load");
		ImGui::Text("Ion Icons");
		ImGui::Text("Roboto Google Font");
		ImGui::Text("Radiance Software");
		ImGui::Text("LibJPEG-Turbo V %s", tImage::Version_LibJpegTurbo);
		ImGui::Text("LibTIFF V %s", tImage::Version_LibTIFF);
		ImGui::Text("OpenEXR V %s", tImage::Version_OpenEXR);
		ImGui::Text("ZLib V %s", tImage::Version_ZLIB);
		ImGui::Text("GIF Load");
		ImGui::Text("GIF Save");
		ImGui::Text("LibWEBP V %d.%d", tImage::Version_WEBP_Major, tImage::Version_WEBP_Minor);
		ImGui::Text("LibPNG V %s", tImage::Version_LibPNG);
		ImGui::Text("LibKTX V %s", tImage::Version_LibKTX);
		ImGui::Text("APNGDis V %s", tImage::Version_ApngDis);
		ImGui::Text("APNGAsm V %s", tImage::Version_ApngAsm);
		ImGui::Text("TinyXML2 V %d.%d.%d", tImage::Version_TinyXML2_Major, tImage::Version_TinyXML2_Minor, tImage::Version_TinyXML2_Patch);
		ImGui::Text("TinyEXIF V %d.%d.%d", tImage::Version_TinyEXIF_Major, tImage::Version_TinyEXIF_Minor, tImage::Version_TinyEXIF_Patch);
		ImGui::Text("BCDec V %d.%d", tImage::Version_BCDec_Major, tImage::Version_BCDec_Minor);
		ImGui::Text("QOI");
	}
	ImGui::End();
}


void Viewer::DoDeleteFileModal(bool deleteFilePressed)
{
	if (deleteFilePressed)
	{
		if (!Config::Current->ConfirmDeletes)
			DeleteImageFile(CurrImage->Filename, true);
		else
			ImGui::OpenPopup("Delete File");
	}

	// The unused isOpenDeleteFile bool is just so we get a close button in ImGui.
	bool isOpenDeleteFile = true;
	if (!ImGui::BeginPopupModal("Delete File", &isOpenDeleteFile, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.Chr()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chr()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();
	ImGui::Checkbox("Confirm file deletions in the future?", &Config::Current->ConfirmDeletes);
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
	{
		DeleteImageFile(fullname, true);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoDeleteFileNoRecycleModal(bool deleteFileNoRecycPressed)
{
	if (deleteFileNoRecycPressed)
		ImGui::OpenPopup("Delete File Permanently");

	// The unused isOpenPerm bool is just so we get a close button in ImGui.
	bool isOpenPerm = true;
	if (!ImGui::BeginPopupModal("Delete File Permanently", &isOpenPerm, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.Chr()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chr()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("This operation cannot be undone. The file\nwill be deleted permanently.");
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
	{
		DeleteImageFile(fullname, false);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoRenameModal(bool renamePressed)
{
	if (renamePressed)
		ImGui::OpenPopup("Rename File");

	// The unused isOpenRen bool is just so we get a close button in ImGui.
	bool isOpenRen = true;
	if (!ImGui::BeginPopupModal("Rename File", &isOpenRen, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tString fullname = CurrImage->Filename;
	tString origname = tSystem::tGetFileName(fullname);

	static char newname[128] = "Filename";
	if (renamePressed)
		tStd::tStrcpy(newname, origname.Chr());

	bool nameChanged = false;
	if (ImGui::InputText("##NewNameText", newname, tNumElements(newname), ImGuiInputTextFlags_EnterReturnsTrue))
		nameChanged = true;
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)) || nameChanged)
	{
		if (origname != newname)
		{
			tString dir = tSystem::tGetDir(fullname);
			bool renamed = tSystem::tRenameFile(dir, origname, newname);
			if (renamed)
			{
				PopulateImages();
				SetCurrentImage(dir+newname);
			}
		}

		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoSnapMessageNoFileBrowseModal(bool justPressed)
{
	if (justPressed)
		ImGui::OpenPopup("Message_NoFileBrowse");

	// The unused isMessage bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isMessage = true;
	if
	(
		!ImGui::BeginPopupModal
		(
			"Message_NoFileBrowse", &isMessage,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		)
	)
	{
		return;
	}
	
	ImGui::Text
	(
		"The Snap version of Tacent View does not\n"
		"support opening Nautilus or Dolphin.\n\n"
		"Please use the deb install or compile if\n"
		"you need the feature on Linux."
	);
	ImGui::NewLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::DoSnapMessageNoFrameTransModal(bool justPressed)
{
	if (justPressed)
		ImGui::OpenPopup("Message_NoFrameTrans");

	// The unused isMessage bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isMessage = true;
	if
	(
		!ImGui::BeginPopupModal
		(
			"Message_NoFrameTrans", &isMessage,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		)
	)
	{
		return;
	}

	ImGui::Text
	(
		"The Snap version of Tacent View does not\n"
		"support transparent work area.\n\n"
		"Please use the deb install or compile if\n"
		"you need the feature on Linux."
	);
	ImGui::NewLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


// Parts of this class are a modification of the one that ships with Dear ImGui. The DearImGui
// licence (MIT) may be found in the txt file Licence_DearImGui_MIT.txt in the Data folder.
void Viewer::NavLogBar::ClearLog()
{
	LogBuf.clear();
	LogLineOffsets.clear();
	LogLineOffsets.push_back(0);
}


void Viewer::NavLogBar::AddLog(const char* fmt, ...)
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


void Viewer::NavLogBar::Draw()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);

	if
	(
		ImGui::ImageButton(ImTextureID(Image_UpFolder.Bind()), tVector2(20.0f, 20.0f), tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
		Viewer::ColourBG, tVector4(1.00f, 1.00f, 1.00f, 1.00f))
	)
	{
		tString upDir = tSystem::tGetUpDir(ImagesDir);
		if (!upDir.IsEmpty())
		{
			CurrImageFile = upDir + "dummyfile.txt";
			PopulateImages();
			SetCurrentImage();
			SetWindowTitle();
		}
	}
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("%s", ImagesDir.Chr());

	if (ImagesSubDirs.NumItems() > 0)
	{
		ImGui::SameLine();
		float offset = 0.0f;
		switch (Config::Current->GetUISize())
		{
			case Config::ProfileSettings::UISizeEnum::Small:	offset = 1.0f;	break;
			case Config::ProfileSettings::UISizeEnum::Medium:	offset = 0.0f;	break;
			case Config::ProfileSettings::UISizeEnum::Large:	offset = -1.0f;	break;
		}
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
		if (ImGui::BeginCombo("##combo", nullptr, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoPreview))
		{
			for (tStringItem* subDir = ImagesSubDirs.First(); subDir; subDir = subDir->Next())
			{
				bool isSelected = false;
				if (ImGui::Selectable(subDir->Chr(), isSelected))
				{
					// Selection made. This only runs once.
					CurrImageFile = ImagesDir + *subDir + "/" + "dummyfile.txt";
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


void Viewer::NavLogBar::DrawLog()
{
	if (ImGui::Button("Clear"))
		ClearLog();

	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	LogFilter.Draw("Filter", -100.0f);
	ImGui::Separator();
	ImGui::BeginChild("scrolling", tVector2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (copy)
		ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(0.0f, 0.0f));
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
