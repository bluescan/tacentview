// TestFoundation.cpp
//
// Foundation module tests.
//
// Copyright (c) 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tVersion.h>
#include <Foundation/tArray.h>
#include <Foundation/tFixInt.h>
#include <Foundation/tBitField.h>
#include <Foundation/tList.h>
#include <Foundation/tRingBuffer.h>
#include <Foundation/tSort.h>
#include <Foundation/tPriorityQueue.h>
#include <Foundation/tPool.h>
#include "UnitTests.h"
using namespace tStd;
namespace tUnitTest
{


tTestUnit(Types)
{
	tRequire(sizeof(uint8) == 1);
	tRequire(sizeof(uint16) == 2);
	tRequire(sizeof(uint32) == 4);
	tRequire(sizeof(uint64) == 8);

	tRequire(sizeof(int8) == 1);
	tRequire(sizeof(int16) == 2);
	tRequire(sizeof(int32) == 4);
	tRequire(sizeof(int64) == 8);
}
	

tTestUnit(Array)
{
	tArray<int> arr(2, 3);
	arr.Append(1);
	arr.Append(2);

	// Grow by 3.
	arr.Append(3);
	arr.Append(4);
	arr.Append(5);
	
	// Grow by 3.
	arr.Append(6);

	tRequire(arr.GetNumElements() == 6);
	tRequire(arr.GetCapacity() == 8);

	for (int i = 0; i < arr.GetNumElements(); i++)
		tPrintf("Array index %d has value %d\n", i, arr[i]);
	tPrintf("Num items: %d  Max items: %d\n", arr.GetNumElements(), arr.GetCapacity());

	tPrintf("Index 2 value change to 42.\n");
	arr[2] = 42;
	for (int i = 0; i < arr.GetNumElements(); i++)
		tPrintf("Array index %d has value %d\n", i, arr[i]);
	tPrintf("Num items: %d  Max items: %d\n", arr.GetNumElements(), arr.GetCapacity());
	tRequire(arr.GetElements()[2] == 42);
}


// For list testing.
struct Item : public tLink<Item>
{
	Item(int val) : Value(val) { }
	int Value;
};


// For list testing.
bool LessThan(const Item& a, const Item& b)
{
	return a.Value < b.Value;
}


// For list testing.
struct NormItem
{
	NormItem()								: Value(0) { tPrintf("Constructing (Def) NormItem with value %d\n", Value); }
	NormItem(int val)						: Value(val) { tPrintf("Constructing (int) NormItem with value %d\n", Value); }
	NormItem(const NormItem& src)			: Value(src.Value) { tPrintf("Constructing (CC) NormItem with value %d\n", Value); }
	virtual ~NormItem()						{ tPrintf("Destructing NormItem with value %d\n", Value); }
	int Value;
};


bool LessThanNorm(const NormItem& a, const NormItem& b)
{
	return a.Value < b.Value;
}


tTestUnit(List)
{
	tList<Item> itemList(true);
	itemList.Append( new Item(7) );
	itemList.Append( new Item(3) );
	itemList.Append( new Item(4) );
	itemList.Append( new Item(9) );
	itemList.Append( new Item(1) );
	itemList.Append( new Item(5) );
	itemList.Append( new Item(4) );

	tPrintf("Before sorting: ");
	for (const Item* item = itemList.First(); item; item = item->Next())
		tPrintf("%d ", item->Value);
	tPrintf("\n");

	itemList.Sort( LessThan );
	// itemList.Bubble( LessThan , true);

	tPrintf("After sorting: ");
	for (const Item* item = itemList.First(); item; item = item->Next())
		tPrintf("%d ", item->Value);
	tPrintf("\n");

	// Test circular on intrusive lists.
	Item* itm = itemList.First();
	for (int circ = 0; circ < 100; circ++)
		itm = itemList.NextCirc(itm);
	tPrintf("NextCirc Item Val %d\n", itm->Value);
	tRequire(itm->Value == 4);

	for (int circ = 0; circ < 100; circ++)
		itm = itemList.PrevCirc(itm);
	tPrintf("PrevCirc Item Val %d\n", itm->Value);
	tRequire(itm->Value == 1);

	// We need this if we didn't construct this list with a true flag.
	//itemList.Empty();

	// Lets try the same thing with const objects.
	tList<const Item> citemList(true);
	citemList.Append( new Item(7) );
	citemList.Append( new Item(3) );
	citemList.Append( new Item(4) );
	citemList.Append( new Item(9) );
	citemList.Append( new Item(1) );
	citemList.Append( new Item(5) );
	citemList.Append( new Item(4) );

	tPrintf("Before sorting: ");
	for (const Item* item = citemList.First(); item; item = item->Next())
		tPrintf("%d ", item->Value);
	tPrintf("\n");

	citemList.Sort( LessThan );
	//	itemList.Bubble( LessThan , true);

	tPrintf("After sorting: ");
	for (const Item* item = citemList.First(); item; item = item->Next())
		tPrintf("%d ", item->Value);
	tPrintf("\n");

	tItList<NormItem> iterList(true);
	iterList.Append( new NormItem(7) );
	iterList.Append( new NormItem(3) );
	iterList.Append( new NormItem(4) );
	iterList.Append( new NormItem(9) );
	iterList.Append( new NormItem(1) );
	iterList.Append( new NormItem(5) );
	iterList.Append( new NormItem(4) );

	// Test range-based iteration.
	for (NormItem& item : iterList)
	{
		tPrintf("Range-based %d\n", item.Value);
	}

	tPrintf("Iterating forward: ");
	for (tItList<NormItem>::Iter iter = iterList.First(); iter; iter++)
		tPrintf("%d ", (*iter).Value);
	tPrintf("\n");

	tPrintf("Iterating backward: ");
	for (tItList<NormItem>::Iter biter = iterList.Tail(); biter; --biter)
		tPrintf("%d ", (*biter).Value);
	tPrintf("\n");

	tItList<NormItem>::Iter lastIter = iterList.Last();
	NormItem ni = iterList[lastIter];
	tPrintf("Last NormItem: %d\n", ni.Value);

	for (int i = 0; i < 10; i++)
	{
		lastIter.NextCirc();
		tPrintf("NextCirc: %d\n", lastIter.GetObject()->Value);
	}

	iterList.Sort(LessThanNorm);
	tPrintf("AfterSorting:\n");
	for (tItList<NormItem>::Iter iter = iterList.First(); iter; iter++)
		tPrintf("%d ", (*iter).Value);
	tPrintf("\n");

	// We need this if we didn't construct this list with a true flag.
	// iterList.Empty();
}


bool IntLess(const int& a, const int& b) { return (a < b); }


tTestUnit(Sort)
{
	int arr[] = { 5, 32, 7, 9, 88, 32, -3, 99, 55 };
	tPrintf("Before sorting:\n");
	for (int i = 0; i < sizeof(arr)/sizeof(*arr); i++)
		tPrintf("%d, ", arr[i]);
	tPrintf("\n");

	tSort::tShell(arr, sizeof(arr)/sizeof(*arr), IntLess);
	//tSort::tInsertion(arr, sizeof(arr)/sizeof(*arr), IntLess);
	//tSort::tQuick(arr, sizeof(arr)/sizeof(*arr), IntLess);

	tPrintf("After sorting:\n");
	for (int i = 0; i < sizeof(arr)/sizeof(*arr); i++)
		tPrintf("%d, ", arr[i]);
	tPrintf("\n");
	
	tRequire(arr[0] <= arr[1]);
	tRequire(arr[1] <= arr[2]);
	tRequire(arr[6] <= arr[7]);
	tRequire(arr[7] <= arr[8]);
}


tTestUnit(FixInt)
{
	// @todo Add a bunch of tRequire calls.
	tuint256 uvalA = 42;
	tuint256 uvalB(uvalA);
	tint256 valA(99);
	tuint256 uvalC(valA);
	tuint256 uvalD("FE", 16);
	tuint256 uvalE(uint16(88));
	uvalD.Set(uvalE);
	uvalC.Set(valA);
	uvalD.Set("808");
	int8 int8Val = int8(uvalE);
	float floatVal = float(uvalE);
	uvalE = uvalA;

	uvalD.MakeZero();
	uvalD.MakeMaxInt();
	uvalA += 2;
	tPrintf("%064|256X\n", uvalA);
	tPrintf("%064|256X\n", uvalB);
	tint256 valB = uvalB.AsSigned();
	const tint256 valC = uvalC.AsSigned();
	uvalA.ClearBit(0);
	uvalA.SetBit(0);
	uvalA.ToggleBit(0);
	bool bval = uvalA.GetBit(0);

	uvalA /= 10;
	tPrintf("%064|256X\n", uvalA);

	uvalC = tDivide(uvalA, uvalB);
	tPrintf("%064|256X\n", uvalA);

	uvalA &= tuint256(12);
	uvalA |= tuint256(12);
	uvalA ^= tuint256(12);
	uvalA >>= 2;
	uvalA <<= 4;
	uvalA += 8;
	uvalA += uvalB;
	uvalA -= 4;
	uvalA -= uvalB;
	uvalA /= tuint256(12);
	uvalA %= tuint256(20);
	uvalA = uvalA >> 2;
	uvalA = uvalA << 4;
	tPrintf("%064|256X\n", uvalA);

	uvalD *= 32;
	uvalD *= tuint256(3);
	if (uvalA < 25)
		tPrintf("Small\n");

	bval = uvalA == uvalB;
	bval = uvalA != uvalB;
	bval = uvalA < uvalB;
	bval = uvalA > uvalB;
	uvalB = uvalC & uvalD;
	uvalC++;
	++uvalC;
	bval = uvalC;
	bval = !uvalC;
	uvalA = ~uvalB;
	uvalB = -uvalB;
	uvalC = +uvalB;
	uvalD = tSqrt(uvalC);
	uvalE = tCurt(uvalD);
	uvalA = tFactorial(uvalE);

	int32 val = 0;
	val = tStrtoi32("0xFD");
	tPrintf("0xFD %d\n", val);

	val = tStrtoi32("#A001");
	tPrintf("#A001 %d\n", val);

	val = tStrtoi32("B01012");
	tPrintf("%b\n", val);

	val = tStrtoi32("0d-88");
	tPrintf("%b\n", val);

	uvalA.Set("0xabcd");
	tPrintf("%064|256X\n", uvalA);

	uvalA.RotateRight(3);

	tint256 a = 100;
	tint256 b = 11;
	tDivide(a, b);
	tDivide(a, 15);
}


tTestUnit(Bitfield)
{
	tString result;

	tbit128 a("0XAAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD");
	tPrintf("A: %032|128X\n", a);
	tsPrintf(result, "A: %032|128X", a);
	tRequire(result == "A: AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD");

	a.Set("FF", 16);
	tPrintf("A: %032|128X\n", a);
	tsPrintf(result, "A: %032|128X", a);
	tRequire(result == "A: 000000000000000000000000000000FF");

	tBitField<30> b;
	b.Set("0xCCCC12FF");
	tsPrintf(result, "%08|32X", b);
	tRequire(result == "0CCC12FF");

	b.Set("0xFCCC12FF");
	tsPrintf(result, "%08|32X", b);
	tRequire(result == "3CCC12FF");

	tBitField<33> bitset33;
	tStaticAssert(sizeof(bitset33) == 8);
	tPrintf("Setting 33 bitset to: decimal %s\n", "323456789");

	// Since there is no base prefix in the string, the Set call assumes base 10. Note that the
	// GetAsString calls default to base 16!
	bitset33.Set("323456789", 10);
	tPrintf("bitset 33 was set to: 0x%s\n", bitset33.GetAsString().Pod());
	tPrintf("bitset 33 was set to: d%s\n", bitset33.GetAsString(10).Pod());
	tRequire(bitset33.GetAsString() == "13478F15");
	tRequire(bitset33.GetAsString(10) == "323456789");

	tBitField<10> bitset10;
	bitset10.SetAll(true);
	tPrintf("bitset10 SetAll yields: d%s\n", bitset10.GetAsString(10).Pod());
	tRequire(bitset10.GetAsString(10) == "1023");

	tBitField<12> bitset12;
	bitset12.Set("abc");
	tPrintf("bitset12: %s\n", bitset12.GetAsString().Pod());
	tRequire(bitset12.GetAsString() == "ABC");

	bitset12 >>= 4;
	tPrintf("bitset12: %s\n", bitset12.GetAsString().Pod());
	tRequire(bitset12.GetAsString() == "AB");

	bitset12 <<= 4;
	tPrintf("bitset12: %s\n", bitset12.GetAsString().Pod());
	tRequire(bitset12.GetAsString() == "AB0");

	tBitField<12> bitsetAB0("AB0");
	tPrintf("bitsetAB0 == bitset12: %s\n", (bitsetAB0 == bitset12) ? "true" : "false");
	tRequire(bitsetAB0 == bitset12);
	tRequire(!(bitsetAB0 != bitset12));

	tBitField<17> bitset17;
	bitset17.SetBit(1, true);
	if (bitset17)
		tPrintf("bitset17: %s true\n", bitset17.GetAsString().Pod());
	else
		tPrintf("bitset17: %s false\n", bitset17.GetAsString().Pod());
	tRequire(bitset17);

	bitset17.InvertAll();
	tPrintf("bitset17: after invert: %s\n", bitset17.GetAsString().Pod());
	tRequire(bitset17.GetAsString() == "1FFFD");
}


tTestUnit(String)
{
	// Testing the string substitution code.
	tString src("abc1234abcd12345abcdef123456");
	tPrintf("Before: '%s'\n", src.ConstText());
	src.Replace("abc", "cartoon");
	tPrintf("Replacing abc with cartoon\n");
	tPrintf("After : '%s'\n\n", src.ConstText());
	tRequire(src == "cartoon1234cartoond12345cartoondef123456");

	src = "abc1234abcd12345abcdef123456";
	tPrintf("Before: '%s'\n", src.ConstText());
	src.Replace("abc", "Z");
	tPrintf("Replacing abc with Z\n");
	tPrintf("After : '%s'\n\n", src.ConstText());
	tRequire(src == "Z1234Zd12345Zdef123456");

	src = "abcabcabc";
	tPrintf("Before: '%s'\n", src.ConstText());
	src.Replace("abc", "");
	tPrintf("Replacing abc with \"\"\n");
	tPrintf("After : '%s'\n\n", src.ConstText());
	tRequire(src == "");

	src = "abcabcabc";
	tPrintf("Before: '%s'\n", src.ConstText());
	src.Replace("abc", 0);
	tPrintf("Replacing abc with null\n");
	tPrintf("After : '%s'\n\n", src.ConstText());
	tRequire(src == "");

	src.Clear();
	tPrintf("Before: '%s'\n", src.ConstText());
	src.Replace("abc", "CART");
	tPrintf("Replacing abc with CART\n");
	tPrintf("After : '%s'\n\n", src.ConstText());
	tRequire(src == "");

	tPrintf("Testing Explode:\n");
	tString src1 = "abc_def_ghi";
	tString src2 = "abcXXdefXXghi";
	tPrintf("src1: %s\n", src1.ConstText());
	tPrintf("src2: %s\n", src2.ConstText());

	tList<tStringItem> exp1(true);
	tList<tStringItem> exp2(true);
	int count1 = tExplode(exp1, src1, '_');
	int count2 = tExplode(exp2, src2, "XX");

	tPrintf("Count1: %d\n", count1);
	for (tStringItem* comp = exp1.First(); comp; comp = comp->Next())
		tPrintf("   Comp: '%s'\n", comp->ConstText());

	tPrintf("Count2: %d\n", count2);
	for (tStringItem* comp = exp2.First(); comp; comp = comp->Next())
		tPrintf("   Comp: '%s'\n", comp->ConstText());

	tList<tStringItem> expl(true);
	tString exdup = "abc__def_ghi";
	tExplode(expl, exdup, '_');
	tPrintf("Exploded: ###%s### to:\n", exdup.ConstText());
	for (tStringItem* comp = expl.First(); comp; comp = comp->Next())
		tPrintf("   Comp:###%s###\n", comp->ConstText());

	tList<tStringItem> expl2(true);
	tString exdup2 = "__a__b_";
	tExplode(expl2, exdup2, '_');
	tPrintf("Exploded: ###%s### to:\n", exdup2.ConstText());
	for (tStringItem* comp = expl2.First(); comp; comp = comp->Next())
		tPrintf("   Comp:###%s###\n", comp->ConstText());

	tString aa("aa");
	tString exaa = aa.ExtractFirstWord('a');
	tPrintf("\n\naa extract first word to a: Extracted:###%s###  Left:###%s###\n", exaa.ConstText(), aa.ConstText());
}


tTestUnit(RingBuffer)
{
	// We're going to use the middle 10 characters as our buffer and write a pattern into the full buffer
	// to check for over and under-runs.
	char buf[14];
	char rem[14];
	char rm;
	bool ok;
	tMemset(buf, '.', 14);
	tRingBuffer<char> ring(buf+2, 10);
	auto printBuf = [&]()
	{
		tPrintf("Buf: ");
		for (int c = 0; c < 14; c++)
			tPrintf("%c", buf[c]);
		tPrintf("\n     ");
		for (int c = 0; c < 14; c++)
		{
			char v = ' ';
			if ((c>=2) && (c<12))
			{
				if ((ring.GetHeadIndex() != -1) && (ring.GetHeadIndex()==c-2))
					v = 'H';
				if ((ring.GetTailIndex() != -1) && (ring.GetTailIndex()==c-2))
					{ if (v == 'H') v = 'B'; else v = 'T'; }
			}

			tPrintf("%c", v);
		}
		tPrintf("\n");
	};

	printBuf();
	tPrintf("\n");

	tPrintf("Append abcd\n");
	int numApp = ring.Append("abcdefghijkl", 4);	printBuf();
	tPrintf("Appended %d items\n\n", numApp);

	tPrintf("Remove 2\n");
	int numRem = ring.Remove(rem, 2);				printBuf();
	tPrintf("Removed %d items\n\n", numRem);

	tPrintf("Remove 1\n");
	ring.Remove(rm);								printBuf();
	tPrintf("Removed %c\n\n", rm);

	tPrintf("Append efghijkl\n");
	numApp = ring.Append("efghijkl", 8);			printBuf();
	tPrintf("Appended %d items\n\n", numApp);

	for (int r = 0; r < 11; r++)
	{
		tPrintf("Remove 1\n");
		ok = ring.Remove(rm);						printBuf();
		if (ok) tPrintf("Removed %c\n", rm);
	}
	tPrintf("\n");
}


tTestUnit(PriorityQueue)
{
	tPQ<int> Q(2, 2);
	int data = 42;

	Q.Insert( tPQ<int>::tItem(data, 7) );
	Q.Insert( tPQ<int>::tItem(data, 24) );
	Q.Insert( tPQ<int>::tItem(data, 2) );
	Q.Insert( tPQ<int>::tItem(data, 16) );
	Q.Insert( tPQ<int>::tItem(data, 24) );
	Q.Insert( tPQ<int>::tItem(data, 3) );
	Q.Insert( tPQ<int>::tItem(data, 1) );
	Q.Insert( tPQ<int>::tItem(data, 0) );
	Q.Insert( tPQ<int>::tItem(data, 43) );
	Q.Insert( tPQ<int>::tItem(data, 16) );

	tPrintf("GetMin %d\n", Q.GetMin().Key);
	tRequire(Q.GetNumItems() == 10);
	for (int i = 0; i < 10; i++)
		tPrintf("ExtractMin %d\n", Q.GetRemoveMin().Key);
}


tTestUnit(MemoryPool)
{
	tPrintf("Sizeof (uint8*): %d\n", sizeof(uint8*));

	int bytesPerItem = 2;
	int initNumItems = 4;
	int growNumItems = 3;
	bool threadSafe = true;

	tMem::tFastPool memPool(bytesPerItem, initNumItems, growNumItems, threadSafe);

	void* memA = memPool.Malloc();
	tPrintf("memA: %08X\n", memA);
	tRequire(memA);

	void* memB = memPool.Malloc();
	tPrintf("memB: %08X\n", memB);
	tRequire(memB);

	void* memC = memPool.Malloc();
	tPrintf("memC: %08X\n", memC);
	tRequire(memC);

	void* memD = memPool.Malloc();
	tPrintf("memD: %08X\n", memD);
	tRequire(memD);

	// Now a grow should happen.
	void* memE = memPool.Malloc();
	tPrintf("memE: %08X\n", memE);
	tRequire(memE);
	tRequire(memPool.GetNumExpansionBlocks() == 1);

	void* memF = memPool.Malloc();
	tPrintf("memF: %08X\n", memF);
	tRequire(memF);

	void* memG = memPool.Malloc();
	tPrintf("memG: %08X\n", memG);
	tRequire(memG);

	// And another grow.
	void* memH = memPool.Malloc();
	tPrintf("memH: %08X\n", memH);
	tRequire(memH);
	tRequire(memPool.GetNumExpansionBlocks() == 2);

	// Try to allocate something too big.
	void* tooBig = memPool.Malloc(9);
	tPrintf("tooBig: %08X\n", tooBig);
	tRequire(tooBig == nullptr);
	tRequire(memPool.GetNumAllocations() == 8);

	tPrintf("free B, free E\n");
	memPool.Free(memB);
	memPool.Free(memE);
	tRequire(memPool.GetNumAllocations() == 6);

	memE = memPool.Malloc();
	tPrintf("memE: %08X\n", memE);
	tRequire(memE);

	memB = memPool.Malloc();
	tPrintf("memB: %08X\n", memB);
	tRequire(memB);
	tRequire(memPool.GetNumAllocations() == 8);

	memPool.Free(memA);
	memPool.Free(memB);
	memPool.Free(memC);
	memPool.Free(memD);
	memPool.Free(memE);
	memPool.Free(memF);
	memPool.Free(memG);
	memPool.Free(memH);
	tRequire(memPool.GetNumAllocations() == 0);
}


}
