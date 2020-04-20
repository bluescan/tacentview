// tImageICO.h
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

#pragma once
#include <Foundation/tString.h>
#include <Math/tColour.h>
#include <Image/tPixelFormat.h>
namespace tImage
{


class tImageICO
{
public:
	// Creates an invalid tImageICO. You must call Load manually.
	tImageICO()																											{ }
	tImageICO(const tString& icoFile)																					{ Load(icoFile); }

	virtual ~tImageICO()																								{ Clear(); }

	// Clears the current tImageICO before loading. If false returned object is invalid.
	bool Load(const tString& icoFile);

	// After this call no memory will be consumed by the object and it will be invalid.
	void Clear();
	bool IsValid() const																								{ return (GetNumParts() >= 1); }
	int GetNumParts() const																								{ return Parts.GetNumItems(); }
	tPixelFormat GetBestSrcPixelFormat() const;

	struct Part : public tLink<Part>
	{
		int Width					= 0;
		int Height					= 0;
		tPixel* Pixels				= nullptr;
		tPixelFormat SrcPixelFormat	= tPixelFormat::Invalid;
	};

	// After this call you are the owner of the part and must eventually delete it. The part you stole will no
	// longer be a valid part of the tImageICO, but the remaining ones will still be valid.
	Part* StealPart(int partNum);
	Part* GetPart(int partNum);

private:
	bool PopulateParts(const uint8* buffer, int numBytes);	
	Part* CreatePart(const uint8* buffer, int width, int height, int numBytes);

	tList<Part> Parts;
};


// Implementation only below.


inline tImageICO::Part* tImage::tImageICO::StealPart(int partNum)
{
	Part* p = GetPart(partNum);
	if (!p)
		return nullptr;

	return Parts.Remove(p);
}


inline tImageICO::Part* tImage::tImageICO::GetPart(int partNum)
{
	if ((partNum >= Parts.GetNumItems()) || (partNum < 0))
		return nullptr;

	Part* p = Parts.First();
	while (partNum--)
		p = p->Next();

	return p;
}


inline void tImageICO::Clear()
{
	while (Part* part = Parts.Remove())
	{
		delete[] part->Pixels;
		delete part;
	}
}


inline tPixelFormat tImageICO::GetBestSrcPixelFormat() const
{
	tPixelFormat bestFormat = tPixelFormat::Invalid;
	for (Part* part = Parts.First(); part; part = part->Next())
	{
		if (part->SrcPixelFormat == tPixelFormat::Invalid)
			continue;
		if (part->SrcPixelFormat == tPixelFormat::R8G8B8A8)
			return tPixelFormat::R8G8B8A8;
		else if (part->SrcPixelFormat < bestFormat)
			bestFormat = part->SrcPixelFormat;
	}
	
	return bestFormat;
}


}
