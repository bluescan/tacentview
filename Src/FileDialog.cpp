// FileDialog.cpp
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

#include <Foundation/tList.h>
#include <Math/tVector2.h>
#include <System/tTime.h>
#include "imgui.h"
#include "imgui_internal.h"		// For SplitterBehavior.
#include "imgui_stdlib.h"		// For editing a std::string.
#include "FileDialog.h"
#include "TacentView.h"
#include "Image.h"
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;


namespace tFileDialog
{
	// Content items are containers used to store information about a piece of information in a file dialog. It stores
	// the stuff you see in the right-hand panel. Namely the information displayed in a table row about a file or
	// directory, as well as if it is selected or not.
	struct ContentItem : tLink<ContentItem>
	{
		ContentItem(const tString& dirName);
		ContentItem(const tSystem::tFileInfo& fileInfo);
		bool Selected;

		// Each field (column) gets its own ID. This is needed so when sorting, we know what information to use in the
		// sorting function. We tell ImGui about the ID, and it passes that along when it tells us to sort.
		enum class FieldID
		{
			Invalid = -1,
			Name,
			ModTime,
			FileType,
			FileSize
		};

		struct CompareFunctionObject
		{
			CompareFunctionObject(ImGuiTableSortSpecs* specs) : SortSpecs(specs) { }
			ImGuiTableSortSpecs* SortSpecs;
			bool operator() (const ContentItem& a, const ContentItem& b) const;
		};

		// Name field.
		tString Name;
		bool IsDir;

		// These are not valid for directories. Note that we store field data redundantly in some cases since we need to be
		// able to sort based on a key decoupled from the display string. For example, sorting by date should be done with
		// an integer, not the string representation. Same with filesizes, we may want them displayed in KB or MB is they
		// get large, but want to sort on num-bytes. We store the string reps because it's more efficient to cache them
		// rather than regenerate them every time.

		// Modification time (last) field.
		int64 ModTime;							// In posix epoch.
		tString ModTimeString;					// Displayed.

		// File type field.
		tSystem::tFileType FileType;
		tString FileTypeString;					// Displayed.

		// File size field.
		int64 FileSize;
		tString FileSizeString;					// Displayed.
	};

	// Tree nodes are displayed directly in the left panel and their contained Contents list in the right. TreeNodes
	// represent directories and other containers like favourites and network shares. A TreeNode has children TreeNodes.
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

		ContentItem* FindSelectedItem() const;

		tString Name;
		FileDialog* Dialog;
		bool ChildrenPopulated = false;
		TreeNode* Parent;
		tItList<TreeNode> Children;

		bool NextOpen = false;

		// Is the Contents list below populated and valid. It is slightly different than having an empty Contents list
		// because a leaf empty directory may have no contents, but it HAS read the filesystem so is populated.
		bool ContentsPopulated = false;
		tList<ContentItem> Contents;
	};

	const int FileDialogConfigVersion = 1;
	tList<FileDialog> FileDialogs(tListMode::StaticZero);				// An internal list of all currently instanced file dialogs.
	tList<tStringItem> ConfigOpenFilePath(tListMode::StaticZero);
	tList<tStringItem> ConfigOpenDirPath(tListMode::StaticZero);
	tList<tStringItem> ConfigSaveFilePath(tListMode::StaticZero);

	// Stores an individual bookmark comprised of a list of string path items. Bookmarks currently
	// represent directories, not individual files.
	struct Bookmark : public tLink<Bookmark>
	{
		enum class Type
		{
			User,								// A normal directory bookmark.
			Home,								// A bookmark of the user's home directory (Linux) or the user directory (Windows).
			Root								// A bookmark of the root of the filesyetem (Linux) or an appropriate drive letter (Windows).
		};
		Bookmark()								/* Creates an invalid bookmark. */										: Items() { }
		Bookmark(const Bookmark& src)			/* Copy cons. */														: Items() { Set(src); }
		Bookmark(const tString& fullPath)		/* From a full path string. */											: Items() { Set(fullPath); }
		Bookmark(const tList<tStringItem>& i)	/* A user bookmark from item list. */									: Items() { Set(i); }
		Bookmark(Type type)						/* Accepts Home or Root. */												: Items() { Set(type); }
		Bookmark(tExpr expr)																							: Items() { Load(expr); }
		~Bookmark()																										{ Items.Empty(); }

		bool Set(const Bookmark& src);
		bool Set(const tString& fullPath);
		bool Set(const tList<tStringItem>&);	// A user bookmark from item list.
 		bool Set(Type type);					// Accepts Home or Root.

		void Clear()							/* Makes bookmark invalid. */											{ Items.Empty(); }
		bool IsValid() const					/* A bookmark is valid if it has items, even if it doesn't exist. */	{ return !Items.IsEmpty(); }
		bool Exists() const;					// Does the bookmark still exist on the filesystem. In case the directory was deleted.
		tString GetPath() const;				// Converts from internal item list to full path.

		bool Save(tExprWriter&) const;
		bool Load(tExpr expr);
		bool operator==(const Bookmark&) const;

		Type BookmarkType					= Type::User;
		tList<tStringItem> Items;
		bool Selected						= false;
	};
	tList<Bookmark> Bookmarks;
	void EnsureDefaultBookmarksExist();
	void UnselectAllBookmarks();
	bool AddUniqueBookmark(const tList<tStringItem>&);
	void ClearBookmarksSelected();

	// Splitter functions. @todo These should go somewhere more public as they are not particularly tied to
	// the file dialog.
	bool HorizontalSplitter(float thickness, float& height1, float& height2, float minHeight1, float minHeight2, float hoverExtend = 4.0f, float hoverDelay = 0.0f);
	bool VerticalSplitter(float thickness, float& width1, float& width2, float minWidth1, float minWidth2, float hoverExtend = 4.0f, float hoverDelay = 0.0f);
}


using namespace tFileDialog;


bool Bookmark::Set(const tString& fullPath)
{
	Items.Empty();
	BookmarkType = Type::User;
	tAssert(!"Bookmark::Set not implemented.");
	return false;
}


bool Bookmark::Set(const tList<tStringItem>& src)
{
	Items.Empty();
	BookmarkType = Type::User;
	for (tStringItem* i = src.First(); i; i = i->Next())
		Items.Append(new tStringItem(*i));

	return true;
}


bool Bookmark::Set(Type type)
{
	if (type == Type::User)
		return false;

	if (type == Type::Home)
	{
		BookmarkType = Type::Home;
 
		// All Windows platforms get a 'user' bookmark. Linux gets the 'home' location.
		tString home = tSystem::tGetHomeDir();

		tList<tStringItem> exploded;
		home.ExtractLeft("/");
		tStd::tExplode(exploded, home, '/');

		Items.Append(new tStringItem("Local"));
		for (tStringItem* item = exploded.First(); item; item = item->Next())
			Items.Append(new tStringItem(*item));
	}
	else if (type == Type::Root)
	{
		BookmarkType = Type::Root;
		Items.Append(new tStringItem("Local"));

		#ifdef PLATFORM_WINDOWS
		tList<tStringItem> drives;
		tSystem::tGetDrives(drives);
		tString root;
		for (tStringItem* drive = drives.First(); drive; drive = drive->Next())
		{
			tDriveInfo driveInfo;
			bool ok = tGetDriveInfo(driveInfo, *drive);
			if (ok && (driveInfo.DriveType != tDriveType::Removable))
			{
				root = *drive;
				break;
			}			
		}
		if (root.IsValid())
			Items.Append(new tStringItem(root));
		else
			Items.Append(new tStringItem("C:"));
		#endif

		#ifdef PLATFORM_LINUX
		Items.Append(new tStringItem("/"));
		#endif
	}
	return true;
}


bool Bookmark::Exists() const
{
	if (!IsValid())
		return false;

	if (BookmarkType != Type::User)
		return true;

	tString loc = *Items.First();

	// We assume network locations exist for now.
	if (loc != "Local")
		return true;

	tString path;
	for (tStringItem* item = Items.First()->Next(); item; item = item->Next())
		path += *item + "/";

	return tSystem::tDirExists(path);
}


tString Bookmark::GetPath() const
{
	if (!IsValid())
		return tString();
	
	tString pathType = *Items.First();
	bool isNetworkLoc = (pathType == "Network");

	tString dir;
	int depth = 0;
	for (tStringItem* item = Items.First()->Next(); item && item->IsValid(); item = item->Next(), depth++)
	{
		if (isNetworkLoc && (depth == 0))
			dir += "\\\\" + *item + "\\";
		else
			dir += *item + "/";
	}

	return dir;
}


bool Bookmark::Save(tExprWriter& writer) const
{
	if (!IsValid())
		return false;

	writer.Begin();
	writer.Atom("Bookmark");
	switch (BookmarkType)
	{
		case Type::User:	writer.Atom("User");	break;
		case Type::Home:	writer.Atom("Home");	break;
		case Type::Root:	writer.Atom("Root");	break;
	}

	for (tStringItem* i = Items.First(); i; i = i->Next())
		writer.Atom(*i);
	writer.End();

	return true;
}


bool Bookmark::Load(tExpr expr)
{
	Items.Empty();
	if (expr.Item0() != "Bookmark")
		return false;

	if (expr.CountItems() < 3)
		return false;

	switch (expr.Item1().Hash())
	{
		case tHash::tHashCT("User"):	BookmarkType = Type::User;		break;
		case tHash::tHashCT("Home"):	BookmarkType = Type::Home;		break;
		case tHash::tHashCT("Root"):	BookmarkType = Type::Root;		break;
	}

	for (tExpr e = expr.Item2(); e.IsValid(); e = e.Next())
		Items.Append(new tStringItem(e.GetAtomString()));

	return false;
}


bool Bookmark::operator==(const Bookmark& b) const
{
	if (!IsValid() && !b.IsValid())
		return true;

	if (Items.GetNumItems() != b.Items.GetNumItems())
		return false;

	if (BookmarkType != b.BookmarkType)
		return false;

	const tStringItem* bitem = b.Items.First();
	for (const tStringItem* item = Items.First(); item; item = item->Next(), bitem = bitem->Next())
		if (*item != *bitem)
			return false;

	return true;
}


bool tFileDialog::Save(tExprWriter& writer, const tString& exprName)
{
	writer.CR();
	writer.Rem("File dialog configuration.");
	writer.Begin();			writer.Indent();	writer.CR();
	writer.Atom(exprName);	writer.CR();

	writer.Begin();
	writer.Atom("FileDialogConfigVersion");		writer.Atom(FileDialogConfigVersion);
	writer.End();
	writer.CR();

	// We save a different last-used path for each mode.
	if (!ConfigOpenFilePath.IsEmpty())
	{
 		writer.Begin();
		writer.Atom("OpenFilePath");
		for (tStringItem* n = ConfigOpenFilePath.First(); n; n = n->Next())
			writer.Atom(*n);
		writer.End();
		writer.CR();
	}

	if (!ConfigOpenDirPath.IsEmpty())
	{
		writer.Begin();
		writer.Atom("OpenDirPath");
		for (tStringItem* n = ConfigOpenDirPath.First(); n; n = n->Next())
			writer.Atom(*n);
		writer.End();
		writer.CR();
	}

	if (!ConfigSaveFilePath.IsEmpty())
	{
		writer.Begin();
		writer.Atom("SaveFilePath");
		for (tStringItem* n = ConfigSaveFilePath.First(); n; n = n->Next())
			writer.Atom(*n);
		writer.End();
		writer.CR();
	}

	if (!Bookmarks.IsEmpty())
	{
		// Save all bookmarks. We save all 3 types.
		writer.Begin();
		writer.Atom("Bookmarks"); writer.Indent();
		for (Bookmark* bookmark = Bookmarks.First(); bookmark; bookmark = bookmark->Next())
		{
			writer.CR();
			bookmark->Save(writer);
		}
		writer.Dedent();
		writer.CR();
		writer.End();
	}

	writer.Dedent();
	writer.CR();
	writer.End();
	return true;
}


bool tFileDialog::Load(tExpr expr, const tString& exprName)
{
	tExpr e = expr.Item0();
	int loadedVersion = 0;
	if (e == exprName)
	{
		for (tExpr e = expr.Item1(); e.IsValid(); e = e.Next())
		{
			switch (e.Command().Hash())
			{
				case tHash::tHashCT("FileDialogConfigVersion"):
					loadedVersion = e.Item1();
					break;

				case tHash::tHashCT("OpenFilePath"):
					ConfigOpenFilePath.Empty();
					for (tExpr ne = e.Item1(); ne.IsValid(); ne = ne.Next())
						ConfigOpenFilePath.Append(new tStringItem(ne.GetAtomString()));
					break;

				case tHash::tHashCT("OpenDirPath"):
					ConfigOpenDirPath.Empty();
					for (tExpr ne = e.Item1(); ne.IsValid(); ne = ne.Next())
						ConfigOpenDirPath.Append(new tStringItem(ne.GetAtomString()));
					break;

				case tHash::tHashCT("SaveFilePath"):
					ConfigSaveFilePath.Empty();
					for (tExpr ne = e.Item1(); ne.IsValid(); ne = ne.Next())
						ConfigSaveFilePath.Append(new tStringItem(ne.GetAtomString()));
					break;

				case tHash::tHashCT("Bookmarks"):
					Bookmarks.Empty();
					for (tExpr be = e.Item1(); be.IsValid(); be = be.Next())
					{
						// We only keep the bookmark if it still exists on the filesystem.
						Bookmark* bookmark = new Bookmark(be);
						if (bookmark->Exists())
							Bookmarks.Append(bookmark);
						else
							delete bookmark;
					}
					break;
			}
		}
	}

	// Now that bookmarks are loaded and exist, we make sure both a home and root bookmark exist.
	EnsureDefaultBookmarksExist();
	return true;
}


void tFileDialog::EnsureDefaultBookmarksExist()
{
	Bookmark* homeBM = nullptr;
	for (Bookmark* bookmark = Bookmarks.First(); bookmark; bookmark = bookmark->Next())
		if (bookmark->BookmarkType == Bookmark::Type::Home)
		{
			homeBM = bookmark;
			break;
		}

	Bookmark* rootBM = nullptr;
	for (Bookmark* bookmark = Bookmarks.First(); bookmark; bookmark = bookmark->Next())
		if (bookmark->BookmarkType == Bookmark::Type::Root)
		{
			rootBM = bookmark;
			break;
		}

	if (!rootBM)
		Bookmarks.Insert(new Bookmark(Bookmark::Type::Root));

	if (!homeBM)
		Bookmarks.Insert(new Bookmark(Bookmark::Type::Home));
}


void tFileDialog::UnselectAllBookmarks()
{
	for (Bookmark* bookmark = Bookmarks.First(); bookmark; bookmark = bookmark->Next())
		bookmark->Selected = false;
}


bool tFileDialog::AddUniqueBookmark(const tList<tStringItem>& items)
{
	Bookmark* bookmarkToAdd = new Bookmark(items);
	bool found = false;
	for (Bookmark* b = Bookmarks.First(); b; b = b->Next())
	{
		if (*bookmarkToAdd == *b)
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		delete bookmarkToAdd;
		return false;
	}

	Bookmarks.Append(bookmarkToAdd);
	return true;
}


void tFileDialog::ClearBookmarksSelected()
{
	for (Bookmark* b = Bookmarks.First(); b; b = b->Next())
		b->Selected = false;
}


void tFileDialog::Reset()
{
	ConfigOpenFilePath.Empty();
	ConfigOpenDirPath.Empty();
	ConfigSaveFilePath.Empty();
	Bookmarks.Empty();
	EnsureDefaultBookmarksExist();
}


ContentItem::ContentItem(const tString& dirName) :
	Selected(false),
	IsDir(true)
{
	Name = dirName;
	ModTime = 0;
	ModTimeString.Clear();
	FileType = tSystem::tFileType::Invalid;
	FileSize = 0;
	FileSizeString.Clear();
}


ContentItem::ContentItem(const tSystem::tFileInfo& fileInfo) :
	Selected(false)
{
	// Name field.
	Name = tSystem::tGetFileName(fileInfo.FileName);
	IsDir = fileInfo.Directory;

	// Modification time field. Choose greater of mod and creation time.
	ModTime = tMath::tMax(fileInfo.ModificationTime, fileInfo.CreationTime);
	tsPrintf(ModTimeString, "%s", tSystem::tConvertTimeToString(tSystem::tConvertTimeToLocal(ModTime)).Chr());

	// File type field.
	FileType = tSystem::tGetFileType(Name);
	FileTypeString.Set(tSystem::tGetFileTypeName(FileType));

	// File size field.
	FileSize = fileInfo.FileSize;
	if (FileSize < 10000)
		tsPrintf(FileSizeString, "%'d B", FileSize);
	else if (FileSize < 10000000)
		tsPrintf(FileSizeString, "%'d KB", FileSize/1024);
	else if (FileSize < 10000000000)
		tsPrintf(FileSizeString, "%'d MB", FileSize/(1024*1024));
	else
		tsPrintf(FileSizeString, "%'d GB", FileSize/(1024*1024*1024));
}


bool ContentItem::CompareFunctionObject::operator() (const ContentItem& a, const ContentItem& b) const
{
	tAssert(SortSpecs);

	// We always put directories at the top regardless of field contents and sorting order.
	if (a.IsDir && !b.IsDir)
		return true;

	// This makes both the directories (that come first) as well as the files (that come after) sort correctly.
	// That is, do the sorting if we're comparing file-to-file or dir-to-dir.
	if (a.IsDir == b.IsDir)
	{
		// NumSpecs will be > 1 if more than one column is selected for sorting. The order is important.
		int numSpecs = SortSpecs->SpecsCount;
		for (int s = 0; s < numSpecs; s++)
		{
			const ImGuiTableColumnSortSpecs& colSortSpec = SortSpecs->Specs[s];
			FieldID field = FieldID(colSortSpec.ColumnUserID);
			int64 delta = 0;
			switch (field)
			{
				case FieldID::Name:
					delta = tStd::tStricmp(a.Name.Chars(), b.Name.Chars());
					break;

				case FieldID::ModTime:
					delta = a.ModTime - b.ModTime;
					break;

				case FieldID::FileType:
					delta = tStd::tStricmp(a.FileTypeString.Chars(), b.FileTypeString.Chars());
					break;

				case FieldID::FileSize:
					delta = a.FileSize - b.FileSize;
					break;
			}
			if ((delta < 0) && (colSortSpec.SortDirection == ImGuiSortDirection_Ascending))
				return true;
			else if ((delta > 0) && (colSortSpec.SortDirection == ImGuiSortDirection_Descending))
				return true;
		}
	}

	return false;
}


TreeNode* TreeNode::Find(const tString& name) const
{
	for (tItList<TreeNode>::Iter child = Children.First(); child; child++)
	{
		#ifdef PLATFORM_WINDOWS
		if (child->Name.IsEqualCI(name))
		#else
		if (child->Name == name)
		#endif
			return child;
	}
	return nullptr;
}


int TreeNode::Depth() const
{
	int depth = 0;
	TreeNode* parent = Parent;
	while (parent)
	{
		depth++;
		parent = parent->Parent;
	}
	return depth;
}


ContentItem* TreeNode::FindSelectedItem() const
{
	for (ContentItem* item = Contents.First(); item; item = item->Next())
		if (item->Selected)
			return item;

	return nullptr;
}


bool TreeNode::IsNetworkLocation() const
{
	bool isNet = false;
	const TreeNode* curr = this;
	while (curr)
	{
		if ((curr->Depth() == 0) && (curr->Name == "Network"))
			isNet = true;
		curr = curr->Parent;
	}
	return isNet;
}


FileDialog::FileDialog(DialogMode mode, const tSystem::tFileTypes& fileTypes) :
	PopupJustOpened(false),
	Mode(mode),
	FileTypes(fileTypes)
{
	LocalTreeNode = new tFileDialog::TreeNode("Local", this);
	#ifdef PLATFORM_WINDOWS
	NetworkTreeNode = new TreeNode("Network", this);
	#endif

	PopulateLocal();
	#ifdef PLATFORM_WINDOWS
	PopulateNetwork();
	#endif

	FileDialogs.Append(this);
}


FileDialog::~FileDialog()
{
	FileDialogs.Remove(this);
}


void FileDialog::OpenPopup()
{
	switch (Mode)
	{
		case DialogMode::OpenDir:
			ImGui::OpenPopup("Open Directory");
			break;

		case DialogMode::OpenFile:
			ImGui::OpenPopup("Open File");
			break;

		case DialogMode::SaveFile:
			ImGui::OpenPopup("Save File");
			break;
	}

	PopupJustOpened	= true;
}


void FileDialog::PopulateLocal()
{
	#if defined(PLATFORM_WINDOWS)
	tList<tStringItem> drives;
	tSystem::tGetDrives(drives);

	for (tStringItem* drive = drives.First(); drive; drive = drive->Next())
	{
		TreeNode* driveNode = new TreeNode(*drive, this, LocalTreeNode);
		LocalTreeNode->AppendChild(driveNode);	
	}

	#elif defined(PLATFORM_LINUX)
	TreeNode* rootNode = new TreeNode("/", this, LocalTreeNode);
	LocalTreeNode->AppendChild(rootNode);

	#endif
}


#ifdef PLATFORM_WINDOWS
void FileDialog::RequestNetworkSharesThread()
{
	tSystem::tGetNetworkShares(NetworkShareResults);
}


void FileDialog::PopulateNetwork()
{
	// Start the request on a different thread.
	std::thread threadGetShares(&FileDialog::RequestNetworkSharesThread, this);
	threadGetShares.detach();
}


bool FileDialog::ProcessShareResults()
{
	// Most of time this will be empty since we remove the items when they are available, just like a message queue.
	tStringItem* share = NetworkShareResults.ShareNames.Remove();
	bool somethingAdded = false;
	if (share)
	{
		tList<tStringItem> exploded;
		tExplodeShareName(exploded, *share);
		tString machName = *exploded.First();
		if (!NetworkTreeNode->Contains(machName))
		{
			TreeNode* machine = new TreeNode(machName.Text(), this, NetworkTreeNode);
			NetworkTreeNode->AppendChild(machine);
			somethingAdded = true;
		}

		TreeNode* machNode = NetworkTreeNode->Find(machName);
		tAssert(machNode);
		tStringItem* shareName = (exploded.GetNumItems() == 2) ? exploded.Last() : nullptr;
		if (shareName && !machNode->Contains(*shareName))
		{
			TreeNode* shareNode = new TreeNode(shareName->Text(), this, machNode);
			machNode->AppendChild(shareNode);
			somethingAdded = true;
		}
		delete share;
	}

	return somethingAdded;
}
#endif


void FileDialog::TreeNodeRecursive(TreeNode* node, tStringItem* selectPathItemName, bool setYScrollToSel)
{
	bool isSelected = (SelectedNode == node);
	int flags =
		ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen |
		((node->Children.GetNumItems() == 0) ? ImGuiTreeNodeFlags_Leaf : 0) |
		(isSelected ? ImGuiTreeNodeFlags_Selected : 0) |
		(isSelected ? ImGuiTreeNodeFlags_AllowItemOverlap : 0);

	bool populate = false;
	bool setScroll = false;
	if (selectPathItemName)
	{
		if (*selectPathItemName == node->Name)
		{
			node->NextOpen = true;
			selectPathItemName = selectPathItemName->Next();
			SelectedNode = node;
			if (setYScrollToSel)
				setScroll = true;
		}
		else
		{
			selectPathItemName = nullptr;
			setYScrollToSel = false;
		}
	}

	// This is the magic that's needed if you select from the right-hand panel.
	if (node->NextOpen)
	{
		ImGui::SetNextItemOpen(true);
		node->NextOpen = false;
		populate = true;
	}

	bool isOpen = ImGui::TreeNodeEx(node->Name.Chr(), flags);
	bool isClicked = ImGui::IsItemClicked();

	// If selected we need a button for adding to bookmarks.
	if (isSelected)
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("  +  ##BookmarkAdd"))
			ImGui::OpenPopup("DirContextMenu");

		if (ImGui::BeginPopup("DirContextMenu")) 
		{
			if (ImGui::MenuItem("Add Bookmark"))
			{
				tList<tStringItem> bookmarkItems;
				GetDir(bookmarkItems, node);
				AddUniqueBookmark(bookmarkItems);
			}
			ImGui::EndPopup();
		}	
	}

	if (setScroll)
	{
		// Couldn't get ImGui::SetScrollHereY() to work. The 2 gives us 2 lines so we have one
		// non-selected line above the selected one (for a bit of context).
		float cy = ImGui::GetCursorPosY();
		ImGui::SetScrollY(cy - ImGui::GetTextLineHeightWithSpacing()*2.0f);
	}

	if (isClicked)
	{
		if (!selectPathItemName)
		{
			SelectedNode = node;

			// SelectedNode Updated. Need to update global (saved) FilePath.
			if (Mode == DialogMode::OpenFile)
				GetDir(ConfigOpenFilePath, SelectedNode);
			else if (Mode == DialogMode::OpenDir)
				GetDir(ConfigOpenDirPath, SelectedNode);
			if (Mode == DialogMode::SaveFile)
				GetDir(ConfigSaveFilePath, SelectedNode);

			ClearBookmarksSelected();
		}

		populate = true;
	}

	// We only bother populating children if we need to.
	if (populate && !node->ChildrenPopulated)
	{
		#ifdef PLATFORM_WINDOWS
		if (!ProcessingNetworkPath || (node->Depth() >= 2))
		#endif
		{
			// Need to be careful here. tFindDirs would (reasonably?) use the current working dir if we passed in an empty string.
			tString currDir = GetDir(node);
			tList<tStringItem> foundDirs;
			if (!currDir.IsEmpty())
				tSystem::tFindDirs(foundDirs, currDir);

			for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
			{
				tString relDir = tSystem::tGetRelativePath(currDir, *dir);
				relDir.ExtractLeft("./");
				relDir.ExtractRight("/");

				TreeNode* child = new TreeNode(relDir, this, node);
				node->AppendChild(child);
			}
			node->ChildrenPopulated = true;
		}
	}

	if (isOpen)
	{
		// Recurse children.
		for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
			TreeNodeRecursive(child.GetObject(), selectPathItemName, setYScrollToSel);

		ImGui::TreePop();
	}
}


void FileDialog::InvalidateAllNodeContent()
{
	#ifdef PLATFORM_WINDOWS
	InvalidateAllNodeContentRecursive(NetworkTreeNode);
	#endif
	InvalidateAllNodeContentRecursive(LocalTreeNode);
}


void FileDialog::InvalidateAllNodeContentRecursive(TreeNode* node)
{
	node->Contents.Empty();
	node->ContentsPopulated = false;

	for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
		InvalidateAllNodeContentRecursive(child.GetObject());
}


void FileDialog::DoSelectable(ContentItem* item)
{
	tString label = item->Name;
	if (ImGui::Selectable(label.Chr(), &item->Selected, ImGuiSelectableFlags_SpanAllColumns))
	{
		// This block enforces single selection.
		if (item->Selected)
		{
			// Only allowed one selected max. Clear the others.
			for (ContentItem* i = SelectedNode->Contents.First(); i; i = i->Next())
			{
				if (i == item)
					continue;
				i->Selected = false;
			}

			// If item just selected and we're in SaveFile mode we need to update the Edit widget.
			// This widget uses the SaveFileResult string, so we update that.
			if (Mode == DialogMode::SaveFile)
				SaveFileResult = tGetFileBaseName(label).Chr();
		}

		// Do not allow deselection of the single item.
		item->Selected = true;

		// If a directory is selected in the right panel, we support updating the current selected TreeNode.
		if (item->IsDir)
		{
			SelectedNode->NextOpen = true;
			TreeNode* node = SelectedNode->Find(item->Name);
			if (!node)
			{
				// Need to create a new one and connect it up.
				node = new TreeNode(item->Name, this, SelectedNode);
				SelectedNode->AppendChild(node);
				tAssert(node->ChildrenPopulated == false);
			}
			SelectedNode = node;

			// SelectedNode Updated. Need to update global FilePath.
			if (Mode == DialogMode::OpenFile)
				GetDir(ConfigOpenFilePath, SelectedNode);
			else if (Mode == DialogMode::OpenDir)
				GetDir(ConfigOpenDirPath, SelectedNode);
			if (Mode == DialogMode::SaveFile)
				GetDir(ConfigSaveFilePath, SelectedNode);
		}
	}
}


tStringItem* FileDialog::BookmarksLoop()
{
	int flags =
		(Bookmarks.IsEmpty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen) |
		ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	bool isOpen = ImGui::TreeNodeEx("Bookmarks", flags);
	tStringItem* bookmarkItem = nullptr;
	Bookmark* bookmarkToRemove = nullptr;
	if (isOpen)
	{
		int bmNum = 0;
		for (Bookmark* bookmark = Bookmarks.First(); bookmark; bookmark = bookmark->Next(), bmNum++)
		{
			int bmflags =
				ImGuiTreeNodeFlags_Leaf |
				(bookmark->Selected ? ImGuiTreeNodeFlags_Selected : 0) |
				ImGuiTreeNodeFlags_AllowItemOverlap;

			tString name;
			switch (bookmark->BookmarkType)
			{
				case Bookmark::Type::Home:
					tsPrintf(name, "Home##Num%d", bmNum);
					break;

				case Bookmark::Type::Root:
					tsPrintf(name, "Root##Num%d", bmNum);
					break;

				case Bookmark::Type::User:
				default:
					tsPrintf(name, "%s##Num%d", bookmark->Items.Last()->Chr(), bmNum);
					break;
			}

			bool isOpenBM = ImGui::TreeNodeEx(name.Chr(), bmflags);
			bool hovered = ImGui::IsItemHovered();
			bool isActive = ImGui::IsItemActive();
			bool isClicked = ImGui::IsItemClicked();
			if (hovered)
			{
				tString toolText = bookmark->GetPath();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(3.0f, 3.0f));
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(toolText.Chr());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
				ImGui::PopStyleVar();
			}

			// If selected we need a button for removing bookmarks.
			if ((bookmark->BookmarkType == Bookmark::Type::User))
			{
				ImGui::SameLine();

				tString buttonName;
				tsPrintf(buttonName, "  +  ##BookmarkSubNum%d", bmNum);
				if (ImGui::SmallButton(buttonName.Chr()))
					ImGui::OpenPopup("BookmarkContextMenu");

				if (ImGui::BeginPopup("BookmarkContextMenu"))
				{
					if (ImGui::MenuItem("Remove Bookmark"))
						bookmarkToRemove = bookmark;
					ImGui::EndPopup();
				}	
			}
			if (isClicked && isActive)
			{
				UnselectAllBookmarks();
				bookmark->Selected = true;
				bookmarkItem = bookmark->Items.First();
			}
			if (isOpenBM)
				ImGui::TreePop();

			if (bookmarkItem || bookmarkToRemove)
				break;
		}
		ImGui::TreePop();	
	}

	if (bookmarkToRemove)
	{
		Bookmarks.Remove(bookmarkToRemove);
		delete bookmarkToRemove;
		return nullptr;
	}

	return bookmarkItem;
}


bool tFileDialog::HorizontalSplitter(float thickness, float& height1, float& height2, float minHeight1, float minHeight2, float hoverExtend, float hoverDelay)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##HorizontalSplitter");
	ImRect bb;
	bb.Min.x = window->DC.CursorPos.x + 0.0f;
	bb.Min.y = window->DC.CursorPos.y + height1;

	ImVec2 calc = ImGui::CalcItemSize(ImVec2(-1.0f, thickness), 0.0f, 0.0f);
	bb.Max.x = bb.Min.x + calc.x;
	bb.Max.y = bb.Min.y + calc.y;

	return ImGui::SplitterBehavior(bb, id, ImGuiAxis_Y, &height1, &height2, minHeight1, minHeight2, hoverExtend, hoverDelay);
}


bool tFileDialog::VerticalSplitter(float thickness, float& width1, float& width2, float minWidth1, float minWidth2, float hoverExtend, float hoverDelay)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##VerticalSplitter");
	ImRect bb;
	bb.Min.x = window->DC.CursorPos.x + width1;
	bb.Min.y = window->DC.CursorPos.y + 0.0f;

	ImVec2 calc = ImGui::CalcItemSize(ImVec2(thickness, -1.0f), 0.0f, 0.0f);
	bb.Max.x = bb.Min.x + calc.x;
	bb.Max.y = bb.Min.y + calc.y;

	return ImGui::SplitterBehavior(bb, id, ImGuiAxis_X, &width1, &width2, minWidth1, minWidth2, hoverExtend, hoverDelay);
}


FileDialog::DialogState FileDialog::DoPopup()
{
	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	const char* label = nullptr;
	tList<tStringItem>* configPath = nullptr;
	switch (Mode)
	{
		case DialogMode::OpenFile:		label = "Open File";		configPath = &ConfigOpenFilePath;	break;
		case DialogMode::OpenDir:		label = "Open Directory";	configPath = &ConfigOpenDirPath;	break;
		case DialogMode::SaveFile:		label = "Save File";		configPath = &ConfigSaveFilePath;	break;
	}
	ImGui::SetNextWindowSize(tVector2(660.0f, 400.0f), ImGuiCond_Appearing);

	tStringItem* selectPathItemName = nullptr;
	if (PopupJustOpened)
	{
		selectPathItemName = (!configPath || configPath->IsEmpty()) ? nullptr : configPath->Head();
		PopupJustOpened = false;
	}

	if (!ImGui::BeginPopupModal(label, &isOpen, 0))
		return DialogState::Closed;

	DialogState state = DialogState::Open;
	Result.Clear();

	// The left and right panels are cells in a 1 row, 2 column table.
	float bottomBarHeight = 20.0f + 28.0f;
	ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, tVector4(0.50f, 0.50f, 0.54f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_TableBorderLight, tVector4(0.50f, 0.50f, 0.54f, 1.00f));
	int outerTableFlags = ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable;
	if (ImGui::BeginTable("FileDialogTable", 2, outerTableFlags, tVector2(0.0f, -bottomBarHeight)))
	{
		ImGui::TableSetupColumn("LeftTreeColumn", ImGuiTableColumnFlags_WidthFixed, 160.0f);
		ImGui::TableSetupColumn("RightContentColumn", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();

		//
		// Left tree panel (bookmarks and tree-view). This is the workhorse of the dialog.
		//
		ImGui::TableSetColumnIndex(0);
		
		static float heightBookmarks = ImGui::GetTextLineHeightWithSpacing() * 5.0f;
		static float heightTreeView = 0.0f;
		static float panelHeight = 0.0f;
		float currPanelHeight = ImGui::GetWindowHeight() - bottomBarHeight - ImGui::GetFrameHeight() - 20;
		if (panelHeight != currPanelHeight)
		{
			panelHeight = currPanelHeight;
			heightTreeView = panelHeight - heightBookmarks;
		}

		// We use a two line min height for the bookmarks and tree-view in case there
		// is a horizontal scrollbar at the bottom (for long paths).
		float minSplitHeight = 2.0f * ImGui::GetTextLineHeightWithSpacing();
		HorizontalSplitter(1.0f, heightBookmarks, heightTreeView, minSplitHeight, minSplitHeight);

		ImGui::BeginChild("LeftBookmarkPanel", tVector2(0.0f, heightBookmarks), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(0.0f, 3.0f));
		bool setYScrollToSel = false;
		tStringItem* bookmarkItem = BookmarksLoop();
		if (bookmarkItem)
		{
			selectPathItemName = bookmarkItem;
			setYScrollToSel = true;
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::BeginChild("LeftTreeViewPanel", tVector2(0.0f, heightTreeView), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(0.0f, 3.0f));

		TreeNodeRecursive(LocalTreeNode, selectPathItemName, setYScrollToSel);
		#ifdef PLATFORM_WINDOWS
		bool somethingAdded = ProcessShareResults();

		// If something added from our network shares request, we need to try the last selected path again.
		if (somethingAdded)
			selectPathItemName = (!configPath || configPath->IsEmpty()) ? nullptr : configPath->Head();

		ProcessingNetworkPath = true;
		TreeNodeRecursive(NetworkTreeNode, selectPathItemName, setYScrollToSel);
		ProcessingNetworkPath = false;
		#endif

		ImGui::PopStyleVar();
		ImGui::EndChild();

		//
		// Right content panel.
		//
		ImGui::TableSetColumnIndex(1);
		ImGui::BeginChild("RightContentPanel", tVector2(0.0f, -bottomBarHeight));

		if (SelectedNode)
		{
			if (!SelectedNode->ContentsPopulated)
			{
				tString selDir = GetDir(SelectedNode);

				// Directories.
				tList<tStringItem> foundDirs;
				if (!selDir.IsEmpty())
					tSystem::tFindDirs(foundDirs, selDir);
				for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
				{
					(*dir)[dir->Length()-1] = '\0';				// Remove slash.
					ContentItem* contentItem = new ContentItem(tSystem::tGetFileName(*dir));
					SelectedNode->Contents.Append(contentItem);
				}

				// Files.
				if (Mode != DialogMode::OpenDir)
				{
					tList<tFileInfo> foundFiles;
					if (!selDir.IsEmpty())
					{
						tSystem::tFileTypes selectedTypes;
						bool allTypesIfNoneSelected = (Mode == DialogMode::OpenFile);
						selectedTypes.AddSelected(FileTypes, allTypesIfNoneSelected);
						tSystem::tExtensions extensions(selectedTypes);
						tSystem::tFindFilesFast(foundFiles, selDir, extensions);
					}
					for (tFileInfo* fileInfo = foundFiles.First(); fileInfo; fileInfo = fileInfo->Next())
					{
						ContentItem* contentItem = new ContentItem(*fileInfo);
						SelectedNode->Contents.Append(contentItem);
					}
				}

				SelectedNode->ContentsPopulated = true;
			}

			int tableFlags =
				ImGuiTableFlags_Resizable | 
				ImGuiTableFlags_NoSavedSettings |
				ImGuiTableFlags_Reorderable |	// Drag columns to an order you like.
				ImGuiTableFlags_Hideable |		// Hide individual columns.
				ImGuiTableFlags_Sortable |		// Sort by column.
				// ImGuiTableFlags_SortTristate	// Ascending, Descending, None. May get 0 sort specs.
				ImGuiTableFlags_SortMulti |		// Allow sorting multiple columns by shift-selecting them. May get > 1 sort specs.
				//ImGuiTableFlags_SizingFixedFit |
				ImGuiTableFlags_ScrollY;		// This is needed so the table itself has a scroll bar that respects the top-row freeze.

			ImGui::PushStyleColor(ImGuiCol_TableBorderLight, tVector4(0.30f, 0.30f, 0.31f, 1.00f));
			if (ImGui::BeginTable("ContentItems", 5, tableFlags))
			{
				// The columns. Each one gets its own unique ID.
				int iconFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;
				int nameFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder;
				int propFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_PreferSortAscending;
				ImGui::TableSetupColumn("Icon",		iconFlags,	24.0f,	uint32(ContentItem::FieldID::Invalid)	);
				ImGui::TableSetupColumn("Name",		nameFlags,	190.0f,	uint32(ContentItem::FieldID::Name)		);
				ImGui::TableSetupColumn("Modified",	propFlags,	120.0f,	uint32(ContentItem::FieldID::ModTime)	);
				ImGui::TableSetupColumn("Type",		propFlags,	36.0f,	uint32(ContentItem::FieldID::FileType)	);
				ImGui::TableSetupColumn("Size",		propFlags,	0.0f,	uint32(ContentItem::FieldID::FileSize)	);
				ImGui::TableSetupScrollFreeze(0, 1); // Make this row always visible.
				ImGui::TableHeadersRow();

				// Sort the rows.
				ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
				if (sortSpecs && (sortSpecs->SpecsDirty) && (sortSpecs->SpecsCount > 0))
				{
					ContentItem::CompareFunctionObject compare(sortSpecs);
					SelectedNode->Contents.Sort(compare);
					sortSpecs->SpecsDirty = false;
				}

				// Do the content rows. We could use ImGuiListClipper here but so far, even with thousands of files in
				// the Contents list, it is very responsive. Also, since it's a list rather than an array, we'd still
				// need a 'Next' loop to get to the right clipper starting point (clipper.DisplayStart).
				for (ContentItem* item = SelectedNode->Contents.First(); item; item = item->Next())
				{
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					uint64 imgID = item->IsDir ? Viewer::FolderImage.Bind() : Viewer::FileImage.Bind();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX()+4.0f);
					ImGui::Image(ImTextureID(imgID), tVector2(16, 16), tVector2(0, 1), tVector2(1, 0), Viewer::ColourEnabledTint);

					// The name column selectable spans all columns.
					ImGui::TableNextColumn();
					DoSelectable(item);

					ImGui::TableNextColumn();
					if (!item->IsDir)
						ImGui::Text("%s", item->ModTimeString.Chr());

					ImGui::TableNextColumn();
					if (!item->IsDir)
						ImGui::Text("%s", item->FileTypeString.Chr());

					ImGui::TableNextColumn();
					if (!item->IsDir)
						ImGui::Text("%s", item->FileSizeString.Chr());
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
		ImGui::EndTable();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	bool resultAvail = false;
	ContentItem* selItem = SelectedNode ? SelectedNode->FindSelectedItem() : nullptr;
	switch (Mode)
	{
		case DialogMode::OpenFile:
		{
			resultAvail = selItem && !selItem->IsDir;

			// For OpenFile mode display the filename as read-only.
			ImGui::SetNextItemWidth( ImGui::GetWindowContentRegionMax().x / 2.0f );
			if (!resultAvail)
				ImGui::PushStyleColor(ImGuiCol_Text, tVector4(0.5f, 0.5f, 0.52f, 1.0f) );
			tString fname = resultAvail ? selItem->Name : "File Name";
			ImGui::InputText("##File Name", fname.Txt(), fname.Length()+1, ImGuiInputTextFlags_ReadOnly);
			if (!resultAvail)
				ImGui::PopStyleColor();

			DoFileTypesDropdown(true);

			// Open button and set final Result to be picked up.
			ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - 20.0f);
			if (resultAvail)
			{
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 140.0f);
				if (ImGui::Button("Open", tVector2(70.0f, 0.0f)))
				{
					Result = GetDir(SelectedNode) + selItem->Name;
					state = DialogState::OK;
				}
				ImGui::SameLine();
			}
			break;
		}

		case DialogMode::SaveFile:
		{
			resultAvail = selItem && !selItem->IsDir;

			// For SaveFile mode always allow input of the filename manually, but populate it with the
			// selected file (if anything selected). The population happens when the file is selected.
			ImGui::SetNextItemWidth( ImGui::GetWindowContentRegionMax().x / 2.0f );
			ImGui::InputTextWithHint("##File Name", "File Name Sans Extension", &SaveFileResult);

			DoFileTypesDropdown(false);
			tFileType fileType = FileTypes.GetFirstSelectedType();

			// Save button and set final Result to be picked up.
			ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - 20.0f);
			if ((fileType != tFileType::Invalid) && (SaveFileResult.length() > 0))
			{
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 140.0f);
				if (ImGui::Button("Save", tVector2(70.0f, 0.0f)))
				{
					tString extension = tSystem::tGetExtension(fileType);
					Result = GetDir(SelectedNode) + tString(SaveFileResult.c_str()) + "." + extension;
					state = DialogState::OK;
					InvalidateAllNodeContent();
				}
				ImGui::SameLine();
			}
			break;
		}

		case DialogMode::OpenDir:
		{
			resultAvail = selItem && selItem->IsDir;
			
			// OK to select dir in left-hand pane where SelectedNode may be valid.
			int depth = SelectedNode ? SelectedNode->Depth() : 0;
			if (!resultAvail && depth)
				resultAvail = (depth > 1) || (!SelectedNode->IsNetworkLocation() && (depth > 0));

			ImGui::SetNextItemWidth( ImGui::GetWindowContentRegionMax().x / 2.0f );
			if (!resultAvail)
				ImGui::PushStyleColor(ImGuiCol_Text, tVector4(0.5f, 0.5f, 0.52f, 1.0f) );
			tString fname = resultAvail ? SelectedNode->Name : "Directory Name";
			ImGui::InputText("##Directory Name", fname.Txt(), fname.Length()+1, ImGuiInputTextFlags_ReadOnly);
			if (!resultAvail)
				ImGui::PopStyleColor();

			// Open button and set final Result to be picked up.
			ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - 20.0f);
			if (resultAvail)
			{
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 140.0f);
				if (ImGui::Button("Open", tVector2(70.0f, 0.0f)))
				{
					Result = GetDir(SelectedNode);
					state = DialogState::OK;
				}
				ImGui::SameLine();
			}
			break;
		}
	}

	// The cancel button is the same for all modes.
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
	if (ImGui::Button("Cancel", tVector2(70.0f, 0.0f)))
		state = DialogState::Cancel;

	if (state != DialogState::Open)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
	return state;
}


void FileDialog::DoFileTypesDropdown(bool supportMultipleTypes)
{
	if (FileTypes.IsEmpty())
		return;

	ImGui::SameLine();
	ImGui::SetNextItemWidth(140);
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 140.0f);

	// Nothing selected means all types used.
	bool anySelected = FileTypes.AnySelected();
	bool allTypes = supportMultipleTypes && !anySelected;
	tString currChosen;
	if (supportMultipleTypes)
	{
		currChosen = allTypes ? "All Types" : FileTypes.GetSelectedString(tSystem::tFileTypes::Separator::CommaSpace, 4);
	}
	else
	{
		if (!anySelected)
		{
			FileTypes.First()->Selected = true;
			InvalidateAllNodeContent();
		}
		currChosen = FileTypes.GetSelectedString(tSystem::tFileTypes::Separator::CommaSpace, 4);
	}

	if (ImGui::BeginCombo("##TypeFilter", currChosen.Chr()))
	{
		if (supportMultipleTypes && ImGui::Selectable("All Types", allTypes))
		{
			FileTypes.ClearSelected();
			InvalidateAllNodeContent();
		}

		for (tFileTypes::tFileTypeItem* typeItem = FileTypes.First(); typeItem; typeItem = typeItem->Next())
		{
			tFileType fileType = typeItem->FileType;
			const char* fileTypeName = tSystem::tGetFileTypeName(fileType);
			if (!fileTypeName)
				continue;

			if (ImGui::Selectable(fileTypeName, typeItem->Selected))
			{
				if (!supportMultipleTypes)
				{
					FileTypes.ClearSelected();
					typeItem->Selected = true;
				}
				else
					typeItem->Selected = !typeItem->Selected;
				InvalidateAllNodeContent();
			}

			if (typeItem->Selected)
				ImGui::SetItemDefaultFocus();
		}

		// Update the filters.
		ImGui::EndCombo();
	}
}


tString FileDialog::GetResult()
{
	return Result;
}


tString FileDialog::GetDir(const TreeNode* node)
{
	if (!node)
		return tString();

	bool isNetworkLoc = node->IsNetworkLocation();

	tString dir;
	while (node)
	{
		if (!node->Name.IsEmpty() && (node->Depth() > 0))
		{
			if (isNetworkLoc && (node->Depth() == 1))
				dir = node->Name + "\\" + dir;
			else
				dir = node->Name + "/" + dir;
		}
		node = node->Parent;
	}
	if (isNetworkLoc)
		dir = tString("\\\\") + dir;

	return dir;
}


void FileDialog::GetDir(tList<tStringItem>& destDirItems, const TreeNode* node)
{
	destDirItems.Empty();
	if (!node)
		return;

	while (node)
	{
		destDirItems.Insert(new tStringItem(node->Name));
		node = node->Parent;
	}
}
