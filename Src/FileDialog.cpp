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
	Mode(mode),
	RootTreeNode(nullptr)
{
	//RootTreeNode = new TreeNode("Everything");
	RootTreeNode = new TreeNode();
	PopulateFavourites();
	PopulateLocal();
	PopulateNetwork();
}


void FileDialog::OpenPopup()
{
	ImGui::SetNextWindowSize(tVector2(600.0f, 400.0f), ImGuiCond_FirstUseEver);
	ImGui::OpenPopup("Open File");
}


void FileDialog::PopulateFavourites()
{
	TreeNode* favourites = new TreeNode("Favourites", this, RootTreeNode);
	RootTreeNode->AppendChild(favourites);
}


void FileDialog::PopulateLocal()
{
	TreeNode* local = new TreeNode("Local", this, RootTreeNode);
	RootTreeNode->AppendChild(local);

	#if defined(PLATFORM_WINDOWS)
	tList<tStringItem> drives;
	tSystem::tGetDrives(drives);

	for (tStringItem* drive = drives.First(); drive; drive = drive->Next())
	{
		TreeNode* driveNode = new TreeNode(*drive, this, local);
		local->AppendChild(driveNode);	
	}
	#endif
}


void FileDialog::PopulateNetwork()
{
	TreeNode* network = new TreeNode("Network", this, RootTreeNode);
	RootTreeNode->AppendChild(network);
}


void FileDialog::RecursiveTreeNode(TreeNode* node)
{
	if (node->Name.IsEmpty())
	{
		for (tItList<TreeNode>::Iter child = node->Children.First(); child; child++)
			RecursiveTreeNode(child.GetObject());
		
		return;
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
			RecursiveTreeNode(child.GetObject());

		ImGui::TreePop();
	}

	if (isClicked)
	{
		SelectedNode = node;
		if (!node->ChildrenPopulated)
		{
			tString currDir = GetSelectedDir();
			tPrintf("SelectedDir: %s\n", currDir.Chars());
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


FileDialog::DialogResult FileDialog::DoPopup()
{
	// The unused isOpenFile bool is just so we get a close button in ImGui. 
	bool isOpenFile = true;
	if (!ImGui::BeginPopupModal("Open File", &isOpenFile, ImGuiWindowFlags_NoScrollbar /*ImGuiWindowFlags_AlwaysAutoResize*/))
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

		RecursiveTreeNode(RootTreeNode);

		ImGui::PopStyleVar();

/*
		if (ImGui::TreeNode("Favourites"))
		{
			if (ImGui::TreeNodeEx("FavouriteA", ImGuiTreeNodeFlags_Leaf))
				ImGui::TreePop();
			if (ImGui::TreeNodeEx("FavouriteB", 0))
				ImGui::TreePop();
			if (ImGui::TreeNodeEx("FavouriteC", ImGuiTreeNodeFlags_Leaf))
				ImGui::TreePop();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Local"))
			ImGui::TreePop();
		if (ImGui::TreeNode("Network"))
			ImGui::TreePop();
			*/

		ImGui::EndChild();
		
		// Right content panel.
		ImGui::TableSetColumnIndex(1);
		ImGui::BeginChild("RightContentPanel", tVector2(0.0f, -bottomBarHeight));
		ImGui::Text("RightPanelFileA");
		ImGui::Separator();
		ImGui::Text("RightPanelFileB");
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
	ImGui::Text("OPEN FILE");

	if (result != DialogResult::Open)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
	return result;
}


tString FileDialog::GetResult()
{
	return Result;
}
