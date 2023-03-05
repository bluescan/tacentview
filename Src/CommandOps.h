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
#include <Image/tQuantize.h>
#include "Image.h"
namespace Command
{


struct Operation : public tLink<Operation>
{
	virtual bool Apply(Viewer::Image&)					= 0;
	virtual ~Operation()								{ }
	bool Valid											= false;
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
	bool UseTestColour									= false;
	tColour4i TestColour								= tColour4i::black;							// Optional.
	tcomps Channels										= tComp_RGBA;								// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationCrop : public Operation
{
	OperationCrop(const tString& args);
	enum class CropMode { Absolute, Relative };
	CropMode Mode										= CropMode::Absolute;						// Optional.
	int OriginX											= 0;
	int OriginY											= 0;
	int WidthOrMaxX										= 4;
	int HeightOrMaxY									= 4;
	tColour4i FillColour								= tColour4i::transparent;					// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationFlip : public Operation
{
	OperationFlip(const tString& args);
	enum class FlipMode { Horizontal, Vertical };
	FlipMode Mode										= FlipMode::Horizontal;						// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationRotate : public Operation
{
	OperationRotate(const tString& args);

	float Angle											= 0.0f;		// Radians.
	enum class ExactMode { Off, Zero, ACW90, CW90, R180 };
	ExactMode Exact										= ExactMode::Zero;

	enum class RotateMode { Fill, Crop, Resize };
	RotateMode Mode										= RotateMode::Crop;							// Optional.

	// UpFilter		DownFilter		Description
	// None			NA				No up/down scaling. Preserves colours. Nearest Neighbour. Fast. Good for pixel art.
	// Valid		Valid			Up/down scaling. Smooth. Good results with up=bilinear, down=box.
	// Valid		None			Up/down scaling. Use alternate (sharper) downscaling scheme (possibe pad + 2X ScaleHalf).
	tImage::tResampleFilter FilterUp					= tImage::tResampleFilter::Bilinear;		// Optional.
	tImage::tResampleFilter FilterDown					= tImage::tResampleFilter::None;			// Optional.
	tColour4i FillColour								= tColour4i::black;							// Optional.

	bool Apply(Viewer::Image&) override;
};


struct OperationLevels : public Operation
{
	OperationLevels(const tString& args);
	float BlackPoint									= 0.0f;
	float MidPoint										= -1.0f;	// -1 = Auto. Halfway between black and white.
	float WhitePoint									= 1.0f;
	float OutBlackPoint									= 0.0f;
	float OutWhitePoint									= 1.0f;
	int FrameNumber										= -1;		// -1 = All Frames.
	Viewer::Image::AdjChan Channels						= Viewer::Image::AdjChan::RGB;
	bool PowerMidGamma									= true;

	bool Apply(Viewer::Image&) override;
};


struct OperationContrast : public Operation
{
	OperationContrast(const tString& args);
	float Contrast										= 0.5f;
	int FrameNumber										= -1;		// -1 = All Frames.
	Viewer::Image::AdjChan Channels						= Viewer::Image::AdjChan::RGB;

	bool Apply(Viewer::Image&) override;
};


struct OperationBrightness : public Operation
{
	OperationBrightness(const tString& args);
	float Brightness									= 0.5f;
	int FrameNumber										= -1;		// -1 = All Frames.
	Viewer::Image::AdjChan Channels						= Viewer::Image::AdjChan::RGB;

	bool Apply(Viewer::Image&) override;
};


struct OperationQuantize : public Operation
{
	OperationQuantize(const tString& args);
	tImage::tQuantize::Method Method					= tImage::tQuantize::Method::Fixed;		// Required.
	int NumColours										= 256;							// Required.
	bool CheckExact										= true;							// Optional.
	int SampFilt										= 0;							// Optional. 0 is invalid.
	double Dither										= 0.0;							// Optional, 0.0 is auto.

	bool Apply(Viewer::Image&) override;
};


struct OperationChannel : public Operation
{
	OperationChannel(const tString& args);

	enum class ChanMode
	{
		// In: chans. colour. Set specified channels to corresponding channel in colour. RGB* default.
		Set,

		// In: chans. colour. Blends specified RGB channels with A and background Colour. Additionally sets alpha to
		// Colour.A once done if A-channel was specified. If it was not specified, alpha channel is left untouched,
		Blend,

		// In: chans.         Spreads specified single channel (R*) to RGB channels.
		Spread,

		// In: chans.         Computes RGB intensity and sets specified channels to that value. Default is RGB.
		Intensity
	};
	ChanMode Mode										= ChanMode::Blend;				// Optional.
	tcomps Channels										= tComp_RGBA;					// Optional.
	tColour4i Colour									= tColour4i::black;				// Optional.

	bool Apply(Viewer::Image&) override;
};


}
