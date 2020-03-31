// tPlatform.h
//
// Tacent platform defines, architecture, and endianness detection. The Tacent library has some preprocessor define
// requirements. One of PLATFORM_NNN, ARCHITECTURE_NNN, and CONFIG_NNN need to be defined. If you haven't bothered
// to define these in the project file with a /D switch, an attempt is made to define them automatically for you.
//
// Copyright (c) 2004-2006, 2015, 2017, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <stdio.h>
#include <stdlib.h>
// No Tacent headers here. tPlatform.h is where the buck stops.


// Attempt to auto-detect platform.
#if (!defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX) && !defined(PLATFORM_MACOS) && !defined(PLATFORM_ANDROID) && !defined(PLATFORM_IOS))
	#if (defined(_M_AMD64) || defined(_M_IX86) || defined(_WIN32) || defined(_WIN64))
		#define PLATFORM_WINDOWS
	#elif defined(__linux__)
		#define PLATFORM_LINUX
	#endif
#endif


// Attempt to auto-detect archetecture.
#if (!defined(ARCHITECTURE_X64) && !defined(ARCHITECTURE_X86) && !defined(ARCHITECTURE_ARM))
	#if (defined(_M_AMD64) || defined(_WIN64) || defined(__x86_64__))
		#define ARCHITECTURE_X64								// For x86_64
	#elif defined(__i386)
		#define ARCHITECTURE_X86
	#endif
#endif


// Debug and Release configurations are only autodetected on windows.
#if (!defined(CONFIG_DEBUG) && !defined(CONFIG_DEVELOP) && !defined(CONFIG_PROFILE) && !defined(CONFIG_RELEASE) && !defined(CONFIG_SHIP))
	#if defined(_DEBUG)
		#define CONFIG_DEBUG
	#endif
	#if defined(NDEBUG)
		#define CONFIG_RELEASE
	#endif
#endif


// Turn off some annoying windows warnings.
#if (defined(PLATFORM_WINDOWS) && !defined(_CRT_SECURE_NO_DEPRECATE))
	#define _CRT_SECURE_NO_DEPRECATE
#endif


// Sanity check. Required defines must be present.
#if (!defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX) && !defined(PLATFORM_MACOS) && !defined(PLATFORM_ANDROID) && !defined(PLATFORM_IOS))
	#error You must define a supported platform.
#endif
#if (!defined(ARCHITECTURE_X64) && !defined(ARCHITECTURE_ARM))
	#error You must define a supported architecture.
#endif
#if (!defined(CONFIG_DEBUG) && !defined(CONFIG_DEVELOP) && !defined(CONFIG_PROFILE) && !defined(CONFIG_RELEASE) && !defined(CONFIG_SHIP))
	#error You must define a supported configuration.
#endif


// Define the endianness.
#if (defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS))
	#define ENDIAN_LITTLE
#else
	#define ENDIAN_BIG
#endif


#if defined(PLATFORM_WINDOWS)
	#include <xmmintrin.h>
#endif


// Many objects have a Pod member function that returns a plain-old-data version of itself. Useful for cases where POD
// is required, like to printf functions.
#define pod(x) (x.Pod())
#define tAlign16 __declspec(align(16))
#define tIsAligned16(addr) ((uint64(addr) & 0xF) == 0)


typedef unsigned char			uchar;
typedef signed char				schar;
typedef unsigned int			uint;
typedef unsigned short			ushort;
typedef unsigned long			ulong;


// Use the following integral types ONLY when you are relying on the specified number of bits. If you need more than
// 64 use a tBitField.
typedef signed char				int8;
typedef unsigned char			uint8;
typedef short					int16;
typedef unsigned short			uint16;
typedef int						int32;
typedef unsigned int			uint32;
typedef long long int			int64;
typedef unsigned long long int	uint64;


// Simple low-level file IO.
typedef FILE* tFileHandle;


enum class tPlatform
{
	Invalid																												= -1,
	First,
	Windows																												= First,
	Linux,													// Linux.
	MacOS,													// Apple's desktop OS. It's no longer called OSX.
	Android,												// Google Android.
	iOS,													// Apple iOS.
	All,													// Not counted as a separate platform.
	NumPlatforms																										= All
};

// Platforms as bitfields.
enum tPlatformFlag
{
	tPlatformFlag_None,
	tPlatformFlag_Windows																								= 1 << int(tPlatform::Windows),
	tPlatformFlag_Linux																									= 1 << int(tPlatform::Linux),
	tPlatformFlag_MacOS																									= 1 << int(tPlatform::MacOS),
	tPlatformFlag_Android																								= 1 << int(tPlatform::Android),
	tPlatformFlag_iOS																									= 1 << int(tPlatform::iOS),
	tPlatformFlag_All																									= 0xFFFFFFFF
};

struct tString;
tPlatform tGetPlatform();									// Based on required platform defines.
tPlatform tGetPlatform(const tString&);
const char* tGetPlatformName(tPlatform);
const char* tGetPlatformNameLong(tPlatform);

enum class tArchitecture
{
	Invalid																												= -1,
	x64,													// Desktop (not Itanium) 64bit architecture. i.e. AMD64.
	A64,													// Arm 64 bit. Also known as AArch64.
	NumArchitectures
};
tArchitecture tGetArchitecture();							// Based on defines.
const char* tGetArchitectureName(tArchitecture);
const char* tGetArchitectureNameLong(tArchitecture);

enum class tEndianness
{
	Invalid																												= -1,
	Big,
	Little
};

// If you want to know the endianness based on the platform define just test for ENDIAN_BIG or ENDIAN_LITTLE.
tEndianness tGetEndianness(tPlatform);						// Returns the native endianness of the tPlatform.
tEndianness tGetEndianness();								// Performs a test on the current architecture.

// These calls allow endianness swapping of variously sized types.
template<typename T> inline T tGetSwapEndian(const T&);
template<typename T> inline void tSwapEndian(T&);
template<typename T> inline void tSwapEndian(T*, int numItems);

// Converts to and from external data representation (XDR/Network) which is big-endian. Does not rely on the
// PLATFORM define by performing an endianness test.
template<typename T> inline T tNtoH(T val);
template<typename T> inline T tHtoN(T val);


// Implementation below this line.


template<typename T> inline T tGetSwapEndian(const T& val)
{
	union endianAccessor
	{
		T Data;
		uint8 Bytes[sizeof(T)];
	};
	endianAccessor& src = (endianAccessor&)val;
	endianAccessor dst;
	for (int d = 0; d < int(sizeof(T)); d++)
		dst.Bytes[d] = src.Bytes[sizeof(T) - d - 1];

	return dst.Data;
}


template<typename T> inline void tSwapEndian(T& val)
{
	union endianAccessor
	{
		T Data;
		uint8 Bytes[sizeof(T)];
	};
	endianAccessor& srcDst = (endianAccessor&)val;
	int numBytes = sizeof(T);
	for (int s = 0; s < (numBytes >> 1); s++)
	{
		uint8& a = srcDst.Bytes[s];
		uint8& b = srcDst.Bytes[numBytes - 1 - s];
		uint8 t = a;
		a = b;
		b = t;
	}
}


template<typename T> inline void tSwapEndian(T* a, int numItems)
{
	for (int i = 0; i < numItems; i++)
		tSwapEndian(a[i]);
}


inline tEndianness tGetEndianness()
{
	union
	{
		uint8 c[2];
		uint16 u;
	} a;

	a.c[1] = 0;
	a.c[0] = 1;

	if (a.u == 1)
		return tEndianness::Little;

	return tEndianness::Big;
}


template<typename T> inline T tNtoH(T val)
{
	if (tGetEndianness() == tEndianness::Big)
		return val;

	return tGetSwapEndian(val);
}


template<typename T> inline T tHtoN(T val)
{
	return NtoH(val);
}


// These defines mitigate Windows all-capital naming ugliness.
#ifdef PLATFORM_WINDOWS
#define ApiEntry APIENTRY
#define WinBitmap BITMAP
#define WinBitmapInfo BITMAPINFO
#define WinBitmapInfoHeader BITMAPINFOHEADER
#define ColorRef COLORREF
#define CreateStruct CREATESTRUCT
#define dWord DWORD
#define False FALSE
#define FileTime FILETIME
#define GlyphMetrics GLYPHMETRICS
#define hBitmap HBITMAP
#define hDc HDC
#define hFont HFONT
#define hGlrc HGLRC
#define hPBufferARB HPBUFFERARB
#define HiWord HIWORD
#define hResult HRESULT
#define hWinIcon HICON
#define hWinCursor HCURSOR
#define WindowHandle HWND
#define IconInfo ICONINFO
#define Infinite INFINITE
#define ItemIdList ITEMIDLIST
#define LargeInteger LARGE_INTEGER
#define Long LONG
#define LongPtr LONG_PTR
#define LoWord LOWORD
#define WinLParam LPARAM
#define lpCItemIdList LPCITEMIDLIST
#define lpCRect LPCRECT
#define lpEnumIdList LPENUMIDLIST
#define lpItemIdList LPITEMIDLIST
#define lpMalloc LPMALLOC
#define lpShellFolder LPSHELLFOLDER
#define lpVoid LPVOID
#define lResult LRESULT
#define MakeWord MAKEWORD
#define MaxPath MAX_PATH
#define Nmhdr NMHDR
#define OleChar OLECHAR
#define PixelFormatDescriptor PIXELFORMATDESCRIPTOR
#define Point POINT
#ifndef MAX_PLUGIN
#define Rect RECT
#endif
#define ShellFolder SHELLFOLDER
#define ShFileInfo SHFILEINFO
#define SystemTime SYSTEMTIME
#define WinSize SIZE
#define True TRUE
#define tStr TSTR
#define tChar TCHAR
#define WaitFailed WAIT_FAILED
#define WaitObject0 WAIT_OBJECT_0
#define WaitTimeout WAIT_TIMEOUT
#define Win32FindData WIN32_FIND_DATA
#define WinApi WINAPI
#define WinBool BOOL
#define WinHandle HANDLE
#define WinInstance HINSTANCE
#define WinIntPtr INT_PTR
#define WinMessage MSG
#define WinProc PROC
#define WinSocket SOCKET
#define WinWord WORD
#define WinCallback CALLBACK
#define WinWParam WPARAM
#define WsaData WSADATA
#define WinUUID __uuidof
#define CriticalSection CRITICAL_SECTION
#define snprintf _snprintf
inline bool WinSucceeded(long hresult)																					{ return (hresult >= 0) ? true : false; }
inline bool WinFailed(long hresult)																						{ return (hresult < 0) ? true : false; }
#endif
