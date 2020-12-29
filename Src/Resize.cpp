// Resize.cpp
//
// Dialog for resizing an image.
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

#include "imgui.h"
#include "Resize.h"
#include "Image.h"
#include "TacentView.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;
using namespace tImage;


void Viewer::DoResizeImageModalDialog(bool justOpened)
{
	tAssert(CurrImage);
	tPicture* picture = CurrImage->GetCurrentPic();
	tAssert(picture);

	static int dstW = 512;
	static int dstH = 512;
	int srcW = picture->GetWidth();
	int srcH = picture->GetHeight();

	if (justOpened)
	{
		dstW = picture->GetWidth();
		dstH = picture->GetHeight();
	}

	//
	// Dimension resize.
	//
	ImGui::Text("Dimension Resize");

	float aspect = float(srcW) / float(srcH);
	static bool lockAspect = true;

	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Width", &dstW) && lockAspect)
		dstH = int( float(dstW) / aspect );
	ImGui::PopItemWidth();
	tiClampMin(dstW, 4); tiClampMin(dstH, 4);

	static char lo[32];
	static char hi[32];

	int loP2W = tNextLowerPower2(dstW);		tiClampMin(loP2W, 4);	tsPrintf(lo, "w%d", loP2W);
	int hiP2W = tNextHigherPower2(dstW);							tsPrintf(hi, "w%d", hiP2W);
	ImGui::SameLine(); if (ImGui::Button(lo))
		{ dstW = loP2W; if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); if (ImGui::Button(hi))
		{ dstW = hiP2W; if (lockAspect) dstH = int( float(dstW) / aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output width in pixels.\nIf dimensions match current no scaling.");

	if (ImGui::Checkbox("Lock Aspect", &lockAspect) && lockAspect)
	{
		dstW = srcW;
		dstH = srcH;
	}

	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Height", &dstH) && lockAspect)
		dstW = int( float(dstH) * aspect );
	ImGui::PopItemWidth();
	tiClampMin(dstW, 4); tiClampMin(dstH, 4);

	int loP2H = tNextLowerPower2(dstH);		tiClampMin(loP2H, 4);	tsPrintf(lo, "h%d", loP2H);
	int hiP2H = tNextHigherPower2(dstH);							tsPrintf(hi, "h%d", hiP2H);
	ImGui::SameLine(); if (ImGui::Button(lo))
		{ dstH = loP2H; if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine(); if (ImGui::Button(hi))
		{ dstH = hiP2H; if (lockAspect) dstW = int( float(dstH) * aspect ); }
	ImGui::SameLine(); ShowHelpMark("Final output height in pixels.\nIf dimensions match current no scaling.");

	if ((dstW != srcW) || (dstH != srcH))
	{
		ImGui::Combo("Filter", &Config.ResampleFilter, tResampleFilterNames, tNumElements(tResampleFilterNames), tNumElements(tResampleFilterNames));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");

		ImGui::Combo("Filter Edge Mode", &Config.ResampleEdgeMode, tResampleEdgeModeNames, tNumElements(tResampleEdgeModeNames), tNumElements(tResampleEdgeModeNames));
		ImGui::SameLine();
		ShowHelpMark("How filter chooses pixels along image edges. Use wrap for tiled textures.");	
	}

	if (ImGui::Button("Reset Values", tVector2(100, 0)))
	{
		dstW = srcW;
		dstH = srcH;
	}
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Apply", tVector2(100, 0)))
	{
		if ((dstW != srcW) || (dstH != srcH))
		{
			CurrImage->Resample(dstW, dstH, tImage::tResampleFilter(Config.ResampleFilter), tImage::tResampleEdgeMode(Config.ResampleEdgeMode));
			Viewer::SetWindowTitle();
		}
		ImGui::CloseCurrentPopup();
	}
	
	//ImGui::Separator();
	//ImGui::NewLine();
	//
	// Aspect resize.
	//
	// ImGui::Text("Aspect Resize");

	ImGui::EndPopup();
}
