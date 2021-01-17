// Undo.cpp
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

#include "Undo.h"
#include "Image.h"
#include "Settings.h"
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;


Undo::Step_PictureList::Step_PictureList(const tString& desc, bool dirty, const tList<tImage::tPicture>& pics) :
	Step(desc, dirty)
{
	for (tPicture* pic = pics.First(); pic; pic = pic->Next())
		Pictures.Append(new tPicture(*pic));

}


void Undo::Step_PictureList::Restore(tList<tImage::tPicture>& pics)
{
	pics.Clear();
	for (tPicture* restorePic = Pictures.First(); restorePic; restorePic = restorePic->Next())
		pics.Append(new tPicture(*restorePic));
}


void Undo::Stack::Push(tList<tImage::tPicture>& preOpState, const tString& desc, bool dirty)
{
	// Create the undo step.
	Undo::Step_PictureList* step = new Undo::Step_PictureList(desc, dirty, preOpState);
	UndoSteps.Insert(step);

	// Drop one from the end if we've reached the limit.
	int numUndoSteps = UndoSteps.Count();
	if (numUndoSteps > Viewer::Config.MaxUndoSteps)
		delete UndoSteps.Drop();
}


void Undo::Stack::Undo(tList<tImage::tPicture>& currPics, bool& dirty)
{
	if (UndoSteps.IsEmpty())
		return;

	Step* undoStep = UndoSteps.Remove();

	// We're going to need a redo step to get to current state. Prepare it first.
	Step* redoStep = new Step_PictureList(undoStep->Description, dirty, currPics);

	((Step_PictureList*)undoStep)->Restore(currPics);
	dirty = undoStep->Dirty;
	delete undoStep;

	RedoSteps.Insert(redoStep);
}


void Undo::Stack::Redo(tList<tImage::tPicture>& currPics, bool& dirty)
{
	if (RedoSteps.IsEmpty())
		return;

	Step* redoStep = RedoSteps.Remove();

	// We're going to need an undo step to get to current state. Prepare it first.
	Step* undoStep = new Step_PictureList(redoStep->Description, dirty, currPics);

	((Step_PictureList*)redoStep)->Restore(currPics);
	dirty = redoStep->Dirty;
	delete redoStep;

	UndoSteps.Insert(undoStep);
}
