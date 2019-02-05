//
// tSort.h
//
// Basic stable and unstable sort for arrays of objects. Both sorting functions require a compare function to be
// provided. If the type you are passing has a corresponding operator< (less) you can use that implicetly by not
// supplying a compare function. The default compare function will call '<' for you. If compare implements
// less-than, the array will be sorted in ascending order. Greater-than will reverse the order.
//
// Copyright (c) 2004-2006, 2015 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once


namespace tSort
{
	template<typename T> bool tCompLess(const T& a, const T& b)															{ return (a < b) ? true : false; }
	template<typename T> bool tCompGreater(const T& a, const T& b)														{ return (a > b) ? true : false; }

	// Insertion sort. O(n^2) in worst case. Faster on already sorted data. This function is stable in that objects with
	// equal values will not be reordered.
	template<typename T> void tInsertion(T array[], int numItems, bool compare(const T& a, const T& b) = tCompLess<T>);

	// Shellsort. Fast when number of items is small. Not stable. See https://en.wikipedia.org/wiki/Shellsort
	template<typename T> void tShell(T array[], int numItems, bool compare(const T& a, const T& b) = tCompLess<T>);

	// Quicksort. O(n log n). Not stable but fast. Good with unordered data. Uses shellsort when numItems < 32.
	template<typename T> void tQuick(T array[], int numItems, bool compare(const T& a, const T& b) = tCompLess<T>);

	// @todo Bubble sort, merge sort, or maybe selection sort instead of merge.
}


// Implementation below this line.


template<typename T> inline void tSort::tInsertion(T A[], int N, bool compare(const T& a, const T& b))
{
	for (int i = 1; i < N; i++)
	{
		const T value = A[i];
		int j = i - 1;

		for (; j >= 0 && compare(value, A[j]); j--)
			A[j+1] = A[j];

		A[j+1] = value;
	}
}


template<typename T> inline void tSort::tShell(T A[], int N, bool compare(const T& a, const T& b))
{
	// This code is a conversion of the pseudocode found on the Wikipedia page for shellsort: https://en.wikipedia.org/wiki/Shellsort
	// It uses Shell's original gap sequence. We start with the largest gap and work down to a gap of 1.
	for (int gap = N/2; gap; gap /= 2)
	{
		// Do a gapped insertion sort for this gap size.
		// The first gap elements A[0..gap-1] are already in gapped order.
		// Keep adding one more element until the entire array is gap sorted.
		for (int i = gap; i < N; i++)
		{
			// Add arr[i] to the elements that have been gap sorted.
			// Save arr[i] in temp and make a hole at position i.
			T temp = A[i];

			// Shift earlier gap-sorted elements up until the correct location for a[i] is found.
			// Since we wanted ascending order for a less-than compare, I reversed the order of the compare args vs the Wikipedia article.
			int j = i;
			for (; (j >= gap) && compare(temp, A[j - gap]); j -= gap)
				A[j] = A[j - gap];

			// Put temp (the original arr[i]) in its correct location.
		    A[j] = temp;
        }
    }
}


namespace tSort
{
	template<typename T> void tQuickRec(T* left, T* right, bool compare(const T& a, const T& b) = tCompLess<T>);
}


template<typename T> inline void tSort::tQuickRec(T* left, T* right, bool compare(const T& a, const T& b))
{
	int numItems = int(right - left) + 1;

	// If there are only a few items in the list we use shellsort.
	const bool useShell = true;
	if (useShell)
	{
		const int shellItemThreshold = 32;
		if (numItems < shellItemThreshold)
		{
			tShell(left, numItems, compare);
			return;
		}
	}
	else
	{
		if (numItems <= 1)
			return;
	}

	// Midpoint.
	T* mid = left + (right-left)/2;

	if (compare(*mid, *left))
		tStd::tSwap(*mid, *left);
	if (compare(*right, *mid))
		tStd::tSwap(*right, *mid);
	if (compare(*mid, *left))
		tStd::tSwap(*mid, *left);

	// Pivot.
	const T pivot = *mid;
	T* i = left;
	T* j = right;

	while (i <= j)
	{
		while (compare(*i, pivot) && i < right)
			i++;

		while (compare(pivot, *j) && left < j)
			j--;

		if (i <= j)
		{
			tStd::tSwap(*i, *j);
			i++, j--;
		}
	}

	// Recurse.
	if (left < j)
		tQuickRec(left, j, compare);

	if (i < right)
		tQuickRec(i, right, compare);
}


template<typename T> inline void tSort::tQuick(T A[], int N, bool compare(const T& a, const T& b))
{
	tQuickRec<T>(A, A + N - 1, compare);
}
