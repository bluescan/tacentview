// tPixelFormat.h
//
// Pixel formats in Tacent. Not all formats are fully supported. Certainly BC 4, 5, and 7 may not have extensive HW
// support at this time.
//
// Copyright (c) 2004-2006, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
namespace tImage
{


// Unlike DirectX, which assumes all machines are little-endian, the enumeration below specifies the components in the
// order they appear in memory. To make matters worse, Microsoft's R8G8B8 format is B8R8G8 in memory, but their R5G6B5
// format is correctly the same order in memory. So a little inconsistent there. BC stands for Block Compression.
enum class tPixelFormat
{
	Invalid				= -1,
	Auto				= Invalid,

	FirstNormal,
	R8G8B8				= FirstNormal,	// 24 bit. Full colour. No alpha. Matches GL_BGR ordering.
	R8G8B8A8,							// 32 bit. Full alpha. Matches GL_BGRA ordering.
	B8G8R8,								// 24 bit. Full colour. No alpha. Matches GL_RGB ordering.
	B8G8R8A8,							// 32 bit. Full alpha. Matches GL_RGBA ordering.
	G3B5A1R5G2,							// 16 bit. 15 colour. Binary alpha. First bits are the low order ones.
	G4B4A4R4,							// 16 bit. 12 colour. 4 bit alpha.
	G3B5R5G3,							// 16 bit. No alpha. The first 3 green bits are the low order ones.
	L8A8,								// 16 bit. Luminance and alpha.
	R32F,
	G32R32F,
	A32B32G32R32F,
	LastNormal			= A32B32G32R32F,

	FirstBlock,
	BC1_DXT1			= FirstBlock,	// BC 1, DXT1. No alpha.
	BC1_DXT1BA,							// BC 1, DXT1. Binary alpha.
	BC2_DXT3,							// BC 2, DXT3. Large alpha gradients (outlines).
	BC3_DXT5,							// BC 3, DXT5. Variable alpha.
	BC4_ATI1,							// BC 4. One colour channel only. May not be HW supported.
	BC5_ATI2,							// BC 5. Two colour channels only. May not be HW supported.
	BC6H,								// BC 6 HDR. No alpha. 3 x 16bit half-floats.
	BC7,								// BC 7. Full colour. Variable alpha 0 to 8 bits.
	LastBlock			= BC7,

	FirstHDR,
	HDR_RAD				= FirstHDR,		// Radiance hdr.
	HDR_EXR,							// OpenEXR.
	LastHDR				= HDR_EXR,

	FirstPAL,
	PAL_8BIT			= FirstPAL,		// 8bit indexes to a Palette. ex. gif files.
	LastPAL				= PAL_8BIT,

	NumPixelFormats,
	NumNormalFormats	= LastNormal - FirstNormal + 1,
	NumBlockFormats		= LastBlock - FirstBlock + 1,
	NumHDRFormats		= LastHDR - FirstHDR + 1,
	NumPALFormats		= LastPAL - FirstPAL + 1
};


bool tIsNormalFormat(tPixelFormat);
bool tIsBlockFormat(tPixelFormat);
bool tIsHDRFormat(tPixelFormat);
bool tIsPaletteFormat(tPixelFormat);
int tGetBitsPerPixel(tPixelFormat);				// Some formats (dxt1) are only half a byte per pixel, so we report bits.
int tGetBytesPer4x4PixelBlock(tPixelFormat);	// This function must be given a BC pixel format.
const char* tGetPixelFormatName(tPixelFormat);


}