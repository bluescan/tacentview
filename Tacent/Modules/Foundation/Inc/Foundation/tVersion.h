// tVersion.h
//
// Version numbers for Tacent.
//
// Copyright (c) 2004-2006, 2016, 2017 Tristan Grimmer.
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


namespace tVersion
{
	// Any break in binary compatibility must increment the major number, although non-breaking major improvements may
	// also justify a major version update.
	const uint Major = 0;

	// Minor version increments on non-breaking fixes and improvements to the current major version. When the major
	// version increments, the minor version resets to 0.
	const uint Minor = 6;

	// The revision number, also called a patch number, is for minor bug fixes and the like. It Resets to 0 when the
	// minor version increments.
	const uint Revision = 1;

	// Additionally, a change number is specified on Tacent releases. This is a monotonically increasing number that
	// never resets. @todo At some point I'll make a versioning tool so that this file, including the ChangeNumber
	// is generated from a script. The tool will use the perforce CL# for the ChangeNumber. If ever the versioning
	// system is changed or perforce reset, the tool will allow a constant offset to be added so this value never goes
	// down.
}
