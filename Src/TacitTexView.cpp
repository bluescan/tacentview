// TacitTexView.cpp
//
// A texture viewer for various formats.
//
// Copyright (c) 2018, 2019, 2020 Tristan Grimmer.
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
#include <Math/tHash.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "TacitTexView.h"
#include "TacitImage.h"
#include "Dialogs.h"
#include "ContactSheet.h"
#include "ImGuiLogWindow.h"
#include "Settings.h"
using namespace tStd;
using namespace tSystem;


namespace TexView
{
	tCommand::tParam ImageFileParam(1, "ImageFile", "File to open.");
	int MajorVersion			= 1;
	int MinorVersion			= 0;
	int Revision				= 4;
	Settings Config;
	bool LogWindowOpen			= true;
	TexView::ImGuiLog LogWindow;
	tList<TacitImage> Images;
	tuint256 ImagesHash			= 0;
	TacitImage* CurrImage		= nullptr;
	TacitImage CursorImage;
	TacitImage PrevImage;
	TacitImage NextImage;
	TacitImage FlipHImage;
	TacitImage FlipVImage;
	TacitImage RotateACWImage;
	TacitImage RotateCWImage;
	TacitImage FullscreenImage;
	TacitImage WindowedImage;
	TacitImage SkipBeginImage;
	TacitImage SkipEndImage;
	TacitImage MipmapsImage;
	TacitImage CubemapImage;
	TacitImage InfoOverlayImage;

	double NextPrevDisappear	= 1.0;
	GLFWwindow* Window			= nullptr;
	bool FullscreenMode			= false;
	bool WindowIconified		= false;
	bool ShowLog				= false;
	bool ShowCheatSheet			= false;
	bool ShowAbout				= false;
	bool RMBDown				= false;
	int DragAnchorX				= 0;
	int DragAnchorY				= 0;

	enum class ZoomMode
	{
		User,
		Fit,
		Downscale,
		OneToOne
	};
	ZoomMode CurrZoomMode		= ZoomMode::Downscale;

	float ZoomPercent			= 100.0f;

	int Dispw					= 1;
	int Disph					= 1;
	int PanOffsetX				= 0;
	int PanOffsetY				= 0;
	int DragDownOffsetX			= 0;
	int DragDownOffsetY			= 0;
	int CursorX					= -1;
	int CursorY					= -1;
	tColouri PixelColour		= tColouri::black;
	const int MaxLoadedCount	= 48;			// If more images that this loaded we start unloading to free mem.
	TacitImage* UnloadImage		= nullptr;

	void DrawBackground(float bgW, float bgH, float bgX, float bgY);
	void DrawTextureViewerLog(float x, float y, float w, float h);
	void PrintRedirectCallback(const char* text, int numChars)															{ LogWindow.AddLog("%s", text); }
	void GlfwErrorCallback(int error, const char* description)															{ tPrintf("Glfw Error %d: %s\n", error, description); }
	bool CompareFunc(const tStringItem& a, const tStringItem& b)														{ return tStricmp(a.ConstText(), b.ConstText()) < 0; }

	void SetWindowTitle();
	bool OnPrevious();
	bool OnNext();
	bool OnSkipBegin();
	bool OnSkipEnd();
	bool ChangeScreenMode(bool fullscreeen, bool force = false);
	void ResetPan(bool resetX = true, bool resetY = true);
	void FindImageFiles(tList<tStringItem>& foundFiles);
	tuint256 ComputeImagesHash(const tList<tStringItem>& files);
	void Update(GLFWwindow* window, double dt, bool dopoll = true);

	void WindowRefreshFun(GLFWwindow* window)																			{ Update(window, 0.0, false); }
	void KeyCallback(GLFWwindow*, int key, int scancode, int action, int modifiers);
	void MouseButtonCallback(GLFWwindow*, int mouseButton, int x, int y);
	void CursorPosCallback(GLFWwindow*, double x, double y);
	void ScrollWheelCallback(GLFWwindow*, double x, double y);
	void FileDropCallback(GLFWwindow*, int count, const char** paths);
	void FocusCallback(GLFWwindow*, int gotFocus);
	void IconifyCallback(GLFWwindow*, int iconified);
}


void TexView::DrawTextureViewerLog(float x, float y, float w, float h)
{
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);

	ImGui::Begin("InfoLog", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	LogWindow.Draw("Log", &LogWindowOpen);
	ImGui::End();
}


void TexView::FindImageFiles(tList<tStringItem>& foundFiles)
{
	tString imagesDir = tSystem::tGetCurrentDir();
	if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
		imagesDir = tSystem::tGetDir(ImageFileParam.Get());

	tPrintf("Looking for image files in %s\n", imagesDir.ConstText());
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.jpg");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.gif");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tga");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.png");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tif");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.tiff");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.bmp");
	tSystem::tFindFilesInDir(foundFiles, imagesDir, "*.dds");
	foundFiles.Sort(CompareFunc, tListSortAlgorithm::Merge);
}


tuint256 TexView::ComputeImagesHash(const tList<tStringItem>& files)
{
	tuint256 hash = 0;
	for (tStringItem* item = files.First(); item; item = item->Next())
		hash = tMath::tHashString256(item->ConstText(), hash);

	return hash;
}


void TexView::PopulateImages()
{
	tList<tStringItem> foundFiles;
	FindImageFiles(foundFiles);
	ImagesHash = ComputeImagesHash(foundFiles);

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
		CurrZoomMode = ZoomMode::Downscale;
		ResetPan();
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
	ResetPan();
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
	ResetPan();
	return true;
}


bool TexView::OnSkipBegin()
{
	if (!CurrImage || !Images.First())
		return false;

	CurrImage = Images.First();
	UnloadImage = CurrImage;
	CurrImage->Load();
	CurrImage->PrintInfo();
	SetWindowTitle();
	ResetPan();
	return true;
}


bool TexView::OnSkipEnd()
{
	if (!CurrImage || !Images.Last())
		return false;

	CurrImage = Images.Last();
	UnloadImage = CurrImage;
	CurrImage->Load();
	CurrImage->PrintInfo();
	SetWindowTitle();
	ResetPan();
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


void TexView::ShowToolTip(const char* desc)
{
	if (!ImGui::IsItemHovered())
		return;

	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(desc);
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
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


void TexView::ResetPan(bool resetX, bool resetY)
{
	if (resetX)
	{
		PanOffsetX = 0;
		DragDownOffsetX = 0;
	}

	if (resetY)
	{
		PanOffsetY = 0;
		DragDownOffsetY = 0;
	}
}


//void TexView::DrawBackground(float draww, float drawh, float hmargin, float vmargin)
void TexView::DrawBackground(float bgW, float bgH, float bgX, float bgY)
{
	switch (Config.BackgroundStyle)
	{
		case Settings::BGStyle::None:
			return;

		case Settings::BGStyle::Checkerboard:
		{
			// Semitransparent checkerboard background.
			int x = 0;
			int y = 0;
			bool lineStartToggle = false;
			float checkSize = 16.0f;
			while (y*checkSize < bgH)
			{
				bool colourToggle = lineStartToggle;

				while (x*checkSize < bgW)
				{
					if (colourToggle)
						glColor4f(0.3f, 0.3f, 0.35f, 1.0f);
					else
						glColor4f(0.4f, 0.4f, 0.45f, 1.0f);

					colourToggle = !colourToggle;

					float cw = checkSize;
					if ((x+1)*checkSize > bgW)
						cw -= (x+1)*checkSize - bgW;

					float ch = checkSize;
					if ((y+1)*checkSize > bgH)
						ch -= (y+1)*checkSize - bgH;

					float l = tMath::tRound(bgX+x*checkSize);
					float r = tMath::tRound(bgX+x*checkSize+cw);
					float b = tMath::tRound(bgY+y*checkSize);
					float t = tMath::tRound(bgY+y*checkSize+ch);

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
			break;
		}

		case Settings::BGStyle::Black:
		case Settings::BGStyle::Grey:
		case Settings::BGStyle::White:
		{
			switch (Config.BackgroundStyle)
			{
				case Settings::BGStyle::Black:	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);		break;
				case Settings::BGStyle::Grey:	glColor4f(0.25f, 0.25f, 0.3f, 1.0f);	break;
				case Settings::BGStyle::White:	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);		break;
			}
			float l = tMath::tRound(bgX);
			float r = tMath::tRound(bgX+bgW);
			float b = tMath::tRound(bgY);
			float t = tMath::tRound(bgY+bgH);

			glBegin(GL_QUADS);
			glVertex2f(l, b);
			glVertex2f(l, t);
			glVertex2f(r, t);
			glVertex2f(r, b);
			glEnd();

			break;
		}
	}
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
	int topUIHeight = 26;
	if (FullscreenMode)
	{
		bottomUIHeight = 0;
		topUIHeight = 0;
	}
	else if (!ShowLog)
	{
		bottomUIHeight = 0;
	}

	ImGui_ImplOpenGL2_NewFrame();		
	ImGui_ImplGlfw_NewFrame();
	int dispw, disph;
	glfwGetFramebufferSize(window, &dispw, &disph);
	if ((dispw != Dispw) || (disph != Disph))
	{
		Dispw = dispw;
		Disph = disph;
		if ((PanOffsetX+DragDownOffsetX == 0) && (PanOffsetY+DragDownOffsetY == 0))
			ResetPan();
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
	float hmargin = 0.0f;
	float vmargin = 0.0f;
	if (CurrImage)
	{
		iw = float(CurrImage->GetWidth());
		ih = float(CurrImage->GetHeight());
		float picAspect = iw/ih;

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

		// w and h are the image width and height. draww and drawh are the drawable area width and height.
		float l = tMath::tRound(hmargin);
		float r = tMath::tRound(hmargin+draww);
		float b = tMath::tRound(vmargin);
		float t = tMath::tRound(vmargin+drawh);

		float uvHMargin = 0.0f;
		float uvVMargin = 0.0f;

		if (CurrZoomMode == ZoomMode::Downscale)
		{
			ZoomPercent = 100.0f;
			if (draww < iw)
				ZoomPercent = 100.0f * draww / iw;
		}
		else if (CurrZoomMode == ZoomMode::Fit)
		{
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
			ResetPan(true, false);

		if (drawh > h)
			ResetPan(false, true);

		float uOffset = -float(PanOffsetX+DragDownOffsetX)/w;
		float vOffset =  float(PanOffsetY+DragDownOffsetY)/h;

		// Draw background.
		if (Config.BackgroundExtend)
			DrawBackground(draww, drawh, hmargin, vmargin);
		else
			DrawBackground(r-l, t-b, l, b);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		CurrImage->Bind();
		glEnable(GL_TEXTURE_2D);

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
		float picY = (areaH - cy - 1) - b;
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

	ImGuiWindowFlags flagsImgButton = 0;
	flagsImgButton |= ImGuiWindowFlags_NoTitleBar;
	flagsImgButton |= ImGuiWindowFlags_NoScrollbar;
	flagsImgButton |= ImGuiWindowFlags_NoMove;
	flagsImgButton |= ImGuiWindowFlags_NoResize;
	flagsImgButton |= ImGuiWindowFlags_NoCollapse;
	flagsImgButton |= ImGuiWindowFlags_NoNav;
	flagsImgButton |= ImGuiWindowFlags_NoBackground;
	flagsImgButton |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	NextPrevDisappear -= dt;
	if (NextPrevDisappear > 0.0)
	{
		if (CurrImage != Images.First())
		{
			ImGui::SetNextWindowPos(ImVec2(0.0f, float(topUIHeight) + float(workAreaH)*0.5f - 33.0f));
			ImGui::SetNextWindowSize(ImVec2(18, 72), ImGuiCond_Always);
			ImGui::Begin("Prev", nullptr, flagsImgButton);
			ImGui::SetCursorPos(ImVec2(4, 2));
			if (ImGui::ImageButton(ImTextureID(PrevImage.GetTexID()), ImVec2(12,56), ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
				OnPrevious();
			ImGui::End();
		}

		if (CurrImage != Images.Last())
		{
			ImGui::SetNextWindowPos(ImVec2(workAreaW-32.0f, float(topUIHeight) + float(workAreaH)*0.5f - 33.0f));
			ImGui::SetNextWindowSize(ImVec2(18, 72), ImGuiCond_Always);
			ImGui::Begin("Next", nullptr, flagsImgButton);
			ImGui::SetCursorPos(ImVec2(4, 2));
			if (ImGui::ImageButton(ImTextureID(NextImage.GetTexID()), ImVec2(12,56), ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
				OnNext();
			ImGui::End();
		}

		// Skip to beginning button.
		if (CurrImage != Images.First())
		{
			ImGui::SetNextWindowPos(ImVec2((workAreaW>>1)-22.0f-40.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
			ImGui::SetNextWindowSize(ImVec2(40, 40), ImGuiCond_Always);
			ImGui::Begin("SkipBegin", nullptr, flagsImgButton);
			if (ImGui::ImageButton(ImTextureID(SkipBeginImage.GetTexID()), ImVec2(24,24), ImVec2(0,0), ImVec2(1,1), 2, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
				OnSkipBegin();
			ImGui::End();
		}

		// Fullscreen / Windowed button.
		ImGui::SetNextWindowPos(ImVec2((workAreaW>>1)-22.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
		ImGui::SetNextWindowSize(ImVec2(40, 40), ImGuiCond_Always);
		ImGui::Begin("Fullscreen", nullptr, flagsImgButton);
		uint64 imageID = FullscreenMode ? WindowedImage.GetTexID() : FullscreenImage.GetTexID();
		if (ImGui::ImageButton(ImTextureID(imageID), ImVec2(24,24), ImVec2(0,0), ImVec2(1,1), 2, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
			ChangeScreenMode(!FullscreenMode);
		ImGui::End();

		// Skip to end button.
		if (CurrImage != Images.Last())
		{
			ImGui::SetNextWindowPos(ImVec2((workAreaW>>1)-22.0f+40.0f, float(topUIHeight) + float(workAreaH) - 42.0f));
			ImGui::SetNextWindowSize(ImVec2(40, 40), ImGuiCond_Always);
			ImGui::Begin("SkipEnd", nullptr, flagsImgButton);
			if (ImGui::ImageButton(ImTextureID(SkipEndImage.GetTexID()), ImVec2(24,24), ImVec2(0,0), ImVec2(1,1), 2, ImVec4(0,0,0,0), ImVec4(1,1,1,1)))
				OnSkipEnd();
			ImGui::End();
		}
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0));

	if (!FullscreenMode)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,6));
		ImGui::BeginMainMenuBar();
		{
			static bool saveAsDialog = false;
			bool justOpenedSaveAsDialog = false;
			if (ImGui::BeginMenu("File"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));

				// Show file menu items...
				if (ImGui::MenuItem("Save As...") && CurrImage)
				{
					saveAsDialog = !saveAsDialog;
					if (saveAsDialog)
						justOpenedSaveAsDialog = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "Alt+F4"))
					glfwSetWindowShouldClose(Window, 1);

				ImGui::PopStyleVar();
				ImGui::EndMenu();

			}
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));
			if (saveAsDialog)
				ShowSaveAsDialog(&saveAsDialog, justOpenedSaveAsDialog);
			ImGui::PopStyleVar();

			static bool contactDialog = false;
			bool justOpenedContactDialog = false;
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));
				if (ImGui::MenuItem("Contact Sheet...") && (Images.GetNumItems() > 1))
				{
					contactDialog = !contactDialog;
					if (contactDialog)
						justOpenedContactDialog = true;
				}
				ImGui::PopStyleVar();
				ImGui::EndMenu();
			}
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));
			if (contactDialog)
				ShowContactSheetDialog(&contactDialog, justOpenedContactDialog);
			ImGui::PopStyleVar();

			if (ImGui::BeginMenu("View"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));
				ImGui::MenuItem("Show Log", "", &ShowLog, true);
				ImGui::MenuItem("Show Overlay", "", &TexView::Config.OverlayShow, true);

				ImGui::Separator();

				bool userMode = CurrZoomMode == ZoomMode::User;
				if (ImGui::MenuItem("Zoom User", "", &userMode, true))
				{
					ResetPan();
					CurrZoomMode = ZoomMode::User;
				}

				bool fitMode = CurrZoomMode == ZoomMode::Fit;
				if (ImGui::MenuItem("Zoom Fit", "", &fitMode, true))
				{
					ResetPan();
					CurrZoomMode = ZoomMode::Fit;
				}

				bool downscale = CurrZoomMode == ZoomMode::Downscale;
				if (ImGui::MenuItem("Zoom Downscale", "", &downscale, true))
				{
					ResetPan();
					CurrZoomMode = ZoomMode::Downscale;
				}

				bool oneToOne = CurrZoomMode == ZoomMode::OneToOne;
				if (ImGui::MenuItem("Zoom 1:1", "", &oneToOne, true))
				{
					ZoomPercent = 100.0f;
					ResetPan();
					CurrZoomMode = ZoomMode::OneToOne;
				}

				ImGui::PushItemWidth(200);
				if (ImGui::SliderFloat("", &ZoomPercent, 20.0f, 2500.0f, " Zoom %.2f"))
					CurrZoomMode = ZoomMode::User;
				ImGui::PopItemWidth();

				ImGui::Separator();

				ImGui::MenuItem("Background Extend", "", &Config.BackgroundExtend, true);

				const char* backgroundItems[] = { "No Background", "Checkerboard Background", "Black Background", "Grey Background", "White Background" };
				ImGui::Combo(" ", &Config.BackgroundStyle, backgroundItems, IM_ARRAYSIZE(backgroundItems));

				if (ImGui::Button("Reset Pan"))
					ResetPan();

				if (ImGui::Button("Reset UI"))
				{
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					const GLFWvidmode* mode = glfwGetVideoMode(monitor);
					Config.Reset(mode->width, mode->height);
					ChangeScreenMode(false, true);
				}

				ImGui::PopStyleVar();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));
				ImGui::MenuItem("Cheat Sheet", "", &ShowCheatSheet, true);
				ImGui::MenuItem("About", "", &ShowAbout, true);
				ImGui::PopStyleVar();
				ImGui::EndMenu();
			}

			tColourf floatCol(PixelColour);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);

			ImVec4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
			
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
			if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, ImVec2(20,20)))
				ImGui::OpenPopup("CopyColourAs");

			if (ImGui::BeginPopup("CopyColourAs"))
				TexView::ColourCopyAs();

			bool buttonAvail = CurrImage ? !CurrImage->IsAltImageEnabled() : false;
			if
			(
				ImGui::ImageButton(ImTextureID(FlipHImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2, ImVec4(0,0,0,0),
				buttonAvail ? ImVec4(1.00f, 1.00f, 1.00f, 1.00f) : ImVec4(0.46f, 0.46f, 0.58f, 1.00f)) && buttonAvail
			)
			{
				CurrImage->Unbind();
				CurrImage->Flip(true);
				CurrImage->Bind();
			}
			ShowToolTip("Flip Horizontally");

			if
			(
				ImGui::ImageButton(ImTextureID(FlipVImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2, ImVec4(0,0,0,0),
				buttonAvail ? ImVec4(1.00f, 1.00f, 1.00f, 1.00f) : ImVec4(0.46f, 0.46f, 0.58f, 1.00f)) && buttonAvail
			)
			{
				CurrImage->Unbind();
				CurrImage->Flip(false);
				CurrImage->Bind();
			}
			ShowToolTip("Flip Vertically");

			if
			(
				ImGui::ImageButton(ImTextureID(RotateACWImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2, ImVec4(0,0,0,0),
				buttonAvail ? ImVec4(1.00f, 1.00f, 1.00f, 1.00f) : ImVec4(0.46f, 0.46f, 0.58f, 1.00f)) && buttonAvail
			)
			{
				CurrImage->Unbind();
				CurrImage->Rotate90(true);
				CurrImage->Bind();
			}
			ShowToolTip("Rotate 90 Anticlockwise");

			if
			(
				ImGui::ImageButton(ImTextureID(RotateCWImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2, ImVec4(0,0,0,0),
				buttonAvail ? ImVec4(1.00f, 1.00f, 1.00f, 1.00f) : ImVec4(0.46f, 0.46f, 0.58f, 1.00f)) && buttonAvail
			)
			{
				CurrImage->Unbind();
				CurrImage->Rotate90(false);
				CurrImage->Bind();
			}
			ShowToolTip("Rotate 90 Clockwise");

			bool altMipmapsImgAvail = CurrImage ? CurrImage->IsAltMipmapsImageAvail() : false;
			bool altMipmapsImgEnabl = altMipmapsImgAvail && CurrImage->IsAltImageEnabled();
			if
			(
				ImGui::ImageButton
				(
					ImTextureID(MipmapsImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2,
					altMipmapsImgEnabl ? ImVec4(0.45f, 0.45f, 0.60f, 1.00f) : ImVec4(0.00f, 0.00f, 0.00f, 0.00f),
					altMipmapsImgAvail ? ImVec4(1.00f, 1.00f, 1.00f, 1.00f) : ImVec4(0.46f, 0.46f, 0.58f, 1.00f)
				) && altMipmapsImgAvail
			)
			{
				CurrImage->EnableAltImage(!altMipmapsImgEnabl);
				CurrImage->Bind();
			}
			ShowToolTip("Display Mipmaps\nDDS files may include mipmaps.");

			bool altCubemapImgAvail = CurrImage ? CurrImage->IsAltCubemapImageAvail() : false;
			bool altCubemapImgEnabl = altCubemapImgAvail && CurrImage->IsAltImageEnabled();
			if
			(
				ImGui::ImageButton
				(
					ImTextureID(CubemapImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2,
					altCubemapImgEnabl ? ImVec4(0.45f, 0.45f, 0.60f, 1.00f) : ImVec4(0.00f, 0.00f, 0.00f, 0.00f),
					altCubemapImgAvail ? ImVec4(1.00f, 1.00f, 1.00f, 1.00f) : ImVec4(0.46f, 0.46f, 0.58f, 1.00f)
				) && altCubemapImgAvail
			)
			{
				CurrImage->EnableAltImage(!altCubemapImgEnabl);
				CurrImage->Bind();
			}
			ShowToolTip("Display Cubemap\nDDS files may be cubemaps.");

			if
			(
				ImGui::ImageButton(ImTextureID(InfoOverlayImage.GetTexID()), ImVec2(16,16), ImVec2(0,1), ImVec2(1,0), 2,
				TexView::Config.OverlayShow ? ImVec4(0.45f, 0.45f, 0.60f, 1.00f) : ImVec4(0.00f, 0.00f, 0.00f, 0.00f),
				ImVec4(1.00f, 1.00f, 1.00f, 1.00f))
			)
			{
				TexView::Config.OverlayShow = !TexView::Config.OverlayShow;			
			}
			ShowToolTip("Information Overlay");
		}

		ImGui::EndMainMenuBar();
		ImGui::PopStyleVar();
	}

	if (!FullscreenMode && ShowLog)
		TexView::DrawTextureViewerLog(0.0f, float(disph - bottomUIHeight), float(dispw), float(bottomUIHeight));

	// We allow the overlay and cheatsheet in fullscreen.
	if (TexView::Config.OverlayShow)
		TexView::ShowInfoOverlay(&TexView::Config.OverlayShow, hmargin, float(topUIHeight)+vmargin, float(dispw)-2.0f*hmargin, float(disph - bottomUIHeight - topUIHeight)-2.0f*vmargin, imgxi, imgyi);

	if (ShowCheatSheet)
		TexView::ShowCheatSheetPopup(&ShowCheatSheet, float(dispw), float(topUIHeight));

	if (ShowAbout)
		TexView::ShowAboutPopup(&ShowAbout, float(dispw), float(topUIHeight));

	ImGui::Render();
	glViewport(0, 0, dispw, disph);
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);

	// Consider unloading some images to keep memory down. The numToCheck roughly means it will take
	// 3 seconds to check all the files assuming a 60Hz update frequency.
	if (UnloadImage && (Images.NumItems() > 0))
	{
		int numToCheck = tMath::tGetClampMin( Images.NumItems() / (3*60), 1 );
		for (int c = 0; c < numToCheck; c++)
		{
			if ((TacitImage::GetNumLoaded() > MaxLoadedCount) && UnloadImage && (UnloadImage != CurrImage) && UnloadImage->IsLoaded())
				UnloadImage->Unload();
			UnloadImage = Images.NextCirc(UnloadImage);
		}
	}
}


bool TexView::ChangeScreenMode(bool fullscreen, bool force)
{
	if (!force && (FullscreenMode == fullscreen))
		return false;

	// If currently in windowed mode, remember our window geometry.
	if (!force && !FullscreenMode)
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
	return true;
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
			if (modifiers == GLFW_MOD_CONTROL)
				OnSkipBegin();
			else
				OnPrevious();
			break;

		case GLFW_KEY_RIGHT:
			if (modifiers == GLFW_MOD_CONTROL)
				OnSkipEnd();
			else
				OnNext();
			break;

		case GLFW_KEY_SPACE:
			OnNext();
			break;

		// Ctrl +
		case GLFW_KEY_EQUAL:
			if (modifiers == GLFW_MOD_CONTROL)
			{
				CurrZoomMode = ZoomMode::User;
				ZoomPercent = tMath::tFloor(ZoomPercent*0.1f + 1.0f) * 10.0f;
				tMath::tClamp(ZoomPercent, 20.0f, 2500.0f);
			}
			break;

		// Ctrl -
		case GLFW_KEY_MINUS:
			if (modifiers == GLFW_MOD_CONTROL)
			{
				CurrZoomMode = ZoomMode::User;
				ZoomPercent = tMath::tCeiling(ZoomPercent*0.1f - 1.0f) * 10.0f;
				tMath::tClamp(ZoomPercent, 20.0f, 2500.0f);
			}
			break;

		case GLFW_KEY_ENTER:
			if (modifiers == GLFW_MOD_ALT)
				ChangeScreenMode(!FullscreenMode);
			else
				TexView::Config.OverlayShow = !TexView::Config.OverlayShow;			
			break;

		case GLFW_KEY_F1:
			ShowCheatSheet = !ShowCheatSheet;
			break;

		case GLFW_KEY_F11:
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
	double topUIHeight = 26.0;
	if ((ypos <= topUIHeight) && !FullscreenMode)
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
}


void TexView::ScrollWheelCallback(GLFWwindow* window, double x, double y)
{
	NextPrevDisappear = 4.0;

	CurrZoomMode = ZoomMode::User;
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
	PopulateImages();
	SetCurrentImage(file);
}


void TexView::FocusCallback(GLFWwindow* window, int gotFocus)
{
	if (!gotFocus)
		return;

	// If we got focus, rescan the current folder to see if the hash is different.
	tList<tStringItem> files;
	FindImageFiles(files);
	tuint256 hash = ComputeImagesHash(files);

	if (hash != ImagesHash)
	{
		tPrintf("Hash mismatch. Dir contents changed. Resynching.\n");
		Images.Clear();
		PopulateImages();
		if (ImageFileParam.IsPresent())
			SetCurrentImage(TexView::ImageFileParam.Get());
		else
			SetCurrentImage();
	}
	else
	{
		tPrintf("Hash match. Dir contents same. Doing nothing.\n");
	}
}


void TexView::IconifyCallback(GLFWwindow* window, int iconified)
{
	WindowIconified = iconified;
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

	tString dataDir = tSystem::tGetProgramDir() + "Data/";
	tString cfgFile = dataDir + "Settings.cfg";
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

	if (!tSystem::tDirExists(dataDir))
	{
		::MessageBoxA
		(
			hwnd,
			"Tacit Texture Viewer failed to launch because it was run from a location "
			"that did not have the Data directory in it. The executable should be in the "
			"same place as the Data directory.",
			"Viewer Message",
			MB_OK
		);

		glfwDestroyWindow(TexView::Window);
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(TexView::Window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetWindowRefreshCallback(TexView::Window, TexView::WindowRefreshFun);
	glfwSetKeyCallback(TexView::Window, TexView::KeyCallback);
	glfwSetMouseButtonCallback(TexView::Window, TexView::MouseButtonCallback);
	glfwSetCursorPosCallback(TexView::Window, TexView::CursorPosCallback);
	glfwSetScrollCallback(TexView::Window, TexView::ScrollWheelCallback);
	glfwSetDropCallback(TexView::Window, TexView::FileDropCallback);
	glfwSetWindowFocusCallback(TexView::Window, TexView::FocusCallback);
	glfwSetWindowIconifyCallback(TexView::Window, TexView::IconifyCallback);

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

	tString fontFile = dataDir + "Roboto-Medium.ttf";
	io.Fonts->AddFontFromFileTTF(fontFile.ConstText(), 14.0f);

	TexView::CursorImage.Load(dataDir + "PixelCursor.png");
	TexView::CursorImage.Bind();

	TexView::PrevImage.Load(dataDir + "PrevArrow.png");
	TexView::PrevImage.Bind();

	TexView::NextImage.Load(dataDir + "NextArrow.png");
	TexView::NextImage.Bind();

	TexView::FlipHImage.Load(dataDir + "FlipH.png");
	TexView::FlipHImage.Bind();

	TexView::FlipVImage.Load(dataDir + "FlipV.png");
	TexView::FlipVImage.Bind();

	TexView::RotateACWImage.Load(dataDir + "RotACW.png");
	TexView::RotateACWImage.Bind();

	TexView::RotateCWImage.Load(dataDir + "RotCW.png");
	TexView::RotateCWImage.Bind();

	TexView::FullscreenImage.Load(dataDir + "Fullscreen.png");
	TexView::FullscreenImage.Bind();

	TexView::WindowedImage.Load(dataDir + "Windowed.png");
	TexView::WindowedImage.Bind();

	TexView::SkipBeginImage.Load(dataDir + "SkipBegin.png");
	TexView::SkipBeginImage.Bind();

	TexView::SkipEndImage.Load(dataDir + "SkipEnd.png");
	TexView::SkipEndImage.Bind();

	TexView::MipmapsImage.Load(dataDir + "Mipmaps.png");
	TexView::MipmapsImage.Bind();

	TexView::CubemapImage.Load(dataDir + "Cubemap.png");
	TexView::CubemapImage.Bind();

	TexView::InfoOverlayImage.Load(dataDir + "InfoOverlay.png");
	TexView::InfoOverlayImage.Bind();

	TexView::PopulateImages();
	if (TexView::ImageFileParam.IsPresent())
		TexView::SetCurrentImage(TexView::ImageFileParam.Get());
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

	// Get current window geometry and set in config file if we're not in fullscreen mode or iconified.
	if (!TexView::FullscreenMode && !TexView::WindowIconified)
	{
		glfwGetWindowPos(TexView::Window, &TexView::Config.WindowX, &TexView::Config.WindowY);
		glfwGetWindowSize(TexView::Window, &TexView::Config.WindowW, &TexView::Config.WindowH);
	}
	TexView::Config.Save(cfgFile);

	// Cleanup.
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(TexView::Window);
	glfwTerminate();
	return 0;
}
