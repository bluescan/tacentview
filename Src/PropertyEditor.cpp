// PropertyEditor.cpp
//
// Image property editor window.
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
#include "PropertyEditor.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
using namespace tMath;


void Viewer::ShowPropertyEditorWindow(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize; // | ImGuiWindowFlags_NoFocusOnAppearing;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = GetDialogOrigin(1);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Property Editor", popen, windowFlags))
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
			ImGui::PushItemWidth(110);

			ImGui::InputFloat("Gamma", &CurrImage->LoadParams.GammaValue, 0.01f, 0.1f, "%.3f"); ImGui::SameLine();
			ShowHelpMark("Gamma to use [0.6, 3.0] if you reload this Radiance hdr file. Open preferences to edit default gamma value.");
			tMath::tiClamp(CurrImage->LoadParams.GammaValue, 0.6f, 3.0f);

			ImGui::InputInt("Exposure", &CurrImage->LoadParams.HDR_Exposure); ImGui::SameLine();
			ShowHelpMark("Exposure adjustment [-10, 10] if you reload this Radiance hdr file.");
			tMath::tiClamp(CurrImage->LoadParams.HDR_Exposure, -10, 10);
			ImGui::PopItemWidth();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
			ImGui::Separator();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

			if (ImGui::Button("Reset", tVector2(100, 0)))
			{
				CurrImage->ResetLoadParams();
				CurrImage->Unload();
				CurrImage->Load();
			}
			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
			if (ImGui::Button("Reload", tVector2(100, 0)))
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

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
			ImGui::Separator();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

			if (ImGui::Button("Reset", tVector2(100, 0)))
			{
				CurrImage->ResetLoadParams();
				CurrImage->Unload();
				CurrImage->Load();
			}
			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
			if (ImGui::Button("Reload", tVector2(100, 0)))
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
		ImGui::PushItemWidth(110);

		int oneBasedFrameNum = CurrImage->FrameNum + 1;
		if (ImGui::InputInt("Frame", &oneBasedFrameNum))
		{
			CurrImage->FrameNum = oneBasedFrameNum - 1;
			tMath::tiClamp(CurrImage->FrameNum, 0, CurrImage->GetNumFrames()-1);
		}
		ImGui::SameLine(); ShowHelpMark("Which image in a multiframe file to display.");

		if (CurrImage->FrameDurationPreviewEnabled)
		{
			ImGui::InputFloat("Seconds", &CurrImage->FrameDurationPreview, 0.01f, 0.1f, "%.4f");
			tMath::tiClamp(CurrImage->FrameDurationPreview, 0.0f, 60.0f);
			ImGui::SameLine();
			if (ImGui::Button("Set All"))
			{
				CurrImage->SetFrameDuration(CurrImage->FrameDurationPreview, true);
				SetWindowTitle();
				CurrImage->FrameDurationPreviewEnabled = false;
			}
			ImGui::SameLine(); ShowHelpMark("Sets every frame duration to the preview duration.");

			if (ImGui::Button("1.0s"))  CurrImage->FrameDurationPreview = 1.0f; ImGui::SameLine();
			if (ImGui::Button("0.5s"))  CurrImage->FrameDurationPreview = 0.5f; ImGui::SameLine();
			if (ImGui::Button("0.1s"))  CurrImage->FrameDurationPreview = 0.1f; ImGui::SameLine();
			if (ImGui::Button("30fps")) CurrImage->FrameDurationPreview = 1.0f/30.0f; ImGui::SameLine();
			if (ImGui::Button("60fps")) CurrImage->FrameDurationPreview = 1.0f/60.0f; ImGui::SameLine();
			ShowHelpMark("Predefined frame duration buttons.");
		}
		else
		{
			tImage::tPicture* currFramePic = CurrImage->GetCurrentPic();
			float duration = currFramePic->Duration;
			char frameDurText[64];
			if (ImGui::InputFloat("Seconds", &duration, 0.01f, 0.1f, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tMath::tiClamp(duration, 0.0f, 60.0f);
				CurrImage->SetFrameDuration(duration);
				SetWindowTitle();
			}

			if (ImGui::Button("1.0s"))	{ CurrImage->SetFrameDuration(1.0f); SetWindowTitle(); }		ImGui::SameLine();
			if (ImGui::Button("0.5s"))	{ CurrImage->SetFrameDuration(0.5f); SetWindowTitle(); }		ImGui::SameLine();
			if (ImGui::Button("0.1s"))	{ CurrImage->SetFrameDuration(0.1f); SetWindowTitle(); }		ImGui::SameLine();
			if (ImGui::Button("30fps"))	{ CurrImage->SetFrameDuration(1.0f/30.0f); SetWindowTitle(); }	ImGui::SameLine();
			if (ImGui::Button("60fps"))	{ CurrImage->SetFrameDuration(1.0f/60.0f); SetWindowTitle(); }	ImGui::SameLine();
			ShowHelpMark("Predefined frame duration buttons.");
		}
		ImGui::Checkbox("Preview Duration", &CurrImage->FrameDurationPreviewEnabled);
		ImGui::SameLine(); ShowHelpMark("If enabled this number of seconds is used for all frame durations while playing.");
		ImGui::Checkbox("Scrubber", &Config::Current.ShowFrameScrubber);

		ImGui::PopItemWidth();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 11);

		uint64 loopImageID = CurrImage->FramePlayLooping ? PlayOnceImage.Bind() : PlayLoopImage.Bind();
		if (ImGui::ImageButton(ImTextureID(loopImageID), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2, ColourBG, ColourEnabledTint))
			CurrImage->FramePlayLooping = !CurrImage->FramePlayLooping;
		ImGui::SameLine();

		bool prevEnabled = !CurrImage->FramePlaying && (CurrImage->FrameNum > 0);
		if (ImGui::ImageButton
		(
			ImTextureID(SkipBeginImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, prevEnabled ? ColourEnabledTint : ColourDisabledTint) && prevEnabled
		)	CurrImage->FrameNum = 0;
		ImGui::SameLine();

		if (ImGui::ImageButton
		(
			ImTextureID(PrevImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, prevEnabled ? ColourEnabledTint : ColourDisabledTint) && prevEnabled
		)	CurrImage->FrameNum = tClampMin(CurrImage->FrameNum-1, 0);
		ImGui::SameLine();

		bool playRevEnabled = !(CurrImage->FramePlaying && !CurrImage->FramePlayRev);
		uint64 playRevImageID = !CurrImage->FramePlaying ? PlayRevImage.Bind() : StopRevImage.Bind();
		if (ImGui::ImageButton
		(
			ImTextureID(playRevImageID), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, playRevEnabled ? ColourEnabledTint : ColourDisabledTint) && playRevEnabled
		)
		{
			CurrImage->FramePlayRev = true;
			if (CurrImage->FramePlaying) CurrImage->Stop(); else CurrImage->Play();
		}
		ImGui::SameLine();

		bool playFwdEnabled = !(CurrImage->FramePlaying && CurrImage->FramePlayRev);
		uint64 playImageID = !CurrImage->FramePlaying ? PlayImage.Bind() : StopImage.Bind();
		if (ImGui::ImageButton
		(
			ImTextureID(playImageID), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, playFwdEnabled ? ColourEnabledTint : ColourDisabledTint) && playFwdEnabled
		)
		{
			CurrImage->FramePlayRev = false;
			if (CurrImage->FramePlaying) CurrImage->Stop(); else CurrImage->Play();
		}
		ImGui::SameLine();

		bool nextEnabled = !CurrImage->FramePlaying && (CurrImage->FrameNum < (numFrames-1));
		if (ImGui::ImageButton
		(
			ImTextureID(NextImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, nextEnabled ? ColourEnabledTint : ColourDisabledTint) && nextEnabled
		)	CurrImage->FrameNum = tClampMax(CurrImage->FrameNum+1, numFrames-1);
		ImGui::SameLine();

		if (ImGui::ImageButton
		(
			ImTextureID(SkipEndImage.Bind()), tVector2(18, 18), tVector2(0, 1), tVector2(1, 0), 2,
			ColourBG, nextEnabled ? ColourEnabledTint : ColourDisabledTint) && nextEnabled
		)	CurrImage->FrameNum = numFrames-1;
		ImGui::SameLine();
	}

	ImGui::End();
}
