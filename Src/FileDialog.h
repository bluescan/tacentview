// FileDialog.h
//
// Dialog that allows selection of a file or directory. May be used for opening a file/directory or saving to a file.
//
// Copyright (c) 2021, 2022 Tristan Grimmer.
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
#include <System/tFile.h>
#include <System/tScript.h>
namespace tFileDialog
{


// Call to save the dialogs settings (bookmarks, current dir/file, etc) to disk. Writes to the file tExprWriter
// was constructed with. All settings are shared between all instances of FileDialog. The expressionName is
// whatever you like. It is the name of the expression you will see in the written file. You should make sure it
// doesn't collide with any other names you may be using. Use the same name when loading. Saving may be performed
// at any time, even if a dialog is open. Returns false if writing fails.
bool Save(tExprWriter&, const tString& expressionName);

// Call to load the dialog settings from disk. By passing in a tExpr you can use your own config file along with
// other expressions. This function reads from the passed-in expression. Pass in a tExprReader directly if you
// want to load from a separate file. All settings are shared between all instances of FileDialog. The
// expressionName is the name of the expression you used when saving. Make sure it doesn't collide with any other
// names you may be using. Use the same name when saving. Returns false if fails to load.
bool Load(tExpr, const tString& expressionName);

void Reset();

enum class DialogMode
{
	OpenFile,
	OpenDir,
	SaveFile
};

struct ContentItem;
class TreeNode;


// You can use multiple instances or repurpose the same one.
class FileDialog : public tLink<FileDialog>
{
public:
	// In OpenDir dialog mode the file-types are ignored. If file-types is empty (default) then all types are used.
	FileDialog(DialogMode, const tSystem::tFileTypes& = tSystem::tFileTypes());
	~FileDialog();

	// Call when you want the modal dialog to open. If you want it to open in a specific directory supply the openDir
	// variable. If either the directory doesn't exist or you leave it blank, the last open directory is used.
	// Note that the dir is remembered in the config file, so persists across runs of the application.
	void OpenPopup(const tString& openDir = tString());

	enum class DialogState
	{
		Open,								// Modal is currently open.
		Closed,								// Modal is currently not open.
		OK,									// Call GetResult to see what the user selected.
		Cancel
	};
	DialogState DoPopup();					// Call every frame and observe returned state.
	tString GetResult();
	
private:
	// The dialog uses a 'path' to represent the current directory. A path consists of a list of strings from root to leaf.
	// You can convert between a directory string and a path using the functions below.
	// @todo The concept of a path made of a list should be brought over to Tacent where it can be formalized.
	tString GetDir(const TreeNode*);								// Converts to a directory string.
	void GetPath(tList<tStringItem>& destPath, const TreeNode*);	// Converts to a path list.
	void GetPath(tList<tStringItem>& destPath, const tString& dir);	// Converts dir to a path.

	void DoSelectable(ContentItem*);
	void DoFileTypesDropdown(bool supportMultipleTypes);

	void PopulateLocal();
	#ifdef PLATFORM_WINDOWS
	void PopulateNetwork();
	#endif

	// This currently does not invalidate network shares on windows because they take a long time to rebuild.
	// @todo Revisit this.
	void InvalidateTree();

	// Needed in cases where we need a reload of content. For example, when changing filetype filters.
	void InvalidateAllNodeContent();
	void InvalidateAllNodeContentRecursive(TreeNode*);
	void TreeNodeRecursive(TreeNode*, tStringItem* selectPathItemName = nullptr, bool setYScrollToSel = false);
	tStringItem* BookmarksLoop();

	#ifdef PLATFORM_WINDOWS
	void RequestNetworkSharesThread();
	bool ProcessShareResults();
	tSystem::tNetworkShareResult NetworkShareResults;
	bool ProcessingNetworkPath = false;
	#endif

	bool PopupJustOpened;
	DialogMode Mode;
	tSystem::tFileTypes FileTypes;
	tString Result;

	// @todo Once tString has proper support for resizing and a concept of current size vs capacity then
	// we can do away with this std::string and use Result directly.
	std::string SaveFileResult;

	TreeNode* LocalTreeNode;
	#ifdef PLATFORM_WINDOWS
	TreeNode* NetworkTreeNode;
	#endif
	TreeNode* SelectedNode = nullptr;
};


}
