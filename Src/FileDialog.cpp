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
#include "FileDialog.h"
#include "TacentView.h"
#include "Image.h"
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;
using namespace tInterface;


namespace tInterface
{
	// An internal list of all currently instanced file dialogs.
	tList<FileDialog> FileDialogs(tListMode::StaticZero);

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
	tsPrintf(ModTimeString, "%s", tSystem::tConvertTimeToString(tSystem::tConvertTimeToLocal(ModTime)).Chars());

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
	Mode(mode),
	FileTypes(fileTypes)
{
	#ifdef TACENTVIEW_BOOKMARKS
	BookmarkTreeNode = new TreeNode("Bookmarks", this);
	#endif
	
	LocalTreeNode = new TreeNode("Local", this);
	#ifdef PLATFORM_WINDOWS
	NetworkTreeNode = new TreeNode("Network", this);
	#endif

	#ifdef TACENTVIEW_BOOKMARKS
	// @wip Bookmarks is disabled until I implement it fully.
	PopulateBookmarks();
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


bool FileDialog::Save(tExprWriter& writer)
{
	return true;
}


bool FileDialog::Load(tExpr expr)
{
	return true;
}


void FileDialog::OpenPopup()
{
	//ImGui::SetNextWindowSize(tVector2(640.0f, 400.0f), ImGuiCond_FirstUseEver);
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
}


#ifdef TACENTVIEW_BOOKMARKS
void FileDialog::PopulateBookmarks()
{
	#if defined(PLATFORM_LINUX)
	// All linux platforms get a 'home' bookmark.
	#endif

	#if defined(PLATFORM_WINDOWS)
	// All windows platforms get a 'user' bookmark.
	TreeNode* user = new TreeNode("User", this, BookmarkTreeNode);
	BookmarkTreeNode->AppendChild(user);
	#endif
}
#endif


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


void FileDialog::ProcessShareResults()
{
	// Most of time this will be empty since we remove the items when they are available, just like a message queue.
	tStringItem* share = NetworkShareResults.ShareNames.Remove();
	if (share)
	{
		tList<tStringItem> exploded;
		tExplodeShareName(exploded, *share);
		tString machName = *exploded.First();
		if (!NetworkTreeNode->Contains(machName))
		{
			TreeNode* machine = new TreeNode(machName.Text(), this, NetworkTreeNode);
			NetworkTreeNode->AppendChild(machine);
		}

		TreeNode* machNode = NetworkTreeNode->Find(machName);
		tAssert(machNode);
		tStringItem* shareName = (exploded.GetNumItems() == 2) ? exploded.Last() : nullptr;

		if (shareName && !machNode->Contains(*shareName))
		{
			TreeNode* shareNode = new TreeNode(shareName->Text(), this, machNode);
			machNode->AppendChild(shareNode);
		}
		delete share;
	}
}
#endif


void FileDialog::TreeNodeRecursive(TreeNode* node)
{
	int flags =
		ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen |
		((node->Children.GetNumItems() == 0) ? ImGuiTreeNodeFlags_Leaf : 0) |
		((SelectedNode == node) ? ImGuiTreeNodeFlags_Selected : 0);

	bool populate = false;

	// This is the magic that's needed if you select from the right-hand panel.
	if (node->NextOpen)
	{
		ImGui::SetNextTreeNodeOpen(true);
		node->NextOpen = false;
		populate = true;
	}

	bool isOpen = ImGui::TreeNodeEx(node->Name, flags);
	bool isClicked = ImGui::IsItemClicked();
	if (isClicked)
	{
		SelectedNode = node;
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
			tString currDir = GetSelectedDir();
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
			TreeNodeRecursive(child.GetObject());

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
	if (ImGui::Selectable(label, &item->Selected, ImGuiSelectableFlags_SpanAllColumns))
	{
		// This block enforces single selection.
		if (Mode != DialogMode::OpenFiles)
		{
			if (item->Selected)
			{
				// Only allowed one selected max. Clear the others.
				for (ContentItem* i = SelectedNode->Contents.First(); i; i = i->Next())
				{
					if (i == item)
						continue;
					i->Selected = false;
				}
			}

			// Do not allow deselection of the single item.
			item->Selected = true;
		}

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
		}
	}
}


void FileDialog::TreeNodeFlat(TreeNode* node)
{
	int flags = (node->Children.GetNumItems() == 0) ? ImGuiTreeNodeFlags_Leaf : 0;
	if (SelectedNode == node)
		flags |= ImGuiTreeNodeFlags_Selected;
	flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	bool isOpen = ImGui::TreeNodeEx(node->Name, flags);
	bool isClicked = ImGui::IsItemClicked();

	if (isOpen)
	{
		// Recurse children.
		// for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
		//	FavouritesTreeNodeFlat(child.GetObject());
		ImGui::TreePop();
	}
}


FileDialog::DialogResult FileDialog::DoPopup()
{
	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	const char* label = nullptr;
	switch (Mode)
	{
		case DialogMode::OpenFile:		label = "Open File";		break;
		case DialogMode::OpenFiles:		label = "Open Files";		break;
		case DialogMode::OpenDir:		label = "Open Directory";	break;
		case DialogMode::SaveFile:		label = "Save File";		break;
	}
	ImGui::SetNextWindowSize(tVector2(660.0f, 400.0f), ImGuiCond_Appearing);
	if (!ImGui::BeginPopupModal(label, &isOpen, 0))
		return DialogResult::Closed;

	DialogResult result = DialogResult::Open;
	Result.Clear();

	// The left and right panels are cells in a 1 row, 2 column table.
	float bottomBarHeight = 20.0f + 28.0f;
	ImGui::PushStyleColor(ImGuiCol_TableBorderLight, tVector4(0.50f, 0.50f, 0.54f, 1.00f));
	if (ImGui::BeginTable("FileDialogTable", 2, ImGuiTableFlags_Resizable, tVector2(0.0f, -bottomBarHeight)))
	{
		ImGui::TableSetupColumn("LeftTreeColumn", ImGuiTableColumnFlags_WidthFixed, 160.0f);
		ImGui::TableSetupColumn("RightContentColumn", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();

		// Left tree panel.
		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("LeftTreePanel", tVector2(0.0f, -bottomBarHeight), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(0.0f, 3.0f));

		// This block is the workhorse of the dialog.

		#ifdef TACENTVIEW_BOOKMARKS
		// @wip Bookmarks are disabled until implemented fully.
		TreeNodeFlat(BookmarkTreeNode);
		#endif
		TreeNodeRecursive(LocalTreeNode);
		#ifdef PLATFORM_WINDOWS
		ProcessShareResults();
		ProcessingNetworkPath = true;
		TreeNodeRecursive(NetworkTreeNode);
		ProcessingNetworkPath = false;
		#endif

		ImGui::PopStyleVar();
		ImGui::EndChild();
		
		// Right content panel.
		ImGui::TableSetColumnIndex(1);
		ImGui::BeginChild("RightContentPanel", tVector2(0.0f, -bottomBarHeight));

		if (SelectedNode)
		{
			if (!SelectedNode->ContentsPopulated)
			{
				tString selDir = GetSelectedDir();

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
						selectedTypes.AddSelected(FileTypes, true);
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

			ImGui::PushStyleColor(ImGuiCol_TableBorderLight, tVector4(0.25f, 0.25f, 0.28f, 1.00f));
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
						ImGui::Text("%s", item->ModTimeString.Text());

					ImGui::TableNextColumn();
					if (!item->IsDir)
						ImGui::Text("%s", item->FileTypeString.Text());

					ImGui::TableNextColumn();
					if (!item->IsDir)
						ImGui::Text("%s", item->FileSizeString.Text());
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
		ImGui::EndTable();
	}
	ImGui::PopStyleColor();

	ContentItem* selItem = SelectedNode ? SelectedNode->FindSelectedItem() : nullptr;
	bool resultAvail = false;
	if (selItem)
	{
		resultAvail =
		(
			((Mode == DialogMode::OpenFile) && !selItem->IsDir) ||
			((Mode == DialogMode::SaveFile) && !selItem->IsDir) ||
			((Mode == DialogMode::OpenDir) && selItem->IsDir)
			// @todo multiple files.
		);
	}

	if (!resultAvail && (Mode == DialogMode::OpenDir) && SelectedNode)
	{
		// OK to select dir in left-hand pane.
		int depth = SelectedNode->Depth();
		resultAvail =
		(
			(depth > 1) ||
			(!SelectedNode->IsNetworkLocation() && (depth > 0))
		);
	}

	// For file modes, display the filename and types combo.
	if (Mode != DialogMode::OpenDir)
	{
		int flags = ImGuiInputTextFlags_ReadOnly;
		ImGui::SetNextItemWidth( ImGui::GetWindowContentRegionMax().x / 2.0f );

		if (!resultAvail)
			ImGui::PushStyleColor(ImGuiCol_Text, tVector4(0.5f, 0.5f, 0.52f, 1.0f) );
		tString fname = resultAvail ? selItem->Name : "File Name";
		ImGui::InputText("##File Name", fname.Text(), fname.Length()+1, flags);
		if (!resultAvail)
			ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::SetNextItemWidth(140);
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 140.0f);

		// Nothing selected means all types used.
		bool allTypes = !FileTypes.AnySelected();
		tString currChosen = allTypes ? "All Image Types" : FileTypes.GetSelectedString(tSystem::tFileTypes::Separator::CommaSpace, 4);
		if (ImGui::BeginCombo("##TypeFilter", currChosen.Chars())) //, ImGuiComboFlags_NoArrowButton))
		{
			if (ImGui::Selectable("All Image Types", allTypes))
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

	// Cancel and OK buttons.
	ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - 20.0f);
	if (resultAvail)
	{
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 140.0f);
		if (ImGui::Button("Open", tVector2(70.0f, 0.0f)))
		{
			Result = GetSelectedDir();
			if ((Mode == DialogMode::OpenFile) && selItem)
				Result += selItem->Name;
			result = DialogResult::OK;
		}
		ImGui::SameLine();
	}

	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
	if (ImGui::Button("Cancel", tVector2(70.0f, 0.0f)))
		result = DialogResult::Cancel;

	if (result != DialogResult::Open)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
	return result;
}


tString FileDialog::GetResult()
{
	return Result;
}


tString FileDialog::GetSelectedDir()
{
	if (!SelectedNode)
		return tString();

	bool isNetworkLoc = SelectedNode->IsNetworkLocation();

	tString dir;
	TreeNode* curr = SelectedNode;
	while (curr)
	{
		if (!curr->Name.IsEmpty() && (curr->Depth() > 0))
		{
			if (isNetworkLoc && (curr->Depth() == 1))
				dir = curr->Name + "\\" + dir;
			else
				dir = curr->Name + "/" + dir;
		}
		curr = curr->Parent;
	}
	if (isNetworkLoc)
		dir = tString("\\\\") + dir;

	return dir;
}
