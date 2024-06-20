// GuiUtil.cpp
//
// Various GUI utility functions used by many of the dialogs.
//
// Copyright (c) 2023, 2024 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "imgui.h"
#include "imgui_internal.h"			// For ProgressArc.
#include <Foundation/tStandard.h>
#include <Image/tImageICO.h>
#include "GuiUtil.h"
#include "TacentView.h"
#include "Image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.
#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
using namespace tMath;
using namespace tSystem;


namespace Gutil
{
	bool Combo(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** outText), void* data, void* desc, int itemsCount, int popupMaxHeightInItems);
	float CalcMaxPopupHeightFromItemCount(int itemsCount);
	bool Text_ArrayGetter(void* data, int idx, const char** outText);
}


void Gutil::SetWindowTitle()
{
	if (!Viewer::Window)
		return;

	tString title = "Tacent View";
	if (Viewer::CurrImage && !Viewer::CurrImage->Filename.IsEmpty())
	{
		title = title + " - " + tGetFileName(Viewer::CurrImage->Filename);
		if (Viewer::CurrImage->IsDirty())
			title += "*";
	}

	glfwSetWindowTitle(Viewer::Window, title.Chr());
}


void Gutil::SetWindowIcon(const tString& icoFile)
{
	#ifdef PLATFORM_LINUX
	tImage::tImageICO icon(icoFile);
	if (!icon.IsValid())
		return;

	const int maxImages = 16;
	GLFWimage* imageTable[maxImages];
	GLFWimage images[maxImages];
	int numImages = tMath::tMin(icon.GetNumFrames(), maxImages);
	for (int i = 0; i < numImages; i++)
	{
		imageTable[i] = &images[i];
		tImage::tFrame* frame = icon.GetFrame(i);
		frame->ReverseRows();
		images[i].width = frame->Width;
		images[i].height = frame->Height;
		images[i].pixels = (uint8*)frame->Pixels;
	}

	// This copies the pixel data out so we can let the tImageICO clean itself up afterwards afterwards.
	glfwSetWindowIcon(Viewer::Window, numImages, *imageTable);
	#endif
}


void Gutil::HelpMark(const char* desc, bool autoWrap)
{
	ImGui::TextDisabled("[?]");
	if (!ImGui::IsItemHovered() || !desc)
		return;

	ImGui::BeginTooltip();
	if (autoWrap)
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	if (autoWrap)
		ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}


void Gutil::ToolTip(const char* desc, bool autoWrap)
{
	if (!ImGui::IsItemHovered() || !desc)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(3.0f, 3.0f));

	ImGui::BeginTooltip();
	if (autoWrap)
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	if (autoWrap)
		ImGui::PopTextWrapPos();
	ImGui::EndTooltip();

	ImGui::PopStyleVar();
}


bool Gutil::Button(const char* label, const tMath::tVector2& size)
{
	return ImGui::Button(label, size) || (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter));
}


void Gutil::ProgressArc(float radius, float percent, const tVector4& colour, const tVector4& colourbg, float thickness, int segments)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	tiSaturate(percent);
	if (percent <= 0.0f)
		return;

	const ImVec2 pos = window->DC.CursorPos;
	window->DrawList->PathArcTo(pos, radius, IM_PI/2.0f-0.10f, IM_PI/2.0f + percent*IM_PI*2.0f +0.10f, segments-1);
	window->DrawList->PathStroke(ImGui::GetColorU32(colourbg), false, thickness+1.5f);

	window->DrawList->PathArcTo(pos, radius, IM_PI/2.0f, IM_PI/2.0f + percent*IM_PI*2.0f, segments-1);
	window->DrawList->PathStroke(ImGui::GetColorU32(colour), false, thickness);
}


void Gutil::Separator()
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
	ImGui::Separator();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
}


float Gutil::CalcMaxPopupHeightFromItemCount(int itemsCount)
{
	ImGuiContext& g = *GImGui;
	if (itemsCount <= 0)
		return FLT_MAX;
	return (g.FontSize + g.Style.ItemSpacing.y) * itemsCount - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}


bool Gutil::Combo(const char* label, int* currentItem, bool (*itemsGetter)(void*, int, const char**), void* data, void* desc, int itemsCount, int popupMaxHeightInItems)
{
	ImGuiContext& g = *GImGui;

	// Call the getter to obtain the preview string which is a parameter to BeginCombo()
	const char* previewValue = nullptr;
	if ((*currentItem >= 0) && (*currentItem < itemsCount))
		itemsGetter(data, *currentItem, &previewValue);

	// The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
	if (popupMaxHeightInItems != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, Gutil::CalcMaxPopupHeightFromItemCount(popupMaxHeightInItems)));

	if (!ImGui::BeginCombo(label, previewValue, ImGuiComboFlags_None))
		return false;

	// Display items
	// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
	bool valueChanged = false;
	for (int i = 0; i < itemsCount; i++)
	{
		ImGui::PushID(i);
		const bool itemSelected = (i == *currentItem);
		const char* itemText = nullptr;
		if (!itemsGetter(data, i, &itemText))
			itemText = "*unknown item*";
		if (ImGui::Selectable(itemText, itemSelected))
		{
			valueChanged = true;
			*currentItem = i;
		}
		ImGui::SameLine();

		const char* descText = nullptr;
		if (!itemsGetter(desc, i, &descText))
			descText = "*no description*";

		HelpMark(descText);
		if (itemSelected)
			ImGui::SetItemDefaultFocus();
		ImGui::PopID();
	}

	ImGui::EndCombo();

	if (valueChanged)
		ImGui::MarkItemEdited(g.LastItemData.ID);

	return valueChanged;
}


bool Gutil::Text_ArrayGetter(void* data, int idx, const char** outText)
{
	const char* const* texts = (const char* const*)data;
	if (outText)
		*outText = texts[idx];
	return true;
}


bool Gutil::Combo(const char* label, int* currentItem, const char* const items[], const char* const descs[], int itemsCount, int heightInItems)
{
	const bool valueChanged = Gutil::Combo(label, currentItem, Gutil::Text_ArrayGetter, (void*)items, (void*)descs, itemsCount, heightInItems);
	return valueChanged;
}


tVector2 Gutil::GetDialogOrigin(DialogID dialogID)
{
	int hindex = int(dialogID) % 4;
	int vindex = int(dialogID) / 4;

	float topOffset		= Gutil::GetUIParamExtent(82.0f, 160.0f);
	float leftOffset	= Gutil::GetUIParamScaled(30.0f, 2.5f);
	float heightDelta	= Gutil::GetUIParamScaled(22.0f, 2.5f);

	float widthDelta = 200.0f;
	float x = leftOffset + widthDelta*float(hindex);
	float y = topOffset + heightDelta*float(vindex);
	return tVector2(x, y);
}


tString Gutil::CropStringToWidth(const tString& toCropStr, float cropWidth, bool ellipsis, float* resultWidth)
{
	if (resultWidth)
		*resultWidth = 0.0f;
	if (toCropStr.IsEmpty())
		return tString();

	tString toCrop(toCropStr);
	ImGuiContext& g = *GImGui;
	ImFont* font = g.Font;
	if (!font || !font->FontSize)
		return tString();

	float scale = g.FontSize/font->FontSize;
	float ellipsisWidth = (tStd::cCodepoint_Ellipsis < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[tStd::cCodepoint_Ellipsis] : font->FallbackAdvanceX) * scale;
	if (ellipsis && (ellipsisWidth > cropWidth))
		return tString();
	if (ellipsis)
		cropWidth -= ellipsisWidth;
	if (cropWidth <= 0)
		return tString();

	float currWidth = 0.0f;
	const char8_t* currCodeUnit = toCrop.Chars();
	int currIndex = 0;
	tString result;
	int toCropLength = toCrop.LengthNullTerminated();
	result.SetLength(toCropLength);
	char8_t* destUnit = result.Text();
	while (currIndex < toCropLength)
	{
		// Get the width of the glyph for the current codeunit.
		char32_t glyph32;
		int srcInc = tStd::tUTF32c(&glyph32, currCodeUnit);

		// Get its rendered width.
		float unitWidth = (glyph32 < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[glyph32] : font->FallbackAdvanceX) * scale;

		// Test what the new width will be to see if we've gone too far.
		if (currWidth+unitWidth > cropWidth)
			break;

		// Write the codeunit into the dest string.
		tStd::tMemcpy(destUnit, currCodeUnit, srcInc);

		// Update width and advance.
		currWidth += unitWidth;
		currIndex += srcInc;
		currCodeUnit += srcInc;
		destUnit += srcInc;
	}

	// Deal with ellipsis. We have reserved enough room to safely append one and stay under the
	// desired width.
	result.ShrinkNullTerminated();
	if (toCropLength > result.LengthNullTerminated())
	{
		result += u8"…";
		currWidth += ellipsisWidth;
	}

	if (resultWidth)
		*resultWidth = currWidth;
	return result;
}
