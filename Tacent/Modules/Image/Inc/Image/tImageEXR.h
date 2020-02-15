// tImageEXR.h
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

#pragma once
#include <Foundation/tString.h>
#include <Math/tColour.h>
namespace tImage
{


class tImageEXR
{
public:
	//inline const static double DefaultGammaCorr		= 2.2;
	//inline const static int DefaultExposureAdj		= 0;

	// Creates an invalid tImageEXR. You must call Load manually.
	tImageEXR()																											{ }
	tImageEXR(const tString& exrFile/*, double gamma = DefaultGammaCorr, int exp = DefaultExposureAdj*/)				{ Load(exrFile/*, gamma, exp*/); }

	// This one sets from a supplied pixel array. It just reads the data (or steals the array if steal set).
	tImageEXR(tPixel* pixels, int width, int height, bool steal = false)												{ Set(pixels, width, height, steal); }

	virtual ~tImageEXR()																								{ Clear(); }

	// Clears the current tImageEXR before loading. If false returned object is invalid.
	bool Load(const tString& exrFile/*, double = DefaultGammaCorr, int = DefaultExposureAdj*/);

	// This one sets from a supplied pixel array.
	bool Set(tPixel* pixels, int width, int height, bool steal = false);

	// Saves the tImageEXR to the exr file specified. The extension of filename must be "exr". Returns success.
	bool Save(const tString& exrFile) const;

	// After this call no memory will be consumed by the object and it will be invalid.
	void Clear();
	bool IsValid() const																								{ return Pixels ? true : false; }

	int GetWidth() const																								{ return Width; }
	int GetHeight() const																								{ return Height; }

	// After this call you are the owner of the pixels and must eventually delete[] them. This tImageEXR object is
	// invalid afterwards.
	tPixel* StealPixels();
	tPixel* GetPixels() const																							{ return Pixels; }

private:
	int Width				= 0;
	int Height				= 0;
	tPixel* Pixels			= nullptr;
};


// Implementation blow this line.


inline void tImageEXR::Clear()
{
	Width = 0;
	Height = 0;
	delete[] Pixels;
	Pixels = nullptr;
}


}



