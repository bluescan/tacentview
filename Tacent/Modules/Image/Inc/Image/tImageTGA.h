// tImageTGA.h
//
// This clas knows how to load and save a targa (.tga) file.
// It does zero processing of image data. It knows the details of the tga file format and loads the
// data into a tPixel array. These tPixels may be 'stolen' by the tPicture's constructor if a targa file is specified.
// After the array is stolen the tImageTGA is invalid. This is purely for performance.
//
// Copyright (c) 2006, 2017, 2019, 2020 Tristan Grimmer.
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
#include <Image/tPixelFormat.h>
namespace tImage
{


class tImageTGA
{
public:
	// Creates an invalid tImageTGA. You must call Load manually.
	tImageTGA()																											{ }
	tImageTGA(const tString& tgaFile)																					{ Load(tgaFile); }

	// The data is copied out of tgaFileInMemory. Go ahead and delete after if you want.
	tImageTGA(const uint8* tgaFileInMemory, int numBytes)																{ Set(tgaFileInMemory, numBytes); }

	// This one sets from a supplied pixel array. If steal is true it takes ownership of the pixels pointer. Otherwise
	// it just copies the data out.
	tImageTGA(tPixel* pixels, int width, int height, bool steal = false)													{ Set(pixels, width, height, steal); }

	virtual ~tImageTGA()																									{ Clear(); }

	// Clears the current tImageTGA before loading. 16, 24, or 32 bit targas can be loaded. The tga may be uncompressed
	// or RLE compressed. Other compression methods are rare and unsupported. Returns success. If false returned,
	// object is invalid.
	bool Load(const tString& tgaFile);
	bool Set(const uint8* tgaFileInMemory, int numBytes);

	// This one sets from a supplied pixel array. If steal is true it takes ownership of the pixels pointer. Otherwise
	// it just copies the data out.
	bool Set(tPixel* pixels, int width, int height, bool steal = false);

	enum class tFormat
	{
		Invalid,	// Invalid must be 0.
		Auto,		// Save function will decide format. Bit24 if all image pixels are opaque and Bit32 otherwise.
		Bit24,		// 24 bit colour.
		Bit32		// 24 bit colour with 8 bits opacity in the alpha channel.
	};
	enum class tCompression
	{
		None,		// No compression.
		RLE			// Run Length Encoding.
	};

	// Saves the tImageTGA to the Targa file specified. The extension of filename must be ".tga". If TargaFormat is Auto,
	// this function will decide the format. Bit24 if all image pixels are opaque and Bit32 otherwise. Returns the format
	// that the file was saved in, or tFormat::Invalid if there was a problem. Since Invalid is 0, you can use an 'if'.
	tFormat Save(const tString& tgaFile, tFormat = tFormat::Auto, tCompression = tCompression::RLE) const;

	// After this call no memory will be consumed by the object and it will be invalid.
	void Clear();
	bool IsValid() const																								{ return Pixels ? true : false; }

	int GetWidth() const																								{ return Width; }
	int GetHeight() const																								{ return Height; }

	// All pixels must be opaque (alpha = 1) for this to return true.
	bool IsOpaque() const;

	// After this call you are the owner of the pixels and must eventually delete[] them. This tImageTGA object is
	// invalid afterwards.
	tPixel* StealPixels();
	tPixel* GetPixels() const																							{ return Pixels; }
	tPixelFormat SrcPixelFormat = tPixelFormat::Invalid;

private:
	bool SaveUncompressed(const tString& tgaFile, tFormat) const;
	bool SaveCompressed(const tString& tgaFile, tFormat) const;
	void ReadColourBytes(tColouri& dest, const uint8* src, int bitDepth);

	// So this is a neat C++11 feature. Allows simplified constructors.
	int Width = 0;
	int Height = 0;
	tPixel* Pixels = nullptr;
};


// Implementation blow this line.


inline void tImageTGA::Clear()
{
	Width = 0;
	Height = 0;
	delete[] Pixels;
	Pixels = nullptr;
	SrcPixelFormat = tPixelFormat::Invalid;
}


}



