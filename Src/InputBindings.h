// InputBindings.h
//
// Allows you to set key bindings for all TacentView operations.
//
// Copyright (c) 2022-2023 Tristan Grimmer.
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
#include "Profile.h"
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

		PixelRight,
		PixelLeft,
		PixelDown,
		PixelUp,

		ZoomIn,
		ZoomOut,
		ZoomFit,
		ZoomDownscaleOnly,
		ZoomOneToOne,
		ResetPan,

		FlipVertically,
		FlipHorizontally,
		Rotate90Anticlockwise,
		Rotate90Clockwise,
		RotateImage,
		Crop,
		ResizeImage,
		ResizeCanvas,
		PixelEdit,
		PropertyEdit,

		ChannelFilter,
		RedChannel,
		GreenChannel,
		BlueChannel,
		AlphaChannel,
		ChannelAsIntensity,

		Details,
		Tile,
		Undo,
		Redo,

		Refresh,
		Rename,
		Delete,
		DeletePermanent,

		SaveAs,
		SaveAll,
		SaveMultiFrameImage,
		SaveContactSheet,

		MenuBar,
		NavBar,
		Thumbnails,
		FileBrowser,
		SlideshowTimer,
		CheatSheet,
		DebugLog,

		Profile,
		Preferences,
		KeyBindings,

		Fullscreen,

		Escape,					// Exit-Fullscreen | Exit-Basic-Profile.
		EscapeSupportingQuit,	// Exit-Fullscreen | Exit-Basic-Profile | Quit.
		Quit,

		//
		// Here are new operations that were added to keep keybindings in tact. Can be cleaned up when config version increases.
		//
		OpenFile,
		OpenDir,
		MetaData,
		Save,					// A save that does not ask for a new filename.
		UISizeInc,				// Increment the UI (font and button) size one step bigger.
		UISizeDec,				// Decrement the UI (font and button) size one step smaller.
		//
		// Add new operations here. Adding above will invalidate save file keybindings.
		//

		NumOperations
	};

	enum Modifier
	{
		Modifier_None,
		Modifier_Ctrl				= 1 << 0,
		Modifier_Alt				= 1 << 1,
		Modifier_Shift				= 1 << 2,
		Modifier_NumCombinations	= 1 << 3
	};
	const char* GetModifiersText(uint32 modifiers);

	// Gets the combined (modifiers plus key) text. ex. "Ctrl-Alt-Shift Space" or "Ctrl A" or "A".
	// Returns empty string if the key is not supported.
	tString GetModKeyText(int glfwkey, uint32 modifiers);

	// Specifies, for a single key, what operations it performs. The same key can map to multiple operations if modifier
	// keys are used.
	struct KeyOps
	{
		KeyOps()																										{ Clear(); }
		KeyOps(const KeyOps& src)																						{ Set(src); }
		void Set(const KeyOps& src)																						{ if (&src == this) return; for (int m = 0; m < Modifier_NumCombinations; m++) Operations[m] = src.Operations[m]; }

		void Clear()																									{ for (int m = 0; m < Modifier_NumCombinations; m++) Operations[m] = Operation::None; }
		bool IsAnythingAssigned() const																					{ for (int m = 0; m < Modifier_NumCombinations; m++) if (Operations[m] != Operation::None) return true; return false; }
		int GetAssignedCount() const																					{ int count = 0; for (int m = 0; m < Modifier_NumCombinations; m++) if (Operations[m] != Operation::None) count++; return count; }

		// Finds the mods for the first (there may be more than one) operation matching the search operation.
		// Returns false if no such operation was found.
		bool FindOperationMods(uint32& mods, Operation searchOp) const													{ for (uint32 m = 0; m < Modifier_NumCombinations; m++) { if (Operations[m] == searchOp) { mods = m; return true; } } return false; }
		KeyOps& operator=(const KeyOps& src)																			{ Set(src); return *this; }

		Operation Operations[Modifier_NumCombinations];
	};

	class InputMap
	{
	public:
		InputMap()																										{ Clear(); }
		InputMap(const InputMap& src)																					{ Set(src); }
		void Set(const InputMap& src)																					{ if (&src == this) return; for (int k = 0; k <= GLFW_KEY_LAST; k++) KeyTable[k] = src.KeyTable[k]; }
		void Clear()								/* Unassigns all keys. */											{ for (int k = 0; k <= GLFW_KEY_LAST; k++) KeyTable[k].Clear(); }

		// Sets all keys to their default operations. Some keys have different operations depending on the profile so we
		// need to pass that in. If onlyIfUnassigned is true, the reset will only apply to keys that are not already
		// bound to something. A 'key' here refers to the actual key plus any modifiers (ctrl, alt, shift).
		void Reset(Viewer::Profile, bool onlyIfUnassigned = false);

		// Returns the operation assigned to a particular key and set of modifiers. This can also be used before an
		// assign call to see what a current key is bound to so an already-assigned message can be dislayed if needed.
		Operation GetOperation(int glfwKey, uint32 modifiers)															{ return KeyTable[glfwKey].Operations[modifiers]; }
		KeyOps& GetKeyOps(int glfwKey)																					{ return KeyTable[glfwKey]; }

		// Assigns the operation to the key and modifiers specified. Returns true is there was a previous assignment
		// that needed to be replaced. Pass in onlyIfUnassigned if you only want assignment to happen if the key is
		// currently unassigned.
		bool AssignKey(int glfwkey, uint32 modifiers, Operation, bool onlyIfUnassigned = false);
		void ClearKey(int glfwkey, uint32 modifiers)																	{ KeyTable[glfwkey].Operations[modifiers] = Operation::None; }
		int GetTotalAssigned() const																					{ int count = 0; for (int k = 0; k <= GLFW_KEY_LAST; k++) count += KeyTable[k].GetAssignedCount(); return count; }

		// Searches for the first occurrence of the supplied operation and returns the key and modifiers that are bound
		// to it. Note that since there may be more than one key bound to the same operation, this function returns the
		// first one found. Returns true if something found.
		bool FindModKey(int& key, uint32& mods, Operation);

		// Convenience. Basically calls FindModKey on the supplied operation and then calls GetModKeyText on the result.
		// The returned string will be empty if nothing is bound to the operation.
		tString FindModKeyText(Operation op)																			{ int key = 0; uint32 mods = 0; if (FindModKey(key, mods, op)) return GetModKeyText(key, mods); else return tString(); }

		void Read(tExpr);
		void Write(tExprWriter&) const;

		InputMap& operator=(const InputMap& src)																		{ Set(src); return *this; }

	private:
		KeyOps KeyTable[GLFW_KEY_LAST+1];
	};

	// Some descriptions may change based on the current config, for example, what the esc key does
	const char* GetOperationDesc(Operation);

	// Converts from GLFW modifiers to viewer modifiers.
	uint32 TranslateModifiers(int glfwModifiers);

	void ShowBindingsWindow(bool* popen, bool justOpened);
	void ShowCheatSheetWindow(bool* popen);
}

}
