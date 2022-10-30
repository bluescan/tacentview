// Properties.cpp
//
// Image properties display and editor window.
//
// Copyright (c) 2019-2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "imgui.h"
#include "Properties.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
using namespace tMath;
using namespace tImage;


namespace Viewer
{
	// These return true if any UI was drawn.
	bool DoAltMipmapsDisplay(tString& texTypeName);
	bool DoAltCubemapDisplay(tString& texTypeName);
	bool DoChooseDisplayImage(tString& texTypeName);
}


bool Viewer::DoAltMipmapsDisplay(tString& texTypeName)
{
	bool altMipmapsPicAvail = CurrImage->IsAltMipmapsPictureAvail() && !CropMode;
	bool altMipmapsPicEnabl = altMipmapsPicAvail && CurrImage->IsAltPictureEnabled();
	bool anyDraw = false;

	if (altMipmapsPicAvail)
	{
		if (ImGui::Checkbox("Display All Mipmaps", &altMipmapsPicEnabl))
		{
			CurrImage->EnableAltPicture(altMipmapsPicEnabl);
			CurrImage->Bind();
		}
		ShowToolTip("Display all mipmaps in a single image.");
		texTypeName = "Mipmap";
		anyDraw = true;
	}

	return anyDraw;
}


bool Viewer::DoAltCubemapDisplay(tString& texTypeName)
{
	bool altCubemapPicAvail = CurrImage->IsAltCubemapPictureAvail() && !CropMode;
	bool altCubemapPicEnabl = altCubemapPicAvail && CurrImage->IsAltPictureEnabled();
	bool anyDraw = false;

	if (altCubemapPicAvail)
	{
		if (ImGui::Checkbox("Display As Cubemap", &altCubemapPicEnabl))
		{
			CurrImage->EnableAltPicture(altCubemapPicEnabl);
			CurrImage->Bind();
		}
		ShowToolTip("Display all cubemap sides in a T-layout.");
		texTypeName = "Cubemap Side";
		anyDraw = true;
	}

	return anyDraw;
}


bool Viewer::DoChooseDisplayImage(tString& texTypeName)
{
	bool anyDraw = false;
	int numTextures = CurrImage->GetNumFrames();
	if (numTextures >= 2)
	{
		if (!CurrImage->IsAltPictureEnabled())
		{
			tString imageNumText;
			tsPrintf(imageNumText, "%s (%d)", texTypeName.Chr(), numTextures);

			int oneBasedTextureNum = CurrImage->FrameNum + 1;
			ImGui::PushItemWidth(110);
			if (ImGui::InputInt(imageNumText.Chr(), &oneBasedTextureNum))
			{
				CurrImage->FrameNum = oneBasedTextureNum - 1;
				tMath::tiClamp(CurrImage->FrameNum, 0, numTextures-1);
			}
			ImGui::PopItemWidth();
			ImGui::SameLine(); ShowHelpMark("Which mipmap or cubemap side to display.\nCubemap sides left-handed +X,-X,+Y,-Y,+Z,-Z");

		}
	}
	return anyDraw;
}


void Viewer::ShowPropertiesWindow(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize; // | ImGuiWindowFlags_NoFocusOnAppearing;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = GetDialogOrigin(1);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(tVector2(238.0f, -1.0f), ImGuiCond_Always);

	if (!ImGui::Begin("Properties", popen, windowFlags))
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
		case tSystem::tFileType::DDS:
		{
			bool anyUIDisplayed = false;
			int numTextures = CurrImage->GetNumFrames();
			bool reloadChanges = false;

			tString texTypeName = "Texture";
			anyUIDisplayed |= DoAltMipmapsDisplay(texTypeName);
			anyUIDisplayed |= DoAltCubemapDisplay(texTypeName);
			anyUIDisplayed |= DoChooseDisplayImage(texTypeName);

			// If we're here show options when have 1 or more frames.
			bool altEnabled = CurrImage->IsAltPictureEnabled();
			if (tIsHDRFormat(CurrImage->Info.SrcPixelFormat))
			{
				// Gamma correction. First read current setting and put it in an int.
				int gammaMode = 0;
				if (CurrImage->LoadParams_DDS.Flags & tImageDDS::LoadFlag_GammaCompression)
					gammaMode = 1;
				if (CurrImage->LoadParams_DDS.Flags & tImageDDS::LoadFlag_SRGBCompression)
					gammaMode = 2;

				const char* gammaCorrectItems[] = { "None", "Gamma", "sRGB" };
				ImGui::PushItemWidth(110);
				if (ImGui::Combo("Gamma Correct", &gammaMode, gammaCorrectItems, tNumElements(gammaCorrectItems)))
				{
					CurrImage->LoadParams_DDS.Flags &= ~(tImageDDS::LoadFlag_GammaCompression | tImageDDS::LoadFlag_SRGBCompression);
					if (gammaMode == 1) CurrImage->LoadParams_DDS.Flags |= tImageDDS::LoadFlag_GammaCompression;
					if (gammaMode == 2) CurrImage->LoadParams_DDS.Flags |= tImageDDS::LoadFlag_SRGBCompression;
					reloadChanges = true;
				}
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ShowHelpMark
				(
					"Gamma Correction\n"
					"Floating-point pixel formats used for HDR images are often in linear space. Before being displayed\n"
					"on a screen with non-linear response they need to be 'corrected' to gamma or sRGB-space (brightened).\n"
					"\n"
					"Use 'None' if you know the source image data is already in either gamma or sRGB-space.\n\n"
					"Use 'Gamma' if you want control over the gamma exponent being used to do the correction. 2.2 is standard.\n\n"
					"Use 'sRGB' if you want to convert to sRGB-space. This more accurately represents a display's response and\n"
					"is close to a 2.2 gamma but with an extra linear region, a non-unity amplitude, and a slightly larger gamma.",
					false
				);

				if (gammaMode == 1)
				{
					ImGui::PushItemWidth(110);
					if (ImGui::InputFloat("Gamma", &CurrImage->LoadParams_DDS.Gamma, 0.01f, 0.1f, "%.3f"))
						reloadChanges = true;
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ShowHelpMark("Gamma to use [0.5, 4.0]. Hold Ctrl to speedup. Open preferences to edit default gamma value.");
					tMath::tiClamp(CurrImage->LoadParams_DDS.Gamma, 0.5f, 4.0f);
				}

				bool expEnabled = (CurrImage->LoadParams_DDS.Flags & tImageDDS::LoadFlag_ToneMapExposure);
				ImGui::PushItemWidth(110);
				if (ImGui::InputFloat("Exposure", &CurrImage->LoadParams_DDS.Exposure, 0.001f, 0.05f, "%.4f", expEnabled ? 0 : ImGuiInputTextFlags_ReadOnly))
					reloadChanges = true;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if (ImGui::CheckboxFlags("##ExposureEnabled", &CurrImage->LoadParams_DDS.Flags, tImageDDS::LoadFlag_ToneMapExposure))
					reloadChanges = true;
				ImGui::SameLine();
				ShowHelpMark("Exposure adjustment [0.0, 4]. Hold Ctrl to speedup.");
				tMath::tiClamp(CurrImage->LoadParams_DDS.Exposure, 0.0f, 4.0f);

				anyUIDisplayed = true;
			}

			if (tIsLuminanceFormat(CurrImage->Info.SrcPixelFormat))
			{
				if (ImGui::CheckboxFlags("Spread Luminance", &CurrImage->LoadParams_DDS.Flags, tImageDDS::LoadFlag_SpreadLuminance))
					reloadChanges = true;
				ImGui::SameLine();
				ShowHelpMark("Luminance-only dds files are represented in this viewer as having a red channel only,\nIf spread is true, the channel is spread to all RGB channels to create a grey-scale image.");
			}

			bool scrubberDisplayed = false;
			if ((numTextures >= 2) && !altEnabled)
			{
				ImGui::Checkbox("Scrubber", &Config::Current->ShowFrameScrubber);
				anyUIDisplayed = true;
				scrubberDisplayed = true;
			}

			if (anyUIDisplayed)
			{
				if (scrubberDisplayed)
					ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 110.0f);
				if (ImGui::Button("Reset", tVector2(110.0f, 0.0f)))
				{
					CurrImage->ResetLoadParams();
					CurrImage->FrameNum = 0;
					reloadChanges = true;
				}
			}

			if (reloadChanges)
			{
				CurrImage->Unload();
				CurrImage->Load();
				if (altEnabled)
				{
					CurrImage->EnableAltPicture(true);
					CurrImage->Bind();
				}
			}

			// Some DDS files have no available properties. No textures, no properties.
			// Only one texture and not HDR and no alt images (no mipmaps or cubemap) -> no properties.
			if (!anyUIDisplayed)
				ImGui::Text("No Editable Image Properties Available");

			ImGui::End();
			return;
		}

		case tSystem::tFileType::KTX:
		case tSystem::tFileType::KTX2:
		{
			bool anyUIDisplayed = false;
			int numTextures = CurrImage->GetNumFrames();
			bool reloadChanges = false;

			tString texTypeName = "Texture";
			anyUIDisplayed |= DoAltMipmapsDisplay(texTypeName);
			anyUIDisplayed |= DoAltCubemapDisplay(texTypeName);
			anyUIDisplayed |= DoChooseDisplayImage(texTypeName);

			// If we're here show options when have 1 or more frames.
			bool altEnabled = CurrImage->IsAltPictureEnabled();
			if (tIsHDRFormat(CurrImage->Info.SrcPixelFormat))
			{
				// Gamma correction. First read current setting and put it in an int.
				int gammaMode = 0;
				if (CurrImage->LoadParams_KTX.Flags & tImageKTX::LoadFlag_GammaCompression)
					gammaMode = 1;
				if (CurrImage->LoadParams_KTX.Flags & tImageKTX::LoadFlag_SRGBCompression)
					gammaMode = 2;

				const char* gammaCorrectItems[] = { "None", "Gamma", "sRGB" };
				ImGui::PushItemWidth(110);
				if (ImGui::Combo("Gamma Correct", &gammaMode, gammaCorrectItems, tNumElements(gammaCorrectItems)))
				{
					CurrImage->LoadParams_KTX.Flags &= ~(tImageKTX::LoadFlag_GammaCompression | tImageKTX::LoadFlag_SRGBCompression);
					if (gammaMode == 1) CurrImage->LoadParams_KTX.Flags |= tImageKTX::LoadFlag_GammaCompression;
					if (gammaMode == 2) CurrImage->LoadParams_KTX.Flags |= tImageKTX::LoadFlag_SRGBCompression;
					reloadChanges = true;
				}
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ShowHelpMark
				(
					"Gamma Correction\n"
					"Floating-point pixel formats used for HDR images are often in linear space. Before being displayed\n"
					"on a screen with non-linear response they need to be 'corrected' to gamma or sRGB-space (brightened).\n"
					"\n"
					"Use 'None' if you know the source image data is already in either gamma or sRGB-space.\n\n"
					"Use 'Gamma' if you want control over the gamma exponent being used to do the correction. 2.2 is standard.\n\n"
					"Use 'sRGB' if you want to convert to sRGB-space. This more accurately represents a display's response and\n"
					"is close to a 2.2 gamma but with an extra linear region, a non-unity amplitude, and a slightly larger gamma.",
					false
				);

				if (gammaMode == 1)
				{
					ImGui::PushItemWidth(110);
					if (ImGui::InputFloat("Gamma", &CurrImage->LoadParams_KTX.Gamma, 0.01f, 0.1f, "%.3f"))
						reloadChanges = true;
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ShowHelpMark("Gamma to use [0.5, 4.0]. Hold Ctrl to speedup. Open preferences to edit default gamma value.");
					tMath::tiClamp(CurrImage->LoadParams_KTX.Gamma, 0.5f, 4.0f);
				}

				bool expEnabled = (CurrImage->LoadParams_KTX.Flags & tImageKTX::LoadFlag_ToneMapExposure);
				ImGui::PushItemWidth(110);
				if (ImGui::InputFloat("Exposure", &CurrImage->LoadParams_KTX.Exposure, 0.001f, 0.05f, "%.4f", expEnabled ? 0 : ImGuiInputTextFlags_ReadOnly))
					reloadChanges = true;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if (ImGui::CheckboxFlags("##ExposureEnabled", &CurrImage->LoadParams_KTX.Flags, tImageKTX::LoadFlag_ToneMapExposure))
					reloadChanges = true;
				ImGui::SameLine();
				ShowHelpMark("Exposure adjustment [0.0, 4]. Hold Ctrl to speedup.");
				tMath::tiClamp(CurrImage->LoadParams_KTX.Exposure, 0.0f, 4.0f);

				anyUIDisplayed = true;
			}

			if (tIsLuminanceFormat(CurrImage->Info.SrcPixelFormat))
			{
				if (ImGui::CheckboxFlags("Spread Luminance", &CurrImage->LoadParams_KTX.Flags, tImageKTX::LoadFlag_SpreadLuminance))
					reloadChanges = true;
				ImGui::SameLine();
				ShowHelpMark("Luminance-only dds files are represented in this viewer as having a red channel only,\nIf spread is true, the channel is spread to all RGB channels to create a grey-scale image.");
			}

			bool scrubberDisplayed = false;
			if ((numTextures >= 2) && !altEnabled)
			{
				ImGui::Checkbox("Scrubber", &Config::Current->ShowFrameScrubber);
				anyUIDisplayed = true;
				scrubberDisplayed = true;
			}

			if (anyUIDisplayed)
			{
				if (scrubberDisplayed)
					ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 110.0f);
				if (ImGui::Button("Reset", tVector2(110.0f, 0.0f)))
				{
					CurrImage->ResetLoadParams();
					CurrImage->FrameNum = 0;
					reloadChanges = true;
				}
			}

			if (reloadChanges)
			{
				CurrImage->Unload();
				CurrImage->Load();
				if (altEnabled)
				{
					CurrImage->EnableAltPicture(true);
					CurrImage->Bind();
				}
			}

			// Some KTX files have no available properties. No textures, no properties.
			// Only one texture and not HDR and no alt images (no mipmaps or cubemap) -> no properties.
			if (!anyUIDisplayed)
				ImGui::Text("No Editable Image Properties Available");

			ImGui::End();
			return;
		}

		case tSystem::tFileType::HDR:
		{
			ImGui::Text("Radiance HDR");
			bool reloadChanges = false;

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Gamma", &CurrImage->LoadParams_HDR.Gamma, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0]. Hold Ctrl to speedup. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams_HDR.Gamma, 0.6f, 3.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputInt("Exposure", &CurrImage->LoadParams_HDR.Exposure))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10, 10]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams_HDR.Exposure, -10, 10);

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 110.0f);
			if (ImGui::Button("Reset", tVector2(110.0f, 0.0f)))
			{
				CurrImage->ResetLoadParams();
				reloadChanges = true;
			}

			if (reloadChanges)
			{
				CurrImage->Unload();
				CurrImage->Load();
			}

			fileTypeSectionDisplayed = true;
			break;
		}

		case tSystem::tFileType::EXR:
		{
			ImGui::Text("Open EXR");
			bool reloadChanges = false;

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Gamma", &CurrImage->LoadParams_EXR.Gamma, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0]. Hold Ctrl to speedup. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams_EXR.Gamma, 0.6f, 3.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Exposure", &CurrImage->LoadParams_EXR.Exposure, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10.0, 10.0]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams_EXR.Exposure, -10.0f, 10.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Defog", &CurrImage->LoadParams_EXR.Defog, 0.001f, 0.01f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Remove fog strength [0.0, 0.1]. Hold Ctrl to speedup. Try to keep under 0.01");
			tMath::tiClamp(CurrImage->LoadParams_EXR.Defog, 0.0f, 0.1f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Knee Low", &CurrImage->LoadParams_EXR.KneeLow, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Lower bound knee taper [-3.0, 3.0]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams_EXR.KneeLow, -3.0f, 3.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Knee High", &CurrImage->LoadParams_EXR.KneeHigh, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Upper bound knee taper [3.5, 7.5]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams_EXR.KneeHigh, 3.5f, 7.5f);

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 110.0f);
			if (ImGui::Button("Reset", tVector2(110.0f, 0.0f)))
			{
				CurrImage->ResetLoadParams();
				reloadChanges = true;
			}

			if (reloadChanges)
			{
				CurrImage->Unload();
				CurrImage->Load();
			}

			fileTypeSectionDisplayed = true;
			break;
		}
	}

	int numFrames = CurrImage->GetNumFrames();
	if ((numFrames <= 1) && !fileTypeSectionDisplayed)
		ImGui::Text("No Editable Image Properties Available");

	if (numFrames > 1)
	{
		if (fileTypeSectionDisplayed)
			ImGui::Separator();

		ImGui::Text("Frames (%d)", CurrImage->GetNumFrames());

		int oneBasedFrameNum = CurrImage->FrameNum + 1;
		ImGui::PushItemWidth(110);
		if (ImGui::InputInt("Frame", &oneBasedFrameNum))
		{
			CurrImage->FrameNum = oneBasedFrameNum - 1;
			tMath::tiClamp(CurrImage->FrameNum, 0, CurrImage->GetNumFrames()-1);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(); ShowHelpMark("Which image in a multiframe file to display.");

		if (CurrImage->FrameDurationPreviewEnabled)
		{
			ImGui::PushItemWidth(110);
			ImGui::InputFloat("Period", &CurrImage->FrameDurationPreview, 0.01f, 0.1f, "%.4f");
			ImGui::PopItemWidth();

			tMath::tiClamp(CurrImage->FrameDurationPreview, 0.0f, 60.0f);
			ImGui::SameLine();
			if (ImGui::Button("Set All"))
			{
				CurrImage->SetFrameDuration(CurrImage->FrameDurationPreview, true);
				SetWindowTitle();
				CurrImage->FrameDurationPreviewEnabled = false;
			}
			ImGui::SameLine(); ShowHelpMark("Sets every frame period to the preview period in seconds.");

			if (ImGui::Button("1.0s"))	CurrImage->FrameDurationPreview = 1.0f;			ImGui::SameLine();
			if (ImGui::Button("0.5s"))	CurrImage->FrameDurationPreview = 0.5f;			ImGui::SameLine();
			if (ImGui::Button("0.1s"))	CurrImage->FrameDurationPreview = 0.1f;			ImGui::SameLine();
			if (ImGui::Button("30Hz"))	CurrImage->FrameDurationPreview = 1.0f/30.0f;	ImGui::SameLine();
			if (ImGui::Button("60Hz"))	CurrImage->FrameDurationPreview = 1.0f/60.0f;	ImGui::SameLine();
			ShowHelpMark("Predefined frame period buttons.");
		}
		else
		{
			tPicture* currFramePic = CurrImage->GetCurrentPic();
			float duration = currFramePic->Duration;
			char frameDurText[64];
			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Period", &duration, 0.01f, 0.1f, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tMath::tiClamp(duration, 0.0f, 60.0f);
				CurrImage->SetFrameDuration(duration);
				SetWindowTitle();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine(); ShowHelpMark("This frame's period in seconds.");

			if (ImGui::Button("1.0s"))	{ CurrImage->SetFrameDuration(1.0f); SetWindowTitle(); }		ImGui::SameLine();
			if (ImGui::Button("0.5s"))	{ CurrImage->SetFrameDuration(0.5f); SetWindowTitle(); }		ImGui::SameLine();
			if (ImGui::Button("0.1s"))	{ CurrImage->SetFrameDuration(0.1f); SetWindowTitle(); }		ImGui::SameLine();
			if (ImGui::Button("30Hz"))	{ CurrImage->SetFrameDuration(1.0f/30.0f); SetWindowTitle(); }	ImGui::SameLine();
			if (ImGui::Button("60Hz"))	{ CurrImage->SetFrameDuration(1.0f/60.0f); SetWindowTitle(); }	ImGui::SameLine();
			ShowHelpMark("Predefined frame period buttons.");
		}
		ImGui::Checkbox("Preview Period", &CurrImage->FrameDurationPreviewEnabled);
		ImGui::SameLine(); ShowHelpMark("If enabled this number of seconds is used for all frame periods while playing.");
		ImGui::Checkbox("Scrubber", &Config::Current->ShowFrameScrubber);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

		uint64 loopImageID = CurrImage->FramePlayLooping ? Image_PlayOnce.Bind() : Image_PlayLoop.Bind();
		if (ImGui::ImageButton(ImTextureID(loopImageID), tVector2(18.0f, 18.0f), tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 2, ColourBG, ColourEnabledTint))
			CurrImage->FramePlayLooping = !CurrImage->FramePlayLooping;
		ImGui::SameLine();

		bool prevEnabled = !CurrImage->FramePlaying && (CurrImage->FrameNum > 0);
		ImGui::PushID("PropSkipBegin");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_SkipEnd_SkipBegin.Bind()), tVector2(18.0f, 18.0f), tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 2,
			ColourBG, prevEnabled ? ColourEnabledTint : ColourDisabledTint) && prevEnabled
		)	CurrImage->FrameNum = 0;
		ImGui::PopID();
		ImGui::SameLine();

		ImGui::PushID("PropPrev");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Next_Prev.Bind()), tVector2(18.0f, 18.0f), tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 2,
			ColourBG, prevEnabled ? ColourEnabledTint : ColourDisabledTint) && prevEnabled
		)	CurrImage->FrameNum = tClampMin(CurrImage->FrameNum-1, 0);
		ImGui::PopID();
		ImGui::SameLine();

		bool playRevEnabled = !(CurrImage->FramePlaying && !CurrImage->FramePlayRev);
		uint64 playRevImageID = !CurrImage->FramePlaying ? Image_Play_PlayRev.Bind() : Image_Stop.Bind();
		ImGui::PushID("PropPlayRev");
		if (ImGui::ImageButton
		(
			ImTextureID(playRevImageID), tVector2(18.0f, 18.0f), tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 2,
			ColourBG, playRevEnabled ? ColourEnabledTint : ColourDisabledTint) && playRevEnabled
		)
		{
			CurrImage->FramePlayRev = true;
			if (CurrImage->FramePlaying) CurrImage->Stop(); else CurrImage->Play();
		}
		ImGui::PopID();
		ImGui::SameLine();

		bool playFwdEnabled = !(CurrImage->FramePlaying && CurrImage->FramePlayRev);
		uint64 playImageID = !CurrImage->FramePlaying ? Image_Play_PlayRev.Bind() : Image_Stop.Bind();
		ImGui::PushID("PropPlayFwd");
		if (ImGui::ImageButton
		(
			ImTextureID(playImageID), tVector2(18.0f, 18.0f), tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 2,
			ColourBG, playFwdEnabled ? ColourEnabledTint : ColourDisabledTint) && playFwdEnabled
		)
		{
			CurrImage->FramePlayRev = false;
			if (CurrImage->FramePlaying) CurrImage->Stop(); else CurrImage->Play();
		}
		ImGui::PopID();
		ImGui::SameLine();

		bool nextEnabled = !CurrImage->FramePlaying && (CurrImage->FrameNum < (numFrames-1));
		ImGui::PushID("PropNext");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_Next_Prev.Bind()), tVector2(18.0f, 18.0f), tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 2,
			ColourBG, nextEnabled ? ColourEnabledTint : ColourDisabledTint) && nextEnabled
		)	CurrImage->FrameNum = tClampMax(CurrImage->FrameNum+1, numFrames-1);
		ImGui::PopID();
		ImGui::SameLine();

		ImGui::PushID("PropSkipEnd");
		if (ImGui::ImageButton
		(
			ImTextureID(Image_SkipEnd_SkipBegin.Bind()), tVector2(18.0f, 18.0f), tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 2,
			ColourBG, nextEnabled ? ColourEnabledTint : ColourDisabledTint) && nextEnabled
		)	CurrImage->FrameNum = numFrames-1;
		ImGui::PopID();
		ImGui::SameLine();
	}

	ImGui::End();
}
