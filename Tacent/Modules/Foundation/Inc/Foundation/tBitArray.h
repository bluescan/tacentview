// tBitArray.h
//
// A tBitArray is a holder for an arbitrary number of bits and allows individual access to each bit, the ability to
// clear or set all bits, and some simple binary bitwise operators such as 'and', 'xor', and 'or'. It currently does
// not support dynamic growing or shrinking. If the number of bits you need is known at compile time, consider using a
// tBitField instead as it is more feature-complete.
//
// Copyright (c) 2004-2006, 2015, 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include "Foundation/tPlatform.h"
#include "Foundation/tStandard.h"


class tBitArray
{
public:
	tBitArray()												/* Creates an invalid bit array. Call Set before using. */	: NumBits(0), BitFields(nullptr) { }
	tBitArray(int numBits)									/* All bit values guaranteed to be 0 after this. */			: NumBits(0), BitFields(nullptr) { Set(numBits); }
	tBitArray(const uint32* data, int numBits)				/* Copies numBits from data. */								: NumBits(0), BitFields(nullptr) { Set(data, numBits); }
	tBitArray(const tBitArray& src)																						: NumBits(0), BitFields(nullptr) { Set(src); }
	~tBitArray()																										{ delete[] BitFields; }

	void Set(int numBits);									// All bit values guaranteed to be 0 after this.
	void Set(const uint32* data, int numBits);				// Copies numBits from data.
	void Set(const tBitArray& src);
	void Clear()											/* Invalidates. Use ClearAll if you want all bits clear. */	{ delete[] BitFields; BitFields = nullptr; NumBits = 0; }
	bool IsValid() const																								{ return BitFields ? true : false; }

	bool GetBit(int n) const;								// n is the bit index with 0 being the least significant.
	void SetBit(int n, bool v);
	void SetAll(bool v = true);
	void ClearAll();
	void InvertAll();
	bool AreAll(bool v) const;
	int GetNumBits() const																								{ return NumBits; }
	int CountBits(bool = true) const;						// Counts how many bits are set to supplied value.

	// Returns how many uint32 fields are used to store the bit array.
	int GetNumFields() const																							{ return (NumBits>>5) + ((NumBits & 0x1F) ? 1 : 0); }
	uint32* GetFields() const																							{ return BitFields; }

	// Returns index of a bit that is clear. Which one is arbitrary. Returns -1 if none are available.
	int GetClearedBitPos() const;

	// Binary operators must operate on arrays with the same number of bits.
	tBitArray& operator=(const tBitArray& src)																			{ Set(src); return *this; }
	tBitArray& operator&=(const tBitArray&);
	tBitArray& operator|=(const tBitArray&);
	tBitArray& operator^=(const tBitArray&);

	const tBitArray operator~() const																					{ tBitArray r(*this); r.InvertAll(); return r; }
	bool operator[](int n) const																						{ return GetBit(n); }
	bool operator==(const tBitArray&) const;
	bool operator!=(const tBitArray&) const;

private:
	int GetClearedBit(int index) const;
	void ClearPadBits();

	int NumBits;											// Number of bits. Not number of fields.
	uint32* BitFields;										// If there are padding bits, they must be set to 0.
};


// Implementation below this line.


inline bool tBitArray::GetBit(int index) const
{
	tAssert(index < NumBits);
	int fieldIndex = index >> 5;
	int offset = index & 0x1F;
	uint32 mask = 1 << offset;

	return (BitFields[fieldIndex] & mask) ? true : false;
}


inline void tBitArray::SetBit(int index, bool v)
{
	tAssert(index < NumBits);
	int fieldIndex = index >> 5;
	int offset = index & 0x1F;
	uint32 mask = 1 << offset;
	if (v)
		BitFields[fieldIndex] |= mask;
	else
		BitFields[fieldIndex] &= ~mask;
}


inline void tBitArray::SetAll(bool v)
{
	int n = GetNumFields();
	tStd::tMemset(BitFields, v ? 0xFF : 0, n*sizeof(uint32));
	if (v)
		ClearPadBits();
}


inline void tBitArray::ClearAll()
{
	tAssert(BitFields);
	int n = GetNumFields();
	tStd::tMemset(BitFields, 0, n*sizeof(uint32));
}


inline bool tBitArray::operator==(const tBitArray& s) const
{
	tAssert(GetNumBits() == s.GetNumBits());
	int n = GetNumFields();
	for (int i = 0; i < n; i++)
		if (BitFields[i] != s.BitFields[i])
			return false;

	return true;
}


inline void tBitArray::ClearPadBits()
{
	tAssert(BitFields);
	int numFields = GetNumFields();
	int last = NumBits & 0x1F;
	uint32 maxFull = (last ? (1 << last) : 0) - 1;
	BitFields[numFields-1] &= maxFull;
}
