// FileDialog.cpp
//
// Dialog that allows selection of a file or directory. May be used for opening a file/directory or saving to a file.
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


FileDialog::FileDialog(DialogMode mode) :
	Mode(mode)
{
	FavouritesTreeNode = new TreeNode("Favourites", this);
	LocalTreeNode = new TreeNode("Local", this);
	NetworkTreeNode = new TreeNode("Network", this);
	PopulateFavourites();
	PopulateLocal();
	PopulateNetwork();
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


void FileDialog::PopulateNetwork()
{
	TreeNode* networkA = new TreeNode("NetworkA", this, NetworkTreeNode);
	NetworkTreeNode->AppendChild(networkA);
}


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
			//tPrintf("SelectedDir: %s\n", currDir.Chars());
			tList<tStringItem> foundDirs;
			tSystem::tFindDirs(foundDirs, currDir);
			for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
			{
				tString relDir = tSystem::tGetRelativePath(currDir, *dir);
				relDir.ExtractLeft("./");
				relDir.ExtractRight("/");

				tPrintf("ChildDir: %s\n", relDir.Chars());
				TreeNode* child = new TreeNode(relDir, this, node);
				node->AppendChild(child);

			}
			node->ChildrenPopulated = true;
		}
	}
}


void FileDialog::NetworkTreeNodeRecursive(TreeNode* node)
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
			NetworkTreeNodeRecursive(child.GetObject());

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
		ImGui::TableSetupColumn("LeftTreeColumn", ImGuiTableColumnFlags_WidthFixed, 125.0f);
		ImGui::TableSetupColumn("RightContentColumn", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();

		// Left tree panel.
		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("LeftTreePanel", tVector2(0.0f, -bottomBarHeight));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tVector2(0,3));

		FavouritesTreeNodeFlat(FavouritesTreeNode);
		LocalTreeNodeRecursive(LocalTreeNode);
		NetworkTreeNodeRecursive(NetworkTreeNode);

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
				tPrintf("Selected Dir: %s\n", selDir.Chars());

				// if (Mode == DialogMode::OpenDir)
				{
					tList<tStringItem> foundDirs;
					tSystem::tFindDirs(foundDirs, selDir);
					for (tStringItem* dir = foundDirs.First(); dir; dir = dir->Next())
					{
						(*dir)[dir->Length()-1] = '\0';				// Remove slash.
						tStringItem* dirName = new tStringItem(tSystem::tGetFileName(*dir) + "/");
						SelectedNode->Contents.Append(dirName);
					}
				}
				// else if (Mode == DialogMode::OpenFile)
				{
					tList<tStringItem> foundFiles;
					tSystem::tFindFiles(foundFiles, selDir);
					for (tStringItem* file = foundFiles.First(); file; file = file->Next())
					{
						//(*dir)[dir->Length()-1] = '\0';				// Remove slash.
						tStringItem* fileName = new tStringItem(tSystem::tGetFileName(*file));
						SelectedNode->Contents.Append(fileName);
					}
				}

				SelectedNode->ContentsPopulated = true;
			}

            if (ImGui::BeginTable("split2", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
			{
				int itemNum = 0;
				for (tStringItem* item = SelectedNode->Contents.First(); item; item = item->Next(), itemNum++)
				{
					ImGui::TableNextRow();
					bool selected = (itemNum == 4) ? true : false;
					ImGui::TableNextColumn();
					ImGui::Selectable(item->Chars(), &selected);// ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");

					ImGui::TableNextColumn();
                    ImGui::Selectable("2022-11-23 2:45am", &selected);
                    // ImGui::Text("2022-11-23 2:45am");
                    ImGui::TableNextColumn();
					ImGui::Selectable("123456 Bytes", &selected);
    				//	ImGui::Text("123456 Bytes");
					//	if (ImGui::Selectable(item->Chars(), false))
					//	{
					//		Select it.
					//	}
				}
				ImGui::EndTable();
			}
		}

		ImGui::EndChild();
		ImGui::EndTable();
	}

	if (ImGui::Button("OK"))
	{
		if (Mode == DialogMode::OpenFile)
			Result = "C:/testfile.txt";
		else if (Mode == DialogMode::OpenDir)
			Result = "C:/";
		result = DialogResult::OK;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
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
