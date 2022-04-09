// InputBindings.h
//
// Allows you to set key bindings for all TacentView operations.
//
// Copyright (c) 2022 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tString.h>
#include <GLFW/glfw3.h>
namespace Viewer {
namespace Bindings {


enum class Operation
{
	None,
	PreviousImage,
	NextImage,
	SkipToFirstImage,
	// WIP

	NumOperations
};
const char* GetOperationDesc(Operation);


enum Modifier
{
	Modifier_None,
	Modifier_Shift				= 1 << 0,
	Modifier_Alt				= 1 << 1,
	Modifier_Ctrl				= 1 << 2,
	Modifier_NumCombinations	= 1 << 3
};
const char* GetModifiersText(uint32 modifiers);

// Converts from GLFW modifiers to viewer modifiers.
uint32 TranslateModifiers(int glfwModifiers);


// Given a glfw key as input, returns the full name of the key. This is useful if you want to display help text that
// describes what a particulr key does. Returns nullptr if the GLFW key define is not supported. Examples:
// GLFW_KEY_SPACE			->		"Space"
// GLFW_KEY_GRAVE_ACCENT	->		"~"
// GLFW_KEY_ECAPE			->		"Esc"
// GLFW_KEY_F11				->		"F11"
// GLFW_KEY_Q				->		"Q"
const char* GetKeyName(int glfwkey);


// Specifies, for a single key, what operations it performs. The same key can map to multiple operations if modifier
// keys are used.
struct KeyOps
{
	KeyOps()																											{ Clear(); }
	void Clear()																										{ for (int e = 0; e < Modifier_NumCombinations; e++) Operations[e] = Operation::None; }
	Operation Operations[Modifier_NumCombinations];
};


class InputMap
{
public:
	InputMap()																											{ Reset(); }

	void Clear();				// Unassigns all keys.
	void Reset();				// Sets all keys to their default operations.

	// Returns the operation assigned to a particular key. This can also be used before an assign call to see what a
	// current key is bound to so an already-assigned message can be dislayed if needed.
	Operation GetOperation(int glfwkey, uint32 modifiers);

	// Assigns the operation to the key and modifiers specified. Returns false if the key is already assigned to
	// something else.
	bool AssignKey(int glfwkey, uint32 modifiers, Operation);
	void ClearKey(int glfwkey, uint32 modifiers);


private:
	KeyOps KeyTable[GLFW_KEY_LAST+1];
};


extern InputMap DefaultInputMap;


// Returns the operation currently bound to the 
//int GetOperation(

// Config should look like
//	[
//		Keybindings
//		[ Keybinding Operation 


void ShowWindow(bool* popen);


// Implementaion only below this line.


inline void InputMap::Clear()
{
	for (int k = 0; k <= GLFW_KEY_LAST; k++)
		KeyTable->Clear();
}


inline Operation InputMap::GetOperation(int key, uint32 modifiers)
{
	return KeyTable[key].Operations[modifiers];
}


inline void InputMap::ClearKey(int key, uint32 modifiers)
{
	KeyTable[key].Operations[modifiers] = Operation::None;
}


}


#if 0
		ImGui::Text("Right Arrow");	"Next Image");
		ImGui::Text("Ctrl-Left");	"Skip to First Image");`
		ImGui::Text("Ctrl-Right");	"Skip to Last Image");
		ImGui::Text("Alt-Left");	"Previous Image Frame");
		ImGui::Text("Alt-Right");	"Next Image Frame");
		ImGui::Text("Shift-Left");	"One Pixel Left");
		ImGui::Text("Shift-Right");	"One Pixel Right");
		ImGui::Text("Shift-Up");	"One Pixel Up");
		ImGui::Text("Shift-Down");	"One Pixel Down");
		ImGui::Text("Space");		"Next Image");
		ImGui::Text("Ctrl +");		"Zoom In");
		ImGui::Text("Ctrl -");		"Zoom Out");
		ImGui::Text("F1");			"Toggle Cheat Sheet");
		ImGui::Text("F2");			"Rename File");
		ImGui::Text("F5");			"Refresh/Reload Image");
		ImGui::Text("F11");			"Toggle Fullscreen");
		ImGui::Text("Alt-Enter");   "Toggle Fullscreen");
		if (Config::Current->EscCanQuit)
			{ ImGui::Text("Esc");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Quit / Exit Fullscreen or Basic"); }
		else
			{ ImGui::Text("Esc");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Exit Fullscreen / Exit Basic"); }
		#ifdef PACKAGE_SNAP
		ImGui::Text("Tab");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Open File Browser (No Snap Support)");
		#else
		ImGui::Text("Tab");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Open File Browser");
		#endif
		ImGui::Text("Delete");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Delete Current Image");
		ImGui::Text("Shift-Delete");ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Delete Current Image Permanently");
		ImGui::Text("LMB-Click");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Set Colour Reticle Pos");
		ImGui::Text("RMB-Drag");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Pan Image");
		ImGui::Text("Alt-F4");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Quit");
		ImGui::Text("Ctrl <");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Flip Vertically");
		ImGui::Text("Ctrl >");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Flip Horizontally");
		ImGui::Text("<");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Rotate Anti-Clockwise");
		ImGui::Text(">");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Rotate Clockwise");
		ImGui::Text("/");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Crop");
		ImGui::Text("A");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Adjust Pixel Colour");
		ImGui::Text("Alt-R");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Resize Image");
		ImGui::Text("Ctrl-R");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Resize Canvas");
		ImGui::Text("R");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Rotate Image");
		ImGui::Text("I");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Image Details");
		ImGui::Text("T");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Tile");
		ImGui::Text("M");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Menu Bar");
		ImGui::Text("Ctrl-M");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save Multi-Frame Image");
		ImGui::Text("N");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Nav Bar");
		ImGui::Text("S");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Slideshow Counddown");

		#ifdef ENABLE_FILE_DIALOG_SUPPORT
		ImGui::Text("Ctrl-O");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Open File...");
		ImGui::Text("Alt-O");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Open Dir...");
		#endif
	
		ImGui::Text("Ctrl-S");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save As...");
		ImGui::Text("Alt-S");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save All...");
		ImGui::Text("B");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Basic Mode");
		ImGui::Text("F");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Fit");
		ImGui::Text("L");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Show Debug Log");
		ImGui::Text("D");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Downscale Only");
		ImGui::Text("Z");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom 1:1 Pixels");
		ImGui::Text("C");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Contact Sheet...");
		ImGui::Text("P");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Preferences...");
		ImGui::Text("V");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Content Thumbnail View...");

		ImGui::Text("~");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Channel Filter");
		ImGui::Text("1");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Red Channel");
		ImGui::Text("2");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Green Channel");
		ImGui::Text("3");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Blue Channel");
		ImGui::Text("4");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Alpha Channel");
		ImGui::Text("5");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Channel As Intensity");
#endif


}
