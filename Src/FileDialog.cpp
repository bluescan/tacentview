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

#include <Math/tVector2.h>
#include "imgui.h"
#include "FileDialog.h"
#include "TacentView.h"
#include "Image.h"
using namespace tStd;
using namespace tMath;
using namespace tSystem;
using namespace tImage;
using namespace tInterface;


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


TreeNode::ContentItem* TreeNode::FindSelectedItem() const
{
	for (ContentItem* item = Contents.First(); item; item = item->Next())
		if (item->Selected)
			return item;

	return nullptr;
}


FileDialog::FileDialog(DialogMode mode) :
	Mode(mode)
{
	FavouritesTreeNode = new TreeNode("Favourites", this);
	LocalTreeNode = new TreeNode("Local", this);
	#ifdef PLATFORM_WINDOWS
	NetworkTreeNode = new TreeNode("Network", this);
	#endif

	PopulateFavourites();
	PopulateLocal();
	#ifdef PLATFORM_WINDOWS
	PopulateNetwork();
	#endif
}


void FileDialog::OpenPopup()
{
	ImGui::SetNextWindowSize(tVector2(600.0f, 400.0f), ImGuiCond_FirstUseEver);
	switch (Mode)
	{
		case DialogMode::OpenDir:
			ImGui::OpenPopup("Open Dir");
			break;

		case DialogMode::OpenFile:
			ImGui::OpenPopup("Open File");
			break;

		case DialogMode::SaveFile:
			ImGui::OpenPopup("Save File");
			break;
	}
}


void FileDialog::PopulateFavourites()
{
	TreeNode* favouriteA = new TreeNode("FavouriteA", this, FavouritesTreeNode);
	FavouritesTreeNode->AppendChild(favouriteA);
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


void FileDialog::NetworkTreeNodeRecursive(TreeNode* node)
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

	int flags = (node->Children.GetNumItems() == 0) ? ImGuiTreeNodeFlags_Leaf : 0;
	if (SelectedNode == node)
		flags |= ImGuiTreeNodeFlags_Selected;
	flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	bool isOpen = ImGui::TreeNodeEx(node->Name, flags);
	bool isClicked = ImGui::IsItemClicked();

	if (isOpen)
	{
		// Recurse children.
		for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
			NetworkTreeNodeRecursive(child.GetObject());

		ImGui::TreePop();
	}

	if (isClicked)
	{
		SelectedNode = node;
		int depth = node->Depth();
		if (!node->ChildrenPopulated && (depth >= 2))
		{
			tString currDir = GetSelectedDir();
			tList<tStringItem> foundDirs;
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
}
#endif


void FileDialog::FavouritesTreeNodeFlat(TreeNode* node)
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
		for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
			FavouritesTreeNodeFlat(child.GetObject());

		ImGui::TreePop();
	}
}


void FileDialog::LocalTreeNodeRecursive(TreeNode* node)
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
		for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
			LocalTreeNodeRecursive(child.GetObject());

		ImGui::TreePop();
	}

	if (isClicked)
	{
		SelectedNode = node;
		if (!node->ChildrenPopulated)
		{
			tString currDir = GetSelectedDir();
			tList<tStringItem> foundDirs;
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
}


FileDialog::DialogResult FileDialog::DoPopup()
{
	// The unused isOpen bool is just so we get a close button in ImGui. 
	bool isOpen = true;
	const char* label = nullptr;
	switch (Mode)
	{
		case DialogMode::OpenDir:		label = "Open Dir";		break;
		case DialogMode::OpenFile:		label = "Open File";	break;
		case DialogMode::SaveFile:		label = "Open File";	break;
	}
	if (!ImGui::BeginPopupModal(label, &isOpen, ImGuiWindowFlags_NoScrollbar /*ImGuiWindowFlags_AlwaysAutoResize*/))
		return DialogResult::Closed;

	DialogResult result = DialogResult::Open;
	Result.Clear();

	// The left and right panels are cells in a 1 row, 2 column table.
	float bottomBarHeight = 20.0f;
	if (ImGui::BeginTable("FileDialogTable", 2, ImGuiTableFlags_Resizable, tVector2(0.0f, -bottomBarHeight)))
	{
		ImGui::TableSetupColumn("LeftTreeColumn", ImGuiTableColumnFlags_WidthFixed, 185.0f);
		ImGui::TableSetupColumn("RightContentColumn", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();

		// Left tree panel.
		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("LeftTreePanel", tVector2(0.0f, -bottomBarHeight));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(0,3));

		FavouritesTreeNodeFlat(FavouritesTreeNode);
		LocalTreeNodeRecursive(LocalTreeNode);
		#ifdef PLATFORM_WINDOWS
		NetworkTreeNodeRecursive(NetworkTreeNode);
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
				tSystem::tFindDirs(foundDirs, selDir);
				for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
				{
					(*dir)[dir->Length()-1] = '\0';				// Remove slash.
					TreeNode::ContentItem* contentItem = new TreeNode::ContentItem(tSystem::tGetFileName(*dir) + "/");
					SelectedNode->Contents.Append(contentItem);
				}

				// Files.
				tList<tStringItem> foundFiles;
				tSystem::tFindFilesFast(foundFiles, selDir);
				for (tStringItem* file = foundFiles.First(); file; file = file->Next())
				{
					TreeNode::ContentItem* contentItem = new TreeNode::ContentItem(tSystem::tGetFileName(*file));
					SelectedNode->Contents.Append(contentItem);
				}

				SelectedNode->ContentsPopulated = true;
			}

			if (ImGui::BeginTable("ContentItems", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
			{
				for (TreeNode::ContentItem* item = SelectedNode->Contents.First(); item; item = item->Next())//, itemNum++)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Selectable(item->Name.Chars(), &item->Selected);// ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");

					ImGui::TableNextColumn();
                    ImGui::Selectable("2022-11-23 2:45am", &item->Selected);
                    ImGui::TableNextColumn();
					ImGui::Selectable("123456 Bytes", &item->Selected);
				}
				ImGui::EndTable();
			}
		}

		ImGui::EndChild();
		ImGui::EndTable();
	}

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		result = DialogResult::Cancel;

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
	{
		TreeNode::ContentItem* selItem = SelectedNode->FindSelectedItem();
		Result = GetSelectedDir();
		if ((Mode == DialogMode::OpenFile) && selItem)
			Result += selItem->Name;
		result = DialogResult::OK;
	}

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
	bool isNetworkLoc = false;
	tString dir;
	TreeNode* curr = SelectedNode;
	while (curr)
	{
		if ((curr->Depth() == 0) && (curr->Name == "Network"))
			isNetworkLoc = true;
		curr = curr->Parent;
	}

	curr = SelectedNode;
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
