// tImageICO.cpp
//
// This class knows how to load windows icon (ico) files. It loads the data into multiple tPixel arrays, one for each
// part (ico files may be multiple images at different resolutions). These arrays may be 'stolen' by tPictures. The
// loading code is a modificaton of code from Victor Laskin. In particular the code now:
// a) Loads all parts of an ico, not just the biggest one.
// b) Supports embedded png images.
// c) Supports widths and heights of 256.
// Victor Laskin's header/licence in the original ico.cpp is shown below.
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
// Includes modified version of code from Victor Laskin.
// Code by Victor Laskin (victor.laskin@gmail.com)
// Rev 2 - 1bit color was added, fixes for bit mask.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <Foundation/tStandard.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include "Image/tImageICO.h"
#include <CxImage/ximage.h>
using namespace tSystem;
namespace tImage
{

	
// These structs represent how the icon information is stored in an ICO file.
struct IconDirEntry
{
	uint8				Width;			// Width of the image.
	uint8				Height;			// Height of the image (times 2).
	uint8				ColorCount;		// Number of colors in image (0 if >=8bpp).
	uint8				Reserved;
	uint16				Planes;			// Colour planes.
	uint16				BitCount;		// Bits per pixel.
	uint32				BytesInRes;		// How many bytes in this resource?
	uint32				ImageOffset;	// Where in the file is this image.
};


struct IconDir
{
	uint16				Reserved;
	uint16				Type;			// Resource type (1 for icons).
	uint16				Count;			// How many images?
	//IconDirEntrys follow. One for each image.
};


// size - 40 bytes
struct BitmapInfoHeader
{
	uint32				Size;
	uint32				Width;
	uint32				Height;			// Icon Height (added height of XOR-Bitmap and AND-Bitmap).
	uint16				Planes;
	uint16				BitCount;
	uint32				Compression;
	int32				SizeImage;
	uint32				XPelsPerMeter;
	uint32				YPelsPerMeter;
	uint32				ClrUsed;
	uint32				ClrImportant;
};


struct IconImage
{
	BitmapInfoHeader	Header;			// DIB header.
	uint32				Colours[1];		// Color table (short 4 bytes) //RGBQUAD.
	uint8				XOR[1];			// DIB bits for XOR mask.
	uint8				AND[1];			// DIB bits for AND mask.
};
	

bool tImageICO::Load(const tString& icoFile)
{
	Clear();

	if (tGetFileType(icoFile) != tFileType::ICO)
		return false;

	if (!tFileExists(icoFile))
		return false;

	int numBytes = 0;
	uint8* icoFileInMemory = tLoadFile(icoFile, nullptr, &numBytes);
	bool success = PopulateParts(icoFileInMemory, numBytes);	
	delete[] icoFileInMemory;

	return success;
}


bool tImageICO::PopulateParts(const uint8* buffer, int numBytes)
{
	IconDir* icoDir = (IconDir*)buffer;
	int iconsCount = icoDir->Count;

	if (icoDir->Reserved != 0)
		return false;
		
	if (icoDir->Type != 1)
		return false;

	if (iconsCount == 0)
		return false;
		
	if (iconsCount > 20)
		return false;

	const uint8* cursor = buffer;
	cursor += 6;
	IconDirEntry* dirEntry = (IconDirEntry*)cursor;

	for (int i = 0; i < iconsCount; i++)
	{
		int w = dirEntry->Width;
		if (w == 0)
			w = 256;
			
		int h = dirEntry->Height;
		if (h == 0)
			h = 256;
			
		int offset = dirEntry->ImageOffset;
		if (!offset || (offset >= numBytes))
			continue;
			
		Part* newPart = CreatePart(buffer+offset, w, h, numBytes);
		if (!newPart)
			continue;

		Parts.Append(newPart);
		dirEntry++;
	}

	return !Parts.IsEmpty();
}


tImageICO::Part* tImageICO::CreatePart(const uint8* cursor, int width, int height, int numBytes)
{
	IconImage* icon = (IconImage*)cursor;
	
	if (icon->Header.Size == 0x474e5089)
	{
		CxImage image;
		bool cxok = image.Decode((uint8_t*)cursor, uint32_t(numBytes), CXIMAGE_FORMAT_PNG);
		if (!cxok)
			return nullptr;

		width = image.GetWidth();
		height = image.GetHeight();
		if (!image.IsValid() || (width <= 0) || (height <= 0))
			return nullptr;

		tPixel* pixels = new tPixel[width*height];

		// CxImage alpha oddness. If we request the alpha using GetPixelColor and there is no alpha channel, it returns 0
		// for the alpha, which is incorrect as alpha is normally interpreted as opacity, not transparency. It should be
		// returning full opacity. That's why we need imageHasValidAlphas -- so we can check if the channel exists at all.
		bool imageHasValidAlphas = image.AlphaIsValid();

		int index = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				RGBQUAD rgba = image.GetPixelColor(x, y);
				tColouri colour;
				colour.R = rgba.rgbRed;
				colour.G = rgba.rgbGreen;
				colour.B = rgba.rgbBlue;

				if (imageHasValidAlphas)
					colour.A = rgba.rgbReserved;
				else
					colour.A = 255;

				pixels[index++] = colour;
			}
		}
		
		Part* newPart = new Part;
		newPart->SrcPixelFormat = imageHasValidAlphas ? tPixelFormat::R8G8B8A8 : tPixelFormat::R8G8B8;
		newPart->Width = width;
		newPart->Height = height;
		newPart->Pixels = pixels;
		return newPart;
	}
	
	int realBitsCount = int(icon->Header.BitCount);
	bool hasAndMask = (realBitsCount < 32) && (height != icon->Header.Height);

	cursor += 40;
	int numPixels = width * height;
	
	tPixel* pixels = new tPixel[numPixels];
	uint8* image = (uint8*)pixels;
	tPixelFormat srcPixelFormat = tPixelFormat::Invalid;
	
	// rgba + vertical swap
	if (realBitsCount == 32)
	{
		srcPixelFormat = tPixelFormat::R8G8B8A8;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int shift = 4 * (x + y * width);
				
				// Rows from bottom to top.
				// int shift2 = 4 * (x + (height - y - 1) * width);
				int shift2 = 4 * (x + y * width);
				
				image[shift] = cursor[shift2 +2];
				image[shift+1] = cursor[shift2 +1];
				image[shift+2] = cursor[shift2 ];
				image[shift+3] = cursor[shift2 +3];
			}
		}
	}

	if (realBitsCount == 24)
	{
		srcPixelFormat = tPixelFormat::R8G8B8;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int shift = 4 * (x + y * width);
				
				// Rows from bottom to top.
				// int shift2 = 3 * (x + (height - y - 1) * width);
				int shift2 = 3 * (x + y * width);
				
				image[shift] = cursor[shift2 +2];
				image[shift+1] = cursor[shift2 +1];
				image[shift+2] = cursor[shift2 ];
				image[shift+3] = 255;
			}
		}
	}

	if (realBitsCount == 8)
	{
		// 256 colour palette.
		srcPixelFormat = tPixelFormat::PAL_8BIT;
		
		uint8* colors = (uint8*)cursor;
		cursor += 256 * 4;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int shift = 4 * (x + y * width);
				
				// Rows from bottom to top.
				// int shift2 = (x + (height - y - 1) * width);
				int shift2 = (x + y * width);
				
				int index = 4 * cursor[shift2];
				image[shift] = colors[index + 2];
				image[shift+1] = colors[index + 1];
				image[shift+2] = colors[index ];
				image[shift+3] = 255;
			}
		}
	}

	if (realBitsCount == 4)
	{
		// 16 colour palette.
		srcPixelFormat = tPixelFormat::PAL_4BIT;
		
		uint8* colors = (uint8*)cursor;
		cursor += 16 * 4;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int shift = 4 * (x + y * width);

				// Rows from bottom to top.
				// int shift2 = (x + (height - y - 1) * width);
				int shift2 = (x + y * width);

				uint8 index = cursor[shift2 / 2];
				if (shift2 % 2 == 0)
					index = (index >> 4) & 0xF;
				else
					index = index & 0xF;
				index *= 4;

				image[shift] = colors[index + 2];
				image[shift+1] = colors[index + 1];
				image[shift+2] = colors[index ];
				image[shift+3] = 255;
			}
		}
	}

	if (realBitsCount == 1)
	{
		// 2 colour palette.
		srcPixelFormat = tPixelFormat::PAL_1BIT;

		uint8* colors = (uint8*)cursor;
		cursor += 2 * 4;

		int boundary = width; //!!! 32 bit boundary (http://www.daubnet.com/en/file-format-ico)
		while (boundary % 32 != 0)
			boundary++;

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int shift = 4 * (x + y * width);

				// Rows from bottom to top.
				// int shift2 = (x + (height - y - 1) * boundary);
				int shift2 = (x + y * boundary);
				
				uint8 index = cursor[shift2 / 8];

				// select 1 bit only
				uint8 bit = 7 - (x % 8);
				index = (index >> bit) & 0x01;
				index *= 4;
				
				image[shift] = colors[index + 2];
				image[shift+1] = colors[index + 1];
				image[shift+2] = colors[index ];
				image[shift+3] = 255;
			}
		}
	}

	// Read AND mask after base color data - 1 BIT MASK
	if (hasAndMask)
	{
		int boundary = width * realBitsCount; //!!! 32 bit boundary (http://www.daubnet.com/en/file-format-ico)
		while (boundary % 32 != 0)
			boundary++;
		cursor += boundary * height / 8;

		boundary = width;
		while (boundary % 32 != 0)
			boundary++;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int shift = 4 * (x + y * width) + 3;
				uint8 bit = 7 - (x % 8);

				// Rows from bottom to top.
				// int shift2 = (x + (height - y - 1) * boundary) / 8;
				int shift2 = (x + y * boundary) / 8;

				int mask = (0x01 & ((unsigned char)cursor[shift2] >> bit));
				image[shift] *= 1 - mask;
			}
		}
	}
	
	Part* newPart = new Part;
	newPart->SrcPixelFormat = srcPixelFormat;
	newPart->Width = width;
	newPart->Height = height;
	newPart->Pixels = pixels;

	return newPart;
}


}
