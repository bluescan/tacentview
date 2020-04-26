// tImageWEBP.cpp
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

#include <Foundation/tStandard.h>
#include <Foundation/tString.h>
#include <System/tFile.h>
#include "Image/tImageWEBP.h"
#ifdef PLATFORM_WINDOWS
#include "../../../Contrib/WebP/Windows/include/demux.h"
#elif defined(PLATFORM_LINUX)
#include "../../../Contrib/WebP/Linux/include/demux.h"
#endif
using namespace tSystem;
namespace tImage
{


bool tImageWEBP::Load(const tString& webpFile)
{
	Clear();

	if (tSystem::tGetFileType(webpFile) != tSystem::tFileType::WEBP)
		return false;

	if (!tFileExists(webpFile))
		return false;

	int numBytes = 0;
	uint8* webpFileInMemory = tLoadFile(webpFile, nullptr, &numBytes);

	// Now we load and populate the frames.
	WebPData webpData;
	webpData.bytes = webpFileInMemory;
	webpData.size = numBytes;

	WebPDemuxer* demux = WebPDemux(&webpData);
	uint32 width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
	uint32 height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
	uint32 flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);
	uint32 numFrames = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);
	
	if ((width <= 0) || (height <= 0) || (numFrames <= 0))
	{
		WebPDemuxDelete(demux);
		return false;
	}

	// Iterate over all frames.
	SrcPixelFormat = tPixelFormat::R8G8B8;
	WebPIterator iter;
	if (WebPDemuxGetFrame(demux, 1, &iter))
	{
		do
		{
			WebPDecoderConfig config;
			WebPInitDecoderConfig(&config);

			config.output.colorspace = MODE_RGBA;
			config.output.is_external_memory = 0;
			config.options.flip = 1;
			int result = WebPDecode(iter.fragment.bytes, iter.fragment.size, &config);
			if (result != VP8_STATUS_OK)
				continue;

			Frame* newFrame = new Frame;
			newFrame->SrcPixelFormat = iter.has_alpha ? tPixelFormat::R8G8B8A8 : tPixelFormat::R8G8B8;
			if (iter.has_alpha)
				SrcPixelFormat = tPixelFormat::R8G8B8A8;
			newFrame->Width = width;
			newFrame->Height = height;
			newFrame->Pixels = new tPixel[width * height];
			newFrame->Duration = float(iter.duration) / 1000.0f;

			tStd::tMemcpy(newFrame->Pixels, config.output.u.RGBA.rgba, width * height * sizeof(tPixel));
			WebPFreeDecBuffer(&config.output);
			Frames.Append(newFrame);
		}
		while (WebPDemuxNextFrame(&iter));

		WebPDemuxReleaseIterator(&iter);
	}

	WebPDemuxDelete(demux);
	return true;
}


}
