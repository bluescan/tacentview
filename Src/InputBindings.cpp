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
	void ShowAddBindingSection(Config::Settings& settings);
	void InitKeyNameTables();

	const int MaxKeyNameLength = 16;
	char KeyNameTable[GLFW_KEY_LAST+1][MaxKeyNameLength];
	struct TableInitializer { TableInitializer() { InitKeyNameTables(); } }; TableInitializer Initializer;

	extern const char* OperationDescriptions[int(Operation::NumOperations)];

	// Given a glfw key as input, returns the full name of the key. This is useful if you want to display help text that
	// describes what a particulr key does. Returns nullptr if the GLFW key define is not supported. Examples:
	// GLFW_KEY_SPACE			->		"Space"
	// GLFW_KEY_GRAVE_ACCENT	->		"~"
	// GLFW_KEY_ECAPE			->		"Esc"
	// GLFW_KEY_F11				->		"F11"
	// GLFW_KEY_Q				->		"Q"
	const char* GetKeyName(int glfwkey);
	bool IsKeySupported(int glfwkay);
	bool IsPermanentBinding(int glfwkey, uint32 modifiers);
}


void Bindings::InitKeyNameTables()
{
	tMemset(KeyNameTable, 0, tNumElements(KeyNameTable));

	tsPrintf(KeyNameTable[GLFW_KEY_SPACE],			"Space");
	KeyNameTable[GLFW_KEY_APOSTROPHE][0]			= GLFW_KEY_APOSTROPHE;
	KeyNameTable[GLFW_KEY_COMMA][0]					= '<';
	KeyNameTable[GLFW_KEY_MINUS][0]					= GLFW_KEY_MINUS;
	KeyNameTable[GLFW_KEY_PERIOD][0]				= '>';
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
}


const char* Bindings::OperationDescriptions[int(Operation::NumOperations)] =
{
	"None",
	"Next Image",
	"Previous Image",
	"Skip To Last Image",
	"Skip To First Image",
	"Next Image Frame",
	"Previous Image Frame",
	"One Pixel Right",
	"One Pixel Left",
	"One Pixel Down",
	"One Pixel Up",

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
	"Property Editor",
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
	"Toggle Debug Log",

	"Zoom Fit",
	"Zoom Downscale Only",
	"Zoom 1:1 Pixels",
	"Contact Sheet...",
	"Preferences...",
	"Content Thumbnail View...",
	"Toggle Key Bindings...",

	"Toggle Channel Filter",
	"Toggle Red Channel",
	"Toggle Green Channel",
	"Toggle Blue Channel",
	"Toggle Alpha Channel",
	"Toggle Channel As Intensity",

	"Undo",
	"Redo"
};


const char* Bindings::GetKeyName(int key)
{
	if (KeyNameTable[key][0] == '\0')
		return nullptr;

	return KeyNameTable[key];
}


bool Bindings::IsKeySupported(int key)
{
	return (KeyNameTable[key][0] != '\0');
}


bool Bindings::IsPermanentBinding(int key, uint32 modifiers)
{
	// These bindings are not changeable or deletable. Currently the only one is the ToggleKeybindings
	// key: Ctrl-Shift Tab. This is needed so that the user can't get 'stuck' if they hide the menu-bar
	// and remove the keybindings.
	if ((key == GLFW_KEY_TAB) && (modifiers == (Modifier_Ctrl | Modifier_Shift)))
		return true;

	return false;
}


const char* Bindings::GetOperationDesc(Operation op)
{
	return OperationDescriptions[int(op)];
}


const char* Bindings::GetModifiersText(uint32 modifiers)
{
	if (!modifiers)
		return nullptr;

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


tString Bindings::GetModKeyText(int key, uint32 modifiers)
{
	tString result;
	const char* keyName = GetKeyName(key);
	if (!keyName)
		return result;

	const char* modText = GetModifiersText(modifiers);
	if (modText)
		tsPrintf(result, "%s %s", modText, keyName);
	else
		tsPrintf(result, "%s", keyName);

	return result;
}


uint32 Bindings::TranslateModifiers(int glfwModifiers)
{
	uint32 modifiers = 0x00000000;
	modifiers |= (glfwModifiers & GLFW_MOD_CONTROL)	? Modifier_Ctrl		: 0;
	modifiers |= (glfwModifiers & GLFW_MOD_ALT)		? Modifier_Alt		: 0;
	modifiers |= (glfwModifiers & GLFW_MOD_SHIFT)	? Modifier_Shift	: 0;
	return modifiers;
}


bool Bindings::InputMap::AssignKey(int key, uint32 modifiers, Operation operation)
{
	// If key already assigned to requested operation we're done.
	if (KeyTable[key].Operations[modifiers] == operation)
		return false;

	KeyTable[key].Operations[modifiers] = operation;
	return true;
}


void Bindings::InputMap::Reset()
{
	Clear();
	AssignKey(GLFW_KEY_RIGHT,		Modifier_None,					Operation::NextImage);
	AssignKey(GLFW_KEY_LEFT,		Modifier_None,					Operation::PrevImage);
	AssignKey(GLFW_KEY_RIGHT,		Modifier_Ctrl,					Operation::SkipToLastImage);
	AssignKey(GLFW_KEY_LEFT,		Modifier_Ctrl,					Operation::SkipToFirstImage);
	AssignKey(GLFW_KEY_RIGHT,		Modifier_Alt,					Operation::NextImageFrame);
	AssignKey(GLFW_KEY_LEFT,		Modifier_Alt,					Operation::PrevImageFrame);
	AssignKey(GLFW_KEY_RIGHT,		Modifier_Shift,					Operation::OnePixelRight);
	AssignKey(GLFW_KEY_LEFT,		Modifier_Shift,					Operation::OnePixelLeft);
	AssignKey(GLFW_KEY_UP,			Modifier_Shift,					Operation::OnePixelUp);
	AssignKey(GLFW_KEY_DOWN,		Modifier_Shift,					Operation::OnePixelDown);
	AssignKey(GLFW_KEY_SPACE,		Modifier_None,					Operation::NextImage);

	AssignKey(GLFW_KEY_EQUAL,		Modifier_Ctrl,					Operation::ZoomIn);
	AssignKey(GLFW_KEY_MINUS,		Modifier_Ctrl,					Operation::ZoomOut);
	AssignKey(GLFW_KEY_F1,			Modifier_None,					Operation::ToggleCheatSheet);
	AssignKey(GLFW_KEY_F2,			Modifier_None,					Operation::RenameFile);
	AssignKey(GLFW_KEY_F5,			Modifier_None,					Operation::RefreshReloadImage);
	AssignKey(GLFW_KEY_F11,			Modifier_None,					Operation::ToggleFullscreen);
	AssignKey(GLFW_KEY_ENTER,		Modifier_Alt,					Operation::ToggleFullscreen);

	AssignKey(GLFW_KEY_ESCAPE,		Modifier_None,					Operation::EscapeSupportingQuit);
	AssignKey(GLFW_KEY_ENTER,		Modifier_None,					Operation::OpenFileBrowser);
	AssignKey(GLFW_KEY_DELETE,		Modifier_None,					Operation::Delete);
	AssignKey(GLFW_KEY_DELETE,		Modifier_Shift,					Operation::DeletePermanent);
	AssignKey(GLFW_KEY_F4,			Modifier_Alt,					Operation::Quit);

	AssignKey(GLFW_KEY_COMMA,		Modifier_Ctrl,					Operation::FlipVertically);
	AssignKey(GLFW_KEY_PERIOD,		Modifier_Ctrl,					Operation::FlipHorizontally);
	AssignKey(GLFW_KEY_COMMA,		Modifier_None,					Operation::Rotate90Anticlockwise);
	AssignKey(GLFW_KEY_PERIOD,		Modifier_None,					Operation::Rotate90Clockwise);
	AssignKey(GLFW_KEY_SLASH,		Modifier_None,					Operation::Crop);
	AssignKey(GLFW_KEY_E,			Modifier_None,					Operation::PropertyEditor);
	AssignKey(GLFW_KEY_A,			Modifier_None,					Operation::AdjustPixelColour);
	AssignKey(GLFW_KEY_R,			Modifier_Alt,					Operation::ResizeImage);
	AssignKey(GLFW_KEY_R,			Modifier_Ctrl,					Operation::ResizeCanvas);
	AssignKey(GLFW_KEY_R,			Modifier_None,					Operation::RotateImage);

	AssignKey(GLFW_KEY_I,			Modifier_None,					Operation::ToggleImageDetails);
	AssignKey(GLFW_KEY_T,			Modifier_None,					Operation::ToggleTile);
	AssignKey(GLFW_KEY_M,			Modifier_None,					Operation::ToggleMenuBar);
	AssignKey(GLFW_KEY_M,			Modifier_Ctrl,					Operation::SaveMultiFrameImage);
	AssignKey(GLFW_KEY_N,			Modifier_None,					Operation::ToggleNavBar);
	AssignKey(GLFW_KEY_S,			Modifier_None,					Operation::ToggleSlideshowCountdown);

	AssignKey(GLFW_KEY_S,			Modifier_Ctrl,					Operation::SaveAs);
	AssignKey(GLFW_KEY_S,			Modifier_Alt,					Operation::SaveAll);
	AssignKey(GLFW_KEY_B,			Modifier_None,					Operation::ToggleBasicMode);
	AssignKey(GLFW_KEY_L,			Modifier_None,					Operation::ToggleDebugLog);

	AssignKey(GLFW_KEY_F,			Modifier_None,					Operation::ZoomFit);
	AssignKey(GLFW_KEY_D,			Modifier_None,					Operation::ZoomDownscaleOnly);
	AssignKey(GLFW_KEY_Z,			Modifier_None,					Operation::ZoomOneToOne);
	AssignKey(GLFW_KEY_C,			Modifier_None,					Operation::ContactSheet);
	AssignKey(GLFW_KEY_P,			Modifier_None,					Operation::Preferences);
	AssignKey(GLFW_KEY_V,			Modifier_None,					Operation::ContentThumbnailView);
	AssignKey(GLFW_KEY_TAB,			Modifier_None,					Operation::ToggleKeyBindings);

	// This one is special and can't be reassigned or removed. This is because the user _could_ turn off the menu,
	// and remove all bindings to ToggleKeyBindings. The user would have to way of getting back to the bindings screen.
	// Having a guaranteed key combo for it solves these issues.
	AssignKey(GLFW_KEY_TAB,			Modifier_Ctrl | Modifier_Shift,	Operation::ToggleKeyBindings);

	AssignKey(GLFW_KEY_GRAVE_ACCENT,Modifier_None,					Operation::ToggleChannelFilter);
	AssignKey(GLFW_KEY_1,			Modifier_None,					Operation::ToggleRedChannel);
	AssignKey(GLFW_KEY_2,			Modifier_None,					Operation::ToggleGreenChannel);
	AssignKey(GLFW_KEY_3,			Modifier_None,					Operation::ToggleBlueChannel);
	AssignKey(GLFW_KEY_4,			Modifier_None,					Operation::ToggleAlphaChannel);
	AssignKey(GLFW_KEY_5,			Modifier_None,					Operation::ToggleChannelAsIntensity);

	AssignKey(GLFW_KEY_Z,			Modifier_Ctrl,					Operation::Undo);
	AssignKey(GLFW_KEY_Y,			Modifier_Ctrl,					Operation::Redo);

	#ifdef ENABLE_FILE_DIALOG_SUPPORT
	AssignKey(GLFW_KEY_O,			Modifier_Ctrl,					Operation::OpenFile);
	AssignKey(GLFW_KEY_O,			Modifier_Alt,					Operation::OpenDir);
	#endif
}


void Bindings::InputMap::Read(tExpression expr)
{
	tExpr command = expr.Command();
	if (command != tString("KeyBindings"))
		return;

	Clear();
	for (tExpr binding = expr.Item1(); binding.IsValid(); binding = binding.Next())
	{
		if (binding.Command() == tString("KeyModOp"))
		{
			int key			= binding.Item1();
			uint32 mods		= binding.Item2();
			int op			= binding.Item3();
			tiClamp(key,	0, GLFW_KEY_LAST - 1);
			tiClamp(mods,	0u, uint32(Modifier_NumCombinations) - 1);
			tiClamp(op,		0, int(Operation::NumOperations) - 1);
			KeyTable[key].Operations[mods] = Operation(op);
		}
	}
}


void Bindings::InputMap::Write(tScriptWriter& writer)
{
	writer.Begin();
	writer.Indent();
	writer.CR();
	writer.WriteAtom("KeyBindings");
	writer.CR();

	for (int key = 0; key <= GLFW_KEY_LAST; key++)
	{
		if (!KeyTable[key].IsAnythingAssigned())
			continue;

		for (uint32 mods = Modifier_None; mods < Modifier_NumCombinations; mods++)
		{
			Operation op = KeyTable[key].Operations[mods];
			if (op == Operation::None)
				continue;

			writer.Begin();	
			writer.WriteAtom("KeyModOp");
			writer.WriteAtom(key);
			writer.WriteAtom(mods);
			writer.WriteAtom(int(op));
			writer.End();
			writer.WriteComment(GetOperationDesc(op));
		}
	}

	writer.Dedent();
	writer.WriteComment("End of KeyBindings.");
	writer.End();
	writer.CR();
}


void Bindings::ShowBindingsWindow(bool* popen, bool justOpened)
{
	tVector2 windowPos = GetDialogOrigin(7);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(tVector2(440.0f, 600.0f), ImGuiCond_Appearing);
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize;

	if (ImGui::Begin("Keyboard Bindings", popen, flags))
	{
		static int profile = 0;
		if (justOpened)
			profile = int(Config::GetProfile());

		ImGui::SetNextItemWidth(100);
		ImGui::Combo("##ProfileToEdit", &profile, Config::ProfileNamesLong, int(Config::Profile::NumProfiles));
		Config::Settings& settings = (profile == 0) ? Config::MainSettings : Config::BasicSettings;

		ImGui::SameLine();
		if (ImGui::Button("Reset Profile", tVector2(100, 0)))
			settings.InputBindings.Reset();
		ShowToolTip("Resets the key bindings to default for the chosen profile.");

		ImGui::SameLine();
		if (ImGui::Button("Set All Profiles", tVector2(100, 0)))
		{
			// Operator= deals with the object being the same one, so just copy them both over indescriminately.
			Config::MainSettings.InputBindings = settings.InputBindings;
			Config::BasicSettings.InputBindings = settings.InputBindings;
		}
		ShowToolTip("Copies the keybindings to all profiles. Useful if you want them all the same.");

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
		if (ImGui::Button("Close", tVector2(100, 0)))
		{
			if (popen)
				*popen = false;
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
		
		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
		const float rowHeight = 25.0f;
		const int maxRowsToDisplay = 16;
		int totalAssigned = settings.InputBindings.GetTotalAssigned();
		const int numRowsToDisplay = tMin(maxRowsToDisplay, totalAssigned);
		tVector2 outerSize = ImVec2(0.0f, rowHeight + rowHeight * float(numRowsToDisplay));
		if (ImGui::BeginTable("KeyBindingTable", 3, tableFlags, outerSize))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 120);//ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed, 240);
			ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed, 20.0f);
			ImGui::TableHeadersRow();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

			ImGuiListClipper clipper;

			// Honestly not sure why I need +2. I am drawing an extra row so the clipper works properly, but
			// that 'should' make it be a +1. Maybe it's the header row that counts as one. In any case, with
			// the +2 and the dummy row at end, it all looks and works properly.
			clipper.Begin(totalAssigned+2);
			while (clipper.Step())
			{
				// Multiple keys can be bound to the same operation (ex space and right-arrow for next image)
				// Multiple operations can NOT be bound to the same key, cuz that would be stupid -- well I guess
				// that's what a macro is -- but anyway, that would be a different system,
				//
				// This loop displays the currently bound keys (and modifiers) and what operation they are bound to.
				//
				// Key				Operation
				// Ctrl-Shift-R 	Next Image[combo]		[-]
				// Space		 	Rotate Image[combo]		[-]
				// --------------------------------------------
				// Key[combo] Mods	Operation[combo]	 	[+] (brings up replace popup if necessary)
				for (int k = 0; k <= GLFW_KEY_LAST; k++)
				{
					KeyOps& keyops = settings.InputBindings.GetKeyOps(k);

					// Skip unsupported keys and don't display keys with nothing assigned.
					if (!keyops.IsAnythingAssigned() || !IsKeySupported(k))
						continue;

					for (int m = 0; m < Modifier_NumCombinations; m++)
					{
						Operation opCurr = keyops.Operations[m];
						if (opCurr == Operation::None)
							continue;

						ImGui::TableNextRow();

						// Key/mods column.
						ImGui::TableSetColumnIndex(0);
						ImGui::Text( GetModKeyText(k, m).Text() );

						// The operation column.
						ImGui::TableSetColumnIndex(1);
						char oplabel[64]; tsPrintf(oplabel, "##op%d_%d", k, m);
						const char* opCurrDesc = GetOperationDesc(opCurr);
						tAssert(opCurrDesc);
						ImGui::SetNextItemWidth(240);

						bool permanent = IsPermanentBinding(k, m);
						if (!permanent)
						{
							if (ImGui::BeginCombo(oplabel, opCurrDesc, ImGuiComboFlags_NoArrowButton))
							{
								for (int oper = int(Operation::First); oper < int(Operation::NumOperations); oper++)
								{
									Operation op = Operation(oper);
									const char* opDesc = GetOperationDesc(op);

									bool isSelected = (op == opCurr);

									// Selectable just displays the item as highlighted or not.
									if (ImGui::Selectable(opDesc, isSelected))
										opCurr = op;

									// Set the initial focus when opening the combo (scrolling + keyboard navigation focus).
									if (isSelected)
										ImGui::SetItemDefaultFocus();
								}

								if ((opCurr != Operation::None) && (keyops.Operations[m] != Operation(opCurr)))
									keyops.Operations[m] = Operation(opCurr);

								ImGui::EndCombo();
							}
						}
						else
						{
							const char* opDesc = GetOperationDesc(Operation::ToggleKeyBindings);
							tString opText;
							tsPrintf(opText, " %s (Permanent Binding)", opDesc);
							ImGui::Text(opText.Chars());
						}
						
						// The remove button column.
						ImGui::TableSetColumnIndex(2);
						if (!permanent)
						{
							char blabel[64];
							tsPrintf(blabel, " - ##b%d_%d", k, m);
							if (ImGui::Button(blabel, tVector2(21.0f, 21.0f)))
								keyops.Operations[m] = Operation::None;
						}
					}
				}

				// The final row is just a dummy row that makes the clipper work properly. Without this you see part of an
				// extra row that shouldn't be visible.
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Button("##LastRowCol0");
				ImGui::TableSetColumnIndex(1);
				ImGui::Button("##LastRowCol1");
				ImGui::TableSetColumnIndex(2);
				ImGui::Button("##LastRowCol2");
			}
			ImGui::EndTable();		
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
		ShowAddBindingSection(settings);
	}
	ImGui::End();
}


void Bindings::ShowAddBindingSection(Config::Settings& settings)
{
	uint32 tableFlags = ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
	const float rowHeight = 25.0f;
	tVector2 outerSize = ImVec2(0.0f, rowHeight);
	if (ImGui::BeginTable("KeyAssignTable", 3, tableFlags, outerSize))
	{
		ImGui::TableSetupColumn("##AssignKey", ImGuiTableColumnFlags_WidthFixed, 120);
		ImGui::TableSetupColumn("##AssignOperation", ImGuiTableColumnFlags_WidthFixed, 240);
		ImGui::TableSetupColumn("##Assign", ImGuiTableColumnFlags_WidthFixed, 20.0f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		int comboFlags = ImGuiComboFlags_NoArrowButton;
		static uint32 addMods = 0;
		const char* addModsName = GetModifiersText(addMods);
		if (!addModsName)
			addModsName = "No Mods";
		ImGui::SetNextItemWidth(71);
		if (ImGui::BeginCombo("##Modifiers", addModsName, comboFlags))
		{
			for (uint32 mods = 0; mods < Modifier_NumCombinations; mods++)
			{
				const char* modsName = GetModifiersText(mods);
				if (!modsName)
					modsName = "No Mods";

				bool isSelected = (mods == addMods);
				if (ImGui::Selectable(modsName, isSelected))
					addMods = mods;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();

		// The key choosing combo. We use the more powerful BeginCombo call to hide the arrow button (no room)
		// as well as deal with the 'holes' in the key-name array (not all keys supported).
		static int addKey = GLFW_KEY_SPACE;
		const char* addKeyName = GetKeyName(addKey);
		tAssert(addKeyName);
		ImGui::SetNextItemWidth(41);
		if (ImGui::BeginCombo("##AddKey", addKeyName, comboFlags))
		{
			for (int key = 0; key <= GLFW_KEY_LAST; key++)
			{
				const char* keyName = GetKeyName(key);
				if (!keyName)
					continue;

				bool isSelected = (key == addKey);
				if (ImGui::Selectable(keyName, isSelected))
					addKey = key;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::TableSetColumnIndex(1);	
		static Operation addOp = Operation::NextImage;
		const char* addOpDesc = GetOperationDesc(addOp);
		tAssert(addOpDesc);
		ImGui::SetNextItemWidth(240);
		if (ImGui::BeginCombo("##AddOperation", addOpDesc, comboFlags))
		{
			for (int oper = int(Operation::First); oper < int(Operation::NumOperations); oper++)
			{
				Operation op = Operation(oper);
				const char* opDesc = GetOperationDesc(op);

				bool isSelected = (op == addOp);
				if (ImGui::Selectable(opDesc, isSelected))
					addOp = op;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::TableSetColumnIndex(2);
		static Operation currentlyAssignedOp = Operation::None;

		// This binding is considered permanent/unchangeable.
		bool permanent = IsPermanentBinding(addKey, addMods);
		if (ImGui::Button(" + ##AddBinding", tVector2(21.0f, 21.0f)))
		{
			if (addOp != Operation::None)
			{
				currentlyAssignedOp = settings.InputBindings.GetOperation(addKey, addMods);

				// If current is assigned to something and that something is not the same as what's being assigned,
				// then bring up the warning popup.
				if ((currentlyAssignedOp != Operation::None) && (currentlyAssignedOp != Operation(addOp)))
					ImGui::OpenPopup("Key Assignment Warning");
				else
					settings.InputBindings.AssignKey(addKey, addMods, Operation(addOp));
			}
		}
		ShowToolTip("Adds the new keybinding. Warns if already assigned to something else.");

		bool isOpen = true;
		if (ImGui::BeginPopupModal("Key Assignment Warning", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			tString msg;
			if (permanent)
			{
				tsPrintf
				(
					msg,
					"Warning, '%s' is a permanent binding that cannot be changed.",
					GetModKeyText(addKey, addMods).Text()
				);
			}
			else
			{
				tsPrintf
				(
					msg,
					"Warning, '%s' is currently assigned to '%s'.\n"
					"Would you like to replace the current binding or cancel?",
					GetModKeyText(addKey, addMods).Text(), GetOperationDesc(currentlyAssignedOp)
				);
			}
			ImGui::NewLine();
			ImGui::Text(msg.Text());
			ImGui::NewLine();

			if (ImGui::Button("Cancel##AssignWarn", tVector2(100, 0)))
				ImGui::CloseCurrentPopup();

			if (!permanent)
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
				if (ImGui::Button("Replace##AssignWarn", tVector2(100, 0)))
				{
					settings.InputBindings.AssignKey(addKey, addMods, Operation(addOp));
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::EndTable();
	}
}


void Bindings::ShowCheatSheetWindow(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(5);
	ImGui::SetNextWindowBgAlpha(0.75f);
	ImGui::SetNextWindowSize(tVector2(350.0f, 600.0f), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Cheat Sheet", popen, flags))
	{
		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
		const float rowHeight = 25.0f;
		const int maxRowsToDisplay = 18;
		int totalAssigned = Config::Current->InputBindings.GetTotalAssigned();
		const int numRowsToDisplay = tMin(maxRowsToDisplay, totalAssigned);
		tVector2 outerSize = ImVec2(0.0f, rowHeight + rowHeight * float(numRowsToDisplay));
		if (ImGui::BeginTable("CheatSheetTable", 2, tableFlags, outerSize))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 110);
			ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed, 240);
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(totalAssigned);
			while (clipper.Step())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Mouse Left-Click");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Set Reticle Pos");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Mouse Right-Hold");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Pan Image");

				for (int k = 0; k <= GLFW_KEY_LAST; k++)
				{
					KeyOps& keyops = Config::Current->InputBindings.GetKeyOps(k);
					if (!keyops.IsAnythingAssigned() || !IsKeySupported(k))
						continue;

					for (int m = 0; m < Modifier_NumCombinations; m++)
					{
						Operation opCurr = keyops.Operations[m];
						if (opCurr == Operation::None)
							continue;

						ImGui::TableNextRow();

						// Action column.
						ImGui::TableSetColumnIndex(0);
						ImGui::Text( GetModKeyText(k, m).Text() );

						// Operation column.
						ImGui::TableSetColumnIndex(1);
						const char* opCurrDesc = GetOperationDesc(opCurr);
						tAssert(opCurrDesc);
						ImGui::Text(opCurrDesc);
					}
				}
			}
			ImGui::EndTable();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
		ImGui::Text("Current Profile: %s", (Config::GetProfile() == Config::Profile::Main) ? "Main" : "Basic");

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
