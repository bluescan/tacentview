// Crop.cpp
//
// GL and ImGui code that implements cropping images.
//
// Copyright (c) 2020-2023 Tristan Grimmer.
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

	tColour CropHovCol		(0.25f, 0.48f, 1.00f, 1.00f);
	tColour CropCol			(0.24f, 0.24f, 0.24f, 1.00f);
	tColour CropSelCol		(0.98f, 0.98f, 0.98f, 1.00f);
	const int CropMin		= 4;
}


bool Viewer::CropWidget::MouseButton(CropLine& line, bool down, float mouse, bool force)
{
	// Up or down.
	line.ScreenVal += line.PressedDelta;
	line.PressedDelta = 0.0f;
	bool justPressed = false;

	if (down)
	{
		if ((!force && (line.Hovered && !line.Pressed)) || (force && !line.Pressed))
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


void Viewer::CropWidget::TestSetHovered(CropLine& line, const tVector2& mouse, const tVector2& ends, bool horizontal)
{
	tARect2 hitBox;
	if (horizontal)
		hitBox.Set(tVector2(ends.x, line.GetScreenVal()), tVector2(ends.y, line.GetScreenVal()));
	else
		hitBox.Set(tVector2(line.GetScreenVal(), ends.x), tVector2(line.GetScreenVal(), ends.y));
	hitBox.Expand(6);
	line.Hovered = hitBox.IsPointInside(mouse);
}


void Viewer::CropWidget::MouseButton(bool down, const tVector2& mouse)
{
	bool downB = MouseButton(LineB, down, mouse.y);
	bool downT = MouseButton(LineT, down, mouse.y);
	bool downL = MouseButton(LineL, down, mouse.x);
	bool downR = MouseButton(LineR, down, mouse.x);
	bool downV = MouseButton(LineV, down, mouse.x);
	bool downH = MouseButton(LineH, down, mouse.y);

	if (downV && downH)
	{
		MouseButton(LineB, down, mouse.y, true);
		MouseButton(LineT, down, mouse.y, true);
		MouseButton(LineL, down, mouse.x, true);
		MouseButton(LineR, down, mouse.x, true);
		MouseButton(LineV, down, mouse.x, true);
		MouseButton(LineH, down, mouse.y, true);
	}

	if (downV && downH)
		LastSelectedHandle = Anchor::Middle;
	else if (downT && downL)
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

	TestSetHovered(LineB, mouse, tVector2(l, r), true);
	TestSetHovered(LineT, mouse, tVector2(l, r), true);
	TestSetHovered(LineL, mouse, tVector2(b, t), false);
	TestSetHovered(LineR, mouse, tVector2(b, t), false);
	TestSetHovered(LineV, mouse, tVector2(b, t), false);
	TestSetHovered(LineH, mouse, tVector2(l, r), true);

	Config::ProfileData& profile = Config::GetProfileData();
	bool centerPressed = LineB.Pressed && LineT.Pressed && LineL.Pressed && LineR.Pressed && LineV.Pressed && LineH.Pressed;
	bool aspectLocked = (profile.GetCropAspectRatio() != tImage::tAspectRatio::Free);

	if (centerPressed && aspectLocked)
	{
		float width = LineR.GetScreenVal() - LineL.GetScreenVal();
		if (LineL.GetScreenVal() < imgext.L)
		{
			LineL.PressedDelta = imgext.L - LineL.ScreenVal;
			LineR.PressedDelta = (imgext.L + width) - LineR.ScreenVal;
		}
		if (LineR.GetScreenVal() > imgext.R)
		{
			LineR.PressedDelta = imgext.R - LineR.ScreenVal;
			LineL.PressedDelta = (imgext.R - width) - LineL.ScreenVal;
		}

		float height = LineT.GetScreenVal() - LineB.GetScreenVal();
		if (LineB.GetScreenVal() < imgext.B)
		{
			LineB.PressedDelta = imgext.B - LineB.ScreenVal;
			LineT.PressedDelta = (imgext.B + height) - LineT.ScreenVal;
		}
		if (LineT.GetScreenVal() > imgext.T)
		{
			LineT.PressedDelta = imgext.T - LineT.ScreenVal;
			LineB.PressedDelta = (imgext.T - height) - LineB.ScreenVal;
		}
	}
	else if (aspectLocked)
	{
		float aspect = profile.GetCropAspectRatioFloat();

		// Top Left.
		if (LineT.Pressed && LineL.Pressed)
		{
			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = nheight * aspect;
			LineL.PressedDelta = (LineR.GetScreenVal() - reqwidth) - LineL.ScreenVal;

			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = nwidth / aspect;
			LineT.PressedDelta = (LineB.GetScreenVal() + reqheight) - LineT.ScreenVal;
		}

		// Top Right.
		else if (LineT.Pressed && LineR.Pressed)
		{
			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = nheight * aspect;
			LineR.PressedDelta = (LineL.GetScreenVal() + reqwidth) - LineR.ScreenVal;

			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = nwidth / aspect;
			LineT.PressedDelta = (LineB.GetScreenVal() + reqheight) - LineT.ScreenVal;
		}

		// Top Middle.
		else if (LineT.Pressed && (mouse.y < imgext.T-1.0f))
		{
			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float xcent = (LineL.GetScreenVal() + LineR.GetScreenVal()) / 2.0f;
			float reqwidth = nheight * aspect;
			LineL.PressedDelta = (xcent - reqwidth*0.5f) - LineL.ScreenVal;
			LineR.PressedDelta = (xcent + reqwidth*0.5f) - LineR.ScreenVal;

			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = nwidth / aspect;
			LineT.PressedDelta = (LineB.GetScreenVal() + reqheight) - LineT.ScreenVal;
		}

		// Bottom Right.
		else if (LineB.Pressed && LineR.Pressed)
		{
			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = nheight * aspect;
			LineR.PressedDelta = (LineL.GetScreenVal() + reqwidth) - LineR.ScreenVal;

			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = nwidth / aspect;
			LineB.PressedDelta = (LineT.GetScreenVal() - reqheight) - LineB.ScreenVal;
		}

		// Right Middle.
		else if (LineR.Pressed)
		{
			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float ycent = (LineB.GetScreenVal() + LineT.GetScreenVal()) / 2.0f;
			float reqheight = nwidth / aspect;
			LineT.PressedDelta = (ycent + reqheight*0.5f) - LineT.ScreenVal;
			LineB.PressedDelta = (ycent - reqheight*0.5f) - LineB.ScreenVal;

			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = nheight * aspect;
			LineR.PressedDelta = (LineL.GetScreenVal() + reqwidth) - LineR.ScreenVal;
		}

		// Bottom left.
		else if (LineB.Pressed && LineL.Pressed)
		{
			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = nheight * aspect;
			LineL.PressedDelta = (LineR.GetScreenVal() - reqwidth) - LineL.ScreenVal;

			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = nwidth / aspect;
			LineB.PressedDelta = (LineT.GetScreenVal() - reqheight) - LineB.ScreenVal;
		}

		// Bottom middle.
		else if (LineB.Pressed)
		{
			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float xcent = (LineL.GetScreenVal() + LineR.GetScreenVal()) / 2.0f;
			float reqwidth = nheight * aspect;
			LineL.PressedDelta = (xcent - reqwidth*0.5f) - LineL.ScreenVal;
			LineR.PressedDelta = (xcent + reqwidth*0.5f) - LineR.ScreenVal;

			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = nwidth / aspect;
			LineB.PressedDelta = (LineT.GetScreenVal() - reqheight) - LineB.ScreenVal;
		}

		// Left middle.
		else if (LineL.Pressed)
		{
			float nwidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float ycent = (LineB.GetScreenVal() + LineT.GetScreenVal()) / 2.0f;
			float reqheight = nwidth / aspect;
			LineT.PressedDelta = (ycent + reqheight*0.5f) - LineT.ScreenVal;
			LineB.PressedDelta = (ycent - reqheight*0.5f) - LineB.ScreenVal;

			float nheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = nheight * aspect;
			LineL.PressedDelta = (LineR.GetScreenVal() - reqwidth * 1.0f) - LineL.ScreenVal;
		}
	}

	ConstrainCropLines(imgext, centerPressed);

	if (LineL.Pressed || LineR.Pressed || LineT.Pressed || LineB.Pressed)
	{
		tVector2 scrPosMin(LineL.GetScreenVal(), LineB.GetScreenVal());
		ConvertScreenPosToImagePos(LineL.ImageVal, LineB.ImageVal, scrPosMin, imgext, uvoffset);
		tVector2 scrPosMax(LineR.GetScreenVal(), LineT.GetScreenVal());
		ConvertScreenPosToImagePos(LineR.ImageVal, LineT.ImageVal, scrPosMax, imgext, uvoffset);

		LineV.ImageVal = (LineL.ImageVal+LineR.ImageVal)/2;
		LineH.ImageVal = (LineB.ImageVal+LineT.ImageVal)/2;
		LineV.ScreenVal = (scrPosMin.x+scrPosMax.x)/2.0f;
		LineH.ScreenVal = (scrPosMin.y+scrPosMax.y)/2.0f;
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
	Config::ProfileData& profile = Config::GetProfileData();

	bool aspectLocked = (profile.GetCropAspectRatio() != tImage::tAspectRatio::Free);

	if (aspectLocked)
	{
		switch (moveDir)
		{
			case CursorMove_Left:
				if (left > 0)					{ left--; right--; }
				break;

			case CursorMove_Right:
				if (right < imgW-1)				{ right++; left++; }
				break;

			case CursorMove_Up:
				if (top < imgH-1)				{ top++; bottom++; }
				break;

			case CursorMove_Down:
				if (bottom > 0)					{ bottom--; top--; }
				break;
		}
	}

	else if (hnd == Anchor::MM)
	{
		switch (moveDir)
		{
			case CursorMove_Left:
				if (left > 0)					left--;
				if (right >= left+CropMin)		right--;
				break;

			case CursorMove_Right:
				if (right < imgW-1)				right++;
				if (left <= right-CropMin)		left++;
				break;

			case CursorMove_Up:
				if (top < imgH-1)				top++;
				if (bottom <= top-CropMin)		bottom++;
				break;

			case CursorMove_Down:
				if (bottom > 0)					bottom--;
				if (top >= bottom+CropMin)		top--;
				break;
		}
	}

	else	// Not MM anchor.
	{
		switch (moveDir)
		{
			case CursorMove_Left:
				if (((hnd == Anchor::TL) || (hnd == Anchor::ML) || (hnd == Anchor::BL)) && (left > 0))
					left--;
				if (((hnd == Anchor::TR) || (hnd == Anchor::MR) || (hnd == Anchor::BR)) && (right >= left+CropMin))
					right--;
				break;

			case CursorMove_Right:
				if (((hnd == Anchor::TL) || (hnd == Anchor::ML) || (hnd == Anchor::BL)) && (left <= right-CropMin))
					left++;
				if (((hnd == Anchor::TR) || (hnd == Anchor::MR) || (hnd == Anchor::BR)) && (right < imgW-1))
					right++;
				break;

			case CursorMove_Up:
				if (((hnd == Anchor::TL) || (hnd == Anchor::TM) || (hnd == Anchor::TR)) && (top < imgH-1))
					top++;
				if (((hnd == Anchor::BL) || (hnd == Anchor::BM) || (hnd == Anchor::BR)) && (bottom <= top-CropMin))
					bottom++;
				break;

			case CursorMove_Down:
				if (((hnd == Anchor::TL) || (hnd == Anchor::TM) || (hnd == Anchor::TR)) && (top >= bottom+CropMin))
					top--;
				if (((hnd == Anchor::BL) || (hnd == Anchor::BM) || (hnd == Anchor::BR)) && (bottom > 0))
					bottom--;
				break;
		}
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
	LineV.ImageVal = (left+right)/2;
	LineH.ImageVal = (bottom+top)/2;

	LineL.ScreenVal = min.x;	LineL.PressedDelta = 0.0f;
	LineR.ScreenVal = max.x;	LineR.PressedDelta = 0.0f;
	LineT.ScreenVal = max.y;	LineT.PressedDelta = 0.0f;
	LineB.ScreenVal = min.y;	LineB.PressedDelta = 0.0f;
	LineV.ScreenVal = (min.x+max.x)/2.0f;	LineV.PressedDelta = 0.0f;
	LineH.ScreenVal = (min.y+max.y)/2.0f;	LineH.PressedDelta = 0.0f;
}


void Viewer::CropWidget::ConstrainLines(int l, int r, int t, int b, const tMath::tVector4& imgext, const tMath::tVector2& uvoffset)
{
	// This function gets called when the image changes.
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

	// If we are in aspect-locked mode we need to 
	Config::ProfileData& profile = Config::GetProfileData();
	bool aspectLocked = (profile.GetCropAspectRatio() != tImage::tAspectRatio::Free);
	if (aspectLocked)
	{
		float cropAspect = profile.GetCropAspectRatioFloat();
		float width = float(r-l);
		float height = float(t-b);
		float imgAspect = width/height;

		if (cropAspect > imgAspect)
		{
			// Width goes to extents.
			float cropHeight = width / cropAspect;
			t = int(height/2.0f + cropHeight/2.0f);
			b = int(height/2.0f - cropHeight/2.0f);
		}
		else
		{
			// Height goes to extents.
			float cropWidth = height * cropAspect;
			r = int(width/2.0f + cropWidth/2.0f);
			l = int(width/2.0f - cropWidth/2.0f);
		}

		ConvertImagePosToScreenPos(min, l, b, imgext, uvoffset, true);
		ConvertImagePosToScreenPos(max, r, t, imgext, uvoffset, true);
		LineL.ImageVal = l; LineL.ScreenVal = min.x; LineL.PressedDelta = 0.0f;
		LineR.ImageVal = r; LineR.ScreenVal = max.x; LineR.PressedDelta = 0.0f;
		LineT.ImageVal = t; LineT.ScreenVal = max.y; LineT.PressedDelta = 0.0f;
		LineB.ImageVal = b; LineB.ScreenVal = min.y; LineB.PressedDelta = 0.0f;
	}
}


void Viewer::CropWidget::ConstrainCropLines(const tVector4& imgext, bool centerPressed)
{
	bool constrainImg = !centerPressed;

	float scrPixelsPerImgPixelW = (imgext.R-imgext.L)/CurrImage->GetWidth();
	float scrPixelsPerImgPixelH = (imgext.T-imgext.B)/CurrImage->GetHeight();
	float cropMin = float(CropMin -1);

	// Left.
	if (LineL.GetScreenVal() + cropMin*scrPixelsPerImgPixelW > LineR.GetScreenVal())
		LineL.PressedDelta = LineR.GetScreenVal() - LineL.ScreenVal - cropMin*scrPixelsPerImgPixelW;
	if (constrainImg && (LineL.GetScreenVal() < imgext.L))
		LineL.PressedDelta = imgext.L - LineL.ScreenVal;

	// Right.
	if (LineR.GetScreenVal() - cropMin*scrPixelsPerImgPixelW < LineL.GetScreenVal())
		LineR.PressedDelta = LineL.GetScreenVal() - LineR.ScreenVal + cropMin*scrPixelsPerImgPixelW;
	if (constrainImg && (LineR.GetScreenVal() > imgext.R))
		LineR.PressedDelta = imgext.R - LineR.ScreenVal;

	// Bottom.
	if (LineB.GetScreenVal() + cropMin*scrPixelsPerImgPixelH > LineT.GetScreenVal())
		LineB.PressedDelta = LineT.GetScreenVal() - LineB.ScreenVal - cropMin*scrPixelsPerImgPixelH;
	if (constrainImg && (LineB.GetScreenVal() < imgext.B))
		LineB.PressedDelta = imgext.B - LineB.ScreenVal;

	// Top.
	if (LineT.GetScreenVal() - cropMin*scrPixelsPerImgPixelH < LineB.GetScreenVal())
		LineT.PressedDelta = LineB.GetScreenVal() - LineT.ScreenVal + cropMin*scrPixelsPerImgPixelH;
	if (constrainImg && (LineT.GetScreenVal() > imgext.T))
		LineT.PressedDelta = imgext.T - LineT.ScreenVal;

	// Aspect constrain.
	Config::ProfileData& profile = Config::GetProfileData();
	if (profile.GetCropAspectRatio() != tImage::tAspectRatio::Free)
	{
		float aspect = profile.GetCropAspectRatioFloat();
		tAssert(aspect > 0.0f);

		// Top Left.
		if (LineT.Pressed && LineL.Pressed)
		{
			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			if (LineL.GetScreenVal() < LineR.GetScreenVal()-reqwidth)
				LineL.PressedDelta = (LineR.GetScreenVal()-reqwidth) - LineL.ScreenVal;
			if (LineR.GetScreenVal() > LineL.GetScreenVal()+reqwidth)
				LineR.PressedDelta = (LineL.GetScreenVal()+reqwidth) - LineR.ScreenVal;

			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			if (LineT.GetScreenVal() > LineB.GetScreenVal()+reqheight)
				LineT.PressedDelta = (LineB.GetScreenVal()+reqheight) - LineT.ScreenVal;
		}

		// Top Right.
		else if (LineT.Pressed && LineR.Pressed)
		{
			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			if (LineR.GetScreenVal() > LineL.GetScreenVal()+reqwidth)
				LineR.PressedDelta = (LineL.GetScreenVal()+reqwidth) - LineR.ScreenVal;
			if (LineL.GetScreenVal() < LineR.GetScreenVal()-reqwidth)
				LineL.PressedDelta = (LineR.GetScreenVal()-reqwidth) - LineL.ScreenVal;

			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			if (LineT.GetScreenVal() > LineB.GetScreenVal()+reqheight)
				LineT.PressedDelta = (LineB.GetScreenVal()+reqheight) - LineT.ScreenVal;
		}

		// Bottom Left.
		else if (LineB.Pressed && LineL.Pressed)
		{
			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			if (LineL.GetScreenVal() < LineR.GetScreenVal()-reqwidth)
				LineL.PressedDelta = (LineR.GetScreenVal()-reqwidth) - LineL.ScreenVal;
			if (LineR.GetScreenVal() > LineL.GetScreenVal()+reqwidth)
				LineR.PressedDelta = (LineL.GetScreenVal()+reqwidth) - LineR.ScreenVal;

			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			if (LineB.GetScreenVal() < LineT.GetScreenVal()-reqheight)
				LineB.PressedDelta = (LineT.GetScreenVal()-reqheight) - LineB.ScreenVal;
		}

		// Bottom Right.
		else if (LineB.Pressed && LineR.Pressed)
		{
			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			if (LineR.GetScreenVal() > LineL.GetScreenVal()+reqwidth)
				LineR.PressedDelta = (LineL.GetScreenVal()+reqwidth) - LineR.ScreenVal;
			if (LineL.GetScreenVal() < LineR.GetScreenVal()-reqwidth)
				LineL.PressedDelta = (LineR.GetScreenVal()-reqwidth) - LineL.ScreenVal;

			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			if (LineB.GetScreenVal() < LineT.GetScreenVal()-reqheight)
				LineB.PressedDelta = (LineT.GetScreenVal()-reqheight) - LineB.ScreenVal;
		}

		// Left Middle.
		else if (LineL.Pressed && !LineT.Pressed && !LineB.Pressed)
		{
			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			float ycent = (LineB.GetScreenVal() + LineT.GetScreenVal()) / 2.0f;
			if (LineB.GetScreenVal() < ycent - reqheight*0.5f)
				LineB.PressedDelta = (ycent - reqheight*0.5f) - LineB.ScreenVal;
			if (LineT.GetScreenVal() > ycent + reqheight*0.5f)
				LineT.PressedDelta = (ycent + reqheight*0.5f) - LineT.ScreenVal;

			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			if (LineL.GetScreenVal() < LineR.GetScreenVal()-reqwidth)
				LineL.PressedDelta = (LineR.GetScreenVal()-reqwidth) - LineL.ScreenVal;
		}

		// Right Middle.
		else if (LineR.Pressed && !LineT.Pressed && !LineB.Pressed)
		{
			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			float ycent = (LineB.GetScreenVal() + LineT.GetScreenVal()) / 2.0f;
			if (LineB.GetScreenVal() < ycent - reqheight*0.5f)
				LineB.PressedDelta = (ycent - reqheight*0.5f) - LineB.ScreenVal;
			if (LineT.GetScreenVal() > ycent + reqheight*0.5f)
				LineT.PressedDelta = (ycent + reqheight*0.5f) - LineT.ScreenVal;

			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			if (LineR.GetScreenVal() > LineL.GetScreenVal()+reqwidth)
				LineR.PressedDelta = (LineL.GetScreenVal()+reqwidth) - LineR.ScreenVal;
		}

		// Bottom Middle.
		else if (LineB.Pressed && !LineL.Pressed && !LineR.Pressed)
		{
			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			float xcent = (LineL.GetScreenVal() + LineR.GetScreenVal()) / 2.0f;
			if (LineR.GetScreenVal() > xcent + reqwidth*0.5f)
				LineR.PressedDelta = (xcent + reqwidth*0.5f) - LineR.ScreenVal;
			if (LineL.GetScreenVal() < xcent - reqwidth*0.5f)
				LineL.PressedDelta = (xcent - reqwidth*0.5f) - LineL.ScreenVal;

			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			if (LineB.GetScreenVal() < LineT.GetScreenVal()-reqheight)
				LineB.PressedDelta = (LineT.GetScreenVal()-reqheight) - LineB.ScreenVal;
		}

		// Top Middle.
		else if (LineT.Pressed && !LineL.Pressed && !LineR.Pressed)
		{
			float sheight = LineT.GetScreenVal() - LineB.GetScreenVal();
			float reqwidth = sheight * aspect;
			float xcent = (LineL.GetScreenVal() + LineR.GetScreenVal()) / 2.0f;
			if (LineR.GetScreenVal() > xcent + reqwidth*0.5f)
				LineR.PressedDelta = (xcent + reqwidth*0.5f) - LineR.ScreenVal;
			if (LineL.GetScreenVal() < xcent - reqwidth*0.5f)
				LineL.PressedDelta = (xcent - reqwidth*0.5f) - LineL.ScreenVal;

			float swidth = LineR.GetScreenVal() - LineL.GetScreenVal();
			float reqheight = swidth / aspect;
			if (LineT.GetScreenVal() > LineB.GetScreenVal()+reqheight)
				LineT.PressedDelta = (LineB.GetScreenVal()+reqheight) - LineT.ScreenVal;
		}
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
	float v = (LineV.GetScreenVal());
	float h = (LineH.GetScreenVal());
	bool anyPressed = LineL.Pressed || LineR.Pressed || LineB.Pressed || LineT.Pressed || LineV.Pressed || LineH.Pressed;
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

	glColor4fv((!anyPressed && LineV.Hovered) || LineV.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(v,	t);
	glVertex2f(v,	b);

	glColor4fv((!anyPressed && LineH.Hovered) || LineH.Pressed ? CropHovCol.E : CropCol.E);
	glVertex2f(l,	h);
	glVertex2f(r,	h);

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
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::TL)														col = CropSelCol;
	else if	((!anyPressed && LineL.Hovered && LineT.Hovered) || (LineL.Pressed && LineT.Pressed))	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(l-4,		t-4);
	glVertex2f(l+4,		t-4);
	glVertex2f(l+4,		t+4);
	glVertex2f(l-4,		t+4);

	// TM
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::TM)														col = CropSelCol;
	else if ((!anyPressed && LineT.Hovered && !LineL.Hovered && !LineR.Hovered) || LineT.Pressed)	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(h-4,		t-4);
	glVertex2f(h+4,		t-4);
	glVertex2f(h+4,		t+4);
	glVertex2f(h-4,		t+4);

	// TR
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::TR)														col = CropSelCol;
	else if	((!anyPressed && LineR.Hovered && LineT.Hovered) || (LineR.Pressed && LineT.Pressed))	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(r-4,		t-4);
	glVertex2f(r+4,		t-4);
	glVertex2f(r+4,		t+4);
	glVertex2f(r-4,		t+4);

	// ML
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::ML)														col = CropSelCol;
	else if	((!anyPressed && LineL.Hovered && !LineT.Hovered && !LineB.Hovered) || LineL.Pressed)	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(l-4,		v-4);
	glVertex2f(l+4,		v-4);
	glVertex2f(l+4,		v+4);
	glVertex2f(l-4,		v+4);

	// MR
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::MR)														col = CropSelCol;
	else if	((!anyPressed && LineR.Hovered && !LineT.Hovered && !LineB.Hovered) || LineR.Pressed)	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(r-4,		v-4);
	glVertex2f(r+4,		v-4);
	glVertex2f(r+4,		v+4);
	glVertex2f(r-4,		v+4);

	// BL
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::BL)														col = CropSelCol;
	else if ((!anyPressed && LineL.Hovered && LineB.Hovered) || (LineL.Pressed && LineB.Pressed))	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(l-4,		b-4);
	glVertex2f(l+4,		b-4);
	glVertex2f(l+4,		b+4);
	glVertex2f(l-4,		b+4);

	// BM
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::BM)														col = CropSelCol;
	else if ((!anyPressed && LineB.Hovered && !LineL.Hovered && !LineR.Hovered) || LineB.Pressed)	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(h-4,		b-4);
	glVertex2f(h+4,		b-4);
	glVertex2f(h+4,		b+4);
	glVertex2f(h-4,		b+4);

	// BR
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::BR)														col = CropSelCol;
	else if	((!anyPressed && LineR.Hovered && LineB.Hovered) || (LineR.Pressed && LineB.Pressed))	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(r-4,		b-4);
	glVertex2f(r+4,		b-4);
	glVertex2f(r+4,		b+4);
	glVertex2f(r-4,		b+4);

	// MM
	col		= CropCol;
	if		(LastSelectedHandle == Anchor::MM)														col = CropSelCol;
	else if	((!anyPressed && LineV.Hovered && LineH.Hovered) || (LineV.Pressed && LineH.Pressed))	col = CropHovCol;
	glColor4fv(col.E);
	glVertex2f(h-4,		v-4);
	glVertex2f(h+4,		v-4);
	glVertex2f(h+4,		v+4);
	glVertex2f(h-4,		v+4);

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

	float windowWidth = Viewer::GetUIParamScaled(198.0f, 2.5f);
	float windowVOffset = Viewer::GetUIParamScaled(48.0f, 2.5f);
	tVector2 windowPos = tVector2((lrtb.R+lrtb.L)/2.0f - windowWidth/2.0f, windowVOffset);
	tVector2 windowSize(windowWidth, -1.0f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
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
		float buttonWidth		= Viewer::GetUIParamScaled(55.0f, 2.5f);
		float shortcutTxtLeft	= Viewer::GetUIParamScaled(57.0f, 2.5f);
		float anchorSize		= Viewer::GetUIParamScaled(24.0f, 2.5f);
		float comboWidth		= Viewer::GetUIParamScaled(102.0f, 2.5f);
		float aspectWidth		= Viewer::GetUIParamScaled(24.0f, 2.5f);

		//
		// Crop info display.
		//
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

		float textDataColumnOffset = Viewer::GetUIParamScaled(86.0f, 2.5f);
		float col = ImGui::GetCursorPosX() + textDataColumnOffset;
		ImGui::Text("Bot Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", minX, minY);
		ImGui::Text("Top Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("X:%d Y:%d", maxX, maxY);
		ImGui::Text("H Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("L:%d R:%d", margL, margR);
		ImGui::Text("V Margins");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("B:%d T:%d", margB, margT);
		ImGui::Text("Orig Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", origW, origH);
		ImGui::Text("New Size");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("W:%d H:%d", newW, newH);

		//
		// Crop aspect ratio chooser.
		//
		Config::ProfileData& profile = Config::GetProfileData();
		ImGui::PushItemWidth(comboWidth);
		if (ImGui::Combo
		(
			"Aspect",
			&profile.CropAspectRatio,
			tImage::tAspectRatioNames,
			int(tImage::tAspectRatio::NumRatios) + 1,
			int(tImage::tAspectRatio::NumRatios) / 2
		))
		{
			Viewer::Request_CropLineConstrain = true;
		}
		ImGui::PopItemWidth();

		if (profile.GetCropAspectRatio() == tImage::tAspectRatio::User)
		{
			float inputWidth = Viewer::GetUIParamScaled(26.0f, 2.5f);
			ImGui::PushItemWidth(inputWidth);
			if (ImGui::InputInt("##Num", &profile.CropAspectUserNum, 0, 0))
				Viewer::Request_CropLineConstrain = true;
			ImGui::SameLine(); ImGui::Text(":"); ImGui::SameLine();
			if (ImGui::InputInt("##Den", &profile.CropAspectUserDen, 0, 0))
				Viewer::Request_CropLineConstrain = true;
			ImGui::PopItemWidth();
			tiClamp(profile.CropAspectUserNum, 1, 99); tiClamp(profile.CropAspectUserDen, 1, 99);
		}
		else
		{
			ImGui::SameLine();
			ShowHelpMark("Aspect ratio for crop area.\nFree means aspect is unlocked.\nUser means enter the aspect ratio manually.\nFor the print presets the L means Landscape.");
		}

		//
		// Shortcuts.
		//
		ImGui::SetCursorPosX(shortcutTxtLeft);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.0f);
		ImGui::Text("View Shortcuts");

		ImGui::SameLine();
		tString leftKey = profile.InputBindings.FindModKeyText(Bindings::Operation::PixelLeft);
		tString leftDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelLeft);
		tString rghtKey = profile.InputBindings.FindModKeyText(Bindings::Operation::PixelRight);
		tString rghtDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelRight);
		tString upKey = profile.InputBindings.FindModKeyText(Bindings::Operation::PixelUp);
		tString upDsc = Bindings::GetOperationDesc(Bindings::Operation::PixelUp);
		tString downKey = profile.InputBindings.FindModKeyText(Bindings::Operation::PixelDown);
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

		tVector2 shortcutImageSize(anchorSize, anchorSize);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2::zero);
		float navSpacing	= Viewer::GetUIParamScaled(2.0f, 2.5f);
		float navWidth		= 3.0f*(anchorSize+2.0f) + 2.0f*navSpacing;		// The anchorSize+2 is because when we draw the image buttons we have a frame padding of 1 on either side.
		float topNavMargin	= Viewer::GetUIParamScaled(3.0f, 2.5f);

		// Top Row
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topNavMargin);
		ImGui::SetCursorPosX(windowWidth/2.0f - navWidth/2.0f);

		ImGui::PushID("TL");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), shortcutImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TL;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + navSpacing);
		ImGui::PushID("TM");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), shortcutImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + navSpacing);
		ImGui::PushID("TR");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), shortcutImageSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::TR;	
		ImGui::PopID();

		// Middle Row
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + navSpacing);
		ImGui::SetCursorPosX(windowWidth/2.0f - navWidth/2.0f);

		ImGui::PushID("ML");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), shortcutImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::ML;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + navSpacing);
		ImGui::PushID("MM");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorMM.Bind()), shortcutImageSize, tVector2(0.0f, 0.0f), tVector2(1.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::MM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + navSpacing);
		ImGui::PushID("MR");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorML.Bind()), shortcutImageSize, tVector2(1.0f, 0.0f), tVector2(0.0f, 1.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::MR;
		ImGui::PopID();

		// Bottom Row.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + navSpacing);
		ImGui::SetCursorPosX(windowWidth/2.0f - navWidth/2.0f);

		ImGui::PushID("BL");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), shortcutImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BL;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + navSpacing);
		ImGui::PushID("BM");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBM.Bind()), shortcutImageSize, tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BM;
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + navSpacing);
		ImGui::PushID("BR");
		if (ImGui::ImageButton(ImTextureID(Image_AnchorBL.Bind()), shortcutImageSize, tVector2(1.0f, 1.0f), tVector2(0.0f, 0.0f), 1, ColourBG, ColourEnabledTint))
			Request_PanSnap = Anchor::BR;
		ImGui::PopID();

		ImGui::PopStyleVar();

		if (Request_PanSnap != Anchor::Invalid)
			CropGizmo.LastSelectedHandle = Request_PanSnap;

		//
		// Buttons.
		//
		float botButtonMargin = Viewer::GetUIParamScaled(7.0f, 2.5f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + botButtonMargin);
		if (ImGui::Button("Cancel", tVector2(buttonWidth, 0.0f)))
			CropMode = false;

		ImGui::SameLine();
		if (ImGui::Button("Reset", tVector2(buttonWidth, 0.0f)))
		{
			// For now we only reset the crop lines, not the zoom or pan. Pan-reset can be done by the anchor widget.
			// ResetPan();
			// CurrZoomMode = Config::ProfileData::ZoomMode::Fit;
			CropGizmo.SetLines(0, CurrImage->GetWidth()-1, CurrImage->GetHeight()-1, 0, lrtb, uvoffset);
			Viewer::Request_CropLineConstrain = true;
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
