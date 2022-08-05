// Properties.cpp
//
// Image properties display and editor window.
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

#include "imgui.h"
#include "Properties.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
using namespace tMath;


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
			// DDS files get their own properties UI.
			int numTextures = CurrImage->GetNumFrames();
			if (numTextures < 1)
			{
				ImGui::Text("No Editable Image Properties Available");
				ImGui::End();
				return;
			}

			bool altMipmapsPicAvail = CurrImage->IsAltMipmapsPictureAvail() && !CropMode;
			bool altCubemapPicAvail = CurrImage->IsAltCubemapPictureAvail() && !CropMode;
			if (numTextures > 1)
			{
				tString texName = "Texture";
				if (altMipmapsPicAvail)
					texName = "Mipmap";
				else if (altCubemapPicAvail)
					texName = "Cubemap Side";

				ImGui::Text("%ss (%d)", texName.Chr(), numTextures);

				int oneBasedTextureNum = CurrImage->FrameNum + 1;
				ImGui::PushItemWidth(110);
				if (ImGui::InputInt(texName.Chr(), &oneBasedTextureNum))
				{
					CurrImage->FrameNum = oneBasedTextureNum - 1;
					tMath::tiClamp(CurrImage->FrameNum, 0, numTextures-1);
				}
				ImGui::PopItemWidth();
				ImGui::SameLine(); ShowHelpMark("Which mipmap or cubemap side to display.\nCubemap sides left-handed +X,-X,+Y,-Y,+Z,-Z");
			}

			bool altMipmapsPicEnabl = altMipmapsPicAvail && CurrImage->IsAltPictureEnabled();
			if (altMipmapsPicAvail)
			{
				if (ImGui::Checkbox("Display Mipmaps", &altMipmapsPicEnabl))
				{
					CurrImage->EnableAltPicture(altMipmapsPicEnabl);
					CurrImage->Bind();
				}
				ShowToolTip("Display all mipmaps.");
			}

			bool altCubemapPicEnabl = altCubemapPicAvail && CurrImage->IsAltPictureEnabled();
			if (altCubemapPicAvail)
			{
				if (ImGui::Checkbox("Display As Cubemap", &altCubemapPicEnabl))
				{
					CurrImage->EnableAltPicture(altCubemapPicEnabl);
					CurrImage->Bind();
				}
				ShowToolTip("Display all cubemap sides in a T-layout.");
			}

			if (!altMipmapsPicAvail && !altCubemapPicAvail && (numTextures <= 1))
				ImGui::Text("No Editable Image Properties Available");

			if (numTextures > 1)
				ImGui::Checkbox("Scrubber", &Config::Current->ShowFrameScrubber);

			ImGui::End();
			return;
		}

		case tSystem::tFileType::HDR:
		{
			ImGui::Text("Radiance HDR");
			bool reloadChanges = false;

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Gamma", &CurrImage->LoadParams.GammaValue, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0]. Hold Ctrl to speedup. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams.GammaValue, 0.6f, 3.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputInt("Exposure", &CurrImage->LoadParams.HDR_Exposure))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10, 10]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams.HDR_Exposure, -10, 10);

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
			if (ImGui::InputFloat("Gamma", &CurrImage->LoadParams.GammaValue, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0]. Hold Ctrl to speedup. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams.GammaValue, 0.6f, 3.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Exposure", &CurrImage->LoadParams.EXR_Exposure, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10.0, 10.0]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams.EXR_Exposure, -10.0f, 10.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Defog", &CurrImage->LoadParams.EXR_Defog, 0.001f, 0.01f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Remove fog strength [0.0, 0.1]. Hold Ctrl to speedup. Try to keep under 0.01");
			tMath::tiClamp(CurrImage->LoadParams.EXR_Defog, 0.0f, 0.1f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Knee Low", &CurrImage->LoadParams.EXR_KneeLow, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Lower bound knee taper [-3.0, 3.0]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams.EXR_KneeLow, -3.0f, 3.0f);

			ImGui::PushItemWidth(110);
			if (ImGui::InputFloat("Knee High", &CurrImage->LoadParams.EXR_KneeHigh, 0.01f, 0.1f, "%.3f"))
				reloadChanges = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ShowHelpMark("Upper bound knee taper [3.5, 7.5]. Hold Ctrl to speedup.");
			tMath::tiClamp(CurrImage->LoadParams.EXR_KneeHigh, 3.5f, 7.5f);

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
			tImage::tPicture* currFramePic = CurrImage->GetCurrentPic();
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
