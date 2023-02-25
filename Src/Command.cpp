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
	tCmdLine::tOption OptionAutoName	("Autogenerate output file names",		"autoname",		'a'			);

	tCmdLine::tOption OptionParamsAPNG	("Save parameters for APNG files",		"paramsAPNG",	2			);
	tCmdLine::tOption OptionParamsBMP	("Save parameters for BMP  files",		"paramsBMP",	1			);
	tCmdLine::tOption OptionParamsGIF	("Save parameters for GIF  files",		"paramsGIF",	8			);
	tCmdLine::tOption OptionParamsJPG	("Save parameters for JPG  files",		"paramsJPG",	1			);
	tCmdLine::tOption OptionParamsPNG	("Save parameters for PNG  files",		"paramsPNG",	1			);
	tCmdLine::tOption OptionParamsQOI	("Save parameters for QOI  files",		"paramsQOI",	2			);
	tCmdLine::tOption OptionParamsTGA	("Save parameters for TGA  files",		"paramsTGA",	2			);
	tCmdLine::tOption OptionParamsTIFF	("Save parameters for TIFF files",		"paramsTIFF",	3			);
	tCmdLine::tOption OptionParamsWEBP	("Save parameters for WEBP files",		"paramsWEBP",	3			);

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

	void DetermineInputTypes();																	// Step 1.
	void InputFilesAddUnique(const tSystem::tFileInfo&);

	void DetermineInputFiles();																	// Step 2.
	void GetItemsFromManifest(tList<tStringItem>& manifestItems, const tString& manifestFile);
	void ProcessInputItem(tList<tSystem::tFileInfo>& inputFiles, const tString& item);

	void PopulateImages();																		// Step 3.

	tSystem::tFileType DetermineOutType();														// Step 4.

	void DetermineOutSaveParameters(tSystem::tFileType);										// Step 5.
	void ParseSaveParametersAPNG();
	void ParseSaveParametersBMP();
	void ParseSaveParametersGIF();
	void ParseSaveParametersJPG();
	void ParseSaveParametersPNG();
	void ParseSaveParametersQOI();
	void ParseSaveParametersTGA();
	void ParseSaveParametersTIFF();
	void ParseSaveParametersWEBP();
	void SetImageSaveParameters(Viewer::Image&, tSystem::tFileType);

	tString DetermineOutputFilename(const tString& inName, tSystem::tFileType outType);

	tImage::tImageAPNG::SaveParams SaveParamsAPNG;
	tImage::tImageBMP::SaveParams  SaveParamsBMP;
	tImage::tImageGIF::SaveParams  SaveParamsGIF;
	tImage::tImageJPG::SaveParams  SaveParamsJPG;
	tImage::tImagePNG::SaveParams  SaveParamsPNG;
	tImage::tImageQOI::SaveParams  SaveParamsQOI;
	tImage::tImageTGA::SaveParams  SaveParamsTGA;
	tImage::tImageTIFF::SaveParams SaveParamsTIFF;
	tImage::tImageWEBP::SaveParams SaveParamsWEBP;

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


void Command::GetItemsFromManifest(tList<tStringItem>& manifestItems, const tString& manifestFile)
{
	// The manifest file still has the @ symbol in it.
	tString manFile = manifestFile;
	manFile.ExtractLeft(1);
	if (!tSystem::tFileExists(manFile))
		return;

	tString manifest;
	bool loaded = tSystem::tLoadFile(manFile, manifest);
	if (!loaded)
		return;

	manifest.Remove('\r');

	tList<tStringItem> lines;
	int numLines = tStd::tExplode(lines, manifest, '\n');
	if (numLines <= 0)
		return;

	for (tStringItem* line = lines.First(); line; line = line->Next())
	{
		if (line->IsEmpty())
			continue;

		// Line comments in manifest files start with a semicolon.
		if (line->Left(1) == ";")
			continue;

		manifestItems.Append(new tStringItem(*line));
	}
}


void Command::ProcessInputItem(tList<tSystem::tFileInfo>& inputFiles, const tString& item)
{
	tSystem::tFileInfo info;
	bool found = tSystem::tGetFileInfo(info, item);
	if (!found)
		return;

	if (info.Directory)
	{
		if (item == ".")
			tSystem::tFindFiles(inputFiles, "", InputTypes);
		else
			tSystem::tFindFiles(inputFiles, item, InputTypes);
	}
	else
	{
		// Convert to simple full absolute path.
		info.FileName = tSystem::tGetAbsolutePath(info.FileName);

		// Add existing files that are of a globally supported filetype. Note we ignore InputTypes
		// here because the user explicitely specified the file, so it must be what they want.
		tSystem::tFileType typ = tSystem::tGetFileType(item);
		if (Viewer::FileTypes_Load.Contains(typ))
			inputFiles.Append(new tSystem::tFileInfo(info));
	}
}


void Command::DetermineInputFiles()
{
	tList<tSystem::tFileInfo> inputFiles;

	// If no input files specified, use the current directory.
	if (!ParamInputFiles)
		tSystem::tFindFiles(inputFiles, "", InputTypes);

	for (tStringItem* fileItem = ParamInputFiles.Values.First(); fileItem; fileItem = fileItem->Next())
	{
		// If the fileItem starts with an 'at' symbol (@), we interpret it as a manifest file.
		if (fileItem->Left(1) == "@")
		{
			tList<tStringItem> manifestItems;
			GetItemsFromManifest(manifestItems, *fileItem);
			for (tStringItem* manifestItem = manifestItems.First(); manifestItem; manifestItem = manifestItem->Next())
				ProcessInputItem(inputFiles, *manifestItem);
		}
		else
		{
			ProcessInputItem(inputFiles, *fileItem);
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


void Command::DetermineOutSaveParameters(tSystem::tFileType fileType)
{
	// We only bother reading save parameter options for the type of files we will be saving.
	switch (fileType)
	{
		case tSystem::tFileType::APNG: ParseSaveParametersAPNG(); break;
		case tSystem::tFileType::BMP:  ParseSaveParametersBMP();  break;
		case tSystem::tFileType::GIF:  ParseSaveParametersGIF();  break;
		case tSystem::tFileType::JPG:  ParseSaveParametersJPG();  break;
		case tSystem::tFileType::PNG:  ParseSaveParametersPNG();  break;
		case tSystem::tFileType::QOI:  ParseSaveParametersQOI();  break;
		case tSystem::tFileType::TGA:  ParseSaveParametersTGA();  break;
		case tSystem::tFileType::TIFF: ParseSaveParametersTIFF(); break;
		case tSystem::tFileType::WEBP: ParseSaveParametersWEBP(); break;
	}
}


void Command::SetImageSaveParameters(Viewer::Image& image, tSystem::tFileType fileType)
{
	// We only bother setting save parameter options for the type of file requested.
	switch (fileType)
	{
		case tSystem::tFileType::APNG: image.SaveParamsAPNG = SaveParamsAPNG; break;
		case tSystem::tFileType::BMP:  image.SaveParamsBMP  = SaveParamsBMP;  break;
		case tSystem::tFileType::GIF:  image.SaveParamsGIF  = SaveParamsGIF;  break;
		case tSystem::tFileType::JPG:  image.SaveParamsJPG  = SaveParamsJPG;  break;
		case tSystem::tFileType::PNG:  image.SaveParamsPNG  = SaveParamsPNG;  break;
		case tSystem::tFileType::QOI:  image.SaveParamsQOI  = SaveParamsQOI;  break;
		case tSystem::tFileType::TGA:  image.SaveParamsTGA  = SaveParamsTGA;  break;
		case tSystem::tFileType::TIFF: image.SaveParamsTIFF = SaveParamsTIFF; break;
		case tSystem::tFileType::WEBP: image.SaveParamsWEBP = SaveParamsWEBP; break;
	}
}


void Command::ParseSaveParametersAPNG()
{
	if (!OptionParamsAPNG)
		return;

	tString formatStr					= OptionParamsAPNG.Arg1();
	switch (tHash::tHashString			(formatStr.Chr()))
	{
		case tHash::tHashCT("24"):		SaveParamsAPNG.Format = tImage::tImageAPNG::tFormat::BPP24;		break;
		case tHash::tHashCT("32"):		SaveParamsAPNG.Format = tImage::tImageAPNG::tFormat::BPP32;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsAPNG.Format = tImage::tImageAPNG::tFormat::Auto;		break;
	}

	tString overrideFrameDurationStr	= OptionParamsAPNG.Arg2();
	if (overrideFrameDurationStr == "*")
		SaveParamsAPNG.OverrideFrameDuration = -1;
	else
		SaveParamsAPNG.OverrideFrameDuration = overrideFrameDurationStr.AsInt32();
}


void Command::ParseSaveParametersBMP()
{
	if (!OptionParamsBMP)
		return;

	tString formatStr					= OptionParamsBMP.Arg1();
	switch (tHash::tHashString			(formatStr.Chr()))
	{
		case tHash::tHashCT("24"):		SaveParamsBMP.Format = tImage::tImageBMP::tFormat::BPP24;		break;
		case tHash::tHashCT("32"):		SaveParamsBMP.Format = tImage::tImageBMP::tFormat::BPP32;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsBMP.Format = tImage::tImageBMP::tFormat::Auto;		break;
	}
}


void Command::ParseSaveParametersGIF()
{
	if (!OptionParamsGIF)
		return;

	tString bppStr = OptionParamsGIF.Arg1();
	if (bppStr == "*")
	{
		SaveParamsGIF.Format = tImage::tPixelFormat::PAL8BIT;
	}
	else
	{
		int bpp = bppStr.AsInt32();
		tMath::tiClamp(bpp, 1, 8);
		SaveParamsGIF.Format = tImage::tPixelFormat(int(tImage::tPixelFormat::PAL8BIT) + bpp - 1);
	}

	tString quantStr = OptionParamsGIF.Arg2();
	switch (tHash::tHashString(quantStr.Chr()))
	{
		case tHash::tHashCT("fixed"):	SaveParamsGIF.Method = tImage::tQuantize::Method::Fixed;		break;
		case tHash::tHashCT("spatial"):	SaveParamsGIF.Method = tImage::tQuantize::Method::Spatial;		break;
		case tHash::tHashCT("neu"):		SaveParamsGIF.Method = tImage::tQuantize::Method::Neu;			break;
		case tHash::tHashCT("wu"):
		case tHash::tHashCT("*"):		SaveParamsGIF.Method = tImage::tQuantize::Method::Wu;			break;
	}

	tString loopStr = OptionParamsGIF.Arg3();
	if (loopStr == "*")
		SaveParamsGIF.Loop = 0;
	else
		SaveParamsGIF.Loop = loopStr.AsInt32();

	tString alphaThresholdStr = OptionParamsGIF.Arg4();
	if (alphaThresholdStr == "*")
		SaveParamsGIF.AlphaThreshold = -1;
	else
		SaveParamsGIF.AlphaThreshold = alphaThresholdStr.AsInt32();

	tString overrideFrameDurStr = OptionParamsGIF.Arg5();
	if (overrideFrameDurStr == "*")
		SaveParamsGIF.OverrideFrameDuration = -1;
	else
		SaveParamsGIF.OverrideFrameDuration = overrideFrameDurStr.AsInt32();

	tString ditherStr = OptionParamsGIF.Arg6();
	if (ditherStr == "*")
		SaveParamsGIF.DitherLevel = 0.0;
	else
		SaveParamsGIF.DitherLevel = ditherStr.AsDouble();

	tString filterSizeStr = OptionParamsGIF.Arg7();
	if (filterSizeStr == "*")
		SaveParamsGIF.FilterSize = 3;
	else
		SaveParamsGIF.FilterSize = filterSizeStr.AsInt32();
	if ((SaveParamsGIF.FilterSize != 1) && (SaveParamsGIF.FilterSize != 3) && (SaveParamsGIF.FilterSize != 5))
		SaveParamsGIF.FilterSize = 3;

	tString sampleFactorStr = OptionParamsGIF.Arg8();
	if (sampleFactorStr == "*")
		SaveParamsGIF.SampleFactor = 1;
	else
		SaveParamsGIF.SampleFactor = sampleFactorStr.AsInt32();
}


void Command::ParseSaveParametersJPG()
{
	if (!OptionParamsJPG)
		return;

	tString qualityStr					= OptionParamsJPG.Arg1();
	if (qualityStr == "*")
	{
		SaveParamsJPG.Quality = 95;
	}
	else
	{
		int quality = qualityStr.AsInt32();
		tMath::tiClamp(quality, 1, 100);
		SaveParamsJPG.Quality = quality;
	}
}


void Command::ParseSaveParametersPNG()
{
	if (!OptionParamsPNG)
		return;

	tString formatStr					= OptionParamsPNG.Arg1();
	switch (tHash::tHashString			(formatStr.Chr()))
	{
		case tHash::tHashCT("24"):		SaveParamsPNG.Format = tImage::tImagePNG::tFormat::BPP24;		break;
		case tHash::tHashCT("32"):		SaveParamsPNG.Format = tImage::tImagePNG::tFormat::BPP32;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsPNG.Format = tImage::tImagePNG::tFormat::Auto;		break;
	}
}


void Command::ParseSaveParametersQOI()
{
	if (!OptionParamsQOI)
		return;

	tString formatStr					= OptionParamsQOI.Arg1();
	switch (tHash::tHashString			(formatStr.Chr()))
	{
		case tHash::tHashCT("24"):		SaveParamsQOI.Format = tImage::tImageQOI::tFormat::BPP24;		break;
		case tHash::tHashCT("32"):		SaveParamsQOI.Format = tImage::tImageQOI::tFormat::BPP32;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsQOI.Format = tImage::tImageQOI::tFormat::Auto;		break;
	}

	tString spaceStr					= OptionParamsQOI.Arg2();
	switch (tHash::tHashString			(spaceStr.Chr()))
	{
		case tHash::tHashCT("srgb"):	SaveParamsQOI.Space = tImage::tImageQOI::tSpace::sRGB;			break;
		case tHash::tHashCT("lin"):		SaveParamsQOI.Space = tImage::tImageQOI::tSpace::Linear;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsQOI.Space = tImage::tImageQOI::tSpace::Auto;			break;
	}
}


void Command::ParseSaveParametersTGA()
{
	if (!OptionParamsTGA)
		return;

	tString formatStr					= OptionParamsTGA.Arg1();
	switch (tHash::tHashString			(formatStr.Chr()))
	{
		case tHash::tHashCT("24"):		SaveParamsTGA.Format = tImage::tImageTGA::tFormat::BPP24;		break;
		case tHash::tHashCT("32"):		SaveParamsTGA.Format = tImage::tImageTGA::tFormat::BPP32;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsTGA.Format = tImage::tImageTGA::tFormat::Auto;		break;
	}

	tString compStr						= OptionParamsTGA.Arg2();
	switch (tHash::tHashString			(compStr.Chr()))
	{
		case tHash::tHashCT("rle"):		SaveParamsTGA.Compression = tImage::tImageTGA::tCompression::RLE;	break;
		case tHash::tHashCT("none"):
		case tHash::tHashCT("*"):		SaveParamsTGA.Compression = tImage::tImageTGA::tCompression::None;	break;
	}
}


void Command::ParseSaveParametersTIFF()
{
	if (!OptionParamsTIFF)
		return;

	tString formatStr					= OptionParamsTIFF.Arg1();
	switch (tHash::tHashString			(formatStr.Chr()))
	{
		case tHash::tHashCT("24"):		SaveParamsTIFF.Format = tImage::tImageTIFF::tFormat::BPP24;		break;
		case tHash::tHashCT("32"):		SaveParamsTIFF.Format = tImage::tImageTIFF::tFormat::BPP32;		break;
		case tHash::tHashCT("auto"):
		case tHash::tHashCT("*"):		SaveParamsTIFF.Format = tImage::tImageTIFF::tFormat::Auto;		break;
	}

	tString zlibCompStr = OptionParamsTIFF.Arg2();
	if (zlibCompStr == "*")
		SaveParamsTIFF.UseZLibCompression = true;
	else
		SaveParamsTIFF.UseZLibCompression = zlibCompStr.AsBool();

	tString overrideFrameDurStr = OptionParamsTIFF.Arg3();
	if (overrideFrameDurStr == "*")
		SaveParamsTIFF.OverrideFrameDuration = -1;
	else
		SaveParamsTIFF.OverrideFrameDuration = overrideFrameDurStr.AsInt32();
}


void Command::ParseSaveParametersWEBP()
{
	if (!OptionParamsWEBP)
		return;

	tString lossyStr = OptionParamsWEBP.Arg1();
	if (lossyStr == "*")
		SaveParamsWEBP.Lossy = false;
	else
		SaveParamsWEBP.Lossy = lossyStr.AsBool();

	tString qualStr = OptionParamsWEBP.Arg2();
	if (qualStr == "*")
		SaveParamsWEBP.QualityCompstr = 90.0f;
	else
		SaveParamsWEBP.QualityCompstr = qualStr.AsFloat();

	tString overrideFrameDurStr = OptionParamsWEBP.Arg3();
	if (overrideFrameDurStr == "*")
		SaveParamsWEBP.OverrideFrameDuration = -1;
	else
		SaveParamsWEBP.OverrideFrameDuration = overrideFrameDurStr.AsInt32();
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
		tString intypes("Supported input file types: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
		{
			intypes += tsPrintf("%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
			if (intypes.Length()%80 > 80-5)
				intypes += tsPrintf("\n");
		}

		tString inexts("These cover file extensions: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
		{
			tList<tStringItem> extensions;
			tSystem::tGetExtensions(extensions, typ->FileType);
			for (tStringItem* ext = extensions.First(); ext; ext = ext->Next())
			{
				inexts += tsPrintf("%s ", ext->Chr());
				if (inexts.Length()%80 > 80-5)
					inexts += tsPrintf("\n");
			}
		}

		tString outtypes("Supported output file types: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Save.First(); typ; typ = typ->Next())
		{
			outtypes += tsPrintf("%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
			if (outtypes.Length()%80 > 80-5)
				outtypes += tsPrintf("\n");
		}

		tCmdLine::tPrintUsage(u8"Tristan Grimmer", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"U5AG3(MODE
You MUST call with -c or --cli to use this program in CLI mode.

Use the --help (-h) flag to print this help. To view generic command-line
syntax help use the --syntax (-s) flag. For example, to print syntax usage you
could call tacentview.exe -cs which expands to tacentview.exe -c -s

INPUT IMAGES
Each parameter of the command line shoud be a file or directory to process. You
may enter as many as you need. If no input files are specified, the current
directory is processed. You may also specify a manifest file containing images
to process using the @ symbol.

eg. @list.txt will load files from a manifest file called list.txt. Each line
of a manifest file should be the name of a file to process, the name of a dir
to process, start with a line-comment semicolon, or simply be empty.

When processing an entire directory of images you may specify what types of
input files to process. A type like 'tif' may have more than one accepted
extension (tif and tiff). The extension is not specified, the type is.
Use the --intype (-i) option to specify an input type. You may have more than
one -i to process multiple types. No -i means all supported types.

%s
%s

OUTPUT IMAGES
The output files are generated based on the input files and chosen operations.
The extract, flipbook, and combine operations consume the current set of input
images when they create their output images. Extract creates one or more new
images. Flipbook and combine create a single new image. All other operations
work on each image separately.

The type of the output images is specified with the --outtype option. If no
outtype is specified the default is tga.
%s

In cases where the input images are processed and not consumed, the output
filename matches the input filename except that the extension/type may be
different. Eg. Seascape.jpg would save as Seascape.tga if the outtype was tga.

By default if an output file already exists, it is not overwritten. To allow
overwrite use the --overwrite (-w) flag. To have the tool try a different
filename if it aready exists, use the --autoname (-a) flag. It will append the
string _NN to the name where NN is a number that keeps incrementing until no
existing file is found.

OUTPUT PARAMETERS
Different output image types have different features and may support different
parameters when saving. The following options cover setting these parameters.
These options do not have single-letter short forms. In all cases entering an
asterisk (*) means use the default value. In the desriptions below the *
indicates which is the default.

--paramsAPNG bpp dur
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  dur:  Frame duration override in milliseconds. -1* means use current frame
        duration. 0 or more means override all frames to supplied value.

--paramsBMP bpp
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.

--paramsGIF bpp qan loop alp dur dith filt samp
  bpp:  Bits per pixel from 1 to 8*. Results in palette colours from 2 to 256.
  qan:  Quantization method for palette generation.
        fixed:   Use a fixed colour palette for the chosen bpp. Low quality.
        spatial: Use scolorq algorithm. Slow but good for 32 colours or fewer.
        neu:     Use neuquant algorithm. Good for 64 colours or more.
        wu*:     Use XiaolinWu algorithm. Good for 64 colours or more.
  loop: Times to loop for animated GIFs. Choose 0* to loop forever. 
  alp:  Alpha threshold. Choose -1 for opaque(*). Otherwise in [0, 255] and
        if pixel alpha <= threshold the pixel is transparent.
  dur:  Frame duration override, -1* means no override. Otherwise in 1/100 s.
  dith: Dither level. Value in range 0.0 to 2.0+. Only applies to spatial
        quantization. 0.0* means auto-determine a good value for the current
        image based on its dimensions. Greater than zero means manually set the
        amount. A dither value of 0.1 results in no dithering. 2.0 results in
        significant dithering.
  filt: Filter size. Only applies to spatial quantization. Must be 1, 3*, or 5.
  samp: Sample factor. Only applies to neu quantization. 1* means whole image
        learning. 10 means 1/10 of image only. Max value is 30 (fastest).

--paramsJPG qual
  qual: Quality of jpeg as integer from 1 to 100. Default is 95*

--paramsPNG bpp
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.

--paramsQOI bpp spc
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  spc:  Colour space. srgb, lin, or auto*. Auto means keep the currenly loaded
        space. Use srgb for the sRGB space. Use lin for linear.

--paramsTGA bpp comp
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  comp: Compression. none* or rle. Use rle for run-length encoded.

--paramsTIFF bpp zlib dur
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  zlib: Use Zlib Compression. T* or F. This is a boolean. See note below.
  dur:  Frame duration override, -1* means no override. Otherwise units are ms.

--paramsWEBP loss qual dur
  loss: Generate lossy image. T or F*. This is a boolean. See note below.
  qual: Quality or compression amount in range 0.0 to 100.0. Default is 90.0*.
        Interpreted as quality for lossy images. Larger looks better but bigger
        files. Interpreted as compression strength for non-lossy. Larger values
        compress more but images take longer to generate.
  dur:  Frame duration override, -1* means no override. Otherwise units are ms.

For boolean arguments you may use "true", "t", "yes", "y", "on", "enable",
"enabled", "1", "+", and strings that represent non-zero integers as true.
These are case-insensitive. False is the result otherwise.
)U5AG3",
			intypes.Chr(), inexts.Chr(), outtypes.Chr()
		);

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
	DetermineOutSaveParameters(outType);

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
			// Set the image save parameters correctly. The user may have modified them from the command line.
			SetImageSaveParameters(*image, outType);
			bool success = image->Save(outFilename, outType, false);
			if (success)
				tPrintf("Saved File: %s\n", outFilename.Chr());
			else
				tPrintf("Failed Save: %s\n", outFilename.Chr());
		}

		image->Unload();
	}

	return 0;
}
