// tRingBuffer.h
//
// A ring (or circular) buffer works like a capacitor in that it can temporarily absorb the shock of a large influx
// of data, but needs to be emptied eventually. The read rate must on average be >= to the write rate if you want to
// avoid a stall. Fortunately, unlike real capacitors, they do not leak information over time.
// 
// Copyright (c) 2016, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tAssert.h>


// tRingBuffer allows you to append one or more items (of user-specified type) to the tail of the ring, remove one or
// more items from the head of the ring, and randomly access any stored item for read/write.
template<typename T> class tRingBuffer
{	
public:
	tRingBuffer()																										: OwnsBuffer(false), Capacity(0), Buffer(nullptr), Head(nullptr), Tail(nullptr) { }
	tRingBuffer(int capacity)								/* Buffer is owned by tRingBuffer. */						: OwnsBuffer(false), Capacity(0), Buffer(nullptr), Head(nullptr), Tail(nullptr) { Set(capacity); }
	tRingBuffer(T* const buffer, int capacity)				/* Buffer ownership is external. */							: OwnsBuffer(false), Capacity(0), Buffer(nullptr), Head(nullptr), Tail(nullptr) { Set(buffer, capacity); }

	void Set(int capacity);									// Buffer is owned by tRingBuffer.
	void Set(T* const buffer, int capacity);				// Buffer is owned by tRingBuffer.
	void Clear();
	bool IsValid() const																								{ return Buffer ? true : false; }

	int GetNumItems() const;
	int GetCapacity() const																								{ return Capacity; }
	int GetRoom() const										/* Returns how many items can be successfully appended. */	{ return Capacity - GetNumItems(); }
	bool IsEmpty() const									/* An invalid ring buffer always returns true. */			{ return !Buffer || ((Tail != nullptr) && (Head == nullptr)); }
	bool IsFull() const										/* An invalid ring buffer always returns true. */			{ return !Buffer || ((Head != nullptr) && (Tail == nullptr)); }

	// There are two versions each of append and remove. One for single items and one for multiple. The multiple item
	// versions are faster than multiple single item appends or removes because they perform the operation with either a
	// single or double tight loop of item copying. If you know the type T can be memory copied (POD types and such),
	// set useMemCpy to true for increased performance. Returns success for the single item versions, and number items
	// appended or removed for the multiple item versions.
	bool Append(const T& item);
	int Append(const T* items, int numItems, bool useMemCpy = false);
	bool Remove(T& item);
	int Remove(T* items, int numItems, bool useMemCpy = false);

	// These two functions are for testing and verification and need not be called for production use. They return -1
	// if the head or tail is invalid.
	int GetHeadIndex() const																							{ if (!Head) return -1; return int(Head - Buffer); }
	int GetTailIndex() const																							{ if (!Tail) return -1; return int(Tail - Buffer); }

private:
	bool OwnsBuffer;										// True if this object owns the buffer.
	int Capacity;											// Maximum number of items that may be stored.
	T* Buffer;												// If null the whole object is invalid.

	// There are few choices regarding head and tail here. I wanted to use the full capacity rather than making
	// head == tail represent 'empty' or something. In the following implementation the head always represents the read
	// location, and is null if nothing can be read. The tail always represents the write location, and is null if
	// nothing can be written. As such, if buffer is null (invalid ring buffer), both head and tail are null. For valid
	// ring buffers the following is true. An empty buffer has a non-null tail and a null head. That is, you can write but
	// not read. A full buffer has a non-null head and a null tail. That is, you can read but not write. eg.
	// - - - - - -   0 items.
	//     T         H=null
	//
	// - - X - - -   1 item.
	//     H T
	//
	// - - X X - -   2 items.
	//     H   T
	//
	// X - X X X X   5 items.
	//   T H
	//
	// X X X X X X   6 items (full).
	//     H         T=null.
	T* Head;												// Points to where you can read from.
	T* Tail;												// Points to where you can write to.
};
template<typename T> using tRing = tRingBuffer<T>;


// Implementation below this line.


template<typename T> inline void tRingBuffer<T>::Set(int capacity)
{
	Clear();
	tAssert(capacity > 0);
	Capacity = capacity;
	Buffer = new T[capacity];

	// We're empty so read head is null, and write tail is valid.
	Tail = Buffer;
	OwnsBuffer = true;
}


template<typename T> inline void tRingBuffer<T>::Set(T* const buffer, int capacity)
{
	Clear();
	tAssert((capacity > 0) && buffer);
	Capacity = capacity;
	Buffer = buffer;
	Tail = Buffer;
	OwnsBuffer = false;
}


template<typename T> inline void tRingBuffer<T>::Clear()
{
	if (OwnsBuffer)
		delete Buffer;
	Buffer = nullptr;
	Head = nullptr;
	Tail = nullptr;
	OwnsBuffer = false;
	Capacity = 0;
}


template<typename T> inline int tRingBuffer<T>::GetNumItems() const
{
	if (!Head)
		return 0;

	if (!Tail)
		return Capacity;

	if (Head > Tail)
		return Capacity - int(Head - Tail);
	
	return int(Tail - Head);
}


template<typename T> inline bool tRingBuffer<T>::Append(const T& item)
{
	if (!Tail)
		return false;

	*Tail = item;
	if (!Head)
		Head = Tail;

	Tail++;
	if (Tail == Buffer+Capacity)
		Tail = Buffer;

	// Are we full?
	if (Tail == Head)
		Tail = nullptr;

	return true;
}


template<typename T> inline int tRingBuffer<T>::Append(const T* items, int numItems, bool useMemCpy)
{
	// This could have been written naively by just sequentially calling the Append(T) above. The reason
	// it is so complicated is because it organized the writes into two separate tight copy loops.
	int room = GetRoom();
	int numAppended = 0;
	if (!items || (numItems <= 0) || (room <= 0))
		return numAppended;

	int numToAppend = (numItems < room) ? numItems : room;

	// We may need two memcpy/write-loops if we go past the end.
	int loopAvail = 0;
	if (Tail > Head)			// This works if pHead is nullptr as well.
		loopAvail = int((Buffer+Capacity) - Tail);
	int itemCount = (numToAppend < loopAvail) ? numToAppend : loopAvail;

	// Copy loop 1.
	if (useMemCpy)
		tMemcpy(Tail, items, itemCount*sizeof(T));
	else
		for (int i = 0; i < itemCount; i++)
			*(Tail+i) = *(items+i);

	if (!Head && itemCount)
		Head = Tail;

	numAppended += itemCount;
	Tail += itemCount;
	if (Tail == Buffer+Capacity)
		Tail = Buffer;

	// Are we full?
	if (Tail == Head)
	{
		Tail = nullptr;
		return numAppended;
	}

	numToAppend -= itemCount;
	if (numToAppend == 0)
		return numAppended;

	// At this point we still have more to append and we have looped around into 'tail < head' territory.
	items += numAppended;
	loopAvail = int(Head - Tail);
	itemCount = (numToAppend < loopAvail) ? numToAppend : loopAvail;

	// Copy loop 2.
	if (useMemCpy)
		tMemcpy(Buffer, items, itemCount*sizeof(T));
	else
		for (int i = 0; i < itemCount; i++)
			*(Buffer+i) = *(items+i);

	if (!Head && itemCount)
		Head = Tail;

	numAppended += itemCount;
	Tail += itemCount;

	// Are we full?
	if (Tail == Head)
		Tail = nullptr;

	return numAppended;
}


template<typename T> inline bool tRingBuffer<T>::Remove(T& item)
{
	if (!Head)
		return false;

	item = *Head;
	if (!Tail)
		Tail = Head;

	Head++;
	if (Head == Buffer+Capacity)
		Head = Buffer;

	// Are we empty?
	if (Head == Tail)
		Head = nullptr;

	return true;
}


template<typename T> inline int tRingBuffer<T>::Remove(T* items, int numItems, bool useMemCpy)
{
	// This could have been written naively by just sequentially calling the Remove(T) above. The reason
	// it is so complicated is because it organized the writes into two separate tight copy loops.
	int avail = GetNumItems();
	int numRemoved = 0;
	if (!items || (numItems <= 0) || (avail <= 0))
		return numRemoved;

	int numToRemove = (numItems < avail) ? numItems : avail;

	// We may need two memcpy/write-loops if we go past the end.
	int loopAvail = 0;
	if (Head > Tail)			// This works if pTail is nullptr as well.
		loopAvail = int((Buffer+Capacity) - Head);
	int itemCount = (numToRemove < loopAvail) ? numToRemove : loopAvail;

	// Copy loop 1.
	if (useMemCpy)
		tMemcpy(items, Head, itemCount*sizeof(T));
	else
		for (int i = 0; i < itemCount; i++)
			*(items+i) = *(Head+i);

	if (!Tail && itemCount)
		Tail = Head;

	numRemoved += itemCount;
	Head += itemCount;
	if (Head == Buffer+Capacity)
		Head = Buffer;

	// Are we empty?
	if (Head == Tail)
	{
		Head = nullptr;
		return numRemoved;
	}

	numToRemove -= itemCount;
	if (numToRemove == 0)
		return numRemoved;

	// At this point we still have more to remove and we have looped around into 'head < tail' territory.
	items += numRemoved;
	loopAvail = int(Tail - Head);
	itemCount = (numToRemove < loopAvail) ? numToRemove : loopAvail;

	// Copy loop 2.
	if (useMemCpy)
		tMemcpy(items, Buffer, itemCount*sizeof(T));
	else
		for (int i = 0; i < itemCount; i++)
			*(items+i) = *(Buffer+i);

	if (!Tail && itemCount)
		Tail = Head;

	numRemoved += itemCount;
	Head += itemCount;

	// Are we empty?
	if (Head == Tail)
		Head = nullptr;

	return numRemoved;
}
