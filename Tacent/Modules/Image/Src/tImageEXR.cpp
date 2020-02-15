// tImageEXR.cpp
//
// This knows how to load and save OpenEXR images (.exr). It knows the details of the exr high dynamic range
// file format and loads the data into a tPixel array. These tPixels may be 'stolen' by the tPicture's constructor if
// an EXR file is specified. After the array is stolen the tImageEXR is invalid. This is purely for performance.
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

#include <Foundation/tStandard.h>
#include <Foundation/tString.h>

#pragma warning( push )
#pragma warning(disable:4541)
#include <OpenEXR/loadImage.h>
#include <OpenEXR/IlmImf/ImfMultiPartInputFile.h>
#pragma warning( pop )

#include <System/tFile.h>
#include "Image/tImageEXR.h"
using namespace tSystem;
using namespace IMF;
using namespace IMATH;
using namespace std;

namespace tImage
{


bool tImageEXR::Load(const tString& exrFile/*, double gamma, int exposure*/)
{
	Clear();

	if (tSystem::tGetFileType(exrFile) != tSystem::tFileType::EXR)
		return false;

	if (!tFileExists(exrFile))
		return false;

	int numParts = 0;
	try
	{
    	MultiPartInputFile* mpfile = new MultiPartInputFile(exrFile.Chars());
        numParts = mpfile->parts();
        delete mpfile;
	}
    catch (IEX_NAMESPACE::BaseExc &e)
    {
        tPrintf("Error: Can't read exr file code: %d\n", e.what());
        return false;
    }

	//const char* channels = "AZRGB";
	//const char* layers = "0";
	bool preview = false;
	int lx = -1; int ly = -1;		// For tiled image shows level (lx,ly)
	bool compositeDeep = true;

	int outZsize = 0;
	Header outHeader;
    IMF::Array<IMF::Rgba> pixels;
    IMF::Array<float*> zbuffer;
    IMF::Array<unsigned int> sampleCount;

	loadImage
	(
		exrFile.Chars(),
		nullptr,			// Channels. Null means all.
		nullptr,			// Channels. O means first one.
		preview, lx, ly,
		0,					// PartNum.
		outZsize, outHeader,
		pixels, zbuffer, sampleCount,
		compositeDeep
	);

	const Box2i &displayWindow = outHeader.displayWindow();
	const Box2i &dataWindow = outHeader.dataWindow();
	float pixelAspectRatio = outHeader.pixelAspectRatio();
	int w  = displayWindow.max.x - displayWindow.min.x + 1;
	int h  = displayWindow.max.y - displayWindow.min.y + 1;
	int dw = dataWindow.max.x - dataWindow.min.x + 1;
	int dh = dataWindow.max.y - dataWindow.min.y + 1;
	int dx = dataWindow.min.x - displayWindow.min.x;
	int dy = dataWindow.min.y - displayWindow.min.y;

	// Set width, height, and allocate and set Pixels.
	Width = dw;
	Height = dh;
	Pixels = new tPixel[Width*Height];

	// Texview has 0,0 at bottom-left. Rows start from bottom.
	int p = 0;
	for (int yi = Height-1; yi >= 0; yi--)
	{
		for (int xi = 0; xi < Width; xi++)
		{
			int idx = yi*Width + xi;
			Pixels[p++] = tPixel
			(
				float(pixels[idx].r),
				float(pixels[idx].g),
				float(pixels[idx].b),
				float(pixels[idx].a)
			);
		}
	}

	return true;
}


bool tImageEXR::Set(tPixel* pixels, int width, int height, bool steal)
{
	Clear();
	if (!pixels || (width <= 0) || (height <= 0))
		return false;

	Width = width;
	Height = height;
	if (steal)
	{
		Pixels = pixels;
	}
	else
	{
		Pixels = new tPixel[Width*Height];
		tStd::tMemcpy(Pixels, pixels, Width*Height*sizeof(tPixel));
	}
	return true;
}


bool tImageEXR::Save(const tString& exrFile) const
{
	tAssertMsg(false, "EXR Save not implemented.");
	if (!IsValid())
		return false;

	if (tSystem::tGetFileType(exrFile) != tSystem::tFileType::EXR)
		return false;

	tFileHandle file = tOpenFile(exrFile.ConstText(), "wb");
	if (!file)
		return false;

	// Write the data....

	tCloseFile(file);
	return true;
}


tPixel* tImageEXR::StealPixels()
{
	tPixel* pixels = Pixels;
	Pixels = nullptr;
	Width = 0;
	Height = 0;
	return pixels;
}


}
