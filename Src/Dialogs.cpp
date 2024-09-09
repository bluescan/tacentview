// Dialogs.cpp
//
// Various dialogs and helpers including a log window, info overlay, and about window.
//
// Copyright (c) 2019-2024 Tristan Grimmer.
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
#include <clip.h>
#include "imgui.h"
#include "Dialogs.h"
#include "Image.h"
#include "TacentView.h"
#include "GuiUtil.h"
#include "Version.cmake.h"
using namespace tMath;
namespace Viewer { extern OutputLog OutLog; }


void Viewer::ShowAboutPopup(bool* popen)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::About);
	tVector2 windowSize = Gutil::GetUIParamScaled(tVector2(280.0f, 400.0f), 2.5f);

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
		tString config		= tGetConfigurationName( tGetConfiguration() );
		tString package		= "Native";
		#if defined(PACKAGE_SNAP)
			package = "Snap";
		#elif defined(PACKAGE_DEB)
			package = "Deb";
		#elif defined(PACKAGE_NIX)
			package = "Nix";
		#elif defined(PACKAGE_DEV)
			package = "Dev";
		#elif defined(PACKAGE_PORTABLE)
			#if defined(PLATFORM_WINDOWS)
				package = "Portable ZIP";
			#elif defined(PLATFORM_LINUX)
				package = "Portable TGZ";
			#endif
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

		// This is the more 'static' way of getting the GLAD version. The full 'dynamic' version string can be quite
		// long (especially in compatibility mode) and is presented with a help-mark (if user hovers over it).
		ImGui::Text("GLAD V %d.%d", GLVersion.major, GLVersion.minor);
		ImGui::SameLine();
		Gutil::HelpMark((const char*)glad_glGetString(GL_VERSION));

		ImGui::Text("GLFW V %d.%d.%d", glfwMajor, glfwMinor, glfwRev);
		ImGui::Text("Ico Load");
		ImGui::Text("Ion Icons");
		ImGui::Text("Roboto Google Font");
		ImGui::Text("Radiance Software");
		ImGui::Text("LibJPEG-Turbo V %s", tImage::Version_LibJpegTurbo);
		ImGui::Text("LibTIFF V %s", tImage::Version_LibTIFF);
		ImGui::Text("OpenEXR V %s", tImage::Version_OpenEXR);
		ImGui::Text("ZLib V %s", tImage::Version_ZLIB);
		ImGui::Text("Clip Library V %s", CLIP_LIBRARY_VERSION);
		ImGui::Text("GIF Load");
		ImGui::Text("GIF Save");
		ImGui::Text("LibWEBP V %d.%d", tImage::Version_WEBP_Major, tImage::Version_WEBP_Minor);
		ImGui::Text("LibSPNG V %d.%d.%d", tImage::Version_LibSPNG_Major, tImage::Version_LibSPNG_Minor, tImage::Version_LibSPNG_Patch);
		ImGui::Text("LibPNG V %s", tImage::Version_LibPNG);
		ImGui::Text("APNGDis V %s", tImage::Version_ApngDis);
		ImGui::Text("APNGAsm V %s", tImage::Version_ApngAsm);
		ImGui::Text("LibKTX %s", tImage::Version_LibKTX);
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
	Config::ProfileData& profile = Config::GetProfileData();
	if (deleteFilePressed)
	{
		if (!profile.ConfirmDeletes)
			DeleteImageFile(CurrImage->Filename, true);
		else
			ImGui::OpenPopup("Delete File");
	}

	// The unused isOpenDeleteFile bool is just so we get a close button in ImGui.
	bool isOpenDeleteFile = true;
	if (!ImGui::BeginPopupModal("Delete File", &isOpenDeleteFile, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);
	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);

	// The overall width is based on the larger of the filename and directory. We clamp
	// the width of these two by calling CropStringToWidth.
	float maxPathWidth = Gutil::GetUIParamScaled(380.0f, 2.5f);
	float minPathWidth = Gutil::GetUIParamScaled(296.0f, 2.5f);
	float fileWidth = 0.0f;
	tString croppedFile = Gutil::CropStringToWidth(file, maxPathWidth, true, &fileWidth);
	float dirWidth = 0.0f;
	tString croppedDir  = Gutil::CropStringToWidth(dir,  maxPathWidth, true, &dirWidth);
	float pathWidth = tMax(fileWidth, dirWidth, minPathWidth);

	ImGui::Text("Delete File");
	ImGui::Indent();
		ImGui::Text("%s", croppedFile.Chr());
	ImGui::Unindent();
	if (croppedFile != file)
		Gutil::ToolTip(file.Chr());

	ImGui::Text("In Folder");
	ImGui::Indent();
		ImGui::Text("%s", croppedDir.Chr());
	ImGui::Unindent();
	if (croppedDir != dir)
		Gutil::ToolTip(dir.Chr());

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
	ImGui::Separator();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
	ImGui::Checkbox("Confirm file deletions in the future?", &profile.ConfirmDeletes);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);

	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	float okOffset = ImGui::GetStyle().IndentSpacing + pathWidth - buttonWidth + ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetCursorPosX(okOffset);

	ImGui::SetItemDefaultFocus();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("OK", tVector2(buttonWidth, 0.0f)))
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
	if (!ImGui::BeginPopupModal("Delete File Permanently", &isOpenPerm, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);

	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);

	// The overall width is based on the larger of the filename and directory. We clamp
	// the width of these two by calling CropStringToWidth.
	float maxPathWidth = Gutil::GetUIParamScaled(380.0f, 2.5f);
	float minPathWidth = Gutil::GetUIParamScaled(296.0f, 2.5f);
	float fileWidth = 0.0f;
	tString croppedFile = Gutil::CropStringToWidth(file, maxPathWidth, true, &fileWidth);
	float dirWidth = 0.0f;
	tString croppedDir  = Gutil::CropStringToWidth(dir,  maxPathWidth, true, &dirWidth);
	float pathWidth = tMax(fileWidth, dirWidth, minPathWidth);

	ImGui::Text("Delete File");
	ImGui::Indent();
		ImGui::Text("%s", croppedFile.Chr());
	ImGui::Unindent();
	if (croppedFile != file)
		Gutil::ToolTip(file.Chr());
	
	ImGui::Text("In Folder");
	ImGui::Indent();
		ImGui::Text("%s", croppedDir.Chr());
	ImGui::Unindent();
	if (croppedDir != dir)
		Gutil::ToolTip(dir.Chr());

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
	ImGui::Separator();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
	ImGui::Text("This cannot be undone. File deletion will be permanent.");
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);

	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	float okOffset = ImGui::GetStyle().IndentSpacing + pathWidth - buttonWidth + ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetCursorPosX(okOffset);

	ImGui::SetItemDefaultFocus();
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (Gutil::Button("OK", tVector2(buttonWidth, 0.0f)))
	{
		DeleteImageFile(fullname, false);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoSnapMessageNoFileBrowseModal(bool justPressed)
{
	if (justPressed)
		ImGui::OpenPopup("Snap Message##NoFileBrowse");

	// The unused isMessage bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isMessage = true;
	if
	(
		!ImGui::BeginPopupModal
		(
			"Snap Message##NoFileBrowse", &isMessage,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
		)
	)
	{
		return;
	}

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);
	ImGui::Text
	(
		"The Snap version of Tacent View does not\n"
		"support opening Nautilus or Dolphin.\n\n"
		"Please use the deb install or build from\n"
		"source if you need this feature on Linux."
	);
	ImGui::NewLine();

	float okOffset = Gutil::GetUIParamScaled(170.0f, 2.5f);
	ImGui::SetCursorPosX(okOffset);

	if (ImGui::Button("OK", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::DoSnapMessageNoFrameTransModal(bool justPressed)
{
	if (justPressed)
		ImGui::OpenPopup("Snap Message##NoTransparentWorkArea");

	// The unused isMessage bool is just so we get a close button in ImGui. Returns false if popup not open.
	bool isMessage = true;
	if
	(
		!ImGui::BeginPopupModal
		(
			"Snap Message##NoTransparentWorkArea", &isMessage,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
		)
	)
	{
		return;
	}

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);
	ImGui::Text
	(
		"The Snap version of Tacent View does not\n"
		"support transparent work area.\n\n"
		"Please use the deb install or build from\n"
		"source if you need this feature on Linux."
	);
	ImGui::NewLine();

	float okOffset = Gutil::GetUIParamScaled(170.0f, 2.5f);
	ImGui::SetCursorPosX(okOffset);
	if (ImGui::Button("OK", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void Viewer::DoRenameModal(bool renamePressed)
{
	if (renamePressed)
		ImGui::OpenPopup("Rename File");

	// The unused isOpenRen bool is just so we get a close button in ImGui.
	bool isOpenRen = true;
	if (!ImGui::BeginPopupModal("Rename File", &isOpenRen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		return;

	float buttonWidth = Gutil::GetUIParamScaled(76.0f, 2.5f);
	tString fullname = CurrImage->Filename;
	tString origname = tSystem::tGetFileName(fullname);

	static char newname[128] = "Filename";
	if (renamePressed)
		tStd::tStrcpy(newname, origname.Chr());

	bool nameChanged = false;
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
	if (ImGui::InputText("##NewNameText", newname, tNumElements(newname), ImGuiInputTextFlags_EnterReturnsTrue))
		nameChanged = true;
	ImGui::NewLine();

	if (Gutil::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();

	float okOffset = Gutil::GetUIParamScaled(165.0f, 2.5f);
	ImGui::SetCursorPosX(okOffset);

	if (Gutil::Button("OK", tVector2(buttonWidth, 0.0f)) || nameChanged)
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

	float fltTextWidth = ImGui::CalcTextSize("Filter##[##?##]").x;
	float fltWidth = ImGui::GetWindowWidth() - clrWidth - cpyWidth - fltTextWidth;
	LogFilter.Draw("Filter", fltWidth);
	ImGui::SameLine();
	Gutil::HelpMark("Place a negative sign in front to exclude (-excluded).");

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f*ImGui::GetStyle().ItemSpacing.y);
	ImGui::Separator();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y);

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
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::LogOutput);
	tVector2 windowSize = Gutil::GetUIParamScaled(tVector2(410.0f, 220.0f), 2.65f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	tVector2 minConstraint = windowSize * 0.53f;
	ImGui::SetNextWindowSizeConstraints(minConstraint, tVector2(4096.0f, 4096.0f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing;
	if (ImGui::Begin("Output Log", popen, flags))
	{
		Viewer::OutLog.DrawLog();
	}
	ImGui::End();
}
