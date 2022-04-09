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
#include "InputBindings.h"
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.
using namespace tStd;
using namespace tMath;
namespace Viewer
{


namespace Bind
{
	void InitKeyNameTable();

	bool KeyNameTableInitialized = false;
	const int MaxKeyNameLength = 16;
	char KeyNameTable[GLFW_KEY_LAST][MaxKeyNameLength];
}


void Bind::InitKeyNameTable()
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


const char* Bind::GetOperationDesc(Operation op)
{
	const char* descriptions[] =
	{
		"None",
		"Previous Image"
	};
	tAssert(sizeof(descriptions)/sizeof(*descriptions) == int(Operation::NumOperations));
	return descriptions[int(op)];
}


const char* Bind::GetModifiersText(uint32 modifiers)
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


const char* Bind::GetKeyName(int key)
{
	if (!KeyNameTableInitialized)
		InitKeyNameTable();

	return KeyNameTable[key];	
}


bool Bind::InputMap::AssignKey(int key, uint32 modifiers, Operation operation)
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


void Bind::InputMap::Reset()
{
	Clear();
	AssignKey(GLFW_KEY_LEFT, Modifier_None, Operation::PreviousImage);
}


}
