// tThrow.cpp
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

#include "System/tPrint.h"
#include "System/tThrow.h"


// All errors that can be thrown by any library module are derived from this. By catching exceptions of the tError
// type all possibilities can be covered. Conversely, derived classes can be caught for more control over who handles
// what type of error.
tError::tError(const char* format, ...) :
	Message("Error: ")
{
	va_list marker;
	va_start(marker, format);
	Message += tvsPrintf(format, marker);
}


tError::tError(const tString& message) :
	Message("Error: ")
{
	Message += message;
}


tError::tError() :
	Message("Error: Unknown.")
{
}


tWarning::tWarning(const char* format, ...) :
	Message("Warning: ")
{
	va_list marker;
	va_start(marker, format);
	Message += tvsPrintf(format, marker);
}


tWarning::tWarning(const tString& message) :
	Message("Warning: ")
{
	Message += message;
}


tWarning::tWarning() :
	Message("Warning: Generic.")
{
}
