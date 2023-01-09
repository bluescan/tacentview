// Crop.cpp
//
// GL and ImGui code that implements cropping images.
//
// Copyright (c) 2020, 2022 Tristan Grimmer.
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
	tColour CropCol			(0.72f, 0.72f, 0.72f, 1.00f);
	tColour CropSelCol		(0.97f, 0.97f, 0.97f, 1.00f);
	const int CropMin		= 4;
}


bool Viewer::CropWidget::MouseButton(CropLine& line, bool down, float mouse)
{
	// Up or down.
	line.ScreenVal += line.PressedDelta;
	line.PressedDelta = 0.0f;
	bool justPressed = false;

	if (down)
	{
		if (line.Hovered && !line.Pressed)
		{
			line.PressedAnchor = mouse;
			line.Pressed = true;
			justPressed = true;
		}
	}
	else
	{
		line.Pressed = false;
	}

	return justPressed;
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
	bool downB = MouseButton(LineB, down, mouse.y);
	bool downT = MouseButton(LineT, down, mouse.y);
	bool downL = MouseButton(LineL, down, mouse.x);
	bool downR = MouseButton(LineR, down, mouse.x);

	if (downT && downL)
		LastSelectedHandle = Anchor::TopLeft;
	else if (downT && !downL && !downR)
		LastSelectedHandle = Anchor::TopMiddle;
	else if (downT && downR)
		LastSelectedHandle = Anchor::TopRight;

	else if (downL && !downT && !downB)
		LastSelectedHandle = Anchor::MiddleLeft;
	else if (downR && !downT && !downB)
		LastSelectedHandle = Anchor::MiddleRight;

	else if (downB && downL)
		LastSelectedHandle = Anchor::BottomLeft;
	else if (downB && !downL && !downR)
		LastSelectedHandle = Anchor::BottomMiddle;
	else if (downB && downR)
		LastSelectedHandle = Anchor::BottomRight;
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

	ConstrainCropLines(imgext, true);

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


void Viewer::CropWidget::MoveDirection(Viewer::CursorMove moveDir, const tMath::tVector4& imgext, const tMath::tVector2& uvoffset, int imgW, int imgH)
{
	int left = LineL.ImageVal;
	int right = LineR.ImageVal;
	int top = LineT.ImageVal;
	int bottom = LineB.ImageVal;
	Anchor hnd = LastSelectedHandle;
	switch (moveDir)
	{
		case CursorMove_Left:
			if 		(((hnd == Anchor::TL) || (hnd == Anchor::ML) || (hnd == Anchor::BL)) && (left > 0))
				left--;
			else if	(((hnd == Anchor::TR) || (hnd == Anchor::MR) || (hnd == Anchor::BR)) && (right >= left+CropMin))
				right--;
			break;

		case CursorMove_Right:
			if		(((hnd == Anchor::TL) || (hnd == Anchor::ML) || (hnd == Anchor::BL)) && (left <= right-CropMin))
				left++;
			else if	(((hnd == Anchor::TR) || (hnd == Anchor::MR) || (hnd == Anchor::BR)) && (right < imgW-1))
				right++;
			break;

		case CursorMove_Up:
			if		(((hnd == Anchor::TL) || (hnd == Anchor::TM) || (hnd == Anchor::TR)) && (top < imgH-1))
				top++;
			else if	(((hnd == Anchor::BL) || (hnd == Anchor::BM) || (hnd == Anchor::BR)) && (bottom <= top-CropMin))
				bottom++;
			break;

		case CursorMove_Down:
			if		(((hnd == Anchor::TL) || (hnd == Anchor::TM) || (hnd == Anchor::TR)) && (top >= bottom+CropMin))
				top--;
			else if	(((hnd == Anchor::BL) || (hnd == Anchor::BM) || (hnd == Anchor::BR)) && (bottom > 0))
				bottom--;
			break;
	}

	SetLines(left, right, top, bottom, imgext, uvoffset);
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
	if (LineL.ImageVal > l)
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
	if (LineB.ImageVal > b)
	{
		LineB.ImageVal = b; LineB.ScreenVal = min.y; LineB.PressedDelta = 0.0f;
	}
}


void Viewer::CropWidget::ConstrainCropLines(const tVector4& imgext, bool forceAll)
{
	float scrPixelsPerImgPixelW = (imgext.R-imgext.L)/CurrImage->GetWidth();
	float scrPixelsPerImgPixelH = (imgext.T-imgext.B)/CurrImage->GetHeight();

	float cropMin = float(CropMin -1);
	if (LineL.Pressed || forceAll)
	{
		if (LineL.GetScreenVal() + cropMin*scrPixelsPerImgPixelW > LineR.GetScreenVal())
			LineL.PressedDelta = LineR.GetScreenVal() - LineL.ScreenVal - cropMin*scrPixelsPerImgPixelW;
		if (LineL.GetScreenVal() < imgext.L)
			LineL.PressedDelta = imgext.L - LineL.ScreenVal;
	}

	if (LineR.Pressed || forceAll)
	{
		if (LineR.GetScreenVal()- cropMin*scrPixelsPerImgPixelW < LineL.GetScreenVal())
			LineR.PressedDelta = LineL.GetScreenVal() - LineR.ScreenVal + cropMin*scrPixelsPerImgPixelW;
		if (LineR.GetScreenVal() > imgext.R)
			LineR.PressedDelta = imgext.R - LineR.ScreenVal;
	}

	if (LineB.Pressed || forceAll)
	{
		if (LineB.GetScreenVal() + cropMin*scrPixelsPerImgPixelH > LineT.GetScreenVal())
			LineB.PressedDelta = LineT.GetScreenVal() - LineB.ScreenVal - cropMin*scrPixelsPerImgPixelH;
		if (LineB.GetScreenVal() < imgext.B)
			LineB.PressedDelta = imgext.B - LineB.ScreenVal;
	}

	if (LineT.Pressed || forceAll)
	{
		if (LineT.GetScreenVal() - cropMin*scrPixelsPerImgPixelH < LineB.GetScreenVal())
			LineT.PressedDelta = LineB.GetScreenVal() - LineT.ScreenVal + cropMin*scrPixelsPerImgPixelH;
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
	tColour col;
	glBegin(GL_QUADS);

	// TL
	col = (!anyPressed && LineL.Hovered && LineT.Hovered) || (LineL.Pressed && LineT.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::TL) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(l-4,		t-4);
	glVertex2f(l+4,		t-4);
	glVertex2f(l+4,		t+4);
	glVertex2f(l-4,		t+4);

	// TM
	col = (!anyPressed && LineT.Hovered && !LineL.Hovered && !LineR.Hovered) || (LineT.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::TM) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(h-4,		t-4);
	glVertex2f(h+4,		t-4);
	glVertex2f(h+4,		t+4);
	glVertex2f(h-4,		t+4);

	// TR
	col = (!anyPressed && LineR.Hovered && LineT.Hovered) || (LineR.Pressed && LineT.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::TR) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(r-4,		t-4);
	glVertex2f(r+4,		t-4);
	glVertex2f(r+4,		t+4);
	glVertex2f(r-4,		t+4);

	// ML
	col = (!anyPressed && LineL.Hovered && !LineT.Hovered && !LineB.Hovered) || (LineL.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::ML) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(l-4,		v-4);
	glVertex2f(l+4,		v-4);
	glVertex2f(l+4,		v+4);
	glVertex2f(l-4,		v+4);

	// MR
	col = (!anyPressed && LineR.Hovered && !LineT.Hovered && !LineB.Hovered) || (LineR.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::MR) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(r-4,		v-4);
	glVertex2f(r+4,		v-4);
	glVertex2f(r+4,		v+4);
	glVertex2f(r-4,		v+4);

	// BL
	col = (!anyPressed && LineL.Hovered && LineB.Hovered) || (LineL.Pressed && LineB.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::BL) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(l-4,		b-4);
	glVertex2f(l+4,		b-4);
	glVertex2f(l+4,		b+4);
	glVertex2f(l-4,		b+4);

	// BM
	col = (!anyPressed && LineB.Hovered && !LineL.Hovered && !LineR.Hovered) || (LineB.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::BM) ? CropSelCol : CropCol);
	glColor4fv(col.E);
	glVertex2f(h-4,		b-4);
	glVertex2f(h+4,		b-4);
	glVertex2f(h+4,		b+4);
	glVertex2f(h-4,		b+4);

	// BR
	col = (!anyPressed && LineR.Hovered && LineB.Hovered) || (LineR.Pressed && LineB.Pressed) ? CropHovCol :
	((LastSelectedHandle == Anchor::BR) ? CropSelCol : CropCol);
	glColor4fv(col.E);
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

	ImGui::SetNextWindowBgAlpha(0.70f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Crop", &CropMode, flags))
	{
		float buttonWidth, panNavLeft, anchorSize;
		switch (Config::Current->GetUISize())
		{
			default:
			case Viewer::Config::ProfileSettings::UISizeEnum::Small:
				buttonWidth	= 55.0f;
				panNavLeft	= 58.0f;
				anchorSize	= 24.0f;
				break;
			case Viewer::Config::ProfileSettings::UISizeEnum::Medium:
				buttonWidth	= 61.0f;
				panNavLeft	= 64.0f;
				anchorSize	= 26.0f;
				break;
			case Viewer::Config::ProfileSettings::UISizeEnum::Large:
				buttonWidth	= 66.0f;
				panNavLeft	= 68.0f;
				anchorSize	= 28.0f;
				break;
		}

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

		float col = ImGui::GetCursorPosX() + 86.0f;
		ImGui::Text("Bot Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", minX, minY);
		ImGui::Text("Top Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", maxX, maxY);
		ImGui::Text("H Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("L:%d R:%d", margL, margR);
		ImGui::Text("V Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("B:%d T:%d", margB, margT);
		ImGui::Text("Orig Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", origW, origH);
		ImGui::Text("New Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", newW, newH);
		ImGui::SameLine();

		tString leftKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::PixelLeft);
		tString leftDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelLeft);
		tString rghtKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::PixelRight);
		tString rghtDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelRight);
		tString upKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::PixelUp);
		tString upDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelUp);
		tString downKey = Config::Current->InputBindings.FindModKeyText(Bindings::Operation::PixelDown);
		tString downDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelDown);

		tString toolTipText;
		tsPrintf
		(
			toolTipText,
			"The crop lines may be moved using the mouse. The last\n"
			"selected anchor may also be moved one pixel at a time:\n"
			"%s: %s\n"
			"%s: %s\n"
			"%s: %s\n"
			"%s: %s",
			leftDsc.Chr(), leftKey.Chr(),
			rghtDsc.Chr(), rghtKey.Chr(),
			upDsc.Chr(), upKey.Chr(),
			downDsc.Chr(), downKey.Chr()
		);
		ShowHelpMark(toolTipText.Chr());

		tVector2 ancImageSize(anchorSize, anchorSize);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);
		float panNavSpace = 6.0f;

		// Top Row
		ImGui::SetCursorPosX(panNavLeft);

		ImGui::PushID("TL");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TL;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("TM");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("TR");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), ancImageSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TR;	
		ImGui::PopID();

		// Middle Row
		ImGui::SetCursorPosX(panNavLeft);

		ImGui::PushID("ML");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::ML;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("MM");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorMM.Bind()), ancImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::MM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("MR");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), ancImageSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::MR;
		ImGui::PopID();

		// Bottom Row.
		ImGui::SetCursorPosX(panNavLeft);

		ImGui::PushID("BL");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), ancImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BL;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("BM");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), ancImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - panNavSpace);
		ImGui::PushID("BR");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), ancImageSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BR;
		ImGui::PopID();

		if (Request_PanSnap != Anchor::Invalid)
			CropGizmo.LastSelectedHandle = Request_PanSnap;

		// Buttons.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);
		if (ImGui::Button("Cancel", tVector2(buttonWidth, 0.0f)))
			CropMode = false;

		ImGui::SameLine();
		if (ImGui::Button("Reset", tVector2(buttonWidth, 0.0f)))
		{
			// For now we only reset the crop lines, not the zoom or pan. Pan-reset can be done by the anchor widget.
			// ResetPan();
			// CurrZoomMode = Config::ProfileSettings::ZoomMode::Fit;
			CropGizmo.SetLines(0, CurrImage->GetWidth()-1, CurrImage->GetHeight()-1, 0, lrtb, uvoffset);
		}

		ImGui::SameLine();
		if (ImGui::Button("Apply", tVector2(buttonWidth, 0.0f)))
		{
			newW = tClampMin(newW, CropMin);
			newH = tClampMin(newH, CropMin);

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
