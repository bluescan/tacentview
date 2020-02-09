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

#include <Foundation/tStandard.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include "Image/tImageHDR.h"
using namespace tSystem;
namespace tImage
{

#define MAXGSHIFT	31		// maximum shift for gamma 
double GammaCorrection = 2.2;
int ExposureAdjustment = 0;
uint8* g_mant = NULL;
uint8* g_nexp = NULL;
uint8 (*g_bval)[256] = NULL;


void SetupGammaTables(double gamma)
{
	if (g_bval)
		return;

	double invGamma = 1.0 / gamma;

	// one direction.
	g_bval = (uint8(*)[256])malloc((MAXGSHIFT+1)*256);
	double mult = 1.0/256.0;
	for (int i = 0; i <= MAXGSHIFT; i++)
	{
		for (int j = 0; j < 256; j++)
			g_bval[i][j] = uint8(uint32( 256.0 * pow((j+.5)*mult, invGamma) ));
		mult *= 0.5;
	}

	// backwards.
	g_mant = (uint8*)malloc(256);
	g_nexp = (uint8*)malloc(256);
	int i = 0;
	mult = 256.0;
	for (int j = 256; j--; )
	{
		while ((g_mant[j] = uint8(uint32(mult * pow((j+.5)/256.0, gamma)))) < 128)
		{
			i++;
			mult *= 2.0;
		}
		g_nexp[j] = i;
	}
}


bool tImageHDR::Load(const tString& hdrFile)
{
	Clear();

	if (tSystem::tGetFileType(hdrFile) != tSystem::tFileType::HDR)
		return false;

	if (!tFileExists(hdrFile))
		return false;

	int numBytes = 0;
	uint8* hdrFileInMemory = tLoadFile(hdrFile, nullptr, &numBytes, true);
	bool success = Set(hdrFileInMemory, numBytes);
	delete[] hdrFileInMemory;

	return success;
}


/////////////

#define RED 0
#define GRN 1
#define BLU 2
#define ALP 3
#define EXP 3
#define tputc(v) *writeP++ = uint8(v)
#define tgetc() (*readP++);
#define tungetc(v) *(--readP) = v;
#define  MINELEN	8	/* minimum scanline length for encoding */
#define  MAXELEN	0x7fff	/* maximum scanline length for encoding */
#define  MINRUN		4	/* minimum run length */
#define  COLXS		128	/* excess used for exponent */

// read in an old-style colr scanline
bool tImageHDR::oldreadcolrs(tPixel  *scanline, int  len)
{
	int  rshift = 0;
	int  i;
	
	while (len > 0)
	{
		scanline[0].E[RED] = tgetc();
		scanline[0].E[GRN] = tgetc();
		scanline[0].E[BLU] = tgetc();
		scanline[0].E[EXP] = i = tgetc();
		if (i == EOF)
			return false;
		if (scanline[0].E[RED] == 1 && scanline[0].E[GRN] == 1 && scanline[0].E[BLU] == 1)
		{
			for (i = scanline[0].E[EXP] << rshift; i > 0; i--)
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


// read in an encoded colr scanline 
bool tImageHDR::freadcolrs(tPixel* scanline, int len)
{
	int  i, j;
	int  code, val;
	
	// determine scanline type
	if ((len < MINELEN) | (len > MAXELEN))
		return oldreadcolrs(scanline, len);

	i = tgetc();
	if (i == EOF)
		return false;
	if (i != 2)
	{
		tungetc(i);
		return oldreadcolrs(scanline, len);
	}
	scanline[0].E[GRN] = tgetc();
	scanline[0].E[BLU] = tgetc();
	i = tgetc();
	if (i == EOF)
		return false;
	if (scanline[0].E[GRN] != 2 || scanline[0].E[BLU] & 128)
	{
		scanline[0].E[RED] = 2;
		scanline[0].E[EXP] = i;
		return oldreadcolrs(scanline+1, len-1);
	}
	if ((scanline[0].E[BLU]<<8 | i) != len)
		return false;		// length mismatch!
	
	// read each component
	for (i = 0; i < 4; i++)
	{
	    for (j = 0; j < len; )
		{
			code = tgetc();
			if (code == EOF)
				return false;
			if (code > 128)
			{	// run
				code &= 127;
				val = tgetc();
				if (val == EOF)
					return false;
				if (j + code > len)
		    		return false;	// overrun 
				while (code--)
					scanline[j++].E[i] = val;
			}
			else
			{		// non-run
				if (j + code > len)
		    		return false;	// overrun
				while (code--)
				{
					val = tgetc();
					if (val == EOF)
						return false;
					scanline[j++].E[i] = val;
				}
			}
	    }
	}
	return true;
}


int colrs_gambs(tPixel* scan, int len)		/* convert scanline of colrs to gamma bytes */
{
	int	i, expo;

	if (g_bval == NULL)
		return(-1);
	while (len-- > 0)
	{
		expo = scan[0].E[EXP] - COLXS;
		if (expo < -MAXGSHIFT)
		{
			if (expo < -MAXGSHIFT-8)
			{
				scan[0].E[RED] =
				scan[0].E[GRN] =
				scan[0].E[BLU] = 0;
			}
			else
			{
				i = (-MAXGSHIFT-1) - expo;
				scan[0].E[RED] = 
				g_bval[MAXGSHIFT][((scan[0].E[RED]>>i)+1)>>1];
				scan[0].E[GRN] =
				g_bval[MAXGSHIFT][((scan[0].E[GRN]>>i)+1)>>1];
				scan[0].E[BLU] =
				g_bval[MAXGSHIFT][((scan[0].E[BLU]>>i)+1)>>1];
			}
		}
		else if (expo > 0)
		{
			if (expo > 8)
			{
				scan[0].E[RED] =
				scan[0].E[GRN] =
				scan[0].E[BLU] = 255;
			} else
			{
				i = (scan[0].E[RED]<<1 | 1) << (expo-1);
				scan[0].E[RED] = i > 255 ? 255 : g_bval[0][i];
				i = (scan[0].E[GRN]<<1 | 1) << (expo-1);
				scan[0].E[GRN] = i > 255 ? 255 : g_bval[0][i];
				i = (scan[0].E[BLU]<<1 | 1) << (expo-1);
				scan[0].E[BLU] = i > 255 ? 255 : g_bval[0][i];
			}
		}
		else
		{
			scan[0].E[RED] = g_bval[-expo][scan[0].E[RED]];
			scan[0].E[GRN] = g_bval[-expo][scan[0].E[GRN]];
			scan[0].E[BLU] = g_bval[-expo][scan[0].E[BLU]];
		}
		scan[0].E[EXP] = COLXS;
		scan++;
	}
	return(0);
}


/////////////

bool tImageHDR::Set(uint8* hdrFileInMemory, int numBytes)
{
	Clear();
	if ((numBytes <= 0) || !hdrFileInMemory)
		return false;

	// Do the work. Set Width, Height, and Colours.
	SetupGammaTables(2.2);

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
		return false;

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
		return false;
	*eolX = '\0';
	tString header((char*)hdrFileInMemory);
	*eolX = '\n';
	readP = (uint8*)(eolX+1);

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

	for (int y = Height-1; y >= 0; y--)
	{
		bool ok = freadcolrs(scanin, Width);
		if (!ok)
		{
			delete[] scanin;
			delete[] Pixels;
			Pixels = nullptr;
			return false;
		}
///		if (bradj)			/* adjust exposure */
///			shiftcolrs(scanin, xmax, bradj);
		colrs_gambs(scanin, Width);	/* gamma correction */

		writeP = (uint8*)&Pixels[y * Width];
		for (int x = 0; x < Width; x++)
		{
			tputc(scanin[x].E[RED]);
			tputc(scanin[x].E[GRN]);
			tputc(scanin[x].E[BLU]);
			tputc(255);
		}
	}
	delete[] scanin;

	return true;
}


bool tImageHDR::Set(tPixel* pixels, int width, int height)
{
	Clear();
	if (!pixels || (width <= 0) || (height <= 0))
		return false;

	// Do the work. Set Width, Height, and Colours.
	Width = 0;
	Height = 0;
	Pixels = nullptr;

	return true;
}


bool tImageHDR::Save(const tString& hdrFile) const
{
	if (!IsValid())
		return false;

	if (tSystem::tGetFileType(hdrFile) != tSystem::tFileType::HDR)
		return false;

	tFileHandle file = tOpenFile(hdrFile.ConstText(), "wb");
	if (!file)
		return false;

	// Write the data....
	tPutc(0, file);

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
