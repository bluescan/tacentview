// tList.h
//
// Linked list implementations. There are two implementations in this module. tList is intrusive, tItList (i for
// iterator) is non-intrusive. Use the intrusive one for performance and fewer fragmentation issues if possible.
//
// tList advantages: Faster and less memory fragmentation (one new per object).
// tList disadvantages: An object can only be on one list at a time. You must derive from tLink.
//
// tItList advantages: The same item only in one list at a time. No change in memory image for the objects. Cleaner
// iterator syntax similar to the STL containers. Supports the new C++11 range-based for loop syntax.
// tItList disadvantages: More memory allocs. Not quite as fast.
//
// Copyright (c) 2004-2006, 2015, 2017 Tristan Grimmer.
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
#include "Foundation/tPlatform.h"


enum class tListSortAlgorithm
{
	Merge,													// Guaranteed O(n ln(n)) even in worst case.
	Bubble													// As bad as O(n^2) on unsorted data. Only O(n) on sorted.
};


// You need to derive your object from this if you want to put them on a tList.
template<typename T> class tLink
{
public:
	tLink()																												: NextItem(nullptr), PrevItem(nullptr) { }
	tLink(const tLink&)																									: NextItem(nullptr), PrevItem(nullptr) { }
	T* Next() const																										{ return (T*)NextItem; }
	T* Prev() const																										{ return (T*)PrevItem; }

private:
	template<typename U> friend class tList;
	template<typename U> friend class tListZ;
	mutable const T* NextItem;
	mutable const T* PrevItem;
};


// A tList is an intrusive list implementation. Less mem fragmentation but objects can only be on one list at a time.
template<typename T> class tList
{
public:
	tList()																												{ }

	// If ownsItems is true the objects will be deleted when the list is destroyed.
	tList(bool ownsItems)					 				/* ownsItems defaults to true. */							: OwnsItems(ownsItems) { }
	virtual ~tList()																									{ if (OwnsItems) Empty(); }

	T* Insert(const T* item);								// Insert item at head.	Returns item.
	T* Insert(const T* item, const T* here);				// Insert item before here. Returns item.
	T* Append(const T* item);								// Append item at tail.	Returns item.
	T* Append(const T* item, const T* here);				// Append item after here. Returns item.
	T* Remove(const T* item);								// Removes and returns item.
	T* Remove();											// Removes and returns head item.
	T* Drop();												// Removes and returns tail item.

	void Clear()											/* Clears the list. Deletes items if ownership flag set. */	{ if (OwnsItems) Empty(); else Reset(); }
	void Reset()											/* Resets the list. Never deletes the objects. */			{ HeadItem = nullptr; TailItem = nullptr; ItemCount = 0; }
	void Empty()											/* Empties the list. Always deletes the objects. */			{ while (!IsEmpty()) delete Remove(); }

	T* Head() const																										{ return (T*)HeadItem; }
	T* Tail() const																										{ return (T*)TailItem; }
	T* First() const																									{ return (T*)HeadItem; }
	T* Last() const																										{ return (T*)TailItem; }

	T* NextCirc(const T* here) const						/* Circular. Gets item after here. */ 						{ return here->NextItem ? (T*)here->NextItem : (T*)HeadItem; }
	T* PrevCirc(const T* here) const						/* Circular. Gets item before here. */						{ return here->PrevItem ? (T*)here->PrevItem : (T*)TailItem; }

	int GetNumItems() const																								{ return ItemCount; }
	int NumItems() const																								{ return ItemCount; }
	int Count() const																									{ return ItemCount; }
	bool IsEmpty() const																								{ return !HeadItem; }
	bool Contains(const T& item) const						/* To use this there must be an operator== for type T. */	{ for (const T* n = First(); n; n = n->Next()) if (*n == item) return true; return false; }

	// Sorts the list using the algorithm specified. The supplied compare function should never return true on equal.
	// To sort ascending return the truth of a < b. Return a > b to sort in descending order. Returns the number of
	// compares performed. The compare function usually implements bool CompareFunc(const T& a, const T& b)
	template<typename CompareFunc> int Sort(CompareFunc, tListSortAlgorithm alg = tListSortAlgorithm::Merge);

	// Inserts item in a sorted list. It will remain sorted.
	template<typename CompareFunc> T* Insert(CompareFunc, const T* item);

	// This does an O(n) single pass of a bubble sort iteration. Allows the cost of sorting to be distributed over time
	// for objects that do not change their order very often. Do the expensive merge sort when the list is initially
	// populated, then use this to keep it approximately correct.
	//
	// The direction is important when doing this type of partial sort. A forward bubble will result in the last object
	// in the list being correct after one pass, last two for two passes etc. If getting the front of the list correct
	// sooner is more important you'll want to bubble backwards. This is true regardless of whether the compare
	// function implements an ascending or descending sort.
	//
	// maxCompares sets how far into the list to perform possible swaps. If maxCompares == -1 it means go all the way.
	// That is, GetNumItems() - 1. Returns number of swaps performed.
	template<typename CompareFunc> int Bubble(CompareFunc compare, bool backwards = false, int maxCompares = -1)		{ if (backwards) return BubbleBackward(compare, maxCompares); else return BubbleForward(compare, maxCompares); }

private:
	// These return the number of swaps performed. If 0 is returned, the items considered were already in the correct
	// order. The maxCompares allows you to limit how many compares are performed. -1 means numItems-1 compares will be
	// made (a full sweep). The variable gets clamped to [0, numItems) otherwise.
	template<typename CompareFunc> int BubbleForward(CompareFunc, int maxCompares = -1);
	template<typename CompareFunc> int BubbleBackward(CompareFunc, int maxCompares = -1);

	// These return the number of compares performed.
	template<typename CompareFunc> int SortMerge(CompareFunc);
	template<typename CompareFunc> int SortBubble(CompareFunc);

	mutable const T* HeadItem = nullptr;
	mutable const T* TailItem = nullptr;
	int ItemCount = 0;
	bool OwnsItems = true;
};


// A tListZ is an intrusive list implementation that relies on zero initialization of global objects at startup.
// It can be used for global or static lists but NOT on the stack as a local var. It does NOT delete objects it
// contains on destruction to avoid both the init and de-init fiasco. It does not contain the sorting functions to
// keep the implementation small. Transer all the items over to tList after main starts if you need sorting.
template<typename T> class tListZ
{
public:
	T* Insert(const T* item);								// Insert item at head.	Returns item.
	T* Insert(const T* item, const T* here);				// Insert item before here. Returns item.
	T* Append(const T* item);								// Append item at tail.	Returns item.
	T* Append(const T* item, const T* here);				// Append item after here. Returns item.
	T* Remove(const T* item);								// Removes and returns item.
	T* Remove();											// Removes and returns head item.
	T* Drop();												// Removes and returns tail item.

	// There is no Clear on purpose since there is no ownership flag.
	void Reset()											/* Resets the list. Does not delete the objects. */			{ HeadItem = nullptr; TailItem = nullptr; ItemCount = 0; }
	void Empty()											/* Empties the list. Always deletes the objects. */			{ while (!IsEmpty()) delete Remove(); }

	T* Head() const																										{ return (T*)HeadItem; }
	T* Tail() const																										{ return (T*)TailItem; }
	T* First() const																									{ return (T*)HeadItem; }
	T* Last() const																										{ return (T*)TailItem; }

	T* NextCirc(const T* here) const						/* Circular. Gets item after here. */						{ return here->NextItem ? (T*)here->NextItem : (T*)HeadItem; }
	T* PrevCirc(const T* here) const						/* Circular. Gets item before here. */						{ return here->PrevItem ? (T*)here->PrevItem : (T*)TailItem; }

	int GetNumItems() const																								{ return ItemCount; }
	int NumItems() const																								{ return ItemCount; }
	int Count() const																									{ return ItemCount; }
	bool IsEmpty() const																								{ return !HeadItem; }
	bool Contains(const T& item) const						/* To use this there must be an operator== for type T. */	{ for (const T* n = First(); n; n = n->Next()) if (*n == item) return true; return false; }

private:
	mutable const T* HeadItem;
	mutable const T* TailItem;
	int ItemCount;
};


// The tItList implements a doubly linked non-intrusive iterator-based list. This list class is implemented by using a
// tList of structs that point to the objects in the list.
template<typename T> class tItList
{
public:
	// If ownObjects is true the objects will be deleted when the list is destroyed.
	explicit tItList(bool ownObjects = true)																			: Nodes(true), OwnsObjects(ownObjects) { }
	virtual ~tItList()																									{ if (OwnsObjects) Empty(); else Clear(); }

private:
	// This internal datatype is declared early on to make inlining a bit easier.
	struct IterNode : public tLink<IterNode>
	{
		IterNode(const T* obj)																							: tLink<IterNode>(), Object(obj) { }
		const T* Get() const																							{ return Object; }
		const T* Object;
	};

public:
	// The tItList iterator class for external use.
	class Iter
	{
	public:
		Iter()																											: Node(nullptr), List(nullptr) { }
		Iter(const Iter& src)																							: Node(src.Node), List(src.List) { }

		// Iterators may be dereferenced to get to the object.
		T& operator*() const																							{ return *((T*)Node->Object); }
		T* operator->() const																							{ return (T*)Node->Object; }
		operator bool() const																							{ return Node ? true : false; }

		// Iterators may be treated as pointers to the object.
		operator T*()																									{ return GetObject(); }
		operator const T*() const																						{ return GetObject(); }
		Iter& operator=(const Iter& i)																					{ if (this != &i) { Node = i.Node; List = i.List; } return *this; }

		// Use ++iter instead of iter++ when possible.
		const Iter operator++(int)																						{ Iter curr(*this); Next(); return curr; }
		const Iter operator--(int)																						{ Iter curr(*this); Prev(); return curr; }
		const Iter operator++()																							{ Next(); return *this; }
		const Iter operator--()																							{ Prev(); return *this; }
		const Iter operator+(int offset) const																			{ Iter i = *this; while (offset--) i.Next(); return i; }
		const Iter operator-(int offset) const																			{ Iter i = *this; while (offset--) i.Prev(); return i; }
		bool operator==(const Iter& i) const																			{ return (Node == i.Node) && (List == i.List); }
		bool operator!=(const Iter& i) const																			{ return (Node != i.Node) || (List != i.List); }

		bool IsValid() const																							{ return Node ? true : false; }
		void Invalidate()																								{ Node = nullptr; List = nullptr; }
		void Next()																										{ if (Node) Node = Node->Next(); }
		void Prev()																										{ if (Node) Node = Node->Prev(); }

		// Accessors that allow the list to be treated as circular.
		void NextCirc()																									{ if (Node) Node = Node->Next(); if (!Node) Node = List->Nodes.Head(); }
		void PrevCirc()																									{ if (Node) Node = Node->Prev(); if (!Node) Node = List->Nodes.Tail(); }
		T* GetObject() const																							{ return (T*)(Node ? Node->Object : nullptr); }

	private:
		friend class tItList<T>;
		Iter(IterNode* listNode, const tItList<T>* list)																: Node(listNode), List(list) { }
		IterNode* Node;
		const tItList<T>* List;
	};

	// Insert before head and append after tail.
	T* Insert(const T* obj)																								{ tAssert(obj); Nodes.Insert(new IterNode(obj)); return obj; }
	T* Insert(const T* obj, const Iter& here)																			{ tAssert(obj); tAssert(this == here.List); Nodes.Insert(new IterNode(obj), here.Node); return obj; }
	T* Append(T* obj)																									{ tAssert(obj); Nodes.Append(new IterNode(obj)); return obj; }
	T* Append(const T* obj, const Iter& here)																			{ tAssert(obj); tAssert(this == here.List); Nodes.Append(new IterNode(obj), here.Node); return obj; }

	T* Remove()												/* Removes and returns head. */								{ Iter head = Head(); return Remove(head); }
	T* Remove(Iter&);										// Removed object referred to by Iter. Invalidates Iter.
	T* Drop()												/* Drops and returns tail. */								{ Iter tail = Tail(); return Drop(tail); }
	T* Drop(Iter& iter)										/* Same a Remove. */										{ return Remove(iter); }

	void Clear()											/* Clears the list. Deletes items if ownership flag set. */	{ if (OwnsObjects) Empty(); else Reset(); }
	void Reset()											/* Resets the list. Never deletes the objects. */			{ while (!IsEmpty()) Remove(); }
	void Empty()											/* Empties the list. Always deletes the objects. */			{ while (!IsEmpty()) delete Remove(); }

	Iter Head() const																									{ return Iter(Nodes.Head(), this); }
	Iter Tail() const																									{ return Iter(Nodes.Tail(), this); }
	Iter First() const																									{ return Iter(Nodes.Head(), this); }
	Iter Last() const																									{ return Iter(Nodes.Tail(), this); }

	// Searches list forward for a particular item. Returns its iterator or an invalid one if it wasn't found.
	Iter Find(const T* item) const																						{ for (IterNode* n = Nodes.First(); n; n = n->Next()) if (n->Object == item) return Iter(n, this); return Iter(); }

	int GetNumItems() const																								{ return Nodes.GetNumItems(); }
	int NumItems() const																								{ return Nodes.NumItems(); }
	int Count() const																									{ return Nodes.Count(); }
	bool IsEmpty()	const																								{ return Nodes.IsEmpty(); }

	Iter begin() const										/* For range-based iteration supported by C++11. */			{ return Head(); }
	Iter end() const										/* For range-based iteration supported by C++11. */			{ return Iter(nullptr, this); }

	const T& operator[](const Iter& iter) const																			{ tAssert(iter.IsValid() && (iter.List == this)); return *iter.Node->Object; }
	T& operator[](const Iter& iter)																						{ tAssert(iter.IsValid() && (iter.List == this)); return *((T*)iter.Node->Object); }

	// Sorts the list using the algorithm specified. The supplied compare function should never return true on equal.
	// To sort ascending return the truth of a < b. Return a > b to sort in descending order. Returns the number of
	// compares performed. The compare function should implement bool CompareFunc(const T& a, const T& b)
	template<typename CompareFunc> int Sort(CompareFunc compare, tListSortAlgorithm algo = tListSortAlgorithm::Merge)	{ auto cmp = [&compare](const IterNode& a, const IterNode& b) { return compare(*a.Get(), *b.Get()); }; return Nodes.Sort(cmp, algo); }

	// Inserts item in a sorted list. It will remain sorted.
	template<typename CompareFunc> T* Insert(const T* item, CompareFunc compare)										{ auto cmp = [&compare](IterNode& a, IterNode& b) { return compare(*a.Get(), *b.Get()); }; return Nodes.Insert(item, cmp); }

	// This does an O(n) single pass of a bubble sort iteration. Allows the cost of sorting to be distributed over time
	// for objects that do not change their order very often. Do the expensive merge sort when the list is initially
	// populated, then use this to keep it approximately correct.
	//
	// The direction is important when doing this type of partial sort. A forward bubble will result in the last object
	// in the list being correct after one pass, last two for two passes etc. If getting the front of the list correct
	// sooner is more important you'll want to bubble backwards. This is true regardless of whether the compare
	// function implements an ascending or descending sort.
	//
	// maxCompares sets how far into the list to perform possible swaps. If maxCompares == -1 it means go all the way.
	// That is, GetNumItems() - 1. Returns number of swaps performed.
	//
	// Note that any iterators that you are maintaining should remain valid.
	template<typename CompareFunc> int Bubble(CompareFunc compare, bool backwards = false, int maxCompares = -1)		{ return Nodes.Bubble(compare, backwards, maxCompares); }

private:
	// tItList is implemented using a tList of Nodes that point to the objects.
	tList<IterNode> Nodes;
	bool OwnsObjects;
};


// Implementation below this line.


template<typename T> inline T* tList<T>::Insert(const T* item)
{
	if (HeadItem)
		HeadItem->PrevItem = item;

	item->NextItem = HeadItem;
	item->PrevItem = nullptr;
	HeadItem = item;
	if (!TailItem)
		TailItem = item;

	ItemCount++;
	return (T*)item;
}


template<typename T> inline T* tList<T>::Append(const T* item)
{
	if (TailItem)
		TailItem->NextItem = item;

	item->PrevItem = TailItem;
	item->NextItem = nullptr;
	TailItem = (T*)item;
	if (!HeadItem)
		HeadItem = (T*)item;

	ItemCount++;
	return (T*)item;
}


template<typename T> template<typename CompareFunc> inline T* tList<T>::Insert(CompareFunc compare, const T* item)
{
	if (!Head())
	{
		Insert(item);
		return (T*)item;
	}

	// Early exit if it should go before head or after tail.
	if (compare(*item, *Head()))
	{
		Insert(item, Head());
		return (T*)item;
	}

	if (!compare(*item, *Tail()))
	{
		Append(item);
		return (T*)item;
	}

	// The variables here are named as if compare implements 'bool IsLessThan()'
	T* insertBefore = Head()->Next();
	bool isLessThan = compare(*item, *insertBefore);
	bool isGreaterThanOrEqualTo = !isLessThan;

	bool searching = true;
	while (searching)
	{
		// We are searching for the first existing item that is greater than our item that we wish to insert...
		// ie. Keep searching when (item >= insertBefore) .. stop searchign when .. (item < insertBefore)
		searching = isGreaterThanOrEqualTo;
		if (searching)
		{
			insertBefore = insertBefore->Next();
			isLessThan = compare(*item, *insertBefore);
			isGreaterThanOrEqualTo = !isLessThan;
		}		
	}

	// Found it... so this is the item we insert before.
	Insert(item, insertBefore);
	return (T*)item;
}


template<typename T> inline T* tList<T>::Insert(const T* item, const T* where)
{
	tAssert(item);
	if (!where)
		Insert(item);

	item->NextItem = where;
	item->PrevItem = where->PrevItem;
	where->PrevItem = item;

	if (item->PrevItem)
		item->PrevItem->NextItem = item;
	else
		HeadItem = item;

	ItemCount++;
	return (T*)item;
}


template<typename T> inline T* tList<T>::Append(const T* item, const T* where)
{
	tAssert(item);
	if (!where)
		Append(item);

	item->PrevItem = where;
	item->NextItem = where->NextItem;
	where->NextItem = item;

	if (item->NextItem)
		item->NextItem->PrevItem = item;
	else
		TailItem = item;

	ItemCount++;
	return item;
}


template<typename T> inline T* tList<T>::Remove()
{
	// It's OK to try to remove from an empty list.
	if (!HeadItem)
		return nullptr;

	T* removed = (T*)HeadItem;
	HeadItem = (T*)HeadItem->NextItem;
	if (!HeadItem)
		TailItem = nullptr;
	else
		HeadItem->PrevItem = nullptr;

	ItemCount--;
	return removed;
}


template<typename T> inline T* tList<T>::Drop()
{
	// It's OK to try to lose something from an empty list.
	if (!TailItem)
		return nullptr;

	T* t = TailItem;

	TailItem = TailItem->PrevItem;
	if (!TailItem)
		HeadItem = nullptr;
	else
		TailItem->NextItem = nullptr;

	ItemCount--;
	return t;
}


template<typename T> inline T* tList<T>::Remove(const T* l)
{
	if (l->PrevItem)
		l->PrevItem->NextItem = l->NextItem;
	else
		HeadItem = l->NextItem;

	if (l->NextItem)
		l->NextItem->PrevItem = l->PrevItem;
	else
		TailItem = l->PrevItem;

	ItemCount--;

	return (T*)l;
}


template<typename T> template<typename CompareFunc> inline int tList<T>::Sort(CompareFunc compare, tListSortAlgorithm algorithm)
{
	switch (algorithm)
	{
		case tListSortAlgorithm::Bubble:
			return SortBubble(compare);
			break;

		case tListSortAlgorithm::Merge:
		default:
			return SortMerge(compare);
			break;
	}

	return 0;
}


template<typename T> template<typename CompareFunc> inline int tList<T>::SortMerge(CompareFunc compare)
{
	if (!HeadItem)
		return 0;

	// Treat every node as a separate list, completely sorted, starting with 1 element each.
	int numNodesPerList = 1;
	int numCompares = 0;

	while (1)
	{
		const T* p = HeadItem;
		HeadItem = nullptr;
		TailItem = nullptr;

		// Num merges in this loop.
		int numMerges = 0;			

		while (p)
		{
			numMerges++;
			const T* q = p;
			int numPNodes = 0;
			for (int i = 0; i < numNodesPerList; i++)
			{
				numPNodes++;
				q = q->Next();
				if (!q)
					break;
			}

			int numQNodes = numNodesPerList;

			// Merge the two lists.
			while (numPNodes > 0 || (numQNodes > 0 && q))
			{
				// Decide whether next tBaseLink of merge comes from p or q.
				const T* e;

				if (numPNodes == 0)
				{
					// p is empty; e must come from q.
					e = q;
					q = q->Next();

					numQNodes--;
				}
				else if (numQNodes == 0 || !q)
				{
					// q is empty; e must come from p.
					e = p;
					p = p->Next();

					numPNodes--;
				}
				else if (++numCompares && !compare(*q, *p))
				{
					// p is lower so e must come from p.
					e = p;
					p = p->Next();
					
					numPNodes--;
				}
				else
				{
					// First gBaseNode of q is bigger or equal; e must come from q.
					e = q;
					q = q->Next();
					
					numQNodes--;
				}

				// add the next gBaseNode to the merged list.
				if (TailItem)
					TailItem->NextItem = e;
				else
					HeadItem = e;

				e->PrevItem = TailItem;
				TailItem = e;
			}

			// P and Q have moved numNodesPerList places along.
			p = q;
		}
		TailItem->NextItem = nullptr;

		// If we have done only one merge, we're all sorted.
		if (numMerges <= 1)
			return numCompares;

		// Otherwise repeat, merging lists twice the size.
		numNodesPerList *= 2;
	}

	return numCompares;
}


template<typename T> template<typename CompareFunc> inline int tList<T>::SortBubble(CompareFunc compare)
{
	// Performs a full bubble sort.
	int numCompares = 0;
	for (int maxCompares = ItemCount - 1; maxCompares >= 1; maxCompares--)
	{
		int numSwaps = BubbleForward(compare, maxCompares);
		numCompares += maxCompares;

		// Early exit detection. If any bubble pass resulted in no swaps, we're done!
		if (!numSwaps)
			return numCompares;
	}

	return numCompares;
}


template<typename T> template<typename CompareFunc> inline int tList<T>::BubbleForward(CompareFunc compare, int maxCompares)
{
	// @todo Can -1 ever happen?
	if (maxCompares == -1)
		maxCompares = ItemCount-1;
	else if (maxCompares > ItemCount-1)
		maxCompares = ItemCount-1;

	// If there are no items, only a single item, or no compares are requested, we're done.
	if ((ItemCount < 2) || (maxCompares == 0))
		return 0;

	const T* a = HeadItem;
	int numSwaps = 0;
	int numCompares = 0;

	while ((a != TailItem) && (numCompares < maxCompares))
	{
		const T* b = a->NextItem;

		// We're sorting from lowest to biggest, so if b < a we need to swap them.
		if (compare(*b, *a))
		{
			// Swap.
			if (a->PrevItem)
				a->PrevItem->NextItem = b;

			if (b->NextItem)
				b->NextItem->PrevItem = a;

			a->NextItem = b->NextItem;
			b->PrevItem = a->PrevItem;

			a->PrevItem = b;
			b->NextItem = a;

			// Fix head and tail if they were involved in the swap.
			if (HeadItem == a)
				HeadItem = b;

			if (TailItem == b)
				TailItem = a;

			// Since we swapped, a is now correctly ready for the next loop.
			numSwaps++;
		}
		else
		{
			a = a->NextItem;
		}
		numCompares++;
	}

	return numSwaps;
}


template<typename T> template<typename CompareFunc> inline int tList<T>::BubbleBackward(CompareFunc compare, int maxCompares)
{
	if (maxCompares == -1)
		maxCompares = ItemCount-1;
	else if (maxCompares > ItemCount-1)
		maxCompares = ItemCount-1;

	// If there are no items, or only a single one, we're done.
	if ((ItemCount < 2) || (maxCompares == 0))
		return 0;

	T* a = TailItem;
	int numSwaps = 0;
	int numCompares = 0;

	while ((a != HeadItem) && (numCompares < maxCompares))
	{
		T* b = a->PrevItem;

		// We're sorting from lowest to biggest, so if a < b we need to swap them.
		if (compare(*a, *b))
		{
			// Swap.
			if (a->NextItem)
				a->NextItem->PrevItem = b;

			if (b->PrevItem)
				b->PrevItem->NextItem = a;

			a->PrevItem = b->PrevItem;
			b->NextItem = a->NextItem;

			a->NextItem = b;
			b->PrevItem = a;

			// Fix head and tail if they were involved in the swap.
			if (HeadItem == b)
				HeadItem = a;

			if (TailItem == a)
				TailItem = b;

			// Since we swapped, a is now correctly ready for the next loop.
			numSwaps++;
		}
		else
		{
			a = a->PrevItem;
		}
		numCompares++;
	}

	return numSwaps;
}


template<typename T> inline T* tListZ<T>::Insert(const T* item)
{
	if (HeadItem)
		HeadItem->PrevItem = item;

	item->NextItem = HeadItem;
	item->PrevItem = nullptr;
	HeadItem = item;
	if (!TailItem)
		TailItem = item;

	ItemCount++;
	return (T*)item;
}


template<typename T> inline T* tListZ<T>::Append(const T* item)
{
	if (TailItem)
		TailItem->NextItem = item;

	item->PrevItem = TailItem;
	item->NextItem = nullptr;
	TailItem = (T*)item;
	if (!HeadItem)
		HeadItem = (T*)item;

	ItemCount++;
	return (T*)item;
}


template<typename T> inline T* tListZ<T>::Insert(const T* item, const T* where)
{
	tAssert(item);
	if (!where)
		Insert(item);

	item->NextItem = where;
	item->PrevItem = where->PrevItem;
	where->PrevItem = item;

	if (item->PrevItem)
		item->PrevItem->NextItem = item;
	else
		HeadItem = item;

	ItemCount++;
	return item;
}


template<typename T> inline T* tListZ<T>::Append(const T* item, const T* where)
{
	tAssert(item);
	if (!where)
		Append(item);

	item->PrevItem = where;
	item->NextItem = where->NextItem;
	where->NextItem = item;

	if (item->NextItem)
		item->NextItem->PrevItem = item;
	else
		TailItem = item;

	ItemCount++;
	return item;
}


template<typename T> inline T* tListZ<T>::Remove()
{
	// It's OK to try to remove from an empty list.
	if (!HeadItem)
		return nullptr;

	T* removed = (T*)HeadItem;
	HeadItem = HeadItem->NextItem;
	if (!HeadItem)
		TailItem = nullptr;
	else
		HeadItem->PrevItem = nullptr;

	ItemCount--;
	return removed;
}


template<typename T> inline T* tListZ<T>::Drop()
{
	// It's OK to try to lose something from an empty list.
	if (!TailItem)
		return nullptr;

	T* t = TailItem;

	TailItem = TailItem->PrevItem;
	if (!TailItem)
		HeadItem = nullptr;
	else
		TailItem->NextItem = nullptr;

	ItemCount--;
	return t;
}


template<typename T> inline T* tListZ<T>::Remove(const T* l)
{
	if (l->PrevItem)
		l->PrevItem->NextItem = l->NextItem;
	else
		HeadItem = l->NextItem;

	if (l->NextItem)
		l->NextItem->PrevItem = l->PrevItem;
	else
		TailItem = l->PrevItem;

	ItemCount--;

	return (T*)l;
}

///////////////////////////////////

template<typename T> inline T* tItList<T>::Remove(Iter& iter)
{
	// It is perfectly valid to try to remove an object referenced by an invalid iterator.
	if (!iter.IsValid() || (this != iter.List))
		return 0;

	IterNode* node = Nodes.Remove(iter.Node);
	T* obj = (T*)node->Object;

	delete node;
	iter.Node = 0;

	return obj;
}
