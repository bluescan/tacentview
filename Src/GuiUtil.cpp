// GuiUtil.cpp
//
// Various GUI utility functions used by many of the dialogs.
//
// Copyright (c) 2023 Tristan Grimmer.
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


void Gutil::ShowHelpMark(const char* desc, bool autoWrap)
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


void Gutil::ShowToolTip(const char* desc, bool autoWrap)
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
