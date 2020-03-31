// tBitField.h
//
// A tBitField is a fixed size array of bits. Similar to STL bitset class. An tBitField needs to know how many bits
// will be stored at compile time and there is no possibility to grow or dynamically change that number. All bitwise
// operators are overloaded appropriately. This class is perfect for flags where a uint32 or uint64 is not enough.
// If you need mathematical operators like subtraction, addition, multiplication, division etc, use the heavier
// tFixInt instead. If you need a dynamic number of bits, use a tBitArray instead. If you don't, this will be faster.
//
// Copyright (c) 2004-2006, 2015, 2017, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include "Foundation/tFixInt.h"
#include "Foundation/tString.h"


// The tBitField class. NumBits represents the number of bits that can be stored by the instance. There are no
// conditions on the value of NumBits used and long as it is a whole number. If NumBits is a multiple of 4, the
// tBitField will be supported by tPrintf so just call something like: tsPrintf(s, "%032|128X", bitvar128) to convert it
// to a string. The memory image size taken up will always be a multiple of 4 bytes. ex: sizeof(tBitField<16>) = 4 and
// sizeof(tBitField<33>) = 8. You can still use tPrintf on a 33-bit bit-field, just be aware of the size. Any padding
// bits are guaranteed to be clear in the internal representation and when saved to a chunk (disk) format.
template<int NumBits> class tBitField
{
public:
	tBitField()												/* All bits cleared. */										{ Clear(); }

	// Disabled CopyCons so class remains a POD-type. Allows it to be passed to tPrintf for non MSVC compilers.
	// tBitField(const tBitField& src)																					{ for (int i = 0; i < NumElements; i++) Elements[i] = src.Elements[i]; }

	tBitField(const tString& str, int base = -1)																		{ Set(str); }
	tBitField(int val)																									{ Set(val); }

	// Power-of-2 constructors from 8 to 256 bits.
	tBitField(uint8 val)																								{ Set(val); }
	tBitField(uint16 val)																								{ Set(val); }
	tBitField(uint32 val)																								{ Set(val); }
	tBitField(uint64 val)																								{ Set(val); }
	tBitField(uint64 msb, uint64 lsb)																					{ Set(msb, lsb); }
	tBitField(uint64 msb, uint64 hb, uint64 lb, uint64 lsb)																{ Set(msb, hb, lb, lsb); }

	// Array constructors.
	tBitField(const uint8* src, int len)					/* See Set(const uint8*, int) */							{ Set(src, len); }
	tBitField(const uint16* src, int len)					/* See Set(const uint8*, int) */							{ Set(src, len); }
	tBitField(const uint32* src, int len)					/* See Set(const uint8*, int) */							{ Set(src, len); }
	tBitField(const uint64* src, int len)					/* See Set(const uint8*, int) */							{ Set(src, len); }

	// The supplied string should follow the formatting characteristics of the tStrtoi functions. If the base is -1 (or
	// not between 2 and 36) the function looks for the following prefixes to determine the base:
	//
	// Base 16 prefixes: x X 0x 0X #
	// Base 10 prefixes: d D 0d 0D
	// Base 8  prefixes: o O 0o 0O @
	// Base 4  prefixes: n N 0n 0N (N for Nibble)
	// Base 2  prefixes: b B 0b 0B !
	//
	// If the prefix is missing (and base == -1), it assumes base 10. Note the default base is 16. This is because this
	// is a bit-field so hex seems more natural.
	void Set(const char*, int base = 16);

	// Sets the bitfield from the supplied data. Asserts if the bit-field is not big enough. The bitfield is allowed to
	// be bigger, in which case zeroes are set in the most-sig bits.
	void Set(int val)																									{ Set(uint32(val)); }
	void Set(uint8 val);
	void Set(uint16 val);
	void Set(uint32 val);
	void Set(uint64 val);
	void Set(uint64 msb, uint64 lsb);
	void Set(uint64 msb, uint64 hb, uint64 lb, uint64 lsb);
	void Set(const uint8* src, int len);
	void Set(const uint16* src, int len);
	void Set(const uint32* src, int len);
	void Set(const uint64* src, int len);

	// Gets the bit-field as a numeric string in the base specified. You can also use tPrintf, if you need leading zeroes
	// or more control over formatting. This function handles arbitrary bases, however. base E [2, 36]
	tString GetAsString(int base = 16) const;

	// Gets the n'th bit as a bool. Zero-based index where zero is the least significant binary digit.
	bool GetBit(int n) const;
	
	// Sets the n'th bit to val. Zero-based index where zero is least significant binary digit.
	void SetBit(int n, bool val);

	void Clear()																										{ for (int i = 0; i < NumElements; i++) Elements[i] = 0x00000000; }
	void SetAll(bool val = true);
	void InvertAll();
	bool AreAll(bool val) const;							/* Checks if all bits are set to val. */
	int GetNumBitsgth() const								/* Returns the number of bits stored by the bit-field. */	{ return NumBits; }
	int Count(bool val) const;								/* Returns the number of bits that match val. */

	// These deal with the raw uint32 elements that represent the bit array.
	int GetNumElements() const								/* Returns how many uint32s are used for the bit array. */	{ return NumElements; }
	void GetElements(uint32* dest) const					/* Least significant at the beginning. */					{ tAssert(dest); tMemcpy(dest, Elements, NumElements*4); }
	uint32* GetElements() const																							{ return (uint32*)Elements; }
	void SetElements(const uint32* src)						/* Least sig at the beginning. Clears unused bits. */		{ tAssert(src); tMemcpy(Elements, src, NumElements*4); ClearUnusedBits(); }
	uint32& GetElement(int i)																							{ return Elements[i]; }

	tBitField& operator=(const tBitField& s)																			{ if (this == &s) return *this; tStd::tMemcpy(Elements, s.Elements, sizeof(Elements)); return *this; }

	// We ensure and assume any pad bits are clear. Since 0 &|^ 0 = 0, we don't need to bother clearing any pad bits.
	tBitField& operator&=(const tBitField& s)																			{ for (int i = 0; i < NumElements; i++) Elements[i] &= s.Elements[i]; return *this; }
	tBitField& operator|=(const tBitField& s)																			{ for (int i = 0; i < NumElements; i++) Elements[i] |= s.Elements[i]; return *this; }
	tBitField& operator^=(const tBitField& s)																			{ for (int i = 0; i < NumElements; i++) Elements[i] ^= s.Elements[i]; return *this; }

	// The pad bits are always zeroed when left shifting.
	tBitField& operator<<=(int);
	tBitField operator<<(int s) const																					{ tBitField<NumBits> set(*this); set <<= s; return set; }
	tBitField& operator>>=(int);
	tBitField operator>>(int s) const																					{ tBitField<NumBits> set(*this); set >>= s; return set; }
	tBitField operator~() const																							{ tBitField set(*this); set.InvertAll(); return set; }

	bool operator[](int n) const																						{ return GetBit(n); }
	bool operator==(const tBitField&) const;
	bool operator!=(const tBitField&) const;
	operator bool() const;

private:
	// The tBitField guarantees clear bits in the internal representation if the number of bits is not a multiple of 32
	// (which is our internal storage type size). This function clears them (and only them).
	void ClearUnusedBits();

	const static int NumElements = (NumBits >> 5) + ((NumBits % 32) ? 1 : 0);

	// The bit-field is stored in an array of uint32s called elements. Any pad bits are set to 0 at all times. The
	// elements at smaller array indexes store less significant digits than the ones at larger indexes.
	uint32 Elements[NumElements];
};


// These commutative binary operators belong _outside_ of the tBitField class for good OO reasons. i.e. They are global
// operators that act on bit-fields. They belong outside in the same way that operator+ does for regular integral types.
template<int NumBits> inline tBitField<NumBits> operator&(const tBitField<NumBits>& a, const tBitField<NumBits>& b)		{ tBitField<NumBits> set(a); set &= b; return set; }
template<int NumBits> inline tBitField<NumBits> operator|(const tBitField<NumBits>& a, const tBitField<NumBits>& b)		{ tBitField<NumBits> set(a); set |= b; return set; }
template<int NumBits> inline tBitField<NumBits> operator^(const tBitField<NumBits>& a, const tBitField<NumBits>& b)		{ tBitField<NumBits> set(a); set ^= b; return set; }


// The tbitNNN types represent convenient bit-field sizes. They can represent large sets of bits and even allow bit
// operations. They are a little slower that native 32 or 64 bit integers and do not support many math operations. For
// full arithmetic support in a 128-bit or larger integer consider using the tFixInt class. Note that since bit-fields
// do not support arithmetic, they should be considered unsigned at all times. For example, the right shift >> operator
// does not sign extend.
typedef tBitField<128> tbit128;
typedef tBitField<256> tbit256;
typedef tBitField<512> tbit512;


// Implementation below this line.


template<int N> inline void tBitField<N>::Set(const char* str, int base)
{
	tFixIntU< NumElements*32 > val;
	val = tStd::tStrtoiT< tFixIntU< NumElements*32 > >(str, base);
	tAssert(NumElements == val.GetRawCount());
	for (int i = 0; i < NumElements; i++)
		Elements[i] = val.RawElement(i);

	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(uint8 val)
{
	Clear();
	tAssert(NumElements >= 1);
	Elements[0] = uint32(val);
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(uint16 val)
{
	Clear();
	tAssert(NumElements >= 1);
	Elements[0] = uint32(val);
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(uint32 val)
{
	Clear();
	tAssert(NumElements >= 1);
	Elements[0] = val;
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(uint64 val)
{
	Clear();
	tAssert(NumElements >= 2);
	Elements[0] = val & 0x00000000FFFFFFFFull;
	Elements[1] = (val >> 32) & 0x00000000FFFFFFFFull;
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(uint64 msb, uint64 lsb)
{
	Clear();
	tAssert(NumElements >= 4);
	Elements[0] = lsb & 0x00000000FFFFFFFFull;
	Elements[1] = (lsb >> 32) & 0x00000000FFFFFFFFull;
	Elements[2] = msb & 0x00000000FFFFFFFFull;
	Elements[3] = (msb >> 32) & 0x00000000FFFFFFFFull;
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(uint64 msb, uint64 hb, uint64 lb, uint64 lsb)
{
	Clear();
	tAssert(NumElements >= 8);
	Elements[0] = lsb & 0x00000000FFFFFFFFull;
	Elements[1] = (lsb >> 32) & 0x00000000FFFFFFFFull;
	Elements[2] = lb & 0x00000000FFFFFFFFull;
	Elements[3] = (lb >> 32) & 0x00000000FFFFFFFFull;
	Elements[4] = hb & 0x00000000FFFFFFFFull;
	Elements[5] = (hb >> 32) & 0x00000000FFFFFFFFull;
	Elements[6] = msb & 0x00000000FFFFFFFFull;
	Elements[7] = (msb >> 32) & 0x00000000FFFFFFFFull;
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(const uint8* src, int len)
{
	Clear();
	tAssert(NumElements*4 >= len);
	for (int i = len-1; i >= 0; i--)
	{
		int j = (len-1) - i;
		Elements[j/4] |= src[j] << ((j%4)*8);
	}
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(const uint16* src, int len)
{
	Clear();
	tAssert(NumElements*2 >= len);
	for (int i = len-1; i >= 0; i--)
	{
		int j = (len-1) - i;
		Elements[j/2] |= src[j] << ((j%2)*16);
	}
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(const uint32* src, int len)
{
	Clear();
	tAssert(NumElements >= len);
	for (int i = len-1; i >= 0; i--)
	{
		int j = (len-1) - i;
		Elements[j] = src[j];
	}
	ClearUnusedBits();
}


template<int N> inline void tBitField<N>::Set(const uint64* src, int len)
{
	Clear();
	tAssert(NumElements >= len*2);
	for (int i = len-1; i >= 0; i--)
	{
		int j = (len-1) - i;
		Elements[j*2]	= src[j] & 0x00000000FFFFFFFFull;
		Elements[j*2+1]	= (src[j] >> 32) & 0x00000000FFFFFFFFull;
	}
	ClearUnusedBits();
}


template<int N> inline tString tBitField<N>::GetAsString(int base) const
{
	tFixIntU< NumElements*32 > val;
	tAssert(NumElements == val.GetRawCount());
	for (int i = 0; i < NumElements; i++)
		val.RawElement(i) = Elements[i];

	// Worst case for string length required is base 2, where N characters are needed.
	tString str(N);
	tStd::tItostrT< tFixInt< NumElements*32 > >(val, str.Text(), N+1, base);
	return str;
}


template<int N> inline bool tBitField<N>::GetBit(int n) const
{
	tAssert(n < N);
	int i = n >> 5;
	int d = n & 0x1F;
	uint32 mask = 1 << d;
	return (Elements[i] & mask) ? true : false;
}


template<int N> inline void tBitField<N>::SetBit(int n, bool v)
{
	tAssert(n < N);
	int i = n >> 5;
	int d = n & 0x1F;
	uint32 mask = 1 << d;
	if (v)
		Elements[i] |= mask;
	else
		Elements[i] &= ~mask;
}


template<int N> inline void tBitField<N>::SetAll(bool v)
{
	for (int i = 0; i < NumElements; i++)
		Elements[i] = v ? 0xFFFFFFFF : 0x00000000;

	if (v)
		ClearUnusedBits();
}


template<int N> inline void tBitField<N>::InvertAll()
{
	for (int i = 0; i < NumElements; i++)
		Elements[i] = ~(Elements[i]);

	ClearUnusedBits();
}


template<int N> inline bool tBitField<N>::AreAll(bool v) const
{
	// To test all clear we rely on any extra bits being cleared as well.
	if (!v)
	{
		for (int i = 0; i < NumElements; i++)
			if (Elements[i] != 0x00000000)
				return false;
		return true;
	}

	for (int i = 0; i < NumElements-1; i++)
		if (Elements[i] != 0xFFFFFFFF)
			return false;

	// The last slot in the array may not be full.
	int last = N & 0x1F;
	uint32 maxFull = (last ? (1<<last) : 0) - 1;

	return (Elements[NumElements-1] == maxFull) ? true : false;
}


template<int N> inline int tBitField<N>::Count(bool v) const
{
	// First compute the total number set.
	int numSet = 0;
	for (int i = 0; i < NumElements; ++i)
	{
		uint32 v = Elements[i];			// Count the number of bits set in v.
		uint32 c;						// c accumulates the total bits set in v.
		for (c = 0; v; c++)
			v &= v - 1;					// Clear the least significant bit set.
		numSet += c;
	}

	// Now numSet is correct. If that's what we were asked, we're done. If not, we just subtract.
	if (v)
		return numSet;

	return N - numSet;
}


template<int N> inline tBitField<N>& tBitField<N>::operator<<=(int s)
{
	// First, if we are shifting by too much, we end up with all zeroes.
	tAssert(s >= 0);
	if (s >= N)
	{
		Clear();
		return *this;
	}

	// Now lets make an uber-set with zeroes to the right.
	uint32 uber[NumElements*2];
	for (int i = 0; i < NumElements; i++)
		uber[i] = 0x00000000;

	for (int i = 0; i < NumElements; i++)
		uber[i+NumElements] = Elements[i];

	int bitIndex = NumElements*32;
	bitIndex -= s;

	// Finally copy the bits over from the new starting position. This might be a bit slow, but it works.
	Clear();
	for (int b = 0; b < N; b++, bitIndex++)
	{
		// Read.
		int i = bitIndex >> 5;
		int d = bitIndex & 0x1F;
		uint32 mask = 1 << d;
		bool val = (uber[i] & mask) ? true : false;

		// Write.
		SetBit(b, val);
	}

	return *this;
}


template<int N> inline tBitField<N>& tBitField<N>::operator>>=(int s)
{
	// First, if we are shifting by too much, we end up with all zeroes.
	tAssert(s >= 0);
	if (s >= N)
	{
		Clear();
		return *this;
	}

	// Now lets make an uber-set with zeroes to the left.
	uint32 uber[NumElements*2];
	for (int i = 0; i < NumElements; i++)
		uber[i+NumElements] = 0x00000000;

	for (int i = 0; i < NumElements; i++)
		uber[i] = Elements[i];

	int bitIndex = 0;
	bitIndex += s;

	// Finally copy the bits over from the new starting position. This might be a bit slow, but it works.
	Clear();
	for (int b = 0; b < N; b++, bitIndex++)
	{
		// Read.
		int i = bitIndex >> 5;
		int d = bitIndex & 0x1F;
		uint32 mask = 1 << d;
		bool val = (uber[i] & mask) ? true : false;

		// Write.
		SetBit(b, val);
	}

	return *this;
}


template<int N> inline bool tBitField<N>::operator==(const tBitField& s) const
{
	// Remember, extra bits MUST be set to zero. This allows easy checking of only the array contents.
	for (int i = 0; i < NumElements; i++)
		if (Elements[i] != s.Elements[i])
			return false;

	return true;
}


template<int N> inline bool tBitField<N>::operator!=(const tBitField& s) const
{
	for (int i = 0; i < NumElements; i++)
		if (Elements[i] != s.Elements[i])
			return true;

	return false;
}


template<int N> inline tBitField<N>::operator bool() const
{
	for (int i = 0; i < NumElements; i++)
		if (Elements[i])
			return true;

	return false;
}


template<int N> inline void tBitField<N>::ClearUnusedBits()
{
	int r = N%32;
	uint32& e = Elements[NumElements-1];
	e &= r ? ~((0xFFFFFFFF >> r) << r) : 0xFFFFFFFF;
}
