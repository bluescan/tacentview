// tAttribute.cpp
//
// All scene objects may have attributes. Attributes are extra data (that has a type) that the content author
// attributed to the scene object.
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

#include "Scene/tAttribute.h"
namespace tScene
{


void tAttribute::Save(tChunkWriter& chunk) const
{
	if (!IsValid())
		return;

	chunk.Begin(tChunkID::Scene_Attribute);
	{
		chunk.Begin(tChunkID::Scene_AttributeName);
		{
			chunk.Write(Name);
		}
		chunk.End();

		chunk.Begin(tChunkID::Scene_AttributeType);
		{
			chunk.Write(Type);
		}
		chunk.End();

		// Values must be switched on type so that endianness corrections are made.
		chunk.Begin(tChunkID::Scene_AttributeValue);
		{
			switch (Type)
			{
				case tType::Bool:
					chunk.Write(GetAsBool());
					break;

				case tType::Int:
					chunk.Write(GetAsInt());
					break;

				case tType::Colour:
					chunk.Write(GetAsColour());
					break;

				case tType::Float:
					chunk.Write(GetAsFloat());
					break;
			};
		}
		chunk.End();
	}
	chunk.End();
}


void tAttribute::Load(const tChunk& attribChunk)
{
	tAssert(attribChunk.GetID() == tChunkID::Scene_Attribute);
	Clear();

	for (tChunk chunk = attribChunk.First(); chunk.IsValid(); chunk = chunk.Next())
	{
		switch (chunk.GetID())
		{
			case tChunkID::Scene_AttributeName:
				chunk.GetItem(Name);
				break;

			case tChunkID::Scene_AttributeType:
				chunk.GetItem(Type);
				break;

			case tChunkID::Scene_AttributeValue:
				tAssert(Type != tType::Invalid);

				// We switch on the type here so that the tChunk system can handle any endian issues.
				switch (Type)
				{
					case tType::Bool:
					{
						bool val;
						chunk.GetItem(val);
						Value = val ? 1 : 0;
						break;
					}

					case tType::Int:
					{
						int val;
						chunk.GetItem(val);
						Value = int(val);
						break;
					}

					case tType::Colour:
					{
						tColouri val;
						chunk.GetItem(val);
						Value = *((uint32*)(&val));
						break;
					}

					case tType::Float:
					{
						float val;
						chunk.GetItem(val);
						Value = *((uint32*)(&val));
						break;
					}
				}
				break;
		}
	}
}


bool tAttributes::Contains(const tAttribute& attrib) const
{
	for (tItList<tAttribute>::Iter attr = Attributes.First(); attr; ++attr)
	{
		if (*attr == attrib)
			return true;
	}

	return false;
}


void tAttributes::Save(tChunkWriter& chunk) const
{
	chunk.Begin(tChunkID::Scene_AttributeList);
	for (tItList<tAttribute>::Iter attr = Attributes.First(); attr; ++attr)
		attr->Save(chunk);
	chunk.End();
}


void tAttributes::Load(const tChunk& chunk)
{
	Clear();
	tAssert(chunk.GetID() == tChunkID::Scene_AttributeList);
	for (tChunk attr = chunk.First(); attr.Valid(); attr = attr.Next())
		Attributes.Append(new tAttribute(attr));
}


bool tAttributes::operator==(const tAttributes& attribs) const
{
	if (Count() != attribs.Count())
		return false;

	for (tItList<tAttribute>::Iter attr = Attributes.First(); attr; ++attr)
	{
		if (!attribs.Contains(*attr))
			return false;
	}

	return true;
}


tAttributes& tAttributes::operator=(const tAttributes& attribs)
{
	if (this == &attribs)
		return *this;

	Clear();
	for (tItList<tAttribute>::Iter attr = attribs.Attributes.First(); attr; ++attr)
		Attributes.Append(new tAttribute(*attr));

	return *this;
}


}
