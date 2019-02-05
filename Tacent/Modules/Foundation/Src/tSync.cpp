// tSync.cpp
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

#ifdef PLATFORM_WIN
#include <windows.h>
#else
#include <pthread.h>
#endif
#include "Foundation/tSync.h"


class tMutex::Implementation
{
public:
	Implementation();
	~Implementation();
	void Lock();
	void Unlock();

private:
	#ifdef PLATFORM_WIN
	CRITICAL_SECTION CriticalSection;
	#else
	pthread_mutex_t Mutex;
	#endif
};


inline tMutex::Implementation::Implementation()
{
	#ifdef PLATFORM_WIN
	InitializeCriticalSection(&CriticalSection);
	#else
	pthread_mutex_init(&Mutex, nullptr);
	#endif
}


inline tMutex::Implementation::~Implementation()
{
	#ifdef PLATFORM_WIN
	DeleteCriticalSection(&CriticalSection);
	#else
	pthread_mutex_destroy(&Mutex);
	#endif
}


inline void tMutex::Implementation::Lock()
{
	#ifdef PLATFORM_WIN
	EnterCriticalSection(&CriticalSection);
	#else
	pthread_mutex_lock(&Mutex);
	#endif
}


inline void tMutex::Implementation::Unlock()
{
	#ifdef PLATFORM_WIN
	LeaveCriticalSection(&CriticalSection);
	#else
	pthread_mutex_unlock(&Mutex);
	#endif
}


tMutex::tMutex() :
	Pimpl(new tMutex::Implementation())
{
}


tMutex::~tMutex()
{
	delete Pimpl;
}


void tMutex::Lock()
{
	Pimpl->Lock();
}


void tMutex::Unlock()
{
	Pimpl->Unlock();
}
