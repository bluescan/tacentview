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
#include <GLFW/glfw3.h>
namespace Viewer {
namespace Bindings {


// Add new operations to the bottom of this enum. If you add in the middle or reorder,
// it will invalidate the key bindings in previous version's config save files.
enum class Operation
{
	None,
	NextImage,						First = NextImage,
	PrevImage,
	SkipToLastImage,
	SkipToFirstImage,
	NextImageFrame,
	PrevImageFrame,
	OnePixelRight,
	OnePixelLeft,
	OnePixelDown,
	OnePixelUp,

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
	PropertyEditor,
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
	ToggleDebugLog,

	ZoomFit,
	ZoomDownscaleOnly,
	ZoomOneToOne,
	ContactSheet,
	Preferences,
	ContentThumbnailView,
	ToggleKeyBindings,

	ToggleChannelFilter,
	ToggleRedChannel,
	ToggleGreenChannel,
	ToggleBlueChannel,
	ToggleAlphaChannel,
	ToggleChannelAsIntensity,

	Undo,
	Redo,

	// Add new operations here. Adding above will invalidate save file keybindings.

	#ifdef ENABLE_FILE_DIALOG_SUPPORT
	OpenFile,
	OpenDir,
	#endif
	
	NumOperations
};


// Some descriptions may change based on the current config, for example, what the esc key does
const char* GetOperationDesc(Operation);

#if 0
	A	ImGui::Text("Left Arrow");	D("Previous Image");
	A	ImGui::Text("Right Arrow");	D("Next Image");
	A	ImGui::Text("Ctrl-Left");	D("Skip to First Image");
	A	ImGui::Text("Ctrl-Right");	D("Skip to Last Image");
	A	ImGui::Text("Alt-Left");	D("Previous Image Frame");
	A	ImGui::Text("Alt-Right");	D("Next Image Frame");
	A	ImGui::Text("Shift-Left");	D("One Pixel Left");
	A	ImGui::Text("Shift-Right");	D("One Pixel Right");
	A	ImGui::Text("Shift-Up");	D("One Pixel Up");
	A	ImGui::Text("Shift-Down");	D("One Pixel Down");
	A	ImGui::Text("Space");		R("Next Image");

	A	ImGui::Text("Ctrl +");		D("Zoom In");
	A	ImGui::Text("Ctrl -");		D("Zoom Out");
	A	ImGui::Text("F1");			D("Toggle Cheat Sheet");
	A	ImGui::Text("F2");			D("Rename File");
	A	ImGui::Text("F5");			D("Refresh/Reload Image");
	A	ImGui::Text("F11");			D("Toggle Fullscreen");
	A	ImGui::Text("Alt-Enter");   R("Toggle Fullscreen");

	A	ImGui::Text("Esc");			D("Exit Fullscreen / Exit Basic"); }
	A	ImGui::Text("Esc");			D("Quit / Exit Fullscreen or Basic");

	A	#ifdef PACKAGE_SNAP
	A	ImGui::Text("Enter");			D("Open File Browser (No Snap Support)");
	A	#else
	A	ImGui::Text("Enter");			D("Open File Browser");
	A	#endif
	A	ImGui::Text("Delete");		D("Delete Current Image");
	A	ImGui::Text("Shift-Delete");D("Delete Current Image Permanently");
		ImGui::Text("LMB-Click");	SKIPPED("Set Colour Reticle Pos");
		ImGui::Text("RMB-Drag");	SKIPPED("Pan Image");

	A	NEED TO KEEp AS OPERATION. DEFAULT BINDING to Alt-F4 (in case system alt-f4 disabled). ImGui::Text("Alt-F4");		D Sep Operation("Quit");
	A	ImGui::Text("Ctrl <");		D("Flip Vertically");
	A	ImGui::Text("Ctrl >");		D("Flip Horizontally");
	A	ImGui::Text("<");			D("Rotate Anti-Clockwise");
	A	ImGui::Text(">");			D("Rotate Clockwise");
	A	ImGui::Text("/");			D("Crop");
	A	ImGui::Text("A");			D("Adjust Pixel Colour");
	A	ImGui::Text("Alt-R");		D("Resize Image");
	A	ImGui::Text("Ctrl-R");		D("Resize Canvas");
	A	ImGui::Text("R");			D("Rotate Image");

	A	ImGui::Text("I");			D("Toggle Image Details");
	A	ImGui::Text("T");			D("Toggle Tile");
	A	ImGui::Text("M");			D("Toggle Menu Bar");
	A	ImGui::Text("Ctrl-M");		D("Save Multi-Frame Image");
	A	ImGui::Text("N");			D("Toggle Nav Bar");
	A	ImGui::Text("S");			D("Toggle Slideshow Counddown");

		#ifdef ENABLE_FILE_DIALOG_SUPPORT
	A	ImGui::Text("Ctrl-O");		SKIPPED("Open File...");
	A	ImGui::Text("Alt-O");		SKIPPED("Open Dir...");
		#endif	
	A	ImGui::Text("Ctrl-S");		D("Save As...");
	A	ImGui::Text("Alt-S");		D("Save All...");
	A	ImGui::Text("B");			D("Toggle Basic Mode");

	A	ImGui::Text("F");			D("Zoom Fit");
	A	ImGui::Text("L");			D("Show Debug Log");
	A	ImGui::Text("D");			D("Zoom Downscale Only");
	A	ImGui::Text("Z");			D("Zoom 1:1 Pixels");
	A	ImGui::Text("C");			D("Contact Sheet...");
	A	ImGui::Text("P");			D("Preferences...");
	A	ImGui::Text("V");			D("Content Thumbnail View...");
	A	ImGui::Text("K");			D("Keybindings");

	A	ImGui::Text("~");			D("Toggle Channel Filter");
	A	ImGui::Text("1");			D("Toggle Red Channel");
	A	ImGui::Text("2");			D("Toggle Green Channel");
	A	ImGui::Text("3");			D("Toggle Blue Channel");
	A	ImGui::Text("4");			D("Toggle Alpha Channel");
	A	ImGui::Text("5");			D("Toggle Channel As Intensity");
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
	int GetAssignedCount() const																						{ int count = 0; for (int m = 0; m < Modifier_NumCombinations; m++) if (Operations[m] != Operation::None) count++; return count; }
	Operation Operations[Modifier_NumCombinations];
};


class InputMap
{
public:
	InputMap()																											{ Clear(); }

	void Clear();				// Unassigns all keys.
	void Reset();				// Sets all keys to their default operations.

	// Returns the operation assigned to a particular key and set of modifiers. This can also be used before an assign
	// call to see what a current key is bound to so an already-assigned message can be dislayed if needed.
	Operation GetOperation(int glfwKey, uint32 modifiers);
	KeyOps& GetOperations(int glfwKey)																					{ return KeyTable[glfwKey]; }

	// Assigns the operation to the key and modifiers specified. Returns true is there was a previous assignment that
	// needed to be replaced.
	bool AssignKey(int glfwkey, uint32 modifiers, Operation);
	void ClearKey(int glfwkey, uint32 modifiers);
	int GetTotalAssigned() const																						{ int count = 0; for (int k = 0; k <= GLFW_KEY_LAST; k++) count += KeyTable[k].GetAssignedCount(); return count; }

	void Read(tExpression);
	void Write(tScriptWriter&);

private:
	KeyOps KeyTable[GLFW_KEY_LAST+1];
};


void ShowWindow(bool* popen, bool justOpened);
// WIP Move cheat sheet over to here.
// WIP Need to make sure always can bring up bindings window.. even if menu not visible.


// Implementaion only below this line.


inline void InputMap::Clear()
{
	for (int k = 0; k <= GLFW_KEY_LAST; k++)
		KeyTable[k].Clear();
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
