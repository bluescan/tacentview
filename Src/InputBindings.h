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


namespace Bindings
{
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
		#ifdef ENABLE_FILE_DIALOG_SUPPORT
		OpenFile,
		OpenDir,
		#endif

		// Add new operations here. Adding above will invalidate save file keybindings.
		NumOperations
	};

	enum Modifier
	{
		Modifier_None,
		Modifier_Shift				= 1 << 0,
		Modifier_Alt				= 1 << 1,
		Modifier_Ctrl				= 1 << 2,
		Modifier_NumCombinations	= 1 << 3
	};
	const char* GetModifiersText(uint32 modifiers);

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

	// Some descriptions may change based on the current config, for example, what the esc key does
	const char* GetOperationDesc(Operation);

	// Converts from GLFW modifiers to viewer modifiers.
	uint32 TranslateModifiers(int glfwModifiers);

	void ShowBindingsWindow(bool* popen, bool justOpened);
	void ShowCheatSheetWindow(bool* popen);
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
