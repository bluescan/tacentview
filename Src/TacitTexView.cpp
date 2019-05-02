// TacitTexView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2017, 2019 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tVersion.h>
#include <System/tCommand.h>
#include <Image/tTexture.h>
#include <System/tFile.h>
#include "TacitTexView.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h> 
using namespace tStd;
using namespace tSystem;


tCommand::tOption PrintAllOutput("Print all output.", 'a', "all");
tCommand::tParam ImageFileParam(1, "ImageFile", "File to open.");


struct TextureViewerLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
	bool                ScrollToBottom;
	TextureViewerLog() : ScrollToBottom(true)
	{
		Clear();
	}

	void    Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
		ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
//		if (!ImGui::Begin(title, p_open))
//		{
//			ImGui::End();
//			return;
//		}
		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive())
		{
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//ImGui::TextUnformatted(buf, buf_end); 
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
			// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
			// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height, 
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
			// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (ScrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::EndChild();
		//ImGui::End();
	}
};


static bool gLogOpen = true;
static TextureViewerLog gLog;


void ShowTextureViewerLog(float x, float y, float w, float h)
{
	// For the demo: add a debug button before the normal log window contents
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);

	ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	//ImGui::SetCursorPosX(500.0f);
	//ImGui::SetCursorPosY(0.0f);
	gLog.Draw("Log", &gLogOpen);
	ImGui::End();
}


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


void PrintRedirectCallback(const char* text, int numChars)
{
	gLog.AddLog("%s", text);
}


tImage::tPicture gPicture;
GLuint tex = 0;
tList<tStringItem> gFoundFiles;
tStringItem* gCurrFile = nullptr;

void LoadCurrFile()
{
	if (!gCurrFile)
		return;

	bool success = gPicture.Load(*gCurrFile);
	if (!success)
	{
		tPrintf("Cannot Load Image: %s\n", tSystem::tGetFileName(*gCurrFile).ConstText());
		return;
	}

	tPrintf
	(
		"Image: %s Width: %d Height: %d Opaque: %s\n",
		tSystem::tGetFileName(*gCurrFile).ConstText(),
		gPicture.GetWidth(), gPicture.GetHeight(),
		gPicture.IsOpaque() ? "True" : "False"
	);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gPicture.GetWidth(), gPicture.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool CompareFunc(const tStringItem& a, const tStringItem& b)
{
	return tStrcmp(a.ConstText(), b.ConstText()) < 0;
}


void FindTextureFiles()
{
	// If an absolute full path is specified I think we should only open that one file.
	if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
	{
		gCurrFile = new tStringItem(ImageFileParam.Get());
		gFoundFiles.Append(gCurrFile);
		return;
	}

	tString currentDir = tSystem::tGetCurrentDir();
	tString imagesDir = currentDir; // +"Textures/";

	tPrintf("Looking for image files in %s\n", imagesDir.ConstText());
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.jpg");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.gif");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.tga");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.png");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.tiff");
//	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.dds");

	gFoundFiles.Sort(CompareFunc, tListSortAlgorithm::Merge);

	gCurrFile = gFoundFiles.First();

	if (ImageFileParam.IsPresent())
	{
		for (tStringItem* si = gFoundFiles.First(); si; si = si->Next())
		{
			tString siName = tSystem::tGetFileName(*si);
			tString imgName = tSystem::tGetFileName(ImageFileParam.Get());

			if (tStricmp(siName.ConstText(), imgName.ConstText()) == 0)
			{
				gCurrFile = si;
				break;
			}
		}
	}

	glGenTextures(1, &tex);
}


void DoFrame(GLFWwindow* window, bool dopoll = true)
{
	// Poll and handle events (inputs, window resize, etc.)
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	if (dopoll)
		glfwPollEvents();

	ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	int bottomUIHeight = 150;
	int topUIHeight = 20;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();		
	ImGui_ImplGlfw_NewFrame();

	int dispw, disph;
	glfwGetFramebufferSize(window, &dispw, &disph);

	int workAreaW = dispw;
	int workAreaH = disph - bottomUIHeight - topUIHeight;
	float workAreaAspect = float(workAreaW)/float(workAreaH);

	glViewport(0, bottomUIHeight, workAreaW, workAreaH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, workAreaW, 0, workAreaH, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	//clear and draw quad with texture (could be in display callback)
	if (gPicture.IsValid())
	{
		int w = gPicture.GetWidth();
		int h = gPicture.GetHeight();
		float picaspect = float(w)/float(h);

		float drawh = 0.0f;
		float draww = 0.0f;
		float hmargin = 0.0f;
		float vmargin = 0.0f;
		if (workAreaAspect > picaspect)
		{
			drawh = float(workAreaH);
			draww = picaspect * drawh;
			hmargin = (workAreaW - draww) * 0.5f;
			vmargin = 0.0f;;
		}
		else
		{
			draww = float(workAreaW);
			drawh = draww / picaspect;
			vmargin = (workAreaH - drawh) * 0.5f;
			hmargin = 0.0f;
		}

		// Semitransparent checker BG.

		int x = 0;
		int y = 0;
		bool lineStartToggle = false;
		float checkSize = 25.0f;
		while (y*checkSize < drawh)
		{
			bool colourToggle = lineStartToggle;

			while (x*checkSize < draww)
			{
				if (colourToggle)
					glColor4f(1.0f,1.0f,1.0f,1.0f);
				else
					glColor4f(0.8f,0.7f,0.6f,1.0f);
				colourToggle = !colourToggle;

				float w = checkSize;
				if ((x+1)*checkSize > draww)
					w -= (x+1)*checkSize - draww;

				float h = checkSize;
				if ((y+1)*checkSize > drawh)
					h -= (y+1)*checkSize - drawh;

				glBegin(GL_QUADS);
				glVertex2f(hmargin+x*checkSize, vmargin+y*checkSize);
				glVertex2f(hmargin+x*checkSize, vmargin+y*checkSize+h);
				glVertex2f(hmargin+x*checkSize+w, vmargin+y*checkSize+h);
				glVertex2f(hmargin+x*checkSize+w, vmargin+y*checkSize);
				glEnd();

				x++;
			}
			x = 0;
			y++;
			lineStartToggle = !lineStartToggle;
		}

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		glBindTexture(GL_TEXTURE_2D, tex);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);

		glTexCoord2i(0, 0); glVertex2f(hmargin, vmargin);
		glTexCoord2i(0, 1); glVertex2f(hmargin, vmargin+drawh);
		glTexCoord2i(1, 1); glVertex2f(hmargin+draww, vmargin+drawh);
		glTexCoord2i(1, 0); glVertex2f(hmargin+draww, vmargin);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFlush(); //don't need this with GLUT_DOUBLE and glutSwapBuffers
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, dispw, 0, disph, -1, 1);
	glMatrixMode(GL_MODELVIEW);

    ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	bool show_demo_window = false;
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::BeginMainMenuBar();
	{
		if (ImGui::Button("Prev"))
		{
			if (gCurrFile && gCurrFile->Prev())
			{
				gCurrFile = gCurrFile->Prev();
				LoadCurrFile();
			}
		}
		if (ImGui::Button("Next"))
		{
			if (gCurrFile && gCurrFile->Next())
			{
				gCurrFile = gCurrFile->Next();
				LoadCurrFile();
			}
		}

		tFileType fileType = tFileType::Unknown;
		if (gCurrFile && gPicture.IsValid())
			fileType = tGetFileType(*gCurrFile);

		if ((fileType != tFileType::Unknown) && (fileType != tFileType::Targa))
		{
			tString tgaFile = *gCurrFile;
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
					bool success = gPicture.SaveTGA(tgaFile, tImage::tFileTGA::tFormat::Auto, tImage::tFileTGA::tCompression::None);
					if (success)
						tPrintf("Saved targa as : %s\n", tgaFile.ConstText());
					else
						tPrintf("Failed to save targa %s\n", tgaFile.ConstText());
				}
			}
		}
	}

	ImGui::EndMainMenuBar();

	ShowTextureViewerLog(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

    // Rendering
    ImGui::Render();
    glViewport(0, 0, dispw, disph);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

void Windowrefreshfun(GLFWwindow* window)
{
	DoFrame(window, false);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if (action != GLFW_PRESS)
		return;

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	switch (key)
	{
		case GLFW_KEY_LEFT:
			if (gCurrFile && gCurrFile->Prev())
			{
				gCurrFile = gCurrFile->Prev();
				LoadCurrFile();
			}
			break;

		case GLFW_KEY_RIGHT:
			if (gCurrFile && gCurrFile->Next())
			{
				gCurrFile = gCurrFile->Next();
				LoadCurrFile();
			}
			break;
	}
}

int main(int argc, char** argv)
{
	tCommand::tParse(argc, argv);

	tSystem::tSetStdoutRedirectCallback(PrintRedirectCallback);	

	tPrintf("Tacit Texture Viewer\n");
	tPrintf("Tacent Version %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear IMGUI Version %s (%d)\n", IMGUI_VERSION, IMGUI_VERSION_NUM);

	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Tacent Texture Viewer", NULL, NULL);
	if (!window)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetWindowRefreshCallback(window, Windowrefreshfun);
	glfwSetKeyCallback(window, KeyCallback);

    // Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	//io.NavActive = false;
	io.ConfigFlags = 0; // |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tString fontFile = tSystem::tGetProgramDir() + "Data/Roboto-Medium.ttf";
	io.Fonts->AddFontFromFileTTF(fontFile.ConstText(), 14.0f);

	bool show_demo_window = true;
	bool show_another_window = false;

	FindTextureFiles();
	LoadCurrFile();


	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		DoFrame(window);
	}

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
