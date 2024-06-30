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
	tString ImportedDstFile;

	enum class CreateResult
	{
		Success,
		UnsupportedFormat,
		DataShortage,
		DecodeError
	};
	CreateResult CreateFrames
	(
		tList<tFrame>& frames, const tString& rawFile, tImage::tPixelFormat rawFmt,
		int width, int height, int offset,
		bool mipmaps, bool mipmapForceSameFrameSize, int surfaceOrMipmapCount,
		bool undoAlphaPremult, tColourSpace
	);
	tString MakeImportedFilename(tSystem::tFileType destType, const tString& rawFile);
	bool CreateImportedFile(tList<tFrame>& frames, const tString& filename);
}


bool Viewer::ShowImportRawOverlay(bool* popen, bool justOpened)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::ImportRaw);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoNav				|	ImGuiWindowFlags_NoScrollbar	;

	//ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;
	float buttonWidth	= Gutil::GetUIParamScaled(80.0f, 2.5f);
	float rightButtons	= Gutil::GetUIParamExtent(196.0f, 480.0f);
	float itemWidth		= Gutil::GetUIParamScaled(140.0f, 2.5f);
	float nameWidth		= Gutil::GetUIParamScaled(180.0f, 2.5f);

	bool handledClose = false;
	if (ImGui::Begin("Import Raw", popen, flags))
	{
		Config::ProfileData& profile = Config::GetProfileData();
		static tString message;

		if (justOpened)
		{
			ImportRaw::ImportedDstFile.Clear();
			if (!tSystem::tFileExists(profile.ImportRawFilename))
				profile.ImportRawFilename.Clear();
			message.Clear();
		}

		if (Gutil::Button("Raw File", tVector2(buttonWidth, 0.0f)))
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
			importName = Gutil::CropStringToWidth(importName, nameWidth, true);

			ImGui::SameLine();
			ImGui::Text(importName.Chr());
			if (ImportRaw::ImportedDstFile.IsValid())
			{
				ImGui::Text(profile.ImportRawLiveUpdate ? "Live Preview" : "Preview");
			}
		}

		bool liveUpdated = false;
		const char* colourSpaceItems[] = { "sRGB", "lRGB" };
		int colSpace = (profile.GetImportRawColourSpace() == tColourSpace::lRGB) ? 1 : 0;
		ImGui::SetNextItemWidth(itemWidth);
		if (ImGui::Combo("Colour Space", &colSpace , colourSpaceItems, tNumElements(colourSpaceItems)))
		{
			profile.SetImportRawColourSpace((colSpace == 0) ? tColourSpace::sRGB : tColourSpace::lRGB);
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
 		}
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"Specify the colour space of the raw pixels being imported. Colour data in\n"
			"the viewer is in sRGB-space. If the imported colours are linear then\n"
			"selecting lRGB will make sure the imported pixels are converted to sRGB."
		);

		// FileType of destination image.
		tString createTypeName = profile.ImportRawFileType;
		tSystem::tFileType dstType = tSystem::tGetFileTypeFromName(createTypeName);
		tSystem::tFileType newType = dstType;
		bool filetypeChanged = false;
		ImGui::SetNextItemWidth(itemWidth);
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

			// If it changed and an existing file exists, delete it and clear the ImportedDstFile.
			if (ImportRaw::ImportedDstFile.IsValid())
			{
				bool deleted = DeleteImageFile(ImportRaw::ImportedDstFile, false);
				if (deleted)
					ImportRaw::ImportedDstFile.Clear();
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

		ImGui::SetNextItemWidth(itemWidth);
		if (ImGui::InputInt("Data Offset##ImportRaw", &profile.ImportRawDataOffset))
		{
			tiClamp(profile.ImportRawDataOffset, 0, Viewer::Image::MaxDim-1);
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		ImGui::SetNextItemWidth(itemWidth);
		if (ImGui::InputInt("Width##ImportRaw", &profile.ImportRawWidth))
		{
			tiClamp(profile.ImportRawWidth, 1, Viewer::Image::MaxDim);
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		ImGui::SetNextItemWidth(itemWidth);
		if (ImGui::InputInt("Height##ImportRaw", &profile.ImportRawHeight))
		{
			tiClamp(profile.ImportRawHeight, 1, Viewer::Image::MaxDim);
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		bool fileTypeSupportsMultipleFrames = (dstType == tSystem::tFileType::TIFF) || (dstType == tSystem::tFileType::APNG) || (dstType == tSystem::tFileType::WEBP);

		if (!fileTypeSupportsMultipleFrames)
			Gutil::PushDisable();

		static int surfaceOrMipmapCount = 1;
		ImGui::SetNextItemWidth(itemWidth);
		if (profile.ImportRawMipmaps)
		{
			if (ImGui::InputInt("Mipmap Count", &surfaceOrMipmapCount))
			{
				tiClamp(surfaceOrMipmapCount, 1, tImage::tGetNumMipmapLevels(profile.ImportRawWidth, profile.ImportRawHeight));
				if (profile.ImportRawLiveUpdate) liveUpdated = true;
			}
		}
		else
		{
			if (ImGui::InputInt("Surface Count", &surfaceOrMipmapCount))
			{
				tiClamp(surfaceOrMipmapCount, 1, 128);
				if (profile.ImportRawLiveUpdate) liveUpdated = true;
			}
		}

		bool mipmapForceSameFrameSize = (dstType == tSystem::tFileType::APNG) || (dstType == tSystem::tFileType::WEBP);
		ImGui::SetNextItemWidth(itemWidth);
		if (ImGui::Checkbox("Mipmaps##ImportRaw", &profile.ImportRawMipmaps))
		{
			surfaceOrMipmapCount = profile.ImportRawMipmaps ? tImage::tGetNumMipmapLevels(profile.ImportRawWidth, profile.ImportRawHeight) : 1;
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		if (!fileTypeSupportsMultipleFrames)
			Gutil::PopDisable();
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"Filetypes TIFF, APNG, and WEBP support mipmaps or multiple surfaces.\n"
			"TIFF: Mipmap frames will have correct sizes.\n"
			"APNG: Mipmap frames will be same size. Unused areas will be opaque black.\n"
			"WEBP: Mipmap frames will be same size. Unused areas will be opaque black."
		);
		bool importMipmaps = fileTypeSupportsMultipleFrames ? profile.ImportRawMipmaps : false;
		int surfOrMipCount = fileTypeSupportsMultipleFrames ? surfaceOrMipmapCount : 1;

		ImGui::SetNextItemWidth(itemWidth);
		if (ImGui::Checkbox("Premultiplied Alpha", &profile.ImportRawPremultAlpha))
		{
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"If the raw data contains premultiplied by alpha colour information\n"
			"checking this will undo it so it displays properly. Images in the\n"
			"viewer are displayed not premultiplied."
		);

		if (!ImportRaw::ImportedDstFile.IsValid())
			Gutil::PushDisable();
		ImGui::SetNextItemWidth(itemWidth);
		ImGui::Checkbox("Live Updates", &profile.ImportRawLiveUpdate);
		if (!ImportRaw::ImportedDstFile.IsValid())
			Gutil::PopDisable();
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"Automatically re-import on any change. This can be slow as the entire raw\n"
			"input file is re-parsed, decoded, saved, and re-bound for display. This\n"
			"option is only available after you have successfully imported at least once."
		);

		Gutil::Separator();
		tImage::tPixelFormat fmt = profile.GetImportRawPixelFormat();
		int currPacked = tImage::tIsPackedFormat(fmt) ? int(fmt) - int(tImage::tPixelFormat::FirstPacked) : -1;
		int currBlock  = tImage::tIsBCFormat(fmt)     ? int(fmt) - int(tImage::tPixelFormat::FirstBC)     : -1;
		int currPVR    = tImage::tIsPVRFormat(fmt)    ? int(fmt) - int(tImage::tPixelFormat::FirstPVR)    : -1;
		int currASTC   = tImage::tIsASTCFormat(fmt)   ? int(fmt) - int(tImage::tPixelFormat::FirstASTC)   : -1;
		const int maxDropdownFormats = 14;
		tString formatText;
		tsPrintf(formatText, "Pixel Format: %s", tGetPixelFormatName(fmt));
		ImGui::Text(formatText.Chr());
		if (fmt != tPixelFormat::Invalid)
			Gutil::ToolTip(tImage::PixelFormatDescs_Packed[currPacked]);

		ImGui::SetNextItemWidth(itemWidth);
		if (Gutil::Combo("Packed", &currPacked, tImage::PixelFormatNames_Packed, tImage::PixelFormatDescs_Packed, int(tImage::tPixelFormat::NumPackedFormats), tMin(int(tImage::tPixelFormat::NumPackedFormats), maxDropdownFormats)))
		{
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPacked) + currPacked;
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		ImGui::SetNextItemWidth(itemWidth);
		if (Gutil::Combo("BC", &currBlock, tImage::PixelFormatNames_Block, tImage::PixelFormatDescs_Block, int(tImage::tPixelFormat::NumBCFormats), tMin(int(tImage::tPixelFormat::NumBCFormats), maxDropdownFormats)))
		{
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstBC) + currBlock;
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		ImGui::SetNextItemWidth(itemWidth);
		if (Gutil::Combo("PVR", &currPVR, tImage::PixelFormatNames_PVR, tImage::PixelFormatDescs_PVR, int(tImage::tPixelFormat::NumPVRFormats), tMin(int(tImage::tPixelFormat::NumPVRFormats), maxDropdownFormats)))
		{
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstPVR) + currPVR;
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		ImGui::SetNextItemWidth(itemWidth);
		if (Gutil::Combo("ASTC", &currASTC, tImage::PixelFormatNames_ASTC, tImage::PixelFormatDescs_ASTC, int(tImage::tPixelFormat::NumASTCFormats), tMin(int(tImage::tPixelFormat::NumASTCFormats), maxDropdownFormats)))
		{
			profile.ImportRawPixelFormat = int(tImage::tPixelFormat::FirstASTC) + currASTC;
			if (profile.ImportRawLiveUpdate) liveUpdated = true;
		}

		Gutil::Separator();

		tString messageText;
		tsPrintf(messageText, "Message: %s", message.IsEmpty() ? "None" :  message.Chr());
		ImGui::Text(messageText.Chr());

		Gutil::Separator();

		if (ImGui::Button("Reset", tVector2(buttonWidth, 0.0f)))
		{
			if (ImportRaw::ImportedDstFile.IsValid())
				DeleteImageFile(ImportRaw::ImportedDstFile, false);
			ImportRaw::ImportedDstFile.Clear();
			Config::ResetProfile(Config::Category_ImportRaw);
			surfaceOrMipmapCount = 1;
		}

		if (profile.ImportRawFilename.IsValid())
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(rightButtons);
			if (Gutil::Button("Import", tVector2(buttonWidth, 0.0f)) || liveUpdated)
			{
				message.Clear();
				tList<tFrame> frames;
				ImportRaw::CreateResult result = ImportRaw::CreateFrames
				(
					frames, profile.ImportRawFilename, profile.GetImportRawPixelFormat(),
					profile.ImportRawWidth, profile.ImportRawHeight, profile.ImportRawDataOffset,
					importMipmaps, mipmapForceSameFrameSize, surfOrMipCount,
					profile.ImportRawPremultAlpha, profile.GetImportRawColourSpace()
				);
				if (result == ImportRaw::CreateResult::Success)
				{
					tString dstFilename = ImportRaw::ImportedDstFile;
					if (dstFilename.IsEmpty())
						dstFilename = ImportRaw::MakeImportedFilename(dstType, profile.ImportRawFilename);
					if (dstFilename.IsEmpty())
					{
						message = "No Destination Filename";
					}
					else
					{
						bool exists = ImportRaw::CreateImportedFile(frames, dstFilename);
						if (exists)
						{
							// We only set ImportedDstFile once we know it exists.
							ImportRaw::ImportedDstFile = dstFilename;
							bool found = SetCurrentImage(ImportRaw::ImportedDstFile, true);
							if (!found)
							{
								Image* newImg = new Image(ImportRaw::ImportedDstFile);
								Images.Append(newImg);
								ImagesLoadTimeSorted.Append(newImg);
								SortImages(profile.GetSortKey(), profile.SortAscending);
								SetCurrentImage(dstFilename);
							}
						}
						else
						{
							message = "File Write Failure";
						}
					}
				}
				else
				{
					switch (result)
					{
						case ImportRaw::CreateResult::UnsupportedFormat:	message = "Unsupported Format";	break;
						case ImportRaw::CreateResult::DataShortage:			message = "Data Shortage";		break;
						case ImportRaw::CreateResult::DecodeError:			message = "Decode Error";		break;
					}
				}
			}
		}

		if (ImGui::Button("Cancel", tVector2(buttonWidth, 0.0f)))
		{
			if (ImportRaw::ImportedDstFile.IsValid())
			{
				bool deleted = DeleteImageFile(ImportRaw::ImportedDstFile, false);
				if (deleted)
					ImportRaw::ImportedDstFile.Clear();
			}
			*popen = false;
			handledClose = true;
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(rightButtons);
		if (ImGui::Button("OK", tVector2(buttonWidth, 0.0f)))
		{
			ImportRaw::ImportedDstFile.Clear();
			*popen = false;
			handledClose = true;
		}
	}

	ImGui::End();
	return handledClose;
}


void Viewer::CloseCancelImportRawOverlay()
{
	if (ImportRaw::ImportedDstFile.IsValid())
	{
		bool deleted = DeleteImageFile(ImportRaw::ImportedDstFile, false);
		if (deleted)
			ImportRaw::ImportedDstFile.Clear();
	}
}


ImportRaw::CreateResult ImportRaw::CreateFrames
(
	tList<tFrame>& frames, const tString& rawFile, tImage::tPixelFormat rawFmt,
	int width, int height, int offset,
	bool mipmaps, bool mipmapForceSameFrameSize, int surfaceOrMipmapCount,
	bool undoAlphaPremult, tColourSpace space
)
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

	int numLevels = surfaceOrMipmapCount;
	int maxLevels = mipmaps ? tImage::tGetNumMipmapLevels(width, height) : 128;
	tMath::tiClamp(numLevels, 1, maxLevels);
	int dataNeeded = 0;
	for (int lev = 0; lev < numLevels; lev++)
	{
		int levW = mipmaps ? tImage::tGetMipmapDim(width, lev) : width;
		int levH = mipmaps ? tImage::tGetMipmapDim(height, lev) : height;
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

	int levW = width; int levH = height;
	for (int lev = 0; lev < numLevels; lev++)
	{
		int blocksW = tGetNumBlocks(blockW, levW);
		int blocksH = tGetNumBlocks(blockH, levH);
		int numBytes = blocksW * blocksH * bytesPerBlock;

		tPixel4b* pixelsLDR = nullptr;
		tPixel4f* pixelsHDR = nullptr;
		tImage::DecodeResult result = DecodePixelData
		(
			rawFmt, rawPixelData, numBytes, levW, levH,
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
			pixelsLDR = new tPixel4b[levW*levH];
			for (int p = 0; p < levW*levH; p++)
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

			for (int y = 0; y < levH; y++)
				for (int x = 0; x < levW; x++)
					frame->SetPixel(x, y, pixelsLDR[y*levW + x]);
			delete[] pixelsLDR;
		}
		else
		{
			frame->StealFrom(pixelsLDR, levW, levH);
		}

		// Possibly undo alpha premultiplication by multiplying by the colour channels
		// by the inverse of the alpha.
		if (undoAlphaPremult || (space == tColourSpace::lRGB))
		{
			for (int p = 0; p < frame->Width*frame->Height; p++)
			{
				tColour4b col = frame->Pixels[p];
				tColour4f colf(col);
				if (undoAlphaPremult)
				{
					float invAlpha = (colf.A > 0.0f) ? 1.0f/colf.A : 1.0f;
					colf.R *= invAlpha;
					colf.G *= invAlpha;
					colf.B *= invAlpha;
				}
				if (space == tColourSpace::lRGB)
				{
					colf.LinearToSRGB(tCompBit_RGB);
				}
				col.Set(colf);
				frame->Pixels[p] = col;
			}
		}
		frames.Append(frame);

		if (mipmaps)
			tImage::tGetNextMipmapLevelDims(levW, levH);
		rawPixelData += numBytes;
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
