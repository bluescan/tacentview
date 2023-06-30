// CommandOps.cpp
//
// Command line operations for batch image processing and conversions. Operations such as rescaling/filtering,
// quantization, cropping, rotation, extracting frames, and levels adjustments are specified/implemented here.
// Some operations are applied to whole sets of images rather than on one image at a time. These are called
// post-operations and implement things like creating contact-sheets or amalgamating multiple images into a single
// animated image.
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

#include <System/tTime.h>
#include <Image/tImageGIF.h>
#include <Image/tImageWEBP.h>
#include <Image/tImageAPNG.h>
#include <Image/tImageTIFF.h>
#include "CommandOps.h"
#include "Command.h"
#include "MultiFrame.h"
#include "OpenSaveDialogs.h"
#include "TacentView.h"


namespace Command
{
	// Parses strCol as a colour string. The string may be a hexadecimal in the form #RRGGBBAA, a single integer spread
	// to RGBA, or a predefined name: black, white, grey, red, green, blue, yellow, cyan, magenta, or trans
	// (transparent black). If the string is not of this form, colour is left unmodified. You may set colour as your
	// default colour before calling this. Returns true if colour was modified.
	bool ParseColour(tColour4i& colour, const tString& strCol);

	// Parses chanStr as a set of channels. The string may contain the characters RGBA in any order and in upper or
	// lower case. If none of these characters are set, channels is left unmodified and false is returned.
	bool ParseChannels(comp_t& channels, const tString& chanStr);
}


bool Command::ParseColour(tColour4i& colour, const tString& colStr)
{
	if (colStr[0] == '#')
	{
		uint32 hex = colStr.AsUInt32(16);
		colour.Set( uint8((hex >> 24) & 0xFF), uint8((hex >> 16) & 0xFF), uint8((hex >> 8) & 0xFF), uint8((hex >> 0) & 0xFF) );
		return true;
	}

	if (colStr.IsNumeric())
	{
		// We also allow entering a single integer that sets the RGBA components.
		int val = colStr.AsInt();
		tMath::tiClamp(val, 0, 255);
		colour.Set(val, val, val, val);
		return true;
	}

	switch (tHash::tHashString(colStr.Chr()))
	{
		case tHash::tHashCT("black"):	colour = tColour4i::black;			return true;
		case tHash::tHashCT("white"):	colour = tColour4i::white;			return true;
		case tHash::tHashCT("grey"):	colour = tColour4i::grey;			return true;
		case tHash::tHashCT("red"):		colour = tColour4i::red;			return true;
		case tHash::tHashCT("green"):	colour = tColour4i::green;			return true;
		case tHash::tHashCT("blue"):	colour = tColour4i::blue;			return true;
		case tHash::tHashCT("yellow"):	colour = tColour4i::yellow;			return true;
		case tHash::tHashCT("cyan"):	colour = tColour4i::cyan;			return true;
		case tHash::tHashCT("magenta"):	colour = tColour4i::magenta;		return true;
		case tHash::tHashCT("trans"):	colour = tColour4i::transparent;	return true;
	}

	return false;
}


bool Command::ParseChannels(comp_t& channels, const tString& chanStr)
{
	comp_t chans = 0;
	if (chanStr.FindAny("rR") != -1)	chans |= tCompBit_R;
	if (chanStr.FindAny("gG") != -1)	chans |= tCompBit_G;
	if (chanStr.FindAny("bB") != -1)	chans |= tCompBit_B;
	if (chanStr.FindAny("aA") != -1)	chans |= tCompBit_A;

	if (chans)
	{
		channels = chans;
		return true;
	}
	
	return false;
}


Command::OperationPixel::OperationPixel(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 3)
	{
		tPrintfNorm("Operation Pixel Invalid. At least 3 arguments required.\n");
		return;
	}

	// The AsInt calls return 0 if * is entered.
	tStringItem* currArg = args.First();
	X = currArg->AsInt32();

	currArg = currArg->Next();
	Y = currArg->AsInt32();

	currArg = currArg->Next();
	tString colStr = *currArg;
	ParseColour(PixelColour, colStr);

	// Channels to set.
	if (numArgs >= 4)
	{
		currArg = currArg->Next();
		tString chanStr = *currArg;
		ParseChannels(Channels, chanStr);
		tAssert(Channels);
	}

	Valid = true;
}


bool Command::OperationPixel::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	int w = image.GetWidth();
	int h = image.GetHeight();
	if ((w <= 0) || (h <= 0))
		return false;

	int x = tMath::tMod(X, w);
	int y = tMath::tMod(Y, h);

	tColour4i setCol = PixelColour;
	tColour4i pixCol = image.GetPixel(x, y);
	if (Channels & tCompBit_R) pixCol.R = setCol.R;
	if (Channels & tCompBit_G) pixCol.G = setCol.G;
	if (Channels & tCompBit_B) pixCol.B = setCol.B;
	if (Channels & tCompBit_A) pixCol.A = setCol.A;

	tPrintfFull("Pixel | SetPixelColour[X:%d Y:%d Col:%02x,%02x,%02x,%02x]\n", x, y, pixCol.R, pixCol.G, pixCol.B, pixCol.A);
	image.SetPixelColour(x, y, pixCol, false, false);
	return true;
}


Command::OperationResize::OperationResize(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 2)
	{
		tPrintfNorm("Operation Resize Invalid. At least 2 arguments required.\n");
		return;
	}

	// The AsInt calls return 0 if * is entered.
	tStringItem* currArg = args.First();
	Width = currArg->AsInt32();

	currArg = currArg->Next();
	Height = currArg->AsInt32();

	// Either width or height needs to be specified. If only one is present it uses aspect preserve.
	if ((Width <= 0) && (Height <= 0))
	{
		tPrintfNorm("Operation Resize Invalid. Width or Height or both must be specified.\n");
		return;
	}

	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		ResampleFilter = tImage::tResampleFilter::Bilinear;
		for (int f = 0; f < int(tImage::tResampleFilter::NumFilters); f++)
		{
			if (currArg->IsEqualCI(tImage::tResampleFilterNamesSimple[f]))
			{
				ResampleFilter = tImage::tResampleFilter(f);
				break;
			}
		}
	}

	if (numArgs >= 4)
	{
		currArg = currArg->Next();
		EdgeMode = tImage::tResampleEdgeMode::Clamp;
		for (int e = 0; e < int(tImage::tResampleEdgeMode::NumEdgeModes); e++)
		{
			if (currArg->IsEqualCI(tImage::tResampleEdgeModeNamesSimple[e]))
			{
				EdgeMode = tImage::tResampleEdgeMode(e);
				break;
			}
		}
	}

	Valid = true;
}


bool Command::OperationResize::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	int srcW = image.GetWidth();
	int srcH = image.GetHeight();
	if ((srcW <= 0) || (srcH <= 0))
		return false;

	float aspect = float(srcW) / float(srcH);

	int dstW = Width;
	int dstH = Height;
	if (dstW <= 0)
		dstW = int( float(dstH) * aspect );
	else if (dstH <= 0)
		dstH = int( float(dstW) / aspect );

	tMath::tiClamp(dstW, 4, 65536);
	tMath::tiClamp(dstH, 4, 65536);

	if ((srcW == dstW) && (srcH == dstH))
	{
		tPrintfFull("Resize not applied. Image already has correct dimensions.\n");
		return true;
	}

	tPrintfFull("Resize | Resample[Dim:%dx%d Filter:%s EdgeMode:%s]\n", dstW, dstH, tImage::tResampleFilterNamesSimple[int(ResampleFilter)], tImage::tResampleEdgeModeNamesSimple[int(EdgeMode)]);
	image.Resample(dstW, dstH, ResampleFilter, EdgeMode);
	return true;
}


Command::OperationCanvas::OperationCanvas(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 2)
	{
		tPrintfNorm("Operation Canvas Invalid. At least 2 arguments required.\n");
		return;
	}

	// The AsInt calls return 0 if * is entered.
	tStringItem* currArg = args.First();
	Width = currArg->AsInt32();

	currArg = currArg->Next();
	Height = currArg->AsInt32();

	// Either width or height needs to be specified. If only one is present it uses aspect preserve.
	if ((Width <= 0) && (Height <= 0))
	{
		tPrintfNorm("Operation Canvas Invalid. Width or Height or both must be specified.\n");
		return;
	}

	// Anchor.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		Anchor = tImage::tPicture::Anchor::MiddleMiddle;
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("tl"):	Anchor = tImage::tPicture::Anchor::LeftTop;			break;
			case tHash::tHashCT("tm"):	Anchor = tImage::tPicture::Anchor::MiddleTop;		break;
			case tHash::tHashCT("tr"):	Anchor = tImage::tPicture::Anchor::RightTop;		break;
			case tHash::tHashCT("ml"):	Anchor = tImage::tPicture::Anchor::LeftMiddle;		break;
			case tHash::tHashCT("mm"):	Anchor = tImage::tPicture::Anchor::MiddleMiddle;	break;
			case tHash::tHashCT("mr"):	Anchor = tImage::tPicture::Anchor::RightMiddle;		break;
			case tHash::tHashCT("bl"):	Anchor = tImage::tPicture::Anchor::LeftBottom;		break;
			case tHash::tHashCT("bm"):	Anchor = tImage::tPicture::Anchor::MiddleBottom;	break;
			case tHash::tHashCT("br"):	Anchor = tImage::tPicture::Anchor::RightBottom;		break;
		}
	}

	// Fill colour.
	if (numArgs >= 4)
	{
		currArg = currArg->Next();
		tString colStr = *currArg;
		ParseColour(FillColour, colStr);
	}

	// Anchor X.
	if (numArgs >= 5)
	{
		currArg = currArg->Next();
		tString xstr = *currArg;
		if (!xstr.IsEmpty() && (xstr[0] != '*'))
			AnchorX = xstr.AsInt32();
	}

	// Anchor Y.
	if (numArgs >= 6)
	{
		currArg = currArg->Next();
		tString ystr = *currArg;
		if (!ystr.IsEmpty() && (ystr[0] != '*'))
			AnchorY = ystr.AsInt32();
	}

	Valid = true;
}


bool Command::OperationCanvas::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	int srcW = image.GetWidth();
	int srcH = image.GetHeight();
	if ((srcW <= 0) || (srcH <= 0))
		return false;

	float aspect = float(srcW) / float(srcH);

	int dstW = Width;
	int dstH = Height;
	if (dstW <= 0)
		dstW = int( float(dstH) * aspect );
	else if (dstH <= 0)
		dstH = int( float(dstW) / aspect );

	tMath::tiClamp(dstW, 4, 65536);
	tMath::tiClamp(dstH, 4, 65536);

	if ((srcW == dstW) && (srcH == dstH))
	{
		tPrintfFull("Canvas not applied. Image has same dimensions.\n");
		return true;
	}

	// Use the specified anchor pos if it was specified.
	if ((AnchorX >= 0) && (AnchorY >= 0))
	{
		// Honestly can't quite remember what this does. I believe it has something to do with
		// the coordinate system of the crop position. In any case, it works in the GUI properly
		// so we need to keep it.
		int originX = (AnchorX * (srcW - dstW)) / srcW;
		int originY = (AnchorY * (srcH - dstH)) / srcH;

		tPrintfFull("Canvas | Crop[Dim:%dx%d Origin:%d,%d Fill:%02x,%02x,%02x,%02x]\n", dstW, dstH, originX, originY, FillColour.R, FillColour.G, FillColour.B, FillColour.A);
		image.Crop(dstW, dstH, originX, originY, FillColour);
	}
	else
	{
		tPrintfFull("Canvas | Crop[Dim:%dx%d Anchor:%d Fill:%02x,%02x,%02x,%02x]\n", dstW, dstH, int(Anchor), FillColour.R, FillColour.G, FillColour.B, FillColour.A);
		image.Crop(dstW, dstH, Anchor, FillColour);
	}

	return true;
}


Command::OperationAspect::OperationAspect(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 2)
	{
		tPrintfNorm("Operation Aspect Invalid. At least 2 arguments required.\n");
		return;
	}

	tStringItem* currArg = args.First();
	tList<tStringItem> aspectArgs;
	int numAspectArgs = tStd::tExplode(aspectArgs, *currArg, ':');
	if (numAspectArgs == 2)
	{
		// The AsInt calls return 0 if * is entered.
		Num = aspectArgs.First()->AsInt32();	if (Num<=0) Num = 16;
		Den = aspectArgs.Last()->AsInt32();		if (Den<=0) Den = 9;
	}

	currArg = currArg->Next();
	switch (tHash::tHashString(currArg->Chr()))
	{
		case tHash::tHashCT("crop"):		Mode = AspectMode::Crop;		break;
		case tHash::tHashCT("letter"):		Mode = AspectMode::Letterbox;	break;
		// Mode is already at default if not found.
	}

	// Anchor.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		Anchor = tImage::tPicture::Anchor::MiddleMiddle;
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("tl"):	Anchor = tImage::tPicture::Anchor::LeftTop;			break;
			case tHash::tHashCT("tm"):	Anchor = tImage::tPicture::Anchor::MiddleTop;		break;
			case tHash::tHashCT("tr"):	Anchor = tImage::tPicture::Anchor::RightTop;		break;
			case tHash::tHashCT("ml"):	Anchor = tImage::tPicture::Anchor::LeftMiddle;		break;
			case tHash::tHashCT("mm"):	Anchor = tImage::tPicture::Anchor::MiddleMiddle;	break;
			case tHash::tHashCT("mr"):	Anchor = tImage::tPicture::Anchor::RightMiddle;		break;
			case tHash::tHashCT("bl"):	Anchor = tImage::tPicture::Anchor::LeftBottom;		break;
			case tHash::tHashCT("bm"):	Anchor = tImage::tPicture::Anchor::MiddleBottom;	break;
			case tHash::tHashCT("br"):	Anchor = tImage::tPicture::Anchor::RightBottom;		break;
		}
	}

	// Fill colour.
	if (numArgs >= 4)
	{
		currArg = currArg->Next();
		tString colStr = *currArg;
		ParseColour(FillColour, colStr);
	}

	// Anchor X.
	if (numArgs >= 5)
	{
		currArg = currArg->Next();
		tString xstr = *currArg;
		if (!xstr.IsEmpty() && (xstr[0] != '*'))
			AnchorX = xstr.AsInt32();
	}

	// Anchor Y.
	if (numArgs >= 6)
	{
		currArg = currArg->Next();
		tString ystr = *currArg;
		if (!ystr.IsEmpty() && (ystr[0] != '*'))
			AnchorY = ystr.AsInt32();
	}

	Valid = true;
}


bool Command::OperationAspect::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	int srcW = image.GetWidth();
	int srcH = image.GetHeight();
	if ((srcW <= 0) || (srcH <= 0))
		return false;

	int dstH = srcH;
	int dstW = srcW;
	float srcAspect = float(srcW)/float(srcH);
	float dstAspect = float(Num)/float(Den);
	switch (Mode)
	{
		case AspectMode::Crop:
			if (dstAspect > srcAspect)
				dstH = tMath::tFloatToInt(float(dstW) / dstAspect);
			else if (dstAspect < srcAspect)
				dstW = tMath::tFloatToInt(float(dstH) * dstAspect);
			break;
		
		case AspectMode::Letterbox:
			if (dstAspect > srcAspect)
				dstW = tMath::tFloatToInt(float(dstH) * dstAspect);
			else if (dstAspect < srcAspect)
				dstH = tMath::tFloatToInt(float(dstW) / dstAspect);
	}

	if ((srcW == dstW) && (srcH == dstH))
	{
		tPrintfFull("Aspect not applied. Image has same dimensions.\n");
		return true;
	}

	// Use the specified anchor pos if it was specified.
	if ((AnchorX >= 0) && (AnchorY >= 0))
	{
		// Honestly can't quite remember what this does. I believe it has something to do with
		// the coordinate system of the crop position. In any case, it works in the GUI properly
		// so we need to keep it.
		int originX = (AnchorX * (srcW - dstW)) / srcW;
		int originY = (AnchorY * (srcH - dstH)) / srcH;

		tPrintfFull("Aspect | Crop[Dim:%dx%d Origin:%d,%d Fill:%02x,%02x,%02x,%02x]\n", dstW, dstH, originX, originY, FillColour.R, FillColour.G, FillColour.B, FillColour.A);
		image.Crop(dstW, dstH, originX, originY, FillColour);
	}
	else
	{
		tPrintfFull("Aspect | Crop[Dim:%dx%d Anchor:%d Fill:%02x,%02x,%02x,%02x]\n", dstW, dstH, int(Anchor), FillColour.R, FillColour.G, FillColour.B, FillColour.A);
		image.Crop(dstW, dstH, Anchor, FillColour);
	}

	return true;
}


Command::OperationDeborder::OperationDeborder(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	tStringItem* currArg = nullptr;

	// Test colour.
	if (numArgs >= 1)
	{
		currArg = args.First();
		tString colStr = *currArg;
		UseTestColour = ParseColour(TestColour, colStr);
	}

	// Channels to test.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();
		tString chanStr = *currArg;
		ParseChannels(Channels, chanStr);
		tAssert(Channels);
	}

	Valid = true;
}


bool Command::OperationDeborder::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	tColour4i testCol = TestColour;

	// Do we need to retrieve test colour from the origin (bottom-left 0,0) of the image?
	if (!UseTestColour)
		testCol = image.GetPixel(0, 0);

	tPrintfFull("Deborder | Crop[Col:%02x,%02x,%02x,%02x Channels:%08x]\n", testCol.R, testCol.G, testCol.B, testCol.A, Channels);
	image.Crop(testCol, Channels);
	return true;
}


Command::OperationCrop::OperationCrop(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 5)
	{
		tPrintfNorm("Operation Crop Invalid. At least 5 arguments required.\n");
		return;
	}

	tStringItem* currArg = args.First();
	switch (tHash::tHashString(currArg->Chr()))
	{
		case tHash::tHashCT("abs"):		Mode = CropMode::Absolute;		break;
		case tHash::tHashCT("rel"):		Mode = CropMode::Relative;		break;
		// Mode is already at default if not found.
	}

	// The AsInt calls return 0 if * is entered. This is the acceptable default for origin.
	currArg = currArg->Next();
	OriginX = currArg->AsInt32();

	currArg = currArg->Next();
	OriginY = currArg->AsInt32();

	currArg = currArg->Next();
	WidthOrMaxX = currArg->AsInt32();
	if ((WidthOrMaxX <= 0) && (Mode == CropMode::Absolute))
		WidthOrMaxX = 3;
	if ((WidthOrMaxX <= 0) && (Mode == CropMode::Relative))
		WidthOrMaxX = 4;

	currArg = currArg->Next();
	HeightOrMaxY = currArg->AsInt32();
	if ((HeightOrMaxY <= 0) && (Mode == CropMode::Absolute))
		HeightOrMaxY = 3;
	if ((HeightOrMaxY <= 0) && (Mode == CropMode::Relative))
		HeightOrMaxY = 4;

	if ((OriginX < 0) || (OriginY < 0))
	{
		tPrintfNorm("Operation Crop Invalid. OriginX and OriginY must be >= 0.\n");
		return;
	}

	switch (Mode)
	{
		case CropMode::Absolute:
			if (((WidthOrMaxX+1 - OriginX) < 4) || ((HeightOrMaxY+1 - OriginY) < 4))
			{
				tPrintfNorm("Operation Crop Invalid. MaxX and MaxY must be at least 3 bigger than origin.\n");
				return;
			}
			break;

		case CropMode::Relative:
			if ((WidthOrMaxX < 4) || (HeightOrMaxY < 4))
			{
				tPrintfNorm("Operation Crop Invalid. Width and Height must be >= 4\n");
				return;
			}
			break;
	}

	// Fill colour.
	if (numArgs >= 6)
	{
		currArg = currArg->Next();
		tString colStr = *currArg;
		ParseColour(FillColour, colStr);
	}

	Valid = true;
}


bool Command::OperationCrop::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	int newW = WidthOrMaxX;
	int newH = HeightOrMaxY;
	if (Mode == CropMode::Absolute)
	{
		newW = WidthOrMaxX+1 - OriginX;
		newH = HeightOrMaxY+1 - OriginY;
	}
	tPrintfFull("Crop | Crop[w:%d h:%d x:%d y:%d fill:%02x,%02x,%02x,%02x]\n", newW, newH, OriginX, OriginY, FillColour.R, FillColour.G, FillColour.B, FillColour.A);
	image.Crop(newW, newH, OriginX, OriginY, FillColour);

	return true;
}


Command::OperationFlip::OperationFlip(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');

	// Flip mode.
	if (numArgs >= 1)
	{
		tStringItem* currArg = args.First();
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("h"):
			case tHash::tHashCT("H"):
			case tHash::tHashCT("horizontal"):
			case tHash::tHashCT("Horizontal"):
				Mode = FlipMode::Horizontal;
				break;

			case tHash::tHashCT("v"):
			case tHash::tHashCT("V"):
			case tHash::tHashCT("vertical"):
			case tHash::tHashCT("Vertical"):
				Mode = FlipMode::Vertical;
				break;
		}
	}
	
	Valid = true;
}


bool Command::OperationFlip::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	bool horizontal = (Mode == FlipMode::Horizontal);
	tPrintfFull("Flip | Flip[horizontal:%B\n", horizontal);
	image.Flip(horizontal);

	return true;
}


Command::OperationRotate::OperationRotate(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 1)
	{
		tPrintfNorm("Operation Rotate Invalid. At least 1 argument required.\n");
		return;
	}

	// Get angle. The argument is assumed to be in degrees. If an 'r' is present it uses radians. There are some
	// special strings for exact mode where we call a faster (possibly more accurate) rotate call. The 'exact'
	// strings are:
	// "90" "90.0" "acw" "ccw" : For an exact 90 degree anti-clockwise rotation.
	// "-90" "-90.0" "cw'      : For an exact 90 degree clockwise rotaion.
	// "180" "180.0"           : For an exact 180 degree rotation. Clockness doesn't matter.
	// "*"                     : Default. Results in an exact Zero rotation (does nothing).
	tStringItem* currArg = args.First();
	tString angleStr = *currArg;

	// Look for the special 'exact' strings first. No other argument is needed if we are in
	// an exact mode -- other than Off, which is the result if no special exact string found.
	switch (tHash::tHashString(angleStr.Chr()))
	{
		case tHash::tHashCT("90"):
		case tHash::tHashCT("90.0"):
		case tHash::tHashCT("acw"):
		case tHash::tHashCT("ccw"):
			Exact = ExactMode::ACW90;	Valid = true;	return;

		case tHash::tHashCT("-90"):
		case tHash::tHashCT("-90.0"):
		case tHash::tHashCT("cw"):
			Exact = ExactMode::CW90;	Valid = true;	return;

		case tHash::tHashCT("180"):
		case tHash::tHashCT("180.0"):
			Exact = ExactMode::R180;	Valid = true;	return;

		case tHash::tHashCT("*"):
			Exact = ExactMode::Zero;	Valid = true;	return;
	}
	Exact = ExactMode::Off;

	bool degrees = true;
	if (angleStr.FindChar('r') != -1)
	{
		angleStr.Remove('r');
		degrees = false;
	}
	
	// If the angle converted to a float is 0.0, we also go into exact-zero mode.
	float angle = angleStr.AsFloat();
	if (angle == 0.0f)
	{
		Exact = ExactMode::Zero;
		Valid = true;
		return;
	}
	Angle = degrees ? tMath::tDegToRad(angle) : angle;

	// Get mode: fill, crop*, or cropresize.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("fill"):		Mode = RotateMode::Fill;	break;
			case tHash::tHashCT("crop"):		Mode = RotateMode::Crop;	break;
			case tHash::tHashCT("resize"):		Mode = RotateMode::Resize;	break;
			// Mode is already at default if not found.
		}
	}

	// Get upfilter.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();

		// The +1 allows "none" to be considered. No match uses default.
		for (int f = 0; f < int(tImage::tResampleFilter::NumFilters)+1; f++)
		{
			if (currArg->IsEqualCI(tImage::tResampleFilterNamesSimple[f]))
			{
				FilterUp = tImage::tResampleFilter(f);
				break;
			}
		}
	}

	// Get downfilter.
	if (numArgs >= 4)
	{
		currArg = currArg->Next();

		// The +1 allows "none" to be considered. No match uses default.
		for (int f = 0; f < int(tImage::tResampleFilter::NumFilters)+1; f++)
		{
			if (currArg->IsEqualCI(tImage::tResampleFilterNamesSimple[f]))
			{
				FilterDown = tImage::tResampleFilter(f);
				break;
			}
		}
	}

	// Fill colour.
	if (numArgs >= 5)
	{
		currArg = currArg->Next();
		tString colStr = *currArg;
		ParseColour(FillColour, colStr);
	}

	Valid = true;
}


bool Command::OperationRotate::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	// First we deal with exact mode rotations.
	switch (Exact)
	{
		case ExactMode::Zero:
			tPrintfFull("Rotate not applied. Zero rotation specified.\n");
			return true;
		
		case ExactMode::ACW90:
			tPrintfFull("Rotate | Rotate90[Anticlockwise:true]\n");
			image.Rotate90(true);
			return true;

		case ExactMode::CW90:
			tPrintfFull("Rotate | Rotate90[Anticlockwise:false]\n");
			image.Rotate90(false);
			return true;

		case ExactMode::R180:
			tPrintfFull("Rotate | 2X Rotate90[Anticlockwise:true]\n");
			image.Rotate90(true);
			image.Rotate90(true);
			return true;

		case ExactMode::Off:
		default:
			break;
	};

	// Not an exact rotation. Rotate the image.
	int origW = image.GetWidth();
	int origH = image.GetHeight();

	tPrintfFull
	(
		"Rotate | Rotate\n[\n  rad:%f deg:%f\n  upfilt:%s dnfilt:%s\n  fill:%02x,%02x,%02x,%02x\n]\n",
		Angle, tMath::tRadToDeg(Angle),
		tImage::tResampleFilterNamesSimple[int(FilterUp)],
		tImage::tResampleFilterNamesSimple[int(FilterDown)],
		FillColour.R, FillColour.G, FillColour.B, FillColour.A
	);
	image.Rotate(Angle, FillColour, FilterUp, FilterDown);

	if ((Mode == RotateMode::Crop) || (Mode == RotateMode::Resize))
	{
		// If one of the crop modes is selected we need to crop the edges. Since rectangles are made of lines and there
		// is symmetry and we can compute the reduced size by subtracting the original size from the rotated size.
		int rotW = image.GetWidth();
		int rotH = image.GetHeight();
		bool aspectFlip = ((origW > origH) && (rotW < rotH)) || ((origW < origH) && (rotW > rotH));
		if (aspectFlip)
			tStd::tSwap(origW, origH);

		int dx = rotW - origW;
		int dy = rotH - origH;
		int newW = origW - dx;
		int newH = origH - dy;

		if (dx > origW/2)
		{
			newW = origW - origW/2;
			newH = (newW*origH)/origW;
		}
		else if (dy > origH/2)
		{
			newH = origH - origH/2;
			newW = (newH*origW)/origH;
		}

		// The above code has been tested with a 1x1 input and (newH,newW) result correcty as (1,1).
		tPrintfFull("Rotate | Crop[w:%d h:%d anc:mm]\n", newW, newH);
		image.Crop(newW, newH, tImage::tPicture::Anchor::MiddleMiddle);
	}

	if (Mode == RotateMode::Resize)
	{
		// The crop is done. Now resample. We use nearest neighbour if no up-filter specified so as to preserve colours.
		tImage::tResampleFilter filter = (FilterUp != tImage::tResampleFilter::None) ? FilterUp : tImage::tResampleFilter::Nearest;

		tPrintfFull("Rotate | Resample[w:%d h:%d filt:%s edge:clamp]\n", origW, origH, tImage::tResampleFilterNamesSimple[int(filter)]);
		image.Resample(origW, origH, filter, tImage::tResampleEdgeMode::Clamp);
	}

	return true;
}


Command::OperationLevels::OperationLevels(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');

	// BlackPoint.
	tStringItem* currArg = args.First();
	BlackPoint = currArg->AsFloat();
	tMath::tiSaturate(BlackPoint);

	// MidPoint.
	currArg = currArg->Next();
	tString mpstr = *currArg;
	if (mpstr == "*")
		MidPoint = -1.0f;
	else
		MidPoint = mpstr.AsFloat();

	// WhitePoint.
	currArg = currArg->Next();
	WhitePoint = currArg->AsFloat();
	tMath::tiSaturate(WhitePoint);

	// If auto-mid-point adjust midpoint here.
	if (MidPoint < 0.0f)
	{
		tMath::tiClampMin(WhitePoint, BlackPoint);
		MidPoint = (WhitePoint+BlackPoint)/2.0f;
	}
	else
	{
		tMath::tiSaturate(MidPoint);
		tMath::tiClampMin(MidPoint, BlackPoint);
		tMath::tiClampMin(WhitePoint, MidPoint);
	}

	// OutBlackPoint.
	if (numArgs >= 4)
	{
		currArg = currArg->Next();
		OutBlackPoint = currArg->AsFloat();
	}

	// OutWhitePoint.
	if (numArgs >= 5)
	{
		currArg = currArg->Next();
		OutWhitePoint = currArg->AsFloat();
		tMath::tiClampMin(OutWhitePoint, OutBlackPoint);
	}

	tMath::tiClampMin(OutWhitePoint, OutBlackPoint);

	// FrameNumber.
	if (numArgs >= 6)
	{
		currArg = currArg->Next();
		tString frameStr = *currArg;
		if (frameStr == "*")
			FrameNumber = -1;
		else
			FrameNumber = frameStr.AsInt32();
	}

	// Channels.
	if (numArgs >= 7)
	{
		currArg = currArg->Next();
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("*"):
			case tHash::tHashCT("rgb"):
			case tHash::tHashCT("RGB"):
				Channels = Viewer::Image::AdjChan::RGB;
				break;

			case tHash::tHashCT("r"):
			case tHash::tHashCT("R"):
				Channels = Viewer::Image::AdjChan::R;
				break;

			case tHash::tHashCT("g"):
			case tHash::tHashCT("G"):
				Channels = Viewer::Image::AdjChan::G;
				break;

			case tHash::tHashCT("b"):
			case tHash::tHashCT("B"):
				Channels = Viewer::Image::AdjChan::B;
				break;

			case tHash::tHashCT("a"):
			case tHash::tHashCT("A"):
				Channels = Viewer::Image::AdjChan::A;
				break;
			// Mode is already at default if not found.
		}
	}

	// PowerMidGamma.
	if (numArgs >= 8)
	{
		currArg = currArg->Next();
		tString powerMid = *currArg;
		if (powerMid == "*")
			PowerMidGamma = true;
		else
			PowerMidGamma = powerMid.AsBool();
	}

	Valid = true;
}


bool Command::OperationLevels::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	if ((BlackPoint == 0.0f) && (MidPoint == 0.5f) && (WhitePoint == 1.0) && (OutBlackPoint == 0.0f) && (OutWhitePoint == 1.0f))
	{
		tPrintfFull("Levels not applied. All point levels at default.\n");
		return true;
	}

	int origFrameNum = image.FrameNum;
	bool allFrames = true;
	if (FrameNumber > -1)
	{
		image.FrameNum = tMath::tClampMax(FrameNumber, image.GetNumFrames()-1);
		allFrames = false;
	}

	tString chanStr;
	switch (Channels)
	{
		case Viewer::Image::AdjChan::RGB:	chanStr = "rgb";	break;
		case Viewer::Image::AdjChan::R:		chanStr = "r";		break;
		case Viewer::Image::AdjChan::G:		chanStr = "g";		break;
		case Viewer::Image::AdjChan::B:		chanStr = "b";		break;
		case Viewer::Image::AdjChan::A:		chanStr = "a";		break;
	};
	tPrintfFull
	(
		"Levels | AdjustLevels\n[\n  blackpoint:%4.2f\n  midpoint:%4.2f\n  whitepoint:%4.2f\n  outblackpoint:%4.2f\n  outwhitepoint:%4.2f\n  powermidgamma:%B\n  channels:%s\n  allframes:%B\n]\n",
		BlackPoint, MidPoint, WhitePoint,
		OutBlackPoint, OutWhitePoint,
		PowerMidGamma, chanStr.Chr(), allFrames
	);

	image.AdjustmentBegin();
	image.AdjustLevels(BlackPoint, MidPoint, WhitePoint, OutBlackPoint, OutWhitePoint, PowerMidGamma, Channels, allFrames);
	image.AdjustmentEnd();

	image.FrameNum = origFrameNum;
	return true;
}


Command::OperationContrast::OperationContrast(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');

	// Contrast.
	tStringItem* currArg = args.First();
	tString contStr = *currArg;
	if (contStr == "*")
		Contrast = 0.5f;
	else
		Contrast = contStr.AsFloat();
	tMath::tiSaturate(Contrast);

	// FrameNumber.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();
		tString frameStr = *currArg;
		if (frameStr == "*")
			FrameNumber = -1;
		else
			FrameNumber = frameStr.AsInt32();
	}

	// Channels.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("*"):
			case tHash::tHashCT("rgb"):
			case tHash::tHashCT("RGB"):
				Channels = Viewer::Image::AdjChan::RGB;
				break;

			case tHash::tHashCT("r"):
			case tHash::tHashCT("R"):
				Channels = Viewer::Image::AdjChan::R;
				break;

			case tHash::tHashCT("g"):
			case tHash::tHashCT("G"):
				Channels = Viewer::Image::AdjChan::G;
				break;

			case tHash::tHashCT("b"):
			case tHash::tHashCT("B"):
				Channels = Viewer::Image::AdjChan::B;
				break;

			case tHash::tHashCT("a"):
			case tHash::tHashCT("A"):
				Channels = Viewer::Image::AdjChan::A;
				break;
			// Mode is already at default if not found.
		}
	}

	Valid = true;
}


bool Command::OperationContrast::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	if (Contrast == 0.5f)
	{
		tPrintfFull("Contrast not applied. Value of 0.5 does not modify image.\n");
		return true;
	}

	int origFrameNum = image.FrameNum;
	bool allFrames = true;
	if (FrameNumber > -1)
	{
		image.FrameNum = tMath::tClampMax(FrameNumber, image.GetNumFrames()-1);
		allFrames = false;
	}

	tString chanStr;
	switch (Channels)
	{
		case Viewer::Image::AdjChan::RGB:	chanStr = "rgb";	break;
		case Viewer::Image::AdjChan::R:		chanStr = "r";		break;
		case Viewer::Image::AdjChan::G:		chanStr = "g";		break;
		case Viewer::Image::AdjChan::B:		chanStr = "b";		break;
		case Viewer::Image::AdjChan::A:		chanStr = "a";		break;
	};
	tPrintfFull
	(
		"Contrast | AdjustContrast[contrast:%4.2f channels:%s allframes:%B]\n",
		Contrast, chanStr.Chr(), allFrames
	);

	image.AdjustmentBegin();
	image.AdjustContrast(Contrast, Channels, allFrames);
	image.AdjustmentEnd();

	image.FrameNum = origFrameNum;
	return true;
}


Command::OperationBrightness::OperationBrightness(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');

	// Brightness.
	tStringItem* currArg = args.First();
	tString brightStr = *currArg;
	if (brightStr == "*")
		Brightness = 0.5f;
	else
		Brightness = brightStr.AsFloat();
	tMath::tiSaturate(Brightness);

	// FrameNumber.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();
		tString frameStr = *currArg;
		if (frameStr == "*")
			FrameNumber = -1;
		else
			FrameNumber = frameStr.AsInt32();
	}

	// Channels.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("*"):
			case tHash::tHashCT("rgb"):
			case tHash::tHashCT("RGB"):
				Channels = Viewer::Image::AdjChan::RGB;
				break;

			case tHash::tHashCT("r"):
			case tHash::tHashCT("R"):
				Channels = Viewer::Image::AdjChan::R;
				break;

			case tHash::tHashCT("g"):
			case tHash::tHashCT("G"):
				Channels = Viewer::Image::AdjChan::G;
				break;

			case tHash::tHashCT("b"):
			case tHash::tHashCT("B"):
				Channels = Viewer::Image::AdjChan::B;
				break;

			case tHash::tHashCT("a"):
			case tHash::tHashCT("A"):
				Channels = Viewer::Image::AdjChan::A;
				break;
			// Mode is already at default if not found.
		}
	}

	Valid = true;
}


bool Command::OperationBrightness::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	if (Brightness == 0.5f)
	{
		tPrintfFull("Brightness not applied. Value of 0.5 does not modify image.\n");
		return true;
	}

	int origFrameNum = image.FrameNum;
	bool allFrames = true;
	if (FrameNumber > -1)
	{
		image.FrameNum = tMath::tClampMax(FrameNumber, image.GetNumFrames()-1);
		allFrames = false;
	}

	tString chanStr;
	switch (Channels)
	{
		case Viewer::Image::AdjChan::RGB:	chanStr = "rgb";	break;
		case Viewer::Image::AdjChan::R:		chanStr = "r";		break;
		case Viewer::Image::AdjChan::G:		chanStr = "g";		break;
		case Viewer::Image::AdjChan::B:		chanStr = "b";		break;
		case Viewer::Image::AdjChan::A:		chanStr = "a";		break;
	};
	tPrintfFull
	(
		"Brightness | AdjustBrightness[brightness:%4.2f channels:%s allframes:%B]\n",
		Brightness, chanStr.Chr(), allFrames
	);

	image.AdjustmentBegin();
	image.AdjustBrightness(Brightness, Channels, allFrames);
	image.AdjustmentEnd();

	image.FrameNum = origFrameNum;
	return true;
}


Command::OperationQuantize::OperationQuantize(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');

	// Method.
	tStringItem* currArg = args.First();
	switch (tHash::tHashString(currArg->Chr()))
	{
		case tHash::tHashCT("fix"):
			Method = tImage::tQuantize::Method::Fixed;
			break;

		case tHash::tHashCT("spc"):
			Method = tImage::tQuantize::Method::Spatial;
			break;

		case tHash::tHashCT("neu"):
			Method = tImage::tQuantize::Method::Neu;
			break;

		case tHash::tHashCT("wu"):
		case tHash::tHashCT("*"):
			Method = tImage::tQuantize::Method::Wu;
			break;
	}

	// NumColours.
	currArg = currArg->Next();
	tString numColsStr = *currArg;
	if (numColsStr == "*")
		NumColours = 256;
	else
		NumColours = numColsStr.AsInt();
	tMath::tiClamp(NumColours, 2, 256);

	// CheckExact.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		CheckExact = currArg->AsBool();
	}

	// SampFilt. Defaults are different depending on Method.
	switch (Method)
	{
		case tImage::tQuantize::Method::Spatial:	SampFilt = 3;	break;
		case tImage::tQuantize::Method::Neu:		SampFilt = 1;	break;
	}

	if (numArgs >= 4)
	{
		currArg = currArg->Next();
		tString sampFiltStr = *currArg;
		switch (Method)
		{
			case tImage::tQuantize::Method::Spatial:	// For spatial it's filterSize.
				if (sampFiltStr == "*")
					SampFilt = 3;
				else
					SampFilt = sampFiltStr.AsInt();

				if ((SampFilt != 1) && (SampFilt != 3) && (SampFilt != 5))
					SampFilt = 3;
				break;

			case tImage::tQuantize::Method::Neu:		// For neu it's sampleFactor.
				if (sampFiltStr == "*")
					SampFilt = 1;
				else
					SampFilt = sampFiltStr.AsInt();
				tMath::tiClamp(SampFilt, 1, 30);
				break;
		}
	}

	// Dither.
	if (numArgs >= 5)
	{
		currArg = currArg->Next();
		tString ditherStr = *currArg;
		switch (Method)
		{
			case tImage::tQuantize::Method::Spatial:
				if (ditherStr == "*")
					Dither = 0.0;
				else
					Dither = ditherStr.AsDouble();
				tMath::tiClamp(Dither, 0.0, 30.0);
				break;
		}
	}

	Valid = true;
}


bool Command::OperationQuantize::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	switch (Method)
	{
		case tImage::tQuantize::Method::Fixed:
			tPrintfFull("Quantize | QuantizeFixed[numcolours:%d exact:%B]\n", NumColours, CheckExact);
			image.QuantizeFixed(NumColours, CheckExact);
			break;

		case tImage::tQuantize::Method::Spatial:
			tPrintfFull("Quantize | QuantizeSpatial[numcolours:%d exact:%B dith:%f filt:%d]\n", NumColours, CheckExact, Dither, SampFilt);
			image.QuantizeSpatial(NumColours, CheckExact, Dither, SampFilt);
			break;

		case tImage::tQuantize::Method::Neu:
			tPrintfFull("Quantize | QuantizeNeu[numcolours:%d exact:%B samp:%d]\n", NumColours, CheckExact, SampFilt);
			image.QuantizeNeu(NumColours, CheckExact, SampFilt);
			break;

		case tImage::tQuantize::Method::Wu:
			tPrintfFull("Quantize | QuantizeWu[numcolours:%d exact:%B]\n", NumColours, CheckExact);
			image.QuantizeWu(NumColours, CheckExact);
			break;
	};

	return true;
}


Command::OperationChannel::OperationChannel(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	tStringItem* currArg = nullptr;

	// Mode.
	// Defaults to Blend if not specified. This switch also sets the default channels based
	// on the mode.
	if (numArgs >= 1)
	{
		currArg = args.First();
		switch (tHash::tHashString(currArg->Chr()))
		{
			case tHash::tHashCT("set"):
				Mode = ChanMode::Set;
				Channels = tCompBit_RGB;
				break;

			case tHash::tHashCT("*"):
			case tHash::tHashCT("blend"):
				Mode = ChanMode::Blend;
				Channels = tCompBit_RGBA;
				break;

			case tHash::tHashCT("spread"):
				Mode = ChanMode::Spread;
				Channels = tCompBit_R;
			break;

			case tHash::tHashCT("intens"):
				Mode = ChanMode::Intensity;
				Channels = tCompBit_RGB;
				break;
		}
	}

	// Channels.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();
		tString chanStr = *currArg;
		ParseChannels(Channels, chanStr);
		tAssert(Channels);

		// If mode is spread, only one channel is allowed. We choose first one (LSB first) if multiple.
		if (Mode == ChanMode::Spread)
		{
			int setIdx = tMath::tFindFirstSetBit(Channels);
			tAssert(setIdx != -1);
			Channels = 1 << setIdx;
		}
	}

	// Colour.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();
		tString colStr = *currArg;
		ParseColour(Colour, colStr);
		// If nothing above set the colour (like "*") then the default of black (0,0,0,255) is used.
	}

	Valid = true;
}


bool Command::OperationChannel::Apply(Viewer::Image& image)
{
	tAssert(Valid);

	tString channelsStr;
	if (Channels & tCompBit_R) channelsStr += "R";
	if (Channels & tCompBit_G) channelsStr += "G";
	if (Channels & tCompBit_B) channelsStr += "B";
	if (Channels & tCompBit_A) channelsStr += "A";

	switch (Mode)
	{
		case ChanMode::Set:
			tPrintfFull("Channel | SetAllPixels[colour:%02x,%02x,%02x,%02x channels:%s]\n", Colour.R, Colour.G, Colour.B, Colour.A, channelsStr.Chr());
			image.SetAllPixels(Colour, Channels);
			break;

		case ChanMode::Blend:
		{
			int finalAlpha = (Channels & tCompBit_A) ? Colour.A : -1;
			tPrintfFull("Channel | AlphaBlendColour[colour:%02x,%02x,%02x,%02x channels:%s finalAlpha:%d]\n", Colour.R, Colour.G, Colour.B, Colour.A, channelsStr.Chr(), finalAlpha);
			image.AlphaBlendColour(Colour, Channels, finalAlpha);
			break;
		}

		case ChanMode::Spread:
			tPrintfFull("Channel | Spread[channel:%s]\n", channelsStr.Chr());
			image.Spread(tComp(tMath::tFindFirstSetBit(Channels)));
			break;

		case ChanMode::Intensity:
			tPrintfFull("Channel | Intensity[channels:%s]\n", channelsStr.Chr());
			image.Intensity(Channels);
			break;
	}

	return true;
}


Command::OperationSwizzle::OperationSwizzle(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	tStringItem* currArg = nullptr;

	// RGBA swizzle string. It seems a bit nicer to have all swizzles in a single
	// string without the comma separator. Defaults to RGBA if not specified.
	if (numArgs >= 1)
	{
		currArg = args.First();
		tString swizzle = *currArg;
		int len = swizzle.Length();
		
		char rs = (len >= 1) ? swizzle[0] : '\0';
		char gs = (len >= 2) ? swizzle[1] : '\0';
		char bs = (len >= 3) ? swizzle[2] : '\0';
		char as = (len >= 4) ? swizzle[3] : '\0';

		SwizzleR = CharToComp(rs);
		SwizzleG = CharToComp(gs);
		SwizzleB = CharToComp(bs);
		SwizzleA = CharToComp(as);
	}

	Valid = true;
}


tComp Command::OperationSwizzle::CharToComp(char cs)
{
	switch (cs)
	{
		case 'R': case 'r':		return tComp::R;
		case 'G': case 'g':		return tComp::G;
		case 'B': case 'b':		return tComp::B;
		case 'A': case 'a':		return tComp::A;
		case '*':				return tComp::Auto;
		case '0':				return tComp::Zero;
		case '1':				return tComp::Full;
	}

	return tComp::Auto;
}


bool Command::OperationSwizzle::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	tPrintfFull("Swizzle | Swizzle[%s%s%s%s]\n", tGetComponentName(SwizzleR), tGetComponentName(SwizzleB), tGetComponentName(SwizzleB), tGetComponentName(SwizzleA));
	image.Swizzle(SwizzleR, SwizzleG, SwizzleB, SwizzleA);
	return true;
}


Command::OperationExtract::OperationExtract(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	tStringItem* currArg = nullptr;

	// Frame numbers.
	if (numArgs >= 1)
	{
		currArg = args.First();
		tString frames = *currArg;
		if (frames == "*")
			FrameSet.Clear();
		else
			// Populate Frames member. If the string was badly formed the frames will be empty.
			// When the frames are empty, all frames are extracted, so there is no more error checking needed.
			FrameSet.Set(frames);
	}

	// Subfolder.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();

		// If SubFolder is empty the default sub-folder name of "Saved" will be used.
		SubFolder = *currArg;
		if (SubFolder.IsEmpty() || (SubFolder == "*"))
		{
			SubFolder.Clear();
		}
		else
		{
			if (SubFolder[SubFolder.Length()-1] == '\\')
				SubFolder[SubFolder.Length()-1] = '/';
			if (SubFolder[SubFolder.Length()-1] != '/')
				SubFolder += "/";
		}
	}

	// Basename.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();

		// If BaseName is empty the default is to use the base name of the image being processed.
		BaseName = *currArg;
		if (BaseName == "*")
			BaseName.Clear();
	}

	Valid = true;
}


bool Command::OperationExtract::Apply(Viewer::Image& image)
{
	tAssert(Valid);
	tString subDir = "Saved/";
	if (!SubFolder.IsEmpty())
		subDir = SubFolder;

	tString destDir = tSystem::tGetDir(image.Filename) + subDir;
	bool dirExists = tSystem::tDirExists(destDir);
	if (!dirExists)
		dirExists = tSystem::tCreateDir(destDir);
	if (!dirExists)
		return false;

	tString baseName = BaseName;
	if (baseName.IsEmpty())
		baseName = tSystem::tGetFileBaseName(image.Filename);

	Command::SetImageSaveParameters(image, Command::OutType);
	for (int frameNum = 0; frameNum < image.GetNumFrames(); frameNum++)
	{
		if (!FrameSet.Contains(frameNum))
			continue;

		image.FrameNum = frameNum;

		tString outFile = Viewer::GetFrameFilename(frameNum, destDir, baseName, Command::OutType);
		tString outFileShort = tSystem::tGetFileName(outFile);
		if (!Command::OptionOverwrite && tSystem::tFileExists(outFile))
		{
			tPrintfFull("Extract | File %s%s exists. Not overwriting.\n", subDir.Chr(), outFileShort.Chr());
			continue;
		}

		bool useConfigSaveParams = false;
		bool onlyCurrentPic = true;

		tPrintfFull("Extract | Save[file:%s%s]\n", subDir.Chr(), outFileShort.Chr());
		image.Save(outFile, Command::OutType, useConfigSaveParams, onlyCurrentPic);
	}

	return true;
}


// Post operations follow.


Command::PostOperationCombine::PostOperationCombine(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	tStringItem* currArg = nullptr;

	// Frame durations.
	if (numArgs >= 1)
	{
		currArg = args.First();
		tString durations = *currArg;

		// The default is an empty durations list. If empty or "*" is passed in, that is what we get.
		if (!durations.IsEmpty() && (durations != "*"))
		{
			tList<tStringItem> pairStrs;
			durations.Replace('U', '+');
			durations.RemoveAnyNot("!-0123456789.:+");
			tStd::tExplode(pairStrs, durations, '+');

			// Populate Durations member. If the string was badly formed the durations will be empty
			// and the default duration will be used for all frames.
			for (tStringItem* pairStr = pairStrs.First(); pairStr; pairStr = pairStr->Next())
			{
				tString intStr = pairStr->Left(':');
				tString durStr = pairStr->Right(':');
				IntervalDurationPair* pair = new IntervalDurationPair;
				pair->FrameInterval.Set(intStr);
				pair->Duration = durStr.AsFloat();
				Durations.Append(pair);

				tPrintfFull("Pair: %s : %f\n", pair->FrameInterval.Get().Chr(), pair->Duration);
			}
		}
	}

	// Subfolder.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();

		// If SubFolder is empty the default sub-folder name of "Saved" will be used.
		SubFolder = *currArg;
		if (SubFolder.IsEmpty() || (SubFolder == "*"))
		{
			SubFolder.Clear();
		}
		else
		{
			if (SubFolder[SubFolder.Length()-1] == '\\')
				SubFolder[SubFolder.Length()-1] = '/';
			if (SubFolder[SubFolder.Length()-1] != '/')
				SubFolder += "/";
		}
	}

	// Basename.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();

		// If BaseName is empty the default is to use the base name of the image being processed.
		BaseName = *currArg;
		if (BaseName == "*")
			BaseName.Clear();
	}

	Valid = true;
}


float Command::PostOperationCombine::GetFrameDuration(int frameNum) const
{
	for (IntervalDurationPair* pair = Durations.Last(); pair; pair = pair->Prev())
		if (pair->FrameInterval.Contains(frameNum))
			return pair->Duration / 1000.0f;

	return 33.0f/1000.0f;
}


bool Command::PostOperationCombine::Apply(tList<Viewer::Image>& images)
{
	tAssert(Valid);
	if (!Viewer::FileTypes_SaveMultiFrame.Contains(Command::OutType))
	{
		tPrintfNorm("Combine | Filetype %s does not support Multi-Frame.\n", tSystem::tGetFileTypeName(Command::OutType).Chr());
		return false;
	}

	// Ensure the output directory exists.
	tString subDir = "Combined/";
	if (!SubFolder.IsEmpty())
		subDir = SubFolder;	
	tString destDir = tSystem::tGetCurrentDir() + subDir;
	bool dirExists = tSystem::tDirExists(destDir);
	if (!dirExists)
		dirExists = tSystem::tCreateDir(destDir);
	if (!dirExists)
	{
		tPrintfNorm("Combine | Could not create sub-directory %s.\n", subDir.Chr());
		return false;
	}

	// Determine the output filename.
	tString extension = tSystem::tGetExtension(Command::OutType);
	tString outFile;
	tString baseName = BaseName;
	if (baseName.IsEmpty())
	{
		tsPrintf
		(
			outFile, "%sCombined_%s_%03d.%s",
			destDir.Chr(),
			tSystem::tConvertTimeToString(tSystem::tGetTimeLocal(), tSystem::tTimeFormat::Filename).Chr(),
			images.GetNumItems(),
			extension.Chr()
		);
	}
	else
	{
		outFile = destDir + baseName + "." + extension;
	}

	// No need to continue if we know we won't be able to save the outFile.
	if (!Command::OptionOverwrite && tSystem::tFileExists(outFile))
	{
		tPrintfNorm("Combine | File %s%s exists. Not overwriting.\n", subDir.Chr(), tSystem::tGetFileName(outFile).Chr());
		return false;
	}

	// We need to load the first image to determine the width and height. All input images must have the same width and
	// height otherwise it is considered an error. This has 2 benefits: a) You get more control of how to resize/crop
	// images to the desired size by using the resize/crop regular operations, and b) This combine call does not need
	// to load all source images at the same time.
	Viewer::Image* firstImage = images.First();
	if (!firstImage->IsLoaded())
		firstImage->Load();
	int width = firstImage->GetWidth();
	int height = firstImage->GetHeight();
	if ((width < 4) || (height < 4))
	{
		tPrintfNorm("Combine | Invalid image dimensions %dx%d.\n", width, height);
		return false;
	}

	// Create the frames of the output image. Make sure for each one we set the duration correctly.
	tList<tImage::tFrame> frames;
	int frameNumber = 0;
	for (Viewer::Image* img = images.First(); img; img = img->Next(), frameNumber++)
	{
		tPrintfFull("Combine | LoadImage[frame:%d]\n", frameNumber);
		if (!img->IsLoaded())
			img->Load();

		tImage::tPicture* currPic = img->GetCurrentPic();
		if (!img->IsLoaded() || !currPic)
		{
			tPrintfNorm("Combine | Error loading input image %s.\n", tSystem::tGetFileName(img->Filename).Chr());
			return false;
		}

		if ((currPic->GetWidth() != width) || (currPic->GetHeight() != height))
		{
			tPrintfNorm("Combine | All input images must be %dx%d.\n", width, height);
			return false;
		}

		// Determine duration.
		float duration = GetFrameDuration(frameNumber);
		tImage::tFrame* frame = new tImage::tFrame(currPic->StealPixels(), width, height, duration);
		frames.Append(frame);
		img->Unload();
	}

	// The set of frames is ready. Now we need to create the combined image file from them.
	bool success = false;
	tPrintfFull("Combine | Save[file:%s]\n", tSystem::tGetFileName(outFile).Chr());
	switch (OutType)
	{
		case tSystem::tFileType::GIF:
		{
			tImage::tImageGIF gif(frames, true);
			success = gif.Save(outFile, SaveParamsGIF);
			break;
		}

		case tSystem::tFileType::WEBP:
		{
			tImage::tImageWEBP webp(frames, true);
			success = webp.Save(outFile, SaveParamsWEBP);
			break;
		}

		case tSystem::tFileType::APNG:
		{
			tImage::tImageAPNG apng(frames, true);
			tImage::tImageAPNG::tFormat savedFormat = apng.Save(outFile, SaveParamsAPNG);
			success = (savedFormat != tImage::tImageAPNG::tFormat::Invalid);
			break;
		}

		case tSystem::tFileType::TIFF:
		{
			tImage::tImageTIFF tiff(frames, true);
			success = tiff.Save(outFile, SaveParamsTIFF);
			break;
		}
	}

	return success;
}


Command::PostOperationContact::PostOperationContact(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	tStringItem* currArg = nullptr;

	// Columns.
	if (numArgs >= 1)
	{
		currArg = args.First();
		Columns = currArg->AsInt32();
	}

	// Rows.
	if (numArgs >= 2)
	{
		currArg = currArg->Next();
		Rows = currArg->AsInt32();
	}

	// Subfolder.
	if (numArgs >= 3)
	{
		currArg = currArg->Next();

		// If SubFolder is empty the default sub-folder name of "Saved" will be used.
		SubFolder = *currArg;
		if (SubFolder.IsEmpty() || (SubFolder == "*"))
		{
			SubFolder.Clear();
		}
		else
		{
			if (SubFolder[SubFolder.Length()-1] == '\\')
				SubFolder[SubFolder.Length()-1] = '/';
			if (SubFolder[SubFolder.Length()-1] != '/')
				SubFolder += "/";
		}
	}

	// Basename.
	if (numArgs >= 4)
	{
		currArg = currArg->Next();

		// If BaseName is empty the default is to use the base name of the image being processed.
		BaseName = *currArg;
		if (BaseName == "*")
			BaseName.Clear();
	}

	Valid = true;
}


bool Command::PostOperationContact::Apply(tList<Viewer::Image>& images)
{
	tAssert(Valid);
	if (!Viewer::FileTypes_Save.Contains(Command::OutType))
	{
		tPrintfNorm("Contact | Filetype %s cannot be saved.\n", tSystem::tGetFileTypeName(Command::OutType).Chr());
		return false;
	}

	// If columns or rows (or both) are 0 we need to compute them based on the total number of images.
	int numImages = images.GetNumItems();
	int rows = Rows;
	int cols = Columns;
	if ((cols <= 0) && (rows <= 0))
	{
		float root = tMath::tSqrt(float(numImages));
		tMath::tiCeiling(root);
		cols = int(root);
		rows = numImages / cols;
		if (cols*rows < numImages)
			rows++;
		tAssert(cols*rows >= numImages);
	}
	else if (cols <= 0)
	{
		cols = numImages / rows;
		if (cols*rows < numImages)
			cols++;
		tAssert(cols*rows >= numImages);
	}
	else if (rows <= 0)
	{
		rows = numImages / cols;
		if (cols*rows < numImages)
			rows++;
		tAssert(cols*rows >= numImages);
	}

	if (cols*rows < numImages)
		tPrintfFull("Warning: %dx%d contact pages is not enough for %d images.\n", cols, rows, numImages);

	// Ensure the output directory exists.
	tString subDir = "Contact/";
	if (!SubFolder.IsEmpty())
		subDir = SubFolder;	
	tString destDir = tSystem::tGetCurrentDir() + subDir;
	bool dirExists = tSystem::tDirExists(destDir);
	if (!dirExists)
		dirExists = tSystem::tCreateDir(destDir);
	if (!dirExists)
	{
		tPrintfNorm("Contact | Could not create sub-directory %s.\n", subDir.Chr());
		return false;
	}

	// Determine the output filename.
	tString extension = tSystem::tGetExtension(Command::OutType);
	tString outFile;
	tString baseName = BaseName;
	if (baseName.IsEmpty())
	{
		tsPrintf
		(
			outFile, "%sContact_%s_%02dx%02d.%s",
			destDir.Chr(),
			tSystem::tConvertTimeToString(tSystem::tGetTimeLocal(), tSystem::tTimeFormat::Filename).Chr(),
			cols, rows,
			extension.Chr()
		);
	}
	else
	{
		outFile = destDir + baseName + "." + extension;
	}

	// No need to continue if we know we won't be able to save the outFile.
	if (!Command::OptionOverwrite && tSystem::tFileExists(outFile))
	{
		tPrintfNorm("Contact | File %s%s exists. Not overwriting.\n", subDir.Chr(), tSystem::tGetFileName(outFile).Chr());
		return false;
	}

	// We need to load the first image to determine the width and height. All input images must have the same width and
	// height otherwise it is considered an error. This has 2 benefits: a) You get more control of how to resize/crop
	// images to the desired size by using the resize/crop regular operations, and b) This combine call does not need
	// to load all source images at the same time.
	Viewer::Image* firstImage = images.First();
	if (!firstImage->IsLoaded())
		firstImage->Load();
	int frameWidth = firstImage->GetWidth();
	int frameHeight = firstImage->GetHeight();
	if ((frameWidth < 4) || (frameHeight < 4))
	{
		tPrintfNorm("Contact | Invalid image dimensions %dx%d. Must be at least 4x4.\n", frameWidth, frameHeight);
		return false;
	}

	// Do the actual work here.
	int contactWidth = frameWidth*cols;
	int contactHeight = frameHeight*rows;
	tImage::tPicture outPic(contactWidth, contactHeight);
	outPic.SetAll(tColouri(0, 0, 0, 0));

	int ix = 0;
	int iy = 0;
	int frame = 0;
	for (Viewer::Image* img = images.First(); img; img = img->Next(), frame++)
	{
		if (!img->IsLoaded())
			img->Load();

		tPrintf("Processing frame %d : %s at (%d, %d).\n", frame, img->Filename.Chr(), ix, iy);

		if ((img->GetWidth() != frameWidth) || (img->GetHeight() != frameHeight))
		{
			img->Unload();
			tPrintfNorm("Contact | All input images must be same size.\n");
			return false;
		}

		tImage::tPicture* currPic = img->GetCurrentPic();

		// Copy resampled frame into place.
		for (int y = 0; y < frameHeight; y++)
		{
			for (int x = 0; x < frameWidth; x++)
			{
				outPic.SetPixel
				(
					x + (ix*frameWidth),
					y + ((rows-1-iy)*frameHeight),
					currPic->GetPixel(x, y)
				);
			}
		}

		img->Unload();
		ix++;
		if (ix >= cols)
		{
			ix = 0;
			iy++;
			if (iy >= rows)
				break;
		}
	}

	if (!outPic.IsValid())
		return false;

	// The outPic is ready. Now we need to create the combined image file from it.
	bool success = false;
	tPrintfFull("Contact | Save[file:%s]\n", tSystem::tGetFileName(outFile).Chr());
	switch (OutType)
	{
		case tSystem::tFileType::TGA:
		{
			tImage::tImageTGA tga(outPic, true);
			tImage::tImageTGA::tFormat savedFmt = tga.Save(outFile, SaveParamsTGA);
			success = (savedFmt != tImage::tImageTGA::tFormat::Invalid);
			break;
		}

		case tSystem::tFileType::PNG:
		{
			tImage::tImagePNG png(outPic, true);
			tImage::tImagePNG::tFormat savedFmt = png.Save(outFile, SaveParamsPNG);
			success = (savedFmt != tImage::tImagePNG::tFormat::Invalid);
			break;
		}

/*
		case tSystem::tFileType::GIF:
		{
			tImage::tImageGIF gif(frames, true);
			success = gif.Save(outFile, SaveParamsGIF);
			break;
		}

		case tSystem::tFileType::WEBP:
		{
			tImage::tImageWEBP webp(frames, true);
			success = webp.Save(outFile, SaveParamsWEBP);
			break;
		}

		case tSystem::tFileType::APNG:
		{
			tImage::tImageAPNG apng(frames, true);
			tImage::tImageAPNG::tFormat savedFormat = apng.Save(outFile, SaveParamsAPNG);
			success = (savedFormat != tImage::tImageAPNG::tFormat::Invalid);
			break;
		}

		case tSystem::tFileType::TIFF:
		{
			tImage::tImageTIFF tiff(frames, true);
			success = tiff.Save(outFile, SaveParamsTIFF);
			break;
		}
		*/
	}

	return success;
}
