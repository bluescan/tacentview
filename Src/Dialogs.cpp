// Dialogs.cpp
//
// Various dialogs and helpers including a log window, info overlay, help window, and about window.
//
// Copyright (c) 2019, 2020, 2021, 2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tVersion.cmake.h>
#include <Math/tVector2.h>
#include <Math/tColour.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Dialogs.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "Version.cmake.h"
using namespace tMath;


void Viewer::ShowPixelEditorOverlay(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(4);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	static bool live = true;
	static tColourf floatCol = tColourf::black;
	static tColourf floatColReset = tColourf::black;
	if (ImGui::Begin("Edit Pixel", popen, flags))
	{
		static int lastCursorX = -1;
		static int lastCursorY = -1;
		static Image* lastImage = nullptr;
		if ((lastCursorX != Viewer::CursorX) || (lastCursorY != Viewer::CursorY) || (lastImage != CurrImage))
		{
			lastCursorX = Viewer::CursorX;
			lastCursorY = Viewer::CursorY;
			lastImage = CurrImage;
			if (live)
				floatCol.Set(Viewer::PixelColour);
			floatColReset.Set(Viewer::PixelColour);
		}

		tColourf origCol = floatCol;
		if ((ImGui::ColorPicker4("Colour", floatCol.E, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_AlphaPreviewHalf), floatColReset.E) && (floatCol != origCol))
		{
			if (live)
			{
				CurrImage->Unbind();
				tColouri col; col.Set(floatCol);

				// Don't push undo steps if we're dragging around the cursor.
				CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, false, true);
				CurrImage->Bind();
				Viewer::SetWindowTitle();
			}
		}

		tVector4 resetVecCol(floatColReset.E);
		bool resetPressed2 = ImGui::ColorButton("Reset Colour", resetVecCol, 0);
		ImGui::SameLine();
		bool resetPressed = ImGui::Button("Reset", tVector2(100.0f, 0.0f));
		ImGui::SameLine();
		if (resetPressed || resetPressed2)
		{
			if (live)
			{
				CurrImage->Unbind();
				tColouri col; col.Set(floatColReset);
				CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true, true);
				CurrImage->Bind();
				Viewer::SetWindowTitle();
			}
			else
			{
				floatCol = floatColReset;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Live", &live);
		ImGui::SameLine();
		if (!live && ImGui::Button("Apply", tVector2(100.0f, 0.0f)))
		{
			CurrImage->Unbind();
			tColouri col; col.Set(floatCol);
			CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
	}

	// If we closed the dialog and we're live, set the colour one more time but push to the undo stack.
	if (popen && (*popen == false) && live)
	{
		CurrImage->Unbind();
		tColouri col;
		col.Set(floatColReset);
		CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, false, true);
		col.Set(floatCol);
		CurrImage->SetPixelColour(Viewer::CursorX, Viewer::CursorY, col, true);
		CurrImage->Bind();
		Viewer::SetWindowTitle();
	}
	ImGui::End();
}


void Viewer::ShowChannelFilterOverlay(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(6);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Channel Filter", popen, flags))
	{
		ImGui::Text("Display");
		if (ImGui::Checkbox("Channel Intensity", &Viewer::DrawChannel_AsIntensity))
		{
			if (Viewer::DrawChannel_AsIntensity)
			{
				Viewer::DrawChannel_R = true;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = false;
			}
			else
			{
				Viewer::DrawChannel_R = true;
				Viewer::DrawChannel_G = true;
				Viewer::DrawChannel_B = true;
				Viewer::DrawChannel_A = true;
			}
		}
		if (Viewer::DrawChannel_AsIntensity)
		{
			if (ImGui::RadioButton("Red", Viewer::DrawChannel_R))
			{
				Viewer::DrawChannel_R = true;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = false;
			}
			if (ImGui::RadioButton("Green", Viewer::DrawChannel_G))
			{
				Viewer::DrawChannel_R = false;
				Viewer::DrawChannel_G = true;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = false;
			}
			if (ImGui::RadioButton("Blue", Viewer::DrawChannel_B))
			{
				Viewer::DrawChannel_R = false;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = true;
				Viewer::DrawChannel_A = false;
			}
			if (ImGui::RadioButton("Alpha", Viewer::DrawChannel_A))
			{
				Viewer::DrawChannel_R = false;
				Viewer::DrawChannel_G = false;
				Viewer::DrawChannel_B = false;
				Viewer::DrawChannel_A = true;
			}
		}
		else
		{
			ImGui::Checkbox("Red", &Viewer::DrawChannel_R);
			ImGui::Checkbox("Green", &Viewer::DrawChannel_G);
			ImGui::Checkbox("Blue", &Viewer::DrawChannel_B);
			ImGui::Checkbox("Alpha", &Viewer::DrawChannel_A);
			ImGui::SameLine(); ShowHelpMark
			(
				"Alpha is interprested as blending opacity. When this channel is set to false full alpha is used and"
				"image is drawn opaque. When true it blends whatever colour channels are selected with the current background."
			);
		}

		tColourf floatCol(Config::Current->BackgroundColour);
		if (ImGui::ColorEdit3("##Background", floatCol.E, ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar))
		{
			Config::Current->BackgroundColour.Set(floatCol);
			Config::Current->BackgroundColour.A = 0xFF;
		}

		ImGui::SameLine();
		const char* backgroundItems[] = { "None", "Checker", "Solid" };
		ImGui::PushItemWidth(83);
		ImGui::Combo("##Background Style", &Config::Current->BackgroundStyle, backgroundItems, tNumElements(backgroundItems));
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ShowHelpMark("Background colour and style.\nThe blend-background button uses the colour regardless of style.");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		ImGui::Text("Modify");

		if (ImGui::Button("Blend Background", tVector2(112.0f, 0.0f)))
		{
			CurrImage->Unbind();
			CurrImage->AlphaBlendColour(Config::Current->BackgroundColour, true);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
		ImGui::SameLine();
		ShowHelpMark("Blend background colour into RGB of image based on alpha. Sets alphas to full when done.");

		tcomps channels = (Viewer::DrawChannel_R ? tComp_R : 0) | (Viewer::DrawChannel_G ? tComp_G : 0) | (Viewer::DrawChannel_B ? tComp_B : 0) | (Viewer::DrawChannel_A ? tComp_A : 0);
		if (ImGui::Button("Max Selected", tVector2(112.0f, 0.0f)))
		{
			CurrImage->Unbind();
			tColouri full(255, 255, 255, 255);
			CurrImage->SetAllPixels(full, channels);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
		ImGui::SameLine();
		ShowHelpMark("Sets selected channel(s) to their maximum value (255).");

		if (ImGui::Button("Zero Selected", tVector2(112.0f, 0.0f)))
		{
			CurrImage->Unbind();
			tColouri zero(0, 0, 0, 0);
			CurrImage->SetAllPixels(zero, channels);
			CurrImage->Bind();
			Viewer::SetWindowTitle();
		}
		ImGui::SameLine();
		ShowHelpMark("Sets selected channel(s) to zero.");
	}

	ImGui::End();
}


void Viewer::ShowAboutPopup(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(3);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoFocusOnAppearing	|	ImGuiWindowFlags_NoNav;

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
		ImGui::Text("BCDec V %d.%d", tImage::Version_BCDec_Major, tImage::Version_BCDec_Minor);
		ImGui::Text("QOI");
	}
	ImGui::End();
}


void Viewer::DoDeleteFileModal()
{
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
	ImGui::Checkbox("Confirm file deletions in the future?", &Config::Current->ConfirmDeletes);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
	{
		DeleteImageFile(fullname, true);
		ImGui::CloseCurrentPopup();
	}
	ImGui::SetItemDefaultFocus();
	ImGui::EndPopup();
}


void Viewer::DoDeleteFileNoRecycleModal()
{
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

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)))
	{
		DeleteImageFile(fullname, false);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void Viewer::DoRenameModal(bool justOpened)
{
	tString fullname = CurrImage->Filename;
	tString origname = tSystem::tGetFileName(fullname);

	static char newname[128] = "Filename";
	if (justOpened)
		tStd::tStrcpy(newname, origname.Chr());

	bool nameChanged = false;
	if (ImGui::InputText("", newname, tNumElements(newname), ImGuiInputTextFlags_EnterReturnsTrue))
		nameChanged = true;
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100.0f, 0.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100.0f, 0.0f)) || nameChanged)
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


// Parts of this class are a modification of the one that ships with Dear ImGui. The DearImGui
// licence (MIT) may be found in the txt file Licence_DearImGui_MIT.txt in the Data folder.
void Viewer::NavLogBar::ClearLog()
{
	LogBuf.clear();
	LogLineOffsets.clear();
	LogLineOffsets.push_back(0);
}


void Viewer::NavLogBar::AddLog(const char* fmt, ...)
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


void Viewer::NavLogBar::Draw()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14.0f);
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
			ImageFileParam.Param = upDir + "dummyfile.txt";
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
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.0f);
		if (ImGui::BeginCombo("##combo", nullptr, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoPreview))
		{
			for (tStringItem* subDir = ImagesSubDirs.First(); subDir; subDir = subDir->Next())
			{
				bool isSelected = false;
				if (ImGui::Selectable(subDir->Chr(), isSelected))
				{
					// Selection made. This only runs once.
					ImageFileParam.Param = ImagesDir + *subDir + "/" + "dummyfile.txt";
					PopulateImages();
					SetCurrentImage();
					SetWindowTitle();
					break;
				}
			}
			ImGui::EndCombo();
		}
	}

	if (ShowLog)
		DrawLog();
}


void Viewer::NavLogBar::DrawLog()
{
	if (ImGui::Button("Clear"))
		ClearLog();

	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	LogFilter.Draw("Filter", -100.0f);
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
