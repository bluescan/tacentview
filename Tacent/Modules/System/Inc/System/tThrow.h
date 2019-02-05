// tThrow.h
//
// Base classes for objects that may be thrown. Mostly warnings and errors.
//
// Copyright (c) 2004-2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once


// The tThrow header should not depend on any header that may need to throw an exception. This is why we can't inline
// the tError member functions. They would depend on tPrint which includes tFile, but tFile throws and needs tThrow.
// The same header included twice in the include chain doesn't work.
#include <Foundation/tString.h>


// All errors that can be thrown by any library module are derived from this. By catching exceptions of the tError
// type all possibilities can be covered. Conversely, derived classes can be caught for more control over who
// handles what type of error.
struct tError
{
	tError(const char* format, ...);
	tError(const tString& message);
	tError();
	tString Message;
};


struct tWarning
{
	tWarning(const char* format, ...);
	tWarning(const tString& message);
	tWarning();
	tString Message;
};
