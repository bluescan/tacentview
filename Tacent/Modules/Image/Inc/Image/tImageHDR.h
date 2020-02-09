// tImageHDR.h
//
// This knows how to load and save a Radiance High Dynamic Range image (.hdr or .rgbe). It knows the details of the hdr
// file format and loads the data into a tPixel array. These tPixels may be 'stolen' by the tPicture's constructor if
// an HDR file is specified. After the array is stolen the tImageHDR is invalid. This is purely for performance.
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
// The conversion code for hdr data is a modified version of code from Radiance. Here is the licence.
//
// The Radiance Software License, Version 1.0
// Copyright (c) 1990 - 2015 The Regents of the University of California, through Lawrence Berkeley National Laboratory. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. The end-user documentation included with the redistribution, if any, must include the following acknowledgment:
//    "This product includes Radiance software (http://radsite.lbl.gov/) developed by the Lawrence Berkeley National Laboratory (http://www.lbl.gov/)."
//    Alternately, this acknowledgment may appear in the software itself, if and wherever such third-party acknowledgments normally appear.
// 4. The names "Radiance," "Lawrence Berkeley National Laboratory" and "The Regents of the University of California" must not be used to endorse or promote products derived from this software without prior written permission. For written permission, please contact radiance@radsite.lbl.gov.
// 5. Products derived from this software may not be called "Radiance", nor may "Radiance" appear in their name, without prior written permission of Lawrence Berkeley National Laboratory.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL Lawrence Berkeley National Laboratory OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <Foundation/tString.h>
#include <Math/tColour.h>
namespace tImage
{


class tImageHDR
{
public:
	// Creates an invalid tImageHDR. You must call Load manually.
	tImageHDR()																											{ }
	tImageHDR(const tString& hdrFile)																					{ Load(hdrFile); }

	// hdrFileInMemory can be deleted after this runs.
	tImageHDR(uint8* hdrFileInMemory, int numBytes)																{ Set(hdrFileInMemory, numBytes); }

	// This one sets from a supplied pixel array. It just reads the data.
	tImageHDR(tPixel* pixels, int width, int height)																	{ Set(pixels, width, height); }

	virtual ~tImageHDR()																								{ Clear(); }

	// Clears the current tImageHDR before loading. If false returned object is invalid.
	bool Load(const tString& hdrFile);
	bool Set(uint8* hdrFileInMemory, int numBytes);

	// This one sets from a supplied pixel array.
	bool Set(tPixel* pixels, int width, int height);

	// Saves the tImageHDR to the hdr file specified. The extension of filename must be "hdr". Returns success.
	bool Save(const tString& hdrFile) const;

	// After this call no memory will be consumed by the object and it will be invalid.
	void Clear();
	bool IsValid() const																								{ return Pixels ? true : false; }

	int GetWidth() const																								{ return Width; }
	int GetHeight() const																								{ return Height; }

	// After this call you are the owner of the pixels and must eventually delete[] them. This tImageTGA object is
	// invalid afterwards.
	tPixel* StealPixels();
	tPixel* GetPixels() const																							{ return Pixels; }

private:

	bool oldreadcolrs(tPixel  *scanline, int  len);
	bool freadcolrs(tPixel  *scanline, int len);

	int Width = 0;
	int Height = 0;
	tPixel* Pixels = nullptr;

	uint8* readP = nullptr;
	uint8* writeP = nullptr;
};


// Implementation blow this line.


inline void tImageHDR::Clear()
{
	Width = 0;
	Height = 0;
	delete[] Pixels;
	Pixels = nullptr;
}


}



