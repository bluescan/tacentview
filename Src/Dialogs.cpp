// Dialogs.h
//
// Viewer dialogs including cheatsheet, about, save-as, and the image information overlay.
//
// Copyright (c) 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <GL/glew.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#include <Foundation/tVersion.h>
#include <System/tFile.h>
#include "imgui.h"
#include "Dialogs.h"
#include "TacitImage.h"
#include "TacitTexView.h"
using namespace tStd;
using namespace tSystem;


void TexView::ShowInfoOverlay(bool* popen, float x, float y, float w, float h, int cursorX, int cursorY)
{
	// This overlay function is pretty much taken from the DearImGui demo code.
	const float margin = 10.0f;

	ImVec2 windowPos = ImVec2
	(
		x + ((Config.OverlayCorner & 1) ? w - margin : margin),
		y + ((Config.OverlayCorner & 2) ? h - margin : margin)
	);
	ImVec2 windowPivot = ImVec2
	(
		(Config.OverlayCorner & 1) ? 1.0f : 0.0f,
		(Config.OverlayCorner & 2) ? 1.0f : 0.0f
	);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("InfoOverlay", popen, flags))
	{
		ImGui::Text("Image Info - Enter to Toggle");
		ImGui::Text("Right-Click to change position.");
		ImGui::Separator();

		if (CurrImage)
		{
			tColourf floatCol(PixelColour);
			ImVec4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
			ImGui::Text("Colour"); ImGui::SameLine();
			if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, ImVec2(15,15)))
				ImGui::OpenPopup("CopyColourOverlayAs");

			if (ImGui::BeginPopup("CopyColourOverlayAs"))
				ColourCopyAs();

			TacitImage::ImgInfo& info = CurrImage->Info;
			if (info.IsValid())
			{
				ImGui::Text("Size: %dx%d", info.Width, info.Height);
				ImGui::Text("Pixel Format: %s", info.PixelFormat.ConstText());
				ImGui::Text("Bit Depth: %d", info.SrcFileBitDepth);
				ImGui::Text("Opaque: %s", info.Opaque ? "true" : "false");
				ImGui::Text("Mipmaps: %d", info.Mipmaps);
				ImGui::Text("File Size (B): %d", info.SizeBytes);
				ImGui::Text("Cursor: (%d, %d)", cursorX, cursorY);
			}
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left",		nullptr, Config.OverlayCorner == 0)) Config.OverlayCorner = 0;
			if (ImGui::MenuItem("Top-right",	nullptr, Config.OverlayCorner == 1)) Config.OverlayCorner = 1;
			if (ImGui::MenuItem("Bottom-left",  nullptr, Config.OverlayCorner == 2)) Config.OverlayCorner = 2;
			if (ImGui::MenuItem("Bottom-right", nullptr, Config.OverlayCorner == 3)) Config.OverlayCorner = 3;
			if (popen && ImGui::MenuItem("Close")) *popen = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}


void TexView::ColourCopyAs()
{
	tColourf floatCol(PixelColour);
	ImGui::Text("Copy As...");
	int ri = PixelColour.R; int gi = PixelColour.G; int bi = PixelColour.B; int ai = PixelColour.A;
	float rf = floatCol.R; float gf = floatCol.G; float bf = floatCol.B; float af = floatCol.A;
	tString cpyTxt;
	tsPrintf(cpyTxt, "%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "#%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "#%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "0x%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "%.3f, %.3f, %.3f, %.3f", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "%.3ff, %.3ff, %.3ff, %.3ff", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "(%.3f, %.3f, %.3f, %.3f)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "(%.3ff, %.3ff, %.3ff, %.3ff)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	ImGui::EndPopup();
}


void TexView::ShowCheatSheetPopup(bool* popen, float right, float top)
{
	const float margin = 32.0f;
	ImVec2 windowPos = ImVec2(right - margin, top + margin);
	ImVec2 windowPivot = ImVec2(1.0f, 0.0f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Cheat Sheet", popen, flags))
	{
		float col = ImGui::GetCursorPosX() + 80.0f;
		ImGui::Text("Left Arrow");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Previous Image");
		ImGui::Text("Right Arrow");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image");
		ImGui::Text("Ctrl-Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Skip to First Image");
		ImGui::Text("Ctrl-Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Skip to Last Image");
		ImGui::Text("Space");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image");
		ImGui::Text("Ctrl+");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom In");
		ImGui::Text("Ctrl-");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Out");
		ImGui::Text("F1");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Cheat Sheet");
		ImGui::Text("F11");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Fullscreen");
		ImGui::Text("Alt-Enter");   ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Fullscreen");
		ImGui::Text("Esc");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Exit Fullscreen");
		ImGui::Text("Enter");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Overlay");
		ImGui::Text("LMB-Click");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Set Colour Reticle Pos");
		ImGui::Text("RMB-Drag");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Pan Image");
		ImGui::Text("Alt-F4");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Quit");
	}
	ImGui::End();
}


void TexView::ShowAboutPopup(bool* popen, float right, float top)
{
	const float margin = 32.0f;
	ImVec2 windowPos = ImVec2(margin, top + margin);
	ImVec2 windowPivot = ImVec2(0.0f, 0.0f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("About", popen, flags))
	{
		int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
		glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
		ImGui::Text("Tacit Viewer V %d.%d.%d by Tristan Grimmer", TexView::MajorVersion, TexView::MinorVersion, TexView::Revision);
		ImGui::Separator();
		ImGui::Text("The following amazing and liberally licenced frameworks are used by this tool.");
		ImGui::Text("Dear ImGui V %s", IMGUI_VERSION);
		ImGui::Text("GLEW V %s", glewGetString(GLEW_VERSION));
		ImGui::Text("GLFW V %d.%d.%d", glfwMajor, glfwMinor, glfwRev);
		ImGui::Text("Tacent Library V %d.%d.%d", tVersion::Major, tVersion::Minor, tVersion::Revision);
		ImGui::Text("CxImage");
		ImGui::Text("nVidia Texture Tools");
		ImGui::Text("Ionicons");
		ImGui::Text("Roboto Google Font");
	}
	ImGui::End();
}


void TexView::ShowSaveAsDialog(bool* popen, bool justOpened)
{
	ImGuiWindowFlags windowFlags = 0;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(370, 210), ImGuiCond_FirstUseEver);

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Save As", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	static int finalWidth = 512;
	static int finalHeight = 512;
	if (justOpened && CurrImage)
	{
		finalWidth = CurrImage->GetWidth();
		finalHeight = CurrImage->GetHeight();
	}
	ImGui::InputInt("Final Width", &finalWidth);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output height in pixels.");

	ImGui::InputInt("Final Height", &finalHeight);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output height in pixels.");

	if (ImGui::Button("Prev Pow2"))
	{
		finalWidth = tMath::tNextLowerPower2(CurrImage->GetWidth());
		finalHeight = tMath::tNextLowerPower2(CurrImage->GetHeight());
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Pow2"))
	{
		finalWidth = tMath::tNextHigherPower2(CurrImage->GetWidth());
		finalHeight = tMath::tNextHigherPower2(CurrImage->GetHeight());
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		finalWidth = CurrImage->GetWidth();
		finalHeight = CurrImage->GetHeight();
	}

	// Matches tImage::tPicture::tFilter.
	const char* filterItems[] = { "NearestNeighbour", "Box", "Bilinear", "Bicubic", "Quadratic", "Hamming" };
	ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, IM_ARRAYSIZE(filterItems));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	const char* fileTypeItems[] = { "TGA", "PNG", "BMP", "JPG", "GIF" };
	static int itemCurrent = 0;
	ImGui::Combo("File Type", &itemCurrent, fileTypeItems, IM_ARRAYSIZE(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Output image format. JPG and GIF do not support alpha channel.");

	tString extension = ".tga";
	switch (itemCurrent)
	{
		case 0: extension = ".tga"; break;
		case 1: extension = ".png"; break;
		case 2: extension = ".bmp"; break;
		case 3: extension = ".jpg"; break;
		case 4: extension = ".gif"; break;
	}

	static bool rleCompression = false;
	if (itemCurrent == 0)
		ImGui::Checkbox("RLE Compression", &rleCompression);

	static char filename[128] = "Filename";
	if (justOpened)
	{
		tString baseName = tSystem::tGetFileBaseName(CurrImage->Filename);
		tStrcpy(filename, baseName.ConstText());
	}
	ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	if (ImGui::Button("Save"))
	{
		tString imagesDir = tSystem::tGetCurrentDir();
		if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
			imagesDir = tSystem::tGetDir(ImageFileParam.Get());
		tString outFile = imagesDir + tString(filename) + extension;

		tImage::tPicture outPic(CurrImage->GetWidth(), CurrImage->GetHeight());
		outPic.Set(*CurrImage->GetPrimaryPicture());

		if ((outPic.GetWidth() != finalWidth) || (outPic.GetHeight() != finalHeight))
			outPic.Resample(finalWidth, finalHeight, tImage::tPicture::tFilter(Config.ResampleFilter));

		if (tFileExists(outFile))
		{
			tPrintf("File %s already exists. Will not overwrite.\n", outFile.ConstText());
		}
		else
		{
			bool success = false;
			tImage::tPicture::tColourFormat colourFmt = outPic.IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
			if (itemCurrent == 0)
				success = outPic.SaveTGA(outFile, tImage::tFileTGA::tFormat::Auto, rleCompression ? tImage::tFileTGA::tCompression::RLE : tImage::tFileTGA::tCompression::None);
			else
				success = outPic.Save(outFile, colourFmt);

			if (success)
				tPrintf("Saved image as : %s\n", outFile.ConstText());
			else
				tPrintf("Failed to save image %s\n", outFile.ConstText());
		}

		Images.Clear();
		PopulateImages();
		SetCurrentImage(outFile);
	}

	ImGui::End();
}
