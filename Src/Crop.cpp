// Crop.cpp
//
// GL and ImGui code that implements cropping images.
//
// Copyright (c) 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include "imgui.h"
#include <Math/tColour.h>
#include <Math/tGeometry.h>
#include <System/tPrint.h>
#include "Crop.h"
#include "TacitTexView.h"
#include "TacitImage.h"
using namespace tMath;


namespace TexView
{
	CropWidget CropGizmo;

	tColour CropHovCol		(0.25f, 0.55f, 1.00f, 1.00f);
	tColour CropCol			(1.00f, 1.00f, 1.00f, 1.00f);
}


void TexView::CropWidget::MouseButton(CropLine& line, bool down, float mouse)
{
	// Up or down.
	line.V += line.PressedDelta;
	line.PressedDelta = 0.0f;

	if (down)
	{
		if (line.Hovered && !line.Pressed)
		{
			line.PressedAnchor = mouse;
			line.Pressed = true;
		}
	}
	else
	{
		line.Pressed = false;
	}
}


void TexView::CropWidget::MouseHovered(CropLine& line, const tVector2& mouse, const tVector2& ends, bool horizontal)
{
	tARect2 hitBox;
	if (horizontal)
		hitBox.Set(tVector2(ends.x, line.V+line.PressedDelta), tVector2(ends.y, line.V+line.PressedDelta));
	else
		hitBox.Set(tVector2(line.V+line.PressedDelta, ends.x), tVector2(line.V+line.PressedDelta, ends.y));
	hitBox.Expand(4);
	line.Hovered = hitBox.IsPointInside(mouse);
}


void TexView::CropWidget::MouseButton(bool down, const tVector2& mouse)
{
	MouseButton(LineB, down, mouse.y);
	MouseButton(LineT, down, mouse.y);
	MouseButton(LineL, down, mouse.x);
	MouseButton(LineR, down, mouse.x);
}


void TexView::CropWidget::SetLines(const tVector4& lines)
{
	LineL.V = lines.L;	LineL.PressedDelta = 0.0f;
	LineR.V = lines.R;	LineR.PressedDelta = 0.0f;
	LineT.V = lines.T;	LineT.PressedDelta = 0.0f;
	LineB.V = lines.B;	LineB.PressedDelta = 0.0f;
}


void TexView::CropWidget::UpdateDraw
(
	const tVector4& imgext, const tVector2& mouse,
	const tVector2& uvmarg, const tVector2& uvoffset
)
{
	if (LineB.Pressed)
		LineB.PressedDelta = mouse.y - LineB.PressedAnchor;
	if (LineT.Pressed)
		LineT.PressedDelta = mouse.y - LineT.PressedAnchor;
	if (LineL.Pressed)
		LineL.PressedDelta = mouse.x - LineL.PressedAnchor;
	if (LineR.Pressed)
		LineR.PressedDelta = mouse.x - LineR.PressedAnchor;

	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;

	MouseHovered(LineB, mouse, tVector2(l, r), true);
	MouseHovered(LineT, mouse, tVector2(l, r), true);
	MouseHovered(LineL, mouse, tVector2(b, t), false);
	MouseHovered(LineR, mouse, tVector2(b, t), false);

	ConstrainCropLines(imgext, false);
	DrawMatt(imgext, uvmarg, uvoffset);
	DrawLines();
	DrawHandles();
}


void TexView::CropWidget::ConstrainCropLines(const tVector4& imgext, bool forceAll)
{
	if (LineL.Pressed || forceAll)
	{
		if (LineL.V + LineL.PressedDelta + 8 > LineR.V + LineR.PressedDelta)
			LineL.PressedDelta = LineR.V + LineR.PressedDelta - LineL.V - 8;
		if (LineL.V + LineL.PressedDelta < imgext.L)
			LineL.PressedDelta = imgext.L - LineL.V;
	}

	if (LineR.Pressed || forceAll)
	{
		if (LineR.V + LineR.PressedDelta - 8 < LineL.V + LineL.PressedDelta)
			LineR.PressedDelta = LineL.V + LineL.PressedDelta - LineR.V + 8;
		if (LineR.V + LineR.PressedDelta > imgext.R)
			LineR.PressedDelta = imgext.R - LineR.V;
	}

	if (LineB.Pressed || forceAll)
	{
		if (LineB.V + LineB.PressedDelta + 8 > LineT.V + LineT.PressedDelta)
			LineB.PressedDelta = LineT.V + LineT.PressedDelta - LineB.V - 8;
		if (LineB.V + LineB.PressedDelta < imgext.B)
			LineB.PressedDelta = imgext.B - LineB.V;
	}

	if (LineT.Pressed || forceAll)
	{
		if (LineT.V + LineT.PressedDelta - 8 < LineB.V + LineB.PressedDelta)
			LineT.PressedDelta = LineB.V + LineB.PressedDelta - LineT.V + 8;
		if (LineT.V + LineT.PressedDelta > imgext.T)
			LineT.PressedDelta = imgext.T - LineT.V;
	}
}


void TexView::CropWidget::DrawMatt(const tVector4& imext, const tVector2& uvmarg, const tVector2& uvoffset)
{
	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;

	tVector2 scrCropMin(LineL.Get(), LineB.Get());
	tVector2 scrCropMax(LineR.Get(), LineT.Get());
	int minX, minY;
	ConvertScreenPosToImagePos(minX, minY, scrCropMin, imext, uvmarg, uvoffset);
	int maxX, maxY;
	ConvertScreenPosToImagePos(maxX, maxY, scrCropMax, imext, uvmarg, uvoffset);

	maxX += 1; maxY += 1;
	tVector2 bl;
	ConvertImagePosToScreenPos(bl, minX, minY, imext, uvmarg, uvoffset);

	tVector2 tr;
	ConvertImagePosToScreenPos(tr, maxX, maxY, imext, uvmarg, uvoffset);

	glColor4f(ColourClear.x, ColourClear.y, ColourClear.z, 0.75f);
	glBegin(GL_QUAD_STRIP);
	glVertex2f(imext.L, imext.B);
	glVertex2f(bl.x, bl.y);
	glVertex2f(imext.R, imext.B);
	glVertex2f(tr.x, bl.y);
	glVertex2f(imext.R, imext.T);
	glVertex2f(tr.x, tr.y);
	glVertex2f(imext.L, imext.T);
	glVertex2f(bl.x, tr.y);
	glVertex2f(imext.L, imext.B);
	glVertex2f(bl.x, bl.y);
	glEnd();
}


void TexView::CropWidget::DrawLines()
{
	glColor4fv(tColourf::white.E);
	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;
	bool anyPressed = LineL.Pressed || LineR.Pressed || LineB.Pressed || LineT.Pressed;
	glBegin(GL_LINES);

	glColor4fv((!anyPressed && LineB.Hovered) || LineB.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(l,	b);
	glVertex2f(r+1,	b);

	glColor4fv((!anyPressed && LineR.Hovered) || LineR.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(r+1,	b);
	glVertex2f(r+1,	t+1);

	glColor4fv((!anyPressed && LineT.Hovered) || LineT.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(r+1,	t+1);
	glVertex2f(l,	t+1);

	glColor4fv((!anyPressed && LineL.Hovered) || LineL.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(l,	t+1);
	glVertex2f(l,	b);

	glColor4fv(tColourf::white.E);
	glEnd();
}


void TexView::CropWidget::DrawHandles()
{
	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;
	bool anyPressed = LineL.Pressed || LineR.Pressed || LineB.Pressed || LineT.Pressed;

	glBegin(GL_QUADS);

	glColor4fv
	(
		(!anyPressed && LineL.Hovered && LineB.Hovered) || (LineL.Pressed && LineB.Pressed) ? CropHovCol.E : CropCol.E
	);
	glVertex2f(l-4,		b-4);
	glVertex2f(l+3,		b-4);
	glVertex2f(l+3,		b+3);
	glVertex2f(l-4,		b+3);

	glColor4fv
	(
		(!anyPressed && LineR.Hovered && LineB.Hovered) || (LineR.Pressed && LineB.Pressed) ? CropHovCol.E : CropCol.E
	);
	glVertex2f(r-3,		b-4);
	glVertex2f(r+4,		b-4);
	glVertex2f(r+4,		b+3);
	glVertex2f(r-3,		b+3);

	glColor4fv
	(
		(!anyPressed && LineR.Hovered && LineT.Hovered) || (LineR.Pressed && LineT.Pressed) ? CropHovCol.E : CropCol.E
	);
	glVertex2f(r-3,		t-3);
	glVertex2f(r+4,		t-3);
	glVertex2f(r+4,		t+4);
	glVertex2f(r-3,		t+4);

	glColor4fv
	(
		(!anyPressed && LineL.Hovered && LineT.Hovered) || (LineL.Pressed && LineT.Pressed) ? CropHovCol.E : CropCol.E
	);
	glVertex2f(l-4,		t-3);
	glVertex2f(l+3,		t-3);
	glVertex2f(l+3,		t+4);
	glVertex2f(l-4,		t+4);

	glColor4fv(tColourf::white.E);
	glEnd();
}


void TexView::ShowCropPopup(const tVector4& lrtb, const tVector2& uvmarg, const tVector2& uvoffset)
{
	if (!CurrImage)
	{
		CropMode = false;
		return;
	}

	static bool cropMode = false;
	bool justOpened = false;
	if (CropMode && !cropMode)
		justOpened = true;
	cropMode = CropMode;

	if (!CropMode)
		return;

	tVector2 windowPos = tVector2((lrtb.R+lrtb.L)/2.0f - 90.0f, 48.0f);
	if (justOpened)
		ImGui::SetNextWindowPos(windowPos);
	else
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	ImGui::SetNextWindowSize(tVector2(180, 160), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.70f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Crop", &CropMode, flags))
	{
		tVector2 scrCropMin(CropGizmo.LineL.Get(), CropGizmo.LineB.Get());
		tVector2 scrCropMax(CropGizmo.LineR.Get(), CropGizmo.LineT.Get());
		int minX, minY;
		ConvertScreenPosToImagePos(minX, minY, scrCropMin, lrtb, uvmarg, uvoffset);
		int maxX, maxY;
		ConvertScreenPosToImagePos(maxX, maxY, scrCropMax, lrtb, uvmarg, uvoffset);

		int origW = CurrImage->GetWidth();
		int origH = CurrImage->GetHeight();
		int newW = tClampMin(maxX - minX + 1, 1);
		int newH = tClampMin(maxY - minY + 1, 1);

		int margL = minX;
		int margR = origW - maxX - 1;
		int margB = minY;
		int margT = origH - maxY - 1;

		float col = ImGui::GetCursorPosX() + 70.0f;
		ImGui::Text("Bot Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", minX, minY);
		ImGui::Text("Top Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", maxX, maxY);
		ImGui::Text("H Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("L:%d R:%d", margL, margR);
		ImGui::Text("V Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("B:%d T:%d", margB, margT);
		ImGui::Text("Orig Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", origW, origH);
		ImGui::Text("New Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", newW, newH);
		if ((maxX - minX + 1 < 4) || (maxY - minY + 1 < 4))
		{
			ImGui::SameLine();
			ImGui::Text("Min 4x4");
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);

		if (ImGui::Button("Cancel", tVector2(64, 0)))
			CropMode = false;

		ImGui::SameLine();	
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 64.0f);

		if (ImGui::Button("Apply", tVector2(64, 0)))
		{
			newW = tClampMin(newW, 4);
			newH = tClampMin(newH, 4);

			CurrImage->Unbind();
			CurrImage->Crop(newW, newH, minX, minY);
			CurrImage->Bind();
			TexView::SetWindowTitle();
			CropMode = false;
		}
	}
	ImGui::End();
}
