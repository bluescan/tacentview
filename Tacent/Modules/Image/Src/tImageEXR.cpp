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
//
// The EXR namespace and functions are a modification of ImageView.cpp from OpenEXR under the following licence:
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucas Digital Ltd. LLC. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// * Neither the name of Industrial Light & Magic nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Foundation/tStandard.h>
#include <Foundation/tString.h>
#include <System/tMachine.h>
#include <System/tFile.h>
#include <OpenEXR/loadImage.h>
#include <OpenEXR/IlmImf/ImfMultiPartInputFile.h>
#include <halfFunction.h>
#include "Image/tImageEXR.h"
using namespace tSystem;
using namespace IMF;
using namespace IMATH;


namespace EXR
{
	inline float KneeFun(double x, double f);
	float FindKneeFun(float x, float y);
	uint8 Dither(float v, int x, int y);

	// This fog colour is used when de-fogging.
	void ComputeFogColour(float& fogR, float& fogG, float& fogB, const IMF::Array<IMF::Rgba>& pixels);

	struct Gamma
	{
		Gamma(float gamma, float exposure, float defog, float kneeLow, float kneeHigh);
		float operator()(half h);
		float invg, m, d, kl, f, s;
	};
}


float EXR::KneeFun(double x, double f)
{
	return float (IMATH::Math<double>::log(x * f + 1.0) / f);
}

	
float EXR::FindKneeFun(float x, float y)
{
	float f0 = 0; float f1 = 1;
	while (KneeFun(x, f1) > y)
	{
		f0 = f1;
		f1 = f1 * 2;
	}

	for (int i = 0; i < 30; ++i)
	{
		float f2 = (f0 + f1) / 2.0f;
		float y2 = KneeFun(x, f2);
		if (y2 < y)
			f1 = f2;
		else
			f0 = f2;
	}

	return (f0 + f1) / 2.0f;
}


void EXR::ComputeFogColour(float& fogR, float& fogG, float& fogB, const IMF::Array<IMF::Rgba>& pixels)
{
	double fogRd = 0.0;
	double fogGd = 0.0;
	double fogBd = 0.0;
	double WeightSum = 0.0;
	int numPixels = pixels.size();

	for (int j = 0; j < numPixels; ++j)
	{
		const IMF::Rgba& rawPixel = pixels[j];
		float weight = tMath::tSaturate(float(rawPixel.a));		// Makes sure transparent pixels don't contribute to the colour.

		fogRd += rawPixel.r.isFinite() ? double(rawPixel.r) : 0.0;
		fogGd += rawPixel.g.isFinite() ? double(rawPixel.g) : 0.0;
		fogBd += rawPixel.b.isFinite() ? double(rawPixel.b) : 0.0;
		WeightSum += weight;
	}

	fogRd /= WeightSum;		fogGd /= WeightSum;		fogBd /= WeightSum;
	fogR = float(fogRd);	fogG = float(fogGd);	fogB = float(fogBd);
}


uint8 EXR::Dither(float v, int x, int y)
{
	static const float d[4][4] =
	{
		{	00.0f/16.0f,	08.0f/16.0f,	02.0f/16.0f,	10.0f/16.0f		},
		{	12.0f/16.0f,	04.0f/16.0f,	14.0f/16.0f,	06.0f/16.0f		},
		{	03.0f/16.0f,	11.0f/16.0f,	01.0f/16.0f,	09.0f/16.0f		},
		{	15.0f/16.0f,	07.0f/16.0f,	13.0f/16.0f,	05.0f/16.0f		}
	};

	return uint8(v + d[y&3][x&3]);
}


EXR::Gamma::Gamma(float gamma, float exposure, float defog, float kneeLow, float kneeHigh) :
	invg(1.0f/gamma),
	m(tMath::tPow(2.0f, exposure + 2.47393f)),
	d(defog),
	kl(tMath::tPow(2.0f, kneeLow)),
	f(FindKneeFun(tMath::tPow(2.0f, kneeHigh) - kl, tMath::tPow(2.0f, 3.5f) - kl)),
	s(255.0f * tMath::tPow(2.0f, -3.5f * invg))
{
}


float EXR::Gamma::operator()(half h)
{
	float x = tMath::tMax(0.0f, (h - d));		// Defog
	x *= m;										// Exposure
	if (x > kl) x = kl + KneeFun(x - kl, f);	// Knee
	x = tMath::tPow(x, invg);					// Gamma
	return tMath::tClamp(x*s, 0.0f, 255.0f);	// Clamp
}


bool tImage::tImageEXR::Load
(
	const tString& exrFile, int partNum, float gamma, float exposure,
	float defog, float kneeLow, float kneeHigh
)
{
	Clear();
	if (tSystem::tGetFileType(exrFile) != tSystem::tFileType::EXR)
		return false;

	if (!tFileExists(exrFile))
		return false;

	// Leave two cores free unless we are on a three core or lower machine, in which case we always use a min of 2 threads.
	int numThreads = tMath::tClampMin((tSystem::tGetNumCores()) - 2, 2);
	setGlobalThreadCount(numThreads);

	int numParts = 0;
	int outZsize = 0;
	Header outHeader;
	IMF::Array<IMF::Rgba> pixels;
	IMF::Array<float*> zbuffer;
	IMF::Array<uint> sampleCount;

	try
	{
		MultiPartInputFile mpfile(exrFile.Chars());
		numParts = mpfile.parts();
		if ((numParts <= 0) || (partNum >= numParts))
			return false;

		// const char* channels = "AZRGB";
		// const char* layers = "0";
		bool preview = false;
		int lx = -1; int ly = -1;		// For tiled image shows level (lx,ly)
		bool compositeDeep = true;

		EXR::loadImage
		(
			exrFile.Chars(),
			nullptr,			// Channels. Null means all.
			nullptr,			// Layers. O means first one.
			preview, lx, ly,
			partNum,
			outZsize, outHeader,
			pixels, zbuffer, sampleCount,
			compositeDeep
		);
	}
	catch (IEX_NAMESPACE::BaseExc& err)
	{
		tPrintf("Error: Can't read exr file. %s\n", err.what());
		return false;
	}

	const Box2i& displayWindow = outHeader.displayWindow();
	const Box2i& dataWindow = outHeader.dataWindow();
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

	// Map floating-point pixel values 0.0 and 1.0 to the display's white and black respectively.
	// if bool zerooneexposure true.
	// exposure = 1.02607f;
	// kneeHigh = 3.5f;
	
	float fogR = 0.0f;
	float fogG = 0.0f;
	float fogB = 0.0f;

	// Save some time if we can.
	if (defog > 0.0f)
		EXR::ComputeFogColour(fogR, fogG, fogB, pixels);

	halfFunction<float> redGamma(EXR::Gamma(gamma, exposure, defog * fogR, kneeLow, kneeHigh), -HALF_MAX, HALF_MAX, 0.0f, 255.0f, 0.0f, 0.0f);
	halfFunction<float> grnGamma(EXR::Gamma(gamma, exposure, defog * fogG, kneeLow, kneeHigh), -HALF_MAX, HALF_MAX, 0.0f, 255.0f, 0.0f, 0.0f);
	halfFunction<float> bluGamma(EXR::Gamma(gamma, exposure, defog * fogB, kneeLow, kneeHigh), -HALF_MAX, HALF_MAX, 0.0f, 255.0f, 0.0f, 0.0f);

	// Conversion from raw pixel data to data for the OpenGL frame buffer:
	// 1) Compensate for fogging by subtracting defog from the raw pixel values.
	// 2) Multiply the defogged pixel values by 2^(exposure + 2.47393).
	// 3) Values that are now 1.0 are called "middle gray". If defog and exposure are both set to 0.0, then middle gray
	//    corresponds to a raw pixel value of 0.18. In step 6, middle gray values will be mapped to an intensity 3.5
	//    f-stops below the display's maximum intensity.
	// 4) Apply a knee function. The knee function has two parameters, kneeLow and kneeHigh. Pixel values below
	//    2^kneeLow are not changed by the knee function. Pixel values above kneeLow are lowered according to a
	//    logarithmic curve, such that the value 2^kneeHigh is mapped to 2^3.5. (In step 6 this value will be mapped to
	//    the the display's maximum intensity.)
	// 5) Gamma-correct the pixel values, according to the screen's gamma. (We assume that the gamma curve is a simple
	//    power function.)
	// 6) Scale the values such that middle gray pixels are mapped to a frame buffer value that is 3.5 f-stops below the
	//    display's maximum intensity. (84.65 if the screen's gamma is 2.2)
	// 7) Clamp the values to [0, 255].
	//
	// Texview has 0,0 at bottom-left. Rows start from bottom.
	int p = 0;
	for (int yi = Height-1; yi >= 0; yi--)
	{
		for (int xi = 0; xi < Width; xi++)
		{
			int idx = yi*Width + xi;
			const IMF::Rgba& rawPixel = pixels[idx];
			Pixels[p++] = tPixel
			(
				EXR::Dither( redGamma(rawPixel.r), xi, yi ),
				EXR::Dither( grnGamma(rawPixel.g), xi, yi ),
				EXR::Dither( bluGamma(rawPixel.b), xi, yi ),
				uint8( tMath::tClamp( tMath::tFloatToInt(float(rawPixel.a)*255.0f), 0, 0xFF ) )
			);
		}
	}

	SrcPixelFormat = tPixelFormat::HDR_EXR;
	return true;
}


bool tImage::tImageEXR::Set(tPixel* pixels, int width, int height, bool steal)
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

	SrcPixelFormat = tPixelFormat::R8G8B8A8;
	return true;
}


bool tImage::tImageEXR::Save(const tString& exrFile) const
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


tPixel* tImage::tImageEXR::StealPixels()
{
	tPixel* pixels = Pixels;
	Pixels = nullptr;
	Width = 0;
	Height = 0;
	return pixels;
}
