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


// This seems like a reasonable namespace for all generic ImGui widgets and dialogs that may eventually find a home
// outside this viewer app.
namespace tInterface
{


class FileDialog;


// Tree nodes are in the left panel. Used for directories and containers with special names
// like favourites, local, and network. A TreeNode has children TreeNodes.
class TreeNode
{
public:
	TreeNode()																											: Name(), Parent(nullptr) { }
	TreeNode(const tString& name, FileDialog* dialog, TreeNode* parent = nullptr)										: Name(name), Dialog(dialog), Parent(parent) { }

	void AppendChild(TreeNode* treeNode)																				{ Children.Append(treeNode); }

	// For windows these do a case-insensitive compare. For case sensitive filesystems like Linux uses
	// these to a case-sensitive compare.
	TreeNode* Find(const tString& name) const;
	bool Contains(const tString& name) const																			{ return Find(name) ? true : false; }
	int Depth() const;
	bool IsNetworkLocation() const;

	struct ContentItem : tLink<ContentItem>
	{
		ContentItem(const tString& name, bool isDir)																	: Name(name), Selected(false), IsDir(isDir) { }
		ContentItem(const tSystem::tFileInfo& fileInfo);

		tString Name;
		bool Selected;
		bool IsDir;

		// These are not valid for directories.
		tString FileSizeString;
		tString ModTimeString;
	};
	ContentItem* FindSelectedItem() const;

	tString Name;
	FileDialog* Dialog;
	bool ChildrenPopulated = false;
	TreeNode* Parent;
	tItList<TreeNode> Children;

	bool NextOpen = false;
	bool ContentsPopulated = false;
	tList<ContentItem> Contents;
};


// You can use multiple instances or repurpose the same one.
class FileDialog
{
public:
	enum class DialogMode
	{
		OpenFile,
		OpenFiles,
		OpenDir,
		SaveFile
	};

	FileDialog(DialogMode);
	void OpenPopup();						// Call when you want the modal dialog to open.

	enum class DialogResult
	{
		Open,								// Modal is currently open.
		Closed,								// Modal is currently not open.
		OK,									// Call GetResult to see what the user selected.
		Cancel
	};
	DialogResult DoPopup();					// Call every frame and observe result.
	tString GetResult();

private:
	tString GetSelectedDir();
	void DoSelectable(const char* label, TreeNode::ContentItem*);

	void PopulateFavourites();
	void PopulateLocal();
	void FavouritesTreeNodeFlat(TreeNode*);
	void TreeNodeRecursive(TreeNode*);

	#ifdef PLATFORM_WINDOWS
	void PopulateNetwork();
	void NetworkTreeNodeRecursive(TreeNode*);
	void RequestNetworkSharesThread();
	void ProcessShareResults();
	tSystem::tNetworkShareResult NetworkShareResults;
	#endif

	bool ProcessingNetworkPath = false;
	DialogMode Mode;
	tString Result;

	TreeNode* FavouritesTreeNode;
	TreeNode* LocalTreeNode;
	TreeNode* NetworkTreeNode;
	TreeNode* SelectedNode = nullptr;
};


}
