// tSelection.h
//
// This file implements scene selection sets. A selection is simply a collection of scene instances referenced by ID.
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Scene/tObject.h>
#include "Scene/tSelection.h"
namespace tScene
{


class tSelection : public tObject
{
public:
	tSelection()																										{ }
	tSelection(const tChunk& chunk)																						{ Load(chunk); }
	virtual ~tSelection()																								{ }

	bool ContainsInstance(uint32 instID) const;
	int GetNumItems() const																								{ return InstanceIDs.GetNumItems(); }
	void Clear()																										{ tObject::Clear(); InstanceIDs.Empty(); }

	void Save(tChunkWriter&) const;
	void Load(const tChunk&);

	tItList<uint32> InstanceIDs;
};


// Implementation below this line.


inline bool tSelection::ContainsInstance(uint32 instID) const
{
	for (tItList<uint32>::Iter i = InstanceIDs.First(); i; ++i)
		if (*i.GetObject() == instID)
			return true;

	return false;
}


}
