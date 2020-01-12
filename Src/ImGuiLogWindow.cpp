// ImGuiLogWindow.cpp
//
// Simple log window for ImGui with filter. This is a modification of ImGui's log window -- licenced under MIT below.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.
//
// Copyright (c) 2014-2019 Omar Cornut
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <Math/tVector2.h>
#include "ImGuiLogWindow.h"
using namespace tMath;


void TexView::ImGuiLog::Clear()
{
	Buf.clear();
	LineOffsets.clear();
	LineOffsets.push_back(0);
}

void TexView::ImGuiLog::AddLog(const char* fmt, ...)
{
	int oldSize = Buf.size();
	va_list args;
	va_start(args, fmt);
	Buf.appendfv(fmt, args);
	va_end(args);

	for (int newSize = Buf.size(); oldSize < newSize; oldSize++)
		if (Buf[oldSize] == '\n')
			LineOffsets.push_back(oldSize + 1);

	ScrollToBottom = true;
}

void TexView::ImGuiLog::Draw(const char* title, bool* popen)
{
	if (ImGui::Button("Clear"))
		Clear();

	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	Filter.Draw("Filter", -100.0f);
	ImGui::Separator();
	ImGui::BeginChild("scrolling", tVector2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (copy)
		ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, tVector2(0, 0));
	const char* buf = Buf.begin();
	const char* bufEnd = Buf.end();
	if (Filter.IsActive())
	{
		for (int lineNo = 0; lineNo < LineOffsets.Size; lineNo++)
		{
			const char* lineStart = buf + LineOffsets[lineNo];
			const char* lineEnd = (lineNo + 1 < LineOffsets.Size) ? (buf + LineOffsets[lineNo + 1] - 1) : bufEnd;
			if (Filter.PassFilter(lineStart, lineEnd))
				ImGui::TextUnformatted(lineStart, lineEnd);
		}
	}
	else
	{
		// The simplest way to display the entire buffer is with ImGui::TextUnformatted(buf, buf_end); TextUnformatted
		// has specialization for large blobs of text and will fast-forward to skip non-visible lines. Here we instead
		// demonstrate using the clipper to only process lines that are within the visible area. If you have tens of
		// thousands of items and their processing cost is non-negligible, coarse clipping them on your side is
		// recommended.
		//
		// ImGuiListClipper requires a) random access into your data, and b) items all being the same height, both of
		// which we can handle since we an array pointing to the beginning of each line of text. When using the filter
		// (in the block of code above) we don't have random access into the data to display anymore, which is why we
		// don't use the clipper. Storing or skimming through the search result would make it possible and would be
		// recommended if you want to search through tens of thousands of entries.
		ImGuiListClipper clipper;
		clipper.Begin(LineOffsets.Size);
		while (clipper.Step())
		{
			for (int lineNo = clipper.DisplayStart; lineNo < clipper.DisplayEnd; lineNo++)
			{
				const char* lineStart = buf + LineOffsets[lineNo];
				const char* lineEnd = (lineNo + 1 < LineOffsets.Size) ? (buf + LineOffsets[lineNo + 1] - 1) : bufEnd;
				ImGui::TextUnformatted(lineStart, lineEnd);
			}
		}
		clipper.End();
	}
	ImGui::PopStyleVar();

	if (ScrollToBottom)
		ImGui::SetScrollHereY(1.0f);

	ScrollToBottom = false;
	ImGui::EndChild();
}
