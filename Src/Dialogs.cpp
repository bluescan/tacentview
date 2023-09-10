// Dialogs.cpp
//
// Various dialogs and helpers including a log window, info overlay, and about window.
//
// Copyright (c) 2019-2023 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tVersion.cmake.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Dialogs.h"
#include "Image.h"
#include "TacentView.h"
#include "Version.cmake.h"
using namespace tMath;

namespace Viewer
{
	extern OutputLog OutLog;
}


void Viewer::ShowAboutPopup(bool* popen)
{
	Config::ProfileSettings& config = *Config::Current;
	tVector2 windowPos = GetDialogOrigin(DialogID::About);
	tVector2 windowSize;
	switch (config.GetUISize())
	{
		case Viewer::Config::ProfileSettings::UISizeEnum::Nano:		windowSize.Set(240, 500);	break;
		case Viewer::Config::ProfileSettings::UISizeEnum::Tiny:		windowSize.Set(280, 501);	break;
		default:
		case Viewer::Config::ProfileSettings::UISizeEnum::Small:	windowSize.Set(320, 515);	break;
	}

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize;
	if (ImGui::Begin("About", popen, flags))
	{
		int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
		glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
		ImGui::Text("Tacent View V %d.%d.%d by Tristan Grimmer", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);

		tString platform	= tGetPlatformName( tGetPlatform() );
		tString architec	= tGetArchitectureNameLong( tGetArchitecture() );
		tString config 		= tGetConfigurationName( tGetConfiguration() );
		tString package		= "Native";
		#if defined(PACKAGE_SNAP)
			package = "Snap";
		#elif defined(PACKAGE_DEB)
			package = "Deb";
		#elif defined(PACKAGE_ZIP)
			package = "Zip";
		#endif

		ImGui::Text("Platform: %s", platform.Chr());
		ImGui::Text("Architecture: %s", architec.Chr());
		ImGui::Text("Configuration: %s", config.Chr());
		ImGui::Text("Package: %s", package.Chr());

		#ifdef TACENT_UTF16_API_CALLS
		ImGui::Text("API Mode: UTF-16");
		#else
		ImGui::Text("API Mode: UTF-8");
		#endif

		ImGui::Separator();
		ImGui::Text("Tacent Library V %d.%d.%d", tVersion::Major, tVersion::Minor, tVersion::Revision);
		ImGui::Text("Dear ImGui V %s", IMGUI_VERSION);

		// This way of getting the version is 'dynamic'. It will, for example, print mesa compatibility mode if it's being used.
		ImGui::Text("GLAD V %s", glad_glGetString(GL_VERSION));
		// This is the more 'static' way. Currently unused.
		// ImGui::Text("GLAD V %d.%d", GLVersion.major, GLVersion.minor);

		ImGui::Text("GLFW V %d.%d.%d", glfwMajor, glfwMinor, glfwRev);
		ImGui::Text("Ico Load");
		ImGui::Text("Ion Icons");
		ImGui::Text("Roboto Google Font");
		ImGui::Text("Radiance Software");
		ImGui::Text("LibJPEG-Turbo V %s", tImage::Version_LibJpegTurbo);
		ImGui::Text("LibTIFF V %s", tImage::Version_LibTIFF);
		ImGui::Text("OpenEXR V %s", tImage::Version_OpenEXR);
		ImGui::Text("ZLib V %s", tImage::Version_ZLIB);
		ImGui::Text("GIF Load");
		ImGui::Text("GIF Save");
		ImGui::Text("LibWEBP V %d.%d", tImage::Version_WEBP_Major, tImage::Version_WEBP_Minor);
		ImGui::Text("LibPNG V %s", tImage::Version_LibPNG);
		ImGui::Text("LibKTX V %s", tImage::Version_LibKTX);
		ImGui::Text("APNGDis V %s", tImage::Version_ApngDis);
		ImGui::Text("APNGAsm V %s", tImage::Version_ApngAsm);
		ImGui::Text("TinyXML2 V %d.%d.%d", tImage::Version_TinyXML2_Major, tImage::Version_TinyXML2_Minor, tImage::Version_TinyXML2_Patch);
		ImGui::Text("TinyEXIF V %d.%d.%d", tImage::Version_TinyEXIF_Major, tImage::Version_TinyEXIF_Minor, tImage::Version_TinyEXIF_Patch);
		ImGui::Text("QOI");
		ImGui::Text("BCDec V %d.%d", tImage::Version_BCDec_Major, tImage::Version_BCDec_Minor);
		ImGui::Text("ETCDec V %d.%d", tImage::Version_ETCDec_Major, tImage::Version_ETCDec_Minor);
	}
	ImGui::End();
}


void Viewer::DoDeleteFileModal(bool deleteFilePressed)
{
	Config::ProfileSettings& config = *Config::Current;
	if (deleteFilePressed)
	{
		if (!config.ConfirmDeletes)
			DeleteImageFile(CurrImage->Filename, true);
		else
			ImGui::OpenPopup("Delete File");
	}

	// The unused isOpenDeleteFile bool is just so we get a close button in ImGui.
	bool isOpenDeleteFile = true;
	if (!ImGui::BeginPopupModal("Delete File", &isOpenDeleteFile, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.Chr()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chr()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();
	ImGui::Checkbox("Confirm file deletions in the future?", &config.ConfirmDeletes);
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
	{
		DeleteImageFile(fullname, true);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoDeleteFileNoRecycleModal(bool deleteFileNoRecycPressed)
{
	if (deleteFileNoRecycPressed)
		ImGui::OpenPopup("Delete File Permanently");

	// The unused isOpenPerm bool is just so we get a close button in ImGui.
	bool isOpenPerm = true;
	if (!ImGui::BeginPopupModal("Delete File Permanently", &isOpenPerm, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.Chr()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.Chr()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("This operation cannot be undone. The file\nwill be deleted permanently.");
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)))
	{
		DeleteImageFile(fullname, false);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoSnapMessageNoFileBrowseModal(bool justPressed)
{
	if (justPressed)
		ImGui::OpenPopup("Message_NoFileBrowse");

	// The unused isMessage bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isMessage = true;
	if
	(
		!ImGui::BeginPopupModal
		(
			"Message_NoFileBrowse", &isMessage,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		)
	)
	{
		return;
	}
	
	ImGui::Text
	(
		"The Snap version of Tacent View does not\n"
		"support opening Nautilus or Dolphin.\n\n"
		"Please use the deb install or compile if\n"
		"you need the feature on Linux."
	);
	ImGui::NewLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::DoSnapMessageNoFrameTransModal(bool justPressed)
{
	if (justPressed)
		ImGui::OpenPopup("Message_NoFrameTrans");

	// The unused isMessage bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isMessage = true;
	if
	(
		!ImGui::BeginPopupModal
		(
			"Message_NoFrameTrans", &isMessage,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		)
	)
	{
		return;
	}

	ImGui::Text
	(
		"The Snap version of Tacent View does not\n"
		"support transparent work area.\n\n"
		"Please use the deb install or compile if\n"
		"you need the feature on Linux."
	);
	ImGui::NewLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::DoRenameModal(bool renamePressed)
{
	if (renamePressed)
		ImGui::OpenPopup("Rename File");

	// The unused isOpenRen bool is just so we get a close button in ImGui.
	bool isOpenRen = true;
	if (!ImGui::BeginPopupModal("Rename File", &isOpenRen, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	tString fullname = CurrImage->Filename;
	tString origname = tSystem::tGetFileName(fullname);

	static char newname[128] = "Filename";
	if (renamePressed)
		tStd::tStrcpy(newname, origname.Chr());

	bool nameChanged = false;
	if (ImGui::InputText("##NewNameText", newname, tNumElements(newname), ImGuiInputTextFlags_EnterReturnsTrue))
		nameChanged = true;
	ImGui::NewLine();

	if (Viewer::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Viewer::Button("OK", tVector2(100.0f, 0.0f)) || nameChanged)
	{
		if (origname != newname)
		{
			tString dir = tSystem::tGetDir(fullname);
			bool renamed = tSystem::tRenameFile(dir, origname, newname);
			if (renamed)
			{
				PopulateImages();
				SetCurrentImage(dir+newname);
			}
		}

		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


// Parts of this class are a modification of the one that ships with Dear ImGui. The DearImGui
// licence (MIT) may be found in the txt file Licence_DearImGui_MIT.txt in the Data folder.
void Viewer::OutputLog::ClearLog()
{
	LogBuf.clear();
	LogLineOffsets.clear();
	LogLineOffsets.push_back(0);
}


void Viewer::OutputLog::AddLog(const char* fmt, ...)
{
	int oldSize = LogBuf.size();
	va_list args;
	va_start(args, fmt);
	LogBuf.appendfv(fmt, args);
	va_end(args);

	for (int newSize = LogBuf.size(); oldSize < newSize; oldSize++)
		if (LogBuf[oldSize] == '\n')
			LogLineOffsets.push_back(oldSize + 1);

	LogScrollToBottom = true;
}


void Viewer::OutputLog::DrawLog()
{
	if (ImGui::Button("Clear"))
		ClearLog();
	float clrWidth = ImGui::GetItemRectSize().x;

	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	float cpyWidth = ImGui::GetItemRectSize().x;

	ImGui::SameLine();

	float fltWidth = ImGui::GetWindowWidth() - clrWidth - cpyWidth;
	LogFilter.Draw("Filter", fltWidth-120.0f);
	ImGui::SameLine();
	Viewer::ShowHelpMark("Place a negative sign in front to exclude (-excluded).");

	ImGui::Separator();
	ImGui::BeginChild("scrolling", tVector2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (copy)
		ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(0.0f, 0.0f));
	const char* buf = LogBuf.begin();
	const char* bufEnd = LogBuf.end();
	if (LogFilter.IsActive())
	{
		for (int lineNo = 0; lineNo < LogLineOffsets.Size; lineNo++)
		{
			const char* lineStart = buf + LogLineOffsets[lineNo];
			const char* lineEnd = (lineNo + 1 < LogLineOffsets.Size) ? (buf + LogLineOffsets[lineNo + 1] - 1) : bufEnd;
			if (LogFilter.PassFilter(lineStart, lineEnd))
				ImGui::TextUnformatted(lineStart, lineEnd);
		}
	}
	else
	{
		// The simplest way to display the entire buffer is with ImGui::TextUnformatted(buf, buf_end); TextUnformatted
		// has specialization for large blobs of text and will fast-forward to skip non-visible lines. Here we instead
		// demonstrate using the clipper to only process lines that are within the visible area. If you have tens of
		// thousands of items and their processing cost is non-negligible, coarse clipping them on your side is
		// recommended.
		//
		// ImGuiListClipper requires a) random access into your data, and b) items all being the same height, both of
		// which we can handle since we an array pointing to the beginning of each line of text. When using the filter
		// (in the block of code above) we don't have random access into the data to display anymore, which is why we
		// don't use the clipper. Storing or skimming through the search result would make it possible and would be
		// recommended if you want to search through tens of thousands of entries.
		ImGuiListClipper clipper;
		clipper.Begin(LogLineOffsets.Size);
		while (clipper.Step())
		{
			for (int lineNo = clipper.DisplayStart; lineNo < clipper.DisplayEnd; lineNo++)
			{
				const char* lineStart = buf + LogLineOffsets[lineNo];
				const char* lineEnd = (lineNo + 1 < LogLineOffsets.Size) ? (buf + LogLineOffsets[lineNo + 1] - 1) : bufEnd;
				ImGui::TextUnformatted(lineStart, lineEnd);
			}
		}
		clipper.End();
	}
	ImGui::PopStyleVar();

	if (LogScrollToBottom)
		ImGui::SetScrollHereY(1.0f);

	LogScrollToBottom = false;
	ImGui::EndChild();
}


void Viewer::ShowOutputLogPopup(bool* popen)
{
	Config::ProfileSettings& config = *Config::Current;

	tVector2 windowPos = GetDialogOrigin(DialogID::LogOutput);
	tVector2 windowSize = tMath::tLinearLookup
	(
		config.UISizeFlt(), config.UISizeSmallestFlt(), config.UISizeLargestFlt(),
		#ifdef ALLOW_ALL_UI_SIZES
		tVector2(410.0f, 220.0f), tVector2(800.0f, 400.0f)
		#else
		tVector2(410.0f, 220.0f), tVector2(520.0f, 270.0f)
		#endif
	);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(tVector2(330.0f, 190.0f), tVector2(4096.0f, 4096.0f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing;
	if (ImGui::Begin("Output Log", popen, flags))
	{
		Viewer::OutLog.DrawLog();
	}
	ImGui::End();
}


void Viewer::NavLogBar::Draw()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);

	if
	(
		ImGui::ImageButton(ImTextureID(Image_UpFolder.Bind()), tVector2(20.0f, 20.0f), tVector2(0.0f, 1.0f), tVector2(1.0f, 0.0f), 1,
		Viewer::ColourBG, tVector4(1.00f, 1.00f, 1.00f, 1.00f))
	)
	{
		tString upDir = tSystem::tGetUpDir(ImagesDir);
		if (!upDir.IsEmpty())
		{
			CurrImageFile = upDir + "dummyfile.txt";
			PopulateImages();
			SetCurrentImage();
			SetWindowTitle();
		}
	}
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("%s", ImagesDir.Chr());

	if (ImagesSubDirs.NumItems() > 0)
	{
		Config::ProfileSettings& config = *Config::Current;
		ImGui::SameLine();
		float offset;
		switch (config.GetUISize())
		{
			case Config::ProfileSettings::UISizeEnum::Nano:		offset = 1.0f;	break;
			case Config::ProfileSettings::UISizeEnum::Tiny:		offset = 0.0f;	break;
			default:
			case Config::ProfileSettings::UISizeEnum::Small:	offset = -1.0f;	break;
		}
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
		if (ImGui::BeginCombo("##combo", nullptr, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoPreview))
		{
			for (tStringItem* subDir = ImagesSubDirs.First(); subDir; subDir = subDir->Next())
			{
				bool isSelected = false;
				if (ImGui::Selectable(subDir->Chr(), isSelected))
				{
					// Selection made. This only runs once.
					CurrImageFile = ImagesDir + *subDir + "/" + "dummyfile.txt";
					PopulateImages();
					SetCurrentImage();
					SetWindowTitle();
					break;
				}
			}
			ImGui::EndCombo();
		}
	}
}
