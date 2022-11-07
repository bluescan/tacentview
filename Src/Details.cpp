// Details.cpp
//
// Image details overlay and a meta-data inspector.
//
// Copyright (c) 2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tVector2.h>
#include "imgui.h"
#include "Details.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
using namespace tMath;
using namespace tImage;


void Viewer::ShowImageDetailsOverlay(bool* popen, float x, float y, float w, float h, int cursorX, int cursorY, float zoom)
{
	// This overlay function is pretty much taken from the DearImGui demo code.
	const float margin = 6.0f;

	tVector2 windowPos = tVector2
	(
		x + ((Config::Current->OverlayCorner & 1) ? w - margin : margin),
		y + ((Config::Current->OverlayCorner & 2) ? h - margin : margin)
	);
	tVector2 windowPivot = tVector2
	(
		(Config::Current->OverlayCorner & 1) ? 1.0f : 0.0f,
		(Config::Current->OverlayCorner & 2) ? 1.0f : 0.0f
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
			if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, tVector2(15,15)))
				ImGui::OpenPopup("CopyColourOverlayAs");

			if (ImGui::BeginPopup("CopyColourOverlayAs"))
				ColourCopyAs();

			ImGui::SameLine(); ImGui::Text("(%d, %d, %d, %d)", PixelColour.R, PixelColour.G, PixelColour.B, PixelColour.A);

			Image::ImgInfo& info = CurrImage->Info;
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
				ImGui::Text("Frames: %d", CurrImage->GetNumFrames());
				tString sizeStr; tsPrintf(sizeStr, "File Size: %'d", info.FileSizeBytes);
				ImGui::Text(sizeStr.Chr());
				ImGui::Text("Cursor: (%d, %d)", cursorX, cursorY);
				ImGui::Text("Pan: (%d, %d)", GetPanX(), GetPanY());
				ImGui::Text("Zoom: %.0f%%", zoom);
			}
		}
		ImGui::Text("Images In Folder: %d", Images.GetNumItems());

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left",		nullptr, Config::Current->OverlayCorner == 0)) Config::Current->OverlayCorner = 0;
			if (ImGui::MenuItem("Top-right",	nullptr, Config::Current->OverlayCorner == 1)) Config::Current->OverlayCorner = 1;
			if (ImGui::MenuItem("Bottom-left",  nullptr, Config::Current->OverlayCorner == 2)) Config::Current->OverlayCorner = 2;
			if (ImGui::MenuItem("Bottom-right", nullptr, Config::Current->OverlayCorner == 3)) Config::Current->OverlayCorner = 3;
			if (popen && ImGui::MenuItem("Close")) *popen = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}


void Viewer::ColourCopyAs()
{
	tColourf floatCol(PixelColour);
	ImGui::Text("Copy As...");
	int ri = PixelColour.R; int gi = PixelColour.G; int bi = PixelColour.B; int ai = PixelColour.A;
	float rf = floatCol.R; float gf = floatCol.G; float bf = floatCol.B; float af = floatCol.A;
	tString cpyTxt;

	tsPrintf(cpyTxt, "%d %d %d %d", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%d %d %d", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%d, %d, %d, %d)", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%d, %d, %d)", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "#%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "#%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "0x%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%.3f, %.3f, %.3f, %.3f", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%.3ff, %.3ff, %.3ff, %.3ff", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%.3f, %.3f, %.3f, %.3f)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%.3ff, %.3ff, %.3ff, %.3ff)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	ImGui::EndPopup();
}


namespace ViewerMetaData
{
	int CopyRowIndex = -1;
}


void Viewer::ShowImageMetaDataOverlay(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(6);
	ImGui::SetNextWindowBgAlpha(0.90f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
	ImGuiWindowFlags flags = 
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Meta Data", popen, flags))
	{
		// Get meta data from current image.
		const tMetaData* metaData = CurrImage ? &CurrImage->Cached_MetaData : nullptr;
		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
		const float rowHeight = 18.0f;
		const int maxRowsToDisplay = 28;
		int numDataRows = 1;
		if (metaData && metaData->IsValid())
			numDataRows = metaData->GetNumValidTags();
		const int numRowsToDisplay = tMin(maxRowsToDisplay, numDataRows);
		tVector2 outerSize = ImVec2(0.0f, rowHeight + rowHeight * float(numRowsToDisplay));
		if (ImGui::BeginTable("MetaDataTable", 2, tableFlags, outerSize))
		{
			ImGui::TableSetupColumn("Tag", ImGuiTableColumnFlags_WidthFixed, 120);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 200);
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableHeadersRow();

			if (!metaData)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("No Image");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(" ");
			}
			else if (!metaData->IsValid())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("No Metadata In Image");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(" ");
			}
			else
			{
				bool colHovered = (ImGui::TableGetColumnFlags(0) & ImGuiTableColumnFlags_IsHovered) || (ImGui::TableGetColumnFlags(1) & ImGuiTableColumnFlags_IsHovered);
				if (colHovered  && (ImGui::GetMousePos().y >= ImGui::GetItemRectMin().y) && (ImGui::GetMousePos().y <= ImGui::GetItemRectMax().y))
				{
					ViewerMetaData::CopyRowIndex = -1;
					if (ImGui::IsMouseReleased(1))
						ImGui::OpenPopup("CopyPopup");
				}

				for (int tagIndex = 0; tagIndex < int(tMetaTag::NumTags); tagIndex++)
				{
					tMetaTag tag = tMetaTag(tagIndex);
					tString value = metaData->GetPrettyValue(tag);
					if (!value.IsValid())
						continue;

					tString tagName = tGetMetaTagName(tag);
					tString tagDesc = tGetMetaTagDesc(tag);
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text(tagName.Chr());
					ShowToolTip(tagDesc.Chr());

					ImGui::TableSetColumnIndex(1);	
					ImGui::Text(value.Chr());

					tVector2 rect = ImGui::GetItemRectSize();
					// If the value was truncated, show it all in a tooltip.
					if (rect.x > 188)
						ShowToolTip(value.Chr());

					if (ImGui::IsMouseReleased(1) && colHovered  && (ImGui::GetMousePos().y >= ImGui::GetItemRectMin().y) && (ImGui::GetMousePos().y <= ImGui::GetItemRectMax().y))
					{
						ViewerMetaData::CopyRowIndex = tagIndex;
						ImGui::OpenPopup("CopyPopup");
					}
				}

				if (ImGui::BeginPopup("CopyPopup"))
				{
					if (ViewerMetaData::CopyRowIndex != -1)
					{
						tMetaTag rowTag = tMetaTag(ViewerMetaData::CopyRowIndex);
						tString rowName = tGetMetaTagName(rowTag);

						tString copyRowText = "Copy " + rowName;
						if (ImGui::MenuItem(copyRowText.Chr(), nullptr, false, true))
						{
							tString rowCopyBuf = rowName + ": " + metaData->GetPrettyValue(rowTag);
							ImGui::SetClipboardText(rowCopyBuf.Chr());
						}
					}

					if (ImGui::MenuItem("Copy All",	nullptr, false, true))
					{
						tString copyAllBuf;
						for (int tagIndex = 0; tagIndex < int(tMetaTag::NumTags); tagIndex++)
						{
							tMetaTag tag = tMetaTag(tagIndex);
							tString value = metaData->GetPrettyValue(tag);
							if (!value.IsValid())
								continue;

							tString rowName = tGetMetaTagName(tag);
							copyAllBuf += rowName + ": " + value + "\n";
						}
						ImGui::SetClipboardText(copyAllBuf.Chr());
					}
					ImGui::EndPopup();
				}
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();
}
