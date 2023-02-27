// CommandOps.cpp
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

#include "CommandOps.h"
#include "Command.h"


Command::OperationResize::OperationResize(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs < 2)
	{
		Op = OpType::Invalid;
		return;
	}

	// The AsInt calls return 0 if * is entered.
	tStringItem* currArg = args.First();
	Width = currArg->AsInt32();

	currArg = currArg->Next();
	Height = currArg->AsInt32();
	tPrintfFull("Operation Resize. Width:%d Height:%d\n", Width, Height);

	// Either width or height needs to be specified. If only one is present it uses aspect preserve.
	if ((Width <= 0) && (Height <= 0))
	{
		tPrintfNorm("Operation resize invalid. Width or Height or both must be specified.\n");
		Op = OpType::Invalid;
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

	Op = OpType::Resize;
}


bool Command::OperationResize::Apply(Viewer::Image& image)
{
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

	tMath::tiClamp(dstW, 4, 32768);
	tMath::tiClamp(dstH, 4, 32768);

	if ((srcW == dstW) && (srcH == dstH))
	{
		tPrintfFull("Resize not applied. Image already has correct dimensions.\n");
		return true;
	}

	tPrintfFull("Resample. Dim:%dx%d Filter:%s EdgeMode:%s\n", dstW, dstH,
		tImage::tResampleFilterNamesSimple[int(ResampleFilter)],
		tImage::tResampleEdgeModeNamesSimple[int(EdgeMode)]);

	image.Resample(dstW, dstH, ResampleFilter, EdgeMode);
	return true;
}


Command::OperationCanvas::OperationCanvas(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	Op = OpType::Canvas;
}


bool Command::OperationCanvas::Apply(Viewer::Image& image)
{
	return true;
}


Command::OperationAspect::OperationAspect(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	Op = OpType::Aspect;
}


bool Command::OperationAspect::Apply(Viewer::Image& image)
{
	return true;
}


Command::OperationDeborder::OperationDeborder(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	Op = OpType::DeBorder;
}


bool Command::OperationDeborder::Apply(Viewer::Image& image)
{
	return true;
}


Command::OperationRotate::OperationRotate(const tString& argsStr)
{
	tList<tStringItem> args;
	int numArgs = tStd::tExplode(args, argsStr, ',');
	if (numArgs != 1)
	{
		Op = OpType::Invalid;
		return;
	}
	tStringItem* currArg = args.First();
	Angle = currArg->AsFloat();

	tPrintfFull("Operation Rotate. Angle:%f\n", Angle);
	Op = OpType::Rotate;
}


bool Command::OperationRotate::Apply(Viewer::Image& image)
{
	float angleRadians = tMath::tDegToRad(Angle);
	image.Rotate(angleRadians, tColouri::black, tImage::tResampleFilter::Bicubic, tImage::tResampleFilter::Box);
	return true;
}
