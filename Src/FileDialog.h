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
#include <string>
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
	// If FileTypes is not empty, selecting "All Types" in the dialog means all types supplied here.
	FileDialog(DialogMode, const tSystem::tFileTypes& = tSystem::tFileTypes());
	~FileDialog();

	// Call when you want the modal dialog to open. If you want it to open in a specific directory supply the openDir
	// variable. If either the directory doesn't exist or you leave it blank, the last open directory is used.
	// The last-open is remembered in the config file per-dialog-mode, and persists across runs of the application (so
	// long as you called tFileDialog:;Load of course). Note that the file dialog does all the work of figuring out
	// what is on the file-system (and network shares for Windows) when you call this function. Currently only the
	// network share query is offloaded to another thread. @todo Consider also affloading the root (main filesystem)
	// queries.
	//
	// saveFileBaseName is only used for the SaveFile dialog mode. If valid it pre-populates the filename without
	// extension in the dialog.
	void OpenPopup(const tString& openDir = tString(), const tString& saveFileBaseName = tString());

	enum class DialogState
	{
		Open,								// Modal is currently open.
		Closed,								// Modal is currently not open.
		OK,									// Call GetResult to see what the user selected.
		Cancel
	};
	DialogState DoPopup();					// Call every frame and observe returned state.
	tString GetResult();
	bool IsPopupOpen() const																							{ return PopupOpen; }
	bool SetPath(const tString& dirPath);
	
private:
	// The dialog uses a 'path' to represent the current directory. A path consists of a list of strings from root to
	// leaf. You can convert to a path from a directory string or a treenode, and you can convert to a directory string
	// from a treenode.
	// @todo The concept of a path made of a string list should be brought over to Tacent where it can be formalized.
	tString NodeToDir(const TreeNode*);									// Converts to a directory string.
	void NodeToPath(tList<tStringItem>& destPath, const TreeNode*);		// Converts to a path list.
	void DirToPath(tList<tStringItem>& destPath, const tString& dir);	// Converts dir to a path.

	void DoRefresh(tStringItem*& selectPathItemName, bool& setYScrollToSel);
	void DoSelectable(ContentItem*);
	void DoSelect(const tString& filename);
	void DoFileTypesDropdown(bool supportMultipleTypes);

	// You can call this more than once if you need to refresh the trees. The old trees (if any) are deleted.
	// For efficiency, only the top level is populated. The user must open directories manually to expand and
	// populate the tree dynamically.
	void PopulateTrees();

	// Needed in cases where we need a reload of content. For example, when changing filetype filters. This function
	// only invalidates the _content_ (the stuff in the right-hand panel). The tree itself remains intact and so
	// the SelectedNode does not need to be cleared.
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

	bool DoPopulate;
	DialogMode Mode;
	tSystem::tFileTypes FileTypes;
	tString Result;

	// std::string is used because InputTextWithHint needs a std::string.
	std::string SaveFileResult;

	TreeNode* RootTreeNode;
	#ifdef PLATFORM_WINDOWS
	TreeNode* NetworkTreeNode;
	#endif

	// This member points to a node in one of the supported trees -- RootTreeNode for Linux and both RootTreeNode
	// and NetworTreeNode for Windows. Be careful, the SelectedNode must be cleared or reset if the tree is deleted.
	TreeNode* SelectedNode = nullptr;
	bool PopupOpen = false;
	tString SelectFile;
};


}
