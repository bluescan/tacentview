// tColour.h
//
// Colour and pixel classes. There are classes for:
// * A 32 bit colour. 4 unsigned 8-bit integer components (rgb + alpha).
// * A 96 bit colour. 3 32-bit float components.
// * A 128 bit colour. 4 32-bit float components (rgb + alpha).
//
// Copyright (c) 2006, 2011, 2017, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include "Math/tFundamentals.h"
#include "Math/tVector3.h"
#include "Math/tVector4.h"
class tColourf;
class tColour3f;


namespace tMath
{
	// Colour space conversions. The integer versions accept angle modes of Degrees and Norm256 only. The angle mode
	// determines the range of the hue. Degrees means angles are in [0, 360). Norm256 means angles are in [0, 256).
	// Saturation and value are both in [0, 256) for the integer conversion functions.
	void tRGBToHSV(int& h, int& s, int& v, int r, int g, int b, tAngleMode = tMath::tAngleMode::Degrees);
	void tHSVToRGB(int& r, int& g, int& b, int h, int s, int v, tMath::tAngleMode = tMath::tAngleMode::Degrees);

	// The floating point colour space conversion functions accept any angle mode. Radians mean angles are in [0.0, 2Pi).
	// Degrees means angles are in [0.0, 360.0). Norm256 means angles are in [0.0, 256.0). NormOne means angles are
	// in [0.0, 1.0].
	void tRGBToHSV(float& h, float& s, float& v, float r, float g, float b, tMath::tAngleMode = tMath::tAngleMode::Radians);
	void tHSVToRGB(float& r, float& g, float& b, float h, float s, float v, tMath::tAngleMode = tMath::tAngleMode::Radians);
}


// The tColouri class represents a colour in 32 bits and is made of 4 unsigned byte-size integers in the order RGBA.
class tColouri
{
public:
	tColouri()												/* Does NOT set the colour values. */						{ }
	tColouri(const tColouri& c)																							: BP(c.BP) { }
	tColouri(int r, int g, int b, int a = 0xFF)																			{ R = tMath::tClamp(r, 0, 0xFF); G = tMath::tClamp(g, 0, 0xFF); B = tMath::tClamp(b, 0, 0xFF); A = tMath::tClamp(a, 0, 0xFF); }
	tColouri(uint8 r, uint8 g, uint8 b, uint8 a = 0xFF)																	: R(r), G(g), B(b), A(a) { }
	tColouri(uint32 bits)																								: BP(bits) { }
	tColouri(const tColourf& c)																							{ Set(c); }
	tColouri(float r, float g, float b, float a = 1.0f)																	{ Set(r, g, b, a); }
	tColouri(const float* src)																							{ Set(src); }

	void Set(const tColouri& c)																							{ BP = c.BP; }
	void Set(int r, int g, int b, int a = 255)																			{ R = uint8(r); G = uint8(g); B = uint8(b); A = uint8(a); }
	void Set(const tColourf& c);
	void Set(const float* src)																							{ SetR(src[0]); SetG(src[1]); SetB(src[2]); SetA(src[3]); }
	void Set(float r, float g, float b, float a = 1.0f)																	{ SetR(r); SetG(g); SetB(b); SetA(a); }
	void SetR(float r)																									{ R = tMath::tClamp( tMath::tFloatToInt(r*255.0f), 0, 0xFF ); }
	void SetG(float g)																									{ G = tMath::tClamp( tMath::tFloatToInt(g*255.0f), 0, 0xFF ); }
	void SetB(float b)																									{ B = tMath::tClamp( tMath::tFloatToInt(b*255.0f), 0, 0xFF ); }
	void SetA(float a)																									{ A = tMath::tClamp( tMath::tFloatToInt(a*255.0f), 0, 0xFF ); }

	// The floating point get and set methods use a range of [0.0, 1.0] for each component.
	float GetR() const																									{ return float(R) / 255.0f; }
	float GetG() const																									{ return float(G) / 255.0f; }
	float GetB() const																									{ return float(B) / 255.0f; }
	float GetA() const																									{ return float(A) / 255.0f; }
	void Get(float* dest) const																							{ dest[0] = GetR(); dest[1] = GetG(); dest[2] = GetB(); dest[3] = GetA(); }
	void Get(tMath::tVector3& dest) const																				{ dest.x = GetR(); dest.y = GetG(); dest.z = GetB(); }
	void Get(tMath::tVector4& dest) const																				{ dest.x = GetR(); dest.y = GetG(); dest.z = GetB(); dest.w = GetA(); }
	void Get(float& r, float&g, float& b, float& a) const																{ r = GetR(); g = GetG(); b = GetB(); a = GetA(); }
	void Get(tColouri& c) const																							{ c.BP = BP; }

	void MakeZero()																										{ R = 0x00; G = 0x00; B = 0x00; A = 0x00; }
	void MakeBlack()																									{ R = 0x00; G = 0x00; B = 0x00; A = 0xFF; }
	void MakeWhite()																									{ R = 0xFF; G = 0xFF; B = 0xFF; A = 0xFF; }
	void MakePink()																										{ R = 0xFF; G = 0x80; B = 0x80; A = 0xFF; }

	void MakeRed()																										{ R = 0xFF; G = 0x00; B = 0x00; A = 0xFF; }
	void MakeGreen()																									{ R = 0x00; G = 0xFF; B = 0x00; A = 0xFF; }
	void MakeBlue()																										{ R = 0x00; G = 0x00; B = 0xFF; A = 0xFF; }

	void MakeGrey()																										{ R = 0x80; G = 0x80; B = 0x80; A = 0xFF; }
	void MakeLightGrey()																								{ R = 0xC0; G = 0xC0; B = 0xC0; A = 0xFF; }
	void MakeDarkGrey()																									{ R = 0x40; G = 0x40; B = 0x40; A = 0xFF;}

	void MakeCyan()																										{ R = 0x00; G = 0xFF; B = 0xFF; A = 0xFF; }
	void MakeMagenta()																									{ R = 0xFF; G = 0x00; B = 0xFF; A = 0xFF; }
	void MakeYellow()																									{ R = 0xFF; G = 0xFF; B = 0x00; A = 0xFF; }

	// These querying calls ignore alpha.
	bool IsBlack() const																								{ return ((R == 0x00) && (G == 0x00) && (B == 0x00)) ? true : false; }
	bool IsWhite() const																								{ return ((R == 0xFF) && (G == 0xFF) && (B == 0xFF)) ? true : false; }
	bool IsRed() const																									{ return ((R == 0xFF) && (G == 0x00) && (B == 0x00)) ? true : false; }
	bool IsGreen() const																								{ return ((R == 0x00) && (G == 0xFF) && (B == 0x00)) ? true : false; }
	bool IsBlue() const																									{ return ((R == 0x00) && (G == 0x00) && (B == 0xFF)) ? true : false; }

	// When using the HSV representation of a tColouri, the hue is in normalized angle units. See tAngleMode::Norm256.
	// Since only one byte is used, we divide the circle into 256 equal parts. All 4 values will be E [0, 255].
	// Consider using a tColoutf object when working in HSV space. It can more accurately represent the hue value
	// without as much loss in precision. See the tRGBToHSV function for retrieval of hue in different angle units.
	// Both of the functions below leave the alpha unchanged.
	void RGBToHSV();										// Assumes current values are RGB.
	void HSVToRGB();										// Assumes current values are HSV.

	bool operator==(const tColouri& c) const																			{ return (BP == c.BP); }
	bool operator!=(const tColouri& c) const 																			{ return (BP != c.BP); }
	tColouri& operator=(const tColouri& c)																				{ BP = c.BP; return *this; }

	// Predefined colours. Initialized using the C++11 aggregate initializer syntax. These may be used before main()
	// in normally (non-aggregate syntax) constructed objects.
	const static tColouri black;
	const static tColouri white;
	const static tColouri pink;

	const static tColouri red;
	const static tColouri green;
	const static tColouri blue;

	const static tColouri grey;
	const static tColouri lightgrey;
	const static tColouri darkgrey;

	const static tColouri cyan;
	const static tColouri magenta;
	const static tColouri yellow;

	const static tColouri transparent;

	union
	{
		struct { uint8 R, G, B, A; };
		struct { uint8 H, S, V, A; };

		// Bit Pattern member.
		// Accessing the colour as a 32 bit value using the BP member means you must take the machine's endianness into
		// account. This explains why the member isn't named something like RGBA. For example, in memory it's always in the
		// RGBA no matter what endianness, but on a little endian machine you'd access the blue component with something
		// like (colour.BP >> 16) % 0xFF
		uint32 BP;

		// Individual elements. Makes it easy to submit colours to OpenGL using glColor3ubv.
		uint8 E[4];
	};
};
typedef tColouri tPixel;


// The tColourf class represents a colour in 4 floats and is made of 4 floats in the order RGBA.
// The values of each float component are E [0.0, 1.0].
class tColourf
{
public:
	tColourf()																											{ }
	tColourf(const tColourf& src)																						{ Set(src); }
	tColourf(float r, float g, float b, float a = 1.0f)																	{ Set(r, g, b, a); }
	tColourf(const tMath::tVector3& c, float a = 1.0f)																	{ Set(c, a); }
	tColourf(const tMath::tVector4& ca)																					{ Set(ca); }
	tColourf(const tColouri& src)																						{ Set(src); }
	tColourf(uint8 r, uint8 g, uint8 b, uint8 a = 0xFF)																	{ Set(r, g, b, a); }
	tColourf(int r, int g, int b, int a = 255)																			{ Set(r, g, b, a); }

	void Unset()																										{ R = -1.0f; G = -1.0f; B = -1.0f; A = -1.0f; }														// An unset colour has value (-1.0f, -1.0f, -1.0f, -1.0f).
	bool IsSet() const																									{ if ((R != -1.0f) || (G != -1.0f) || (B != -1.0f) || (A != -1.0f)) return true; return false; }	// Any set component means the whole colour is considered set.
	void Set(const tColourf& c)																							{ BP0 = c.BP0; BP1 = c.BP1; }
	void Set(float r, float g, float b, float a = 1.0f)																	{ R = r; G = g; B = b; A = a; }
	void Set(const float* src)																							{ R = src[0]; G = src[1]; B = src[2]; A = src[3]; }
	void Set(const tMath::tVector3& c, float a = 1.0f)																	{ R = c.x; G = c.y; B = c.z; A = a; }
	void Set(const tMath::tVector4& ca)																					{ R = ca.x; G = ca.y; B = ca.z; A = ca.w; }
	void Set(const tColouri& c)																							{ Set(float(c.R)/255.0f, float(c.G)/255.0f, float(c.B)/255.0f, float(c.A)/255.0f); }
	void Set(int r, int g, int b, int a = 255)																			{ Set(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, float(a)/255.0f); }
	void SetR(int r)																									{ R = float(r)/255.0f; }
	void SetG(int g)																									{ G = float(g)/255.0f; }
	void SetB(int b)																									{ B = float(b)/255.0f; }
	void SetA(int a)																									{ A = float(a)/255.0f; }

	// The integer get and set methods use a range of [0, 255] for each component.
	int GetR() const																									{ return tMath::tFloatToInt(R * 255.0f); }
	int GetG() const																									{ return tMath::tFloatToInt(G * 255.0f); }
	int GetB() const																									{ return tMath::tFloatToInt(B * 255.0f); }
	int GetA() const																									{ return tMath::tFloatToInt(A * 255.0f); }
	void Get(int* dest) const																							{ dest[0] = GetR(); dest[1] = GetG(); dest[2] = GetB(); dest[3] = GetA(); }
	void Get(tMath::tVector3& dest) const																				{ dest.x = R; dest.y = G; dest.z = B; }
	void Get(tMath::tVector4& dest) const																				{ dest.x = R; dest.y = G; dest.z = B; dest.w = A; }
	void Get(float& r, float&g, float& b, float& a) const																{ r = R; g = G; b = B; a = A; }
	void Get(tColourf& c) const																							{ c.BP0 = BP0; c.BP1 = BP1;}

	void MakeBlack()																									{ R = 0.0f; G = 0.0f; B = 0.0f; A = 1.0f; }
	void MakeWhite()																									{ R = 1.0f; G = 1.0f; B = 1.0f; A = 1.0f; }
	void MakePink()																										{ R = 1.0f; G = 0.5f; B = 0.5f; A = 1.0f; }

	void MakeRed()																										{ R = 1.0f; G = 0.0f; B = 0.0f; A = 1.0f; }
	void MakeGreen()																									{ R = 0.0f; G = 1.0f; B = 0.0f; A = 1.0f; }
	void MakeBlue()																										{ R = 0.0f; G = 0.0f; B = 1.0f; A = 1.0f; }

	void MakeGrey()																										{ R = 0.5f; G = 0.5f; B = 0.5f; A = 1.0f; }
	void MakeLightGrey()																								{ R = 0.75f; G = 0.75f; B = 0.75f; A = 1.0f; }
	void MakeDarkGrey()																									{ R = 0.25f; G = 0.25f; B = 0.25f; A = 1.0f;}

	void MakeCyan()																										{ R = 0.0f; G = 1.0f; B = 1.0f; A = 1.0f; }
	void MakeMagenta()																									{ R = 1.0f; G = 0.0f; B = 1.0f; A = 1.0f; }
	void MakeYellow()																									{ R = 1.0f; G = 1.0f; B = 0.0f; A = 1.0f; }

	// These querying calls ignore alpha.
	bool IsBlack() const																								{ return ((R == 0.0f) && (G == 0.0f) && (B == 0.0f)) ? true : false; }
	bool IsWhite() const																								{ return ((R == 1.0f) && (G == 1.0f) && (B == 1.0f)) ? true : false; }
	bool IsRed() const																									{ return ((R == 1.0f) && (G == 0.0f) && (B == 0.0f)) ? true : false; }
	bool IsGreen() const																								{ return ((R == 0.0f) && (G == 1.0f) && (B == 0.0f)) ? true : false; }
	bool IsBlue() const																									{ return ((R == 0.0f) && (G == 0.0f) && (B == 1.0f)) ? true : false; }

	// Colours in textures in files may be in Gamma space and ought to be converted to linear space before
	// lighting calculations are made. They should then be converted back to Gamma space before being displayed.
	// Gamma-space here should really be sRGB but we're just using an approximation by squaring (gamma=2) when the
	// average sRGB gamma should be 2.2. To do the conversion properly, the gamma varies with intensity from 1 to 2.4,
	// but, again, we're only approximating here.
	void ToLinearSpaceApprox()																							{ R *= R; G *= G; B *= B; }
	void ToGammaSpaceApprox()																							{ R = tMath::tSqrt(R); G = tMath::tSqrt(G); B = tMath::tSqrt(B); }

	// When using the HSV representation of a tColourf, the hue is in NormOne angle mode. See the tRGBToHSV and
	// tHSVToRGB functions if you wish to use different angle units. All the components (h, s, v, r, g, b, a) are in
	// [0.0, 1.0]. Both of the functions below leave the alpha unchanged.
	void RGBToHSV();										// Assumes current values are RGB.
	void HSVToRGB();										// Assumes current values are HSV.

	bool operator==(const tColourf& c) const																			{ return ((BP0 == c.BP0) && (BP1 == c.BP1)); }
	bool operator!=(const tColourf& c) const 																			{ return ((BP0 != c.BP0) || (BP1 != c.BP1)); }
	tColourf& operator=(const tColourf& c)																				{ BP0 = c.BP0; BP1 = c.BP1; return *this; }

	// Predefined colours.
	const static tColourf invalid;
	const static tColourf black;
	const static tColourf white;
	const static tColourf hotpink;

	const static tColourf red;
	const static tColourf green;
	const static tColourf blue;

	const static tColourf grey;
	const static tColourf lightgrey;
	const static tColourf darkgrey;

	const static tColourf cyan;
	const static tColourf magenta;
	const static tColourf yellow;

	const static tColourf transparent;

	union
	{
		struct { float R, G, B, A; };
		struct { float H, S, V, A; };
		struct { uint64 BP0, BP1; };						// Bit Pattern.
		float E[4];
	};
};
typedef tColourf tColour;


// The tColour3f class represents a colour in 3 floats and is made of 3 floats in the order RGB.
// The values of each float component are E [0.0, 1.0].
class tColour3f
{
public:
	tColour3f()																											{ }
	tColour3f(const tColour3f& src)																						{ Set(src); }
	tColour3f(float r, float g, float b)																				{ Set(r, g, b); }
	tColour3f(const tMath::tVector3& c)																					{ Set(c); }
	tColour3f(const tMath::tVector4& c)																					{ Set(c); }
	tColour3f(const tColouri& src)																						{ Set(src); }
	tColour3f(uint8 r, uint8 g, uint8 b)																				{ Set(r, g, b); }
	tColour3f(int r, int g, int b)																						{ Set(r, g, b); }

	void Unset()																										{ R = -1.0f; G = -1.0f; B = -1.0f; }						// An unset colour has value (-1.0f, -1.0f, -1.0f).
	bool IsSet() const																									{ return ((R != -1.0f) || (G != -1.0f) || (B != -1.0f)); }	// Any set component means the whole colour is considered set.
	void Set(const tColour3f& c)																						{ R = c.R; G = c.G; B = c.B; }
	void Set(float r, float g, float b)																					{ R = r; G = g; B = b; }
	void Set(const float* src)																							{ R = src[0]; G = src[1]; B = src[2]; }
	void Set(const tMath::tVector3& c)																					{ R = c.x; G = c.y; B = c.z; }
	void Set(const tMath::tVector4& c)																					{ R = c.x; G = c.y; B = c.z; }
	void Set(const tColouri& c)																							{ Set(float(c.R)/255.0f, float(c.G)/255.0f, float(c.B)/255.0f); }
	void Set(int r, int g, int b)																						{ Set(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f); }
	void SetR(int r)																									{ R = float(r)/255.0f; }
	void SetG(int g)																									{ G = float(g)/255.0f; }
	void SetB(int b)																									{ B = float(b)/255.0f; }

	// The integer get and set methods use a range of [0, 255] for each component.
	int GetR() const																									{ return tMath::tFloatToInt(R * 255.0f); }
	int GetG() const																									{ return tMath::tFloatToInt(G * 255.0f); }
	int GetB() const																									{ return tMath::tFloatToInt(B * 255.0f); }
	void Get(int* dest) const																							{ dest[0] = GetR(); dest[1] = GetG(); dest[2] = GetB(); }
	void Get(tMath::tVector3& dest) const																				{ dest.x = R; dest.y = G; dest.z = B; }
	void Get(tMath::tVector4& dest) const																				{ dest.x = R; dest.y = G; dest.z = B; dest.w = 1.0f; }
	void Get(float& r, float&g, float& b) const																			{ r = R; g = G; b = B; }
	void Get(tColour3f& c) const																						{ c.R = R; c.G = G; c.B = B; }

	void MakeBlack()																									{ R = 0.0f; G = 0.0f; B = 0.0f; }
	void MakeWhite()																									{ R = 1.0f; G = 1.0f; B = 1.0f; }
	void MakePink()																										{ R = 1.0f; G = 0.5f; B = 0.5f; }

	void MakeRed()																										{ R = 1.0f; G = 0.0f; B = 0.0f; }
	void MakeGreen()																									{ R = 0.0f; G = 1.0f; B = 0.0f; }
	void MakeBlue()																										{ R = 0.0f; G = 0.0f; B = 1.0f; }

	void MakeGrey()																										{ R = 0.5f; G = 0.5f; B = 0.5f; }
	void MakeLightGrey()																								{ R = 0.75f; G = 0.75f; B = 0.75f; }
	void MakeDarkGrey()																									{ R = 0.25f; G = 0.25f; B = 0.25f; }

	void MakeCyan()																										{ R = 0.0f; G = 1.0f; B = 1.0f; }
	void MakeMagenta()																									{ R = 1.0f; G = 0.0f; B = 1.0f; }
	void MakeYellow()																									{ R = 1.0f; G = 1.0f; B = 0.0f; }

	// These querying calls ignore alpha.
	bool IsBlack() const																								{ return ((R == 0.0f) && (G == 0.0f) && (B == 0.0f)); }
	bool IsWhite() const																								{ return ((R == 1.0f) && (G == 1.0f) && (B == 1.0f)); }
	bool IsRed() const																									{ return ((R == 1.0f) && (G == 0.0f) && (B == 0.0f)); }
	bool IsGreen() const																								{ return ((R == 0.0f) && (G == 1.0f) && (B == 0.0f)); }
	bool IsBlue() const																									{ return ((R == 0.0f) && (G == 0.0f) && (B == 1.0f)); }

	// Colours in textures in files may be in Gamma space and ought to be converted to linear space before
	// lighting calculations are made. They should then be converted back to Gamma space before being displayed.
	// Gamma-space here should really be sRGB but we're just using an approximation by squaring (gamma=2) when the
	// average sRGB gamma should be 2.2. To do the conversion properly, the gamma varies with intensity from 1 to 2.4,
	// but, again, we're only approximating here.
	void ToLinearSpaceApprox()																							{ R *= R; G *= G; B *= B; }
	void ToGammaSpaceApprox()																							{ R = tMath::tSqrt(R); G = tMath::tSqrt(G); B = tMath::tSqrt(B); }

	// When using the HSV representation of a tColourf, the hue is in NormOne angle mode. See the tRGBToHSV and
	// tHSVToRGB functions if you wish to use different angle units. All the components (h, s, v, r, g, b, a) are in
	// [0.0, 1.0]. Both of the functions below leave the alpha unchanged.
	void RGBToHSV();										// Assumes current values are RGB.
	void HSVToRGB();										// Assumes current values are HSV.

	bool operator==(const tColourf& c) const																			{ return ((R == c.R) && (G == c.G) && (B == c.B)); }
	bool operator!=(const tColourf& c) const 																			{ return ((R != c.R) || (G != c.G) || (B != c.B)); }
	tColour3f& operator=(const tColour3f& c)																			{ R = c.R; G = c.G; B = c.B; return *this; }

	// Predefined colours.
	const static tColour3f invalid;
	const static tColour3f black;
	const static tColour3f white;
	const static tColour3f hotpink;

	const static tColour3f red;
	const static tColour3f green;
	const static tColour3f blue;

	const static tColour3f grey;
	const static tColour3f lightgrey;
	const static tColour3f darkgrey;

	const static tColour3f cyan;
	const static tColour3f magenta;
	const static tColour3f yellow;

	union
	{
		struct { float R, G, B; };
		struct { float H, S, V; };
		float E[3];
	};
};
typedef tColour3f tColour3;


// Implementation below this line.


inline void tColouri::Set(const tColourf& c)
{
	Set(c.R, c.G, c.B, c.A);
}


inline void tColouri::RGBToHSV()
{
	int r = R;
	int g = G;
	int b = B;
	int h, s, v;
	tRGBToHSV(h, s, v, r, g, b, tMath::tAngleMode::Norm256);
	H = h;
	S = s;
	V = v;
}


inline void tColouri::HSVToRGB()
{
	int h = H;
	int s = S;
	int v = V;
	int r, g, b;
	tHSVToRGB(r, g, b, h, s, v, tMath::tAngleMode::Norm256);
	R = r;
	G = g;
	B = b;
}


inline void tColourf::RGBToHSV()
{
	float r = R;
	float g = G;
	float b = B;
	tRGBToHSV(H, S, V, r, g, b, tMath::tAngleMode::NormOne);
}


inline void tColourf::HSVToRGB()
{
	float h = H;
	float s = S;
	float v = V;
	tHSVToRGB(R, G, B, h, s, v, tMath::tAngleMode::NormOne);
}
