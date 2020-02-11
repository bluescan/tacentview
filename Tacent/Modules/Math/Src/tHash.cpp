// tHash.cpp
//
// Hash functions for various kinds of data. Using 64 or 256 bit versions if you want to avoid collisions. There are two
// 32 bit hash functions. A fast version used for most string hashes, and a slower but better version. All functions
// return the supplied initialization vector(iv) if there was no data to hash. To compute a single hash from multiple
// data sources like strings, binary data, or files, you do NOT need to consolidate all the source data into one buffer
// first. Just set the initialization vector to the hash computed from the previous step.
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

#include <Foundation/tStandard.h>
#include "Math/tHash.h"


uint32 tMath::tHashDataFast32(const uint8* data, int length, uint32 iv)
{
	uint32 hash = iv;
	while (length--)
	{
		hash += hash << 5;
		hash += *(uint8*)data++;
	}

	return hash;
}


// This 32bit hash was written originally by Robert J. Jenkins Jr., 1997
// See http://burtleburtle.net/bob/hash/evahash.html
namespace tHash
{
	inline void Mix32(uint32& a, uint32& b, uint32& c)
	{
		a -= b; a -= c; a ^= (c>>13);
		b -= c; b -= a; b ^= (a<<8);
		c -= a; c -= b; c ^= (b>>13);
		a -= b; a -= c; a ^= (c>>12);
		b -= c; b -= a; b ^= (a<<16);
		c -= a; c -= b; c ^= (b>>5);
		a -= b; a -= c; a ^= (c>>3);
		b -= c; b -= a; b ^= (a<<10);
		c -= a; c -= b; c ^= (b>>15);
	}
}


uint32 tMath::tHashData32(const uint8* data, int length, uint32 iv)
{
	uint32 a,b,c;											// The internal state.
	int len;												// How many key bytes still need mixing.

	len = length;
	a = b = 0x9e3779b9;										// The golden ratio; an arbitrary value.
	c = iv;													// Variable initialization of internal state.

	// Do as many 12 byte chunks as we can.
	while (len >= 12)
	{
		a += data[0] + (uint32(data[1]) << 8) + (uint32(data[2]) << 16) + (uint32(data[3]) << 24);
		b += data[4] + (uint32(data[5]) << 8) + (uint32(data[6]) << 16) + (uint32(data[7]) << 24);
		c += data[8] + (uint32(data[9]) << 8) + (uint32(data[10]) << 16) + (uint32(data[11]) << 24);
		tHash::Mix32(a,b,c);
		data += 12; len -= 12;
	}

	// Finish up the last 11 bytes.
	c += length;
	switch (len)											// All the case statements fall through.
	{
		case 11: c += uint32(data[10]) << 24;
		case 10: c += uint32(data[9]) << 16;
		case 9 : c += uint32(data[8]) << 8;					// The first byte of c is reserved for the length.
		case 8 : b += uint32(data[7]) << 24;
		case 7 : b += uint32(data[6]) << 16;
		case 6 : b += uint32(data[5]) << 8;
		case 5 : b += data[4];
		case 4 : a += uint32(data[3]) << 24;
		case 3 : a += uint32(data[2]) << 16;
		case 2 : a += uint32(data[1]) << 8;
		case 1 : a += data[0];
	}
	tHash::Mix32(a,b,c);

	return c;
}


// This 64bit hash was written originally by Robert J. Jenkins Jr., 1997
// See http://burtleburtle.net/bob/hash/evahash.html
namespace tHash
{
	inline void Mix64(uint64& a, uint64& b, uint64& c)
	{
		a -= b; a -= c; a ^= (c>>43);
		b -= c; b -= a; b ^= (a<<9);
		c -= a; c -= b; c ^= (b>>8);
		a -= b; a -= c; a ^= (c>>38);
		b -= c; b -= a; b ^= (a<<23);
		c -= a; c -= b; c ^= (b>>5);
		a -= b; a -= c; a ^= (c>>35);
		b -= c; b -= a; b ^= (a<<49);
		c -= a; c -= b; c ^= (b>>11);
		a -= b; a -= c; a ^= (c>>12);
		b -= c; b -= a; b ^= (a<<18);
		c -= a; c -= b; c ^= (b>>22);
	}
}


uint64 tMath::tHashData64(const uint8* data, int length, uint64 iv)
{
	#ifndef PLATFORM_WIN
		tAssert(!"HashData64 has only been tested in windows.");
	#endif
	uint64 a,b,c;											// The internal state.
	int len;												// How many key bytes still need mixing.

	len = length;
	a = b = 0x9e3779b97f4a7c13ULL;							// The golden ratio; an arbitrary value.
	c = iv;													// Variable initialization of internal state.

	// Do as many 24 byte chunks as we can.
	while (len >= 24)
	{
		a += (uint64(data[0]) << 0) + (uint64(data[1]) << 8) + (uint64(data[2]) << 16) + (uint64(data[3]) << 24)
		  + (uint64(data[4]) << 32) + (uint64(data[5]) << 40) + (uint64(data[6]) << 48) + (uint64(data[7]) << 56);

		b += (uint64(data[8]) << 0) + (uint64(data[9]) << 8) + (uint64(data[10]) << 16) + (uint64(data[11]) << 24)
		  + (uint64(data[12]) << 32) + (uint64(data[13]) << 40) + (uint64(data[14]) << 48) + (uint64(data[15]) << 56);

		c += (uint64(data[16]) << 0) + (uint64(data[17]) << 8) + (uint64(data[18]) << 16) + (uint64(data[19]) << 24)
		  + (uint64(data[20]) << 32) + (uint64(data[21]) << 40) + (uint64(data[22]) << 48) + (uint64(data[23]) << 56);

		tHash::Mix64(a,b,c);
		data += 24; len -= 24;
	}

	// Finish up the last 23 bytes.
	c += length;
	switch (len)											// All the case statements fall through.
	{
		case 23: c += uint64(data[22]) << 56;
		case 22: c += uint64(data[21]) << 48;
		case 21: c += uint64(data[20]) << 40;
		case 20: c += uint64(data[19]) << 32;
		case 19: c += uint64(data[18]) << 24;
		case 18: c += uint64(data[17]) << 16;
		case 17: c += uint64(data[16]) << 8;				// The first byte of c is reserved for the length.

		case 16: b += uint64(data[15]) << 56;
		case 15: b += uint64(data[14]) << 48;
		case 14: b += uint64(data[13]) << 40;
		case 13: b += uint64(data[12]) << 32;
		case 12: b += uint64(data[11]) << 24;
		case 11: b += uint64(data[10]) << 16;
		case 10: b += uint64(data[9])  << 8;
		case 9 : b += uint64(data[8])  << 0;

		case 8:  a += uint64(data[7])  << 56;
		case 7:  a += uint64(data[6])  << 48;
		case 6:  a += uint64(data[5])  << 40;
		case 5:  a += uint64(data[4])  << 32;
		case 4:  a += uint64(data[3])  << 24;
		case 3:  a += uint64(data[2])  << 16;
		case 2:  a += uint64(data[1])  << 8;
		case 1:  a += uint64(data[0])  << 0;
	}

	tHash::Mix64(a,b,c);

	return c;
}


// Here is the 128 bit MD5 hash algorithm.
namespace tHash
{
	// Below are constants for MD5Transform routine.
	const static int MD5_S11 = 7;
	const static int MD5_S12 = 12;
	const static int MD5_S13 = 17;
	const static int MD5_S14 = 22;
	const static int MD5_S21 = 5;
	const static int MD5_S22 = 9;
	const static int MD5_S23 = 14;
	const static int MD5_S24 = 20;
	const static int MD5_S31 = 4;
	const static int MD5_S32 = 11;
	const static int MD5_S33 = 16;
	const static int MD5_S34 = 23;
	const static int MD5_S41 = 6;
	const static int MD5_S42 = 10;
	const static int MD5_S43 = 15;
	const static int MD5_S44 = 21;

	const static int iMD5BlockSize = 64;
	
	// Decodes input (uint8) into output (uint32). Assumes len is a multiple of 4.
	void MD5Decode(uint32* output, const uint8* input, int length);

	// Encodes input (uint32) into output (uint8). Assumes len is a multiple of 4.
	void MD5Encode(uint8* output, const uint32* input, int length);

	// Apply MD5 algo on a block.
	void MD5Transform(uint32 state[4], const uint8* block);

	void MD5Update(uint32 count[2], uint32 state[4], const uint8* data, uint32 length, uint8 buffer[iMD5BlockSize]);

	uint32 MD5_F(uint32 x, uint32 y, uint32 z)																			{ return x&y | ~x&z; }
	uint32 MD5_G(uint32 x, uint32 y, uint32 z)																			{ return x&z | y&~z; }
	uint32 MD5_H(uint32 x, uint32 y, uint32 z)																			{ return x^y^z; }
	uint32 MD5_I(uint32 x, uint32 y, uint32 z)																			{ return y ^ (x | ~z); }
	uint32 MD5_RotateLeft(uint32 x, int n)																				{ return (x << n) | (x >> (32-n)); }
	void MD5_FF(uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac)									{ a = MD5_RotateLeft(a + MD5_F(b, c, d) + x + ac, s) + b; }
	void MD5_GG(uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac)									{ a = MD5_RotateLeft(a + MD5_G(b, c, d) + x + ac, s) + b; }
	void MD5_HH(uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac)									{ a = MD5_RotateLeft(a + MD5_H(b, c, d) + x + ac, s) + b; }
	void MD5_II(uint32& a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 ac)									{ a = MD5_RotateLeft(a + MD5_I(b, c, d) + x + ac, s) + b; }
};


void tHash::MD5Decode(uint32* output, const uint8* input, int length)
{
	for (int i = 0, j = 0; j < length; i++, j += 4)
	{
		uint32 j0 = uint32(input[j]);
		uint32 j1 = uint32(input[j+1]);
		uint32 j2 = uint32(input[j+2]);
		uint32 j3 = uint32(input[j+3]);
		output[i] = (j0 | (j1 << 8) | (j2 << 16) | (j3 << 24));
	}
}


void tHash::MD5Encode(uint8* output, const uint32* input, int length)
{
	for (int i = 0, j = 0; j < length; i++, j += 4)
	{
		output[j] = input[i] & 0xFF;
		output[j+1] = (input[i] >> 8) & 0xFF;
		output[j+2] = (input[i] >> 16) & 0xFF;
		output[j+3] = (input[i] >> 24) & 0xFF;
	}
}


void tHash::MD5Transform(uint32 state[4], const uint8* block)
{
	uint32 a = state[0];
	uint32 b = state[1];
	uint32 c = state[2];
	uint32 d = state[3];
	uint32 x[16];

	MD5Decode(x, block, iMD5BlockSize);

	// Round 1
	tHash::MD5_FF(a, b, c, d, x[ 0], tHash::MD5_S11, 0xd76aa478); // 1
	tHash::MD5_FF(d, a, b, c, x[ 1], tHash::MD5_S12, 0xe8c7b756); // 2
	tHash::MD5_FF(c, d, a, b, x[ 2], tHash::MD5_S13, 0x242070db); // 3
	tHash::MD5_FF(b, c, d, a, x[ 3], tHash::MD5_S14, 0xc1bdceee); // 4
	tHash::MD5_FF(a, b, c, d, x[ 4], tHash::MD5_S11, 0xf57c0faf); // 5
	tHash::MD5_FF(d, a, b, c, x[ 5], tHash::MD5_S12, 0x4787c62a); // 6
	tHash::MD5_FF(c, d, a, b, x[ 6], tHash::MD5_S13, 0xa8304613); // 7
	tHash::MD5_FF(b, c, d, a, x[ 7], tHash::MD5_S14, 0xfd469501); // 8
	tHash::MD5_FF(a, b, c, d, x[ 8], tHash::MD5_S11, 0x698098d8); // 9
	tHash::MD5_FF(d, a, b, c, x[ 9], tHash::MD5_S12, 0x8b44f7af); // 10
	tHash::MD5_FF(c, d, a, b, x[10], tHash::MD5_S13, 0xffff5bb1); // 11
	tHash::MD5_FF(b, c, d, a, x[11], tHash::MD5_S14, 0x895cd7be); // 12
	tHash::MD5_FF(a, b, c, d, x[12], tHash::MD5_S11, 0x6b901122); // 13
	tHash::MD5_FF(d, a, b, c, x[13], tHash::MD5_S12, 0xfd987193); // 14
	tHash::MD5_FF(c, d, a, b, x[14], tHash::MD5_S13, 0xa679438e); // 15
	tHash::MD5_FF(b, c, d, a, x[15], tHash::MD5_S14, 0x49b40821); // 16

	// Round 2
	tHash::MD5_GG(a, b, c, d, x[ 1], tHash::MD5_S21, 0xf61e2562); // 17
	tHash::MD5_GG(d, a, b, c, x[ 6], tHash::MD5_S22, 0xc040b340); // 18
	tHash::MD5_GG(c, d, a, b, x[11], tHash::MD5_S23, 0x265e5a51); // 19
	tHash::MD5_GG(b, c, d, a, x[ 0], tHash::MD5_S24, 0xe9b6c7aa); // 20
	tHash::MD5_GG(a, b, c, d, x[ 5], tHash::MD5_S21, 0xd62f105d); // 21
	tHash::MD5_GG(d, a, b, c, x[10], tHash::MD5_S22, 0x02441453); // 22
	tHash::MD5_GG(c, d, a, b, x[15], tHash::MD5_S23, 0xd8a1e681); // 23
	tHash::MD5_GG(b, c, d, a, x[ 4], tHash::MD5_S24, 0xe7d3fbc8); // 24
	tHash::MD5_GG(a, b, c, d, x[ 9], tHash::MD5_S21, 0x21e1cde6); // 25
	tHash::MD5_GG(d, a, b, c, x[14], tHash::MD5_S22, 0xc33707d6); // 26
	tHash::MD5_GG(c, d, a, b, x[ 3], tHash::MD5_S23, 0xf4d50d87); // 27
	tHash::MD5_GG(b, c, d, a, x[ 8], tHash::MD5_S24, 0x455a14ed); // 28
	tHash::MD5_GG(a, b, c, d, x[13], tHash::MD5_S21, 0xa9e3e905); // 29
	tHash::MD5_GG(d, a, b, c, x[ 2], tHash::MD5_S22, 0xfcefa3f8); // 30
	tHash::MD5_GG(c, d, a, b, x[ 7], tHash::MD5_S23, 0x676f02d9); // 31
	tHash::MD5_GG(b, c, d, a, x[12], tHash::MD5_S24, 0x8d2a4c8a); // 32

	// Round 3
	tHash::MD5_HH(a, b, c, d, x[ 5], tHash::MD5_S31, 0xfffa3942); // 33
	tHash::MD5_HH(d, a, b, c, x[ 8], tHash::MD5_S32, 0x8771f681); // 34
	tHash::MD5_HH(c, d, a, b, x[11], tHash::MD5_S33, 0x6d9d6122); // 35
	tHash::MD5_HH(b, c, d, a, x[14], tHash::MD5_S34, 0xfde5380c); // 36
	tHash::MD5_HH(a, b, c, d, x[ 1], tHash::MD5_S31, 0xa4beea44); // 37
	tHash::MD5_HH(d, a, b, c, x[ 4], tHash::MD5_S32, 0x4bdecfa9); // 38
	tHash::MD5_HH(c, d, a, b, x[ 7], tHash::MD5_S33, 0xf6bb4b60); // 39
	tHash::MD5_HH(b, c, d, a, x[10], tHash::MD5_S34, 0xbebfbc70); // 40
	tHash::MD5_HH(a, b, c, d, x[13], tHash::MD5_S31, 0x289b7ec6); // 41
	tHash::MD5_HH(d, a, b, c, x[ 0], tHash::MD5_S32, 0xeaa127fa); // 42
	tHash::MD5_HH(c, d, a, b, x[ 3], tHash::MD5_S33, 0xd4ef3085); // 43
	tHash::MD5_HH(b, c, d, a, x[ 6], tHash::MD5_S34, 0x04881d05); // 44
	tHash::MD5_HH(a, b, c, d, x[ 9], tHash::MD5_S31, 0xd9d4d039); // 45
	tHash::MD5_HH(d, a, b, c, x[12], tHash::MD5_S32, 0xe6db99e5); // 46
	tHash::MD5_HH(c, d, a, b, x[15], tHash::MD5_S33, 0x1fa27cf8); // 47
	tHash::MD5_HH(b, c, d, a, x[ 2], tHash::MD5_S34, 0xc4ac5665); // 48

	// Round 4
	tHash::MD5_II(a, b, c, d, x[ 0], tHash::MD5_S41, 0xf4292244); // 49
	tHash::MD5_II(d, a, b, c, x[ 7], tHash::MD5_S42, 0x432aff97); // 50
	tHash::MD5_II(c, d, a, b, x[14], tHash::MD5_S43, 0xab9423a7); // 51
	tHash::MD5_II(b, c, d, a, x[ 5], tHash::MD5_S44, 0xfc93a039); // 52
	tHash::MD5_II(a, b, c, d, x[12], tHash::MD5_S41, 0x655b59c3); // 53
	tHash::MD5_II(d, a, b, c, x[ 3], tHash::MD5_S42, 0x8f0ccc92); // 54
	tHash::MD5_II(c, d, a, b, x[10], tHash::MD5_S43, 0xffeff47d); // 55
	tHash::MD5_II(b, c, d, a, x[ 1], tHash::MD5_S44, 0x85845dd1); // 56
	tHash::MD5_II(a, b, c, d, x[ 8], tHash::MD5_S41, 0x6fa87e4f); // 57
	tHash::MD5_II(d, a, b, c, x[15], tHash::MD5_S42, 0xfe2ce6e0); // 58
	tHash::MD5_II(c, d, a, b, x[ 6], tHash::MD5_S43, 0xa3014314); // 59
	tHash::MD5_II(b, c, d, a, x[13], tHash::MD5_S44, 0x4e0811a1); // 60
	tHash::MD5_II(a, b, c, d, x[ 4], tHash::MD5_S41, 0xf7537e82); // 61
	tHash::MD5_II(d, a, b, c, x[11], tHash::MD5_S42, 0xbd3af235); // 62
	tHash::MD5_II(c, d, a, b, x[ 2], tHash::MD5_S43, 0x2ad7d2bb); // 63
	tHash::MD5_II(b, c, d, a, x[ 9], tHash::MD5_S44, 0xeb86d391); // 64

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	// Clear sensitive information.
	tStd::tMemset(x, 0, sizeof(x));
}


void tHash::MD5Update(uint32 count[2], uint32 state[4], const uint8* data, uint32 length, uint8 buffer[iMD5BlockSize])
{
	int index = count[0] / 8 % iMD5BlockSize;				// Compute number of bytes mod 64.

	// Update number of bits.
	if ((count[0] += (length << 3)) < (length << 3))
		count[1]++;
	count[1] += (length >> 29);

	uint32 firstpart = 64 - index;							// Number of bytes we need to fill in buffer.

	// Transform as many times as possible.
	uint32 i = 0;
	if (length >= firstpart)
	{
		// Fill buffer first, transform.
		tStd::tMemcpy(&buffer[index], data, firstpart);
		MD5Transform(state, buffer);

		// Transform chunks of blocksize (64 bytes).
		for (i = firstpart; i + iMD5BlockSize <= length; i += iMD5BlockSize)
			MD5Transform(state, &data[i]);

		index = 0;
	}

	// Buffer remaining input.
	tStd::tMemcpy(&buffer[index], &data[i], length-i);
}


tuint128 tMath::tHashDataMD5(const uint8* data, int len, tuint128 iv)
{
	uint32 length = len;
	uint8 buffer[tHash::iMD5BlockSize];						// Bytes that didn't fit in last 64 byte chunk.
	uint32 count[2];										// 64bit counter for number of bits (lo, hi).
	uint32 state[4];										// Digest so far.
	uint8 digest[16];										// The result.

	// Phase 1. Initialize state variables.
	count[0] = 0;
	count[1] = 0;
	state[0] = 0x67452301;									// Load magic initialization constants.
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;

	// Phase 2. Block update. Could be put in a loop to process multiple chunks of data. Continues an MD5
	// message-digest operation, processing another message block.
	tHash::MD5Update(count, state, data, length, buffer);

	// Phase 3. Finalize.
	// Ends an MD5 message-digest operation, writing the the message digest and clearing the context.
	static uint8 padding[64] =
	{
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	// Save number of bits.
	unsigned char bits[8];
	tHash::MD5Encode(bits, count, 8);

	// Pad out to 56 mod 64.
	int index = count[0] / 8 % 64;
	int padLen = (index < 56) ? (56 - index) : (120 - index);
	tHash::MD5Update(count, state, padding, padLen, buffer);

	// Append length (before padding).
	tHash::MD5Update(count, state, bits, 8, buffer);

	// Store state in digest
	tHash::MD5Encode(digest, state, 16);

	// Clear sensitive information.
	tStd::tMemset(buffer, 0, sizeof buffer);
	tStd::tMemset(count, 0, sizeof count);

	// Digest is now valid. The lower indexed numbers are least significant so we need to reverse the order.
	tuint128 result;
	tAssert(sizeof(result) == sizeof(digest));

	for (int i = 0; i < 16; i++)
		((uint8*)&result)[15-i] = digest[i];

	return result;
}


// This 256bit hash was written originally by Robert J. Jenkins Jr., 1997
// See http://burtleburtle.net/bob/hash/evahash.html
namespace tHash
{
	inline void Mix256(uint32& a, uint32& b, uint32& c, uint32& d, uint32& e, uint32& f, uint32& g, uint32& h)
	{
		a ^= b<<11; d += a; b += c;
		b ^= c>>2;  e += b; c += d;
		c ^= d<<8;  f += c; d += e;
		d ^= e>>16; g += d; e += f;
		e ^= f<<10; h += e; f += g;
		f ^= g>>4;  a += f; g += h;
		g ^= h<<8;  b += g; h += a;
		h ^= a>>9;  c += h; a += b;
	}
}


tuint256 tMath::tHashData256(const uint8* data, int len, tuint256 iv)
{
	#ifndef PLATFORM_WIN
	// @todo Are there endianness concerns?
	tAssert(!"HashData256 has only been tested in windows.");
	#endif

	uint32 a, b, c, d, e, f, g, h, length;

	// Use the length and level. Add in the golden ratio. Remember, 'a' is most significant.
	length = len;
	uint32& A = iv.RawElement(7); uint32& B = iv.RawElement(6); uint32& C = iv.RawElement(5); uint32& D = iv.RawElement(4);
	uint32& E = iv.RawElement(3); uint32& F = iv.RawElement(2); uint32& G = iv.RawElement(1); uint32& H = iv.RawElement(0);
	a = A; b = B; c = C; d = D;
	e = E; f = F; g = G; h = H;

	// Process most of the key.
	while (len >= 32)
	{
		a += *(uint32*)(data+0);
		b += *(uint32*)(data+4);
		c += *(uint32*)(data+8);
		d += *(uint32*)(data+12);
		e += *(uint32*)(data+16);
		f += *(uint32*)(data+20);
		g += *(uint32*)(data+24);
		h += *(uint32*)(data+28);
		tHash::Mix256(a,b,c,d,e,f,g,h);
		tHash::Mix256(a,b,c,d,e,f,g,h);
		tHash::Mix256(a,b,c,d,e,f,g,h);
		tHash::Mix256(a,b,c,d,e,f,g,h);
		data += 32; len -= 32;
	}

	// Process the last 31 bytes.
	h += length;
	switch (len)
	{
		case 31: h += (data[30] << 24);
		case 30: h += (data[29] << 16);
		case 29: h += (data[28] << 8);
		case 28: g += (data[27] << 24);
		case 27: g += (data[26] << 16);
		case 26: g += (data[25] << 8);
		case 25: g +=  data[24];
		case 24: f += (data[23] << 24);
		case 23: f += (data[22] << 16);
		case 22: f += (data[21] << 8);
		case 21: f +=  data[20];
		case 20: e += (data[19] << 24);
		case 19: e += (data[18] << 16);
		case 18: e += (data[17] << 8);
		case 17: e +=  data[16];
		case 16: d += (data[15] << 24);
		case 15: d += (data[14] << 16);
		case 14: d += (data[13] << 8);
		case 13: d +=  data[12];
		case 12: c += (data[11] << 24);
		case 11: c += (data[10] << 16);
		case 10: c += (data[9] << 8);
		case 9 : c +=  data[8];
		case 8 : b += (data[7] << 24);
		case 7 : b += (data[6] << 16);
		case 6 : b += (data[5] << 8);
		case 5 : b +=  data[4];
		case 4 : a += (data[3] << 24);
		case 3 : a += (data[2] << 16);
		case 2 : a += (data[1] << 8);
		case 1 : a +=  data[0];
	}

	tHash::Mix256(a,b,c,d,e,f,g,h);
	tHash::Mix256(a,b,c,d,e,f,g,h);
	tHash::Mix256(a,b,c,d,e,f,g,h);
	tHash::Mix256(a,b,c,d,e,f,g,h);

	A = a; B = b; C = c; D = d;
	E = e; F = f; G = g; H = h;
	return iv;
}
