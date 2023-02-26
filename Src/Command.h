// Command.h
//
// Command line use of TacentView to perform batch image processing and conversions. The command-line interface supports
// operations such as quantization, rescaling/filtering, cropping, rotation, extracting frames, creating contact-sheets,
// amalgamating images into animated formats, and levels adjustments.
//
// Copyright (c) 2023 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <System/tPrint.h>


namespace Command
{	
	int Process();

	// There are 3 levels of print verbosity that may be controlled from the CLI:
	// 0 (none): No print output whatsoever.
	// 1 (norm): Normal/moderate print output.
	// 2 (full): Full verbose print output.
	// Using regular always prints. Useful for debugging and when a command line option was explicitly asked to print
	// some output, like when --help or --syntax is used.
	int tPrintfNorm(const char* format, ...);		// Appears for verbosity level 1.
	int tPrintfFull(const char* format, ...);		// Appears for verbosily level 1 and 2.
}


// Implementation only below this line.


inline int Command::tPrintfNorm(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Verbosity0, f, l);
	va_end(l);			return n;
}


inline int Command::tPrintfFull(const char* f, ...)
{
	va_list l;			va_start(l, f);
	int n = tvPrintf	(tSystem::tChannel_Verbosity1, f, l);
	va_end(l);			return n;
}
