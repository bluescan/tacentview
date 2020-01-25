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

//#include <System/tFile.h>
//#include <System/tScript.h>
#include <Math/tColour.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Crop.h"
#include "TacitTexView.h"
//#include "TacitImage.h"
using namespace tMath;


void TexView::CropWidget::SetLines(const tMath::tVector4& lines)
{
	LineL.V = lines.L;
	LineR.V = lines.R;
	LineT.V = lines.T;
	LineB.V = lines.B;
}


void TexView::CropWidget::UpdateDraw(const tMath::tVector4& imgext, const tMath::tVector2& mouse)
{
	ConstrainCropLines(imgext);
	DrawMatt(imgext);
	DrawLines();
	DrawHandles();
}


void TexView::CropWidget::ConstrainCropLines(const tVector4& imgext)
{
	tiClamp(LineL.V, imgext.L, imgext.R);
	tiClamp(LineR.V, imgext.L, imgext.R);
	tiClamp(LineT.V, imgext.B, imgext.T);
	tiClamp(LineB.V, imgext.B, imgext.T);
	
	tiClampMax(LineL.V, LineR.V);
	tiClampMax(LineB.V, LineT.V);
}


void TexView::CropWidget::DrawMatt(const tVector4& imext)
{
	glColor4f(ColourClear.x, ColourClear.y, ColourClear.z, 0.75f);
	glBegin(GL_QUAD_STRIP);
	glVertex2f(imext.L, imext.B);
	glVertex2f(LineL.V, LineB.V);
	glVertex2f(imext.R, imext.B);
	glVertex2f(LineR.V, LineB.V);
	glVertex2f(imext.R, imext.T);
	glVertex2f(LineR.V, LineT.V);
	glVertex2f(imext.L, imext.T);
	glVertex2f(LineL.V, LineT.V);
	glVertex2f(imext.L, imext.B);
	glVertex2f(LineL.V, LineB.V);
	glEnd();
}


void TexView::CropWidget::DrawLines()
{
	glColor4f(tColourf::white.R, tColourf::white.G, tColourf::white.B, 1.00f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(LineL.V,		LineB.V-1);
	glVertex2f(LineR.V+1,	LineB.V-1);
	glVertex2f(LineR.V+1,	LineT.V);
	glVertex2f(LineL.V,		LineT.V);
	glVertex2f(LineL.V,		LineB.V-1);
	glEnd();
}


void TexView::CropWidget::DrawHandles()
{
	glBegin(GL_QUADS);

	// Each of the 4 crop lines need to be a seperate class.
	glVertex2f(LineL.V-7,		LineB.V-7);
	glVertex2f(LineL.V+0,		LineB.V-7);
	glVertex2f(LineL.V+0,		LineB.V+0);
	glVertex2f(LineL.V-7,		LineB.V+0);

	// glColor4f(tColourf::blue.R, tColourf::blue.G, tColourf::blue.B, 1.00f);
	glVertex2f(LineR.V-0,		LineB.V-7);
	glVertex2f(LineR.V+7,		LineB.V-7);
	glVertex2f(LineR.V+7,		LineB.V+0);
	glVertex2f(LineR.V-0,		LineB.V+0);

	glVertex2f(LineR.V-0,		LineT.V-0);
	glVertex2f(LineR.V+7,		LineT.V-0);
	glVertex2f(LineR.V+7,		LineT.V+7);
	glVertex2f(LineR.V-0,		LineT.V+7);

	glVertex2f(LineL.V-7,		LineT.V+7);
	glVertex2f(LineL.V+0,		LineT.V+7);
	glVertex2f(LineL.V+0,		LineT.V+0);
	glVertex2f(LineL.V-7,		LineT.V+0);

	glEnd();
}
