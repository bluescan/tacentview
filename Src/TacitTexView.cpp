// TextureViewer.cpp
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
#include "TextureViewer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
using namespace tStd;


tCommand::tOption PrintAllOutput("Print all output.", 'a', "all");


// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
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
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}
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
		ImGui::End();
	}
};


static bool gLogOpen = true;
static TextureViewerLog gLog;


void ShowTextureViewerLog()
{
	// For the demo: add a debug button before the normal log window contents
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
//	ImGui::Begin("Log", &gLogOpen);
	/*
	if (ImGui::SmallButton("Add 5 entries"))
	{
		static int counter = 0;
		for (int n = 0; n < 5; n++)
		{
			const char* categories[3] = { "info", "warn", "error" };
			const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
			gLog.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
				ImGui::GetFrameCount(), categories[counter % IM_ARRAYSIZE(categories)], ImGui::GetTime(), words[counter % IM_ARRAYSIZE(words)]);
			counter++;
		}
	}
	*/
//	ImGui::End();

	gLog.Draw("Log", &gLogOpen);
}



namespace tUnitTest
{
	int UnitRequirementNumber = 0;
	int UnitGoalNumber = 0;
	int UnitsSkipped = 0;
	int TotalRequirements = 0;
	int RequirementsPassed = 0;
	int TotalGoals = 0;
	int GoalsPassed = 0;
}

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h> 

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma. 
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
//#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
//#pragma comment(lib, "legacy_stdio_definitions")
//#endif


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

	tPrintf("Loading Image: %s\n", gCurrFile->ConstText());
	gPicture.Load(*gCurrFile);

	glBindTexture(GL_TEXTURE_2D, tex);
	tPrintf("Width: %d Height: %d\n", gPicture.GetWidth(), gPicture.GetHeight());
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gPicture.GetWidth(), gPicture.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gPicture.GetWidth(), gPicture.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gPicture.GetWidth(), gPicture.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());

	glBindTexture(GL_TEXTURE_2D, 0);
}

void LoadTextureFromDisk()
{
	tString currentDir = tSystem::tGetCurrentDir();
	tString imagesDir = currentDir + "TestData/";

	tPrintf("Looking for image files in %s\n", imagesDir.ConstText());
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.jpg");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.gif");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.tga");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.png");
	tSystem::tFindFilesInDir(gFoundFiles, imagesDir, "*.tiff");
	//for (tStringItem* image = foundFiles.First(); image; image = image->Next())
	gCurrFile = gFoundFiles.First();
	if (gCurrFile)
	{
		tPrintf("Loading Image: %s\n", gCurrFile->ConstText());
		gPicture.Load(*gCurrFile);

		//upload to GPU texture
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 8, 8, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texDat);
		tPrintf("Width: %d Height: %d Opaque: %s\n", gPicture.GetWidth(), gPicture.GetHeight(), gPicture.IsOpaque() ? "true" : "false");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gPicture.GetWidth(), gPicture.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gPicture.GetWidth(), ,
		//	GL_RGBA, GL_UNSIGNED_BYTE, gPicture.GetPixelPointer());

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//if (!tSystem::tDirExists("TestData/"))

		// Test dxt1 texture.
	//	tImage::tTexture dxt1Tex("TestData/TestDXT1.dds");

	// Test tPicture loading jpg and saving as tga.
	///tImage::tPicture jpgPic("TestData/WiredDrives.jpg");
}


int main(int, char**)
{
	tSystem::tSetStdoutRedirectCallback(PrintRedirectCallback);	

	tPrintf("Tacent Texture Viewer\n");
	tPrintf("Tacent Version %d.%d.%d\n", tVersion::Major, tVersion::Minor, tVersion::Revision);
	tPrintf("Dear IMGUI Version %s (%d)\n", IMGUI_VERSION, IMGUI_VERSION_NUM);

	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Tacent Texture Viewer", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

	io.Fonts->AddFontFromFileTTF("Data/Roboto-Medium.ttf", 14.0f);

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

	LoadTextureFromDisk();

	//match projection to window resolution (could be in reshape callback)
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, 1280, 0, 720, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
		
		ImGui_ImplGlfw_NewFrame();

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, display_w, 0, display_h, -1, 1);
		glMatrixMode(GL_MODELVIEW);




		//clear and draw quad with texture (could be in display callback)
		if (gPicture.IsValid())
		{
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);

			int w = gPicture.GetWidth();
			int h = gPicture.GetHeight();
			glTexCoord2i(0, 0); glVertex2i(10+0, 10+0);
			glTexCoord2i(0, 1); glVertex2i(10+0, 10+h);
			glTexCoord2i(1, 1); glVertex2i(10+w, 10+h);
			glTexCoord2i(1, 0); glVertex2i(10+w, 10+0);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFlush(); //don't need this with GLUT_DOUBLE and glutSwapBuffers
		}


        ImGui::NewFrame();

		 if (ImGui::Button("Next"))
		 {
			 if (gCurrFile && gCurrFile->Next())
			 {
				 gCurrFile = gCurrFile->Next();
				 LoadCurrFile();
			 }
		 }
		 if (ImGui::Button("Prev"))
		 {
			 if (gCurrFile && gCurrFile->Prev())
			 {
				 gCurrFile = gCurrFile->Prev();
				 LoadCurrFile();
			 }
		 }
		//tPrintf("Logging...\n");
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		ShowTextureViewerLog();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
   //     glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
   //     glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
	}

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



