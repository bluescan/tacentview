// tPicture.h
//
// This class represents a simple one layer image. It is a collection of raw uncompressed 32-bit tPixels. It can load
// various formats from disk such as jpg, tga, png, etc. It intentionally _cannot_ load a dds file. More on that later.
// This class can load tga files with a native implementation, and uses CxImage for everything else. Saving
// functionality is restricted to saving tga files only (targa files are not lossless when RLE compressed). Image
// manipulation (excluding compression) happens in a tPicture, so there are crop, scale, etc functions in this class.
//
// Copyright (c) 2006, 2016, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tList.h>
#include <Math/tColour.h>
#include <System/tFile.h>
#include <System/tChunk.h>
#include "Image/tImageTGA.h"
#include "Image/tImageHDR.h"
namespace tImage
{


// A tPicture is a single 2D image. A rectangular collection of RGBA pixels (32bits per pixel). The origin is the lower
// left, and the rows are ordered from bottom to top in memory. This matches the expectation of OpenGL texture
// manipulation functions for the most part (there are cases when it is inconsistent with itself).
class tPicture : public tLink<tPicture>
{
public:
	// Constructs an empty picture that is invalid. You must call Load yourself later.
	tPicture()																											{ }

	// Constructs a picture that is width by height pixels. It will be all black pixels with an opaque alpha of 255.
	// That is, every pixel will be (0, 0, 0, 255).
	tPicture(int width, int height)																						{ Set(width, height); }

	// This constructor allows you to specify an external buffer of pixels to use. If copyPixels is true, it simply
	// copies the values from the buffer you supply. If copyPixels is false, it means you are giving the buffer to the
	// tPicture. In this case the tPicture will delete[] the buffer for you when appropriate.
	tPicture(int width, int height, tPixel* pixelBuffer, bool copyPixels = true)										{ Set(width, height, pixelBuffer, copyPixels); }

	struct LoadParams
	{
		double HDR_GammaCorr		= tImageHDR::DefaultGammaCorr;
		int HDR_ExposureAdj			= tImageHDR::DefaultExposureAdj;
	};
	// Loads the supplied image file. If the image couldn't be loaded, IsValid will return false afterwards. Uses the
	// filename extension to determine what file type it is loading. dds files may _not_ be loaded into a tPicture.
	// Use a tTexture if you want to load a dds. Loading tga and hdr files is handled by native code. Other formats like
	// jpg and png are handled by CxImage.
	tPicture(const tString& imageFile, LoadParams params = LoadParams())												{ Load(imageFile, params); }

	// Copy constructor.
	tPicture(const tPicture& src)																						: tPicture() { Set(src); }

	virtual ~tPicture()																									{ Clear(); }
	bool IsValid() const																								{ return Pixels ? true : false; }

	// Invalidated the picture and frees memory associated with it. The tPicture will be invalid after this.
	void Clear();

	// Sets the image to the dimensions provided. Image will be opaque black after this call. Internally, if the
	// existing buffer is the right size, it is reused. In all cases, the entire image is cleared to black.
	void Set(int width, int height, const tPixel& colour = tPixel::black);

	// Sets the image to the dimensions provided. Allows you to specify an external buffer of pixels to use. If
	// copyPixels is true, it simply copies the values from the buffer you supply. In this case it will attempt to
	// reuse it's existing buffer if it can. If copyPixels is false, it means you are giving the buffer to the
	// tPicture. In this case the tPicture will delete[] the buffer for you when appropriate. In all cases, existing
	// pixel data is lost.
	void Set(int width, int height, tPixel* pixelBuffer, bool copyPixels = true);
	void Set(const tPicture& src)																						{ if (src.Pixels) { Set(src.Width, src.Height, src.Pixels); Filename = src.Filename; } }

	// Can this class save the the filetype supplied?
	static bool CanSave(const tString& imageFile);
	static bool CanSave(tSystem::tFileType);

	// Can this class load the the filetype supplied?
	static bool CanLoad(const tString& imageFile);
	static bool CanLoad(tSystem::tFileType);

	enum class tColourFormat
	{
		Invalid,	// Invalid must be 0.
		Auto,		// Save function will decide format. Colour if all image pixels are opaque and ColourAndAlpha otherwise.
		Colour,
		ColourAndAlpha
	};

	// Saves to the image file you specify and examines the extension to determine filetype. Supports tga, png, bmp, jpg,
	// and gif. If tColourFormat is set to auto, the opacity/alpha channel will be excluded if all pixels are opaque.
	// Alpha channels are not supported for gif and jpg files. Quality (used for jpg) is in [0.0, 100.0].
	bool Save(const tString& imageFile, tColourFormat = tColourFormat::Auto, float quality = 90.0f);

	bool SaveTGA
	(
		const tString& tgaFile, tImageTGA::tFormat = tImageTGA::tFormat::Auto,
		tImageTGA::tCompression = tImageTGA::tCompression::RLE
	) const;

	// Always clears the current image before loading. If false returned, you will have an invalid tPicture.
	bool Load(const tString& imageFile, LoadParams params = LoadParams());

	// Save and Load to tChunk format.
	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	// Returns true if all pixels are completely opaque (an alphas of 255). This function checks the entire pixel
	// buffer every time it is called.
	bool IsOpaque() const;

	// These functions allow reading and writing pixels.
	tPixel& Pixel(int x, int y)																							{ return Pixels[ GetIndex(x, y) ]; }
	tPixel* operator[](int i)						/* Syntax: image[y][x] = colour;  No bounds checking performed. */	{ return Pixels + GetIndex(0, i); }
	tPixel GetPixel(int x, int y) const																					{ return Pixels[ GetIndex(x, y) ]; }
	tPixel* GetPixelPointer(int x = 0, int y = 0)																		{ return &Pixels[ GetIndex(x, y) ]; }

	void SetPixel(int x, int y, const tColouri& c)																		{ Pixels[ GetIndex(x, y) ] = c; }
	void SetPixel(int x, int y, uint8 r, uint8 g, uint8 b, uint8 a = 0xFF)												{ Pixels[ GetIndex(x, y) ] = tColouri(r, g, b, a); }
	void SetAll(const tColouri& = tColouri(0, 0, 0));

	int GetWidth() const																								{ return Width; }
	int GetHeight() const																								{ return Height; }
	int GetNumPixels() const																							{ return Width*Height; }

	void Rotate90(bool antiClockWise);
	void Flip(bool horizontal);

	// Cropping. If width or height are smaller than the current size the image is cropped. (0, 0) is the anchor. If
	// larger, transparent (alpha 0) black pixels are added.
	enum class Anchor
	{
		LeftTop,		MiddleTop,		RightTop,
		LeftMiddle,		MiddleMiddle,	RightMiddle,
		LeftBottom,		MiddleBottom,	RightBottom
	};
	void Crop(int newWidth, int newHeight, Anchor = Anchor::MiddleMiddle);
	void Crop(int newWidth, int newHeight, int originX, int originY);

	// This function scales the image by half. Useful for generating mipmaps. Only use this if speed is an issue.
	// Offline mipmap generation should use a higher quality filter like Lanczos-Windowed or Kaiser or Kaiser-Gamma. I
	// believe all of these are based on the sinc function. Anyways, we need to generate some mipmaps at runtime, and
	// this simple box-filter (pixel averaging) should be reasonable. Note that bilinear and box filters work arguably
	// better for minification purposes anyways. This function returns false if the rescale could not be performed. For
	// this function to succeed:
	//		- The image needs to be valid AND
	//		- The width must be divisible by two if it is not equal to 1 AND
	//		- The height must be divisible by two if it is not equal to 1.
	// Note I'm saying _divisible_ by 2. Non-powers of 2 are fine! We handle dimensions of 1 because it's handy for
	// mipmap generation. Eg. If width=10 and height=1, we'd end up with a 5x1 image. An 11x1 image would yield an
	// error and return false. A 1x1 successfully yields the same 1x1 image.
	bool ScaleHalf();

	enum class tFilter
	{
		NearestNeighbour,				// Useless.
		Box,							// Fast pixel averaging.
		Bilinear,						// Also known as a triangle filter.  Fast and not too bad quality.
		Bicubic,						// Your standard good PS filter.
		Quadratic,
		Hamming
	};

	// Resizes the image using the specified filter. Returns success. If the resample fails the tPicture is unmodified.
	bool Resample(int width, int height, tFilter filter = tFilter::Bilinear);
	bool Resize(int width, int height, tFilter filter = tFilter::Bilinear)												{ return Resample(width, height, filter); }

	bool operator==(const tPicture&) const;
	bool operator!=(const tPicture&) const;

	tString Filename;
	int SrcFileBitDepth = 32;

private:
	static int GetCxFormat(tSystem::tFileType);
	int GetIndex(int x, int y) const																					{ tAssert((x >= 0) && (y >= 0) && (x < Width) && (y < Height)); return y * Width + x; }
	static int GetIndex(int x, int y, int w, int h)																		{ tAssert((x >= 0) && (y >= 0) && (x < w) && (y < h)); return y * w + x; }

	int Width = 0;
	int Height = 0;
	tPixel* Pixels = nullptr;
};


// Implementation below this line.


inline void tPicture::Clear()
{
	Filename.Clear();
	delete[] Pixels;
	Pixels = nullptr;
	Width = 0;
	Height = 0;
}


inline bool tPicture::CanLoad(const tString& imageFile)
{
	return CanLoad( tSystem::tGetFileType(imageFile) );
}


inline bool tPicture::CanSave(const tString& imageFile)
{
	return CanSave( tSystem::tGetFileType(imageFile) );
}


inline bool tPicture::IsOpaque() const
{
	for (int pixel = 0; pixel < (Width*Height); pixel++)
	{
		if (Pixels[pixel].A < 255)
			return false;
	}

	return true;
}


inline void tPicture::SetAll(const tColouri& clearColour)
{
	if (!Pixels)
		return;

	int numPixels = Width*Height;
	for (int p = 0; p < numPixels; p++)
		Pixels[p] = clearColour;
}


inline bool tPicture::operator==(const tPicture& src) const
{
	if (!Pixels || !src.Pixels)
		return false;

	if ((Width != src.Width) || (Height != src.Height))
		return false;

	for (int pixel = 0; pixel < (Width * Height); pixel++)
		if (Pixels[pixel] != src.Pixels[pixel])
			return false;

	return true;
}


inline bool tPicture::operator!=(const tPicture& src) const
{
	return !(*this == src);
}


}
