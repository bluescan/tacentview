// tPool.cpp
//
// Memory management pools. These are significantly faster than standard memory allocators. All pool implementations
// found here must not call new or delete. The reason for this is to allow a user of the pool to overload operator new.
// We would get into an infinite loop if we called new from inside the pool implementation.
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

#include "Foundation/tPool.h"
#include "Foundation/tMemory.h"


void tMem::tFastPool::SlotBlock::Init(int slotSizeInBytes, int numSlots)
{
	SlotSize = slotSizeInBytes;
	NumSlots = numSlots;

	tAssert((!(SlotSize % 4)) && (SlotSize >= 8));
	Slots = (uint8*)tMalloc(SlotSize * NumSlots);

	// Next we need to initialize all the slots so they all "point" to the next one.
	uint8* currSlot = Slots;
	for (int i = 0; i < NumSlots; i++)
	{
		uint8** currPointerPointer = (uint8**)currSlot;

		if (i < (NumSlots-1))
		{
			uint8* next = currSlot+SlotSize;
			*currPointerPointer = next;
		}
		else
		{
			*currPointerPointer = 0;
		}

		currSlot += SlotSize;
	}
}


tMem::tFastPool::tFastPool(int maxItemSizeInBytes, int initialSlotsInBlock, int slotsPerExpansionBlock, bool threadSafe) :
	Blocks(false),
	FreeSlot(nullptr),
	SlotSize(maxItemSizeInBytes),
	SlotsInitialBlock(initialSlotsInBlock),
	SlotsPerExpansionBlock(slotsPerExpansionBlock),
	ThreadSafe(threadSafe),
	PoolMutex()
{
	// OK, the slot size must be big enough to hold a pointer. We currently support up to 8 byte pointers. Also, the slot
	// size must be a multiple of 4 for alignment purposes.
	if (SlotSize < 8)
	{
		SlotSize = 8;
	}
	else
	{
		int mod = SlotSize % 4;
		if (mod > 0)
			SlotSize += 4 - mod;
	}

	tAssert(!(SlotSize % 4));
	SlotBlock* initialBlock = (SlotBlock*)tMalloc(sizeof(SlotBlock));
	initialBlock->Init(SlotSize, initialSlotsInBlock);
	Blocks.Append(initialBlock);

	FreeSlot = initialBlock->Slots;
}


tMem::tFastPool::~tFastPool()
{
	// Statically allocated pools can't assume NumAllocations is 0 even if there are no leaks.
	if (NumAllocations == 0)
	{
		while (SlotBlock* block = Blocks.Remove())
		{
			block->Shutdown();
			tFree(block);
		}
	}
}


void tMem::tFastPool::GrowPool()
{
	// You're only allowed to grow if there is currently no free slot.
	tAssert(!FreeSlot);

	SlotBlock* expansionBlock = (SlotBlock*)tMalloc(sizeof(SlotBlock));
	expansionBlock->Init(SlotSize, SlotsPerExpansionBlock);

	Blocks.Append(expansionBlock);
	FreeSlot = expansionBlock->Slots;
}


#ifdef MEMORY_ALLOCATOR_CHECK
void tMem::tFastPool::SanityCheckSlotValid(void* slot)
{
	uint64 slotValue = uint64(slot);

	bool found = false;
	SlotBlock* block = Blocks.Head();
	while (block)
	{
		uint64 memStart = uint64(block->Slots);
		uint64 memSize = block->SlotSize * block->NumSlots;
		uint64 position = slotValue - memStart;

		if ((position >= 0) && (position < memSize))
		{
			found = true;
			tAssert((position % block->SlotSize) == 0);
			break;
		}

		block = block->Next();
	}

	tAssert(found);
}


void tMem::tFastPool::SanityCheckSlotFreeable(void* slot)
{
	SanityCheckSlotValid(slot);

	// This check is really slow since it goes through all the allocated slots. We verify that the
	// slot is not in the free list.
	#ifdef MEMORY_ALLOCATOR__DEEP_CHECK
	uint8* freeSlot = FreeSlot;
	while (freeSlot)
	{
		tAssert(freeSlot != (uint8*)slot);
		freeSlot = *((uint8**)freeSlot);
	}
	#endif
}
#endif
