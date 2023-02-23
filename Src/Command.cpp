// Command.cpp
//
// Command line use of TacentView to perform batch image processing and conversions. The command-line interface supports
// operations such as quantization, rescaling/filtering, cropping, rotation, extracting frames, creating contact-sheets,
// amalgamating images into animated formats, and levels adjustments.
//
// Copyright (c) 2023 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <Foundation/tFundamentals.h>
#include <System/tCmdLine.h>
#include <System/tPrint.h>
#include <System/tFile.h>
#include "Version.cmake.h"
#include "Command.h"
#include "TacentView.h"
#include "Image.h"
#include "OpenSaveDialogs.h"


namespace Command
{
	tCmdLine::tParam  ParamInputFiles	("Input image files",					"inputfiles",	0			);

	// Note, -c and --cli are reserved.
	tCmdLine::tOption OptionOutType		("Output file type",					"outtype",		'o',	1	);
	tCmdLine::tOption OptionInType		("Input file type(s)",					"intype",		'i',	1	);
	tCmdLine::tOption OptionHelp		("Print help/usage information",		"help",			'h'			);
	tCmdLine::tOption OptionSyntax		("Print syntax help",					"syntax",		's'			);
	tCmdLine::tOption OptionOverwrite	("Overwrite existing output files",		"overwrite",	'w'			);
	tCmdLine::tOption OptionAutoName	("Autogenerate output file names",		"autonames",	'a'			);

	void BeginConsoleOutput();
	void EndConsoleOutput();
	#ifdef PLATFORM_WINDOWS
	bool AttachedToConsole = false;
	#endif
	struct ConsoleOutputScoped
	{
		ConsoleOutputScoped()			{ BeginConsoleOutput(); }
		~ConsoleOutputScoped()			{ EndConsoleOutput(); }
	};

	void DetermineInputTypes();															// Step 1.
	void InputFilesAddUnique(const tSystem::tFileInfo&);
	void DetermineInputFiles();															// Step 2.
	void PopulateImages();																// Step 3.
	tSystem::tFileType DetermineOutType();
	tString DetermineOutputFilename(const tString& inName, tSystem::tFileType outType);

	tSystem::tFileTypes InputTypes;
	tList<tSystem::tFileInfo> InputFiles;
	tList<Viewer::Image> Images;
}


void Command::BeginConsoleOutput()
{
	#ifdef PLATFORM_WINDOWS
	if (AttachedToConsole)
		return;

	// See https://www.tillett.info/2013/05/13/how-to-create-a-windows-program-that-works-as-both-as-a-gui-and-console-application/
	// We only want to attach to an existing console if the current out stream is not a pipe or disk
	// stream. This allows stuff like "prog.exe > hello.txt" and "prog.exe | prob2.exe" to work.
	bool wantStdOutAttach = true;
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle)
	{
		DWORD fileType = GetFileType(stdoutHandle);
		if ((fileType == FILE_TYPE_DISK) || (fileType == FILE_TYPE_PIPE))
			wantStdOutAttach = false;
	}

	bool wantStdErrAttach = true;
	HANDLE stderrHandle = GetStdHandle(STD_ERROR_HANDLE);
	if (stderrHandle)
	{
		DWORD fileType = GetFileType(stderrHandle);
		if ((fileType == FILE_TYPE_DISK) || (fileType == FILE_TYPE_PIPE))
			wantStdErrAttach = false;
	}

	if (wantStdOutAttach || wantStdErrAttach)
	{
		// Attach to parent console. We have currently disabled creating a new console as we never
		// want a standalone console window appearing.
		if (AttachConsole(ATTACH_PARENT_PROCESS))
			AttachedToConsole = true;
		// else if (AllocConsole())
		//  	attachedToConsole = true;

		if (AttachedToConsole)
		{
			// Redirect stdout and stdin to the console.
			if (wantStdOutAttach)
			{
				freopen("CONOUT$", "w", stdout);
				setvbuf(stdout, NULL, _IONBF, 0);
			}

			if (wantStdErrAttach)
			{
				freopen("CONOUT$", "w", stderr);
				setvbuf(stderr, NULL, _IONBF, 0);
			}
		}
	}
	#endif
}


void Command::EndConsoleOutput()
{
	#ifdef PLATFORM_WINDOWS
	if (!AttachedToConsole)
		return;

	// Send the "enter" key to the console to release the command prompt on the parent console.
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;				// Hardware scan code for key.
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Send the Enter key.
	ip.ki.wVk = 0x0D;				// Virtual-key code for the Enter key.
	ip.ki.dwFlags = 0;				// 0 for key press.
	SendInput(1, &ip, sizeof(INPUT));

	// Release the Enter key.
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
	#endif
}


void Command::DetermineInputTypes()
{
	if (OptionInType)
	{
		tList<tStringItem> types;
		OptionInType.GetArgs(types);
		for (tStringItem* typ = types.First(); typ; typ = typ->Next())
		{
			tSystem::tFileType ft = tSystem::tGetFileTypeFromName(*typ);
			InputTypes.Add(ft);
		}
	}
	else
	{
		InputTypes.Add(Viewer::FileTypes_Load);
	}
	tPrintf("Input types:");
	for (tSystem::tFileTypes::tFileTypeItem* typ = InputTypes.First(); typ; typ = typ->Next())
		tPrintf(" %s", tSystem::tGetFileTypeName(typ->FileType).Chr());
	tPrintf("\n");
}


void Command::InputFilesAddUnique(const tSystem::tFileInfo& infoToAdd)
{
	bool alreadyAdded = false;
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
	{
		#ifdef PLAYFORM_WINDOWS
		if (info->FileName.IsEqualCI(infoToAdd.FileName))
		#else
		if (info->FileName.IsEqual(infoToAdd.FileName))
		#endif
		{
			alreadyAdded = true;
			break;
		}
	}

	if (!alreadyAdded)
		InputFiles.Append(new tSystem::tFileInfo(infoToAdd));
}


void Command::DetermineInputFiles()
{
	tList<tSystem::tFileInfo> inputFiles;

	if (!ParamInputFiles)
		tSystem::tFindFiles(inputFiles, "", InputTypes);

	for (tStringItem* fileItem = ParamInputFiles.Values.First(); fileItem; fileItem = fileItem->Next())
	{
		tSystem::tFileInfo info;
		bool found = tSystem::tGetFileInfo(info, *fileItem);
		if (!found)
			continue;

		if (info.Directory)
		{
			if (*fileItem == ".")
				tSystem::tFindFiles(inputFiles, "", InputTypes);
			else
				tSystem::tFindFiles(inputFiles, *fileItem, InputTypes);
		}
		else
		{
			// Convert to simple full absolute path.
			info.FileName = tSystem::tGetAbsolutePath(info.FileName);

			// Only add existing files that are of a globally supported filetype.
			tSystem::tFileType typ = tSystem::tGetFileType(*fileItem);
			if (Viewer::FileTypes_Load.Contains(typ))
				inputFiles.Append(new tSystem::tFileInfo(info));
		}
	}

	for (tSystem::tFileInfo* info = inputFiles.First(); info; info = info->Next())
	{
		InputFilesAddUnique(*info);
	}

	tPrintf("Input Files:\n");
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
	{
		tPrintf("Input File: %s\n", info->FileName.Chr());
	}
}


void Command::PopulateImages()
{
	// This doesn't actually load the images. Just prepares them on the Images list.
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
	{
		Images.Append(new Viewer::Image(*info));
	}
}


tSystem::tFileType Command::DetermineOutType()
{
	tSystem::tFileType outType = tSystem::tFileType::TGA;
	if (OptionOutType)
	{
		outType = tSystem::tGetFileTypeFromName(OptionOutType.Arg1());
		if (outType == tSystem::tFileType::Invalid)
		{
			tPrintf("Invalid output file type specified. Defaulting to tga.\n");
			outType = tSystem::tFileType::TGA;
		}
	}
	return outType;
}


tString Command::DetermineOutputFilename(const tString& inName, tSystem::tFileType outType)
{
	tString outExt = tSystem::tGetExtension(outType);
	tString nonAutoName = tSystem::tGetDir(inName) + tSystem::tGetFileBaseName(inName) + "." + outExt;
	if (!OptionAutoName)
		return nonAutoName;

	// Autoname is true. Test up to 100 consecutive filenames.
	for (int nameIter = 0; nameIter < 100; nameIter++)
	{
		tString contender;
		if (nameIter == 0)
			contender = nonAutoName;
		else
			tsPrintf(contender, "%s%s_%02d.%s", tSystem::tGetDir(inName).Chr(), tSystem::tGetFileBaseName(inName).Chr(), nameIter, outExt.Chr());

		if (!tSystem::tFileExists(contender))
			return contender;
	}

	return nonAutoName;
}


int Command::Process()
{
	ConsoleOutputScoped scopedConsoleOutput;
	
	tPrintf("\nTacent View %d.%d.%d in CLI Mode.\nCall with --help for instructions.\n", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);
	if (OptionHelp)
	{
		tCmdLine::tPrintUsage(u8"Tristan Grimmer", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);
		tPrintf
		(
R"U5AG3(Additional Notes:
01234567890123456789012345678901234567890123456789012345678901234567890123456789
You MUST call with -c or --cli to use this program in CLI mode. Even if you
just want to print syntax usage you would need -c -s in the command line.

Specify a manifest file containing images to process using the @ symbol.
eg. @list.txt will load files from a manifest file called list.txt

When processing an entire directory of images you may specify what types of
input files to process. A type like 'tif' may have more than one accepted
extension (like tiff). You don't specify the extension, you specify the type.

If no input files are specified, the current directory is processed.
If no input types are specified, all supported types are processed.
)U5AG3"
		);

		tPrintf("Supported input file types:");
		int col = 27;
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
		{
			col += tPrintf(" %s", tSystem::tGetFileTypeName(typ->FileType).Chr());
			if (col > 80-4)
			{
				tPrintf("\n");
				col = 0;
			}
		}
		tPrintf("\n");

		tPrintf("Supported input file extensions:");
		col = 32;
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
		{
			tList<tStringItem> extensions;
			tSystem::tGetExtensions(extensions, typ->FileType);
			for (tStringItem* ext = extensions.First(); ext; ext = ext->Next())
			{
				tPrintf(" %s", ext->Chr());
				if (col > 80-4)
				{
					tPrintf("\n");
					col = 0;
				}
			}
		}
		tPrintf("\n");
		return 0;
	}

	if (OptionSyntax)
	{
		tCmdLine::tPrintSyntax();
		return 0;
	}

	// Determine what input types will be processed when specifying a directory.
	DetermineInputTypes();

	// Collect all input files into a single list.
	DetermineInputFiles();

	// Populates the Images list. Does not load the images.
	PopulateImages();

	tSystem::tFileType outType = DetermineOutType();

	// Processing. We process and save images individually to save memory.
	for (Viewer::Image* image = Images.First(); image; image = image->Next())
	{
		image->Load();

		// Process.

		// Determine out filename,
		tString outFilename = DetermineOutputFilename(image->Filename, outType);
		bool doSave = true;
		if (tSystem::tFileExists(outFilename) && !OptionOverwrite)
		{
			tPrintf("File %s exists. Not overwriting.\n", tSystem::tGetFileName(outFilename).Chr());
			doSave = false;
		}

		// Save.
		if (doSave)
		{
			bool success = image->Save(outFilename, outType);
			if (success)
				tPrintf("Saved File: %s\n", outFilename.Chr());
			else
				tPrintf("Failed Save: %s\n", outFilename.Chr());
		}

		image->Unload();
	}

	return 0;
}
