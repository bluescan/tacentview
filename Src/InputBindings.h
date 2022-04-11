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
#include <System/tScript.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
namespace Viewer {
namespace Bindings {


// Add new operations to the bottom of this enum. If you add in the middle or reorder,
// it will invalidate the key bindings in previous version's config save files.
enum class Operation
{
	None,
	PrevImage,			First = PrevImage,
	NextImage,
	SkipToFirstImage,
	SkipToLastImage,
	PrevImageFrame,
	NextImageFrame,
	OnePixelLeft,
	OnePixelRight,
	OnePixelUp,
	OnePixelDown,

	ZoomIn,
	ZoomOut,
	ToggleCheatSheet,
	RenameFile,
	RefreshReloadImage,
	ToggleFullscreen,

	Escape,
	EscapeSupportingQuit,
	OpenFileBrowser,
	Delete,
	DeletePermanent,
	Quit,

	FlipVertically,
	FlipHorizontally,
	Rotate90Anticlockwise,
	Rotate90Clockwise,
	Crop,
	AdjustPixelColour,
	ResizeImage,
	ResizeCanvas,
	RotateImage,

	ToggleImageDetails,
	ToggleTile,
	ToggleMenuBar,
	SaveMultiFrameImage,
	ToggleNavBar,
	ToggleSlideshowCountdown,
	SaveAs,
	SaveAll,
	ToggleBasicMode,

	ZoomFit,
	ShowDebugLog,
	ZoomDownscaleOnly,
	ZoomOneToOne,
	ContactSheet,
	Preferences,
	ContentThumbnailView,

	ToggleChannelFilter,
	ToggleRedChannel,
	ToggleGreenChannel,
	ToggleBlueChannel,
	ToggleAlphaChannel,
	ToggleChannelAsIntensity,

	// Add new operations here. Adding above will invalidate save file keybindings.

	NumOperations
};


// Some descriptions may change based on the current config, for example, what the esc key does
const char* GetOperationDesc(Operation);

#if 0
		ImGui::Text("Left Arrow");	D("Previous Image");
		ImGui::Text("Right Arrow");	D("Next Image");
		ImGui::Text("Ctrl-Left");	D("Skip to First Image");
		ImGui::Text("Ctrl-Right");	D("Skip to Last Image");
		ImGui::Text("Alt-Left");	D("Previous Image Frame");
		ImGui::Text("Alt-Right");	D("Next Image Frame");
		ImGui::Text("Shift-Left");	D("One Pixel Left");
		ImGui::Text("Shift-Right");	D("One Pixel Right");
		ImGui::Text("Shift-Up");	D("One Pixel Up");
		ImGui::Text("Shift-Down");	D("One Pixel Down");
		ImGui::Text("Space");		R("Next Image");

		ImGui::Text("Ctrl +");		D("Zoom In");
		ImGui::Text("Ctrl -");		D("Zoom Out");
		ImGui::Text("F1");			D("Toggle Cheat Sheet");
		ImGui::Text("F2");			D("Rename File");
		ImGui::Text("F5");			D("Refresh/Reload Image");
		ImGui::Text("F11");			D("Toggle Fullscreen");
		ImGui::Text("Alt-Enter");   R("Toggle Fullscreen");

		ImGui::Text("Esc");			D("Exit Fullscreen / Exit Basic"); }
		ImGui::Text("Esc");			D("Quit / Exit Fullscreen or Basic");

		#ifdef PACKAGE_SNAP
		ImGui::Text("Tab");			D("Open File Browser (No Snap Support)");
		#else
		ImGui::Text("Tab");			D("Open File Browser");
		#endif
		ImGui::Text("Delete");		D("Delete Current Image");
		ImGui::Text("Shift-Delete");D("Delete Current Image Permanently");
		ImGui::Text("LMB-Click");	SKIPPED("Set Colour Reticle Pos");
		ImGui::Text("RMB-Drag");	SKIPPED("Pan Image");
		ImGui::Text("Alt-F4");		D Sep Operation("Quit");
		ImGui::Text("Ctrl <");		D("Flip Vertically");
		ImGui::Text("Ctrl >");		D("Flip Horizontally");
		ImGui::Text("<");			D("Rotate Anti-Clockwise");
		ImGui::Text(">");			D("Rotate Clockwise");
		ImGui::Text("/");			D("Crop");
		ImGui::Text("A");			D("Adjust Pixel Colour");
		ImGui::Text("Alt-R");		D("Resize Image");
		ImGui::Text("Ctrl-R");		D("Resize Canvas");
		ImGui::Text("R");			D("Rotate Image");
		ImGui::Text("I");			D("Toggle Image Details");
		ImGui::Text("T");			D("Toggle Tile");
		ImGui::Text("M");			D("Toggle Menu Bar");
		ImGui::Text("Ctrl-M");		D("Save Multi-Frame Image");
		ImGui::Text("N");			D("Toggle Nav Bar");
		ImGui::Text("S");			D("Toggle Slideshow Counddown");

		#ifdef ENABLE_FILE_DIALOG_SUPPORT
		ImGui::Text("Ctrl-O");		SKIPPED("Open File...");
		ImGui::Text("Alt-O");		SKIPPED("Open Dir...");
		#endif	
		ImGui::Text("Ctrl-S");		D("Save As...");
		ImGui::Text("Alt-S");		D("Save All...");
		ImGui::Text("B");			D("Toggle Basic Mode");

		ImGui::Text("F");			D("Zoom Fit");
		ImGui::Text("L");			D("Show Debug Log");
		ImGui::Text("D");			D("Zoom Downscale Only");
		ImGui::Text("Z");			D("Zoom 1:1 Pixels");
		ImGui::Text("C");			D("Contact Sheet...");
		ImGui::Text("P");			D("Preferences...");
		ImGui::Text("V");			D("Content Thumbnail View...");

		ImGui::Text("~");			D("Toggle Channel Filter");
		ImGui::Text("1");			D("Toggle Red Channel");
		ImGui::Text("2");			D("Toggle Green Channel");
		ImGui::Text("3");			D("Toggle Blue Channel");
		ImGui::Text("4");			D("Toggle Alpha Channel");
		ImGui::Text("5");			D("Toggle Channel As Intensity");
#endif



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
	bool IsAnythingAssigned() const																						{ for (int m = 0; m < Modifier_NumCombinations; m++) if (Operations[m] != Operation::None) return true; return false; }
	Operation Operations[Modifier_NumCombinations];
};


class InputMap
{
public:
	InputMap()																											{ Reset(); }
	// WIP InputMap()																											{ Clear(); }

	void Clear();				// Unassigns all keys.
	void Reset();				// Sets all keys to their default operations.

	// Returns the operation assigned to a particular key. This can also be used before an assign call to see what a
	// current key is bound to so an already-assigned message can be dislayed if needed.
	Operation GetOperation(int glfwkey, uint32 modifiers);

	// Assigns the operation to the key and modifiers specified. Returns false if the key is already assigned to
	// something else.
	bool AssignKey(int glfwkey, uint32 modifiers, Operation);
	void ClearKey(int glfwkey, uint32 modifiers);

	void Read(tExpression);
	void Write(tScriptWriter&);

private:
	KeyOps KeyTable[GLFW_KEY_LAST+1];
};


extern InputMap DefaultInputMap;


void ShowWindow(bool* popen);
// WIP Move cheat sheet over to here.


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


} }
