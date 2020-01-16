// tMachine.h
//
// Hardware ans OS access functions like querying supported instruction sets, number or cores, and computer name/ip
// accessors.
//
// Copyright (c) 2004-2006, 2017, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tString.h>
#include "System/tThrow.h"
#include "System/tPrint.h"
namespace tSystem
{

// Returns true if the processor and the OS support Streaming SIMD Extensions.
bool tSupportsSSE();
bool tSupportsSSE2();

// Returns the computer's name.
tString tGetCompName();

// Returns the ip address of the machine if it is set. String will look like: "xxx.xxx.xxx.xxx". Returns an empty
// string if it is not set.
tString tGetIPAddress();

// Returns the number of cores (processors) the current machine has.
int tGetNumCores();

// Opens the Os's file explorer for the folder and file specified. If file doesn't exist, no file will be selected.
// If dir doesn't exist, an explorer window is opened at a location decided by the system.
bool tOpenSystemFileExplorer(const tString& dir, const tString& file);
bool tOpenSystemFileExplorer(const tString& fullFilename);

}
