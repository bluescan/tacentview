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
#include "CommandOps.h"
#include "TacentView.h"


namespace Command
{
	tCmdLine::tParam  ParamInputFiles		("Input image files",				"inputfiles",	0			);

	// Note, -c and --cli are reserved.
	tCmdLine::tOption OptionHelp			("Print help/usage information",	"help",			'h'			);
	tCmdLine::tOption OptionSyntax			("Print syntax help",				"syntax",		's'			);
	tCmdLine::tOption OptionVerbosity		("Verbosity from 0 to 2",			"verbosity",	'v',	1	);

	tCmdLine::tOption OptionInType			("Input file type(s)",				"intype",		'i',	1	);
	tCmdLine::tOption OptionOperation		("Operation",						"op",					1	);
	tCmdLine::tOption OptionPostOperation	("Post operation",					"po",					1	);

	tCmdLine::tOption OptionOutType			("Output file type",				"outtype",		'o',	1	);
	tCmdLine::tOption OptionOverwrite		("Overwrite existing output files",	"overwrite",	'w'			);
	tCmdLine::tOption OptionAutoName		("Autogenerate output file names",	"autoname",		'a'			);
	tCmdLine::tOption OptionParamsAPNG		("Save parameters for APNG files",	"paramsAPNG",	2			);
	tCmdLine::tOption OptionParamsBMP		("Save parameters for BMP  files",	"paramsBMP",	1			);
	tCmdLine::tOption OptionParamsGIF		("Save parameters for GIF  files",	"paramsGIF",	8			);
	tCmdLine::tOption OptionParamsJPG		("Save parameters for JPG  files",	"paramsJPG",	1			);
	tCmdLine::tOption OptionParamsPNG		("Save parameters for PNG  files",	"paramsPNG",	1			);
	tCmdLine::tOption OptionParamsQOI		("Save parameters for QOI  files",	"paramsQOI",	2			);
	tCmdLine::tOption OptionParamsTGA		("Save parameters for TGA  files",	"paramsTGA",	2			);
	tCmdLine::tOption OptionParamsTIFF		("Save parameters for TIFF files",	"paramsTIFF",	3			);
	tCmdLine::tOption OptionParamsWEBP		("Save parameters for WEBP files",	"paramsWEBP",	3			);
	tCmdLine::tOption OptionEarlyExit		("Early exit / no skipping",		"earlyexit",	'e'			);
	tCmdLine::tOption OptionSkipUnchanged	("Don't save unchanged files",		"skipunchanged",'k'			);

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
	void ParseInputItem(tList<tSystem::tFileInfo>& inputFiles, const tString& item);

	void PopulateOperations();
	void PopulatePostOperations();
	void PopulateImagesList();																	// Step 3.
	bool ProcessOperationsOnImage(Viewer::Image&);												// Applies all the operations (in order) to the supplied image.

	void DetermineOutType();																	// Step 4.

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
	tList<Operation> Operations;
	tList<PostOperation> PostOperations;
	tSystem::tFileType OutType;
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

	tPrintfFull("Input types:");
	for (tSystem::tFileTypes::tFileTypeItem* typ = InputTypes.First(); typ; typ = typ->Next())
		tPrintfFull(" %s", tSystem::tGetFileTypeName(typ->FileType).Chr());
	tPrintfFull("\n");
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


void Command::ParseInputItem(tList<tSystem::tFileInfo>& inputFiles, const tString& item)
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
				ParseInputItem(inputFiles, *manifestItem);
		}
		else
		{
			ParseInputItem(inputFiles, *fileItem);
		}
	}

	for (tSystem::tFileInfo* info = inputFiles.First(); info; info = info->Next())
	{
		InputFilesAddUnique(*info);
	}

	tPrintfFull("Input Files:\n");
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
		tPrintfFull("Input File: %s\n", info->FileName.Chr());
}


void Command::PopulateImagesList()
{
	// This doesn't actually load the images. Just prepares them on the Images list.
	// It also turns off the undo-stack since we don't use that in CLI mode.
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
	{
		Viewer::Image* newImage = new Viewer::Image(*info);
		newImage->SetUndoEnabled(false);
		Images.Append(newImage);
	}
}


void Command::PopulateOperations()
{
	if (!OptionOperation)
		return;

	tList<tStringItem> opstrings;
	OptionOperation.GetArgs(opstrings);
	for (tStringItem* opstr = opstrings.First(); opstr; opstr = opstr->Next())
	{
		// Now we need to parse something of the form: resize[640,*] or rotate[45]
		tString str = *opstr;
		tString op = str.ExtractLeft('[');
		tString args = str.ExtractLeft(']');

		switch (tHash::tHashString(op))
		{
			case tHash::tHashCT("pixel"):		Operations.Append(new OperationPixel(args));		break;
			case tHash::tHashCT("resize"):		Operations.Append(new OperationResize(args));		break;
			case tHash::tHashCT("canvas"):		Operations.Append(new OperationCanvas(args));		break;
			case tHash::tHashCT("aspect"):		Operations.Append(new OperationAspect(args));		break;
			case tHash::tHashCT("deborder"):	Operations.Append(new OperationDeborder(args));		break;
			case tHash::tHashCT("crop"):		Operations.Append(new OperationCrop(args));			break;
			case tHash::tHashCT("flip"):		Operations.Append(new OperationFlip(args));			break;
			case tHash::tHashCT("rotate"):		Operations.Append(new OperationRotate(args));		break;
			case tHash::tHashCT("levels"):		Operations.Append(new OperationLevels(args));		break;
			case tHash::tHashCT("contrast"):	Operations.Append(new OperationContrast(args));		break;
			case tHash::tHashCT("brightness"):	Operations.Append(new OperationBrightness(args));	break;
			case tHash::tHashCT("quantize"):	Operations.Append(new OperationQuantize(args));		break;
			case tHash::tHashCT("channel"):		Operations.Append(new OperationChannel(args));		break;
			case tHash::tHashCT("swizzle"):		Operations.Append(new OperationSwizzle(args));		break;
			case tHash::tHashCT("extract"):		Operations.Append(new OperationExtract(args));		break;
		}
	}
}


void Command::PopulatePostOperations()
{
	if (!OptionPostOperation)
		return;

	tList<tStringItem> postrings;
	OptionPostOperation.GetArgs(postrings);
	for (tStringItem* postr = postrings.First(); postr; postr = postr->Next())
	{
		// Now we need to parse something of the form: combine[2,*] or contact[7]
		tString str = *postr;
		tString po = str.ExtractLeft('[');
		tString args = str.ExtractLeft(']');

		switch (tHash::tHashString(po))
		{
			case tHash::tHashCT("combine"):		PostOperations.Append(new PostOperationCombine(args));		break;
			case tHash::tHashCT("contact"):		PostOperations.Append(new PostOperationContact(args));		break;
		}
	}
}


bool Command::ProcessOperationsOnImage(Viewer::Image& image)
{
	if (!image.IsLoaded())
		return false;

	bool somethingFailed = false;
	for (Operation* operation = Operations.First(); operation; operation = operation->Next())
	{
		if (!operation->Valid)
			continue;
		bool success = operation->Apply(image);
		if (!success)
			somethingFailed = true;
	}
	return !somethingFailed;
}


void Command::DetermineOutType()
{
	OutType = tSystem::tFileType::TGA;
	if (OptionOutType)
	{
		OutType = tSystem::tGetFileTypeFromName(OptionOutType.Arg1());
		if (OutType == tSystem::tFileType::Invalid)
		{
			tPrintfNorm("Invalid output file type specified. Defaulting to tga.\n");
			OutType = tSystem::tFileType::TGA;
		}
	}
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
			tsPrintf(contender, "%s%s_%03d.%s", tSystem::tGetDir(inName).Chr(), tSystem::tGetFileBaseName(inName).Chr(), nameIter, outExt.Chr());

		if (!tSystem::tFileExists(contender))
			return contender;
	}

	return nonAutoName;
}


int Command::Process()
{
	ConsoleOutputScoped scopedConsoleOutput;

	// Default is normal (1) verbosity.
	int verbLevel = 1;
	if (OptionVerbosity)
	{
		tString verbStr = OptionVerbosity.Arg1();
		if (verbStr == "*")
			verbLevel = 1;
		else
			verbLevel = tMath::tClamp(verbStr.AsInt(), 0, 2);
	}

	// Uncomment to debug force verbosity level.
	// verbLevel = 2;
	switch (verbLevel)
	{
		case 0: tSystem::tSetChannels(tSystem::tChannel_Default);																break;
		case 1: tSystem::tSetChannels(tSystem::tChannel_Default | tSystem::tChannel_Verbosity0);								break;
		case 2: tSystem::tSetChannels(tSystem::tChannel_Default | tSystem::tChannel_Verbosity0 | tSystem::tChannel_Verbosity1);	break;
	}

	tPrintf("\n");
	tPrintfNorm("Tacent View %d.%d.%d in CLI Mode. Use --help for details.\n", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);

	if (OptionHelp)
	{
		const int maxCols = 80;
		// The +1 on the FindChar calls deals with not found as well as incrementing 1 passed the newline.
		tString intypes("Supported input file types: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
		{
			int lineStart = intypes.FindChar('\n', true) + 1;
			if ( (tStd::tStrlen(intypes.Chr()+lineStart)%maxCols) >= (maxCols-tSystem::tGetFileTypeName(typ->FileType).Length()) )
				tsaPrintf(intypes, "\n");
			tsaPrintf(intypes, "%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
		}

		tString inexts("These cover file extensions: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
		{
			tList<tStringItem> extensions;
			tSystem::tGetExtensions(extensions, typ->FileType);
			for (tStringItem* ext = extensions.First(); ext; ext = ext->Next())
			{
				int lineStart = inexts.FindChar('\n', true) + 1;
				if ( (tStd::tStrlen(inexts.Chr()+lineStart)%maxCols) >= (maxCols-ext->Length()) )
					tsaPrintf(inexts, "\n");
				tsaPrintf(inexts, "%s ", ext->Chr());
			}
		}

		tString outtypes("Supported output file types: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Save.First(); typ; typ = typ->Next())
		{
			int lineStart = outtypes.FindChar('\n', true) + 1;
			if ( (tStd::tStrlen(outtypes.Chr()+lineStart)%maxCols) >= (maxCols-tSystem::tGetFileTypeName(typ->FileType).Length()) )
				tsaPrintf(outtypes, "\n");
			tsaPrintf(outtypes, "%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
		}

		tString outtypesanim("  Supported animated types: ");
		for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_SaveMultiFrame.First(); typ; typ = typ->Next())
		{
			int lineStart = outtypesanim.FindChar('\n', true) + 1;
			if ( (tStd::tStrlen(outtypesanim.Chr()+lineStart)%maxCols) >= (maxCols-tSystem::tGetFileTypeName(typ->FileType).Length()) )
				tsaPrintf(outtypesanim, "\n");
			tsaPrintf(outtypesanim, "%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
		}

		tString filters("Supported filters: ");
		for (int f = 0; f < int(tImage::tResampleFilter::NumFilters)+1; f++)
		{
			int lineStart = filters.FindChar('\n', true) + 1;
			if ( (tStd::tStrlen(filters.Chr()+lineStart)%maxCols) >= (maxCols-tStd::tStrlen(tImage::tResampleFilterNamesSimple[f])) )
				tsaPrintf(filters, "\n");
			tsaPrintf(filters, "%s ", tImage::tResampleFilterNamesSimple[f]);
		}

		tString edgemodes("Supported edge modes: ");
		for (int e = 0; e < int(tImage::tResampleEdgeMode::NumEdgeModes); e++)
		{
			int lineStart = edgemodes.FindChar('\n', true) + 1;
			if ( (tStd::tStrlen(edgemodes.Chr()+lineStart)%maxCols) >= (maxCols-tStd::tStrlen(tImage::tResampleEdgeModeNamesSimple[e])) )
				tsaPrintf(edgemodes, "\n");
			tsaPrintf(edgemodes, "%s ", tImage::tResampleEdgeModeNamesSimple[e]);
		}

		tCmdLine::tPrintUsage(u8"Tristan Grimmer", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"USAGE010(MODE
----
You MUST call with -c or --cli to use this program in CLI mode.

Use the --help (-h) flag to print this help. To view generic command-line
syntax help use the --syntax (-s) flag. For example, to print syntax usage you
could call tacentview.exe -cs which expands to tacentview.exe -c -s

Set output verbosity with --verbosity (-v) and a single integer value after it
from 0 to 2. 0 means no text output, 1* is normal, and 2 is full/detailed.

INPUT IMAGES
------------
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
)USAGE010", intypes.Chr(), inexts.Chr()
		);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"OPERATIONS010(
OPERATIONS
----------
Operations are specified using --op opname[arg1,arg2,...]
There must be no spaces between arguments. The operations get applied in the
order they were specified on the command line. The full sequence of operations
is applied to each and every input image. Optional arguments are denoted with
an asterisk and do not need to be supplied. When either optional arguments are
not provided or * is entered, the default value is used -- look for the
asterisk in the argument description. eg. --op zap[a,b,c*,d*] may be called
with --op zap[a,b] which would do the same thing as --op zap[a,b,*,*]. If the
operation has all optional arguments you may include an empty arg list with []
or leave it out. Eg. zap[*a,b*] may be called with --op zap[] or just --op zap.

--op pixel[x,y,col,chan*]
  Sets the pixel at (x,y) to the colour supplied. The chan argument lets you
  optionally select which pixel colour channels should be modified. You may
  choose any combination of RGBA colour channels.
  x:    The pixel x coordinate. 0* is the first/leftmost pixel. Specifying -1
        will be the pixel at the far right regardless of image width. A -2
        represents the second to last pixel on the right. Using negatives this
        way is handy since not all processed images are required to have the
        same width. 
  y:    The pixel y coordinate. 0* is the first pixel on the bottom. Specifying
        -1 will be the pixel at the image top regardless of image height. A -2
        represents the second to last pixel from the top. Using negatives this
        way is handy since not all processed images are required to have the
        same height.
  col:  Pixel colour. Specify the colour using a hexadecimal in the form
        #RRGGBBAA, a single integer spread to RGBA, or a predefined name:
        black*, white, grey, red, green, blue, yellow, cyan, magenta, or trans
        (transparent black).
  chan: Colour channels to set. The channels are specified with any combination
        of the letters RGBA or rgba. Default is RGBA*. At least one
        valid channel should be specified otherwise the default is used. Eg. RG
        sets the red and green channels. abG sets alpha, blue, and green.

--op resize[w,h,filt*,edge*]
  Resizes image by resampling. Allows non-uniform scale.
  w:    Width. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the height and original aspect.
  h:    Height. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the width and original aspect.
  filt: Resample filter. Default is bilinear*. Only used if dimensions changed
        for the image being processed. See below for valid filter names.
  edge: Edge mode. Default is clamp*. Only used if dimensions changed for the
        image being processed. See note below for valid edge mode names.

--op canvas[w,h,anc*,fill*,ancx*,ancy*]
  Resizes image by modifying the canvas area of the image. You specify the new
  width and height. Vertical or horizontal letterboxes may be needed. This
  operation does not perform resampling.
  w:    Width. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the height and original aspect.
  h:    Height. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the width and original aspect.
  anc:  Anchor. One of tl, tm, tr, ml, mm*, mr, bl, bm. br. These are
        abbreviations for top-left, top-middle, top-right, etc.
  fill: Fill colour. If letterboxes needed this is their colour. Specify the
        colour using a hexadecimal in the form #RRGGBBAA, a single integer
        spread to RGBA, or a predefined name: black*, white, grey, red, green,
        blue, yellow, cyan, magenta, or trans (transparent black).
  ancx: Explicit anchor X position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.
  ancy: Explicit anchor Y position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.

--op aspect[asp,mode,anc*,fill*,ancx*,ancy*]
  Resizes image by modifying the canvas area of the image. You specify the new
  aspect ratio in the form NUM:DEN. Vertical or horizontal letterboxes may be
  needed. This operation does not perform resampling.
  asp:  Aspect ratio specified in form MM:NN where MM and NN are natural
        numbers. Default* for whole argument yields 16:9. Defaults for
        components are 16*:9*. These are also used if numbers <= 0 are input.
  mode: Mode when resizing. Accepts crop* or letter. In crop mode some of the
        image pixels may be cropped to get the correct aspect. In letterbox
        mode all the image pixels are guaranteed to be kept, but it may be
        necessary to add either horizontal or vertical letterboxes (not both).
  anc:  Anchor. One of tl, tm, tr, ml, mm*, mr, bl, bm. br. These are
        abbreviations for top-left, top-middle, top-right, etc.
  fill: Fill colour. If letterboxes needed this is their colour. Specify the
        colour using a hexadecimal in the form #RRGGBBAA, a single integer
        spread to RGBA, or a predefined name: black*, white, grey, red, green,
        blue, yellow, cyan, magenta, or trans (transparent black).
  ancx: Explicit anchor X position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.
  ancy: Explicit anchor Y position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.

--op deborder[col*,chan*]
  Removes same-colour borders from images. Looks around the perimeter of the
  image to see if all rows or columns have the same test-colour and decides
  whether to remove or not. You may check any combination of RGBA colour
  channels. You may retrieve the test-colour from the image itself.
  col:  Test colour. Specify the colour using a hexadecimal in the form
        #RRGGBBAA, a single integer spread to RGBA, or a predefined name:
        black, white, grey, red, green, blue, yellow, cyan, magenta, or trans
        (transparent black). The default* is to get the colour from the origin
        of the image being processed. This is the bottom-left pixel.
  chan: Colour channels to test. You may test the border by looking only for
        matches in particular colour channels. These are specified with any
        combination of the letters RGBA or rgba. Default is RGBA*. At least one
        valid channel should be specified otherwise the default is used. Eg. RG
        tests the red and green channels. abG tests alpha, blue, and green.

--op crop[mode,x,y,mw,mh,fill*]
  Crops an image. You get to specify the lower-left origin and either a new
  width/height or the top-right corner. The values are pixels starting at 0.
  If the crop area you specify goes outside the image being processed, the fill
  colour is used. The resultant image must be at least 4x4.
  mode: Coordinate mode. Either abs* or rel. In absolute mode mw and mh are the
        position of the top right extent of the crop area. Pixels outside of
        this are cropped. In relative mode mw and mh are the new width and
        height of the cropped image.
  x:    The x of the lower-left origin of the crop area. Included in final
        pixels. Defaults to 0*.
  y:    The y of the lower-left origin of the crop area. Included in final
        pixels. Defaults to 0*.
  mw:   The max x of the upper-right extent of the crop area OR the new image
        width if in rel mode. Included in final pixels. Defaults to 3* in
        absolute mode or 4* in relative mode. Both defaults result in a 4x4.
  mh:   The max y of the upper-right extent of the crop area OR the new image
        height if in rel mode. Included in final pixels. Defaults to 3* in
        absolute mode or 4* in relative mode. Both defaults result in a 4x4.
  fill: Fill colour. Specify the colour using a hexadecimal in the form
        #RRGGBBAA, a single integer spread to RGBA, or a predefined name:
        black, white, grey, red, green, blue, yellow, cyan, magenta, or
        trans* (transparent black).

--op flip[mode*]
  Flips an image either horizontally or vertically.
  mode: Either horizontal or vertical. Synonyms include h, v, H, V, Horizontal
        and Vertical. If mode not specified or specified as *, default is
        horizontal* which is about the vertical axis (left becomes right and
        vica-versa.

--op rotate[ang,mode*,upft*,dnft*,fill*]
  Rotates an image. Use negative angles for clockwise rotations. At a minimum
  you must supply the rotation angle in degrees or radians. There is also the
  ability to specify different sampling filters to get quality results, or
  preserve original colours for pixel art. After rotation is complete the areas
  not containing image pixels may be filled with a specified colour, or the
  image may be cropped and possibly resized back to original dimensions.
  ang:  Rotation angle. Specified in degrees. Defaults to 0.0* degrees. To
        specify in radians include the letter 'r' after the value. eg. -1.2r.
        Negatives rotate clockwise. For exact 90 and 180 degree rotations the
        following strings should be used so it uses a faster exact algorithm:
        For 90 Degree Anticlockwise :  90  90.0 acw ccw
        For 90 Degree Clockwise     : -90 -90.0 cw
        For 180 Degree Rotation     : 180 180.0
  mode: Rotation mode. One of fill, crop*, or resize. Fill mode will result in
        larger images and the fill-colour is used because the image bounds get
        rotated outside of the original area. Preserves all image content. Crop
        mode does the rotation and then crops anywhere that went outside. This
        may crop a little of the image depending on angle, but no fill is used.
        Resize mode does the same as crop but then proceeds to resample the
        image back to the original dimensions. Resampling does lose a little
        quality so this is not the default. Use resize mode when it is
        desireable to preserve the image size.
  upft: Upsample filter. See below for filter names. Default is bilinear*.
        Upsampling is used to create a larger image before rotation to give
        much better results. It is also valid to enter 'none' in which case no
        scaling-up of the image is performed. In this case all original pixel
        colours are preserved by using nearest neighbour colours. This is fast
        and a good choice for pixel-art and sprites.
  dnft: Downsample filter. Only used if up-filter is not none. This filter is
        used to restore image size after rotation. Specifying none* here uses
        a special upsample method that produces sharper results. Using box
        filter here is also a good choice. See below for valid filter names.
  fill: Fill colour. Only used if mode was fill. Specify the colour using a
        hexadecimal in the form #RRGGBBAA, a single integer spread to RGBA, or
        a predefined name: black*, white, grey, red, green, blue, yellow, cyan,
        magenta, or trans (transparent black).
)OPERATIONS010"
		);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"OPERATIONS020(
--op levels[bp,mp,wp,obp,owp,fram*,chan*,alg*]
  Adjusts image levels in a manner similar to photo-editing software. You
  specify black, mid, and white points, whether to operate on all frames or a
  single frame, which channel to use, and the gamma algorithm to use. At a
  minimum you must specify the 5 black, mid, and white points.
  bp:   Black point. Value must be between 0.0* and 1.0.
  mp:   Mid point. Value must be between blackpoint and whitepoint. If set to *
        or -1.0 the midpoint is computed automatically as the halfway point
        between the blackpoint and whitepoint.
  wp:   White point. Value must be between midpoint and 1.0*.
  obp:  Output black point. Value must be between 0.0* and 1.0.
  owp:  Output white point. Value must be between 0.0 and 1.0* and bigger than
        the output black point.
  fram: Frame number. This is the 0-based frame number to apply the levels
        adjustments to. If set to -1* all frames are adjusted. If frame number
        is too big campared to the number of frames in the current image, it
        will get clamped to the number available (last frame).
  chan: The channels the level adjustments should be made to. Choices are RGB*,
        R, G, B, and A. Lower-case also works.
  alg:  Power midpoint gamma algorithm. Boolean defaulting to true*. Lets you
        decide between 2 algorithms for the curve on the mid-tone gamma. If
        true uses a continuous base-10 power curve that smoothly transitions
        the full range. For this algo the gamma range is [0.1, 10.0] where 1.0
        is linear. This approximates GIMP. If false it tries to mimic Photoshop
        where there is a C1 discontinuity at gamma = 1. In this mode the gamma
        range is [0.01, 9.99] where 1.0 is linear. See below for valid
        arguments to supply boolean true or false.

--op contrast[cont,fram*,chan*]
  Adjusts contrast. Specify the contrast, whether to operate on all frames or a
  single frame, and which channel(s) to use.
  cont: Contrast value from 0.0 (none) to 1.0 (full contrast). A value of 0.5*
        leaves the image unmodified.
  fram: Frame number. This is the 0-based frame number to apply the contrast
        adjustment to. If set to -1* all frames are adjusted. If frame number
        is too big campared to the number of frames in the current image, it
        will get clamped to the number available (last frame).
  chan: The channels the contrast adjustment should be made to. Choices are
        RGB*, R, G, B, and A. Lower-case also works.

--op brightness[brit,fram*,chan*]
  Adjusts brightness. Specify the brightness, whether to operate on all frames
  or a single frame, and which channel(s) to use.
  brit: Brightness value from 0.0 (black) to 1.0 (white). A value of 0.5*
        leaves the image unmodified.
  fram: Frame number. This is the 0-based frame number to apply the brightness
        adjustment to. If set to -1* all frames are adjusted. If frame number
        is too big campared to the number of frames in the current image, it
        will get clamped to the number available (last frame).
  chan: The channels the brightness adjustment should be made to. Choices are
        RGB*, R, G, B, and A. Lower-case also works.

--op quantize[algo,ncol,xact*,fsam*,dith*]
  Quantize the image to a reduced set of colours using various methods. Specify
  the algorithm, number of colours, and any optional parameters. Quantization
  is based on the colour (RGB) components. The alpha channel, if present, is
  preserved/unchanged.
  algo: The method/algorithm for quantization: fix, wu*, neu, spc.
        fix: Fixed/predefined palette used. Pure black and white are
             guaranteed to be present.
        wu*: The Xiaolin Wu algorithm. Fast and good quality.
        neu: NeuQuant algorithm. Learning neural-net. The original high-quality
             quantization method.
        spc: Scolorq algorithm. Spatial quantiation. Slow but good quality for
             low numbers of colours (< 32). Only algo to offer dithering.
  ncol: Number of colours that will be present in image afterwards. Must be
        between 2 and 256* (both inclusive).
  xact: Boolean exact. Default is true*. See below for valid boolean arguments.
        If exact is true and the number of existing colours is less or equal to
        the requested, then the image will look identical after processing. If
        exact is false, all pixels get run through the chosen algorithm.
  fsam: FilterSize or SampleFactor. Only applies to spatial and neu algorithms.
        For scolorq this is the filter-size and must be 1, 3*, or 5.
        For neu quant this is the sample-factor from 1* to 30. Smaller values
        are better quality -- more faithful representation of the original.
  dith: Dither amount. Only applies to scolorq. Values from 0.0 to 30.0.
        The default is 0.0* which means auto-determine the dither amount based
        on the image dimensions and number of colours. Otherwise, a value of
        0.1 results in essentially no dither, and at around 20.0 there is
        significant dithering.

--op channel[mode*,chan*,col*]
  Channel operations affect components of all pixels. The supported modes
  support setting channels, blending using alpha, spreading a channel, and
  writing intensity to channels.
  mode: The channel operation mode. One of:
        set:    Sets specified channels to corresponding component in the
                supplied colour value.
        blend*: Blends background colour (bg) into the specified RGB channels
                by using the pixel-alpha to modulate. The new pixel colour is
                alpha*src_comp + (1-alpha)*bg_comp. This applies to any
                combination of input RGB channels. The final alpha is left
                unmodified if A not specified in channels. If A is specified
                the alpha is set to the A component of the supplied bg colour.
        spread: Spreads a specific single channel into the RGB chanbels.
        intens: Computes pixel intensity and sets any combination of RGBA
                channels to that intensity.
  chan: Colour channels. For set and intensity modes the default is RGB*. For
        blend mode default is RGBA*. For spread mode the single-channel default
        is R. Channels are specified with any combination of the letters RGBA
        or rgba.
  col:  Colour. Only used if mode is blend or set. Specify the colour using a
        hexadecimal in the form #RRGGBBAA, a single integer spread to RGBA, or
        a predefined name: black*, white, grey, red, green, blue, yellow, cyan,
        magenta, or trans (transparent black).
  As an example, the command --op channel will create an image with
  pre-multiplied alphas. The blended in background will be black and the alpha
  will be 255 for all pixels.

--op swizzle[rgba*]
  The swizzle operation allows you to manipulate the RGBA channels of an image
  and swap, duplicate, clear or set them. You can basically take the existing
  channels and rearrange them as you see fit.
  rgba: This is the destination mapping used by the swizzle. It is always in
        the order RGBA. It is made of the characters R, G, B, A, 0, 1, and *.
        The characters are case-insensitive so r, g, b, and a may also be used.
        The asterisk means automatic channel selection. 0 means the channel is
        set to 0 for all pixels. 1 means the channel is set to full for all
        pixels. R means the destiniation channel is taken from the original red
        channel. Similarly for G, B, and A. You do not need to specify all four
        characters if you want the remaining ones to be defaulted to their
        corresponding source channel. This is what the asterisk does.
        The default is **** which is the same as RGBA, both of which leave the
        image unmodified.
  Example 1: --op swizzle[BGR] will swap the red and the blue channels. In
  order, the new red channel gets the original blue channel, the green gets
  green, and the new blue channel gets red. This is the same as swizzle[B*R]
  and is also the same as swizzle[B*R*]. The asterisks just grab the
  corresponding original channel.
  Example 2: --op swizzle[***1] keeps the colours the same but sets the alpha
  to full for all pixels. This is the same as swizzle[RGB1].
  Example 3: --op swizzle[0] clears the red channel. Same as [0GBA] and [0***]
  Example 4: --op swizzle[GGG1] places the original green channel in the new
  red, green and blue channels. It also sets the alpha to full (opaque).

--op extract[frms*,sdir*,base*]
  Extracts frames from a multiframe or animated image. Specify the frame
  numbers to extract, the base filename, and the directory to put them in. The
  output type is specified using -o or --outtype and the output parameters are
  specified using the --paramsTYPE options. See below.
  frms: The frame numbers to extract in range format. In this format you may
        specify multiple ranges separated by a | or U character. A ! means
        exclusive and a hyphen specifies a range. The default is to extract
        all* frames. For example, --op extract[0-2|!4-6|!7-10!] will extract
        frames 0,1,2,5,6,8,9. This is the same as --op extract[0-2|5-6|8-9]
        If the image being processed does not have enough frames for the
        specified range, those frame files will not be created.
  sdir: The sub-directory, relative to the directory the image is in, to place
        the extracted frames. If the sub-directory does not exist, it is
        created for you. Defaults to a directory called Saved*.
  base: The base filename used when saving extracted frames. Defaults* to the
        base filename of the input image. The final filename will be of the
        form Basename_NNN.ext where NNN is the frame number and ext is the
        extension.
  Example 1: --op extract[0-4!] will extract 4 frames (0, 1, 2, and 3) and
  save them to a folder called Saved with names Base_001.tga, Base_002, and
  Base_002.tga.

%s

%s

Boolean arguments: You may use "true", "t", "yes", "y", "on", "enable",
"enabled", "1", "+", and strings that represent non-zero integers as true.
These are case-insensitive. False is the result otherwise.
)OPERATIONS020", filters.Chr(), edgemodes.Chr()
		);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"POSTOPS010(
POST OPERATIONS
---------------
Post operations are specified using --po opname[arg1,arg2,...]
These are operations that take more than a single image as input. They are
separated out into a different pass for efficiency -- if we were to do these as
regular inline operations (--op) we would need to have all input images in
memory at the same time. The post-op pass still uses the _same_ set of input
images and it runs after all normal operations have completed. If a regular
operation modifies any of the input files, the modified file(s) are used as
input to the post operation. If a normal operation generates a new file not
included in the inputs, the new file is not used by the post operation.

--po combine[durs*,sdir*,base*]
  Combines multiple input images into a single animated image. The output file
  type must support animation or be able to store multiple sub-images / pages.
%s
  Input images must be all the same dimensions. Use normal operations to resize
  the source images beforehand if necessary. This can be done in a single
  command if you don't mind overwriting your existing source files with the
  --overwrite flag (see below), or do it as two passes.
  durs: Durations for each frame specified in milliseconds. The syntax is a
        sequence of frame-interval:duration pairs separated by |. Frame numbers
        start at 0. The intervals are applied in the order they are entered so
        an overlap will overwrite previous durations. For an interval bang (!)
        means exclusive and a hyphen specifies a range. The default duration
        for all frames is 33*. If a frame interval is not specified, the
        supplied duration applies to all frames. For example, if you are
        combining 100 images these are equivalent: combine[], combine[*],
        combine[33], and combine[0-99:33]. To create a 2 second pause on (zero-
        based) frame 10 only: combine[10:2000]. To set the first 3 frames to
        100ms, frame 6 to 16ms, and frame 10, 11, 12 to 1.1s, the following
        could be used: [0-2:100|6:16|10-12:1100]
  sdir: The sub-directory, relative to the current directory, to place the
        combined image in. If the sub-directory does not exist, it is created
        for you. Defaults to a directory called Combined*.
  base: The base filename (not including the extension) used when saving the
        combined image. Defaults* to Combined_YYYY_MM_DD_HH_MM_SS_NNN where
        NNN is the number of frames. The final filename will include the
        correct extension based on the output image type.
  Example 1: -o webp --po combine[0-24:100|50-74:200,OutDir,Animated] creates a
  file called Animated.webp with the first 25 frames each lasting 1/10 second,
  the next 25 at 30fps, the 25 after at 1/5 second, and the remainder at 33ms.

--po contact[cols*,rows*,frmw*,frmh*,sdir*,base*]
  Creates a contact sheet (AKA flipbook) from multiple input images. Input
  images are scaled (only if needed) to fit the requested frame dimensions. If
  you have specific cropping or scaling requirements use normal operations to
  resize/crop the source images beforehand. This can be done in a single
  command if you don't mind overwriting your existing source files with the
  --overwrite flag (see below), or do it as two passes. The final output
  image always has a single frame. The final output image width will always be
  the frame width times the number of columns. The height will be the frame
  height times the number of rows. In some cases there will need to be empty
  frames in the contact sheet -- If any input image has a transparency, the
  final image will have transparency (assuming the output format supports it).
  Blank frames are filled with either opaque black (if no transparency) or
  transparent black (if there is transparency).
  cols: Specify the number of columns you want in the contact sheet. If not
        default, must be an integer bigger than 0. If set to default* it will
        be computed for you based on the number of rows entered so that all
        input frames will be included. If rows is also default, both cols and
        rows will be computed for you so that there are enough pages for all
        input images. If both are set and their product is less than the number
        of input images, not all imput images will be in the contact sheet.
  rows: The number of rows. Similar to description of cols above.
  frmw: The individual frame width. Specified as an integer greater than 0. The
        default* is to read the width from the first frame image processed.
  frmh: The individual frame height. Specified as an integer greater than 0. The
        default* is to read the height from the first frame image processed.
  filt: Resample filter for frames. Default is bilinear*. Only used if
        input image frame dimensions don't match requested. See above for valid
        filter names.
  edge: Edge mode when filtering. Default is clamp*. Only used if necessary.
        See above for valid edge mode names.
  sdir: The sub-directory, relative to the current directory, to place the
        contact-sheet image in. If the sub-directory does not exist, it is
        created for you. Defaults to a directory called Contact*.
  base: The base filename (not including the extension) used when saving the
        contact image. Defaults* to Contact_YYYY_MM_DD_HH_MM_SS_NNN where
        NNN is the number of frames. The final filename will include the
        correct extension based on the output image type.
  Example 1: -o tga --po contact[10,5,512,256,bicubic_catmullrom,clamp,*,Sheet]
  will create a contact sheet image called Sheet.tga in a directory called
  Contacts. The tga file will be 5120x1280 and have 50 512x256 images in it.
)POSTOPS010", outtypesanim.Chr()
		);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"OUTIMAGES010(
OUTPUT IMAGES
-------------
The output files are generated based on the input files and chosen operations.
The type of the output images is specified with --outtype type. The short
version -o may also be used. If no outtype is specified the default is tga.
%s

The output filename matches the input filename except that the extension/type
may be different. Eg. Seascape.jpg would save as Seascape.tga if the outtype
was tga.

If an output file already exists, it is not overwritten. To allow overwrite use
the --overwrite (-w) flag. To have the tool try a different filename if it
aready exists, use the --autoname (-a) flag. It will append the string _NNN to
the name where NNN is a number that keeps incrementing until no existing file
is found.

By default if no modifications are made to an input file, the file is still
saved. This is so easy batch conversions from one type to another may be
performed. Sometimes you may not want to save unmodified files. An example of
this is using the extract operation by itself. If you don't want the unmodified
input image saved, specify -k or --skipunchanged on the command line.
)OUTIMAGES010", outtypes.Chr()
		);

		// In editor the column num at EOL (after last character) should be 80 or less.
		tPrintf
		(
R"OUTPARAMS010(
OUTPUT PARAMETERS
-----------------
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

EXIT CODE
---------
The return error code is 0 for success and 1 for failure. For 0 to be returned
every specified image must be successfully loaded, processed, and saved. A
failure in any step for any image results in an error. By default processing
continues to the next image even on a failure. If the --earlyexit (-e) flag is
set, processing stops immediately on any failure. Either way, any failure
returns a non-zero exit code.
)OUTPARAMS010"
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
	PopulateImagesList();

	// Populates the Operations list.
	PopulateOperations();

	// Populates the PostOperations list.
	PopulatePostOperations();

	DetermineOutType();
	DetermineOutSaveParameters(OutType);

	// Process standard operations.
	// We do the images one at a time to save memory. That is, we only need to load one image in at a time
	// and can unload them when done.
	bool somethingFailed = false;
	for (Viewer::Image* image = Images.First(); image; image = image->Next())
	{
		image->Load();

		tString inNameShort = tSystem::tGetFileName(image->Filename);
		if (!image->IsLoaded())
		{
			tPrintfNorm("Failed Load: %s. Skipping.\n", inNameShort.Chr());
			somethingFailed = true;
			if (OptionEarlyExit)
				break;
			continue;
		}

		// Process the standard operations on the current image.
		tPrintfNorm("Processing: %s\n", inNameShort.Chr());
		bool processed = ProcessOperationsOnImage(*image);
		if (!processed)
		{
			image->Unload();
			somethingFailed = true;
			if (OptionEarlyExit)
			{
				image->Unload();
				break;
			}
			continue;
		}

		// Some operations do not modify the input image at all. For example, the extract operation saves every frame
		// of the input image but does not modify it. In these cases the image dirty flag is not set so we can
		// skip saving if OptionSkipUnchanged is true.
		if (OptionSkipUnchanged && !image->IsDirty())
		{
			tPrintfNorm("Skipping Save: %s\n", inNameShort.Chr());
			continue;
		}

		// Determine out filename.
		tString outFilename = DetermineOutputFilename(image->Filename, OutType);
		tString outNameShort = tSystem::tGetFileName(outFilename);
		bool doSave = true;
		if (!OptionOverwrite && tSystem::tFileExists(outFilename))
		{
			tPrintfNorm("File %s exists. Not overwriting.\n", outNameShort.Chr());
			somethingFailed = true;
			if (OptionEarlyExit)
			{
				image->Unload();
				break;
			}

			doSave = false;
		}

		// Save.
		if (doSave)
		{
			// Set the image save parameters correctly. The user may have modified them from the command line.
			SetImageSaveParameters(*image, OutType);
			bool success = image->Save(outFilename, OutType, false);
			if (success)
			{
				tPrintfFull("Saved File: %s\n", outNameShort.Chr());
			}
			else
			{
				tPrintfNorm("Failed Save: %s\n", outNameShort.Chr());
				somethingFailed = true;
				if (OptionEarlyExit)
				{
					image->Unload();
					break;
				}
			}
		}

		image->Unload();
	}

	// Do post save operations here --po. These are operations that take more than a single image as input.
	// They are separated out into a different pass for efficiency -- if we were to do these as regular inline
	// operations (--op) we would need to have all input images in memory at the same time. The post-op pass
	// still uses the _same_ set of input images. It runs after all normal operations have saved to disk.
	//
	// Example post-op coperations include: combining multiple images into a single animated image or creating
	// a contact sheet from multiple images.
	return somethingFailed ? 1 : 0;
}
