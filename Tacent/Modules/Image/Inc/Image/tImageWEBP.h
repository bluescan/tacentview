// tImageWEBP.h
//
// This knows how to load WebPs. It knows the details of the webp file format and loads the data into multiple tPixel
// arrays, one for each frame (WebPs may be animated). These arrays may be 'stolen' by tPictures.
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
namespace tImage
{


class tImageWEBP
{
public:
	// Creates an invalid tImageWEBP. You must call Load manually.
	tImageWEBP()																										{ }
	tImageWEBP(const tString& webpFile)																					{ Load(webpFile); }

	virtual ~tImageWEBP()																								{ Clear(); }

	// Clears the current tImageWEBP before loading. If false returned object is invalid.
	bool Load(const tString& webpFile);

	// After this call no memory will be consumed by the object and it will be invalid.
	void Clear();
	bool IsValid() const																								{ return (GetNumFrames() >= 1); }
	int GetNumFrames() const																							{ return Frames.GetNumItems(); }

	struct Frame : public tLink<Frame>
	{
		int Width = 0;
		int Height = 0;
		tPixel* Pixels = nullptr;
		float Duration = 0.0f;			// Frame duration in seconds. Converted from the WebPs 1ms count.
		tPixelFormat SrcPixelFormat = tPixelFormat::Invalid;
	};

	// After this call you are the owner of the frame and must eventually delete it. The frame you stole will no
	// longer be a valid frame of the tImageWEBP, but the remaining ones will still be valid.
	Frame* StealFrame(int frameNum);
	Frame* GetFrame(int frameNum);
	tPixelFormat SrcPixelFormat = tPixelFormat::Invalid;

private:
	tList<Frame> Frames;
};


// Implementation only below.


inline tImageWEBP::Frame* tImage::tImageWEBP::StealFrame(int frameNum)
{
	Frame* f = GetFrame(frameNum);
	if (!f)
		return nullptr;

	return Frames.Remove(f);
}


inline tImageWEBP::Frame* tImage::tImageWEBP::GetFrame(int frameNum)
{
	if ((frameNum >= Frames.GetNumItems()) || (frameNum < 0))
		return nullptr;

	Frame* f = Frames.First();
	while (frameNum--)
		f = f->Next();

	return f;
}


inline void tImageWEBP::Clear()
{
	while (Frame* frame = Frames.Remove())
	{
		delete[] frame->Pixels;
		delete frame;
	}

	SrcPixelFormat = tPixelFormat::Invalid;
}


}
