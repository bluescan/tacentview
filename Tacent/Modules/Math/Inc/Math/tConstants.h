// tConstants.h
//
// Physical and mathematical constants.
//
// Copyright (c) 2004-2006, 2015, 2017, 2019, 2020 Tristan Grimmer.
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
namespace tMath
{


const float Pi									= 3.14159265358979323f;
const float TwoPi								= Pi * 2.0f;
const float PiOver2								= Pi / 2.0f;
const float PiOver3								= Pi / 3.0f;
const float PiOver4								= Pi / 4.0f;
const float KmPerMi								= 1.609344f;
const float MpsToKmph							= 3.6f;
const float DefaultGamma						= 2.2f;
const float MaxFloat							= 3.402823466e+38f;

// This is a practical epsilon that can be used in many circumstances. One one millionth.
const float Epsilon								= 0.000001f;

// This is the smallest effective positive float such that 1.0f+EpsilonEff != 1.0f.
const float EpsilonEff							= 1.192092896e-07f;

// This is the smallest positive value that can be represented by a single precision floating point.
const float EpsilonRep							= 1.175494351e-38f;
const float Infinity							= MaxFloat;

// The infinities below are not the special IEEE bit patterns that represent infinity. They are the very largest and
// smallest values that may be represented without use of the special bit patterns.
const float PosInfinity							= MaxFloat;
const float NegInfinity							= -MaxFloat;

// C++11 should promote 2147483648 to long long int, but VS currently turns it into and unsigned int. That's why we're
// using the hex string literal representation for now.
const int32 MinInt32							= 0x80000000;	// Decimal value -2147483648.
const int32 MaxInt32							= 2147483647;	// Hex value 0x7FFFFFFF.
const uint32 MinUint32							= 0;
const uint32 MaxUint32							= 0xFFFFFFFF;
const uint32 MinUint16							= 0;
const uint32 MaxUint16							= 0xFFFF;


}
