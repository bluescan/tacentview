// CommandOps.h
//
// Command line operations for batch image processing and conversions. Ooperations such as quantization,
// rescaling/filtering, cropping, rotation, extracting frames, creating contact-sheets, amalgamating images into
// animated formats, and levels adjustments are specified/implemented here.
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

#pragma once
#include <Image/tPicture.h>
#include "Image.h"
namespace Command
{


enum class OpType
{
	Invalid,
	Resize,			// Resize image width/height by resampling.
	Canvas,			// Modify width height of image by adjusting canvas size and specifying an anchor.
	Aspect,			// Change aspect ratio of image without stretching.
	DeBorder,		// Detect and remove borders from image.
	Crop,			// Similar to Canvas but only can make image smaller and more control over crop area.
	Flip,			// Horizontal or vertical flips.
	Rotate,			// Rotate an arbitrary number of degrees about a center point.
	Levels,			// Adjust image levels. Blackoint, mids, whitepoint etc.
	Contrast,		// Adjust contrast.
	Brightness,		// Adjust brightness.
	Quantize,		// Quantize (reduce) the number of colours used in an image.
	Multalpha		// Remove the alpha channel by multiplying RGB against A and adding (1-A) * a specific colour.
};


struct Operation : public tLink<Operation>
{
	virtual bool Apply(Viewer::Image&)					= 0;
	OpType Op											= OpType::Invalid;
};


struct OperationResize : public Operation
{
	OperationResize(const tString& args);
	int Width											= 0;
	int Height											= 0;
	tImage::tResampleFilter ResampleFilter				= tImage::tResampleFilter::Bilinear;		// Optional.
	tImage::tResampleEdgeMode EdgeMode					= tImage::tResampleEdgeMode::Clamp;			// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationCanvas : public Operation
{
	OperationCanvas(const tString& args);
	int Width											= 0;
	int Height											= 0;
	tImage::tPicture::Anchor Anchor						= tImage::tPicture::Anchor::MiddleMiddle;	// Optional.
	tColour4i FillColour								= tColour4i::black;							// Optional.
	int AnchorX											= -1;										// Optional.
	int AnchorY											= -1;										// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationAspect : public Operation
{
	OperationAspect(const tString& args);
	int Num												= 16;
	int Den												= 9;
	enum class AspectMode { Crop, Letterbox };
	AspectMode Mode										= AspectMode::Crop;
	tImage::tPicture::Anchor Anchor						= tImage::tPicture::Anchor::MiddleMiddle;	// Optional.
	tColour4i FillColour								= tColour4i::black;							// Optional.
	int AnchorX											= -1;										// Optional.
	int AnchorY											= -1;										// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationDeborder : public Operation
{
	OperationDeborder(const tString& args);
	// Colour.

	bool Apply(Viewer::Image&) override;
};


struct OperationRotate : public Operation
{
	OperationRotate(const tString& args);
	float Angle;
	bool Apply(Viewer::Image&) override;
};


}
