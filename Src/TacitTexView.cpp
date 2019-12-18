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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "TacitImage.h"
#include "ImGuiLogWindow.h"
using namespace tStd;
using namespace tSystem;


tCommand::tParam ImageFileParam(1, "ImageFile", "File to open.");


namespace TexView
{
	int MajorVersion			= 0;
	int MinorVersion			= 9;
	int Revision				= 0;
	bool LogWindowOpen			= true;
	TexView::ImGuiLog LogWindow;
	tList<TacitImage> Images;
	TacitImage* CurrImage		= nullptr;
	GLFWwindow* Window			= nullptr;
	bool LMBDown				= false;
	int DragAnchorX				= 0;
	int DragAnchorY				= 0;
	float ZoomPercent			= 100.0f;
	bool RecomputeZoom			= false;
	int Dispw					= 1;
	int Disph					= 1;
	int PanOffsetX				= 0;
	int PanOffsetY				= 0;
	int DragDownOffsetX			= 0;
	int DragDownOffsetY			= 0;

	void DrawTextureViewerLog(float x, float y, float w, float h);
	void PrintRedirectCallback(const char* text, int numChars)															{ LogWindow.AddLog("%s", text); }
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }
	bool CompareFunc(const tStringItem& a, const tStringItem& b)														{ return tStrcmp(a.ConstText(), b.ConstText()) < 0; }
	void FindTextureFiles();
	void SetCurrentImage(const tString& currFilename = tString());

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	void ShowHelpMark(const char* desc);
	void ShowContactSheetDialog(bool* popen);
	void SetWindowTitle();
	bool OnPrevious();
	bool OnNext();
	void DoFrame(GLFWwindow* window, bool dopoll = true);
	void WindowRefreshFun(GLFWwindow* window)																			{ DoFrame(window, false); }
	void KeyCallback(GLFWwindow*, int key, int scancode, int action, int modifiers);
	void MouseButtonCallback(GLFWwindow*, int mouseButton, int x, int y);
	void CursorPosCallback(GLFWwindow*, double x, double y);
}


void TexView::DrawTextureViewerLog(float x, float y, float w, float h)
{
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);

	ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
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
			break;
		}
	}

	if (!CurrImage)
		CurrImage = Images.First();

	if (CurrImage)
	{
		CurrImage->Load();
		CurrImage->PrintInfo();
		SetWindowTitle();
		RecomputeZoom = true;
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
	CurrImage->Load();
	CurrImage->PrintInfo();
	SetWindowTitle();
	RecomputeZoom = true;
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
	CurrImage->Load();
	CurrImage->PrintInfo();
	SetWindowTitle();
	RecomputeZoom = true;
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


void TexView::ShowContactSheetDialog(bool* popen)
{
	ImGuiWindowFlags windowFlags = 0;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(380, 220), ImGuiCond_FirstUseEver);

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Contact Sheet Generator", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	static int contactWidth = 1024;
	static int contactHeight = 1024;
	static int numRows = 4;
	static int numColumns = 4;

	ImGui::InputInt("Width", &contactWidth);
	ImGui::SameLine();
	ShowHelpMark("Contact sheet width in pixels.");

	ImGui::InputInt("Height", &contactHeight);
	ImGui::SameLine();
	ShowHelpMark("Contact sheet height in pixels.");

	ImGui::InputInt("Columns", &numColumns);
	ImGui::SameLine();
	ShowHelpMark("Number of columns. The width must be divisible by this number.");
 
	ImGui::InputInt("Rows", &numRows);
	ImGui::SameLine();
	ShowHelpMark("Number of rows. The height must be divisible by this number.");

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

	bool canGenerate = true;
	if ((contactWidth <= 0) || (contactHeight <= 0) || (numRows <= 0) || (numColumns <= 0))
	{
		ImGui::Text("Error: Width, Height, Rows, and Columns must all be bigger than 0.");
		canGenerate = false;
	}

	if (canGenerate)
	{
		if (contactWidth % numColumns)
		{
			ImGui::Text("Error: Width must be a multiple of Columns.");
			canGenerate = false;
		}
		else
		{
			ImGui::Text("Each frame will have width %d.", contactWidth / numColumns);
		}
		if (contactHeight % numRows)
		{
			ImGui::Text("Error: Height must be a multiple of Rows.");
			canGenerate = false;
		}
		else
		{
			ImGui::Text("Each frame will have height %d.", contactHeight / numRows);
		}
	}

	if (canGenerate)
	{
		if (ImGui::Button("Generate"))
		{
			tString imagesDir = tSystem::tGetCurrentDir();
			if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
				imagesDir = tSystem::tGetDir(ImageFileParam.Get());
			tString outFile = imagesDir + tString(filename) + extension;
			tImage::tPicture outPic(contactWidth, contactHeight);
			outPic.SetAll(tColouri(0, 0, 0, 0));

			// Do the work.
			int frameWidth = contactWidth / numColumns;
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

				if (img->IsLoaded() && img->PictureImage.IsValid() && !img->PictureImage.IsOpaque())
					allOpaque = false;
			}

			TacitImage* currImg = Images.First();
			while (currImg)
			{
				if (!currImg->PictureImage.IsValid())
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

				tImage::tPicture resampled(currImg->PictureImage);
				resampled.Resample(frameWidth, frameHeight);

				// Copy resampled frame into place.
				for (int y = 0; y < frameHeight; y++)
					for (int x = 0; x < frameWidth; x++)
						outPic.SetPixel(x + (ix*frameWidth), y + ((numRows-1-iy)*frameHeight), resampled.GetPixel(x, y));						

				currImg = currImg->Next();

				ix++;
				if (ix >= numColumns)
				{
					ix = 0;
					iy++;
					if (iy >= numRows)
						break;
				}
			}

			tImage::tPicture::tColourFormat colourFmt = allOpaque ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
			outPic.Save(outFile, colourFmt);
			Images.Clear();
			FindTextureFiles();
			SetCurrentImage(outFile);
		}
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


void TexView::DoFrame(GLFWwindow* window, bool dopoll)
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
			RecomputeZoom = true;
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
					glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				else
					glColor4f(0.8f, 0.7f, 0.6f, 1.0f);
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

		if (RecomputeZoom)
		{
			ZoomPercent = 100.0f;
			if (draww < iw)
				ZoomPercent = 100.0f * draww / iw;
			RecomputeZoom = false;
		}

		float w = iw * ZoomPercent/100.0f;
		float h = ih * ZoomPercent/100.0f;

		// If the image is smaller than the drawable area we draw a quad of the correct size with full 0..1 range in the uvs.
		if (w < draww)
		{
			float offsetW = (draww - w) / 2.0f;
			l += offsetW;
			r -= offsetW;
			float offsetH = (drawh - h) / 2.0f;
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

		// WIP. Modify the UVs here to magnify.
		if (draww < w)
		{
			if (LMBDown)
			{
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				DragDownOffsetX = int(xpos) - DragAnchorX;
				DragDownOffsetY = int(ypos) - DragAnchorY;
			}

			if (float(PanOffsetX+DragDownOffsetX) > (w-draww)/2.0f)
				DragDownOffsetX = int((w-draww)/2.0f) - PanOffsetX;
			if (float(PanOffsetX+DragDownOffsetX) < -(w-draww)/2.0f)
				DragDownOffsetX = int(-(w-draww)/2.0f) - PanOffsetX;

			if (float(PanOffsetY+DragDownOffsetY) > (h-drawh)/2.0f)
				DragDownOffsetY = int((h-drawh)/2.0f) - PanOffsetY;
			if (float(PanOffsetY+DragDownOffsetY) < -(h-drawh)/2.0f)
				DragDownOffsetY = int(-(h-drawh)/2.0f) - PanOffsetY;
		}

		float uOffset = -float(PanOffsetX+DragDownOffsetX)/w;
		float vOffset =  float(PanOffsetY+DragDownOffsetY)/h;
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f + uvHMargin + uOffset, 0.0f + uvVMargin + vOffset); glVertex2f(l, b);
		glTexCoord2f(0.0f + uvHMargin + uOffset, 1.0f - uvVMargin + vOffset); glVertex2f(l, t);
		glTexCoord2f(1.0f - uvHMargin + uOffset, 1.0f - uvVMargin + vOffset); glVertex2f(r, t);
		glTexCoord2f(1.0f - uvHMargin + uOffset, 0.0f + uvVMargin + vOffset); glVertex2f(r, b);
		glEnd();
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
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	ImGui::BeginMainMenuBar();
	{
		if (ImGui::Button("Prev"))
			OnPrevious();

		if (ImGui::Button("Next"))
			OnNext();

		static bool contactDialog = false;
		if (ImGui::Button("Contact Sheet"))
			contactDialog = !contactDialog;

		if (contactDialog)
			ShowContactSheetDialog(&contactDialog);

		tFileType fileType = tFileType::Unknown;
		if (CurrImage && CurrImage->IsLoaded())
			fileType = CurrImage->Filetype;

		if ((fileType != tFileType::Unknown) && (fileType != tFileType::Targa) && (fileType != tFileType::DDS))
		{
			tString tgaFile = CurrImage->Filename;
			tgaFile.ExtractLastWord('.');
			tgaFile += ".tga";
			if (ImGui::Button("Save As Targa"))
			{
				if (tFileExists(tgaFile))
				{
					tPrintf("Targa %s already exists. Will not overwrite.\n", tgaFile.ConstText());
				}
				else
				{
					bool success = CurrImage->PictureImage.SaveTGA(tgaFile, tImage::tFileTGA::tFormat::Auto, tImage::tFileTGA::tCompression::None);
					if (success)
						tPrintf("Saved targa as : %s\n", tgaFile.ConstText());
					else
						tPrintf("Failed to save targa %s\n", tgaFile.ConstText());
				}
			}
		}
		ImGui::PushItemWidth(200);
		ImGui::SliderFloat("", &ZoomPercent, 20.0f, 2000.0f, " Zoom %.2f");
		ImGui::PopItemWidth();

		float zp = 100.0f;
		if (draww < iw)
			zp = 100.0f * draww / iw;
		if ((zp != ZoomPercent) && ImGui::Button("Reset"))
		{
			RecomputeZoom = true;
			PanOffsetX = 0;
			PanOffsetY = 0;
			DragDownOffsetX = 0;
			DragDownOffsetY = 0;
		}

		if ((ZoomPercent != 100.0f) && ImGui::Button("100%"))
		{
			RecomputeZoom = false;
			ZoomPercent = 100.0f;
			PanOffsetX = 0;
			PanOffsetY = 0;
			DragDownOffsetX = 0;
			DragDownOffsetY = 0;
		}
	}

	ImGui::EndMainMenuBar();
	TexView::DrawTextureViewerLog(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

	ImGui::Render();
	glViewport(0, 0, dispw, disph);
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
}


void TexView::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if ((action != GLFW_PRESS) && (action != GLFW_REPEAT))
		return;

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	switch (key)
	{
		case GLFW_KEY_LEFT:
			OnPrevious();
			break;

		case GLFW_KEY_RIGHT:
			OnNext();
			break;

		case GLFW_KEY_EQUAL:
			if (modifiers == 2)
			{
				ZoomPercent = tMath::tFloor(ZoomPercent*0.1f + 1.0f) * 10.0f;
				tMath::tClamp(ZoomPercent, 20.0f, 2000.0f);
			}
			break;

		case GLFW_KEY_MINUS:
			if (modifiers == 2)
			{
				ZoomPercent = tMath::tCeiling(ZoomPercent*0.1f - 1.0f) * 10.0f;
				tMath::tClamp(ZoomPercent, 20.0f, 2000.0f);
			}
			break;
	}
}


void TexView::MouseButtonCallback(GLFWwindow* window, int mouseButton, int press, int mods)
{
	if (mouseButton != 1)
		return;

	LMBDown = press ? true : false;
	if (LMBDown)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		DragAnchorX = int(xpos);
		DragAnchorY = int(ypos);
		PanOffsetX += DragDownOffsetX;
		PanOffsetY += DragDownOffsetY;
		DragDownOffsetX = 0;
		DragDownOffsetY = 0;
	}
}


void TexView::CursorPosCallback(GLFWwindow* window, double x, double y)
{
	// tPrintf("Cursor: %f %f\n", x, y);
}


int main(int argc, char** argv)
{
	tSystem::tSetStdoutRedirectCallback(TexView::PrintRedirectCallback);	

	tPrintf("Tacit Texture Viewer Version %d.%d.%d\n", TexView::MajorVersion, TexView::MinorVersion, TexView::Revision);
	tPrintf("Tacent Version %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear IMGUI Version %s (%d)\n", IMGUI_VERSION, IMGUI_VERSION_NUM);
	tPrintf("GLEW Version %s\n", glewGetString(GLEW_VERSION));
	tCommand::tParse(argc, argv);

	// Setup window
	glfwSetErrorCallback(TexView::GlfwErrorCallback);
	if (!glfwInit())
		return 1;

	TexView::Window = glfwCreateWindow(1280, 720, "Tacit Viewer", NULL, NULL);
	if (!TexView::Window)
		return 1;

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

	TexView::FindTextureFiles();
	if (ImageFileParam.IsPresent())
		TexView::SetCurrentImage(ImageFileParam.Get());
	else
		TexView::SetCurrentImage();

	// Main loop.
	while (!glfwWindowShouldClose(TexView::Window))
		TexView::DoFrame(TexView::Window);

	// Cleanup.
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(TexView::Window);
	glfwTerminate();
	return 0;
}
