// tStream.h
//
// A stream is an abstract base class that specifies a consistent and simple API for input and output. For example, if
// a tool needs to write files to disk, or memory, or possibly a custom file system, it makes sense to use the tStream
// API since it abstracts away the underlying device. Essentially a stream is any object that can have bytes written
// to, or read from. All streams support the idea of a current read/write index, along with read and write functions.
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
namespace tSystem
{


class tStream
{
public:
	enum tMode
	{
		tMode_Invalid																									= 1 << 0,
		tMode_Null																										= tMode_Invalid,
		tMode_Read																										= 1 << 1,
		tMode_Write																										= 1 << 2,
	};
	typedef uint32 tModes;

	tStream(tModes);
	virtual ~tStream();

	void SetMode(uint32 modeBits);
	tModes GetMode() const																								{ return Modes; }

	int GetPos() const																									{ return Position; }
	int Tell() const																									{ return GetPos(); }

	// Returns the number of bytes read from the stream and written to the dest buffer. If an error occurs, -1 is
	// returned. This can happen if, but not iff: a) The stream is not in read mode. b) 'dest' is null. c) 'numBytes'
	// is not >= 0, or d) There was a read error.
	virtual int Read(uint8* dest, int numBytes)																			= 0;

	// Returns the number of bytes written to the stream and read from the src buffer. If an error occurs, -1 is
	// returned. This can happen if, but not iff: a) The stream is not in write mode. b) 'src' is null. c) 'numBytes'
	// is not >= 0, or d) There was a write error.
	virtual int Write(const uint8* src, int numBytes)																	= 0;

protected:
	tModes Modes;
	int Position;
};


}
