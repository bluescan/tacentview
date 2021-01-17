// Undo.h
//
// An undo stack object that each image can use. Undo::Step is the type of object that may be pushed onto the
// stack. Undo namespace functions provide an overall interface for setting and querying things like memory use.
//
// Copyright (c) 2021 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tList.h>
#include <Foundation/tString.h>
#include <Image/tPicture.h>
namespace Undo
{


// An Step is is capable of undoing (or redoing) an operation.
class Step : public tLink<Step>
{
public:
	Step(const tString& desc, bool dirty)																				: Description(desc), Dirty(dirty) { }
	virtual ~Step()																										{ }

	tString Description;					// A biref description of the operation that this step undoes.
	bool Dirty;								// The dirty state prior to the operation.
};


// A particular type of restore step. This one is simple but takes quite a lot of memory. Good one to use while trying
// to get this running.
class Step_PictureList : public Step
{
public:
	Step_PictureList(const tString& desc, bool dirty, const tList<tImage::tPicture>& pics);
	virtual ~Step_PictureList()																							{ Pictures.Empty(); }
	void Restore(tList<tImage::tPicture>& pics);

	tList<tImage::tPicture> Pictures;
};


class Stack
{
public:
	// Call push before doing whatever op you are doing.
	void Push(tList<tImage::tPicture>& preOpState, const tString& desc, bool dirty);

	void Undo(tList<tImage::tPicture>& currPics, bool& dirty);
	void Redo(tList<tImage::tPicture>& currPics, bool& dirty);

	bool UndoAvailable() const { return !UndoSteps.IsEmpty(); }
	bool RedoAvailable() const { return !RedoSteps.IsEmpty(); }
	tString GetUndoDesc() const;
	tString GetRedoDesc() const;

private:
	tList<Step> UndoSteps;
	tList<Step> RedoSteps;
};


}


// Implementation only below.


inline tString Undo::Stack::GetUndoDesc() const
{
	return UndoSteps.IsEmpty() ? tString() : UndoSteps.Head()->Description;
}


inline tString Undo::Stack::GetRedoDesc() const
{
	return RedoSteps.IsEmpty() ? tString() : RedoSteps.Head()->Description;
}
