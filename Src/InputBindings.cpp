// InputBindings.cpp
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

#include <Math/tVector2.h>
#include <System/tPrint.h>
#include "imgui.h"
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.
#include "InputBindings.h"
#include "TacentView.h"
using namespace tStd;
using namespace tMath;
namespace Viewer
{


namespace Bindings
{
	void InitKeyNameTable();

	bool KeyNameTableInitialized = false;
	const int MaxKeyNameLength = 16;
	char KeyNameTable[GLFW_KEY_LAST][MaxKeyNameLength];

	InputMap DefaultInputMap;
}


void Bindings::InitKeyNameTable()
{
	tMemset(KeyNameTable, 0, tNumElements(KeyNameTable));

	tsPrintf(KeyNameTable[GLFW_KEY_SPACE],			"Space");
	KeyNameTable[GLFW_KEY_APOSTROPHE][0]			= GLFW_KEY_APOSTROPHE;
	KeyNameTable[GLFW_KEY_COMMA][0]					= GLFW_KEY_COMMA;
	KeyNameTable[GLFW_KEY_MINUS][0]					= GLFW_KEY_MINUS;
	KeyNameTable[GLFW_KEY_PERIOD][0]				= GLFW_KEY_PERIOD;
	KeyNameTable[GLFW_KEY_SLASH][0]					= GLFW_KEY_SLASH;

	// We do blocks of consecutive values when we can. Most of the glfw keys map to ascii.
	for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; key++)
		KeyNameTable[key][0] = key;

	KeyNameTable[GLFW_KEY_SEMICOLON][0]				= GLFW_KEY_SEMICOLON;
	KeyNameTable[GLFW_KEY_EQUAL][0]					= GLFW_KEY_EQUAL;

	for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++)
		KeyNameTable[key][0] = key;

	KeyNameTable[GLFW_KEY_LEFT_BRACKET][0]			= GLFW_KEY_LEFT_BRACKET;
	KeyNameTable[GLFW_KEY_BACKSLASH][0]				= GLFW_KEY_BACKSLASH;
	KeyNameTable[GLFW_KEY_RIGHT_BRACKET][0]			= GLFW_KEY_RIGHT_BRACKET;
	KeyNameTable[GLFW_KEY_GRAVE_ACCENT][0]			= '~';

	tsPrintf(KeyNameTable[GLFW_KEY_ESCAPE],			"Esc");
	tsPrintf(KeyNameTable[GLFW_KEY_ENTER],			"Enter");
	tsPrintf(KeyNameTable[GLFW_KEY_TAB],			"Tab");
	tsPrintf(KeyNameTable[GLFW_KEY_BACKSPACE],		"Back");
	tsPrintf(KeyNameTable[GLFW_KEY_INSERT],			"Ins");
	tsPrintf(KeyNameTable[GLFW_KEY_DELETE],			"Del");

	tsPrintf(KeyNameTable[GLFW_KEY_RIGHT],			"Right");
	tsPrintf(KeyNameTable[GLFW_KEY_LEFT],			"Left");
	tsPrintf(KeyNameTable[GLFW_KEY_DOWN],			"Down");
	tsPrintf(KeyNameTable[GLFW_KEY_UP],				"Up");

	tsPrintf(KeyNameTable[GLFW_KEY_PAGE_UP],		"PgUp");
	tsPrintf(KeyNameTable[GLFW_KEY_PAGE_DOWN],		"PgDn");
	tsPrintf(KeyNameTable[GLFW_KEY_HOME],			"Home");
	tsPrintf(KeyNameTable[GLFW_KEY_END],			"End");

	for (int idx = 0; idx <= GLFW_KEY_F25-GLFW_KEY_F1; idx++)
		tsPrintf(KeyNameTable[GLFW_KEY_F1+idx], "F%d", idx+1);

	for (int idx = 0; idx <= GLFW_KEY_KP_9-GLFW_KEY_KP_0; idx++)
		tsPrintf(KeyNameTable[GLFW_KEY_KP_0+idx], "KP%d", idx);

	tsPrintf(KeyNameTable[GLFW_KEY_KP_DECIMAL],		"KP.");
	tsPrintf(KeyNameTable[GLFW_KEY_KP_DIVIDE],		"KP/");
	tsPrintf(KeyNameTable[GLFW_KEY_KP_MULTIPLY],	"KP*");
	tsPrintf(KeyNameTable[GLFW_KEY_KP_SUBTRACT],	"KP-");
	tsPrintf(KeyNameTable[GLFW_KEY_KP_ADD],			"KP+");
	tsPrintf(KeyNameTable[GLFW_KEY_KP_ENTER],		"KP-Ent");
	tsPrintf(KeyNameTable[GLFW_KEY_KP_EQUAL],		"KP=");

	KeyNameTableInitialized	= true;
}


const char* Bindings::GetOperationDesc(Operation op)
{
	const char* descriptions[] =
	{
		"None",
		"Previous Image",
		"Next Image",
		"Skip To First Image",
		"Skip To Last Image",
		"Previous Image Frame",
		"Next Image Frame",
		"One Pixel Left",
		"One Pixel Right",
		"One Pixel Up",
		"One Pixel Down",

		"Zoom In",
		"Zoom Out",
		"Toggle Cheat Sheet",
		"Rename File",
		"Refresh / Reload Image",
		"Toggle Fullscreen",

		"Exit-Fullscreen | Exit-Basic-Profile",
		"Quit | Exit-Fullscreen | Exit-Basic-Profile",
		#ifdef PACKAGE_SNAP
		"Open File Browser (No Snap Support)",
		#else
		"Open File Browser",
		#endif
		"Delete Current Image",
		"Delete Current Image Permanently",
		"Quit",

		"Flip Image Vertically",
		"Flip Image Horizontally",
		"Rotate 90 Anticlockwise",
		"Rotate 90 Clockwise",
		"Crop",
		"Adjust Pixel Colour",
		"Resize Image",
		"Resize Canvas",
		"Rotate Image",

		"Toggle Image Details",
		"Toggle Tile",
		"Toggle Menu Bar",
		"Save Multi-Frame Image",
		"Toggle Navigation Bar",
		"Toggle Slideshow Countdown",
		"Save As...",
		"Save All...",
		"Toggle Basic Mode",

		"Zoom Fit",
		"Show Debug Log",
		"Zoom Downscale Only",
		"Zoom 1:1 Pixels",
		"Contact Sheet...",
		"Preferences...",
		"Content Thumbnail View...",

		"Toggle Channel Filter",
		"Toggle Red Channel",
		"Toggle Green Channel",
		"Toggle Blue Channel",
		"Toggle Alpha Channel",
		"Toggle Channel As Intensity"
	};
	tStaticAssert(tNumElements(descriptions) == int(Operation::NumOperations));
	return descriptions[int(op)];
}


const char* Bindings::GetModifiersText(uint32 modifiers)
{
	// Order is ctrl-alt-shift. MSB to LSB.
	const char* dispstr[] =
	{
		"",
		"Shift",
		"Alt",
		"Alt-Shift",
		"Ctrl",
		"Ctrl-Shift",
		"Ctrl-Alt",
		"Ctrl-Alt-Shift"
	};
	tStaticAssert(tNumElements(dispstr) == Modifier_NumCombinations);
	return dispstr[modifiers];
}


uint32 Bindings::TranslateModifiers(int glfwModifiers)
{
	uint32 modifiers = 0x00000000;
	modifiers |= (glfwModifiers & GLFW_MOD_CONTROL)	? Modifier_Ctrl		: 0;
	modifiers |= (glfwModifiers & GLFW_MOD_ALT)		? Modifier_Alt		: 0;
	modifiers |= (glfwModifiers & GLFW_MOD_SHIFT)	? Modifier_Shift	: 0;
	return modifiers;
}


const char* Bindings::GetKeyName(int key)
{
	if (!KeyNameTableInitialized)
		InitKeyNameTable();

	return KeyNameTable[key];	
}


bool Bindings::InputMap::AssignKey(int key, uint32 modifiers, Operation operation)
{
	// If key already assigned to requested operation we're done.
	if (KeyTable[key].Operations[modifiers] == operation)
		return true;

	// If key already assigned to somethng else we fail.
	if (KeyTable[key].Operations[modifiers] != Operation::None)
		return false;

	KeyTable[key].Operations[modifiers] = operation;
	return true;
}


void Bindings::InputMap::Reset()
{
	Clear();
	AssignKey(GLFW_KEY_LEFT, Modifier_None, Operation::PrevImage);
	AssignKey(GLFW_KEY_RIGHT, Modifier_None, Operation::NextImage);

	// TESTING
	//	AssignKey(GLFW_KEY_COMMA, Modifier_None, Operation::PreviousImage);
	//	AssignKey(GLFW_KEY_PERIOD, Modifier_None, Operation::NextImage);
}


void Bindings::ShowWindow(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(7);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Keyboard Bindings", popen, flags))
	{
		const char* profiles[] = { "Main", "Basic" };
		static int profile = 0;
		ImGui::PushItemWidth(80);
		ImGui::Combo("Profile", &profile, profiles, tNumElements(profiles));
		ImGui::PopItemWidth();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		
		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerH;
		const int rowHeight = 24;
		const int firstRowHeight = 24;
		const int numRowsToDiaply = 20;
		tVector2 outerSize = ImVec2(0.0f, firstRowHeight + rowHeight * numRowsToDiaply);
		if (ImGui::BeginTable("KeyBindingTable", 5, tableFlags, outerSize))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableSetupColumn("Operation", ImGuiTableFlags_SizingFixedFit);
			ImGui::TableSetupColumn("Ctrl", ImGuiTableFlags_SizingFixedFit);
			ImGui::TableSetupColumn("Alt", ImGuiTableFlags_SizingFixedFit);
			ImGui::TableSetupColumn("Shift", ImGuiTableFlags_SizingFixedFit);
			ImGui::TableSetupColumn("Key", ImGuiTableFlags_SizingFixedFit);
			ImGui::TableHeadersRow();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);

			ImGuiListClipper clipper;
			clipper.Begin(int(Operation::NumOperations)-1);
			while (clipper.Step())
			{
				for (int op = int(Operation::First); op < int(Operation::NumOperations); op++)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", GetOperationDesc(Operation(op)));

					uint32 modifiers = 0;
					ImGui::TableSetColumnIndex(1);
					ImGui::CheckboxFlags("##lable", &modifiers, 1<<2);

					ImGui::TableSetColumnIndex(2);
					ImGui::CheckboxFlags("##lable", &modifiers, 1<<2);

					ImGui::PushItemWidth(50);
					ImGui::TableSetColumnIndex(3);
					ImGui::CheckboxFlags("##lable", &modifiers, 1<<2);
					ImGui::PopItemWidth();

					ImGui::TableSetColumnIndex(4);
					const char* keys[] = { "F1", "Enter", "Space", "F6" };
					static int key = 0;
					ImGui::PushItemWidth(70);
					char clabel[16]; tsPrintf(clabel, "       ##Keys%d", op);
					ImGui::Combo(clabel, &key, keys, tNumElements(keys));
					ImGui::PopItemWidth();
				}
			}
			ImGui::EndTable();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		if (ImGui::Button("Reset", tVector2(100, 0)))
		{
			//Config::ResetAll();
		}
		ShowToolTip("Resets the key bindings to default for the chosen profile.");
		ImGui::SameLine();

		if (ImGui::Button("Set All Profiles", tVector2(100, 0)))
		{
			//Config::ResetAll();
		}
		ShowToolTip("Copies the keybindings to all profiles. Useful if you want them all the same.");
		ImGui::SameLine();


		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
		if (ImGui::Button("Close", tVector2(100, 0)))
		{
			if (popen)
				*popen = false;
		}

	}

	ImGui::End();
}


}
