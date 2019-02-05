// tSync.h
//
// Synchronization objects that abstract platform implementation details. Since windows doesn't implement pthreads it
// makes sense to have a platform agnostic interface.
//
// Copyright (c) 2016 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once


class tMutex
{
public:
	tMutex();
	~tMutex();
	void Lock();
	void Unlock();

	// The absence of an accessor to get the lock state is intentional. This is because by the time you receive it, it may have
	// changed. You can't, therefore, do anything meaningful/sound based on the return value.
	
private:
	class Implementation;
	Implementation* Pimpl;
};
