// tPicture.cpp
//
// This class represents a simple one layer image. It is a collection of raw uncompressed 32-bit tPixels. It can load
// various formats from disk such as jpg, tga, png, etc. It intentionally _cannot_ load a dds file. More on that later.
// This class can load tga files with a native implementation, and uses CxImage for everything else. Saving
// functionality is restricted to saving tga files only (targa files are not lossless when RLE compressed). Image
// manipulation (excluding compression) happens in a tPicture, so there are crop, scale, etc functions in this class.
//
// Copyright (c) 2006, 2016, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "Foundation/tStandard.h"
#include "Image/tPicture.h"
#include "Image/tFileTGA.h"
#include <CxImage/ximage.h>
using namespace tImage;
using namespace tSystem;


void tPicture::Set(int width, int height)
{
	tAssert((width > 0) && (height > 0));

	// Reuse the existing buffer if possible.
	if (width*height != Width*Height)
	{
		delete[] Pixels;
		Pixels = new tPixel[width*height];
	}
	Width = width;
	Height = height;
	for (int pixel = 0; pixel < (Width*Height); pixel++)
		Pixels[pixel].MakeBlack();	
}


void tPicture::Set(int width, int height, tPixel* pixelBuffer, bool copyPixels)
{
	tAssert((width > 0) && (height > 0) && pixelBuffer);

	// If we're copying the pixels we may be able to reuse the existing buffer if it's the right size. If we're not
	// copying and the buffer is being handed to us, we just need to free our current buffer.
	if (copyPixels)
	{
		if (width*height != Width*Height)
		{
			delete[] Pixels;
			Pixels = new tPixel[width*height];
		}
	}
	else
	{
		delete[] Pixels;
		Pixels = pixelBuffer;
	}
	Width = width;
	Height = height;

	if (copyPixels)
		for (int pixel = 0; pixel < (Width*Height); pixel++)
			Pixels[pixel] = pixelBuffer[pixel];	
}


bool tPicture::CanSave(tFileType fileType)
{
	switch (fileType)
	{
		case tFileType::TGA:				// Targas handled natively.
		case tFileType::BMP:
		case tFileType::GIF:
		case tFileType::JPG:
		case tFileType::PNG:
			return true;
	}

	return false;
}


bool tPicture::CanLoad(tFileType fileType)
{
	// Targas are handled natively.
	if (fileType == tFileType::TGA)
		return true;

	// The rest are handled by CxImage.
	if (GetCxFormat(fileType) != CXIMAGE_FORMAT_UNKNOWN)
		return true;

	return false;
}


bool tPicture::Save(const tString& imageFile, tPicture::tColourFormat colourFmt)
{
	if (!IsValid())
		return false;

	tFileType fileType = tGetFileType(imageFile);
	if (!CanSave(fileType))
		return false;

	if (fileType == tFileType::TGA)
		return SaveTGA(imageFile, tImage::tFileTGA::tFormat(colourFmt), tImage::tFileTGA::tCompression::None);

	tPixel* reorderedPixelArray = new tPixel[Width*Height];
	for (int p = 0; p < Width*Height; p++)
	{
		tPixel pixel = Pixels[p];
		tStd::tSwap(pixel.R, pixel.B);
		reorderedPixelArray[p] = pixel;
	}

	CxImage image;
	image.CreateFromArray((uint8*)reorderedPixelArray, Width, Height, 32, Width*4, false);
	delete[] reorderedPixelArray;

	uint32 cxImgFormat = CXIMAGE_FORMAT_PNG;
	switch (fileType)
	{
		case tFileType::BMP: cxImgFormat = CXIMAGE_FORMAT_BMP; break;
		case tFileType::GIF: cxImgFormat = CXIMAGE_FORMAT_GIF; break;
		case tFileType::JPG: cxImgFormat = CXIMAGE_FORMAT_JPG; break;
		case tFileType::PNG: cxImgFormat = CXIMAGE_FORMAT_PNG; break;
	}

	if (colourFmt == tPicture::tColourFormat::Colour)
		image.AlphaDelete();
	else if ((colourFmt == tPicture::tColourFormat::Auto) && IsOpaque())
		image.AlphaDelete();

	return image.Save(imageFile.ConstText(), cxImgFormat);
}


bool tPicture::SaveTGA(const tString& tgaFile, tFileTGA::tFormat format, tFileTGA::tCompression compression) const
{
	tFileType fileType = tGetFileType(tgaFile);
	if (!IsValid() || (fileType != tFileType::TGA))
		return false;

	tFileTGA targa(Pixels, Width, Height);
	tFileTGA::tFormat savedFormat = targa.Save(tgaFile, format, compression);
	if (savedFormat == tFileTGA::tFormat::Invalid)
		return false;

	return true;
}


bool tPicture::Load(const tString& imageFile)
{
	Clear();
	if (!tFileExists(imageFile))
		return false;

	tFileType fileType = tGetFileType(imageFile);
	if (!CanLoad(fileType))
		return false;

	// We handle tga files natively.
	if (fileType == tFileType::TGA)
	{
		tFileTGA targa(imageFile);
		if (!targa.IsValid())
			return false;

		Width = targa.GetWidth();
		Height = targa.GetHeight();
		Pixels = targa.StealPixels();
		SrcFileBitDepth = targa.SrcFileBitDepth;
		return true;
	}

	// For everything else we use the CxImage library for loading.
	ENUM_CXIMAGE_FORMATS cxFormat = ENUM_CXIMAGE_FORMATS(GetCxFormat(fileType));
	if (cxFormat == CXIMAGE_FORMAT_UNKNOWN)
		return false;

	CxImage image;
	image.Load(imageFile.ConstText(), cxFormat);
	int width = image.GetWidth();
	int height = image.GetHeight();
	if (!image.IsValid() || (width <= 0) || (height <= 0))
		return false;

	Width = width;
	Height = height;
	Pixels = new tPixel[Width*Height];

	// CxImage alpha oddness. If we request the alpha using GetPixelColor and there is no alpha channel, it returns 0
	// for the alpha, which is incorrect as alpha is normally interpreted as opacity, not transparency. It should be
	// returning full opacity. That's why we need imageHasValidAlphas -- so we can check if the channel exists at all.
	bool imageHasValidAlphas = image.AlphaIsValid();
	SrcFileBitDepth = imageHasValidAlphas ? 32 : 24;

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

			Pixels[index++] = colour;
		}
	}
	Filename = imageFile;
	return true;
}


void tPicture::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Image_Picture);
	{
		chunk.Begin(tChunkID::Image_PictureProperties);
		{
			chunk.Write(Width);
			chunk.Write(Height);
		}
		chunk.End();

		chunk.Begin(tChunkID::Image_PicturePixels);
		{
			chunk.Write(Pixels, GetNumPixels());
		}
		chunk.End();
	}
	chunk.End();
}


void tPicture::Load(const tChunk& chunk)
{
	Clear();
	if (chunk.ID() != tChunkID::Image_Picture)
		return;

	for (tChunk ch = chunk.First(); ch.IsValid(); ch = ch.Next())
	{
		switch (ch.ID())
		{
			case tChunkID::Image_PictureProperties:
			{
				ch.GetItem(Width);
				ch.GetItem(Height);
				break;
			}

			case tChunkID::Image_PicturePixels:
			{
				tAssert(!Pixels && (GetNumPixels() > 0));
				Pixels = new tPixel[GetNumPixels()];
				ch.GetItems(Pixels, GetNumPixels());
				break;
			}
		}
	}
}


void tPicture::Crop(int newWidth, int newHeight)
{
	tAssert((newWidth > 0) && (newHeight > 0));
	if ((newWidth == Width) && (newHeight == Height))
		return;

	tPixel* newPixels = new tPixel[newWidth * newHeight];

	// Set the new pixel colours.
	for (int y = 0; y < newHeight; y++)
	{
		for (int x = 0; x < newWidth; x++)
		{
			// If we're in range of the old picture we just copy the colour. If the old image is invalid no problem, as
			// we'll fall through to the else and the pixel will be set to black.
			if ((x < Width) && (y < Height))
				newPixels[y * newWidth + x] = GetPixel(x, y);
			else
				newPixels[y * newWidth + x].MakeBlack();
		}
	}

	Clear();
	Width = newWidth;
	Height = newHeight;
	Pixels = newPixels;
}


bool tPicture::ScaleHalf()
{
	if (!IsValid())
		return false;

	// A 1x1 image is defined as already being rescaled.
	if ((Width == 1) && (Height == 1))
		return true;

	// We only allow non-divisible-by-2 dimensions if that dimension is exactly 1.
	if ( ((Width & 1) && (Width != 1)) || ((Height & 1) && (Height != 1)) )
		return false;

	int newWidth = Width >> 1;
	int newHeight = Height >> 1;
	if (newWidth == 0)
		newWidth = 1;
	if (newHeight == 0)
		newHeight = 1;

	int numNewPixels = newWidth*newHeight;
	tPixel* newPixels = new tPixel[numNewPixels];

	// Deal with case where src height is 1 and src width is divisible by 2 OR where src width is 1 and src height is
	// divisible by 2. Image is either a row or column vector in this case.
	if ((Height == 1) || (Width == 1))
	{
		for (int p = 0; p < numNewPixels; p++)
		{
			int p2 = 2*p;

			int p0r = Pixels[p2].R;
			int p1r = Pixels[p2 + 1].R;
			newPixels[p].R = tMath::tGetClamp((p0r + p1r)>>1, 0, 255);

			int p0g = Pixels[p2].G;
			int p1g = Pixels[p2 + 1].G;
			newPixels[p].G = tMath::tGetClamp((p0g + p1g)>>1, 0, 255);

			int p0b = Pixels[p2].B;
			int p1b = Pixels[p2 + 1].B;
			newPixels[p].B = tMath::tGetClamp((p0b + p1b)>>1, 0, 255);

			int p0a = Pixels[p2].A;
			int p1a = Pixels[p2 + 1].A;
			newPixels[p].A = tMath::tGetClamp((p0a + p1a)>>1, 0, 255);
		}
	}

	// Handle the case where both width and height are both divisible by 2.
	else
	{
		for (int x = 0; x < newWidth; x++)
		{
			int x2 = 2*x;
			for (int y = 0; y < newHeight; y++)
			{
				int y2 = 2*y;

				// @todo Use SSE/SIMD here?
				int p0r = Pixels[y2*Width + x2].R;
				int p1r = Pixels[y2*Width + x2 + 1].R;
				int p2r = Pixels[(y2+1)*Width + x2].R;
				int p3r = Pixels[(y2+1)*Width + x2 + 1].R;
				newPixels[y*newWidth + x].R = tMath::tGetClamp((p0r + p1r + p2r + p3r)>>2, 0, 255);

				int p0g = Pixels[y2*Width + x2].G;
				int p1g = Pixels[y2*Width + x2 + 1].G;
				int p2g = Pixels[(y2+1)*Width + x2].G;
				int p3g = Pixels[(y2+1)*Width + x2 + 1].G;
				newPixels[y*newWidth + x].G = tMath::tGetClamp((p0g + p1g + p2g + p3g)>>2, 0, 255);

				int p0b = Pixels[y2*Width + x2].B;
				int p1b = Pixels[y2*Width + x2 + 1].B;
				int p2b = Pixels[(y2+1)*Width + x2].B;
				int p3b = Pixels[(y2+1)*Width + x2 + 1].B;
				newPixels[y*newWidth + x].B = tMath::tGetClamp((p0b + p1b + p2b + p3b)>>2, 0, 255);

				int p0a = Pixels[y2*Width + x2].A;
				int p1a = Pixels[y2*Width + x2 + 1].A;
				int p2a = Pixels[(y2+1)*Width + x2].A;
				int p3a = Pixels[(y2+1)*Width + x2 + 1].A;
				newPixels[y*newWidth + x].A = tMath::tGetClamp((p0a + p1a + p2a + p3a)>>2, 0, 255);
			}
		}
	}

	Clear();
	Pixels = newPixels;
	Width = newWidth;
	Height = newHeight;
	return true;
}


bool tPicture::Resample(int width, int height, tFilter filter)
{
	if (!IsValid())
		return false;

	int origWidth = GetWidth();
	int origHeight = GetHeight(); 
	tAssert((origWidth > 0) && (origHeight > 0));

	if ((width == origWidth) && (height == origHeight))
		return true;

	// For now we're using CxImage because it's there.
	CxImage image(origWidth, origHeight, 32);

	// Saying 32bbp isn't enough for CxImage to do alphas. Odd.
	image.AlphaCreate();

	for (int y = 0; y < origHeight; y++)
	{
		for (int x = 0; x < origWidth; x++)
		{
			tPixel p = GetPixel(x, y);
			RGBQUAD q;
			q.rgbBlue = p.B;
			q.rgbGreen = p.G;
			q.rgbRed = p.R;
			q.rgbReserved = p.A;
			image.SetPixelColor(x, y, q, true);
		}
	}

	CxImage::InterpolationMethod interpolation = CxImage::IM_NEAREST_NEIGHBOUR;
	switch (filter)
	{
		case tFilter::NearestNeighbour:
			interpolation = CxImage::IM_NEAREST_NEIGHBOUR;
			break;

		case tFilter::Box:
			interpolation = CxImage::IM_BOX;
			break;

		case tFilter::Bilinear:
			interpolation = CxImage::IM_BILINEAR;
			break;

		case tFilter::Bicubic:
			interpolation = CxImage::IM_BICUBIC2;
			break;

		case tFilter::Lanczos:
			interpolation = CxImage::IM_LANCZOS;
			break;

		case tFilter::Kaiser:
			interpolation = CxImage::IM_SINC;
			break;
	}

	// Here's where the work is done.
	bool ok = image.Resample2(width, height, interpolation);
	if (!ok)
		return false;

	Clear();
	Width = width;
	Height = height;
	Pixels = new tPixel[Width*Height];

	// Now we just pull the pixels from the CxImage.
	int index = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// GetPixelColor gets the alpha as well.
			RGBQUAD rgba = image.GetPixelColor(x, y);
			tColouri colour;
			colour.R = rgba.rgbRed;
			colour.G = rgba.rgbGreen;
			colour.B = rgba.rgbBlue;
			colour.A = rgba.rgbReserved;
			Pixels[index++] = colour;
		}
	}

	return true;
}


int tPicture::GetCxFormat(tFileType fileType)
{
	switch (fileType)
	{
		case tFileType::TGA:
			return CXIMAGE_FORMAT_TGA;

		case tFileType::BMP:
			return CXIMAGE_FORMAT_BMP;

		case tFileType::PNG:
			return CXIMAGE_FORMAT_PNG;

		case tFileType::GIF:
			return CXIMAGE_FORMAT_GIF;

		case tFileType::JPG:
			return CXIMAGE_FORMAT_JPG;

		case tFileType::TIFF:
			return CXIMAGE_FORMAT_TIF;

		case tFileType::PCX:
			return CXIMAGE_FORMAT_PCX;

		case tFileType::WBMP:
			return CXIMAGE_FORMAT_WBMP;

		case tFileType::WMF:
			return CXIMAGE_FORMAT_WMF;

		case tFileType::JP2:
			return CXIMAGE_FORMAT_JP2;

		case tFileType::JPC:
			return CXIMAGE_FORMAT_JPC;
	}

	return CXIMAGE_FORMAT_UNKNOWN;
}
