// ImportRaw.cpp
//
// Import contiguous raw pixel data from any file.
//
// Copyright (c) 2024 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tVector2.h>
#include <Math/tColour.h>
#include <System/tFile.h>
#include <Image/tPixelFormat.h>
#include <Image/tPixelUtil.h>
#include <Image/tFrame.h>
#include <Image/tImageTIFF.h>
#include <Image/tImageAPNG.h>
#include <Image/tImageWEBP.h>
#include <Image/tImageTGA.h>
#include <Image/tImagePNG.h>
#include <Image/tImageQOI.h>
#include <Image/tImageBMP.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "GuiUtil.h"
#include "ImportRaw.h"
#include "Config.h"
#include "FileDialog.h"
#include "TacentView.h"
#include "Image.h"
using namespace tMath;
using namespace tFileDialog;
using namespace tImage;


namespace ImportRaw
{
	FileDialog SelectFileDialog(DialogMode::OpenFile);
	tString ImportedFile;

	enum class CreateResult
	{
		Success,
		UnsupportedFormat,
		DataShortage,
		DecodeError
	};
	CreateResult CreateFrames(tList<tFrame>& frames, const tString& rawFile, tImage::tPixelFormat rawFmt, int width, int height, int offset, bool mipmaps, bool mipmapForceSameFrameSize);
	tString MakeImportedFilename(tSystem::tFileType destType, const tString& rawFile);
	bool CreateImportedFile(tList<tFrame>& frames, const tString& filename);
}


void Viewer::ShowImportRawOverlay(bool* popen, bool justOpened)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::ImportRaw);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("Import Raw", popen, flags))
	{
		Config::ProfileData& profile = Config::GetProfileData();
		if (justOpened)
		{
			ImportRaw::ImportedFile.Clear();
			if (!tSystem::tFileExists(profile.ImportRawFilename))
				profile.ImportRawFilename.Clear();
		}

		if (Gutil::Button("Select File"))
			ImportRaw::SelectFileDialog.OpenPopup();

		FileDialog::DialogState state = ImportRaw::SelectFileDialog.DoPopup();

		// You will get an OK dialog state once after the DoPopup when OK is pressed.
		switch (state)
		{
			case FileDialog::DialogState::Cancel: break;
			case FileDialog::DialogState::Closed: break;
			case FileDialog::DialogState::OK:
				profile.ImportRawFilename = ImportRaw::SelectFileDialog.GetResult();
				break;
			case FileDialog::DialogState::Open: break;
		}

		if (profile.ImportRawFilename.IsValid())
		{
			tString importName = tSystem::tGetFileName(profile.ImportRawFilename);
			ImGui::SameLine();
			ImGui::Text(importName.Chr());
		}

		// FileType of destination image.
		tString createTypeName = profile.ImportRawFileType;
		tSystem::tFileType dstType = tSystem::tGetFileTypeFromName(createTypeName);
		tSystem::tFileType newType = dstType;
		bool filetypeChanged = false;
		if (ImGui::BeginCombo("Dest Type", createTypeName.Chr()))
		{
			for (tSystem::tFileTypes::tFileTypeItem* item = FileTypes_ImportRaw.First(); item; item = item->Next())
			{
				tSystem::tFileType ft = item->FileType;
				bool selected = (ft == dstType);

				tString ftName = tGetFileTypeName(ft);
				if (ImGui::Selectable(ftName.Chr(), &selected))
				{
					profile.ImportRawFileType = ftName;
					newType = tSystem::tGetFileTypeFromName(ftName);
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}				
			ImGui::EndCombo();
		}
		if (dstType != newType)
		{
			dstType = newType;

			// If it changed and an existing file exists, delete it and clear the ImportedFile.
			if (ImportRaw::ImportedFile.IsValid())
			{
				bool deleted = DeleteImageFile(ImportRaw::ImportedFile, false);
				if (deleted)
					ImportRaw::ImportedFile.Clear();
			}
		}
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"When an image is imported from raw data it creates a new image of this type.\n"
			"Valid types are ones that are lossless or support lossless encoding like webp.\n"
			"Created images support alpha channel. If no alpha it saves the image without it.\n"
			"If the creation type supports multiple frames, the option to import mipmaps will\n"
			"be available. TIFF, APNG, and WEBP support mipmaps."
		);

		if (ImGui::InputInt("Data Offset##ImportRaw", &profile.ImportRawDataOffset))
			tiClamp(profile.ImportRawDataOffset, 0, Viewer::Image::MaxDim-1);

		if (ImGui::InputInt("Width##ImportRaw", &profile.ImportRawWidth))
			tiClamp(profile.ImportRawWidth, 1, Viewer::Image::MaxDim);

		if (ImGui::InputInt("Height##ImportRaw", &profile.ImportRawHeight))
			tiClamp(profile.ImportRawHeight, 1, Viewer::Image::MaxDim);

		bool fileTypeSupportsMipmaps = (dstType == tSystem::tFileType::TIFF) || (dstType == tSystem::tFileType::APNG) || (dstType == tSystem::tFileType::WEBP);
		bool mipmapForceSameFrameSize = (dstType == tSystem::tFileType::APNG) || (dstType == tSystem::tFileType::WEBP);
		if (!fileTypeSupportsMipmaps)
			Gutil::PushDisable();
		ImGui::Checkbox("Mipmaps##ImportRaw", &profile.ImportRawMipmaps);
		if (!fileTypeSupportsMipmaps)
			Gutil::PopDisable();
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"Filetypes TIFF, APNG, and WEBP support mipmaps.\n"
			"TIFF: Saved mipmap frames have correct sizes.\n"
			"APNG: Saved mipmap frames of same size. Unused areas will be opaque black.\n"
			"WEBP: Saved mipmap frames of same size. Unused areas will be opaque black."
		);

		bool importMipmaps = fileTypeSupportsMipmaps ? profile.ImportRawMipmaps : false;

		static bool liveUpdates = false;
		ImGui::Checkbox("Live Updates", &liveUpdates);
		ImGui::SameLine();
		Gutil::HelpMark("Automatically re-import on any change.");

		Gutil::Separator();
		tImage::tPixelFormat fmt = profile.GetImportRawPixelFormat();
		int currPacked = tImage::tIsPackedFormat(fmt) ? int(fmt) - int(tImage::tPixelFormat::FirstPacked) : -1;
		int currBlock  = tImage::tIsBCFormat(fmt)     ? int(fmt) - int(tImage::tPixelFormat::FirstBC)     : -1;
		int currPVR    = tImage::tIsPVRFormat(fmt)    ? int(fmt) - int(tImage::tPixelFormat::FirstPVR)    : -1;
		int currASTC   = tImage::tIsASTCFormat(fmt)   ? int(fmt) - int(tImage::tPixelFormat::FirstASTC)   : -1;
		const int maxDropdownFormats = 14;
		ImGui::Text("Pixel Format:");
		if (Gutil::Combo("Packed", &currPacked, tImage::PixelFormatNames_Packed, tImage::PixelFormatDescs_Packed, int(tImage::tPixelFormat::NumPackedFormats), tMin(int(tImage::tPixelFormat::NumPackedFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPacked) + currPacked;
		if (currPacked != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_Packed[currPacked]);

		if (Gutil::Combo("BC", &currBlock, tImage::PixelFormatNames_Block, tImage::PixelFormatDescs_Block, int(tImage::tPixelFormat::NumBCFormats), tMin(int(tImage::tPixelFormat::NumBCFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstBC) + currBlock;
		if (currBlock != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_Block[currBlock]);

		if (Gutil::Combo("PVR", &currPVR, tImage::PixelFormatNames_PVR, tImage::PixelFormatDescs_PVR, int(tImage::tPixelFormat::NumPVRFormats), tMin(int(tImage::tPixelFormat::NumPVRFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPVR) + currPVR;
		if (currPVR != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_PVR[currPVR]);

		if (Gutil::Combo("ASTC", &currASTC, tImage::PixelFormatNames_ASTC, tImage::PixelFormatDescs_ASTC, int(tImage::tPixelFormat::NumASTCFormats), tMin(int(tImage::tPixelFormat::NumASTCFormats), maxDropdownFormats)))
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstASTC) + currASTC;
		if (currASTC != -1)
			Gutil::ToolTip(tImage::PixelFormatDescs_ASTC[currASTC]);

		if (profile.ImportRawFilename.IsValid())
		{
			if (Gutil::Button("Import"))
			{
				tList<tFrame> frames;
				ImportRaw::CreateResult cr = ImportRaw::CreateFrames(frames, profile.ImportRawFilename, profile.GetImportRawPixelFormat(), profile.ImportRawWidth, profile.ImportRawHeight, profile.ImportRawDataOffset, importMipmaps, mipmapForceSameFrameSize);
				if (cr == ImportRaw::CreateResult::Success)
				{
					tString importedFilename = ImportRaw::ImportedFile;
					if (importedFilename.IsEmpty())
						importedFilename = ImportRaw::MakeImportedFilename(dstType, profile.ImportRawFilename);
					if (importedFilename.IsEmpty())
					{
						// Update message.
					}
					else
					{
						bool exists = ImportRaw::CreateImportedFile(frames, importedFilename);
						if (exists)
						{
							// We only set ImportedFile once we know it exists.
							ImportRaw::ImportedFile = importedFilename;
							bool found = SetCurrentImage(ImportRaw::ImportedFile);
							if (found)
							{
								CurrImage->Unbind();
								CurrImage->Unload(true);
								CurrImage->Load();
								CurrImage->Bind();
							}
							else
							{
								Image* newImg = new Image(ImportRaw::ImportedFile);
								Images.Append(newImg);
								ImagesLoadTimeSorted.Append(newImg);
								SortImages(profile.GetSortKey(), profile.SortAscending);
								SetCurrentImage(importedFilename);
							}
						}
						else
						{
							// Message.
						}
					}
				}
				else
				{
					// @todo Set Message Text.
					switch (cr)
					{
						case ImportRaw::CreateResult::UnsupportedFormat:	break;
						case ImportRaw::CreateResult::DataShortage:			break;
						case ImportRaw::CreateResult::DecodeError:			break;
					}
				}
			}
		}

		if (ImGui::Button("Cancel"))
		{
			if (ImportRaw::ImportedFile.IsValid())
			{
				bool deleted = DeleteImageFile(ImportRaw::ImportedFile, false);
				if (deleted)
					ImportRaw::ImportedFile.Clear();
			}
			*popen = false;
		}
		// @wip ImportRawColourProfile
	}

	ImGui::End();
}


ImportRaw::CreateResult ImportRaw::CreateFrames(tList<tFrame>& frames, const tString& rawFile, tImage::tPixelFormat rawFmt, int width, int height, int offset, bool mipmaps, bool mipmapForceSameFrameSize)
{
	if (rawFile.IsEmpty() || !tSystem::tFileExists(rawFile))
		return ImportRaw::CreateResult::DataShortage;

	if ((width <= 0) || (height <= 0) || (offset < 0))
		return ImportRaw::CreateResult::DecodeError;

	// How much data do we have to work with?
	int fileSize = tSystem::tGetFileSize(rawFile);
	int dataHave = fileSize - offset;

	// How much data do we need?
	int blockW = tGetBlockWidth(rawFmt);		// These return 1 for packed.
	int blockH = tGetBlockHeight(rawFmt);
	int bytesPerBlock = tGetBytesPerBlock(rawFmt);

	int numLevels = mipmaps ? tImage::tGetNumMipmapLevels(width, height) : 1;
	int dataNeeded = 0;
	for (int lev = 0; lev < numLevels; lev++)
	{
		int levW = tImage::tGetMipmapDim(width, lev);
		int levH = tImage::tGetMipmapDim(height, lev);
		int blocksNeededW = tGetNumBlocks(blockW, levW);
		int blocksNeededH = tGetNumBlocks(blockH, levH);
		dataNeeded += blocksNeededW * blocksNeededH * bytesPerBlock;
	}

	// Do we have enough?
	if (dataHave < dataNeeded)
		return ImportRaw::CreateResult::DataShortage;

	// Get the raw data. We only need to read as much as we need.
	int numReadBytes = dataNeeded+offset;
	uint8* rawDataStart = tSystem::tLoadFileHead(rawFile, numReadBytes);
	if (numReadBytes != dataNeeded+offset)
	{
		delete[] rawDataStart;
		return ImportRaw::CreateResult::DataShortage;
	}

	uint8* rawPixelData = rawDataStart + offset;

	int mipW = width; int mipH = height;
	for (int lev = 0; lev < numLevels; lev++)
	{
		int blocksW = tGetNumBlocks(blockW, mipW);
		int blocksH = tGetNumBlocks(blockH, mipH);
		int needed = blocksW * blocksH * bytesPerBlock;

		tPixel4b* pixelsLDR = nullptr;
		tPixel4f* pixelsHDR = nullptr;
		tImage::DecodeResult result = DecodePixelData
		(
			rawFmt, rawPixelData, needed, mipW, mipH,
			pixelsLDR, pixelsHDR
		);

		if (result != tImage::DecodeResult::Success)
		{
			delete[] rawDataStart;
			delete[] pixelsLDR;
			delete[] pixelsHDR;
		}

		switch (result)
		{
			case tImage::DecodeResult::Success:
				break;
			case tImage::DecodeResult::BuffersNotClear:
			case tImage::DecodeResult::InvalidInput:
				return ImportRaw::CreateResult::DataShortage;
			case tImage::DecodeResult::UnsupportedFormat:
				return ImportRaw::CreateResult::UnsupportedFormat;
			case tImage::DecodeResult::PackedDecodeError:
			case tImage::DecodeResult::BlockDecodeError:
			case tImage::DecodeResult::ASTCDecodeError:
			case tImage::DecodeResult::PVRDecodeError:
			default:
				return ImportRaw::CreateResult::DecodeError;
		}

		// At this point either pixelsHDR or pixelsLDR will be valid (but not both).
		tAssert((pixelsLDR || pixelsHDR) && (!pixelsLDR || !pixelsHDR));

		// Since the internal representation of data in the viewer is tPixel4b (32-bit) we convert here if necessary.
		if (!pixelsLDR)
		{
			pixelsLDR = new tPixel4b[mipW*mipH];
			for (int p = 0; p < mipW*mipH; p++)
				pixelsLDR[p].Set(pixelsHDR[p]);
			delete[] pixelsHDR;
			pixelsHDR = nullptr;
		}

		tFrame* frame = new tFrame;
		if (mipmaps && mipmapForceSameFrameSize)
		{
			tPixel4b* pixels = new tPixel4b[width*height];
			for (int p = 0; p < width*height; p++)
				pixels[p] = tPixel4b::black;
			frame->StealFrom(pixels, width, height);

			for (int y = 0; y < mipH; y++)
				for (int x = 0; x < mipW; x++)
					frame->SetPixel(x, y, pixelsLDR[y*mipW + x]);
			delete[] pixelsLDR;
		}
		else
		{
			frame->StealFrom(pixelsLDR, mipW, mipH);
		}
		frames.Append(frame);

		tImage::tGetNextMipmapLevelDims(mipW, mipH);
		rawPixelData += needed;
	}

	delete[] rawDataStart;
	return ImportRaw::CreateResult::Success;
}


tString ImportRaw::MakeImportedFilename(tSystem::tFileType destType, const tString& rawFile)
{
	tString dir = tSystem::tGetDir(rawFile);
	tString base = tSystem::tGetFileBaseName(rawFile);
	tString filename;
	int attempt = 0;
	do
	{
		tsPrintf
		(
			filename, "%s%s_Imported_%02d.%s",
			Viewer::ImagesDir.Chr(),
			base.Chr(),
			attempt,
			tGetExtension(destType).Chr()
		);
		attempt++;
	} while (tSystem::tFileExists(filename) && (attempt < 100));

	return filename;
}


bool ImportRaw::CreateImportedFile(tList<tFrame>& frames, const tString& filename)
{
	// OverWrite the file if necessary.
	tSystem::tFileType type = tSystem::tGetFileTypeFromExtension( tSystem::tGetFileExtension(filename) );

	switch (type)
	{
		case tSystem::tFileType::TIFF:
		{
			tImage::tImageTIFF tiff;
			tiff.Set(frames, true);
			bool zlibComp = true;
			bool saved = tiff.Save(filename, tImage::tImageTIFF::tFormat::BPP32, zlibComp, 1000);
			return saved;
		}

		case tSystem::tFileType::APNG:
		{
			tImage::tImageAPNG apng;
			apng.Set(frames, true);
			tImage::tImageAPNG::tFormat savedFmt = apng.Save(filename, tImage::tImageAPNG::tFormat::Auto, 1000);
			return (savedFmt != tImage::tImageAPNG::tFormat::Invalid);
		}

		case tSystem::tFileType::WEBP:
		{
			tImage::tImageWEBP webp;
			webp.Set(frames, true);
			bool saved = webp.Save(filename, false, 90.0f, 1000);
			return saved;
		}

		case tSystem::tFileType::TGA:
		{
			tImage::tImageTGA tga;
			tga.Set(frames.Remove(), true);
			tImageTGA::tFormat savedFmt = tga.Save(filename, tImageTGA::tFormat::Auto);
			return (savedFmt != tImage::tImageTGA::tFormat::Invalid);
		}

		case tSystem::tFileType::PNG:
		{
			tImage::tImagePNG png;
			png.Set(frames.Remove(), true);
			tImagePNG::tFormat savedFmt = png.Save(filename, tImagePNG::tFormat::Auto);
			return (savedFmt != tImage::tImagePNG::tFormat::Invalid);
		}

		case tSystem::tFileType::QOI:
		{
			tImage::tImageQOI qoi;
			qoi.Set(frames.Remove(), true);
			tImageQOI::tFormat savedFmt = qoi.Save(filename, tImageQOI::tFormat::Auto);
			return (savedFmt != tImage::tImageQOI::tFormat::Invalid);
		}

		case tSystem::tFileType::BMP:
		{
			tImage::tImageBMP bmp;
			bmp.Set(frames.Remove(), true);
			tImageBMP::tFormat savedFmt = bmp.Save(filename, tImageBMP::tFormat::Auto);
			return (savedFmt != tImage::tImageBMP::tFormat::Invalid);
		}
	}

	return false;
}
