// ColourDialogs.cpp
//
// Dialogs related to adjusting colours including pixel-editing, channels, contrast, brightness, and level adjustments.
//
// Copyright (c) 2019-2024 Tristan Grimmer.
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
#include <Image/tImageJPG.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "GuiUtil.h"
#include "ColourDialogs.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "Preferences.h"
#include "Version.cmake.h"
using namespace tMath;


namespace Viewer
{
	float HistogramCallbackBridge(void* data, int index);
	struct HistogramCallback
	{
		HistogramCallback(Image::AdjChan channels, bool logarithmic, tImage::tPicture* picture, int numIndices) : Channels(channels), Logarithmic(logarithmic), Picture(picture), NumIndices(numIndices) { }
		float GetCount(int index) const;
		Image::AdjChan Channels;
		bool Logarithmic;
		tImage::tPicture* Picture;
		int NumIndices;
	};
}


void Viewer::ShowPixelEditorOverlay(bool* popen)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::PixelEditor);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	// I imagine NoResize is implied by AlwaysAutoResize. However I have seen cases where
	// a scrollbar width gets reserved even with AutoResize is on. This is why we're using
	// NoScrollBar every time we use AutoResize.
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar;

	static bool live = true;
	static tColour4f floatCol = tColour4f::black;
	static tColour4f floatColReset = tColour4f::black;
	if (ImGui::Begin("Edit Pixel", popen, flags))
	{
		float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);

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

		tColour4f origCol = floatCol;
		if ((ImGui::ColorPicker4("Colour", floatCol.E, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf), floatColReset.E) && (floatCol != origCol))
		{
			if (live)
			{
				CurrImage->Unbind();
				tColour4b col; col.Set(floatCol);

				// Don't push undo steps if we're dragging around the cursor.
				CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, false, true);
				CurrImage->Bind();
				Gutil::SetWindowTitle();
			}
		}

		tVector4 resetVecCol(floatColReset.E);
		bool resetPressed2 = ImGui::ColorButton("Reset Colour", resetVecCol, 0);
		ImGui::SameLine();
		bool resetPressed = ImGui::Button("Reset", tVector2(buttonWidth, 0.0f));
		ImGui::SameLine();
		if (resetPressed || resetPressed2)
		{
			if (live)
			{
				CurrImage->Unbind();
				tColour4b col; col.Set(floatColReset);
				CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true, true);
				CurrImage->Bind();
				Gutil::SetWindowTitle();
			}
			else
			{
				floatCol = floatColReset;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Live", &live);
		ImGui::SameLine();
		if (!live && ImGui::Button("Apply", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			tColour4b col; col.Set(floatCol);
			CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true);
			CurrImage->Bind();
			Gutil::SetWindowTitle();
		}
	}

	// If we closed the dialog and we're live, set the colour one more time but push to the undo stack.
	if (popen && (*popen == false) && live)
	{
		CurrImage->Unbind();
		tColour4b col;
		col.Set(floatColReset);
		CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, false, true);
		col.Set(floatCol);
		CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true);
		CurrImage->Bind();
		Gutil::SetWindowTitle();
	}
	ImGui::End();
}


void Viewer::ShowChannelFilterOverlay(bool* popen)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::ChannelFilter);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar;

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
			ImGui::SameLine(); Gutil::HelpMark
			(
				"Alpha is interpreted as blending opacity. When this channel is set to false full alpha is used and"
				"image is drawn opaque. When true it blends whatever colour channels are selected with the current background."
			);
		}

		float comboWidth	= Gutil::GetUIParamScaled(80.0f, 2.5f);
		float buttonWidth	= Gutil::GetUIParamScaled(120.0f, 2.5f);

		Config::ProfileData& profile = Config::GetProfileData();
		tColour4f floatCol(profile.BackgroundColour);
		if (ImGui::ColorEdit3("##Background", floatCol.E, ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar))
		{
			profile.BackgroundColour.Set(floatCol);
			profile.BackgroundColour.A = 0xFF;
		}

		ImGui::SameLine();
		const char* backgroundItems[] = { "None", "Checker", "Solid" };
		ImGui::SetNextItemWidth(comboWidth);
		ImGui::Combo("##Background Style", &profile.BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
		ImGui::SameLine();
		Gutil::HelpMark("Background colour and style.\nThe blend-background button uses the colour regardless of style.");

		Gutil::Separator();

		ImGui::Text("Modify");
		if (ImGui::Button("Blend Background", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			CurrImage->AlphaBlendColour(profile.BackgroundColour, true);
			CurrImage->Bind();
			Gutil::SetWindowTitle();
		}
		ImGui::SameLine();
		Gutil::HelpMark("Blend background colour into RGB of image based on alpha. Sets alphas to full when done.");

		comp_t channels = (Viewer::DrawChannel_R ? tCompBit_R : 0) | (Viewer::DrawChannel_G ? tCompBit_G : 0) | (Viewer::DrawChannel_B ? tCompBit_B : 0) | (Viewer::DrawChannel_A ? tCompBit_A : 0);
		if (ImGui::Button("Max Selected", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			tColour4b full(255, 255, 255, 255);
			CurrImage->SetAllPixels(full, channels);
			CurrImage->Bind();
			Gutil::SetWindowTitle();
		}
		ImGui::SameLine();
		Gutil::HelpMark("Sets selected channel(s) to their maximum value (255).");

		if (ImGui::Button("Zero Selected", tVector2(buttonWidth, 0.0f)))
		{
			CurrImage->Unbind();
			tColour4b zero(0, 0, 0, 0);
			CurrImage->SetAllPixels(zero, channels);
			CurrImage->Bind();
			Gutil::SetWindowTitle();
		}
		ImGui::SameLine();
		Gutil::HelpMark("Sets selected channel(s) to zero.");

		Gutil::Separator();

		ImGui::Text("Copy/Paste");
		Viewer::DoCopyPastePreferences(true);
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
	if (!Picture || (index >= NumIndices))
		return 0.0f;

	int groupIndex = tMath::tLinearInterp(float(index), 0.0f, float(NumIndices-1), 0, tImage::tPicture::NumGroups-1);
	tiClamp(groupIndex, 0, tImage::tPicture::NumGroups-1);
	float* table = nullptr;
	switch (Channels)
	{
		case Image::AdjChan::RGB:	table = Picture->HistogramI;	break;
		case Image::AdjChan::R:		table = Picture->HistogramR;	break;
		case Image::AdjChan::G:		table = Picture->HistogramG;	break;
		case Image::AdjChan::B:		table = Picture->HistogramB;	break;
		case Image::AdjChan::A:		table = Picture->HistogramA;	break;
	}
	float count = table[groupIndex];
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
	if (!ImGui::BeginPopupModal("Adjust Levels", &isOpenLevels, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
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
	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth	= Gutil::GetUIParamScaled(76.0f, 2.5f);
	float itemWidth		= Gutil::GetUIParamScaled(258.0f, 2.5f);

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
				ImGui::SameLine(); Gutil::HelpMark("If image is animated or otherwise has more than one frame\nsetting this to false allows only the single current frames to be adjusted.\nMake sure the image is stopped on the frame you want before opening the levels dialog.");
			}

			//
			// Mid-tones algorithm.
			//
			modified = ImGui::Checkbox("Continuous Mid Gamma", &profile.LevelsPowerMidGamma) || modified;
			ImGui::SameLine();
			Gutil::HelpMark
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
			modified = ImGui::Checkbox("Auto Mid Point", &profile.LevelsAutoMidPoint) || modified;
			ImGui::SameLine(); Gutil::HelpMark("If true forces the midpoint to be half way between the black and white points.");

			//
			// Logarithmic histogram?
			//
			ImGui::Checkbox("Logarithmic Histogram", &profile.LevelsLogarithmicHisto);
			ImGui::SameLine(); Gutil::HelpMark("Logarithmic scale is useful when you have a 'clumpy' intensity distribution.\nTurning this on uses the natural logarithm to scale the histogram counts.");
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
			tVector2 histSize = Gutil::GetUIParamScaled(tVector2(256.0f, 80.0f), 2.5f);
			HistogramCallback histoCB(Image::AdjChan(channels), profile.LevelsLogarithmicHisto, pic, int(histSize.x));
			if (profile.LevelsLogarithmicHisto)
				max = tMath::tLog(max);

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, colour);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2);

			// Why the +1 to NumGroups? It may be a an out-by-1 mistake in PlotHistogram. I can't hover the last group with my mouse without the +1.
			ImGui::PlotHistogram(histLabel.Chr(), HistogramCallbackBridge, &histoCB, int(histSize.x), 0, histName.Chr(), 0.0f, max, histSize);
			ImGui::PopStyleColor();

			//
			// Black/mid/white point sliders.
			//
			ImGui::PushItemWidth(itemWidth);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(6.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 8.0);
			modified = ImGui::SliderFloat("Black Point", &levelsBlack, 0.0f, 1.0f)		|| modified;
			bool manualMidAdj = ImGui::SliderFloat("Mid Point", &levelsMid, 0.0f, 1.0f);
			modified = manualMidAdj														|| modified;
			modified = ImGui::SliderFloat("White Point", &levelsWhite, 0.0f, 1.0f)		|| modified;
			modified = ImGui::SliderFloat("Black Out", &levelsOutBlack, 0.0f, 1.0f)		|| modified;
			modified = ImGui::SliderFloat("White Out", &levelsOutWhite, 0.0f, 1.0f)		|| modified;
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			//
			// Channels combo.
			//
			modified = ImGui::Combo("Channel##Levels", &channels, channelItems, tNumElements(channelItems)) || modified;
			ImGui::SameLine(); Gutil::HelpMark("Which channel(s) to apply adjustments to.");
			ImGui::PopItemWidth();

			//
			// Adjust the image.
			//
			if (modified)
			{
				// Constrain.
				tiClampMin(levelsWhite, levelsBlack);
				if (profile.LevelsAutoMidPoint && !manualMidAdj)
					levelsMid = (levelsWhite+levelsBlack)/2.0f;

				// In auto midpoint mode we allow the midpoint to 'push' the white and black points.
				if (profile.LevelsAutoMidPoint)
				{
					tiClampMax(levelsBlack, levelsMid);
					tiClampMin(levelsWhite, levelsMid);
				}
				else
				{
					tiClamp(levelsMid, levelsBlack, levelsWhite);
				}
				tiClampMin(levelsOutWhite, levelsOutBlack);

				CurrImage->Unbind();
				CurrImage->AdjustLevels(levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite, profile.LevelsPowerMidGamma, Image::AdjChan(channels), allFrames);
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
				ImGui::SameLine(); Gutil::HelpMark("If image is animated or otherwise has more than one frame\nsetting this to false allows only the single current frames to be adjusted.\nMake sure the image is stopped on the frame you want before opening the levels dialog.");
			}

			//
			// Contrast slider.
			//
			ImGui::PushItemWidth(itemWidth);
			modified = ImGui::SliderFloat("Contrast", &contrast, 0.0f, 1.0f) || modified;

			//
			// Channels combo.
			//
			modified = ImGui::Combo("Channel##Contrast", &channels, channelItems, tNumElements(channelItems)) || modified;
			ImGui::SameLine(); Gutil::HelpMark("Which channel(s) to apply adjustments to.");
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
				ImGui::SameLine(); Gutil::HelpMark("If image is animated or otherwise has more than one frame\nsetting this to false allows only the single current frames to be adjusted.\nMake sure the image is stopped on the frame you want before opening the levels dialog.");
			}

			//
			// Brightness slider.
			//
			ImGui::PushItemWidth(itemWidth);
			modified = ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f) || modified;

			//
			// Channels combo.
			//
			modified = ImGui::Combo("Channel##Brightness", &channels, channelItems, tNumElements(channelItems)) || modified;
			ImGui::SameLine(); Gutil::HelpMark("Which channel(s) to apply adjustments to.");
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

	if (Gutil::Button("Reset", tVector2(buttonWidth, 0.0f)))
	{
		CurrImage->AdjustGetDefaults(brightness, contrast, levelsBlack, levelsMid, levelsWhite, levelsOutBlack, levelsOutWhite);
		profile.LevelsPowerMidGamma		= true;
		profile.LevelsAutoMidPoint		= false;
		profile.LevelsLogarithmicHisto	= true;
		channels						= int(Image::AdjChan::RGB);

		CurrImage->Unbind();
		CurrImage->AdjustRestoreOriginal();
		CurrImage->Bind();
	}

	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
	{
		Gutil::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();

	float okOffset = Gutil::GetUIParamScaled(264.0f, 2.5f);
	ImGui::SetCursorPosX(okOffset);
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("OK", tVector2(buttonWidth, 0.0f)))
	{
		okPressed = true;
		Gutil::SetWindowTitle();
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}
