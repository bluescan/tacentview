// tPriorityQueue.h
//
// A priority queue implemented using the heap data structure. Priority queues support retrieval of min or max item
// in the collection in O(1) time. Removal of the min or max in O(lg(n)) time, and insertion in O(lg(n)) time.
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
#include "Foundation/tStandard.h"


template <typename T> class tPriorityQueue
{
public:
	// A tPriorityQueue places nodes with smaller key values closer to the root of the tree. If you set 'ascending' to
	// false then it will place larger key values closer to the root.
	tPriorityQueue(int initialSize, int growSize, bool ascending = true);
	virtual ~tPriorityQueue()																							{ delete[] Heap; }

	struct tItem
	{
		tItem()																											: Data(), Key(0x0000000000000000) { }
		tItem(const tItem& src)																							: Data(src.Data), Key(src.Key) { }
		tItem(T d, int64 k)																								: Data(d), Key(k) { }
		T Data;												// Up to client what this is for. A pointer is often used.
		int64 Key;
	};

	void Insert(const tItem&);								// Put a node into the queue.
	tItem GetMin() const									/* Error to call if GetCount() < 1. */						{ tAssert(NumItems > 0); return Heap[0]; }
	tItem GetRemoveMin();									// Error to call if Count() < 1.

	int GetNumItems() const																								{ return NumItems; }
	bool IsEmpty() const																								{ return NumItems == 0; }

	// Iterates through all nodes updating their data if it matches origData. Returns number of replacements.
	int Replace(T origData, T newData);

private:
	int GetLeftIndex(int i) const																						{ return 1 + (i << 1); }
	int GetRightIndex(int i) const																						{ return 2 + (i << 1); }
	int GetParentIndex(int i) const																						{ return (i - 1) >> 1; }
	void Heapify(int i);

	bool Ascending;
	int NumItems;											// Number of items in the heap.
	int MaxItems;											// Total number of items currently available to be used.
	int NumItemsGrow;										// How many items to grow by if we run out of room.
	tItem* Heap;
};
template<typename T> using tPQ = tPriorityQueue<T>;


// Implementation below this line.


template <typename T> inline tPriorityQueue<T>::tPriorityQueue(int initSize, int growSize, bool ascending) :
	Ascending(ascending),
	NumItems(0),
	MaxItems(initSize),
	NumItemsGrow(growSize)
{
	tAssert((MaxItems >= 1) && (NumItemsGrow >= 1));
	Heap = new tItem[initSize];
}


template <typename T> inline void tPriorityQueue<T>::Heapify(int i)
{
	tAssert(i >= 0);

	// Initially an invalid index.
	int smallest = -1;
	while (smallest != i)
	{
		if (smallest != -1)
		{
			// Exchange.
			tItem temp = Heap[i];
			Heap[i] = Heap[smallest];
			Heap[smallest] = temp;
			i = smallest;
		}

		int left = GetLeftIndex(i);
		if
		(
			(left < NumItems) &&
			(Ascending ? (Heap[left].Key < Heap[i].Key) : (Heap[left].Key > Heap[i].Key))
		)
		{
			smallest = left;
		}
		else
		{
			smallest = i;
		}

		int right = GetRightIndex(i);
		if
		(
			(right < NumItems) &&
			(Ascending ? (Heap[right].Key < Heap[smallest].Key) : (Heap[right].Key > Heap[smallest].Key))
		)
		{
			smallest = right;
		}
	}
}


template <typename T> inline typename tPriorityQueue<T>::tItem tPriorityQueue<T>::GetRemoveMin()
{
	tAssert(NumItems > 0);
	tItem min = Heap[0];
	Heap[0] = Heap[NumItems-1];

	NumItems--;
	Heapify(0);
	return min;
}


template <typename T> inline void tPriorityQueue<T>::Insert(const typename tPriorityQueue<T>::tItem& k)
{
	NumItems++;

	// Do we need to grow array?
	if (NumItems > MaxItems)
	{
		MaxItems += NumItemsGrow;
		tItem* newHeap = new tItem[MaxItems];

		tStd::tMemcpy(newHeap, Heap, sizeof(tItem)*(NumItems-1));
		delete[] Heap;
		Heap = newHeap;
	}

	int i = NumItems - 1;
	while
	(
		(i > 0) &&
		(Ascending ? (Heap[GetParentIndex(i)].Key > k.Key) : (Heap[GetParentIndex(i)].Key < k.Key))
	)
	{
		Heap[i] = Heap[GetParentIndex(i)];
		i = GetParentIndex(i);
	}

	Heap[i] = k;
}


template <typename T> inline int tPriorityQueue<T>::Replace(T origData, T newData)
{
	int numReplaced = 0;
	for (int i = 0; i < NumItems; i++)
	{
		if (Heap[i].Data == origData)
		{
			Heap[i].Data = newData;
			numReplaced++;
		}
	}

	return numReplaced;
}
