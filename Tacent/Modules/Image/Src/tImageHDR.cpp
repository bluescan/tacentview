// tImageHDR.cpp
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
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The end-user documentation included with the redistribution, if any, must include the following acknowledgment:
//    "This product includes Radiance software (http://radsite.lbl.gov/) developed by the Lawrence Berkeley National Laboratory (http://www.lbl.gov/)."
//    Alternately, this acknowledgment may appear in the software itself, if and wherever such third-party acknowledgments normally appear.
// 4. The names "Radiance," "Lawrence Berkeley National Laboratory" and "The Regents of the University of California" must not be used to endorse
//    or promote products derived from this software without prior written permission. For written permission, please contact radiance@radsite.lbl.gov.
// 5. Products derived from this software may not be called "Radiance", nor may "Radiance" appear in their name, without prior written permission of
//    Lawrence Berkeley National Laboratory.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Lawrence Berkeley National Laboratory OR ITS CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Foundation/tStandard.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include "Image/tImageHDR.h"
using namespace tSystem;
namespace tImage
{


void tImageHDR::SetupGammaTables(float gammaCorr)
{
	double gamma = double(gammaCorr);
	if (GammaTable)
		return;

	double invGamma = 1.0 / gamma;

	// This table is used to convert from Radiance format to 24-bit.
	GammaTable = (uint8(*)[256])malloc((MaxGammaShift+1)*256);
	double mult = 1.0/256.0;
	for (int i = 0; i <= MaxGammaShift; i++)
	{
		for (int j = 0; j < 256; j++)
			GammaTable[i][j] = uint8(uint32( 256.0 * pow( (j+0.5)*mult, invGamma ) ));
		mult *= 0.5;
	}

	// These tables are used to go from 24-bit to a Radiance-encoded value.
	MantissaTable = (uint8*)malloc(256);
	ExponentTable = (uint8*)malloc(256);
	int i = 0;
	mult = 256.0;
	for (int j = 256; j--; )
	{
		while ((MantissaTable[j] = uint8(uint32(mult * pow((j+0.5)/256.0, gamma)))) < 128)
		{
			i++;
			mult *= 2.0;
		}
		ExponentTable[j] = i;
	}
}


void tImageHDR::CleanupGammaTables()
{
	if (MantissaTable) free(MantissaTable);
	MantissaTable = nullptr;
	if (ExponentTable) free(ExponentTable);
	ExponentTable = nullptr;
	if (GammaTable) free(GammaTable);
	GammaTable = nullptr;
}


bool tImageHDR::Load(const tString& hdrFile, float gamma, int exposure)
{
	Clear();

	if (tSystem::tGetFileType(hdrFile) != tSystem::tFileType::HDR)
		return false;

	if (!tFileExists(hdrFile))
		return false;

	int numBytes = 0;
	uint8* hdrFileInMemory = tLoadFile(hdrFile, nullptr, &numBytes, true);
	bool success = Set(hdrFileInMemory, numBytes, gamma, exposure);
	delete[] hdrFileInMemory;

	return success;
}


bool tImageHDR::LegacyReadRadianceColours(tPixel* scanline, int len)
{
	int  rshift = 0;
	int  i;
	
	while (len > 0)
	{
		scanline[0].R = GetB();
		scanline[0].G = GetB();
		scanline[0].B = GetB();
		scanline[0].A = i = GetB();
		if (i == EOF)
			return false;
		if (scanline[0].R == 1 && scanline[0].G == 1 && scanline[0].B == 1)
		{
			for (i = scanline[0].A << rshift; i > 0; i--)
			{
				scanline[0] = scanline[-1];
				scanline++;
				len--;
			}
			rshift += 8;
		}
		else
		{
			scanline++;
			len--;
			rshift = 0;
		}
	}
	return true;
}


bool tImageHDR::ReadRadianceColours(tPixel* scanline, int len)
{
	int  i, j;
	int  code, val;
	
	// Determine if scanline is legacy and needs to be processed the old way.
	if ((len < MinScanLen) | (len > MaxScanLen))
		return LegacyReadRadianceColours(scanline, len);

	i = GetB();
	if (i == EOF)
		return false;
	if (i != 2)
	{
		UngetB(i);
		return LegacyReadRadianceColours(scanline, len);
	}
	scanline[0].G = GetB();
	scanline[0].B = GetB();
	i = GetB();
	if (i == EOF)
		return false;

	if (scanline[0].G != 2 || scanline[0].B & 128)
	{
		scanline[0].R = 2;
		scanline[0].A = i;
		return LegacyReadRadianceColours(scanline+1, len-1);
	}
	if ((scanline[0].B << 8 | i) != len)
		return false;
	
	// Read each component.
	for (i = 0; i < 4; i++)
	{
	    for (j = 0; j < len; )
		{
			code = GetB();
			if (code == EOF)
				return false;

			if (code > 128)
			{
				// RLE run.
				code &= 127;
				val = GetB();
				if (val == EOF)
					return false;
				if (j + code > len)
		    		return false;	// Overrun.
				while (code--)
					scanline[j++].E[i] = val;
			}
			else
			{
				// New non-RLE colour.
				if (j + code > len)
		    		return false;	// Overrun.
				while (code--)
				{
					val = GetB();
					if (val == EOF)
						return false;
					scanline[j++].E[i] = val;
				}
			}
	    }
	}
	return true;
}


bool tImageHDR::ConvertRadianceToGammaCorrected(tPixel* scan, int len)
{
	if (!GammaTable)
		return false;

	while (len-- > 0)
	{
		int expo = scan[0].A - ExpXS;
		if (expo < -MaxGammaShift)
		{
			if (expo < -MaxGammaShift-8)
			{
				scan[0].MakeBlack();
			}
			else
			{
				int i = (-MaxGammaShift-1) - expo;
				scan[0].R = GammaTable[MaxGammaShift][ ((scan[0].R >> i) + 1) >> 1 ];
				scan[0].G = GammaTable[MaxGammaShift][ ((scan[0].G >> i) + 1) >> 1 ];
				scan[0].B = GammaTable[MaxGammaShift][ ((scan[0].B >> i) + 1) >> 1 ];
			}
		}
		else if (expo > 0)
		{
			if (expo > 8)
			{
				scan[0].MakeWhite();
			}
			else
			{
				int i;
				i = (scan[0].R<<1 | 1) << (expo-1);		scan[0].R = i > 255 ? 255 : GammaTable[0][i];
				i = (scan[0].G<<1 | 1) << (expo-1);		scan[0].G = i > 255 ? 255 : GammaTable[0][i];
				i = (scan[0].B<<1 | 1) << (expo-1);		scan[0].B = i > 255 ? 255 : GammaTable[0][i];
			}
		}
		else
		{
			scan[0].R = GammaTable[-expo][scan[0].R];
			scan[0].G = GammaTable[-expo][scan[0].G];
			scan[0].B = GammaTable[-expo][scan[0].B];
		}
		scan[0].A = ExpXS;
		scan++;
	}
	return true;
}


void tImageHDR::AdjustExposure(tPixel* scan, int len, int adjust)
{
	// Shift a scanline of colors by 2^adjust.
	if (adjust == 0)
		return;

	int minexp = adjust < 0 ? -adjust : 0;
	while (len-- > 0)
	{
		if (scan[0].A <= minexp)
			scan[0].MakeZero();
		else
			scan[0].A += adjust;
		scan++;
	}
}


bool tImageHDR::Set(uint8* hdrFileInMemory, int numBytes, float gammaCorr, int exposureAdj)
{
	Clear();
	if ((numBytes <= 0) || !hdrFileInMemory)
		return false;

	SetupGammaTables(gammaCorr);

	// Search for the first double 0x0A (linefeed).
	// Note that hdrFileInMemory has an extra EOF at the end. The (numBytes+1)th character.
	int doubleLFIndex = -1;
	for (int c = 0; c < numBytes; c++)
	{
		if ((hdrFileInMemory[c] == 0x0A) && (hdrFileInMemory[c+1] == 0x0A))
		{
			doubleLFIndex = c;
			break;
		}
	}
	if (doubleLFIndex == -1)
	{
		CleanupGammaTables();
		return false;
	}

	// We are not allowed any '\0' characters in the header. Some Mac-generated images have one!
	for (int c = 0; c < doubleLFIndex; c++)
	{
		if (hdrFileInMemory[c] == '\0')
			hdrFileInMemory[c] = '_';
	}

	char* foundY = tStd::tStrstr((char*)hdrFileInMemory, "-Y");
	char* foundX = tStd::tStrstr((char*)hdrFileInMemory, "+X");
	char* eolY = tStd::tStrchr(foundY, '\n');
	char* eolX = tStd::tStrchr(foundX, '\n');
	if (!eolX || (eolX != eolY))
	{
		CleanupGammaTables();
		return false;
	}
	*eolX = '\0';
	tString header((char*)hdrFileInMemory);
	*eolX = '\n';
	ReadP = (uint8*)(eolX+1);

	tList<tStringItem> lines;
	tStd::tExplode(lines, header, '\n');

	// Display the header lines.
	for (tStringItem* headerLine = lines.First(); headerLine; headerLine = headerLine->Next())
		tPrintf("HDR Info: %s\n", headerLine->Chars());

	tStringItem* resLine = lines.Last();

	tList<tStringItem> comps;
	tStd::tExplode(comps, *resLine, ' ');
	Height = comps.First()->Next()->AsInt();
	Width = comps.First()->Next()->Next()->Next()->AsInt();

	Pixels = new tPixel[Width*Height];
	tPixel* scanin = new tPixel[Width];

	bool ok = true;
	for (int y = Height-1; y >= 0; y--)
	{
		ok = ReadRadianceColours(scanin, Width);
		if (!ok)
			break;

		AdjustExposure(scanin, Width, exposureAdj);

		ok = ConvertRadianceToGammaCorrected(scanin, Width);
		if (!ok)
			break;

		WriteP = (uint8*)&Pixels[y * Width];
		for (int x = 0; x < Width; x++)
		{
			PutB(scanin[x].R);
			PutB(scanin[x].G);
			PutB(scanin[x].B);
			PutB(255);
		}
	}

	CleanupGammaTables();
	delete[] scanin;
	if (!ok)
	{
		delete[] Pixels;
		Pixels = nullptr;
		return false;
	}

	return true;
}


bool tImageHDR::Set(tPixel* pixels, int width, int height, bool steal)
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


bool tImageHDR::Save(const tString& hdrFile) const
{
	tAssertMsg(false, "HDR Save not implemented.");
	if (!IsValid())
		return false;

	if (tSystem::tGetFileType(hdrFile) != tSystem::tFileType::HDR)
		return false;

	tFileHandle file = tOpenFile(hdrFile.ConstText(), "wb");
	if (!file)
		return false;

	// Write the data....

	tCloseFile(file);
	return true;
}


tPixel* tImageHDR::StealPixels()
{
	tPixel* pixels = Pixels;
	Pixels = nullptr;
	Width = 0;
	Height = 0;
	return pixels;
}


}
