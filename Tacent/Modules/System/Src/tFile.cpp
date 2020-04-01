// tFile.cpp
//
// This file contains a class implementation of a file on a disk. It derives from tStream. By passing around tStreams
// any user code can be oblivious to the type of stream. It may be a file on disk, it may be a file in a custom
// filesystem, it may be a pipe, or even a network resource.
//
// A path can refer to either a file or a directory. All paths use forward slashes as the separator. Input paths can
// use backslashes, but consistency in using forward slashes is advised. Directory path specifications always end with
// a trailing slash. Without the trailing separator the path will be interpreted as a file.
//
// Copyright (c) 2004-2006, 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tPlatform.h>
#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#elif defined(PLATFORM_LINUX)
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#endif
#include <filesystem>
#include "System/tTime.h"
#include "System/tFile.h"


namespace tSystem
{
	std::time_t tFileTimeToStdTime(std::filesystem::file_time_type tp);

	#ifdef PLATFORM_WINDOWS
	std::time_t tFileTimeToPosixEpoch(FILETIME);
	#endif
}


tFileHandle tSystem::tOpenFile(const char* filename, const char* mode)
{
	return fopen(filename, mode);
}


void tSystem::tCloseFile(tFileHandle f)
{
	if (!f)
		return;

	fclose(f);
}


int tSystem::tReadFile(tFileHandle f, void* buffer, int sizeBytes)
{
	// Load the entire thing into memory.
	int numRead = int(fread((char*)buffer, 1, sizeBytes, f));
	return numRead;
}


int tSystem::tWriteFile(tFileHandle f, const void* buffer, int sizeBytes)
{
	// Load the entire thing into memory.
	int numRead = int(fwrite((char*)buffer, 1, sizeBytes, f));
	return numRead;
}


int tSystem::tFileTell(tFileHandle handle)
{
	return int(ftell(handle));
}


int tSystem::tFileSeek(tFileHandle handle, int offsetBytes, tSeekOrigin seekOrigin)
{
	int origin = SEEK_SET;
	switch (seekOrigin)
	{
		case tSeekOrigin::Beginning:
			origin = SEEK_SET;
			break;

		case tSeekOrigin::Current:
			origin = SEEK_CUR;
			break;

		case tSeekOrigin::End:
			origin = SEEK_END;
			break;
	}
	
	return fseek(handle, long(offsetBytes), origin);
}


int tSystem::tGetFileSize(tFileHandle file)
{
	if (!file)
		return 0;

	tFileSeek(file, 0, tSeekOrigin::End);
	int fileSize = tFileTell(file);

	tFileSeek(file, 0, tSeekOrigin::Beginning);			// Go back to beginning.
	return fileSize;
}


int tSystem::tGetFileSize(const tString& filename)
{
	#ifdef PLATFORM_WINDOWS
	if (filename.IsEmpty())
		return 0;

	tString file(filename);
	file.Replace('/', '\\');
	uint prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

	Win32FindData fd;
	WinHandle h = FindFirstFile(file, &fd);

	// If file doesn't exist, h will be invalid.
	if (h == INVALID_HANDLE_VALUE)
	{
		SetErrorMode(prevErrorMode);
		return 0;
	}

	FindClose(h);
	SetErrorMode(prevErrorMode);
	return fd.nFileSizeLow;
	#else

	tFileHandle fd = tOpenFile(filename, "rb");
	int size = tGetFileSize(fd);
	tCloseFile(fd);

	return size;
	#endif
}


tSystem::tFileType tSystem::tGetFileType(const tString& file)
{
	if (file.IsEmpty())
		return tFileType::Unknown;

	tString ext = tGetFileExtension(file);
	return tGetFileTypeFromExtension(ext);
}


tSystem::tFileType tSystem::tGetFileTypeFromExtension(const tString& e)
{
	tString ext(e);
	if (ext.IsEmpty())
		return tFileType::Unknown;

	ext.ToLower();

	struct ExtTypePair
	{
		const char* Ext;
		tFileType Type;
	};
	
	ExtTypePair extToType[] =
	{
		{ "tga",		tFileType::TGA				},
		{ "bmp",		tFileType::BMP				},
		{ "png",		tFileType::PNG				},
		{ "gif",		tFileType::GIF				},
		{ "jpg",		tFileType::JPG				},
		{ "jpeg",		tFileType::JPG				},
		{ "tif",		tFileType::TIFF				},
		{ "tiff",		tFileType::TIFF				},
		{ "dds",		tFileType::DDS				},
		{ "hdr",		tFileType::HDR				},
		{ "rgbe",		tFileType::HDR				},
		{ "exr",		tFileType::EXR				},
		{ "pcx",		tFileType::PCX				},
		{ "wbmp",		tFileType::WBMP				},
		{ "wmf",		tFileType::WMF				},
		{ "jp2",		tFileType::JP2				},
		{ "jpc",		tFileType::JPC				},
		{ "tex",		tFileType::TEX				},
		{ "img",		tFileType::IMG				},
		{ "cub",		tFileType::CUB				},
		{ "ae2",		tFileType::TacentImage		},
		{ "aec",		tFileType::TacentImageCube	},
		{ "ae3",		tFileType::TacentImageVol	},
		{ "cfg",		tFileType::TacentConfig		},
	};
	int numExtensions = sizeof(extToType)/sizeof(*extToType);

	for (int e = 0; e < numExtensions; e++)
		if (ext == extToType[e].Ext)
			return extToType[e].Type;

	return tFileType::Unknown;	
}


bool tSystem::tFileExists(const tString& filename)
{
	#if defined(PLATFORM_WINDOWS)
	tString file(filename);
	file.Replace('/', '\\');

	int length = file.Length();
	if (file[ length - 1 ] == ':')
		file += "\\*";

	uint prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

	Win32FindData fd;
	WinHandle h = FindFirstFile(file, &fd);
	SetErrorMode(prevErrorMode);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	FindClose(h);
	if (fd.dwFileAttributes & _A_SUBDIR)
		return false;
	
	return true;

	#else
	tString file(filename);
	file.Replace('\\', '/');

	struct stat statbuf;
	return stat(file.Chars(), &statbuf) == 0;

	#endif
}


bool tSystem::tDirExists(const tString& dirname)
{
	if (dirname.IsEmpty())
		return false;
		
	tString dir = dirname;
	
	#if defined(PLATFORM_WINDOWS)
	dir.Replace('/', '\\');
	int length = dir.Length();
	if (dir[ length - 1 ] == '\\')
		dir[ length - 1 ] = '\0';

	length = dir.Length();

	// Can't quite remember what the * does. Needs testing.
	if (dir[ length - 1 ] == ':')
		dir += "\\*";

	uint prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

	Win32FindData fd;
	WinHandle h = FindFirstFile(dir, &fd);
	SetErrorMode(prevErrorMode);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	FindClose(h);
	if (fd.dwFileAttributes & _A_SUBDIR)
		return true;

	return false;

	#else
	dir.Replace('\\', '/');
	
	if (dir[dir.Length()-1] == '/')
		dir[dir.Length()-1] = '\0';

	std::filesystem::file_status fstat = std::filesystem::status(dir.Chars());
	
	return std::filesystem::is_directory(fstat);
	#endif
}


#if defined(PLATFORM_WINDOWS)
bool tSystem::tDriveExists(const tString& driveLetter)
{
	tString drive = driveLetter;
	drive.ToUpper();

	char driveLet = drive[0];
	if ((driveLet > 'Z') || (driveLet < 'A'))
		return false;

	ulong driveBits = GetLogicalDrives();
	if (driveBits & (0x00000001 << (driveLet-'A')))
		return true;

	return false;
}
#endif


bool tSystem::tIsFileNewer(const tString& filenameA, const tString& filenameB)
{
	#if defined(PLATFORM_WINDOWS)
	tString fileA(filenameA);
	fileA.Replace('/', '\\');

	tString fileB(filenameB);
	fileB.Replace('/', '\\');

	Win32FindData fd;
	WinHandle h = FindFirstFile(fileA, &fd);
	if (h == INVALID_HANDLE_VALUE)
		throw tFileError("Invalid file handle for file: " + fileA);

	FileTime timeA = fd.ftLastWriteTime;
	FindClose(h);

	h = FindFirstFile(fileB, &fd);
	if (h == INVALID_HANDLE_VALUE)
		throw tFileError("Invalid file handle for file: " + fileB);

	FileTime timeB = fd.ftLastWriteTime;
	FindClose(h);

	if (CompareFileTime(&timeA, &timeB) > 0)
		return true;

	#elif defined(PLAYFORM_LINUX)
	tToDo("Implement tISFileNewer.");

	#endif
	return false;
}


std::time_t tSystem::tFileTimeToStdTime(std::filesystem::file_time_type tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}


#ifdef PLATFORM_WINDOWS
std::time_t tSystem::tFileTimeToPosixEpoch(FILETIME filetime)
{
	LARGE_INTEGER date;
	date.HighPart = filetime.dwHighDateTime;
	date.LowPart = filetime.dwLowDateTime;

	// Milliseconds.
	LARGE_INTEGER adjust;
	adjust.QuadPart = 11644473600000 * 10000;
	
	// Removes the diff between 1970 and 1601.
	date.QuadPart -= adjust.QuadPart;

	// Converts back from 100-nanoseconds (Windows) to seconds (Posix).
	return date.QuadPart / 10000000;
}
#endif


bool tSystem::tGetFileInfo(tFileInfo& fileInfo, const tString& fileName)
{
	fileInfo.Clear();
	fileInfo.FileName = fileName;

	tString file(fileName);
	#ifdef PLATFORM_WINDOWS
	file.Replace('/', '\\');

	// Seems like FindFirstFile cannot deal with a trailing backslash when
	// trying to access directory information.  We remove it here.
	int l = file.Length();
	if (file[l-1] == '\\')
		file[l-1] = '\0';

	#else
	file.Replace('\\', '/');

	#endif

	#ifdef PLATFORM_WINDOWS
	Win32FindData fd;
	WinHandle h = FindFirstFile(file, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	FindClose(h);

	fileInfo.CreationTime = tFileTimeToPosixEpoch(fd.ftCreationTime);
	fileInfo.ModificationTime = tFileTimeToPosixEpoch(fd.ftLastWriteTime);
	fileInfo.AccessTime = tFileTimeToPosixEpoch(fd.ftLastAccessTime);

	// Occasionally, a file does not have a valid modification or access time.  The fileInfo struct
	// may, erronously, contain a modification or access time that is smaller than the creation time!
	// This happens, for example, with some files that have been transferred from a USB camera.
	// In this case, we simply use the creation time for the modification or access times.
	// Actually, this happens quite a bit, sometimes even if the times are valid!
	//
	// On seconds thought... we're going to leave the modification date alone.  Although I don't agree
	// with how the OS deals with this, I think the behaviour is best left untouched for mod time.
	// Basically, a file copied from, say, a mem card will get a current creation time, but the mod date
	// will be left at whatever the original file was.  Silly, although perhaps a little defensible.
	// We still correct any possible problems with access date though.
	if (fileInfo.AccessTime < fileInfo.CreationTime)
		fileInfo.AccessTime = fileInfo.CreationTime;

	tGetFileSize(file);
	fileInfo.FileSize = fd.nFileSizeHigh;
	fileInfo.FileSize <<= 32;
	fileInfo.FileSize |= fd.nFileSizeLow;

	fileInfo.ReadOnly = (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? true : false;
	fileInfo.Hidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? true : false;
	fileInfo.Directory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
	return true;

	#else

	Use stat() here on linux.
	std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(path, ec);
	fileInfo.ModificationTime = tFileTimeToStdTime(lastWriteTime);

	#endif
}


#ifdef PLATFORM_WINDOWS
bool tSystem::tGetFileDetails(tFileDetails& details, const tString& fullFileName)
{
	tString ffn = fullFileName;
	ffn.Replace('/', '\\');
	if (ffn[ ffn.Length() - 1 ] == '\\')
		ffn[ ffn.Length() - 1 ] = '\0';

	tString fileName = tSystem::tGetFileName(ffn);
	tString fileDir = tSystem::tGetDir(ffn);
	fileDir.Replace('/', '\\');

	if ((fileName.Length() == 2) && (fileName[1] == ':'))
	{
		fileName += "\\";
		fileDir = "";
	}

	// This interface is used for freeing up PIDLs.
	lpMalloc mallocInterface = 0;
	hResult result = SHGetMalloc(&mallocInterface);
	if (!mallocInterface)
		return false;

	// Get the desktop interface.
	IShellFolder* desktopInterface = 0;
	result = SHGetDesktopFolder(&desktopInterface);
	if (!desktopInterface)
	{
		mallocInterface->Release();
		return false;
	}

	// IShellFolder::ParseDisplayName requires the path name in Unicode wide characters.
	OleChar olePath[MaxPath];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fileDir.ConstText(), -1, olePath, MaxPath);

	// Parse path for absolute PIDL, and connect to target folder.
	lpItemIdList pidl = 0;
	result = desktopInterface->ParseDisplayName(0, 0, olePath, 0, &pidl, 0);
	if (result != S_OK)
	{
		desktopInterface->Release();
		mallocInterface->Release();
		return false;
	}

	IShellFolder2* shellFolder2 = 0;
	result = desktopInterface->BindToObject
	(
		pidl, 0,
		IID_IShellFolder2, (void**)&shellFolder2
	);

	// Release what we no longer need.
	desktopInterface->Release();
	mallocInterface->Free(pidl);

	if ((result != S_OK) || !shellFolder2)
	{
		mallocInterface->Release();
		return false;
	}

	OleChar unicodeName[MaxPath];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fileName.ConstText(), -1, unicodeName, MaxPath);
	lpItemIdList localPidl = 0;
	result = shellFolder2->ParseDisplayName(0, 0, unicodeName, 0, &localPidl, 0);
	if (result != S_OK)
	{
		mallocInterface->Release();
		shellFolder2->Release();
		return false;
	}

	int columnIndexArray[] =
	{
		// 0,				Name (Not Needed)
		1,				// Size / Type (Logical Drives)
		2,				// Type / Total Size (Logical Drives)
		3,				// Date Modified / Free Space (Logical Drives)
		4,				// Date Created / File System (Logical Drives)
		5,				// Date Accessed / Comments (Logical Drives)
		6,				// Attributes
		// 7,				Status (Not Needed)
		// 8,				Owner (Not Needed)
		9,				// Author
		10,			// Title
		11,			// Subject
		12,			// Category
		13,			// Pages
		14,			// Comments
		15,			// Copyright
		16,			// Artist
		17,			// Album Title
		18,			// Year
		19,			// Track Number
		20,			// Genre
		21,			// Duration
		22,			// Bit Rate
		23,			// Protected
		24,			// Camera Model
		25,			// Date Picture Taken
		26,			// Dimensions
		// 27,			Blank
		// 28,			Blank
		29,			// Episode Name
		30,			// Program Description
		// 31,			Blank
		32,			// Audio Sample Size
		33,			// Audio Sample Rate
		34,			// Channels
		// 35,			File State (Too Slow)
		// 36,			Rev (Useful but Too Slow)
		// 37,			Action (Too Slow)
		38,			// Company
		39,			// Description
		40,			// File VErsion
		41,			// Product Name
		42				// Product Version
	};

	const int maxDetailColumnsToTry = sizeof(columnIndexArray)/sizeof(*columnIndexArray);
	for (int c = 0; c < maxDetailColumnsToTry; c++)
	{
		int col = columnIndexArray[c];
		SHELLDETAILS shellDetail;

		// Get title.
		result = shellFolder2->GetDetailsOf(0, col, &shellDetail);
		if (result == S_OK)
		{
			tString title(33);
			StrRetToBuf(&shellDetail.str, localPidl, title.Text(), 32);

			// Get detail.
			tString detail(33);
			result = shellFolder2->GetDetailsOf(localPidl, col, &shellDetail);
			if (result == S_OK)
			{
				StrRetToBuf(&shellDetail.str, localPidl, detail.Text(), 32);

				// We only add the detail to the list if both title and detail are present.
				if (!title.IsEmpty() && !detail.IsEmpty())
				{
					details.DetailTitles.Append(new tStringItem(title));
					details.Details.Append(new tStringItem(detail));
				}
			}
		}
	}

	mallocInterface->Free(localPidl);

	// Release all remaining interface pointers.
	mallocInterface->Release();
	shellFolder2->Release();

	return true;
}


void tSystem::tSetFileOpenAssoc(const tString& program, const tString& extension, const tString& programOptions)
{
	tString baseName = tSystem::tGetFileBaseName(program);
	baseName.ToLower();

	tString keyString = "Software\\Classes\\Tacent_";
	keyString += baseName;
	keyString += "\\shell\\open\\command";

	HKEY key;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, keyString.ConstText(), 0, 0, 0, KEY_SET_VALUE, 0, &key, 0) == ERROR_SUCCESS)
	{
		// Create value string and set it.
		tString options = programOptions;
		if (options.IsEmpty())
			options = " ";
		else
			options = tString(" ") + options + " ";
		tString valString = tString("\"") + tSystem::tGetSimplifiedPath(program) + "\"" + options + "\"%1\"";
		valString.Replace('/', '\\');
		RegSetValueEx(key, "", 0, REG_SZ, (uint8*)valString.ConstText(), valString.Length()+1);
		RegCloseKey(key);
	}

	tString ext = extension;
	ext.ToLower();
	keyString = "Software\\Classes\\.";
	keyString += ext;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, keyString.ConstText(), 0, 0, 0, KEY_SET_VALUE, 0, &key, 0) == ERROR_SUCCESS)
	{
		tString valString = "Tacent_";
		valString += baseName;
		RegSetValueEx(key, "", 0, REG_SZ, (uint8*)valString.ConstText(), valString.Length()+1);
		RegCloseKey(key);
	}
}


void tSystem::tSetFileOpenAssoc(const tString& program, const tList<tStringItem>& extensions, const tString& programOptions)
{
	for (auto ext = extensions.First(); ext; ext = ext->Next())
		tSetFileOpenAssoc(program, *ext, programOptions);
}


tString tSystem::tGetFileOpenAssoc(const tString& extension)
{
	if (extension.IsEmpty())
		return tString();

	HKEY key;
	tString ext = extension;
	ext.ToLower();
	tString keyString = "Software\\Classes\\.";
	keyString += ext;
	tString appName(127);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyString.ConstText(), 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
	{
		ulong numBytesIO = 127;
		RegGetValue(key, "", 0, RRF_RT_REG_SZ | RRF_ZEROONFAILURE, 0, appName.Text(), &numBytesIO);
		RegCloseKey(key);
	}

	if (appName.IsEmpty())
		return tString();

	keyString = "Software\\Classes\\";
	keyString += appName;
	keyString += "\\shell\\open\\command";
	tString exeName(255);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyString.ConstText(), 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
	{
		ulong numBytesIO = 255;
		RegGetValue(key, "", 0, RRF_RT_REG_SZ | RRF_ZEROONFAILURE, 0, exeName.Text(), &numBytesIO);
		RegCloseKey(key);
	}

	return exeName;
}
#endif // PLATFORM_WINDOWS


tString tSystem::tGetSimplifiedPath(const tString& srcPath, bool ensureTrailingSlash)
{
	tString path = srcPath;
	path.Replace('\\', '/');

	// We do support filenames at the end.  However, if the name ends with a "." (or "..") we
	// know it is a folder and so add a trailing "/".
	if (path[path.Length()-1] == '.')
		path += "/";

	if (tIsAbsolutePath(path))
	{
		if ((path[0] >= 'a') && (path[0] <= 'z'))
			path[0] = 'A' + (path[0] - 'a');
	}

	// First we'll replace any "../" strings with "|/".  Note that pipe indicators are not allowed
	// in filenames so we can safely use them.
	int numUps = path.Replace("../", "|");

	// Now we can remove any "./" strings since all that's left will be up-directory markers.
	path.Remove("./");
	if (!numUps)
		return path;

	// We need to preserve leading '..'s so that paths like ../../Hello/There/ will work.
	int numLeading = path.RemoveLeading("|");
	numUps -= numLeading;
	for (int nl = 0; nl < numLeading; nl++)
		path = "../" + path;

	tString simp;
	for (int i = 0; i < numUps; i++)
	{
		simp += path.ExtractFirstWord('|');
		simp = tGetUpDir(simp);
	}

	tString res = simp + path;
	if (ensureTrailingSlash && (res.Length() > 1) && (res[res.Length()-1] != '/'))
		res += "/";

	return res;
}


bool tSystem::tIsAbsolutePath(const tString& path)
{
	if ((path.Length() > 1) && (path[1] == ':'))
		return true;

	if ((path.Length() > 0) && ((path[0] == '/') || (path[0] == '\\')))
		return true;

	return false;
}


tString tSystem::tGetFileName(const tString& filename)
{
	tString retStr(filename);
	retStr.Replace('\\', '/');

	int lastSlash = retStr.FindChar('/', true);

	if (lastSlash == -1)
		return retStr;

	return retStr.Suffix(lastSlash);
}


tString tSystem::tGetFileBaseName(const tString& filename)
{
	tString r = tGetFileName(filename);
	int dot = r.FindChar('.', true);

	if (dot == -1)
		return r;

	return r.Prefix(dot);
}


tString tSystem::tGetDir(const tString& path)
{
	tString ret(path);
	ret.Replace('\\', '/');

	// If string is empty or there is no filename on the end of the path just return what we have.
	if (ret.IsEmpty() || (ret[ret.Length()-1] == '/'))
		return ret;

	int lastSlash = ret.FindChar('/', true);

	// If there is no path, treat it as if it were a stand-alone file and return the current directory.
	if (lastSlash == -1)
		return tString("./");

	// At this point, we know there was a slash and that it isn't the last character, so
	// we know we aren't going out of bounds when we insert our string terminator after the slash.
	ret[ lastSlash + 1 ] = '\0';

	return ret;
}


tString tSystem::tGetUpDir(const tString& path, int levels)
{
	if (path.IsEmpty())
		return tString();

	tString ret(path);

	bool isNetLoc = false;
	ret.Replace('\\', '/');

	#ifdef PLATFORM_WINDOWS
	// Are we a network location starting with two slashes?
	if ((ret.Length() >= 2) && (ret[0] == '/') && (ret[1] == '/'))
		isNetLoc = true;
	#endif

	if (isNetLoc)
	{
		ret[0] = '\\';
		ret[1] = '\\';
	}

	tString upPath = ret;
	upPath[ upPath.Length() - 1 ] = '\0';

	for (int i = 0; i < levels; i++)
	{
		int lastSlash = upPath.FindChar('/', true);

		if (isNetLoc && (upPath.CountChar('/') == 1))
			lastSlash = -1;

		if (lastSlash == -1)
			return tString();

		upPath[lastSlash] = '\0';
	}

	upPath += "/";

	if (isNetLoc)
	{
		ret[0] = '/';
		ret[1] = '/';
	}
	return upPath;
}


tString tSystem::tGetRelativePath(const tString& basePath, const tString& path)
{
	#if defined(PLATFORM_WINDOWS)
	tString relLoc(MAX_PATH);
	tAssert(basePath[ basePath.Length() - 1 ] == '/');
	bool isDir = (path[ path.Length() - 1 ] == '/') ? true : false;

	tString basePathMod = basePath;
	basePathMod.Replace('/', '\\');

	tString pathMod = path;
	pathMod.Replace('/', '\\');

	int success = PathRelativePathTo
	(
		relLoc.Text(), basePathMod, FILE_ATTRIBUTE_DIRECTORY,
		pathMod, isDir ? FILE_ATTRIBUTE_DIRECTORY : 0
	);

	if (!success)
		return tString();

	relLoc.Replace('\\', '/');

	if (relLoc[0] == '/')
		return relLoc.ConstText() + 1;
	else
		return relLoc;

	#else
	tString refPath(basePath);
	tString absPath(path);
	
	int sizer = refPath.Length()+1;
	int sizea = absPath.Length()+1;
	if (sizea <= 1)
		return tString();
	if (sizer<= 1)
		return absPath;

	// From stackoverflow cuz I don't feel like thinking.
	// https://stackoverflow.com/questions/36173695/how-to-retrieve-filepath-relatively-to-a-given-directory-in-c	
	char relPath[1024];
	relPath[0] = '\0';
	char* pathr = refPath.Text();
	char* patha = absPath.Text();
	int inc = 0;

	for (; (inc < sizea) && (inc < sizer); inc += tStd::tStrlen(patha+inc)+1)
	{
		char* tokena = tStd::tStrchr(patha+inc, '/');
		char* tokenr = tStd::tStrchr(pathr+inc, '/');
		
		if (tokena) *tokena = '\0';
		if (tokenr) *tokenr = '\0';
		if (tStd::tStrcmp(patha+inc, pathr+inc) != 0)
			break;
	}

	for (int incr = inc; incr < sizer; incr += tStd::tStrlen(pathr+incr)+1)
	{
		tStd::tStrcat(relPath, "../");
		if (!tStd::tStrchr(refPath.Text()+incr, '/'))
			break;
	}

	if (inc < sizea)
		tStd::tStrcat(relPath, absPath.Text()+inc);
		
	return tString(relPath);
	#endif
}


tString tSystem::tGetAbsolutePath(const tString& pth, const tString& basePath)
{
	tString path(pth);
	path.Replace('\\', '/');
	if (tIsRelativePath(path))
	{
		if (basePath.IsEmpty())
			path = tGetCurrentDir() + path;
		else
			path = basePath + path;
	}

	return tGetSimplifiedPath(path);
}


tString tSystem::tGetLinuxPath(const tString& pth, const tString& mountPoint)
{
	tString path(pth);
	path.Replace('\\', '/');
	if (tIsAbsolutePath(path) && (path.Length() > 1) && (path[1] == ':') && !mountPoint.IsEmpty())
	{
		tString mnt = mountPoint;
		mnt.Replace('\\', '/');
		if (mnt[ mnt.Length() - 1 ] != '/')
			mnt += "/";

		char drive = tStd::tChrlwr(path[0]);
		path.ExtractPrefix(2);
		path = mnt + tString(drive) + path;
	}
	return path;
}


tString tSystem::tGetFileFullName(const tString& filename)
{
	tString file(filename);
	
	#if defined(PLATFORM_WINDOWS)
	file.Replace('/', '\\');
	tString ret(_MAX_PATH + 1);
	_fullpath(ret.Text(), file, _MAX_PATH);
	ret.Replace('\\', '/');
	
	#else

	file.Replace('\\', '/');
	tString ret(PATH_MAX + 1);
	realpath(file, ret.Text());	
	#endif

	return ret;
}


uint32 tSystem::tHashFileFast32(const tString& filename, uint32 iv)
{
	int dataSize = 0;
	uint8* data = tLoadFile(filename, nullptr, &dataSize);
	if (!data)
		return iv;

	uint32 hash = tMath::tHashDataFast32(data, dataSize, iv);
	delete[] data;
	return hash;
}


uint32 tSystem::tHashFile32(const tString& filename, uint32 iv)
{
	int dataSize = 0;
	uint8* data = tLoadFile(filename, nullptr, &dataSize);
	if (!data)
		return iv;

	uint32 hash = tMath::tHashData32(data, dataSize, iv);
	delete[] data;
	return hash;
}


uint64 tSystem::tHashFile64(const tString& filename, uint64 iv)
{
	int dataSize = 0;
	uint8* data = tLoadFile(filename, nullptr, &dataSize);
	if (!data)
		return iv;

	uint64 hash = tMath::tHashData64(data, dataSize, iv);
	delete[] data;
	return hash;
}


tuint128 tSystem::tHashFileMD5(const tString& filename, tuint128 iv)
{
	int dataSize = 0;
	uint8* data = tLoadFile(filename, nullptr, &dataSize);
	if (!data)
		return iv;

	tuint128 hash = tMath::tHashData128(data, dataSize, iv);
	delete[] data;
	return hash;
}


tuint256 tSystem::tHashFile256(const tString& filename, tuint256 iv)
{
	int dataSize = 0;
	uint8* data = tLoadFile(filename, nullptr, &dataSize);
	if (!data)
		return iv;

	tuint256 hash = tMath::tHashData256(data, dataSize, iv);
	delete[] data;
	return hash;
}


bool tSystem::tIsReadOnly(const tString& fileName)
{
	tString file(fileName);

	#if defined(PLATFORM_WINDOWS)
	file.Replace('/', '\\');
	int length = file.Length();
	if ((file[length - 1] == '/') || (file[length - 1] == '\\'))
		file[length - 1] = '\0';

	// The docs for this should be clearer!  GetFileAttributes returns INVALID_FILE_ATTRIBUTES if it
	// fails.  Rather dangerously, and undocumented, INVALID_FILE_ATTRIBUTES has a value of 0xFFFFFFFF.
	// This means that all attribute are apparently true!  This is very lame.  Thank goodness there aren't
	// 32 possible attributes, or there could be real problems.  Too bad it didn't just return 0 on error...
	// especially since they specifically have a FILE_ATTRIBUTES_NORMAL flag that is non-zero!
	ulong attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	return (attribs & FILE_ATTRIBUTE_READONLY) ? true : false;

	#else
	file.Replace('\\', '/');

	struct stat st;
	int errCode = stat(file, &st);
	if (errCode != 0)
		return false;

	bool w = (st.st_mode & S_IWUSR) ? true : false;
	bool r = (st.st_mode & S_IRUSR) ? true : false;
	return r && !w;

	#endif
}


bool tSystem::tSetReadOnly(const tString& fileName, bool readOnly)
{
	tString file(fileName);
	
	#if defined(PLATFORM_WINDOWS)	
	file.Replace('/', '\\');
	int length = file.Length();
	if ((file[length - 1] == '/') || (file[length - 1] == '\\'))
		file[length - 1] = '\0';

	ulong attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!(attribs & FILE_ATTRIBUTE_READONLY) && readOnly)
		SetFileAttributes(file, attribs | FILE_ATTRIBUTE_READONLY);
	else if ((attribs & FILE_ATTRIBUTE_READONLY) && !readOnly)
		SetFileAttributes(file, attribs & ~FILE_ATTRIBUTE_READONLY);

	attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!!(attribs & FILE_ATTRIBUTE_READONLY) == readOnly)
		return true;

	return false;

	#else
	file.Replace('\\', '/');
	
	struct stat st;
	int errCode = stat(file, &st);
	if (errCode != 0)
		return false;
	
	uint32 permBits = st.st_mode;

	// Set user R and clear user w. Leave rest unchanged.
	permBits |= S_IRUSR;
	permBits &= ~S_IWUSR;
	errCode = chmod(file, permBits);
	
	return (errCode == 0);

	#endif
}


bool tSystem::tIsHidden(const tString& path)
{
	// Even in windows treat files starting with a dot as hidden.
	tString fileName = tGetFileName(path);
	if ((fileName != ".") && (fileName != "..") && (fileName[0] == '.'))
		return true;

	// In windows also check the attribute.
	#if defined(PLATFORM_WINDOWS)
	tString file(path);
	file.Replace('/', '\\');
	int length = file.Length();
	if ((file[length - 1] == '/') || (file[length - 1] == '\\'))
		file[length - 1] = '\0';

	ulong attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	return (attribs & FILE_ATTRIBUTE_HIDDEN) ? true : false;

	#else
	return false;

	#endif
}


#if defined(PLATFORM_WINDOWS)
bool tSystem::tSetHidden(const tString& fileName, bool hidden)
{
	tString file(fileName);
	file.Replace('/', '\\');
	int length = file.Length();
	if ((file[length - 1] == '/') || (file[length - 1] == '\\'))
		file[length - 1] = '\0';

	ulong attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!(attribs & FILE_ATTRIBUTE_HIDDEN) && hidden)
		SetFileAttributes(file, attribs | FILE_ATTRIBUTE_HIDDEN);
	else if ((attribs & FILE_ATTRIBUTE_HIDDEN) && !hidden)
		SetFileAttributes(file, attribs & ~FILE_ATTRIBUTE_HIDDEN);

	attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!!(attribs & FILE_ATTRIBUTE_HIDDEN) == hidden)
		return true;

	return false;
}


bool tSystem::tIsSystem(const tString& fileName)
{
	tString file(fileName);
	file.Replace('/', '\\');
	int length = file.Length();
	if ((file[length - 1] == '/') || (file[length - 1] == '\\'))
		file[length - 1] = '\0';

	ulong attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	return (attribs & FILE_ATTRIBUTE_SYSTEM) ? true : false;
}


bool tSystem::tSetSystem(const tString& fileName, bool system)
{
	tString file(fileName);
	file.Replace('/', '\\');
	int length = file.Length();
	if ((file[length - 1] == '/') || (file[length - 1] == '\\'))
		file[length - 1] = '\0';

	ulong attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!(attribs & FILE_ATTRIBUTE_SYSTEM) && system)
		SetFileAttributes(file, attribs | FILE_ATTRIBUTE_SYSTEM);
	else if ((attribs & FILE_ATTRIBUTE_SYSTEM) && !system)
		SetFileAttributes(file, attribs & ~FILE_ATTRIBUTE_SYSTEM);

	attribs = GetFileAttributes(file);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (!!(attribs & FILE_ATTRIBUTE_SYSTEM) == system)
		return true;

	return false;
}


void tSystem::tGetDrives(tList<tStringItem>& drives)
{
	ulong ad = GetLogicalDrives();

	char driveLet = 'A';
	for (int i = 0; i < 26; i++)
	{
		if (ad & 0x00000001)
		{
			tStringItem* drive = new tStringItem(driveLet);
			*drive += ":";
			drives.Append(drive);
		}

		driveLet++;
		ad = ad >> 1;
	}
}


bool tSystem::tGetDriveInfo(tDriveInfo& driveInfo, const tString& drive, bool getDisplayName, bool getVolumeAndSerial)
{
	tString driveRoot = drive;
	driveRoot.ToUpper();

	if (driveRoot.Length() == 1)							// Assume string was of form "C"
		driveRoot += ":\\";
	else if (driveRoot.Length() == 2)						// Assume string was of form "C:"
		driveRoot += "\\";
	else													// Assume string was of form "C:/" or "C:\"
		driveRoot.Replace('/', '\\');

	uint driveType = GetDriveType(driveRoot);
	switch (driveType)
	{
		case DRIVE_NO_ROOT_DIR:
			return false;

		case DRIVE_REMOVABLE:
			if ((driveRoot == "A:\\") || (driveRoot == "B:\\"))
				driveInfo.DriveType = tDriveType::Floppy;
			else
				driveInfo.DriveType = tDriveType::Removable;
			break;

		case DRIVE_FIXED:
			driveInfo.DriveType = tDriveType::HardDisk;
			break;

		case DRIVE_REMOTE:
			driveInfo.DriveType = tDriveType::Network;
			break;

		case DRIVE_CDROM:
			driveInfo.DriveType = tDriveType::Optical;
			break;

		case DRIVE_RAMDISK:
			driveInfo.DriveType = tDriveType::RamDisk;
			break;

		case DRIVE_UNKNOWN:
		default:
			driveInfo.DriveType = tDriveType::Unknown;
			break;
	}

	if (getDisplayName)
	{
		// Here we try getting the name by using the shell api.  It should give the
		// same name as seen by windows explorer.
		SHFILEINFO fileInfo;
		fileInfo.szDisplayName[0] = '\0';
		SHGetFileInfo
		(
			driveRoot.ConstText(),
			0,
			&fileInfo,
			sizeof(SHFILEINFO),
			SHGFI_DISPLAYNAME
		);
		driveInfo.DisplayName = fileInfo.szDisplayName;
	}

	if (getVolumeAndSerial)
	{
		tString volumeInfoName(256);
		ulong componentLength = 0;
		ulong flags = 0;
		ulong serial = 0;
		int success = GetVolumeInformation
		(
			driveRoot,
			volumeInfoName.Text(),
			256,
			&serial,
			&componentLength,
			&flags,
			0,							// File system name not needed.
			0							// Buffer for system name is 0 long.
		);

		driveInfo.VolumeName = volumeInfoName;
		driveInfo.SerialNumber = serial;
	}

	return true;
}


bool tSystem::tSetVolumeName(const tString& drive, const tString& newVolumeName)
{
	tString driveRoot = drive;
	driveRoot.ToUpper();

	if (driveRoot.Length() == 1)				// Assume string was of form "C"
		driveRoot += ":\\";
	else if (driveRoot.Length() == 2)		// Assume string was of form "C:"
		driveRoot += "\\";
	else												// Assume string was of form "C:/" or "C:\"
		driveRoot.Replace('/', '\\');

	int success = SetVolumeLabel(driveRoot.ConstText(), newVolumeName.ConstText());
	return success ? true : false;
}


tString tSystem::tGetWindowsDir()
{
	tString windir(MAX_PATH);
	GetWindowsDirectory(windir.Text(), MAX_PATH);
	windir.Replace('\\', '/');
	if (windir[windir.Length()-1] != '/')
		windir += "/";

	return windir;
}


tString tSystem::tGetSystemDir()
{
	tString sysdir(MAX_PATH);
	GetSystemDirectory(sysdir.Text(), MAX_PATH);
	sysdir.Replace('\\', '/');
	if (sysdir[sysdir.Length()-1] != '/')
		sysdir += "/";

	return sysdir;
}
#endif


tString tSystem::tGetProgramDir()
{
	#if defined(PLATFORM_WINDOWS)
	tString result(MAX_PATH + 1);
	ulong l = GetModuleFileName(0, result.Text(), MAX_PATH);
	result.Replace('\\', '/');

	int bi = result.FindChar('/', true);
	tAssert(bi != -1);

	result[bi + 1] = '\0';
	return result;

	#elif defined(PLATFORM_LINUX)
	tString result(PATH_MAX+1);
	readlink("/proc/self/exe", result.Text(), PATH_MAX);
	
	int bi = result.FindChar('/', true);
	tAssert(bi != -1);
	result[bi + 1] = '\0';
	return result;

	#else
	return tString();

	#endif
}


tString tSystem::tGetProgramPath()
{
	#if defined(PLATFORM_WINDOWS)
	tString result(MAX_PATH + 1);
	ulong l = GetModuleFileName(0, result.Text(), MAX_PATH);
	result.Replace('\\', '/');
	return result;

	#elif defined(PLATFORM_LINUX)
	tString result(PATH_MAX+1);
	readlink("/proc/self/exe", result.Text(), PATH_MAX);
	return result;

	#else
	return tString();

	#endif
}


tString tSystem::tGetCurrentDir()
{
	#ifdef PLATFORM_WINDOWS
	tString r(MAX_PATH + 1);
	GetCurrentDirectory(MAX_PATH, r.Text());

	r.Replace('\\', '/');

	int l = r.Length();
	if (r[l - 1] != '/')
		r += "/";

	return r;

	#else
	tString r(PATH_MAX + 1);
	getcwd(r.Text(), PATH_MAX);
	return r;

	#endif
}


bool tSystem::tSetCurrentDir(const tString& directory)
{
	if (directory.IsEmpty())
		return false;

	tString dir = directory;

	#ifdef PLATFORM_WINDOWS
	dir.Replace('/', '\\');
	tString cd;

	// "." and ".." get left alone.
	if ((dir == ".") || (dir == ".."))
	{
		cd = dir;
	}
	else
	{
		if (dir.FindChar(':') != -1)
			cd = dir;
		else
			cd = ".\\" + dir;

		if (cd[cd.Length() - 1] != '\\')
			cd += "\\";
	}

	// So there is no dialog asking user to insert a floppy.
	uint prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	int success = SetCurrentDirectory(cd);
	SetErrorMode(prevErrorMode);

	return success ? true : false;

	#else
	dir.Replace('\\', '/');
	int errCode = chdir(dir.Chars());
	return (errCode == 0);

	#endif
}


bool tSystem::tFilesIdentical(const tString& fileA, const tString& fileB)
{
	auto localCloseFiles = [](tFileHandle a, tFileHandle b)
	{
		tCloseFile(a);
		tCloseFile(b);
	};

	tFileHandle fa = tOpenFile(fileA, "rb");
	tFileHandle fb = tOpenFile(fileB, "rb");
	if (!fa || !fb)
	{
		localCloseFiles(fa, fb);
		return false;
	}

	int faSize = tGetFileSize(fa);
	int fbSize = tGetFileSize(fb);
	if (faSize != fbSize)
	{
		localCloseFiles(fa, fb);
		return false;
	}

	uint8* bufA = new uint8[faSize];
	uint8* bufB = new uint8[fbSize];

	int numReadA = tReadFile(fa, bufA, faSize);
	int numReadB = tReadFile(fb, bufB, fbSize);
	tAssert((faSize + fbSize) == (numReadA + numReadB));

	for (int i = 0; i < faSize; i++)
	{
		if (bufA[i] != bufB[i])
		{
			localCloseFiles(fa, fb);
			delete[] bufA;
			delete[] bufB;
			return false;
		}
	}

	localCloseFiles(fa, fb);
	delete[] bufA;
	delete[] bufB;
	return true;
}


bool tSystem::tCreateFile(const tString& file)
{
	tFileHandle f = tOpenFile(file.ConstText(), "wt");
	if (!f)
		return false;

	tCloseFile(f);
	return true;
}


bool tSystem::tCreateFile(const tString& filename, const tString& contents)
{
	uint32 len = contents.Length();
	return tCreateFile(filename, (uint8*)contents.ConstText(), len);
}


bool tSystem::tCreateFile(const tString& filename, uint8* data, int dataLength)
{
	tFileHandle dst = tOpenFile(filename.ConstText(), "wb");
	if (!dst)
		return false;

	// Sometimes this needs to be done, for some mysterious reason.
	tFileSeek(dst, 0, tSeekOrigin::Beginning);

	// Write data and close file.
	int numWritten = tWriteFile(dst, data, dataLength);
	tCloseFile(dst);

	// Make sure it was created and an appropriate amount of bytes were written.
	bool verify = tFileExists(filename);
	return verify && (numWritten >= dataLength);
}


bool tSystem::tCreateDir(const tString& dir)
{
	tString dirPath = dir;
	
	#if defined(PLATFORM_WINDOWS)
	dirPath.Replace('/', '\\');

	bool success = ::CreateDirectory(dirPath.ConstText(), 0) ? true : false;
	if (!success)
		success = tDirExists(dirPath.ConstText());

	return success;

	#else
	dirPath.Replace('\\', '/');
	bool ok = std::filesystem::create_directory(dirPath.Chars());
	if (!ok)
		return tDirExists(dirPath.Chars());

	return ok;

	#endif
}


bool tSystem::tLoadFile(const tString& filename, tString& dst, char convertZeroesTo)
{
	if (!tFileExists(filename))
	{
		dst.Clear();
		return false;
	}

	int filesize = tGetFileSize(filename);
	if (filesize == 0)
	{
		dst.Clear();
		return true;
	}

	dst.Reserve(filesize);
	uint8* check = tLoadFile(filename, (uint8*)dst.Text());
	if ((check != (uint8*)dst.Text()) || !check)
		return false;

	if (convertZeroesTo != '\0')
	{
		for (int i = 0; i < filesize; i++)
			if (dst[i] == '\0')
			dst[i] = convertZeroesTo;
	}

	return true;
}


uint8* tSystem::tLoadFile(const tString& filename, uint8* buffer, int* fileSize, bool appendEOF)
{
	tFileHandle f = tOpenFile(filename.ConstText(), "rb");
	tAssert(f);

	int size = tGetFileSize(f);
	if (fileSize)
		*fileSize = size;

	if (size == 0)
	{
		// It is perfectly valid to load a file with no data (0 bytes big).
		// In this case we always return 0 even if a non-zero buffer was passed in.
		// The fileSize member will already be set if necessary.
		tCloseFile(f);
		return 0;
	}

	bool bufferAllocatedHere = false;
	if (!buffer)
	{
		int bufSize = appendEOF ? size+1 : size;
		buffer = new uint8[bufSize];
		bufferAllocatedHere = true;
	}

	int numRead = tReadFile(f, buffer, size);			// Load the entire thing into memory.
	tAssert(numRead == size);

	if (appendEOF)
		buffer[numRead] = EOF;

	tCloseFile(f);
	return buffer;
}


uint8* tSystem::tLoadFileHead(const tString& fileName, int& bytesToRead, uint8* buffer)
{
	tFileHandle f = tOpenFile(fileName, "rb");
	if (!f)
	{
		bytesToRead = 0;
		return buffer;
	}

	int size = tGetFileSize(f);
	if (!size)
	{
		tCloseFile(f);
		bytesToRead = 0;
		return buffer;
	}

	bytesToRead = (size < bytesToRead) ? size : bytesToRead;

	bool bufferAllocatedHere = false;
	if (!buffer)
	{
		buffer = new uint8[bytesToRead];
		bufferAllocatedHere = true;
	}

	// Load the first bytesToRead into memory.  We assume complete failure if the
	// number we asked for was not returned.
	int numRead = tReadFile(f, buffer, bytesToRead);
	if (numRead != bytesToRead)
	{
		if (bufferAllocatedHere)
		{
			delete[] buffer;
			buffer = 0;
		}

		tCloseFile(f);
		bytesToRead = 0;
		return buffer;
	}

	tCloseFile(f);
	return buffer;
}


bool tSystem::tCopyFile(const tString& dest, const tString& src, bool overWriteReadOnly)
{
	#if defined(PLATFORM_WINDOWS)
	int success = ::CopyFile(src, dest, 0);
	if (!success && overWriteReadOnly)
	{
		tSetReadOnly(dest, false);
		success = ::CopyFile(src, dest, 0);
	}

	return success ? true : false;

	#else
	std::filesystem::path pathFrom(src.Chars());
	std::filesystem::path pathTo(dest.Chars());
	bool success = std::filesystem::copy_file(pathFrom, pathTo);
	if (!success && overWriteReadOnly)
	{
		tSetReadOnly(dest, false);
		success = std::filesystem::copy_file(pathFrom, pathTo);
	}
		
	return success;

	#endif
}


bool tSystem::tRenameFile(const tString& dir, const tString& oldName, const tString& newName)
{
	#if defined(PLATFORM_WINDOWS)
	tString fullOldName = dir + oldName;
	fullOldName.Replace('/', '\\');

	tString fullNewName = dir + newName;
	fullNewName.Replace('/', '\\');

	int success = ::MoveFile(fullOldName, fullNewName);
	return success ? true : false;

	#else
	tString fullOldName = dir + oldName;
	fullOldName.Replace('\\', '/');
	std::filesystem::path oldp(fullOldName.Chars());

	tString fullNewName = dir + newName;
	fullNewName.Replace('\\', '/');
	std::filesystem::path newp(fullNewName.Chars());

	std::error_code ec;
	std::filesystem::rename(oldp, newp, ec);
	return !bool(ec);

	#endif
}


bool tSystem::tFindDirs(tList<tStringItem>& foundDirs, const tString& dir, bool includeHidden)
{
	#ifdef PLATFORM_WINDOWS

	// First lets massage fileName a little.
	tString massagedName = dir;
	if ((massagedName[massagedName.Length() - 1] == '/') || (massagedName[massagedName.Length() - 1] == '\\'))
		massagedName += "*.*";

	Win32FindData fd;
	WinHandle h = FindFirstFile(massagedName, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	tString path = tGetDir(massagedName);
	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || includeHidden)
			{
				// If the directory name is not "." or ".." then it's a real directory.
				// Note that you cannot just check for the first character not being "."  Some directories (and files)
				// may have a name that starts with a dot, especially if they were copied from a unix machine.
				tString fn(fd.cFileName);
				if ((fn != ".") && (fn != ".."))
					foundDirs.Append(new tStringItem(path + fd.cFileName + "/"));
			}
		}
	} while (FindNextFile(h, &fd));

	FindClose(h);
	if (GetLastError() != ERROR_NO_MORE_FILES)
		return false;

	#else
	tString dirPath(dir);
	if (dirPath.IsEmpty())
		dirPath = std::filesystem::current_path().u8string().c_str();

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dirPath.Text()))
	{
		if (!entry.is_directory())
			continue;
		tString foundDir(entry.path().u8string().c_str());
		if (includeHidden || !tIsHidden(foundDir))
			foundDirs.Append(new tStringItem(foundDir));
	}

	#endif
	return true;
}


bool tSystem::tFindFiles(tList<tStringItem>& foundFiles, const tString& dir, const tString& ext, bool includeHidden)
{
	#ifdef PLATFORM_WINDOWS

	// FindFirstFile etc seem to like backslashes better.
	tString dirStr(dir);
	dirStr.Replace('/', '\\');

	if (dirStr[dirStr.Length() - 1] != '\\')
		dirStr += "\\";

	tString path = dirStr + "*." + ext;

	Win32FindData fd;
	WinHandle h = FindFirstFile(path, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// It's not a directory... so it's actually a real file.
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || includeHidden)
			{
				// Holy obscure and annoying FindFirstFile bug! FindFirstFile("*.abc", ...) will also find
				// files like file.abcd. This isn't correct I guess we have to check the extension here.
				// FileMask is required to specify an extension, even if it is ".*"
				if (path[path.Length() - 1] != '*')
				{
					tString foundExtension = tGetFileExtension(fd.cFileName);
					if (ext.IsEqualCI(foundExtension))
						foundFiles.Append(new tStringItem(dirStr + fd.cFileName));
				}
				else
				{
					foundFiles.Append(new tStringItem(dirStr + fd.cFileName));
				}
			}
		}
	} while (FindNextFile(h, &fd));

	FindClose(h);
	if (GetLastError() != ERROR_NO_MORE_FILES)
		return false;

	#else
	tString dirPath(dir);

	// Use current directory if no dirPath supplied.
	if (dirPath.IsEmpty())
		dirPath = std::filesystem::current_path().u8string().c_str();

	if ((dirPath[dirPath.Length() - 1] == '/') || (dirPath[dirPath.Length() - 1] == '\\'))
		dirPath[dirPath.Length() - 1] = '\0';
		
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dirPath.Text()))
	{
		if (!entry.is_regular_file())
			continue;

		tString foundFile(entry.path().u8string().c_str());
		if (!ext.IsEmpty() && (!ext.IsEqualCI(tGetFileExtension(foundFile))))
			continue;

		if (includeHidden || !tIsHidden(foundFile))
			foundFiles.Append(new tStringItem(foundFile));
	}
	#endif

	return true;
}


bool tSystem::tFindFilesRecursive(tList<tStringItem>& foundFiles, const tString& dir, const tString& ext, bool includeHidden)
{
	#ifdef PLATFORM_WINDOWS
	// The windows functions seem to like backslashes better.
	tString pathStr(dir);
	pathStr.Replace('/', '\\');

	if (pathStr[pathStr.Length() - 1] != '\\')
		pathStr += "\\";

	tFindFiles(foundFiles, dir, ext, includeHidden);
	Win32FindData fd;

	// Look for all directories.
	WinHandle h = FindFirstFile(pathStr + "*.*", &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Don't recurse into hidden subdirectories if includeHidden is false.
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || includeHidden)
			{
				// If the directory name is not "." or ".." then it's a real directory.
				// Note that you cannot just check for the first character not being "."  Some directories (and files)
				// may have a name that starts with a dot, especially if they were copied from a unix machine.
				tString fn(fd.cFileName);
				if ((fn != ".") && (fn != ".."))
					tFindFilesRecursive(foundFiles, pathStr + fd.cFileName + "\\", ext, includeHidden);
			}
		}
	} while (FindNextFile(h, &fd));

	FindClose(h);
	if (GetLastError() != ERROR_NO_MORE_FILES)
		return false;

	#else
	for (const std::filesystem::directory_entry& entry: std::filesystem::recursive_directory_iterator(dir.Chars()))
	{
		if (!entry.is_regular_file())
			continue;

		tString foundFile(entry.path().u8string().c_str());
		if (!ext.IsEmpty() && (!ext.IsEqualCI(tGetFileExtension(foundFile))))
			continue;

		if (includeHidden || !tIsHidden(foundFile))
			foundFiles.Append(new tStringItem(foundFile));
	}
	#endif

	return true;
}


bool tSystem::tFindDirsRecursive(tList<tStringItem>& foundDirs, const tString& dir, bool includeHidden)
{
	#ifdef PLATFORM_WINDOWS
	tString pathStr(dir);
	pathStr.Replace('/', '\\');
	if (pathStr[pathStr.Length() - 1] != '\\')
		pathStr += "\\";

	tFindDirs(foundDirs, pathStr, includeHidden);
	Win32FindData fd;
	WinHandle h = FindFirstFile(pathStr + "*.*", &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Don't recurse into hidden subdirectories if includeHidden is false.
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || includeHidden)
			{
				// If the directory name is not "." or ".." then it's a real directory.
				// Note that you cannot just check for the first character not being "."  Some directories (and files)
				// may have a name that starts with a dot, especially if they were copied from a unix machine.
				tString fn(fd.cFileName);
				if ((fn != ".") && (fn != ".."))
					tFindDirsRecursive(foundDirs, pathStr + fd.cFileName + "\\", includeHidden);
			}
		}
	} while (FindNextFile(h, &fd));

	FindClose(h);
	if (GetLastError() != ERROR_NO_MORE_FILES)
		return false;

	#else
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(dir.Chars()))
	{
		if (!entry.is_directory())
			continue;

		tString foundDir(entry.path().u8string().c_str());

		if (includeHidden || !tIsHidden(foundDir))
			foundDirs.Append(new tStringItem(foundDir));
	}

	#endif
	return true;
}


bool tSystem::tDeleteFile(const tString& filename, bool deleteReadOnly, bool useRecycleBin)
{
	#ifdef PLATFORM_WINDOWS
	tString file(filename);
	file.Replace('/', '\\');
	if (deleteReadOnly)
		SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);

	if (!useRecycleBin)
	{
		if (DeleteFile(file))
			return true;
		else
			return false;
	}
	else
	{
		tString filenameDoubleNull = filename + "Z";
		filenameDoubleNull[filenameDoubleNull.Length()-1] = '\0';
		SHFILEOPSTRUCT operation;
		tStd::tMemset(&operation, 0, sizeof(operation));
		operation.wFunc = FO_DELETE;
		operation.pFrom = filenameDoubleNull.ConstText();
		operation.fFlags = FOF_ALLOWUNDO | FOF_NO_UI | FOF_NORECURSION;
		int errCode = SHFileOperation(&operation);
		return errCode ? false : true;
	}

	return true;
	
	#else
	if (!deleteReadOnly && tIsReadOnly(filename))
		return true;
		
	std::filesystem::path p(filename.Chars());
	
	if (useRecycleBin)
	{
		tString recycleDir = "~/.local/share/Trash/files/";
		if (tDirExists(recycleDir))
		{
			tString toFile = recycleDir + tGetFileName(filename);
			std::filesystem::path toPath(toFile.Chars());
			std::error_code ec;
			std::filesystem::rename(p, toPath, ec);
			return ec ? false : true;
		}
		
		return false;
	}

	return std::filesystem::remove(p);
	#endif
}


bool tSystem::tDeleteDir(const tString& dir, bool deleteReadOnly)
{
	#ifdef PLATFORM_WINDOWS
	// Are we done before we even begin?
	if (!tDirExists(dir))
		return false;

	tList<tStringItem> fileList;
	tFindFiles(fileList, dir);
	tStringItem* file = fileList.First();
	while (file)
	{
		tDeleteFile(*file, deleteReadOnly);		// We don't really care whether it succeeded or not.
		file = file->Next();
	}

	fileList.Empty();							// Clean up the file list.
	tString directory(dir);
	directory.Replace('/', '\\');

	Win32FindData fd;
	WinHandle h = FindFirstFile(directory + "*.*", &fd);
	if (h == INVALID_HANDLE_VALUE)
		return true;

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// If the directory name is not "." or ".." then it's a real directory.
			// Note that you cannot just check for the first character not being "."  Some directories (and files)
			// may have a name that starts with a dot, especially if they were copied from a unix machine.
			tString fn(fd.cFileName);
			if ((fn != ".") && (fn != ".."))
				tDeleteDir(dir + fd.cFileName + "/", deleteReadOnly);
		}
	} while (FindNextFile(h, &fd));

	bool deleteFilesOK = (GetLastError() == ERROR_NO_MORE_FILES) ? true : false;
	FindClose(h);

	if (deleteReadOnly)
		SetFileAttributes(directory, FILE_ATTRIBUTE_NORMAL);	// Directories can be read-only too.

	bool success = false;
	for (int delTry = 0; delTry < 32; delTry++)
	{
		if (RemoveDirectory(dir))
		{
			success = true;
			break;
		}

		// In some cases we might need to wait just a little and try again.  This can even take up to 10 seconds or so.
		// This seems to happen a lot when the target manager is streaming music, say, from the folder.
		else if (GetLastError() == ERROR_DIR_NOT_EMPTY)
		{
			tSystem::tSleep(500);
		}
		else
		{
			tSystem::tSleep(10);
		}
	}

	if (!success || !deleteFilesOK)
		return false;

	#else
	// Are we done before we even begin?
	if (!tDirExists(dir))
		return false;

	if (tIsReadOnly(dir) && !deleteReadOnly)
		return true;

	std::filesystem::path p(dir.Chars());
	std::error_code ec;
	uintmax_t numRemoved = std::filesystem::remove_all(p, ec);
	if (ec)
		return false;

	#endif

	return true;
}
