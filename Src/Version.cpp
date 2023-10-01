// Version.cpp
//
// Version string parser to extract major, minor, and rev fro the header.
//
// Copyright (c) 2020, 2023 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "Version.cmake.h"
#include <Foundation/tString.h>


namespace ViewerVersion
{
	int Major = 0;
	int Minor = 0;
	int Revision = 0;
	bool Parsed = false;
}


ViewerVersion::Parser::Parser(const char* verStr)
{
	if (Parsed)
		return;

	tList<tStringItem> components;
	tStd::tExplode(components, tString(verStr), '.');

	tStringItem* comp = components.First();	Major = comp->GetAsInt(10);
	comp = comp->Next();					Minor = comp->GetAsInt(10);
	comp = comp->Next();					Revision = comp->GetAsInt(10);
	Parsed = true;
}
