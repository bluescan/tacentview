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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include <Math/tColour.h>
#include <Math/tGeometry.h>
#include <System/tPrint.h>
#include "Crop.h"
#include "TacentView.h"
#include "Image.h"
using namespace tMath;


namespace Viewer
{
	CropWidget CropGizmo;

	tColour CropHovCol		(0.15f, 0.45f, 1.00f, 1.00f);
	tColour CropCol			(0.85f, 0.85f, 0.85f, 1.00f);
}


void Viewer::CropWidget::MouseButton(CropLine& line, bool down, float mouse)
{
	// Up or down.
	line.ScreenVal += line.PressedDelta;
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


void Viewer::CropWidget::MouseHovered(CropLine& line, const tVector2& mouse, const tVector2& ends, bool horizontal)
{
	tARect2 hitBox;
	if (horizontal)
		hitBox.Set(tVector2(ends.x, line.GetScreenVal()), tVector2(ends.y, line.GetScreenVal()));
	else
		hitBox.Set(tVector2(line.GetScreenVal(), ends.x), tVector2(line.GetScreenVal(), ends.y));
	hitBox.Expand(4);
	line.Hovered = hitBox.IsPointInside(mouse);
}


void Viewer::CropWidget::MouseButton(bool down, const tVector2& mouse)
{
	MouseButton(LineB, down, mouse.y);
	MouseButton(LineT, down, mouse.y);
	MouseButton(LineL, down, mouse.x);
	MouseButton(LineR, down, mouse.x);
}


void Viewer::CropWidget::Update(const tVector4& imgext, const tVector2& mouse, const tVector2& uvoffset)
{
	if (LineB.Pressed)
		LineB.PressedDelta = mouse.y - LineB.PressedAnchor;
	if (LineT.Pressed)
		LineT.PressedDelta = mouse.y - LineT.PressedAnchor;
	if (LineL.Pressed)
		LineL.PressedDelta = mouse.x - LineL.PressedAnchor;
	if (LineR.Pressed)
		LineR.PressedDelta = mouse.x - LineR.PressedAnchor;

	float l = LineL.GetScreenVal();
	float r = LineR.GetScreenVal();
	float b = LineB.GetScreenVal();
	float t = LineT.GetScreenVal();

	MouseHovered(LineB, mouse, tVector2(l, r), true);
	MouseHovered(LineT, mouse, tVector2(l, r), true);
	MouseHovered(LineL, mouse, tVector2(b, t), false);
	MouseHovered(LineR, mouse, tVector2(b, t), false);

	ConstrainCropLines(imgext, false);

	if (LineL.Pressed || LineR.Pressed || LineT.Pressed || LineB.Pressed)
	{
		tVector2 scrPosMin(LineL.GetScreenVal(), LineB.GetScreenVal());
		ConvertScreenPosToImagePos(LineL.ImageVal, LineB.ImageVal, scrPosMin, imgext, uvoffset);
		tVector2 scrPosMax(LineR.GetScreenVal(), LineT.GetScreenVal());
		ConvertScreenPosToImagePos(LineR.ImageVal, LineT.ImageVal, scrPosMax, imgext, uvoffset);
	}
	else
	{
		SetLines(LineL.ImageVal, LineR.ImageVal, LineT.ImageVal, LineB.ImageVal, imgext, uvoffset);
	}

	DrawMatt(imgext, uvoffset);
	DrawLines();
	DrawHandles();
}


void Viewer::CropWidget::SetLines(int left, int right, int top, int bottom, const tMath::tVector4& imgext, const tMath::tVector2& uvoffset)
{
	tVector2 min;
	ConvertImagePosToScreenPos(min, left, bottom, imgext, uvoffset, true);
	tVector2 max;
	ConvertImagePosToScreenPos(max, right, top, imgext, uvoffset, true);

	LineL.ImageVal = left;
	LineR.ImageVal = right;
	LineT.ImageVal = top;
	LineB.ImageVal = bottom;

	LineL.ScreenVal = min.x;	LineL.PressedDelta = 0.0f;
	LineR.ScreenVal = max.x;	LineR.PressedDelta = 0.0f;
	LineT.ScreenVal = max.y;	LineT.PressedDelta = 0.0f;
	LineB.ScreenVal = min.y;	LineB.PressedDelta = 0.0f;
}


void Viewer::CropWidget::ConstrainLines(int l, int r, int t, int b, const tMath::tVector4& imgext, const tMath::tVector2& uvoffset)
{
	tVector2 min; ConvertImagePosToScreenPos(min, l, b, imgext, uvoffset, true);
	tVector2 max; ConvertImagePosToScreenPos(max, r, t, imgext, uvoffset, true);
	if (LineL.ImageVal < l)
	{
		LineL.ImageVal = l; LineL.ScreenVal = min.x; LineL.PressedDelta = 0.0f;
	}
	if (LineR.ImageVal > r)
	{
		LineR.ImageVal = r; LineR.ScreenVal = max.x; LineR.PressedDelta = 0.0f;
	}
	if (LineT.ImageVal > t)
	{
		LineT.ImageVal = t; LineT.ScreenVal = max.y; LineT.PressedDelta = 0.0f;
	}
	if (LineB.ImageVal < b)
	{
		LineB.ImageVal = b; LineB.ScreenVal = min.y; LineB.PressedDelta = 0.0f;
	}
}


void Viewer::CropWidget::ConstrainCropLines(const tVector4& imgext, bool forceAll)
{
	float scrPixelsPerImgPixelW = (imgext.R-imgext.L)/CurrImage->GetWidth();
	float scrPixelsPerImgPixelH = (imgext.T-imgext.B)/CurrImage->GetHeight();

	if (LineL.Pressed || forceAll)
	{
		if (LineL.GetScreenVal() + 3.0f*scrPixelsPerImgPixelW > LineR.GetScreenVal())
			LineL.PressedDelta = LineR.GetScreenVal() - LineL.ScreenVal - 3.0f*scrPixelsPerImgPixelW;
		if (LineL.GetScreenVal() < imgext.L)
			LineL.PressedDelta = imgext.L - LineL.ScreenVal;
	}

	if (LineR.Pressed || forceAll)
	{
		if (LineR.GetScreenVal()- 3.0f*scrPixelsPerImgPixelW < LineL.GetScreenVal())
			LineR.PressedDelta = LineL.GetScreenVal() - LineR.ScreenVal + 3.0f*scrPixelsPerImgPixelW;
		if (LineR.GetScreenVal() > imgext.R)
			LineR.PressedDelta = imgext.R - LineR.ScreenVal;
	}

	if (LineB.Pressed || forceAll)
	{
		if (LineB.GetScreenVal() + 3.0f*scrPixelsPerImgPixelH > LineT.GetScreenVal())
			LineB.PressedDelta = LineT.GetScreenVal() - LineB.ScreenVal - 3.0f*scrPixelsPerImgPixelH;
		if (LineB.GetScreenVal() < imgext.B)
			LineB.PressedDelta = imgext.B - LineB.ScreenVal;
	}

	if (LineT.Pressed || forceAll)
	{
		if (LineT.GetScreenVal() - 3.0f*scrPixelsPerImgPixelH < LineB.GetScreenVal())
			LineT.PressedDelta = LineB.GetScreenVal() - LineT.ScreenVal + 3.0f*scrPixelsPerImgPixelH;
		if (LineT.GetScreenVal() > imgext.T)
			LineT.PressedDelta = imgext.T - LineT.ScreenVal;
	}
}


void Viewer::CropWidget::DrawMatt(const tVector4& imext, const tVector2& uvoffset)
{
	float l = LineL.GetScreenVal();
	float r = LineR.GetScreenVal();
	float b = LineB.GetScreenVal();
	float t = LineT.GetScreenVal();

	tVector2 scrCropMin(l, b);
	tVector2 scrCropMax(r, t);
	int minX, minY;
	ConvertScreenPosToImagePos(minX, minY, scrCropMin, imext, uvoffset);
	int maxX, maxY;
	ConvertScreenPosToImagePos(maxX, maxY, scrCropMax, imext, uvoffset);

	maxX += 1; maxY += 1;
	tVector2 bl;
	ConvertImagePosToScreenPos(bl, minX, minY, imext, uvoffset);

	tVector2 tr;
	ConvertImagePosToScreenPos(tr, maxX, maxY, imext, uvoffset);	

	glColor4f(ColourClear.x, ColourClear.y, ColourClear.z, 0.75f);

	// Imagine a picture frame with mitre cuts at each corner.
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


void Viewer::CropWidget::DrawLines()
{
	glColor4fv(tColourf::white.E);
	float l = (LineL.GetScreenVal());
	float r = (LineR.GetScreenVal()) - 1.0f;
	float b = (LineB.GetScreenVal());
	float t = (LineT.GetScreenVal()) - 1.0f;
	bool anyPressed = LineL.Pressed || LineR.Pressed || LineB.Pressed || LineT.Pressed;
	glBegin(GL_LINES);

	glColor4fv((!anyPressed && LineB.Hovered) || LineB.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(l,	b);
	glVertex2f(r,	b);

	glColor4fv((!anyPressed && LineR.Hovered) || LineR.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(r,	b);
	glVertex2f(r,	t);

	glColor4fv((!anyPressed && LineT.Hovered) || LineT.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(r,	t);
	glVertex2f(l,	t);

	glColor4fv((!anyPressed && LineL.Hovered) || LineL.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(l,	t);
	glVertex2f(l,	b);

	glColor4fv(tColourf::white.E);
	glEnd();
}


void Viewer::CropWidget::DrawHandles()
{
	float l = (LineL.GetScreenVal());
	float r = (LineR.GetScreenVal()) - 1.0f;
	float b = (LineB.GetScreenVal());
	float t = (LineT.GetScreenVal()) - 1.0f;
	float h = (l+r)/2.0f;
	float v = (b+t)/2.0f;
	bool anyPressed = LineL.Pressed || LineR.Pressed || LineB.Pressed || LineT.Pressed;

	glBegin(GL_QUADS);

	// TL
	glColor4fv( (!anyPressed && LineL.Hovered && LineT.Hovered) || (LineL.Pressed && LineT.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(l-4,		t-4);
	glVertex2f(l+4,		t-4);
	glVertex2f(l+4,		t+4);
	glVertex2f(l-4,		t+4);

	// TM
	glColor4fv( (!anyPressed && LineT.Hovered && !LineL.Hovered && !LineR.Hovered) || (LineT.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(h-4,		t-4);
	glVertex2f(h+4,		t-4);
	glVertex2f(h+4,		t+4);
	glVertex2f(h-4,		t+4);

	// TR
	glColor4fv( (!anyPressed && LineR.Hovered && LineT.Hovered) || (LineR.Pressed && LineT.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(r-4,		t-4);
	glVertex2f(r+4,		t-4);
	glVertex2f(r+4,		t+4);
	glVertex2f(r-4,		t+4);

	// ML
	glColor4fv( (!anyPressed && LineL.Hovered && !LineT.Hovered && !LineB.Hovered) || (LineL.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(l-4,		v-4);
	glVertex2f(l+4,		v-4);
	glVertex2f(l+4,		v+4);
	glVertex2f(l-4,		v+4);

	// MR
	glColor4fv( (!anyPressed && LineR.Hovered && !LineT.Hovered && !LineB.Hovered) || (LineR.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(r-4,		v-4);
	glVertex2f(r+4,		v-4);
	glVertex2f(r+4,		v+4);
	glVertex2f(r-4,		v+4);

	// BL
	glColor4fv( (!anyPressed && LineL.Hovered && LineB.Hovered) || (LineL.Pressed && LineB.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(l-4,		b-4);
	glVertex2f(l+4,		b-4);
	glVertex2f(l+4,		b+4);
	glVertex2f(l-4,		b+4);

	// BM
	glColor4fv( (!anyPressed && LineB.Hovered && !LineL.Hovered && !LineR.Hovered) || (LineB.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(h-4,		b-4);
	glVertex2f(h+4,		b-4);
	glVertex2f(h+4,		b+4);
	glVertex2f(h-4,		b+4);

	// BR
	glColor4fv( (!anyPressed && LineR.Hovered && LineB.Hovered) || (LineR.Pressed && LineB.Pressed) ? CropHovCol.E : CropCol.E );
	glVertex2f(r-4,		b-4);
	glVertex2f(r+4,		b-4);
	glVertex2f(r+4,		b+4);
	glVertex2f(r-4,		b+4);

	glColor4fv(tColourf::white.E);
	glEnd();
}


void Viewer::ShowCropPopup(const tVector4& lrtb, const tVector2& uvoffset)
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

	ImGui::SetNextWindowSize(tVector2(180.0f, 280.0f), ImGuiCond_Appearing);
	ImGui::SetNextWindowBgAlpha(0.70f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Crop", &CropMode, flags))
	{
		tVector2 scrCropMin(CropGizmo.LineL.GetScreenVal(), CropGizmo.LineB.GetScreenVal());
		tVector2 scrCropMax(CropGizmo.LineR.GetScreenVal(), CropGizmo.LineT.GetScreenVal());
		int minX, minY;
		ConvertScreenPosToImagePos(minX, minY, scrCropMin, lrtb, uvoffset);
		int maxX, maxY;
		ConvertScreenPosToImagePos(maxX, maxY, scrCropMax, lrtb, uvoffset);

		int origW = CurrImage->GetWidth();
		int origH = CurrImage->GetHeight();
		int newW = tClampMin(maxX - minX + 1, 1);
		int newH = tClampMin(maxY - minY + 1, 1);

		int margL = minX;
		int margR = origW - maxX - 1;
		int margB = minY;
		int margT = origH - maxY - 1;

		float col = ImGui::GetCursorPosX() + 76.0f;
		ImGui::Text("Bot Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", minX, minY);
		ImGui::Text("Top Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", maxX, maxY);
		ImGui::Text("H Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("L:%d R:%d", margL, margR);
		ImGui::Text("V Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("B:%d T:%d", margB, margT);
		ImGui::Text("Orig Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", origW, origH);
		ImGui::Text("New Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", newW, newH);

		tVector2 ancImageSize(24.0f, 24.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);
		float panNavLeft = 48.0f;
		float panNavSpace = 6.0f;

		// Top Row
		ImGui::SetCursorPosX(panNavLeft);

		ImGui::PushID("TL");
		if (ImGui::ImageButton(ImTextureID(AnchorBLImage.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TL;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("TM");
		if (ImGui::ImageButton(ImTextureID(AnchorBMImage.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("TR");
		if (ImGui::ImageButton(ImTextureID(AnchorBLImage.Bind()), ancImageSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TR;	
		ImGui::PopID();

		// Middle Row
		ImGui::SetCursorPosX(panNavLeft);

		ImGui::PushID("ML");
		if (ImGui::ImageButton(ImTextureID(AnchorMLImage.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::ML;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("MM");
		if (ImGui::ImageButton(ImTextureID(AnchorMMImage.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::MM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("MR");
		if (ImGui::ImageButton(ImTextureID(AnchorMLImage.Bind()), ancImageSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::MR;
		ImGui::PopID();

		// Bottom Row.
		ImGui::SetCursorPosX(panNavLeft);

		ImGui::PushID("BL");
		if (ImGui::ImageButton(ImTextureID(AnchorBLImage.Bind()), ancImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BL;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("BM");
		if (ImGui::ImageButton(ImTextureID(AnchorBMImage.Bind()), ancImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("BR");
		if (ImGui::ImageButton(ImTextureID(AnchorBLImage.Bind()), ancImageSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BR;
		ImGui::PopID();

		// Buttons.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);
		if (ImGui::Button("Cancel", tVector2(50, 0)))
			CropMode = false;

		ImGui::SameLine();
		if (ImGui::Button("Reset", tVector2(50, 0)))
		{
			ResetPan();
			CurrZoomMode = Config::ProfileSettings::ZoomMode::DownscaleOnly;
			CropGizmo.SetLines(0, CurrImage->GetWidth()-1, CurrImage->GetHeight()-1, 0, lrtb, uvoffset);
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 50.0f);

		if (ImGui::Button("Apply", tVector2(50, 0)))
		{
			newW = tClampMin(newW, 4);
			newH = tClampMin(newH, 4);

			CurrImage->Unbind();
			CurrImage->Crop(newW, newH, minX, minY, tColouri::transparent);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
			CropMode = false;
			ResetPan();
		}
	}
	ImGui::End();
}
