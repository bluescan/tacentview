// Command.cpp
//
// Command line use of TacentView to perform batch image processing and conversions. The command-line interface supports
// operations such as quantization, rescaling/filtering, cropping, rotation, extracting frames, creating contact-sheets,
// amalgamating images into animated formats, and levels adjustments.
//
// Copyright (c) 2023, 2024 Tristan Grimmer.
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
#include "CommandHelp.h"
#include "CommandOps.h"
#include "TacentView.h"


namespace Command
{
	tCmdLine::tParam  ParamInputFiles		("Input image files",				"inputfiles",			0	);

	// Note, -c and --cli are reserved.
	tCmdLine::tOption OptionHelp			("Print help/usage information",	"help",			'h'			);
	tCmdLine::tOption OptionSyntax			("Print syntax help",				"syntax",		's'			);
	tCmdLine::tOption OptionExamples		("Print examples",					"examples",		'x'			);
	tCmdLine::tOption OptionMarkdown		("Print examples in markdown",		"markdown",		'm'			);
	tCmdLine::tOption OptionVerbosity		("Verbosity from 0 to 2",			"verbosity",	'v',	1	);

	tCmdLine::tOption OptionInTypes			("Input file type(s)",				"in",			'i',	1	);
	tCmdLine::tOption OptionInASTC			("Load parameters for ASTC files",	"inASTC",				1	);
	tCmdLine::tOption OptionInDDS			("Load parameters for DDS files",	"inDDS",				1	);
	tCmdLine::tOption OptionInEXR			("Load parameters for EXR files",	"inEXR",				1	);
	tCmdLine::tOption OptionInHDR			("Load parameters for HDR files",	"inHDR",				1	);
	tCmdLine::tOption OptionInJPG			("Load parameters for JPG files",	"inJPG",				1	);
	tCmdLine::tOption OptionInKTX			("Load parameters for KTX files",	"inKTX",				1	);
	tCmdLine::tOption OptionInPKM			("Load parameters for PKM files",	"inPKM",				1	);
	tCmdLine::tOption OptionInPNG			("Load parameters for PNG files",	"inPNG",				1	);

	tCmdLine::tOption OptionOperation		("Operation",						"op",					1	);
	tCmdLine::tOption OptionPostOperation	("Post operation",					"po",					1	);

	tCmdLine::tOption OptionOutTypes		("Output file type(s)",				"out",			'o',	1	);
	tCmdLine::tOption OptionOutName			("Output file name modifications",	"outname",		'n',	1	);
	tCmdLine::tOption OptionOutAPNG			("Save parameters for APNG files",	"outAPNG",				1	);
	tCmdLine::tOption OptionOutBMP			("Save parameters for BMP  files",	"outBMP",				1	);
	tCmdLine::tOption OptionOutGIF			("Save parameters for GIF  files",	"outGIF",				1	);
	tCmdLine::tOption OptionOutJPG			("Save parameters for JPG  files",	"outJPG",				1	);
	tCmdLine::tOption OptionOutPNG			("Save parameters for PNG  files",	"outPNG",				1	);
	tCmdLine::tOption OptionOutQOI			("Save parameters for QOI  files",	"outQOI",				1	);
	tCmdLine::tOption OptionOutTGA			("Save parameters for TGA  files",	"outTGA",				1	);
	tCmdLine::tOption OptionOutTIFF			("Save parameters for TIFF files",	"outTIFF",				1	);
	tCmdLine::tOption OptionOutWEBP			("Save parameters for WEBP files",	"outWEBP",				1	);
	tCmdLine::tOption OptionOverwrite		("Overwrite existing output files",	"overwrite",	'w'			);
	tCmdLine::tOption OptionAutoName		("Autogenerate output file names",	"autoname",		'a'			);
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

	struct ParamValuePair : public tLink<ParamValuePair>
	{
		tString Param;
		tString Value;
	};
	int ParseParamValuePairs(tList<ParamValuePair>& pairs, const tString& pairsStr);			// Parsed pairsStr of form "param1=value1,param2=value2,etc".

	void DetermineInputTypes();																	// Step 1.
	void InputFilesAddUnique(const tSystem::tFileInfo&);
	void DetermineInputLoadParameters();
	void ParseLoadParametersASTC();
	void ParseLoadParametersDDS();
	void ParseLoadParametersEXR();
	void ParseLoadParametersHDR();
	void ParseLoadParametersJPG();
	void ParseLoadParametersKTX();
	void ParseLoadParametersPKM();
	void ParseLoadParametersPNG();

	void DetermineInputFiles();																	// Step 2.
	void GetItemsFromManifest(tList<tStringItem>& manifestItems, const tString& manifestFile);
	void ParseInputItem(tList<tSystem::tFileInfo>& inputFiles, const tString& item);

	void PopulateOperations();
	void PopulatePostOperations();
	void PopulateImagesList();																	// Step 3.
	bool ProcessOperationsOnImage(Viewer::Image&);												// Applies all the operations (in order) to the supplied image.

	void DetermineOutputTypes();																// Step 4.
	void DetermineOutputNameParameters();														// Step 5.
	void DetermineOutputSaveParameters();														// Step 6.
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

	tImage::tImageAPNG::SaveParams	SaveParamsAPNG;
	tImage::tImageBMP::SaveParams	SaveParamsBMP;
	tImage::tImageGIF::SaveParams	SaveParamsGIF;
	tImage::tImageJPG::SaveParams	SaveParamsJPG;
	tImage::tImagePNG::SaveParams	SaveParamsPNG;
	tImage::tImageQOI::SaveParams	SaveParamsQOI;
	tImage::tImageTGA::SaveParams	SaveParamsTGA;
	tImage::tImageTIFF::SaveParams	SaveParamsTIFF;
	tImage::tImageWEBP::SaveParams	SaveParamsWEBP;

	tImage::tImageASTC::LoadParams	LoadParamsASTC;
	tImage::tImageDDS::LoadParams	LoadParamsDDS;
	tImage::tImagePVR::LoadParams	LoadParamsPVR;
	tImage::tImageEXR::LoadParams	LoadParamsEXR;
	tImage::tImageHDR::LoadParams	LoadParamsHDR;
	tImage::tImageJPG::LoadParams	LoadParamsJPG;
	tImage::tImageKTX::LoadParams	LoadParamsKTX;
	tImage::tImagePKM::LoadParams	LoadParamsPKM;
	tImage::tImagePNG::LoadParams	LoadParamsPNG;
	bool LoadParams_DetectAPNGInsidePNG = false;

	tSystem::tFileTypes InputTypes;
	tList<tSystem::tFileInfo> InputFiles;
	tList<Viewer::Image> Images;
	tList<Operation> Operations;
	tList<PostOperation> PostOperations;
	tSystem::tFileTypes OutTypes;

	tString OutNamePrefix;
	tString OutNameSuffix;
	tString OutNameSearch;
	tString OutNameReplace;
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
		//	else if (AllocConsole())
		//		attachedToConsole = true;

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


int Command::ParseParamValuePairs(tList<ParamValuePair>& pairs, const tString& pairsStr)
{
	if (pairsStr.IsEmpty())
		return 0;

	tList<tStringItem> paramValueStrList;
	tStd::tExplode(paramValueStrList, pairsStr, ',');
	for (tStringItem* pvstr = paramValueStrList.First(); pvstr; pvstr = pvstr->Next())
	{
		if (pvstr->FindChar('=') == -1)
			continue;

		tString param = pvstr->Left('=');
		tString value = pvstr->Right('=');
		if (param.IsEmpty() || value.IsEmpty())
			continue;

		ParamValuePair* newPair = new ParamValuePair;
		newPair->Param = param;
		newPair->Value = value;
		pairs.Append(newPair);
	}

	return pairs.Count();
}


void Command::DetermineInputTypes()
{
	if (OptionInTypes)
	{
		tList<tStringItem> types;
		OptionInTypes.GetArgs(types);
		for (tStringItem* typ = types.First(); typ; typ = typ->Next())
		{
			// Each argument will either be a single type, or a comma-sperated list of types.
			tList<tStringItem> sepTypes;
			tStd::tExplode(sepTypes, *typ, ',');
			for (tStringItem* t = sepTypes.First(); t; t = t->Next())
			{
				tSystem::tFileType ft = tSystem::tGetFileTypeFromName(*t);
				if (ft == tSystem::tFileType::Invalid)
					tPrintfNorm("Warning: Unknown input file type: %s\n", t->Chr());
				else if (!Viewer::FileTypes_Load.Contains(ft))
					tPrintfNorm("Warning: Unsupported input image type: %s\n", t->Chr());
				else
					InputTypes.Add(ft);
			}
		}

		// If InputTypes is still empty because the caller didn't enter any valid types, we
		// default to tga-only.
		if (InputTypes.IsEmpty())
		{
			tPrintfNorm("Warning: No valid input types specified. Defaulting to tga.\n");
			InputTypes.Add(tSystem::tFileType::TGA);
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


void Command::DetermineInputLoadParameters()
{
	// We only bother reading load parameters for the types of files we will be loading.
	for (tSystem::tFileTypes::tFileTypeItem* typeItem = InputTypes.First(); typeItem; typeItem = typeItem->Next())
	{
		tSystem::tFileType fileType = typeItem->FileType;
		switch (fileType)
		{
			case tSystem::tFileType::ASTC:	ParseLoadParametersASTC();	break;
			case tSystem::tFileType::DDS:	ParseLoadParametersDDS();	break;
			case tSystem::tFileType::EXR:	ParseLoadParametersEXR();	break;
			case tSystem::tFileType::HDR:	ParseLoadParametersHDR();	break;
			case tSystem::tFileType::JPG:	ParseLoadParametersJPG();	break;
			case tSystem::tFileType::KTX:
			case tSystem::tFileType::KTX2:	ParseLoadParametersKTX();	break;
			case tSystem::tFileType::PKM:	ParseLoadParametersPKM();	break;
			case tSystem::tFileType::PNG:	ParseLoadParametersPNG();	break;
		}
	}
}


void Command::ParseLoadParametersASTC()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInASTC.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("colp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("*"):
					case tHash::tHashCT("sRGB"):	LoadParamsASTC.Profile = tColourProfile::sRGB;	break;
					case tHash::tHashCT("gRGB"):	LoadParamsASTC.Profile = tColourProfile::gRGB;	break;
					case tHash::tHashCT("lRGB"):	LoadParamsASTC.Profile = tColourProfile::lRGB;	break;
					case tHash::tHashCT("HDRa"):	LoadParamsASTC.Profile = tColourProfile::HDRa;	break;
					case tHash::tHashCT("HDRA"):	LoadParamsASTC.Profile = tColourProfile::HDRA;	break;
				}
				break;

			case tHash::tHashCT("corr"):
			{
				uint32 gamaF = tImage::tImageASTC::LoadFlag_GammaCompression;
				uint32 autoF = tImage::tImageASTC::LoadFlag_AutoGamma;
				uint32 srgbF = tImage::tImageASTC::LoadFlag_SRGBCompression;
				uint32& flags = LoadParamsASTC.Flags;
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("none"):	flags &= ~(gamaF | srgbF | autoF);			break;
					case tHash::tHashCT("*"):
					case tHash::tHashCT("auto"):	flags &= ~(gamaF | srgbF); flags |= autoF;	break;
					case tHash::tHashCT("gamc"):	flags &= ~(srgbF | autoF); flags |= gamaF;	break;
					case tHash::tHashCT("srgb"):	flags &= ~(gamaF | autoF); flags |= srgbF;	break;
				}
				break;
			}

			case tHash::tHashCT("gamma"):
				LoadParamsASTC.Gamma = (value == "*") ? 2.2f : tMath::tClamp(value.AsFloat(), 0.5f, 4.0f);
				break;

			case tHash::tHashCT("tone"):
				LoadParamsASTC.Exposure = (value == "*") ? -1.0f : tMath::tClamp(value.AsFloat(), -1.0f, 4.0f);
				if (LoadParamsASTC.Exposure < 0.0f)
					LoadParamsASTC.Flags &= ~(tImage::tImageASTC::LoadFlag_ToneMapExposure);
				else
					LoadParamsASTC.Flags |= tImage::tImageASTC::LoadFlag_ToneMapExposure;
				break;
		}
	}
}


void Command::ParseLoadParametersDDS()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInDDS.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("corr"):
			{
				uint32 gamaF = tImage::tImageDDS::LoadFlag_GammaCompression;
				uint32 autoF = tImage::tImageDDS::LoadFlag_AutoGamma;
				uint32 srgbF = tImage::tImageDDS::LoadFlag_SRGBCompression;
				uint32& flags = LoadParamsDDS.Flags;
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("none"):	flags &= ~(gamaF | srgbF | autoF);			break;
					case tHash::tHashCT("*"):
					case tHash::tHashCT("auto"):	flags &= ~(gamaF | srgbF); flags |= autoF;	break;
					case tHash::tHashCT("gamc"):	flags &= ~(srgbF | autoF); flags |= gamaF;	break;
					case tHash::tHashCT("srgb"):	flags &= ~(gamaF | autoF); flags |= srgbF;	break;
				}
				break;
			}

			case tHash::tHashCT("gamma"):
				LoadParamsDDS.Gamma = (value == "*") ? 2.2f : tMath::tClamp(value.AsFloat(), 0.5f, 4.0f);
				break;

			case tHash::tHashCT("tone"):
				LoadParamsDDS.Exposure = (value == "*") ? -1.0f : tMath::tClamp(value.AsFloat(), -1.0f, 4.0f);
				if (LoadParamsDDS.Exposure < 0.0f)
					LoadParamsDDS.Flags &= ~(tImage::tImageDDS::LoadFlag_ToneMapExposure);
				else
					LoadParamsDDS.Flags |= tImage::tImageDDS::LoadFlag_ToneMapExposure;
				break;

			case tHash::tHashCT("spred"):
			{
				bool spread = (value == "*") ? true : value.AsBool();
				if (spread)
					LoadParamsDDS.Flags |= tImage::tImageDDS::LoadFlag_SpreadLuminance;
				else
					LoadParamsDDS.Flags &= ~(tImage::tImageDDS::LoadFlag_SpreadLuminance);
				break;
			}

			case tHash::tHashCT("strct"):
			{
				bool strict = (value == "*") ? false : value.AsBool();
				if (strict)
					LoadParamsDDS.Flags |= tImage::tImageDDS::LoadFlag_StrictLoading;
				else
					LoadParamsDDS.Flags &= ~(tImage::tImageDDS::LoadFlag_StrictLoading);
				break;
			}
		}
	}
}


void Command::ParseLoadParametersEXR()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInEXR.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("gamma"):
				LoadParamsEXR.Gamma = (value == "*") ? 2.2f : tMath::tClamp(value.AsFloat(), 0.6f, 3.0f);
				break;

			case tHash::tHashCT("expo"):
				LoadParamsEXR.Exposure = (value == "*") ? 1.0f : tMath::tClamp(value.AsFloat(), -10.0f, 10.0f);
				break;

			case tHash::tHashCT("defog"):
				LoadParamsEXR.Defog = (value == "*") ? 0.0f : tMath::tClamp(value.AsFloat(), 0.0f, 0.1f);
				break;

			case tHash::tHashCT("knelo"):
				LoadParamsEXR.KneeLow = (value == "*") ? 0.0f : tMath::tClamp(value.AsFloat(), -3.0f, 3.0f);
				break;

			case tHash::tHashCT("knehi"):
				LoadParamsEXR.KneeHigh = (value == "*") ? 3.5f : tMath::tClamp(value.AsFloat(), 3.5f, 7.5f);
				break;
		}
	}
}


void Command::ParseLoadParametersHDR()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInHDR.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("gamma"):
				LoadParamsHDR.Gamma = (value == "*") ? 2.2f : tMath::tClamp(value.AsFloat(), 0.6f, 3.0f);
				break;

			case tHash::tHashCT("expo"):
				LoadParamsHDR.Exposure = (value == "*") ? 0 : tMath::tClamp(value.AsInt(), -10, 10);
				break;
		}
	}
}


void Command::ParseLoadParametersJPG()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInJPG.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("strct"):
			{
				bool strict = (value == "*") ? false : value.AsBool();
				if (strict)
					LoadParamsJPG.Flags |= tImage::tImageJPG::LoadFlag_Strict;
				else
					LoadParamsJPG.Flags &= ~(tImage::tImageJPG::LoadFlag_Strict);
				break;
			}

			case tHash::tHashCT("exifo"):
			{
				bool exifReorient = (value == "*") ? false : value.AsBool();
				if (exifReorient)
					LoadParamsJPG.Flags |= tImage::tImageJPG::LoadFlag_ExifOrient;
				else
					LoadParamsJPG.Flags &= ~(tImage::tImageJPG::LoadFlag_ExifOrient);
				break;
			}
		}
	}
}


void Command::ParseLoadParametersKTX()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInKTX.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("corr"):
			{
				uint32 gamaF = tImage::tImageKTX::LoadFlag_GammaCompression;
				uint32 autoF = tImage::tImageKTX::LoadFlag_AutoGamma;
				uint32 srgbF = tImage::tImageKTX::LoadFlag_SRGBCompression;
				uint32& flags = LoadParamsKTX.Flags;
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("none"):	flags &= ~(gamaF | srgbF | autoF);			break;
					case tHash::tHashCT("*"):
					case tHash::tHashCT("auto"):	flags &= ~(gamaF | srgbF); flags |= autoF;	break;
					case tHash::tHashCT("gamc"):	flags &= ~(srgbF | autoF); flags |= gamaF;	break;
					case tHash::tHashCT("srgb"):	flags &= ~(gamaF | autoF); flags |= srgbF;	break;
				}
				break;
			}

			case tHash::tHashCT("gamma"):
				LoadParamsKTX.Gamma = (value == "*") ? 2.2f : tMath::tClamp(value.AsFloat(), 0.5f, 4.0f);
				break;

			case tHash::tHashCT("tone"):
				LoadParamsKTX.Exposure = (value == "*") ? -1.0f : tMath::tClamp(value.AsFloat(), -1.0f, 4.0f);
				if (LoadParamsKTX.Exposure < 0.0f)
					LoadParamsKTX.Flags &= ~(tImage::tImageKTX::LoadFlag_ToneMapExposure);
				else
					LoadParamsKTX.Flags |= tImage::tImageKTX::LoadFlag_ToneMapExposure;
				break;

			case tHash::tHashCT("spred"):
			{
				bool spread = (value == "*") ? true : value.AsBool();
				if (spread)
					LoadParamsKTX.Flags |= tImage::tImageKTX::LoadFlag_SpreadLuminance;
				else
					LoadParamsKTX.Flags &= ~(tImage::tImageKTX::LoadFlag_SpreadLuminance);
				break;
			}
		}
	}
}


void Command::ParseLoadParametersPKM()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInPKM.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("corr"):
			{
				uint32 gamaF = tImage::tImagePKM::LoadFlag_GammaCompression;
				uint32 autoF = tImage::tImagePKM::LoadFlag_AutoGamma;
				uint32 srgbF = tImage::tImagePKM::LoadFlag_SRGBCompression;
				uint32& flags = LoadParamsPKM.Flags;
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("none"):	flags &= ~(gamaF | srgbF | autoF);			break;
					case tHash::tHashCT("*"):
					case tHash::tHashCT("auto"):	flags &= ~(gamaF | srgbF); flags |= autoF;	break;
					case tHash::tHashCT("gamc"):	flags &= ~(srgbF | autoF); flags |= gamaF;	break;
					case tHash::tHashCT("srgb"):	flags &= ~(gamaF | autoF); flags |= srgbF;	break;
				}
				break;
			}

			case tHash::tHashCT("gamma"):
				LoadParamsPKM.Gamma = (value == "*") ? 2.2f : tMath::tClamp(value.AsFloat(), 0.5f, 4.0f);
				break;

			case tHash::tHashCT("spred"):
			{
				bool spread = (value == "*") ? true : value.AsBool();
				if (spread)
					LoadParamsPKM.Flags |= tImage::tImagePKM::LoadFlag_SpreadLuminance;
				else
					LoadParamsPKM.Flags &= ~(tImage::tImagePKM::LoadFlag_SpreadLuminance);
				break;
			}
		}
	}
}


void Command::ParseLoadParametersPNG()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionInPNG.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("strct"):
			{
				bool strict = (value == "*") ? false : value.AsBool();
				if (!strict)
					LoadParamsPNG.Flags |= tImage::tImagePNG::LoadFlag_AllowJPG;
				else
					LoadParamsPNG.Flags &= ~(tImage::tImagePNG::LoadFlag_AllowJPG);
				break;
			}

			case tHash::tHashCT("apng"):
				LoadParams_DetectAPNGInsidePNG = (value == "*") ? false : value.AsBool();
				break;
		}
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

	tPrintfFull("Input files:\n");
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
		tPrintfFull("File: %s\n", info->FileName.Chr());
}


void Command::PopulateImagesList()
{
	// This doesn't actually load the images. It just prepares them on the Images list.
	// a) Depending of the filetype it may set custom load parameters.
	// b) It also turns off the undo-stack since we don't use that in CLI mode.
	for (tSystem::tFileInfo* info = InputFiles.First(); info; info = info->Next())
	{
		Viewer::Image* newImage = new Viewer::Image(*info);
		newImage->SetUndoEnabled(false);

		tSystem::tFileType fileType = tSystem::tGetFileType(info->FileName);
		switch (fileType)
		{
			case tSystem::tFileType::ASTC:	newImage->LoadParams_ASTC = LoadParamsASTC;		break;
			case tSystem::tFileType::DDS:	newImage->LoadParams_DDS  = LoadParamsDDS;		break;
			case tSystem::tFileType::PVR:	newImage->LoadParams_PVR  = LoadParamsPVR;		break;
			case tSystem::tFileType::EXR:	newImage->LoadParams_EXR  = LoadParamsEXR;		break;
			case tSystem::tFileType::HDR:	newImage->LoadParams_HDR  = LoadParamsHDR;		break;
			case tSystem::tFileType::JPG:	newImage->LoadParams_JPG  = LoadParamsJPG;		break;
			case tSystem::tFileType::KTX:	newImage->LoadParams_KTX  = LoadParamsKTX;		break;
			case tSystem::tFileType::PKM:	newImage->LoadParams_PKM  = LoadParamsPKM;		break;
			case tSystem::tFileType::PNG:
				newImage->LoadParams_PNG = LoadParamsPNG;
				newImage->LoadParams_DetectAPNGInsidePNG = LoadParams_DetectAPNGInsidePNG;
				break;
		}

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
		// It should also handle rotate[] and rotate by itself.
		tString str = *opstr;
		tString op = str.ExtractLeft('[');

		// If op is empty it means the '[' wasn't present. In this case the op needs to be set to str.
		if (op.IsEmpty())
			op = str;

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
		// It should also handle contact[] and contact by itself.
		tString str = *postr;
		tString po = str.ExtractLeft('[');

		// If po is empty it means the '[' wasn't present. In this case po needs to be set to str.
		if (po.IsEmpty())
			po = str;

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


void Command::DetermineOutputTypes()
{
	if (OptionOutTypes)
	{
		tList<tStringItem> types;
		OptionOutTypes.GetArgs(types);
		for (tStringItem* typ = types.First(); typ; typ = typ->Next())
		{
			// Each argument will either be a single type, or a comma-sperated list of types.
			tList<tStringItem> sepTypes;
			tStd::tExplode(sepTypes, *typ, ',');
			for (tStringItem* t = sepTypes.First(); t; t = t->Next())
			{
				tSystem::tFileType ft = tSystem::tGetFileTypeFromName(*t);
				if (ft == tSystem::tFileType::Invalid)
					tPrintfNorm("Warning: Unknown output file type: %s\n", t->Chr());
				else if (!Viewer::FileTypes_Load.Contains(ft))
					tPrintfNorm("Warning: Unsupported output image type: %s\n", t->Chr());
				else
					OutTypes.Add(ft);
			}
		}
	}

	// The default outtype if nothing is specified is tga.
	if (OutTypes.IsEmpty())
		OutTypes.Add(tSystem::tFileType::TGA);
}


void Command::DetermineOutputNameParameters()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutName.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("prefix"):
				OutNamePrefix = value;
				break;

			case tHash::tHashCT("suffix"):
				OutNameSuffix = value;
				break;

			case tHash::tHashCT("replace"):
			{
				if (value.FindChar(':') == -1)
				{
					OutNameSearch = value;
					OutNameReplace.Clear();
				}
				else
				{
					OutNameSearch = value.Left(':');
					OutNameReplace = value.Right(':');
				}
				break;
			}
		}
	}
}


void Command::DetermineOutputSaveParameters()
{
	// We only bother reading save parameters for the types of files we will be saving.
	for (tSystem::tFileTypes::tFileTypeItem* typeItem = OutTypes.First(); typeItem; typeItem = typeItem->Next())
	{
		tSystem::tFileType fileType = typeItem->FileType;
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
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutAPNG.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("24"):		SaveParamsAPNG.Format = tImage::tImageAPNG::tFormat::BPP24;		break;
					case tHash::tHashCT("32"):		SaveParamsAPNG.Format = tImage::tImageAPNG::tFormat::BPP32;		break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsAPNG.Format = tImage::tImageAPNG::tFormat::Auto;		break;
				}
				break;

			case tHash::tHashCT("dur"):
				SaveParamsAPNG.OverrideFrameDuration = (value == "*") ? -1 : tMath::tClampMin(value.AsInt(), -1);
				break;
		}
	}
}


void Command::ParseSaveParametersBMP()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutBMP.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("24"):		SaveParamsBMP.Format = tImage::tImageBMP::tFormat::BPP24;		break;
					case tHash::tHashCT("32"):		SaveParamsBMP.Format = tImage::tImageBMP::tFormat::BPP32;		break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsBMP.Format = tImage::tImageBMP::tFormat::Auto;		break;
				}
				break;
		}
	}
}


void Command::ParseSaveParametersGIF()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutGIF.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				if (value == "*")
					SaveParamsGIF.Format = tImage::tPixelFormat::PAL8BIT;
				else
					SaveParamsGIF.Format = tImage::tPixelFormat(int(tImage::tPixelFormat::PAL1BIT) + tMath::tClamp(value.AsInt32(), 1, 8) - 1);
				break;

			case tHash::tHashCT("qan"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("fix"):	SaveParamsGIF.Method = tImage::tQuantize::Method::Fixed;		break;
					case tHash::tHashCT("spc"):	SaveParamsGIF.Method = tImage::tQuantize::Method::Spatial;		break;
					case tHash::tHashCT("neu"):	SaveParamsGIF.Method = tImage::tQuantize::Method::Neu;			break;
					case tHash::tHashCT("wu"):
					case tHash::tHashCT("*"):	SaveParamsGIF.Method = tImage::tQuantize::Method::Wu;			break;
				}
				break;

			case tHash::tHashCT("loop"):
				if (value == "*")
					SaveParamsGIF.Loop = 0;
				else
					SaveParamsGIF.Loop = value.AsInt32();
				break;

			case tHash::tHashCT("alp"):
				if (value == "*")
					SaveParamsGIF.AlphaThreshold = -1;
				else
					SaveParamsGIF.AlphaThreshold = value.AsInt32();
				break;

			case tHash::tHashCT("dur"):
				SaveParamsGIF.OverrideFrameDuration = (value == "*") ? -1 : tMath::tClampMin(value.AsInt(), -1);
				break;

			case tHash::tHashCT("dith"):
				SaveParamsGIF.DitherLevel = (value == "*") ? 0.0 : tMath::tClampMin(value.AsDouble(), 0.0);
				break;

			case tHash::tHashCT("filt"):
				if (value == "*")
					SaveParamsGIF.FilterSize = 3;
				else
					SaveParamsGIF.FilterSize = value.AsInt32();
				if ((SaveParamsGIF.FilterSize != 1) && (SaveParamsGIF.FilterSize != 3) && (SaveParamsGIF.FilterSize != 5))
					SaveParamsGIF.FilterSize = 3;
				break;

			case tHash::tHashCT("samp"):
				SaveParamsGIF.SampleFactor = (value == "*") ? 1 : value.AsInt32();
				break;
		}
	}
}


void Command::ParseSaveParametersJPG()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutJPG.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("qual"):
				if (value == "*")
					SaveParamsJPG.Quality = 95;
				else
					SaveParamsJPG.Quality = tMath::tClamp(value.AsInt(), 1, 100);
				break;
		}
	}
}


void Command::ParseSaveParametersPNG()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutPNG.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("24"):		SaveParamsPNG.Format = tImage::tImagePNG::tFormat::BPP24_RGB_BPC8;	break;
					case tHash::tHashCT("32"):		SaveParamsPNG.Format = tImage::tImagePNG::tFormat::BPP32_RGBA_BPC8;	break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsPNG.Format = tImage::tImagePNG::tFormat::Auto;			break;
				}
				break;
		}
	}
}


void Command::ParseSaveParametersQOI()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutQOI.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("24"):		SaveParamsQOI.Format = tImage::tImageQOI::tFormat::BPP24;		break;
					case tHash::tHashCT("32"):		SaveParamsQOI.Format = tImage::tImageQOI::tFormat::BPP32;		break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsQOI.Format = tImage::tImageQOI::tFormat::Auto;		break;
				}
				break;

			case tHash::tHashCT("spc"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("srgb"):	SaveParamsQOI.ColourProfile = tColourProfile::sRGB;				break;
					case tHash::tHashCT("lin"):		SaveParamsQOI.ColourProfile = tColourProfile::lRGB;				break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsQOI.ColourProfile = tColourProfile::Auto;				break;
				}
				break;
		}
	}
}


void Command::ParseSaveParametersTGA()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutTGA.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("24"):		SaveParamsTGA.Format = tImage::tImageTGA::tFormat::BPP24;		break;
					case tHash::tHashCT("32"):		SaveParamsTGA.Format = tImage::tImageTGA::tFormat::BPP32;		break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsTGA.Format = tImage::tImageTGA::tFormat::Auto;		break;
				}
				break;

			case tHash::tHashCT("rle"):
			{
				bool rle = (value == "*") ? false : value.AsBool();
				SaveParamsTGA.Compression = rle ? tImage::tImageTGA::tCompression::RLE : tImage::tImageTGA::tCompression::None;
				break;
			}
		}
	}
}


void Command::ParseSaveParametersTIFF()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutTIFF.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("bpp"):
				switch (tHash::tHashString(value.Chr()))
				{
					case tHash::tHashCT("24"):		SaveParamsTIFF.Format = tImage::tImageTIFF::tFormat::BPP24;		break;
					case tHash::tHashCT("32"):		SaveParamsTIFF.Format = tImage::tImageTIFF::tFormat::BPP32;		break;
					case tHash::tHashCT("auto"):
					case tHash::tHashCT("*"):		SaveParamsTIFF.Format = tImage::tImageTIFF::tFormat::Auto;		break;
				}
				break;

			case tHash::tHashCT("zlib"):
				SaveParamsTIFF.UseZLibCompression = (value == "*") ? true : value.AsBool();
				break;

			case tHash::tHashCT("dur"):
				SaveParamsTIFF.OverrideFrameDuration = (value == "*") ? -1 : tMath::tClampMin(value.AsInt(), -1);
				break;
		}
	}
}


void Command::ParseSaveParametersWEBP()
{
	tList<ParamValuePair> pairs;
	ParseParamValuePairs(pairs, OptionOutWEBP.Arg1());
	for (ParamValuePair* p = pairs.First(); p; p = p->Next())
	{
		tString& param = p->Param;
		tString& value = p->Value;
		switch (tHash::tHashString(param.Chr()))
		{
			case tHash::tHashCT("loss"):
				SaveParamsWEBP.Lossy = (value == "*") ? false : value.AsBool();
				break;

			case tHash::tHashCT("qual"):
				if (value == "*")
					SaveParamsWEBP.QualityCompstr = 90.0f;
				else
					SaveParamsWEBP.QualityCompstr = tMath::tClamp(value.AsFloat(), 0.0f, 100.0f);
				break;

			case tHash::tHashCT("dur"):
				SaveParamsWEBP.OverrideFrameDuration = (value == "*") ? -1 : tMath::tClampMin(value.AsInt(), -1);
				break;
		}
	}
}


tString Command::DetermineOutputFilename(const tString& inName, tSystem::tFileType outType)
{
	tString baseName = tSystem::tGetFileBaseName(inName);

	if (OutNameSearch.IsValid())
		baseName.Replace(OutNameSearch.Chr(), OutNameReplace.Chr());
	if (OutNamePrefix.IsValid())
		baseName = OutNamePrefix + baseName;
	if (OutNameSuffix.IsValid())
		baseName = baseName + OutNameSuffix;

	tString outExt = tSystem::tGetExtension(outType);
	tString outName = tSystem::tGetDir(inName) + baseName + "." + outExt;

	if (!OptionAutoName)
		return outName;

	// Autoname is true. Test up to 100 consecutive filenames.
	for (int nameIter = 0; nameIter < 100; nameIter++)
	{
		tString contender;
		if (nameIter == 0)
			contender = outName;
		else
			tsPrintf(contender, "%s%s_%03d.%s", tSystem::tGetDir(inName).Chr(), baseName.Chr(), nameIter, outExt.Chr());

		if (!tSystem::tFileExists(contender))
			return contender;
	}

	return outName;
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

	if (OptionMarkdown)
	{
		Command::PrintExamplesMarkdown();
		return 0;
	}
	else
	{
		tPrintf("\n");
		tPrintfNorm("Tacent View %d.%d.%d by Tristan Grimmer\n", ViewerVersion::Major, ViewerVersion::Minor, ViewerVersion::Revision);
		tString platform	= tGetPlatformNameShort( tGetPlatform() );
		tString architec	= tGetArchitectureName( tGetArchitecture() );
		tString config		= tGetConfigurationName( tGetConfiguration() );
		#ifdef TACENT_UTF16_API_CALLS
		tString api			= "UTF-16";
		#else
		tString api			= "UTF-8";
		#endif
		tPrintfNorm("%s %s %s %s API\n", platform.Chr(), architec.Chr(), config.Chr(), api.Chr());

		if (OptionHelp)
			tPrintfNorm("CLI Mode\n");
		else
			tPrintfNorm("CLI Mode Details: tacentview --help\n");
		tPrintfNorm("\n");
	}

	if (OptionHelp)
	{
		Command::PrintHelp();
		return 0;
	}

	if (OptionSyntax)
	{
		tCmdLine::tPrintSyntax();
		return 0;
	}

	if (OptionExamples)
	{
		Command::PrintExamples();
		return 0;
	}

	// Determine what input types will be processed when specifying a directory.
	DetermineInputTypes();
	DetermineInputLoadParameters();

	// Collect all input files into a single list.
	DetermineInputFiles();

	// Populates the Images list. Each added image gets its load-parameters set correctly and the undo-stack turned
	// off. Does not load the images.
	PopulateImagesList();

	// Populates the Operations list.
	PopulateOperations();

	// Populates the PostOperations list.
	PopulatePostOperations();

	DetermineOutputTypes();
	DetermineOutputNameParameters();
	DetermineOutputSaveParameters();

	// Process standard operations.
	// We do the images one at a time to save memory. That is, we only need to load one image in at a time
	// and can unload them when done.
	bool somethingFailed = false;
	for (Viewer::Image* image = Images.First(); image; image = image->Next())
	{
		// We do not read the config file when using the CLI. All parameters need to com from the command-line.
		bool loadParamsFromConfig = false;
		image->Load(loadParamsFromConfig);

		tString inNameShort = tSystem::tGetFileName(image->Filename);
		if (!image->IsLoaded())
		{
			tPrintfNorm("Warning: Failed load: %s. Skipping.\n", inNameShort.Chr());
			somethingFailed = true;
			if (OptionEarlyExit)
				return 1;
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
				return 1;
			continue;
		}

		// Some operations do not modify the input image at all. For example, the extract operation saves every frame
		// of the input image but does not modify it. In these cases the image dirty flag is not set so we can
		// skip saving if OptionSkipUnchanged is true.
		if (OptionSkipUnchanged && !image->IsDirty())
		{
			tPrintfNorm("Skipping unchanged: %s\n", inNameShort.Chr());
			image->Unload();
			continue;
		}

		// Now we iterate through the output types, saving if needed.
		tAssert(OutTypes.Count() >= 1);
		for (tSystem::tFileTypes::tFileTypeItem* typeItem = OutTypes.First(); typeItem; typeItem = typeItem->Next())
		{
			tSystem::tFileType outType = typeItem->FileType;

			// Determine out filename.
			tString outFilename = DetermineOutputFilename(image->Filename, outType);
			tString outNameShort = tSystem::tGetFileName(outFilename);
			if (!OptionOverwrite && tSystem::tFileExists(outFilename))
			{
				tPrintfNorm("Warning: %s exists. No overwrite.\n", outNameShort.Chr());
				somethingFailed = true;
				if (OptionEarlyExit)
				{
					image->Unload();
					return 1;
				}
				continue;
			}

			// Set the image save parameters correctly. The user may have modified them from the command line.
			SetImageSaveParameters(*image, outType);
			bool success = image->Save(outFilename, outType, false);
			if (success)
			{
				tPrintfNorm("Saved File: %s\n", outNameShort.Chr());
			}
			else
			{
				tPrintfNorm("Warning: Failed save: %s\n", outNameShort.Chr());
				somethingFailed = true;
				if (OptionEarlyExit)
				{
					image->Unload();
					return 1;
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
	// Example post-operations include: combining multiple images into a single animated image or creating
	// a contact sheet from multiple images.
	if (!PostOperations.IsEmpty())
	{
		if (Images.Count() < 2)
		{
			tPrintfNorm("Warning: Post operations require 2+ input images. Skipping.\n");
		}
		else
		{
			tPrintfFull("Processing post operations on %d images.\n", Images.Count());
			for (PostOperation* postop = PostOperations.First(); postop; postop = postop->Next())
			{
				if (!postop->Valid)
					continue;

				tPrintfNorm("Processing post operation: %s\n", postop->GetName());
				bool success = postop->Apply(Images);
				if (!success)
				{
					tPrintfNorm("Warning: Failed post operation: %s\n", postop->GetName());
					somethingFailed = true;
					if (OptionEarlyExit)
						return 1;
				}
			}
			tPrintfFull("Done processing post operations on %d images.\n", Images.Count());
		}
	}

	return somethingFailed ? 1 : 0;
}
