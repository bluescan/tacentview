// tPool.h
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

#pragma once
#include "Foundation/tAssert.h"
#include "Foundation/tList.h"
#include "Foundation/tSync.h"
#include "Foundation/tMemory.h"
namespace tMem
{


// Define MEMORY_ALLOCATOR_CHECK for cursory sanity checking of pool pointers.
// Define MEMORY_ALLOCATOR_CHECK and MEMORY_ALLOCATOR__DEEP_CHECK for slower but more thorough sanity checking of pool pointers.
// #define MEMORY_ALLOCATOR_CHECK
// #define MEMORY_ALLOCATOR__DEEP_CHECK


// Abstract base class for all types of memory allocators. By having the Malloc and Free calls be virtual, a call to an
// operator new that passes in the memory pool to use can simply pass in a tAllocator pointer, allowing different types
// of memory pools without adding complexity to the implementation of new. The overloaded new can just call Malloc. The
// same reasoning holds for delete and free.
class tAllocator
{
public:
	tAllocator()																										: Constructed(true), NumAllocations(0) { }
	virtual ~tAllocator()																								{ Constructed = false; }
	virtual void* Malloc(int numBytes)																					= 0;
	virtual void Free(void*)																							= 0;
	int GetNumAllocations() const																						{ return NumAllocations; }

	// This variable is to keep track of whether the allocator object is initialized. If the allocator is static and
	// we are overriding operator new, we don't know when the allocator is constructed or destructed. This can cause the
	// allocator to be destructed before global objects that have been allocated from it get destructed.
	//
	// Static initialization guarantees Constructed is false. Any Malloc calls before construction will return nullptr
	// allowing us to fall back to std malloc. In the allocator destructor we can set Constructed to false but do not
	// free the allocator memory unless NumAllocations == 0. Any other allocator free calls can now check if Constructed
	// is false and free the allocator memory if NumAllocations is 0.
	//
	// The other option is to simply not use custom allocators before main at all. You could add initialize and shutdown
	// calls at the beginning and end of main. Even then, something like a static tString would initialize using
	// non-allocator memory, but could be reassigned during main (using allocator memory), so we'd still need to keep
	// the pool around after main for when the static destructor is called. In any case, if the allocator is used before
	// main by being statically constructed we need deallocations to work after the destructor may have been called.
	bool Constructed;
	int NumAllocations;
};


// tFastPool gives you O(1) malloc and free at the expense of some wasted memory. tFastPool is a slot-based pool. It
// implements a block of slots, each slot big enough for a single item. Initially the free pointer points to the first
// slot, and the first 4 bytes of each slot point to the next slot. A malloc simply returns the free pointer and then
// points the free pointer to the next slot. If you run out of slots a new expansion block is created and initialized.
class tFastPool : public tAllocator
{
public:
	tFastPool
	(
		// This is the max item size in bytes.
		int slotSize,
		int initialSlotsInBlock = 256,

		// If slotsPerExpansionBlock is zero, Malloc will fail (return 0) once all initial slots are used up.
		int slotsPerExpansionBlock = 128,

		// Faster if you don't need threadsafeness. If pool is threadSafe, all pool function calls will be too.
		bool threadSafe = true
	);
	~tFastPool();

	// Allocates some memory. If numBytes is over slotSize it will return nullptr. If slotsPerExpansionBlock is set to 0
	// and all initial slots are used, it will return nullptr. This can be handy if you want to fall-back onto regular
	// memory allocation when dealing with variable sized objects like strings. Extraneous large mallocs can be dealt
	// with by a different strategy while you still get the speed benefit for all of the smaller mallocs. Calling
	// without an argument will return memory at least slotSize big.
	void* Malloc(int numBytes = 0) override;

	// Deallocates the passed in memory.
	void Free(void*) override;

	int GetSlotSize() const																								{ return SlotSize; }
	int GetNumExpansionBlocks() const																					{ return Blocks.GetNumItems() - 1; }
	int GetTotalPoolSize() const																						{ return SlotSize * (GetNumExpansionBlocks()*SlotsPerExpansionBlock + SlotsInitialBlock); }

private:
	void GrowPool();

	// Sanity checking functions. These are slow.
	#ifdef MEMORY_ALLOCATOR_CHECK
	void SanityCheckSlotValid(void* slot);
	void SanityCheckSlotFreeable(void* slot);
	#endif

	struct SlotBlock : public tLink<SlotBlock>
	{
		void Init(int slotSizeInBytes, int numSlots);
		void Shutdown()																									{ if (Slots) tFree(Slots); Slots = nullptr; }

		int SlotSize;
		int NumSlots;
		uint8* Slots;
	};

	// A tList does not call new or delete and so we can use it without risking an infinite loop.
	tList<SlotBlock> Blocks;

	// FreeSlot may be nullptr. This happens when we've run out of room in the currently allocated blocks.
	uint8* FreeSlot;
	int SlotSize;
	int SlotsInitialBlock;
	int SlotsPerExpansionBlock;

	bool ThreadSafe;
	tMutex Mutex;											// Only used if ThreadSafe is true.
};


}


// Implementation below this line.


inline void* tMem::tFastPool::Malloc(int numBytes)
{
	// This can happen for a statically allocated pool that has yet to be constructed. It is safe to call Malloc on
	// such an object but nullptr is returned, allowing caller to fall back to tMalloc or something.
	if (!Constructed)
		return nullptr;

	// The caller can fall-back on regular tMalloc if nullptr is returned.
	if (int(numBytes) > SlotSize)
		return nullptr;

	if (ThreadSafe)
		Mutex.Lock();

	// If the slot pointer is nullptr we need to grow the pool by adding another block. This will automatically set
	// FreeSlot to non-zero.
	if (!FreeSlot)
	{
		if (SlotsPerExpansionBlock > 0)
			GrowPool();
		else
			return nullptr;
	}

	// Free pointer to next free pointer.
	void* freeSlot = (void*)FreeSlot;

	#ifdef MEMORY_ALLOCATOR_CHECK
	if (*((uint8**)FreeSlot))
		SanityCheckSlotValid(*((uint8**)FreeSlot));
	#endif

	FreeSlot = *((uint8**)FreeSlot);
	NumAllocations++;

	if (ThreadSafe)
		Mutex.Unlock();

	return freeSlot;
}


inline void tMem::tFastPool::Free(void* slot)
{
	#ifdef MEMORY_ALLOCATOR_CHECK
	SanityCheckSlotFreeable(slot);
	#endif

	// This isn't delete. Zero is not allowed.
	tAssert(slot);

	if (ThreadSafe)
		Mutex.Lock();

	*((uint8**)slot) = FreeSlot;
	FreeSlot = (uint8*)slot;

	NumAllocations--;
	if (ThreadSafe)
		Mutex.Lock();

	// If Constructed is false and NumAllocations is 0 we free the pool memory.
	if (!Constructed && (NumAllocations == 0))
	{
		while (SlotBlock* block = Blocks.Remove())
		{
			block->Shutdown();
			tFree(block);
		}

		if (ThreadSafe)
			Mutex.Unlock();
	}
}
