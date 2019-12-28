// TacitTexView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2018, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <dwmapi.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Foundation/tVersion.h>
#include <System/tCommand.h>
#include <Image/tPicture.h>
#include <System/tFile.h>
#include <System/tTime.h>
#include <System/tScript.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "TacitImage.h"
#include "ImGuiLogWindow.h"
#include "Settings.h"
using namespace tStd;
using namespace tSystem;


tCommand::tParam ImageFileParam(1, "ImageFile", "File to open.");


namespace TexView
{
	int MajorVersion			= 0;
	int MinorVersion			= 9;
	int Revision				= 1;
	Settings Config;
	bool LogWindowOpen			= true;
	TexView::ImGuiLog LogWindow;
	tList<TacitImage> Images;
	TacitImage* CurrImage		= nullptr;
	TacitImage CursorImage;
	TacitImage PrevImage;
	TacitImage NextImage;
	double NextPrevDisappear	= 1.0;
	GLFWwindow* Window			= nullptr;
	bool FullscreenMode			= false;
	bool RMBDown				= false;
	int DragAnchorX				= 0;
	int DragAnchorY				= 0;
	float ZoomPercent			= 100.0f;
	bool FitMode				= true;
	int Dispw					= 1;
	int Disph					= 1;
	int PanOffsetX				= 0;
	int PanOffsetY				= 0;
	int DragDownOffsetX			= 0;
	int DragDownOffsetY			= 0;
	int CursorX					= -1;
	int CursorY					= -1;
	tColouri PixelColour		= tColouri::black;
	const int MaxLoadedCount	= 35;			// If more images that this loaded we start unloading to free mem.
	TacitImage* UnloadImage		= nullptr;

	void DrawTextureViewerLog(float x, float y, float w, float h);
	void PrintRedirectCallback(const char* text, int numChars)															{ LogWindow.AddLog("%s", text); }
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }
	bool CompareFunc(const tStringItem& a, const tStringItem& b)														{ return tStrcmp(a.ConstText(), b.ConstText()) < 0; }
	void FindTextureFiles();
	void SetCurrentImage(const tString& currFilename = tString());

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	void ShowHelpMark(const char* desc);
	void ShowContactSheetDialog(bool* popen, bool justOpened);
	void SetWindowTitle();
	bool OnPrevious();
	bool OnNext();
	bool ChangeScreenMode(bool fullscreeen);
	void Update(GLFWwindow* window, double dt, bool dopoll = true);
	void WindowRefreshFun(GLFWwindow* window)																			{ Update(window, 0.0, false); }
	void KeyCallback(GLFWwindow*, int key, int scancode, int action, int modifiers);
	void MouseButtonCallback(GLFWwindow*, int mouseButton, int x, int y);
	void CursorPosCallback(GLFWwindow*, double x, double y);
	void ScrollWheelCallback(GLFWwindow*, double x, double y);
	void FileDropCallback(GLFWwindow*, int count, const char** paths);
}


void TexView::DrawTextureViewerLog(float x, float y, float w, float h)
{
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);

	ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	LogWindow.Draw("Log", &LogWindowOpen);
	ImGui::End();
}


void TexView::FindTextureFiles()
{
	tString imagesDir = tSystem::tGetCurrentDir();
	if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
		imagesDir = tSystem::tGetDir(ImageFileParam.Get());

	tPrintf("Looking for image files in %s\n", imagesDir.ConstText());
	tList<tStringItem> foundFiles;

	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.jpg");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.gif");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tga");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.png");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tif");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tiff");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.bmp");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.dds");
	foundFiles.Sort(CompareFunc, tListSortAlgorithm::Merge);
	for (tStringItem* filename = foundFiles.First(); filename; filename = filename->Next())
		Images.Append(new TacitImage(*filename));

	CurrImage = nullptr;
	UnloadImage = nullptr;
}


void TexView::SetCurrentImage(const tString& currFilename)
{
	for (TacitImage* si = Images.First(); si; si = si->Next())
	{
		tString siName = tSystem::tGetFileName(si->Filename);
		tString imgName = tSystem::tGetFileName(currFilename);

		if (tStricmp(siName.ConstText(), imgName.ConstText()) == 0)
		{
			CurrImage = si;
			UnloadImage = si;
			break;
		}
	}

	if (!CurrImage)
	{
		CurrImage = Images.First();
		UnloadImage = CurrImage;
		if (!currFilename.IsEmpty())
			tPrintf("Could not display [%s].\n", tSystem::tGetFileName(currFilename).ConstText());
		if (CurrImage && !CurrImage->Filename.IsEmpty())
			tPrintf("Displaying [%s] instead.\n", tSystem::tGetFileName(CurrImage->Filename).ConstText());
	}

	if (CurrImage)
	{
		CurrImage->Load();
		CurrImage->PrintInfo();
		SetWindowTitle();
		FitMode = true;
		PanOffsetX = 0;
		PanOffsetY = 0;
		DragDownOffsetX = 0;
		DragDownOffsetY = 0;
	}
}


bool TexView::OnPrevious()
{
	if (!CurrImage || !CurrImage->Prev())
		return false;

	CurrImage = CurrImage->Prev();
	UnloadImage = CurrImage;
	CurrImage->Load();
	CurrImage->PrintInfo();
	SetWindowTitle();
	FitMode = true;
	PanOffsetX = 0;
	PanOffsetY = 0;
	DragDownOffsetX = 0;
	DragDownOffsetY = 0;
	return true;
}


bool TexView::OnNext()
{
	if (!CurrImage || !CurrImage->Next())
		return false;

	CurrImage = CurrImage->Next();
	UnloadImage = CurrImage;
	CurrImage->Load();
	CurrImage->PrintInfo();
	SetWindowTitle();
	FitMode = true;
	PanOffsetX = 0;
	PanOffsetY = 0;
	DragDownOffsetX = 0;
	DragDownOffsetY = 0;
	return true;
}


void TexView::ShowHelpMark(const char* desc)
{
	ImGui::TextDisabled("[?]");
	if (!ImGui::IsItemHovered())
		return;

	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}


void TexView::ShowContactSheetDialog(bool* popen, bool justOpened)
{
	ImGuiWindowFlags windowFlags = 0;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(370, 254), ImGuiCond_FirstUseEver);

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Contact Sheet Generator", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	static int frameWidth = 256;
	static int frameHeight = 256;
	static int numRows = 4;
	static int numCols = 4;
	static int finalWidth = 2048;
	static int finalHeight = 2048;
	if (justOpened && CurrImage)
	{
		frameWidth = CurrImage->GetWidth();
		frameHeight = CurrImage->GetHeight();
		numRows = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
		numCols = int(tMath::tCeiling(tMath::tSqrt(float(Images.Count()))));
	}

	int contactWidth = frameWidth * numCols;
	int contactHeight = frameHeight * numRows;
	if (justOpened && CurrImage)
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}

	ImGui::InputInt("Frame Width", &frameWidth);
	ImGui::SameLine();
	ShowHelpMark("Single frame width in pixels.");

	ImGui::InputInt("Frame Height", &frameHeight);
	ImGui::SameLine();
	ShowHelpMark("Single frame height in pixels.");

	ImGui::InputInt("Columns", &numCols);
	ImGui::SameLine();
	ShowHelpMark("Number of columns. Determines overall width.");
 
	ImGui::InputInt("Rows", &numRows);
	ImGui::SameLine();
	ShowHelpMark("Number of rows. Determines overall height.");

	ImGui::InputInt("Final Width", &finalWidth);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output sheet height in pixels.");

	ImGui::InputInt("Final Height", &finalHeight);
	ImGui::SameLine();
	ShowHelpMark("Final scaled output sheet height in pixels.");

	if (ImGui::Button("Prev Pow2"))
	{
		finalWidth = tMath::tNextLowerPower2(contactWidth);
		finalHeight = tMath::tNextLowerPower2(contactHeight);
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Pow2"))
	{
		finalWidth = tMath::tNextHigherPower2(contactWidth);
		finalHeight = tMath::tNextHigherPower2(contactHeight);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		finalWidth = contactWidth;
		finalHeight = contactHeight;
	}

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

	static char filename[128] = "ContactSheet";
	ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	if (ImGui::Button("Generate"))
	{
		tString imagesDir = tSystem::tGetCurrentDir();
		if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
			imagesDir = tSystem::tGetDir(ImageFileParam.Get());
		tString outFile = imagesDir + tString(filename) + extension;

		tImage::tPicture outPic(contactWidth, contactHeight);
		outPic.SetAll(tColouri(0, 0, 0, 0));

		// Do the work.
		int frameWidth = contactWidth / numCols;
		int frameHeight = contactHeight / numRows;
		int ix = 0;
		int iy = 0;
		int frame = 0;

		tPrintf("Loading all frames...\n");
		bool allOpaque = true;
		for (TacitImage* img = Images.First(); img; img = img->Next())
		{
			if (!img->IsLoaded())
				img->Load();

			if (img->IsLoaded() && !img->IsOpaque())
				allOpaque = false;
		}

		TacitImage* currImg = Images.First();
		while (currImg)
		{
			if (!currImg->IsLoaded())
			{
				currImg = currImg->Next();
				continue;
			}

			if (tSystem::tGetFileBaseName(currImg->Filename) == tSystem::tGetFileBaseName(outFile))
			{
				currImg = currImg->Next();
				continue;
			}

			tPrintf("Processing frame %d : %s at (%d, %d).\n", frame, currImg->Filename.ConstText(), ix, iy);
			frame++;
			tImage::tPicture* currPic = currImg->PictureImages.First();

			tImage::tPicture resampled;
			if ((currImg->GetWidth() != frameWidth) || (currImg->GetHeight() != frameHeight))
			{
				resampled.Set(*currPic);
				resampled.Resample(frameWidth, frameHeight);
			}

			// Copy resampled frame into place.
			for (int y = 0; y < frameHeight; y++)
				for (int x = 0; x < frameWidth; x++)
					outPic.SetPixel
					(
						x + (ix*frameWidth),
						y + ((numRows-1-iy)*frameHeight),
						resampled.IsValid() ? resampled.GetPixel(x, y) : currPic->GetPixel(x, y)
					);

			currImg = currImg->Next();

			ix++;
			if (ix >= numCols)
			{
				ix = 0;
				iy++;
				if (iy >= numRows)
					break;
			}
		}

		tImage::tPicture::tColourFormat colourFmt = allOpaque ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
		if ((finalWidth == contactWidth) && (finalHeight == contactHeight))
		{
			outPic.Save(outFile, colourFmt);
		}
		else
		{
			tImage::tPicture finalResampled(outPic);
			finalResampled.Resample(finalWidth, finalHeight);
			finalResampled.Save(outFile, colourFmt);
		}
		Images.Clear();
		FindTextureFiles();
		SetCurrentImage(outFile);
	}

	ImGui::End();
}


void TexView::SetWindowTitle()
{
	if (!Window)
		return;

	tString title = "Tacit Viewer";
	if (CurrImage && !CurrImage->Filename.IsEmpty())
		title = title + " - " + tGetFileName(CurrImage->Filename);

	glfwSetWindowTitle(Window, title.ConstText());
}


void TexView::Update(GLFWwindow* window, double dt, bool dopoll)
{
	// Poll and handle events like inputs, window resize, etc. You can read the io.WantCaptureMouse,
	// io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	//
	// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	// When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	//
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those
	// two flags.
	if (dopoll)
		glfwPollEvents();

	ImVec4 clearColour = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int bottomUIHeight = 150;
	int topUIHeight = 22;
	if (FullscreenMode)
	{
		bottomUIHeight = 0;
		topUIHeight = 0;
	}

	// Start the Dear ImGui frame.
	ImGui_ImplOpenGL2_NewFrame();		
	ImGui_ImplGlfw_NewFrame();
	int dispw, disph;
	glfwGetFramebufferSize(window, &dispw, &disph);
	if ((dispw != Dispw) || (disph != Disph))
	{
		Dispw = dispw;
		Disph = disph;
		if ((PanOffsetX+DragDownOffsetX == 0) && (PanOffsetY+DragDownOffsetY == 0))
		{
			PanOffsetX = 0;
			PanOffsetY = 0;
			DragDownOffsetX = 0;
			DragDownOffsetY = 0;
		}
	}

	int workAreaW = Dispw;
	int workAreaH = Disph - bottomUIHeight - topUIHeight;
	float workAreaAspect = float(workAreaW)/float(workAreaH);

	glViewport(0, bottomUIHeight, workAreaW, workAreaH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, workAreaW, 0, workAreaH, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	float draww = 1.0f;
	float drawh = 1.0f;
	float iw = 1.0f;
	float ih = 1.0f;
	static int imgxi = 0;
	static int imgyi = 0;
	if (CurrImage)
	{
		iw = float(CurrImage->GetWidth());
		ih = float(CurrImage->GetHeight());
		float picAspect = iw/ih;

		float hmargin = 0.0f;
		float vmargin = 0.0f;
		if (workAreaAspect > picAspect)
		{
			drawh = float(workAreaH);
			draww = picAspect * drawh;
			hmargin = (workAreaW - draww) * 0.5f;
			vmargin = 0.0f;;
		}
		else
		{
			draww = float(workAreaW);
			drawh = draww / picAspect;
			vmargin = (workAreaH - drawh) * 0.5f;
			hmargin = 0.0f;
		}

		// Semitransparent checker background.
		int x = 0;
		int y = 0;
		bool lineStartToggle = false;
		float checkSize = 16.0f;
		while (y*checkSize < drawh)
		{
			bool colourToggle = lineStartToggle;

			while (x*checkSize < draww)
			{
				if (colourToggle)
					glColor4f(0.3f, 0.3f, 0.35f, 1.0f);
				else
					glColor4f(0.4f, 0.4f, 0.45f, 1.0f);

				colourToggle = !colourToggle;

				float cw = checkSize;
				if ((x+1)*checkSize > draww)
					cw -= (x+1)*checkSize - draww;

				float ch = checkSize;
				if ((y+1)*checkSize > drawh)
					ch -= (y+1)*checkSize - drawh;

				float l = tMath::tRound(hmargin+x*checkSize);
				float r = tMath::tRound(hmargin+x*checkSize+cw);
				float b = tMath::tRound(vmargin+y*checkSize);
				float t = tMath::tRound(vmargin+y*checkSize+ch);

				glBegin(GL_QUADS);
				glVertex2f(l, b);
				glVertex2f(l, t);
				glVertex2f(r, t);
				glVertex2f(r, b);
				glEnd();

				x++;
			}
			x = 0;
			y++;
			lineStartToggle = !lineStartToggle;
		}

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		CurrImage->Bind();
		glEnable(GL_TEXTURE_2D);

		// w and h are the image width and height. draww and drawh are the drawable area width and height.
		float l = tMath::tRound(hmargin);
		float r = tMath::tRound(hmargin+draww);
		float b = tMath::tRound(vmargin);
		float t = tMath::tRound(vmargin+drawh);

		float uvHMargin = 0.0f;
		float uvVMargin = 0.0f;

		if (FitMode)
		{
			ZoomPercent = 100.0f;
			if (draww < iw)
				ZoomPercent = 100.0f * draww / iw;
		}

		float w = iw * ZoomPercent/100.0f;
		float h = ih * ZoomPercent/100.0f;

		// If the image is smaller than the drawable area we draw a quad of the correct size with full 0..1 range in the uvs.
		if (w < draww)
		{
			float offsetW = tMath::tRound((draww - w) / 2.0f);
			l += offsetW;
			r -= offsetW;
			float offsetH = tMath::tRound((drawh - h) / 2.0f);
			b += offsetH;
			t -= offsetH;
		}
		else
		{
			float propw = draww / w;
			uvHMargin = (1.0f - propw)/2.0f;
			float proph = drawh / h;
			uvVMargin = (1.0f - proph)/2.0f;
		}

		// Modify the UVs here to magnify.
		if (draww < w)
		{
			if (RMBDown)
			{
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				DragDownOffsetX = int(xpos) - DragAnchorX;
			}

			if (float(PanOffsetX+DragDownOffsetX) > (w-draww)/2.0f)
				DragDownOffsetX = int((w-draww)/2.0f) - PanOffsetX;
			if (float(PanOffsetX+DragDownOffsetX) < -(w-draww)/2.0f)
				DragDownOffsetX = int(-(w-draww)/2.0f) - PanOffsetX;
		}

		if (drawh < h)
		{
			if (RMBDown)
			{
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				DragDownOffsetY = int(ypos) - DragAnchorY;
			}

			if (float(PanOffsetY + DragDownOffsetY) > (h - drawh) / 2.0f)
				DragDownOffsetY = int((h - drawh) / 2.0f) - PanOffsetY;
			if (float(PanOffsetY + DragDownOffsetY) < -(h - drawh) / 2.0f)
				DragDownOffsetY = int(-(h - drawh) / 2.0f) - PanOffsetY;
		}

		if (draww > w)
		{
			PanOffsetX = 0;
			DragDownOffsetX = 0;
		}
		if (drawh > h)
		{
			PanOffsetY = 0;
			DragDownOffsetY = 0;
		}

		float uOffset = -float(PanOffsetX+DragDownOffsetX)/w;
		float vOffset =  float(PanOffsetY+DragDownOffsetY)/h;
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f + uvHMargin + uOffset, 0.0f + uvVMargin + vOffset); glVertex2f(l, b);
		glTexCoord2f(0.0f + uvHMargin + uOffset, 1.0f - uvVMargin + vOffset); glVertex2f(l, t);
		glTexCoord2f(1.0f - uvHMargin + uOffset, 1.0f - uvVMargin + vOffset); glVertex2f(r, t);
		glTexCoord2f(1.0f - uvHMargin + uOffset, 0.0f + uvVMargin + vOffset); glVertex2f(r, b);
		glEnd();

		float cw = float((CursorImage.GetWidth() - 1) >> 1);
		float ch = float((CursorImage.GetHeight() - 1) >> 1);
		float cx = float(CursorX);
		float cy = float(CursorY);
		float areaH = float(workAreaH) + topUIHeight;

		float picX = cx - l;
		float picY = (areaH - cy) - b;
		float normX = picX / (r-l);
		float normY = picY / (t-b);
		bool insideRect = tMath::tInRange(normX, 0.0f, 1.0f) && tMath::tInRange(normY, 0.0f, 1.0f);
		bool showCursor = true;
		if (insideRect)
		{
			float imgx = iw * tMath::tLesc
			(
				tMath::tGetSaturate(normX),
				0.0f + uvHMargin + uOffset, 1.0f - uvHMargin + uOffset
			);
			float imgy = ih * tMath::tLesc
			(
				tMath::tGetSaturate(normY),
				0.0f + uvVMargin + vOffset, 1.0f - uvVMargin + vOffset
			);
			imgxi = tMath::tGetClamp(int(imgx), 0, CurrImage->GetWidth()-1);
			imgyi = tMath::tGetClamp(int(imgy), 0, CurrImage->GetHeight()-1);
			PixelColour = CurrImage->GetPixel(imgxi, imgyi);
		}
		else
		{
			showCursor = false;
		}

		if (showCursor)
		{
			CursorImage.Bind();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(cx-cw, areaH-(cy+ch));
			glTexCoord2f(0.0f, 1.0f); glVertex2f(cx-cw, areaH-(cy-ch));
			glTexCoord2f(1.0f, 1.0f); glVertex2f(cx+cw, areaH-(cy-ch));
			glTexCoord2f(1.0f, 0.0f); glVertex2f(cx+cw, areaH-(cy+ch));
			glEnd();
		}

		glDisable(GL_TEXTURE_2D);
		glFlush(); // Don't need this with GLUT_DOUBLE and glutSwapBuffers.
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, dispw, 0, disph, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	ImGui::NewFrame();

	// Show the big demo window. You can browse its code to learn more about Dear ImGui.
	static bool showDemoWindow = false;
	//static bool showDemoWindow = true;
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

    ImGuiWindowFlags flagsNextPrev = 0;
    flagsNextPrev |= ImGuiWindowFlags_NoTitleBar;
    flagsNextPrev |= ImGuiWindowFlags_NoScrollbar;
    //flagsNextPrev |= ImGuiWindowFlags_MenuBar;
    flagsNextPrev |= ImGuiWindowFlags_NoMove;
    flagsNextPrev |= ImGuiWindowFlags_NoResize;
    flagsNextPrev |= ImGuiWindowFlags_NoCollapse;
	flagsNextPrev |= ImGuiWindowFlags_NoNav;
    flagsNextPrev |= ImGuiWindowFlags_NoBackground;
    flagsNextPrev |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	NextPrevDisappear -= dt;
	if (NextPrevDisappear > 0.0)
	{
		ImGui::SetNextWindowPos(ImVec2(0, float(topUIHeight) + float(workAreaH)*0.5f - 33.0f));
		ImGui::SetNextWindowSize(ImVec2(18, 72), ImGuiCond_Always);
		ImGui::Begin("Prev", nullptr, flagsNextPrev);
		ImGui::SetCursorPos(ImVec2(4, 2));
		if (ImGui::ImageButton(ImTextureID(uint64(PrevImage.GLTextureID)), ImVec2(12,56), ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
			OnPrevious();
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(workAreaW-32.0f, float(topUIHeight) + float(workAreaH)*0.5f - 33.0f));
		ImGui::SetNextWindowSize(ImVec2(18, 72), ImGuiCond_Always);
		ImGui::Begin("Next", nullptr, flagsNextPrev);
		ImGui::SetCursorPos(ImVec2(4, 2));
		if (ImGui::ImageButton(ImTextureID(uint64(NextImage.GLTextureID)), ImVec2(12,56), ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
			OnNext();
		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0));

	if (!FullscreenMode)
	{
		ImGui::BeginMainMenuBar();
		{
			if (ImGui::BeginMenu("File"))
			{
				// Show file menu items...
				if (ImGui::MenuItem("Save As..."))
				{
				}
				ImGui::Separator();
			    if (ImGui::MenuItem("Quit", "Alt+F4"))
					glfwSetWindowShouldClose(Window, 1);

				ImGui::EndMenu();
			}

			static bool contactDialog = false;
			bool justOpenedContactDialog = false;
			if (ImGui::BeginMenu("Edit"))
			{
				// Show file menu items...
				if (ImGui::MenuItem("Contact Sheet..."))
				{
					contactDialog = !contactDialog;
					if (contactDialog)
						justOpenedContactDialog = true;
				}
				ImGui::EndMenu();
			}
			if (contactDialog)
				ShowContactSheetDialog(&contactDialog, justOpenedContactDialog);

			tFileType fileType = tFileType::Unknown;
			if (CurrImage && CurrImage->IsLoaded())
				fileType = CurrImage->Filetype;

			if (fileType != tFileType::Unknown)
			{
				static bool rleCompression = false;

				tString tgaFile = CurrImage->Filename;
				tgaFile.ExtractLastWord('.');
				tgaFile += "_Saved.tga";
				if (ImGui::Button("Save TGA"))
				{
					if (tFileExists(tgaFile))
					{
						tPrintf("Targa %s already exists. Will not overwrite.\n", tgaFile.ConstText());
					}
					else
					{
						bool success = CurrImage->PictureImages.First()->SaveTGA(tgaFile, tImage::tFileTGA::tFormat::Auto, rleCompression ? tImage::tFileTGA::tCompression::RLE : tImage::tFileTGA::tCompression::None);
						if (success)
							tPrintf("Saved tga as : %s\n", tgaFile.ConstText());
						else
							tPrintf("Failed to save tga %s\n", tgaFile.ConstText());
					}
				}

				ImGui::Checkbox("RLE", &rleCompression);
			}

			ImGui::PushItemWidth(200);
			if (ImGui::SliderFloat("", &ZoomPercent, 20.0f, 2500.0f, " Zoom %.2f"))
				FitMode = false;

			ImGui::PopItemWidth();

			if (ImGui::Button("Reset"))
			{
				FitMode = true;
				PanOffsetX = 0;
				PanOffsetY = 0;
				DragDownOffsetX = 0;
				DragDownOffsetY = 0;
			}

			if (ImGui::Button("1:1"))
			{
				FitMode = false;
				ZoomPercent = 100.0f;
				PanOffsetX = 0;
				PanOffsetY = 0;
				DragDownOffsetX = 0;
				DragDownOffsetY = 0;
			}

			if (ImGui::Checkbox("Fit", &FitMode) && FitMode)
			{
				PanOffsetX = 0;
				PanOffsetY = 0;
				DragDownOffsetX = 0;
				DragDownOffsetY = 0;
			}

			int colourFlags = 0;		// ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions;
			tColourf floatCol(PixelColour);
			ImGui::PushItemWidth(180);
			ImGui::ColorEdit4("Colour##2f", floatCol.E, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | colourFlags);
			ImGui::PopItemWidth();
			ImGui::Text("PXL(%d,%d)",imgxi, imgyi);
		}

		ImGui::EndMainMenuBar();
	}

	if (!FullscreenMode)
		TexView::DrawTextureViewerLog(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

	ImGui::Render();
	glViewport(0, 0, dispw, disph);
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);

	// Consider unloading some images to keep memory down. The numToCheck roughly means it will take
	// 3 seconds to check all the files assuming a 60Hz update frequency.
	int numToCheck = tMath::tGetClampMin( Images.NumItems() / (3*60), 1 );
	for (int c = 0; c < numToCheck; c++)
	{
		if ((TacitImage::GetNumLoaded() > MaxLoadedCount) && UnloadImage && (UnloadImage != CurrImage) && UnloadImage->IsLoaded())
			UnloadImage->Unload();
		UnloadImage = Images.NextCirc(UnloadImage);
	}
}


bool TexView::ChangeScreenMode(bool fullscreen)
{
	if (TexView::FullscreenMode == fullscreen)
		return false;

	// If currently in windowed mode, remember our window geometry.
	if (!FullscreenMode)
	{
		glfwGetWindowPos(TexView::Window, &TexView::Config.WindowX, &TexView::Config.WindowY);
		glfwGetWindowSize(TexView::Window, &TexView::Config.WindowW, &TexView::Config.WindowH);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	if (fullscreen)
		glfwSetWindowMonitor(TexView::Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	else
		glfwSetWindowMonitor(TexView::Window, nullptr, TexView::Config.WindowX, TexView::Config.WindowY, TexView::Config.WindowW, TexView::Config.WindowH, mode->refreshRate);

	FullscreenMode = fullscreen;
}


void TexView::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if ((action != GLFW_PRESS) && (action != GLFW_REPEAT))
		return;

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	NextPrevDisappear = 4.0;
	switch (key)
	{
		case GLFW_KEY_LEFT:
			OnPrevious();
			break;

		case GLFW_KEY_RIGHT:
			OnNext();
			break;

		// Ctrl +
		case GLFW_KEY_EQUAL:
			if (modifiers == GLFW_MOD_CONTROL)
			{
				FitMode = false;
				ZoomPercent = tMath::tFloor(ZoomPercent*0.1f + 1.0f) * 10.0f;
				tMath::tClamp(ZoomPercent, 20.0f, 2500.0f);
			}
			break;

		// Ctrl -
		case GLFW_KEY_MINUS:
			if (modifiers == GLFW_MOD_CONTROL)
			{
				FitMode = false;
				ZoomPercent = tMath::tCeiling(ZoomPercent*0.1f - 1.0f) * 10.0f;
				tMath::tClamp(ZoomPercent, 20.0f, 2500.0f);
			}
			break;

		case GLFW_KEY_ENTER:
			if (modifiers == GLFW_MOD_ALT)
				ChangeScreenMode(!FullscreenMode);
			break;

		case GLFW_KEY_ESCAPE:
			ChangeScreenMode(false);
			break;
	}
}


void TexView::MouseButtonCallback(GLFWwindow* window, int mouseButton, int press, int mods)
{
	NextPrevDisappear = 4.0;
	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		return;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	double topUIHeight = 22.0;
	if (ypos <= topUIHeight)
		return;

	bool down = press ? true : false;
	switch (mouseButton)
	{
		// Left mouse button.
		case 0:
		{
			if (down)
			{
				CursorX = int(xpos);
				CursorY = int(ypos);
			}
			break;
		}

		// Right mouse button.
		case 1:
		{
			RMBDown = down;
			if (RMBDown)
			{
				DragAnchorX = int(xpos);
				DragAnchorY = int(ypos);
				PanOffsetX += DragDownOffsetX;
				PanOffsetY += DragDownOffsetY;
				DragDownOffsetX = 0;
				DragDownOffsetY = 0;
			}
			break;
		}
	}
}


void TexView::CursorPosCallback(GLFWwindow* window, double x, double y)
{
	NextPrevDisappear = 4.0;
	// tPrintf("Cursor: %f %f\n", x, y);
}


void TexView::ScrollWheelCallback(GLFWwindow* window, double x, double y)
{
	NextPrevDisappear = 4.0;

	FitMode = false;
	float zoomDelta = ZoomPercent * 0.1f * float(y);
	ZoomPercent += zoomDelta;

	ZoomPercent = tMath::tRound(ZoomPercent*10.0f) / 10.0f;
	tMath::tClamp(ZoomPercent, 20.0f, 2500.0f);
}


void TexView::FileDropCallback(GLFWwindow* window, int count, const char** files)
{
	if (count < 1)
		return;

	tString file = tString(files[0]);
	tString path = tSystem::tGetDir(file);

	ImageFileParam.Param = file;
	Images.Clear();
	FindTextureFiles();
	SetCurrentImage(file);
}


int main(int argc, char** argv)
{
	tSystem::tSetStdoutRedirectCallback(TexView::PrintRedirectCallback);	
	tCommand::tParse(argc, argv);

	// Setup window
	glfwSetErrorCallback(TexView::GlfwErrorCallback);
	if (!glfwInit())
		return 1;

	int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);

	tPrintf("Tacit Viewer V %d.%d.%d\n", TexView::MajorVersion, TexView::MinorVersion, TexView::Revision);
	tPrintf("Tacent Library V %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear ImGui V %s\n", IMGUI_VERSION);
	tPrintf("GLEW V %s\n", glewGetString(GLEW_VERSION));
	tPrintf("GLFW V %d.%d.%d\n", glfwMajor, glfwMinor, glfwRev);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	tString cfgFile = tSystem::tGetProgramDir() + "Data/Settings.cfg";
	TexView::Config.Load(cfgFile, mode->width, mode->height);
	TexView::Window = glfwCreateWindow(TexView::Config.WindowW, TexView::Config.WindowH, "Tacit Viewer", nullptr, nullptr);
	if (!TexView::Window)
		return 1;

	glfwSetWindowPos(TexView::Window, TexView::Config.WindowX, TexView::Config.WindowY);

	// Make the window title bar show up in black.
	HWND hwnd = glfwGetWin32Window(TexView::Window);
	const int DWMWA_USE_IMMERSIVE_DARK_MODE = 19;
	BOOL isDarkMode = 1;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &isDarkMode, sizeof(isDarkMode));
	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(hwnd, SW_SHOW);

	glfwMakeContextCurrent(TexView::Window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetWindowRefreshCallback(TexView::Window, TexView::WindowRefreshFun);
	glfwSetKeyCallback(TexView::Window, TexView::KeyCallback);
	glfwSetMouseButtonCallback(TexView::Window, TexView::MouseButtonCallback);
	glfwSetCursorPosCallback(TexView::Window, TexView::CursorPosCallback);
	glfwSetScrollCallback(TexView::Window, TexView::ScrollWheelCallback);
	glfwSetDropCallback(TexView::Window, TexView::FileDropCallback);

	GLenum err = glewInit();
	if (err != GLEW_OK)
		return err;

	// Setup Dear ImGui context.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.IniFilename = nullptr;
	io.ConfigFlags = 0;
	// io.NavActive = false;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// Setup Dear ImGui style.
	ImGui::StyleColorsDark();

	// Setup platform/renderer bindings.
	ImGui_ImplGlfw_InitForOpenGL(TexView::Window, true);
	ImGui_ImplOpenGL2_Init();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tString fontFile = tSystem::tGetProgramDir() + "Data/Roboto-Medium.ttf";
	if (tFileExists(fontFile))
		io.Fonts->AddFontFromFileTTF(fontFile.ConstText(), 14.0f);

	tString cursorFile = tSystem::tGetProgramDir() + "Data/PixelCursor.png";
	TexView::CursorImage.Load(cursorFile);
	TexView::CursorImage.Bind();

	tString prevFile = tSystem::tGetProgramDir() + "Data/PrevArrow.png";
	TexView::PrevImage.Load(prevFile);
	TexView::PrevImage.Bind();

	tString nextFile = tSystem::tGetProgramDir() + "Data/NextArrow.png";
	TexView::NextImage.Load(nextFile);
	TexView::NextImage.Bind();

	TexView::FindTextureFiles();
	if (ImageFileParam.IsPresent())
		TexView::SetCurrentImage(ImageFileParam.Get());
	else
		TexView::SetCurrentImage();

	// Main loop.
	static double lastUpdateTime = glfwGetTime();
	while (!glfwWindowShouldClose(TexView::Window))
	{
		double currUpdateTime = glfwGetTime();
		TexView::Update(TexView::Window, currUpdateTime - lastUpdateTime);
		lastUpdateTime = currUpdateTime;
	}

	// Save current window geometry to config file.
	glfwGetWindowPos(TexView::Window, &TexView::Config.WindowX, &TexView::Config.WindowY);
	glfwGetWindowSize(TexView::Window, &TexView::Config.WindowW, &TexView::Config.WindowH);
	TexView::Config.Save(cfgFile);

	// Cleanup.
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(TexView::Window);
	glfwTerminate();
	return 0;
}
