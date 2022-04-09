// Input.cpp
//
// Allows you to set keybindings for all TacentView operations.
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
#include "imgui.h"
#include "Input.h"
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL declarations.
using namespace tStd;
using namespace tMath;
using namespace Viewer;
using namespace Input;


const char* Input::GetOperationDescription(Operation op)
{
	const char* descriptions[] =
	{
		"None",
		"Previous Image",
		
	};
	tAssert(sizeof(descriptions)/sizeof(*descriptions) == int(Operation::NumOperations));
	return descriptions[int(op)];
}


const char* Input::GetModifiersDisplayString(uint32 modifiers)
{
	const char* dispstr[] =
	{
		"",
		"Shift",
		"Ctrl",
		"Ctrl-Shift",
		"Alt",
		"Alt-Shift",
		"Ctrl-Alt",
		"Ctrl-Alt-Shift"
	};
	tStaticAssert(tNumElements(dispstr) == Modifier_NumCombinations);
	return dispstr[modifiers];
}


bool InputMap::AssignKey(int key, uint32 modifiers, Operation operation)
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


void InputMap::Reset()
{
	Clear();
	AssignKey(GLFW_KEY_LEFT, Modifier_None, Operation::PreviousImage);
}

