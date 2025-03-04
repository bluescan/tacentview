// InputBindings.cpp
//
// Allows you to set key bindings for all TacentView operations.
//
// Copyright (c) 2022-2024 Tristan Grimmer.
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
#include "GuiUtil.h"
using namespace tStd;
using namespace tMath;
namespace Viewer
{


namespace Bindings
{
	void ShowAddBindingSection(Config::ProfileData& settings, float keyWidth, float operationWidth, float removeAddSize);
	void InitKeyNameTables();

	const int MaxKeyNameLength = 16;
	char KeyNameTable[GLFW_KEY_LAST+1][MaxKeyNameLength];
	struct TableInitializer { TableInitializer() { InitKeyNameTables(); } }; TableInitializer Initializer;

	// Do not specify the size here so that we can static assert that the size matches the operations enum.
	extern const char* OperationDescriptions[];

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

	// We're going to skip F13 to F25 here. No normal keyboard has them.
	for (int idx = 0; idx <= GLFW_KEY_F12-GLFW_KEY_F1; idx++)
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


// Do not specify the size here so that we can static assert that the size matches the operations enum.
const char* Bindings::OperationDescriptions[] =
{
	"None",
	"Next Image",
	"Previous Image",
	"Skip To Last Image",
	"Skip To First Image",
	"Next Image Frame",
	"Previous Image Frame",

	"Pixel Right",
	"Pixel Left",
	"Pixel Down",
	"Pixel Up",

	"Zoom In",
	"Zoom Out",
	"Zoom Fit",
	"Zoom Downscale Only",
	"Zoom 1:1 Pixels",
	"Reset Pan",

	"Flip Vertically",
	"Flip Horizontally",
	"Rotate 90 Anticlockwise",
	"Rotate 90 Clockwise",
	"Rotate Image",
	"Crop",
	"Resize Image",
	"Resize Canvas",
	"Pixel Edit",
	"Property Edit",

	"Channel Filter",
	"Red Channel",
	"Green Channel",
	"Blue Channel",
	"Alpha Channel",
	"Channel As Intensity",

	"Details",
	"Tile",
	"Undo",
	"Redo",

	"Refresh / Reload Image",
	"Rename File",
	"Delete Image",
	"Delete Image Permanently",

	"Save As...",
	"Save All...",
	"Save Multi-Frame Image",
	"Save Contact Sheet...",

	"Menu Bar",
	"Navigation Bar",
	"Thumbnails...",
	#ifdef PACKAGE_SNAP
	"Open File Browser (No Snap Support)",
	#else
	"Open File Browser",
	#endif
	"Slideshow Timer",
	"Cheat Sheet",
	"Debug Log",

	"Profile Change",
	"Preferences...",
	"Key Bindings...",

	"Fullscreen",
	"Exit-Fullscreen | Exit-Basic-Profile",
	"Exit-Fullscreen | Exit-Basic-Profile | Quit",
	"Quit",

	"Open File",
	"Open Dir",
	"Meta Data",
	"Save",
	"UI Size Up",
	"UI Size Down",
	"Zoom Per Image",
	"Levels",
	"Copy",
	"Paste",
	"Save Extract Frames...",
	"Quantize...",
	"Slideshow Auto-Reshuffle",
	"Main Profile",
	"Basic Profile",
	"Kiosk Profile",
	"Refresh Directory",
	"Alt Profile",
	"Import Raw"
	// Add new entries here.
};
tStaticAssert(sizeof(Bindings::OperationDescriptions)/sizeof(*Bindings::OperationDescriptions) == int(Bindings::Operation::NumOperations));


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

	// Order is ctrl-alt-shift.
	const char* dispstr[] =
	{
		"",
		"Ctrl",
		"Alt",
		"Ctrl-Alt",
		"Shift",
		"Ctrl-Shift",
		"Alt-Shift",
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


bool Bindings::InputMap::AssignKey(int key, uint32 modifiers, Operation operation, bool onlyIfUnassigned)
{
	// If key already assigned to requested operation we're done.
	if (KeyTable[key].Operations[modifiers] == operation)
		return false;

	if (!onlyIfUnassigned || KeyTable[key].Operations[modifiers] == Operation::None)
		KeyTable[key].Operations[modifiers] = operation;

	return true;
}


void Bindings::InputMap::Reset(Viewer::Profile profile, bool onlyIfUnassigned)
{
	if (!onlyIfUnassigned)
		Clear();

	// Testing. Assign all keys to NextImage so we can test them all individually.
	// #define TEST_ALL_GLFW_KEY_ASSIGNMENTS
	#ifdef TEST_ALL_GLFW_KEY_ASSIGNMENTS
	for (int k = 0; k <= GLFW_KEY_LAST; k++)
	{
		if ((k >= GLFW_KEY_KP_0) && (k <= GLFW_KEY_KP_EQUAL))
			AssignKey(k, Modifier_None, Operation::PrevImage);

		else if (IsKeySupported(k))
			AssignKey(k, Modifier_None, Operation::NextImage);
	}
	return;
	#endif

	// Order is unimportant, but for consistency it's in same order as the Operation enum unless it is a new operation that
	// was added to the bottom of the enum (to keep binding config compatibility). In these cases we can re-order when we're
	// willing to increase the config version number and make enum match this. Sometimes more than one key
	// maps to the same operation and some operations are not bound by default, so the number of lines != num_ops.
	AssignKey(GLFW_KEY_SPACE,		Modifier_None,					Operation::NextImage,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_RIGHT,		Modifier_None,					Operation::NextImage,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_LEFT,		Modifier_None,					Operation::PrevImage,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_RIGHT,		Modifier_Ctrl,					Operation::SkipToLastImage,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_LEFT,		Modifier_Ctrl,					Operation::SkipToFirstImage,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_RIGHT,		Modifier_Alt,					Operation::NextImageFrame,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_LEFT,		Modifier_Alt,					Operation::PrevImageFrame,			onlyIfUnassigned);

	AssignKey(GLFW_KEY_RIGHT,		Modifier_Shift,					Operation::PixelRight,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_LEFT,		Modifier_Shift,					Operation::PixelLeft,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_DOWN,		Modifier_Shift,					Operation::PixelDown,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_UP,			Modifier_Shift,					Operation::PixelUp,					onlyIfUnassigned);

	AssignKey(GLFW_KEY_EQUAL,		Modifier_Shift,					Operation::UISizeInc,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_MINUS,		Modifier_Shift,					Operation::UISizeDec,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_EQUAL,		Modifier_Ctrl,					Operation::ZoomIn,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_MINUS,		Modifier_Ctrl,					Operation::ZoomOut,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_F,			Modifier_None,					Operation::ZoomFit,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_D,			Modifier_None,					Operation::ZoomDownscaleOnly,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_Z,			Modifier_None,					Operation::ZoomOneToOne,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_Z,			Modifier_Shift,					Operation::ZoomPerImage,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_P,			Modifier_Ctrl,					Operation::ResetPan,				onlyIfUnassigned);

	AssignKey(GLFW_KEY_COMMA,		Modifier_Ctrl,					Operation::FlipVertically,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_PERIOD,		Modifier_Ctrl,					Operation::FlipHorizontally,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_COMMA,		Modifier_None,					Operation::Rotate90Anticlockwise,	onlyIfUnassigned);
	AssignKey(GLFW_KEY_PERIOD,		Modifier_None,					Operation::Rotate90Clockwise,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_R,			Modifier_None,					Operation::RotateImage,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_SLASH,		Modifier_None,					Operation::Crop,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_R,			Modifier_Alt,					Operation::ResizeImage,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_R,			Modifier_Ctrl,					Operation::ResizeCanvas,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_A,			Modifier_None,					Operation::PixelEdit,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_E,			Modifier_None,					Operation::PropertyEdit,			onlyIfUnassigned);

	AssignKey(GLFW_KEY_GRAVE_ACCENT,Modifier_None,					Operation::ChannelFilter,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_1,			Modifier_None,					Operation::RedChannel,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_2,			Modifier_None,					Operation::GreenChannel,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_3,			Modifier_None,					Operation::BlueChannel,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_4,			Modifier_None,					Operation::AlphaChannel,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_5,			Modifier_None,					Operation::ChannelAsIntensity,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_L,			Modifier_None,					Operation::Levels,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_Q,			Modifier_None,					Operation::Quantize,				onlyIfUnassigned);

	AssignKey(GLFW_KEY_I,			Modifier_None,					Operation::Details,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_I,			Modifier_Ctrl,					Operation::ImportRaw,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_X,			Modifier_None,					Operation::MetaData,				onlyIfUnassigned);	// Update enum pending.
	AssignKey(GLFW_KEY_T,			Modifier_Ctrl,					Operation::Tile,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_Z,			Modifier_Ctrl,					Operation::Undo,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_Y,			Modifier_Ctrl,					Operation::Redo,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_C,			Modifier_Ctrl,					Operation::Copy,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_V,			Modifier_Ctrl,					Operation::Paste,					onlyIfUnassigned);

	AssignKey(GLFW_KEY_F5,			Modifier_None,					Operation::Refresh,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_F5,			Modifier_Ctrl,					Operation::RefreshDir,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_F5,			Modifier_Shift,					Operation::RefreshDir,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_F2,			Modifier_None,					Operation::Rename,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_DELETE,		Modifier_None,					Operation::Delete,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_DELETE,		Modifier_Shift,					Operation::DeletePermanent,			onlyIfUnassigned);

	AssignKey(GLFW_KEY_O,			Modifier_Ctrl,					Operation::OpenFile,				onlyIfUnassigned);	// Update enum pending.
	AssignKey(GLFW_KEY_O,			Modifier_Alt,					Operation::OpenDir,					onlyIfUnassigned);	// Update enum pending.
	AssignKey(GLFW_KEY_S,			Modifier_Ctrl,					Operation::Save,					onlyIfUnassigned);	// Update enum pending.
	AssignKey(GLFW_KEY_S,			Modifier_Shift,					Operation::SaveAs,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_S,			Modifier_Alt,					Operation::SaveAll,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_C,			Modifier_None,					Operation::SaveContactSheet,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_M,			Modifier_Ctrl,					Operation::SaveMultiFrameImage,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_E,			Modifier_Ctrl,					Operation::SaveExtractFrames,		onlyIfUnassigned);

	AssignKey(GLFW_KEY_M,			Modifier_None,					Operation::MenuBar,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_N,			Modifier_None,					Operation::NavBar,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_T,			Modifier_None,					Operation::Thumbnails,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_ENTER,		Modifier_None,					Operation::FileBrowser,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_S,			Modifier_None,					Operation::SlideshowTimer,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_L,			Modifier_Alt,					Operation::SlideshowReshuffle,		onlyIfUnassigned);
	AssignKey(GLFW_KEY_F1,			Modifier_None,					Operation::CheatSheet,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_L,			Modifier_Ctrl,					Operation::DebugLog,				onlyIfUnassigned);

	AssignKey(GLFW_KEY_1,			Modifier_Alt,					Operation::ProfileMain,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_2,			Modifier_Alt,					Operation::ProfileBasic,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_3,			Modifier_Alt,					Operation::ProfileKiosk,			onlyIfUnassigned);
	AssignKey(GLFW_KEY_4,			Modifier_Alt,					Operation::ProfileAlt,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_P,			Modifier_None,					Operation::Preferences,				onlyIfUnassigned);

	// This one is special and can't be reassigned or removed. This is because the user _could_ turn off the menu,
	// and remove all bindings to ToggleKeyBindings. The user would have to way of getting back to the bindings screen.
	// Having a guaranteed key combo for it solves these issues.
	AssignKey(GLFW_KEY_TAB,			Modifier_Ctrl | Modifier_Shift,	Operation::KeyBindings,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_TAB,			Modifier_None,					Operation::KeyBindings,				onlyIfUnassigned);

	AssignKey(GLFW_KEY_F11,			Modifier_None,					Operation::Fullscreen,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_ENTER,		Modifier_Alt,					Operation::Fullscreen,				onlyIfUnassigned);
	AssignKey(GLFW_KEY_ESCAPE,		Modifier_None,					Operation::Escape,					onlyIfUnassigned);
	AssignKey(GLFW_KEY_F4,			Modifier_Alt,					Operation::Quit,					onlyIfUnassigned);
}


bool Bindings::InputMap::FindModKey(int& key, uint32& mods, Operation op)
{
	for (int k = 0; k <= GLFW_KEY_LAST; k++)
	{
		uint32 m;
		if (KeyTable[k].FindOperationMods(m, op))
		{
			key = k;
			mods = m;
			return true;
		}
	}

	return false;
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


void Bindings::InputMap::Write(tExprWriter& writer) const
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
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::Bindings);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
	if (ImGui::Begin("Keyboard Bindings", popen, flags))
	{
		static int profileIdx = 0;
		if (justOpened)
			profileIdx = int(Config::GetProfile());

		float profileWidth		= Gutil::GetUIParamScaled(104.0f, 2.5f);
		float keyWidth			= Gutil::GetUIParamScaled(132.0f, 2.5f);
		float operationWidth	= Gutil::GetUIParamScaled(240.0f, 2.5f);
		float buttonWidth		= Gutil::GetUIParamScaled(75.0f, 2.5f);
		float removeAddSize		= Gutil::GetUIParamScaled(21.0f, 2.5f);
		float removeColSize		= Gutil::GetUIParamScaled(20.0f, 2.5f);

		ImGui::SetNextItemWidth(profileWidth);
		ImGui::Combo("##ProfileToEdit", &profileIdx, ProfileNamesLong, int(Profile::NumProfiles));
		Config::ProfileData* prof = nullptr;
		switch (profileIdx)
		{
			default:
			case int(Profile::Main):	prof = &Config::MainProfile;	break;
			case int(Profile::Basic):	prof = &Config::BasicProfile;	break;
			case int(Profile::Kiosk):	prof = &Config::KioskProfile;	break;
			case int(Profile::Alt):		prof = &Config::AltProfile;		break;
		}
		tAssert(prof);

		ImGui::SameLine();
		if (ImGui::Button("Reset", tVector2(buttonWidth, 0.0f)))
			prof->InputBindings.Reset(Viewer::Profile(profileIdx));
		Gutil::ToolTip("Resets the key bindings to default for the chosen profile.");

		ImGui::SameLine();
		if (ImGui::Button("Reset All", tVector2(buttonWidth, 0.0f)))
		{
			Config::MainProfile.InputBindings.Reset(Profile::Main);
			Config::BasicProfile.InputBindings.Reset(Profile::Basic);
			Config::KioskProfile.InputBindings.Reset(Profile::Kiosk);
			Config::AltProfile.InputBindings.Reset(Profile::Alt);
		}
		Gutil::ToolTip("Resets the key bindings to default for all profiles.");

		ImGui::SameLine();
		if (ImGui::Button("Set All", tVector2(buttonWidth, 0.0f)))
		{
			// Operator= deals with the object being the same one, so just copy them both over indescriminately.
			Config::MainProfile.InputBindings = prof->InputBindings;
			Config::BasicProfile.InputBindings = prof->InputBindings;
			Config::KioskProfile.InputBindings = prof->InputBindings;
			Config::AltProfile.InputBindings = prof->InputBindings;
		}
		Gutil::ToolTip("Copies the keybindings to all profiles. Useful if you want them all the same.");

		ImGui::SameLine();
		if (ImGui::Button("Close", tVector2(buttonWidth, 0.0f)))
		{
			if (popen)
				*popen = false;
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;

		float rowHeight			= Gutil::GetUIParamExtent(24.85f, 61.0f);
		int maxRowsToDisplay	= Gutil::GetUIParamExtent(16, 11);
		float keyTextOffsetY	= Gutil::GetUIParamScaled(3.0f, 2.5f);

		int totalAssigned = prof->InputBindings.GetTotalAssigned();
		const int numRowsToDisplay = tMin(maxRowsToDisplay, totalAssigned);
		tVector2 outerSize = ImVec2(0.0f, rowHeight + rowHeight * float(numRowsToDisplay));
		if (ImGui::BeginTable("KeyBindingTable", 3, tableFlags, outerSize))
		{
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, keyWidth);//ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed, operationWidth);
			ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed, removeColSize);
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableHeadersRow();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

			// The data is not easily randomly accessible which makes the ImGuiListClipper difficult to use.
			// If we were to implement a clipper, it would go here.

			// Multiple keys can be bound to the same operation (ex space and right-arrow for next image)
			// Multiple operations can NOT be bound to the same key, cuz that would be stupid -- well I guess
			// that's what a macro is -- but anyway, that would be a different system,
			//
			// This loop displays the currently bound keys (and modifiers) and what operation they are bound to.
			//
			// Key				Operation
			// Ctrl-Shift-R		Next Image[combo]		[-]
			// Space			Rotate Image[combo]		[-]
			// --------------------------------------------
			// Key[combo] Mods	Operation[combo]		[+] (brings up replace popup if necessary)
			for (int k = 0; k <= GLFW_KEY_LAST; k++)
			{
				KeyOps& keyops = prof->InputBindings.GetKeyOps(k);

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
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + keyTextOffsetY);
					ImGui::Text( GetModKeyText(k, m).Chr() );

					// The operation column.
					ImGui::TableSetColumnIndex(1);
					char oplabel[64]; tsPrintf(oplabel, "##op%d_%d", k, m);
					const char* opCurrDesc = GetOperationDesc(opCurr);
					tAssert(opCurrDesc);
					ImGui::SetNextItemWidth(operationWidth);

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
						const char* opDesc = GetOperationDesc(Operation::KeyBindings);
						tString opText;
						tsPrintf(opText, " %s (Permanent Binding)", opDesc);

						if (ImGui::BeginCombo(oplabel, opText.Chr(), ImGuiComboFlags_NoArrowButton))
						{
							bool sel = false;
							ImGui::Selectable("Rebinding Disabled", &sel);
							ImGui::EndCombo();
						}
					}
					
					// The remove button column.
					ImGui::TableSetColumnIndex(2);
					if (!permanent)
					{
						char blabel[64];
						tsPrintf(blabel, "-##b%d_%d", k, m);
						if (ImGui::Button(blabel, tVector2(removeAddSize, removeAddSize)))
							keyops.Operations[m] = Operation::None;
					}
				}
			}
			ImGui::EndTable();		
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
		ShowAddBindingSection(*prof, keyWidth, operationWidth, removeAddSize);
	}
	ImGui::End();
}


void Bindings::ShowAddBindingSection(Config::ProfileData& settings, float keyWidth, float operationWidth, float removeAddSize)
{
	uint32 tableFlags = ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
	const float rowHeight = 25.0f;
	tVector2 outerSize = ImVec2(0.0f, rowHeight);

	float removeColSize		= Gutil::GetUIParamScaled(20.0f, 2.5f);
	float buttonSize		= Gutil::GetUIParamScaled(80.0f, 2.5f);

	if (ImGui::BeginTable("KeyAssignTable", 3, tableFlags, outerSize))
	{
		ImGui::TableSetupColumn("##AssignKey", ImGuiTableColumnFlags_WidthFixed, keyWidth);
		ImGui::TableSetupColumn("##AssignOperation", ImGuiTableColumnFlags_WidthFixed, operationWidth);
		ImGui::TableSetupColumn("##Assign", ImGuiTableColumnFlags_WidthFixed, removeColSize);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		int comboFlags = ImGuiComboFlags_NoArrowButton;
		static uint32 addMods = 0;
		const char* addModsName = GetModifiersText(addMods);
		if (!addModsName)
			addModsName = "No Mods";
		ImGui::SetNextItemWidth(keyWidth*0.59);
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
		ImGui::SetNextItemWidth(keyWidth*0.34f);
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
		ImGui::SetNextItemWidth(operationWidth);
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
		if (ImGui::Button("+##AddBinding", tVector2(removeAddSize, removeAddSize)))
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
		Gutil::ToolTip("Adds the new keybinding. Warns if already assigned to something else.");

		bool isOpen = true;
		if (ImGui::BeginPopupModal("Key Assignment Warning", &isOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
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
			ImGui::Text(msg.Chr());
			ImGui::NewLine();

			if (ImGui::Button("Cancel##AssignWarn", tVector2(buttonSize, 0.0f)))
				ImGui::CloseCurrentPopup();

			if (!permanent)
			{
				ImGui::SameLine();

				float buttonOffset = Gutil::GetUIParamScaled(236.0f, 2.5f);
				ImGui::SetCursorPosX(buttonOffset);
				if (ImGui::Button("Replace##AssignWarn", tVector2(buttonSize, 0.0f)))
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
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::CheatSheet);
	ImGui::SetNextWindowBgAlpha(0.80f);

	float actionWidth = Gutil::GetUIParamScaled(106.0f, 2.5f);
	float operatWidth = Gutil::GetUIParamScaled(238.0f, 2.5f);
	float windowHeight = Gutil::GetUIParamExtent(380.0f, 940.0f);

	tVector2 windowSize(actionWidth+operatWidth, windowHeight);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, tVector2(0.0f, 1.0f));

	tString title;
	tsPrintf(title, "Cheat Sheet (%s Profile)", Config::GetProfileName());

	if (ImGui::Begin(title.Chr(), popen, flags))
	{
		Config::ProfileData& profile = Config::GetProfileData();
		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
		const int maxRowsToDisplay =17;
		const float rowHeight = Gutil::GetUIParamScaled(18.0f, 2.5f);
		int totalAssigned = profile.InputBindings.GetTotalAssigned();
		const int numRowsToDisplay = tMin(maxRowsToDisplay, totalAssigned);

		tVector2 outerSize = ImVec2(0.0f, rowHeight + rowHeight * float(numRowsToDisplay));
		if (ImGui::BeginTable("CheatSheetTable", 2, tableFlags, outerSize))
		{
			ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, actionWidth);
			ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed, operatWidth);
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableHeadersRow();

			// The data is not easily randomly accessible which makes the ImGuiListClipper difficult to use.
			// If we were to implement a clipper, it would go here.
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
				KeyOps& keyops = profile.InputBindings.GetKeyOps(k);
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
					ImGui::Text( GetModKeyText(k, m).Chr() );

					// Operation column.
					ImGui::TableSetColumnIndex(1);
					const char* opCurrDesc = GetOperationDesc(opCurr);
					tAssert(opCurrDesc);
					ImGui::Text(opCurrDesc);
				}
			}
			ImGui::EndTable();
		}

		tVector2 buttonSize = Gutil::GetUIParamScaled(tVector2(100.0f, 22.0f), 2.5f);
		tVector2 buttonPad(buttonSize.x*1.06f, buttonSize.y*1.22);
		ImGui::SetCursorPos(tVector2(ImGui::GetWindowContentRegionMax()) - buttonPad);
		if (ImGui::Button("Close", buttonSize))
		{
			if (popen)
				*popen = false;
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();
}


}
