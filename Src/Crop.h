// Crop.h
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

#pragma once
#include <Math/tVector2.h>
#include <Math/tVector4.h>
#include "TacentView.h"
namespace Viewer
{


void ShowCropPopup(const tMath::tVector4& lrtb, const tMath::tVector2& uvoffset);


class CropLine
{
public:
	CropLine()					{ }
	float GetScreenVal() const	{ return ScreenVal + PressedDelta; }
	float ScreenVal				= -1.0f;
	int ImageVal				= -1;
	bool Hovered				= false;
	bool Pressed				= false;
	float PressedAnchor			= 0.0f;
	float PressedDelta			= 0.0f;
};


class CropWidget
{
public:
	CropWidget()																										{ }

	void MouseButton(bool down, const tMath::tVector2& mouse);
	void Update(const tMath::tVector4& imgext, const tMath::tVector2& mouse, const tMath::tVector2& uvoffset);

	// Don't call this every frame. Only when you want to reset the crop lines.
	// The LRTB ints are in image-space.
	void SetLines
	(
		int l, int r, int t, int b,
		const tMath::tVector4& imgext, const tMath::tVector2& uvoffset
	);
	void ConstrainLines
	(
		int l, int r, int t, int b,
		const tMath::tVector4& imgext, const tMath::tVector2& uvoffset
	);
	void MoveDirection(CursorMove, const tMath::tVector4& imgext, const tMath::tVector2& uvoffset, int imgW, int imgH);

	CropLine LineL;
	CropLine LineR;
	CropLine LineT;
	CropLine LineB;
	CropLine LineV;		// Middle vertical line.
	CropLine LineH;		// Middle horizontal line.
	Anchor LastSelectedHandle = Anchor::Middle;

private:
	void DrawMatt(const tMath::tVector4& imgext, const tMath::tVector2& uvoffset);
	void DrawLines();
	void DrawHandles();

	void TestSetHovered(CropLine&, const tMath::tVector2& mouse, const tMath::tVector2& ends, bool horizontal);
	bool MouseButton(CropLine&, bool down, float mouse, bool force = false);
	void ConstrainCropLines(const tMath::tVector4& imgext, bool centerPressed);
};


extern CropWidget CropGizmo;


}
