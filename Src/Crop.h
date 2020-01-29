// Crop.h
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

#pragma once
#include<Math/tVector2.h>
#include<Math/tVector4.h>
namespace TexView
{


class CropLine
{
public:
	CropLine()				{ }
	float Get() const		{ return V + PressedDelta; }
	float V					= -1.0f;
	bool Hovered			= false;
	bool Pressed			= false;
	float PressedAnchor		= 0.0f;
	float PressedDelta		= 0.0f;
};


class CropWidget
{
public:
	CropWidget() { }

	void SetLines(const tMath::tVector4& linesLRTB);								// Don't call this every frame. Only once for each image.
	void MouseButton(bool down, const tMath::tVector2& mouse);
	void UpdateDraw(const tMath::tVector4& imgext, const tMath::tVector2& mouse);

	CropLine LineL;
	CropLine LineR;
	CropLine LineT;
	CropLine LineB;

private:
	void DrawMatt(const tMath::tVector4& imgext);
	void DrawLines();
	void DrawHandles();

	void MouseHovered(CropLine& line, const tMath::tVector2& mouse, const tMath::tVector2& ends, bool horizontal);
	void MouseButton(CropLine&, bool down, float mouse);

	void ConstrainCropLines(const tMath::tVector4& imgext, bool forceAll = true);
};


}
