// tImageGIF.h
//
// This knows how to load gifs. It knows the details of the gif file format and loads the data into multiple tPixel
// arrays, one for each frame (gifs may be animated). These arrays may be 'stolen' by tPictures.
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
#include <Image/tPixelFormat.h>
#include "../../../Contrib/gif_load/gif_load.h"
namespace tImage
{


class tImageGIF
{
public:
	// Creates an invalid tImageGIF. You must call Load manually.
	tImageGIF()																											{ }
	tImageGIF(const tString& gifFile)																					{ Load(gifFile); }

	virtual ~tImageGIF()																								{ Clear(); }

	// Clears the current tImageGIF before loading. If false returned object is invalid.
	bool Load(const tString& gifFile);

	// After this call no memory will be consumed by the object and it will be invalid.
	void Clear();
	bool IsValid() const																								{ return (GetNumFrames() >= 1); }

	int GetWidth() const																								{ return Width; }
	int GetHeight() const																								{ return Height; }
	int GetNumFrames() const																							{ return Frames.GetNumItems(); }

	struct Frame : public tLink<Frame>
	{
		tPixel* Pixels = nullptr;
		float Duration = 0.0f;			// Frame duration in seconds. Converted from the gif 10ms count.
	};

	// After this call you are the owner of the frame and must eventually delete it. The frame you stole will no
	// longer be a valid frame of the tImageGIF, but the remaining ones will still be valid.
	Frame* StealFrame(int frameNum);
	Frame* GetFrame(int frameNum);
	tPixelFormat SrcPixelFormat = tPixelFormat::Invalid;

private:
	static void FrameCallbackBridge(void* imgGifRaw, struct GIF_WHDR*);
	void FrameCallback(struct GIF_WHDR*);

	// Variables used during callback processing.
	int FrmLast = 0;
	tPixel* FrmPict = nullptr;
	tPixel* FrmPrev = nullptr;

	int Width				= 0;
	int Height				= 0;
	tList<Frame> Frames;
};


// Implementation only below.


inline void tImageGIF::FrameCallbackBridge(void* imgGifRaw, struct GIF_WHDR* whdr)
{
	tImageGIF* imgGif = (tImageGIF*)imgGifRaw;
	imgGif->FrameCallback(whdr);
}


inline tImageGIF::Frame* tImage::tImageGIF::StealFrame(int frameNum)
{
	Frame* f = GetFrame(frameNum);
	if (!f)
		return nullptr;

	return Frames.Remove(f);
}


inline tImageGIF::Frame* tImage::tImageGIF::GetFrame(int frameNum)
{
	if ((frameNum >= Frames.GetNumItems()) || (frameNum < 0))
		return nullptr;

	Frame* f = Frames.First();
	while (frameNum--)
		f = f->Next();

	return f;
}


inline void tImageGIF::Clear()
{
	Width = 0;
	Height = 0;
	while (Frame* frame = Frames.Remove())
	{
		delete[] frame->Pixels;
		delete frame;
	}
	SrcPixelFormat = tPixelFormat::Invalid;
}


}
