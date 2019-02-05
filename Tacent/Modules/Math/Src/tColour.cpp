// tColour.cpp
//
// Colour and pixel classes. Both a 32 bit integral representation as well as a 4 component floating point one can be
// found in this file.
//
// Copyright (c) 2006, 2011, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tColour.h>
using namespace tMath;


// Uses C++11 aggregate initialization syntax so construction happens earlier.
const tColouri tColouri::black		= { 0x00, 0x00, 0x00 };
const tColouri tColouri::white		= { 0xFF, 0xFF, 0xFF };
const tColouri tColouri::pink		= { 0xFF, 0x80, 0x80 };
const tColouri tColouri::red		= { 0xFF, 0x00, 0x00 };
const tColouri tColouri::green		= { 0x00, 0xFF, 0x00 };
const tColouri tColouri::blue		= { 0x00, 0x00, 0xFF };
const tColouri tColouri::grey		= { 0x80, 0x80, 0x80 };
const tColouri tColouri::lightgrey	= { 0xC0, 0xC0, 0xC0 };
const tColouri tColouri::darkgrey	= { 0x40, 0x40, 0x40 };
const tColouri tColouri::cyan		= { 0x00, 0xFF, 0xFF };
const tColouri tColouri::magenta	= { 0xFF, 0x00, 0xFF };
const tColouri tColouri::yellow		= { 0xFF, 0xFF, 0x00 };


const tColourf tColourf::invalid	= { -1.0f, -1.0f, -1.0f, -1.0f };
const tColourf tColourf::black		= { 0.0f, 0.0f, 0.0f };
const tColourf tColourf::white		= { 1.0f, 1.0f, 1.0f };
const tColourf tColourf::hotpink	= { 1.0f, 0.5f, 0.5f };
const tColourf tColourf::red		= { 1.0f, 0.0f, 0.0f };
const tColourf tColourf::green		= { 0.0f, 1.0f, 0.0f };
const tColourf tColourf::blue		= { 0.0f, 0.0f, 1.0f };
const tColourf tColourf::grey		= { 0.5f, 0.5f, 0.5f };
const tColourf tColourf::lightgrey	= { 0.75f, 0.75f, 0.75f };
const tColourf tColourf::darkgrey	= { 0.25f, 0.25f, 0.25f };
const tColourf tColourf::cyan		= { 0.0f, 1.0f, 1.0f };
const tColourf tColourf::magenta	= { 1.0f, 0.0f, 1.0f };
const tColourf tColourf::yellow		= { 1.0f, 1.0f, 0.0f };


void tRGBToHSV(int& h, int& s, int& v, int r, int g, int b, tAngleMode angleMode)
{
	double min = double( tMin(r, g, b) );
	double max = double( tMax(r, g, b) );
	double delta = max - min;

	v = int(max);
	if (!delta)
	{
		h = s = 0;
		return;
	}

	double temp = delta/max;
	s = int(temp*255.0);

	if (r == int(max))
		temp = double(g-b) / delta;
	else if (g == int(max))
		temp = 2.0 + double(b-r) / delta;
	else
		temp = 4.0 + double(r-g) / delta;

	// Compute hue in correct angle units.
	tAssert((angleMode == tAngleMode::Degrees) || (angleMode == tAngleMode::Norm256));
	double fullCircle = 360.0;
	if (angleMode == tAngleMode::Norm256)
		fullCircle = 256.0;

	temp *= fullCircle / 6.0;
	if (temp < 0.0)
		temp += fullCircle;

	if (temp >= fullCircle)
		temp = 0;

	h = int(temp);
}


void tHSVToRGB(int& r, int& g, int& b, int h, int s, int v, tAngleMode angleMode)
{
	tAssert((angleMode == tAngleMode::Degrees) || (angleMode == tAngleMode::Norm256));
	int fullCircle = 360;
	if (angleMode == tAngleMode::Norm256)
		fullCircle = 256;

	while (h >= fullCircle)
		h -= fullCircle;

	while (h < 0)
		h += fullCircle;

	tClamp(h, 0, fullCircle-1);
	tClamp(s, 0, 255);
	tClamp(v, 0, 255);

	if (!h && !s)
	{
		r = g = b = v;
		return;
	}

	double max = double(v);
	double delta = max*s / 255.0;
	double min = max - delta;
	double hue = double(h);
	double circle = double(fullCircle);
	double oneSixthCircle	= circle/6.0;		// 60 degrees.
	double oneThirdCircle	= circle/3.0;		// 120 degrees.
	double oneHalfCircle	= circle/2.0;		// 180 degrees.
	double twoThirdCircle	= (2.0*circle)/3.0;	// 240 degrees.
	double fiveSixthCircle	= (5.0*circle)/6.0;	// 300 degrees.

	if (h > fiveSixthCircle || h <= oneSixthCircle)
	{
		r = int(max);
		if (h > fiveSixthCircle)
		{
			g = int(min);
			hue = (hue - circle)/oneSixthCircle;
			b = int(min - hue*delta);
		}
		else
		{
			b = int(min);
			hue = hue / oneSixthCircle;
			g = int(hue*delta + min);
		}
	}
	else if (h > oneSixthCircle && h < oneHalfCircle)
	{
		g = int(max);
		if (h < oneThirdCircle)
		{
			b = int(min);
			hue = (hue/oneSixthCircle - 2.0) * delta;
			r = int(min - hue);
		}
		else
		{
			r = int(min);
			hue = (hue/oneSixthCircle - 2.0) * delta;
			b = int(min + hue);
		}
	}
	else
	{
		b = int(max);
		if (h < twoThirdCircle)
		{
			r = int(min);
			hue = (hue/oneSixthCircle - 4.0) * delta;
			g = int(min - hue);
		}
		else
		{
			g = int(min);
			hue = (hue/oneSixthCircle - 4.0) * delta;
			r = int(min + hue);
		}
	}
}


void tRGBToHSV(float& h, float& s, float& v, float r, float g, float b, tAngleMode angleMode)
{
	float min = tMin(r, g, b);
	float max = tMax(r, g, b);

	v = max;
	float delta = max - min;
	if (max > 0.0f)
	{
		s = (delta / max);
	}
	else
	{
		// Max is 0 so we're black with v = 0.
		// Hue and Sat are irrelevant at this point but we zero them to be clean.
		s = 0.0f;
		h = 0.0f;
	}

	if (r >= max)
		h = (g - b) / delta;				// Between yellow & magenta.
	else if (g >= max)
		h = 2.0f + (b - r) / delta;			// Between cyan & yellow.
	else
		h = 4.0f + (r - g) / delta;			// Between magenta & cyan.

	float fullCircle = 360.0f;
	switch (angleMode)
	{
		case tAngleMode::Radians:
			fullCircle = TwoPi;
			break;

		case tAngleMode::Degrees:
			fullCircle = 360.0f;
			break;

		case tAngleMode::Norm256:
			fullCircle = 256.0f;
			break;

		case tAngleMode::NormOne:
			fullCircle = 1.0f;
			break;
	}

	h *= fullCircle / 6.0f;

	if (h < 0.0f)
		h += fullCircle;
}


void tHSVToRGB(float& r, float& g, float& b, float h, float s, float v, tAngleMode angleMode)
{
	// If sat is zero we always ignore the hue. That is, we're a shade of grey on the vertical line.
	if (s <= 0.0f)
	{
		r = v;
		g = v;
		b = v;
		return;
	}

	float fullCircle = 360.0f;
	switch (angleMode)
	{
		case tAngleMode::Radians:
			fullCircle = TwoPi;
			break;

		case tAngleMode::Degrees:
			fullCircle = 360.0f;
			break;

		case tAngleMode::Norm256:
			fullCircle = 256.0f;
			break;

		case tAngleMode::NormOne:
			fullCircle = 1.0f;
			break;
	}

	if (h >= fullCircle)
		h = 0.0f;
	h /= (fullCircle/6.0f);

	int i = int(h);
	float rem = h - i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - (s * rem));
	float t = v * (1.0f - (s * (1.0f - rem)));

	switch (i)
	{
		case 0:
			r = v;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = v;
			b = p;
			break;

		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;

		case 4:
			r = t;
			g = p;
			b = v;
			break;

		case 5:
		default:
			r = v;
			g = p;
			b = q;
			break;
	}
}
