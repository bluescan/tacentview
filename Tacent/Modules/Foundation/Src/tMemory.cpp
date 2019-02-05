// tMemory.cpp
//
// Tacent memory management API.
//
// Copyright (c) 2004-2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "Foundation/tMemory.h"
#include "Foundation/tAssert.h"


void* tMem::tMalloc(int size, int alignSize)
{
	// This code works for both 32 and 64 bit pointers.
	bool isPow2 = ((alignSize < 1) || (alignSize & (alignSize-1))) ? false : true;
	tAssert(isPow2);
		
	uint8* rawAddr = (uint8*)malloc(size + alignSize + sizeof(int));
	if (!rawAddr)
		return nullptr;

	uint8* base = rawAddr + sizeof(int);

	// The align mask only works if alignSize is a power or 2. Essentially the '&' does a mod (%) and we find
	// an aligned address starting from base.
	int64 alignMask = alignSize - 1;
	uint8* alignedPtr = base + (alignSize - (int64(base) & alignMask));

	// We now need to write the offset in the 4 bytes before the aligned a
	base = alignedPtr - sizeof(int);
	*((int*)base) = int(alignedPtr - rawAddr);

	return alignedPtr;
}


void tMem::tFree(void* mem)
{
	uint8* rawAddr = (uint8*)mem;
	int* offsetAddr = ((int*)rawAddr) - 1;
	rawAddr -= *offsetAddr;
	free(rawAddr);
}
