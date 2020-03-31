// tImageDDS.cpp
//
// This class knows how to load Direct Draw Surface (.dds) files. Saving is not implemented yet.
// It does zero processing of image data. It knows the details of the dds file format and loads the data into tLayers.
// Currently it does not compress or decompress the image data if it is compressed (DXTn), it simply keeps it in the
// same format as the source file. The layers may be 'stolen' from a tImageDDS so that excessive memcpys are avoided.
// After they are stolen the tImageDDS is invalid.
//
// Copyright (c) 2006, 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tString.h>
#include "Image/tImageDDS.h"
#define STRICT_DDS_HEADER_CHECKING
#define FourCC(ch0, ch1, ch2, ch3) (uint(uint8(ch0)) | (uint(uint8(ch1)) << 8) | (uint(uint8(ch2)) << 16) | (uint(uint8(ch3)) << 24))
namespace tImage
{


tImageDDS::tImageDDS() :
	Filename(),
	PixelFormat(tPixelFormat::Invalid),
	IsCubeMap(false),
	NumImages(0),
	NumMipmapLayers(0)
{
	tStd::tMemset(MipmapLayers, 0, sizeof(MipmapLayers));
}


tImageDDS::tImageDDS(const tString& ddsFile, bool reverseRowOrder) :
	Filename(ddsFile),
	PixelFormat(tPixelFormat::Invalid),
	IsCubeMap(false),
	NumImages(0),
	NumMipmapLayers(0)
{
	tStd::tMemset(MipmapLayers, 0, sizeof(MipmapLayers));
	Load(ddsFile, reverseRowOrder);
}


tImageDDS::tImageDDS(const uint8* ddsFileInMemory, int numBytes, bool reverseRowOrder) :
	Filename(),
	PixelFormat(tPixelFormat::Invalid),
	IsCubeMap(false),
	NumImages(0),
	NumMipmapLayers(0)
{
	tStd::tMemset(MipmapLayers, 0, sizeof(MipmapLayers));
	Load(ddsFileInMemory, numBytes, reverseRowOrder);
}


void tImageDDS::Clear()
{
	for (int image = 0; image < NumImages; image++)
	{
		for (int layer = 0; layer < NumMipmapLayers; layer++)
		{
			delete MipmapLayers[layer][image];
			MipmapLayers[layer][image] = 0;
		}
	}

	Filename.Clear();
	PixelFormat = tPixelFormat::Invalid;
	IsCubeMap = false;
	NumImages = 0;
	NumMipmapLayers = 0;
}


bool tImageDDS::IsOpaque() const
{
	switch (PixelFormat)
	{
		case tPixelFormat::R8G8B8A8:
		case tPixelFormat::B8G8R8A8:
		case tPixelFormat::BC1_DXT1BA:
		case tPixelFormat::BC2_DXT3:
		case tPixelFormat::BC3_DXT5:
		case tPixelFormat::G3B5A1R5G2:
		case tPixelFormat::G4B4A4R4:
			return false;

		default:
			return true;
	}

	return true;
}


bool tImageDDS::StealTextureLayers(tList<tLayer>& layers)
{
	if (!IsValid() || IsCubemap() || (NumImages <= 0))
		return false;

	for (int mip = 0; mip < NumMipmapLayers; mip++)
	{
		layers.Append(MipmapLayers[mip][0]);
		MipmapLayers[mip][0] = nullptr;
	}

	Clear();
	return true;
}


int tImageDDS::StealCubemapLayers(tList<tLayer> layerLists[tSurfIndex_NumSurfaces], uint32 sideFlags)
{
	if (!IsValid() || !IsCubemap() || !sideFlags)
		return 0;

	int sideCount = 0;
	for (int side = 0; side < tSurfIndex_NumSurfaces; side++)
	{
		uint32 sideFlag = 1 << side;
		if (!(sideFlag & sideFlags))
			continue;

		tList<tLayer>& layers = layerLists[side];
		for (int mip = 0; mip < NumMipmapLayers; mip++)
		{
			layers.Append( MipmapLayers[mip][side] );
			MipmapLayers[mip][side] = nullptr;
		}
		sideCount++;
	}

	Clear();
	return sideCount;
}


enum tDDSPixelFormatFlag
{
	// May be used in the DDSPixelFormat struct to indicate alphas present for RGB formats.
	tDDSPixelFormatFlag_Alpha	= 0x00000001,

	// A DDS file may contain this type of data (pixel format). eg. DXT1 is a fourCC format.
	tDDSPixelFormatFlag_FourCC	= 0x00000004,

	// A DDS file may contain this type of data (pixel format). eg. A8R8G8B8
	tDDSPixelFormatFlag_RGB		= 0x00000040
};


#pragma pack(push, 4)
struct tDDSPixelFormat
{
	// Must be 32.
	uint32 Size;

	// See tDDSPixelFormatFlag. Flags to indicate valid fields. Uncompressed formats will usually use
	// tDDSPixelFormatFlag_RGB to indicate an RGB format, while compressed formats will use tDDSPixelFormatFlag_FourCC
	// with a four-character code.
	uint32 Flags;

	// "DXT1", "DXT3", and "DXT5" are examples. m_flags should have DDSPixelFormatFlag_FourCC.
	uint32 FourCC;

	// Valid if flags has DDSPixelFormatFlag_RGB. For RGB formats this is the total number of bits per pixel. This
	// value is usually 16, 24, or 32. For A8R8G8B8, this value would be 32.
	uint32 RGBBitCount;

	// For RGB formats these three fields contain the masks for the red, green, and blue channels. For A8R8G8B8 these
	// values would be 0x00FF0000, 0x0000FF00, and 0x000000FF respectively.
	uint32 MaskRed;
	uint32 MaskGreen;
	uint32 MaskBlue;

	// If the flags have DDSPixelFormatFlag_Alpha set, this is valid and contains tha alpha mask. Eg. For A8R8G8B8 this
	// value would be 0xFF000000.
	uint32 MaskAlpha;
};
#pragma pack(pop)


enum tDDSCapsBasic
{
	tDDSCapsBasic_Complex		= 0x00000008,
	tDDSCapsBasic_Texture		= 0x00001000,
	tDDSCapsBasic_Mipmap		= 0x00400000
};


enum tDDSCapsExtra
{
	tDDSCapsExtra_CubeMap		= 0x00000200,
	tDDSCapsExtra_CubeMapPosX	= 0x00000400,
	tDDSCapsExtra_CubeMapNegX	= 0x00000800,
	tDDSCapsExtra_CubeMapPosY	= 0x00001000,
	tDDSCapsExtra_CubeMapNegY	= 0x00002000,
	tDDSCapsExtra_CubeMapPosZ	= 0x00004000,
	tDDSCapsExtra_CubeMapNegZ	= 0x00008000,
	tDDSCapsExtra_Volume		= 0x00200000
};


#pragma pack(push, 4)
struct tDDSCapabilities
{
	// DDS files should always include tDDSCapsBasic_Texture. If the file contains mipmaps tDDSCapsBasic_Mipmap should
	// be set. For any dds file with more than one main surface, such as a mipmap, cubic environment map, or volume
	// texture, DDSCapsBasic_Complex should also be set.
	uint32 FlagsCapsBasic;

	// For cubic environment maps tDDSCapsExtra_CubeMap should be included as well as one or more faces of the map
	// (tDDSCapsExtra_CubeMapPosX, etc). For volume textures tDDSCapsExtra_Volume should be included.
	uint32 FlagsCapsExtra;
	uint32 Unused[2];
};
#pragma pack(pop)


enum tDDSFlag
{
	tDDSFlag_Caps				= 0x00000001,	// Always included.
	tDDSFlag_Height				= 0x00000002,	// Always included. Height of largest image if mipmaps included.
	tDDSFlag_Width				= 0x00000004,	// Always included. Width of largest image if mipmaps included.
	tDDSFlag_Pitch				= 0x00000008,
	tDDSFlag_PixelFormat		= 0x00001000,	// Always included.
	tDDSFlag_MipmapCount		= 0x00020000,
	tDDSFlag_LinearSize			= 0x00080000,
	tDDSFlag_Depth				= 0x00800000
};


enum tD3DFORMAT
{
	tD3DFMT_UNKNOWN				=  0,

	tD3DFMT_R8G8B8				= 20,
	tD3DFMT_A8R8G8B8			= 21,
	tD3DFMT_X8R8G8B8			= 22,
	tD3DFMT_R5G6B5				= 23,
	tD3DFMT_X1R5G5B5			= 24,
	tD3DFMT_A1R5G5B5			= 25,
	tD3DFMT_A4R4G4B4			= 26,
	tD3DFMT_R3G3B2				= 27,
	tD3DFMT_A8					= 28,
	tD3DFMT_A8R3G3B2			= 29,
	tD3DFMT_X4R4G4B4			= 30,
	tD3DFMT_A2B10G10R10			= 31,
	tD3DFMT_A8B8G8R8			= 32,
	tD3DFMT_X8B8G8R8			= 33,
	tD3DFMT_G16R16				= 34,
	tD3DFMT_A2R10G10B10			= 35,
	tD3DFMT_A16B16G16R16		= 36,

	tD3DFMT_A8P8				= 40,
	tD3DFMT_P8					= 41,

	tD3DFMT_L8					= 50,
	tD3DFMT_A8L8				= 51,
	tD3DFMT_A4L4				= 52,

	tD3DFMT_V8U8				= 60,
	tD3DFMT_L6V5U5				= 61,
	tD3DFMT_X8L8V8U8			= 62,
	tD3DFMT_Q8W8V8U8			= 63,
	tD3DFMT_V16U16				= 64,
	tD3DFMT_A2W10V10U10			= 67,

	tD3DFMT_UYVY				= FourCC('U', 'Y', 'V', 'Y'),
	tD3DFMT_R8G8_B8G8			= FourCC('R', 'G', 'B', 'G'),
	tD3DFMT_YUY2				= FourCC('Y', 'U', 'Y', '2'),
	tD3DFMT_G8R8_G8B8			= FourCC('G', 'R', 'G', 'B'),
	tD3DFMT_DXT1				= FourCC('D', 'X', 'T', '1'),
	tD3DFMT_DXT2				= FourCC('D', 'X', 'T', '2'),
	tD3DFMT_DXT3				= FourCC('D', 'X', 'T', '3'),
	tD3DFMT_DXT4				= FourCC('D', 'X', 'T', '4'),
	tD3DFMT_DXT5				= FourCC('D', 'X', 'T', '5'),

	tD3DFMT_D16_LOCKABLE		= 70,
	tD3DFMT_D32					= 71,
	tD3DFMT_D15S1				= 73,
	tD3DFMT_D24S8				= 75,
	tD3DFMT_D24X8				= 77,
	tD3DFMT_D24X4S4				= 79,
	tD3DFMT_D16					= 80,

	tD3DFMT_D32F_LOCKABLE		= 82,
	tD3DFMT_D24FS8				= 83,

	tD3DFMT_D32_LOCKABLE		= 84,
	tD3DFMT_S8_LOCKABLE			= 85,

	tD3DFMT_L16					= 81,

	tD3DFMT_VERTEXDATA			= 100,
	tD3DFMT_INDEX16				= 101,
	tD3DFMT_INDEX32				= 102,

	tD3DFMT_Q16W16V16U16		= 110,

	tD3DFMT_MULTI2_ARGB8		= FourCC('M','E','T','1'),

	tD3DFMT_R16F				= 111,
	tD3DFMT_G16R16F				= 112,
	tD3DFMT_A16B16G16R16F		= 113,

	tD3DFMT_R32F				= 114,
	tD3DFMT_G32R32F				= 115,
	tD3DFMT_A32B32G32R32F		= 116,

	tD3DFMT_CxV8U8				= 117,

	tD3DFMT_FORCE_DWORD			= 0x7fffffff
};


// Default packing is 8 bytes but the header is 128 bytes (mult of 4), so we make it all work here.
#pragma pack(push, 4)
struct tDDSHeader
{
	uint32 Size;								// Must be set to 124.
	uint32 Flags;								// See tDDSFlags.
	uint32 Height;								// Height of main image.
	uint32 Width;								// Width of main image.

	// For uncompressed formats, this is the number of bytes per scan line (32-bit aligned) for the main image. dwFlags
	// should include DDSD_PITCH in this case. For compressed formats, this is the total number of bytes for the main
	// image. m_flags should have tDDSFlag_LinearSize in this case.
	uint32 PitchLinearSize;
	uint32 Depth;								// For volume textures. tDDSFlag_Depth is set for this to be valid.
	uint32 MipmapCount;							// Valid if tDDSFlag_MipmapCount set. @todo Count includes main image?
	uint32 UnusedA[11];
	tDDSPixelFormat PixelFormat;				// 32 Bytes.
	tDDSCapabilities Capabilities;				// 16 Bytes.
	uint32 UnusedB;
};
#pragma pack(pop)


// These DXT blocks are needed so that the tImageDDS class can re-order the rows by messing with each block's lookup
// table and alpha tables. This is because DDS files have the rows of their textures upside down (texture origin in
// OpenGL is lower left, while in DirectX it is upper left). See: http://en.wikipedia.org/wiki/S3_Texture_Compression
#pragma pack(push, 1)


// This block is used for both DXT1 and DXT1 with binary alpha. It's also used as the colour information block in the
// DXT 2, 3, 4 and 5 formats. Size is 64 bits.
struct tDXT1Block
{
	uint16 Colour0;								// R5G6B5
	uint16 Colour1;								// R5G6B5
	uint8 LookupTableRows[4];
};


// This one is the same for DXT2 and 3, although we don't support 2 (premultiplied alpha). Size is 128 bits.
struct tDXT3Block
{
	uint16 AlphaTableRows[4];					// Each alpha is 4 bits.
	tDXT1Block ColourBlock;
};


// This one is the same for DXT4 and 5, although we don't support 4 (premultiplied alpha). Size is 128 bits.
struct tDXT5Block
{
	uint8 Alpha0;
	uint8 Alpha1;
	uint8 AlphaTable[6];						// Each of the 4x4 pixel entries is 3 bits.
	tDXT1Block ColourBlock;

	// These accessors are needed because of the unusual alignment of the 3bit alpha indexes. They each return or set a
	// value in [0, 2^12) which represents a single row. The row variable should be in [0, 3]
	uint16 GetAlphaRow(int row)
	{
		tAssert(row < 4);
		switch (row)
		{
			case 1:
				return (AlphaTable[2] << 4) | (0x0F & (AlphaTable[1] >> 4));

			case 0:
				return ((AlphaTable[1] & 0x0F) << 8) | AlphaTable[0];

			case 3:
				return (AlphaTable[5] << 4) | (0x0F & (AlphaTable[4] >> 4));

			case 2:
				return ((AlphaTable[4] & 0x0F) << 8) | AlphaTable[3];
		}
		return 0;
	}

	void SetAlphaRow(int row, uint16 val)
	{
		tAssert(row < 4);
		tAssert(val < 4096);
		switch (row)
		{
			case 1:
				AlphaTable[2] = val >> 4;
				AlphaTable[1] = (AlphaTable[1] & 0x0F) | ((val & 0x000F) << 4);
				break;

			case 0:
				AlphaTable[1] = (AlphaTable[1] & 0xF0) | (val >> 8);
				AlphaTable[0] = val & 0x00FF;
				break;

			case 3:
				AlphaTable[5] = val >> 4;
				AlphaTable[4] = (AlphaTable[4] & 0x0F) | ((val & 0x000F) << 4);
				break;

			case 2:
				AlphaTable[4] = (AlphaTable[4] & 0xF0) | (val >> 8);
				AlphaTable[3] = val & 0x00FF;
				break;
		}
	}
};
#pragma pack(pop)


void tImageDDS::Load(const tString& ddsFile, bool reverseRowOrder)
{
	Clear();
	if (tSystem::tGetFileType(ddsFile) != tSystem::tFileType::DDS)
		throw tDDSError(tDDSError::tCode::IncorrectExtension, tSystem::tGetFileName(ddsFile));

	if (!tSystem::tFileExists(ddsFile))
		throw tDDSError(tDDSError::tCode::FileNonexistent, tSystem::tGetFileName(ddsFile));

	int ddsSizeBytes;
	uint8* ddsData = (uint8*)tSystem::tLoadFile(ddsFile, 0, &ddsSizeBytes);
	LoadFromMemory(ddsData, ddsSizeBytes, reverseRowOrder);

	delete[] ddsData;
}


void tImageDDS::Load(const uint8* ddsFileInMemory, int ddsSizeBytes, bool reverseRowOrder)
{
	Clear();
	LoadFromMemory(ddsFileInMemory, ddsSizeBytes, reverseRowOrder);
}


void tImageDDS::LoadFromMemory(const uint8* ddsData, int ddsSizeBytes, bool reverseRowOrder)
{
	tString baseName = tSystem::tGetFileName(Filename);

	// This will deal with zero-sized files properly as well.
	if (ddsSizeBytes < int(sizeof(tDDSHeader)+4))
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::IncorrectFileSize, baseName);
	}

	const uint8* ddsCurr = ddsData;
	uint32& magic = *((uint32*)ddsCurr); ddsCurr += sizeof(uint32);

	if (magic != ' SDD')
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::Magic);
	}

	tDDSHeader& header = *((tDDSHeader*)ddsCurr);  ddsCurr += sizeof(header);
	tAssert(sizeof(tDDSHeader) == 124);
	const uint8* pixelData = ddsCurr;

	if (header.Size != 124)
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::IncorrectHeaderSize, baseName);
	}

	uint32 flags = header.Flags;
	int mainWidth = header.Width;						// Main image.
	int mainHeight = header.Height;						// Main image.

	if (!tMath::tIsPower2(mainWidth) || !tMath::tIsPower2(mainHeight))
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::LoaderSupportsPowerOfTwoDimsOnly, baseName);
	}

	// It seems ATI tools like GenCubeMap don't set the correct bits.
	#ifdef STRICT_DDS_HEADER_CHECKING
	int pitch = 0;										// Num bytes per line on main image (uncompressed images only).
	int linearSize = 0;									// Num bytes total main image (compressed images only).

	if (flags & tDDSFlag_Pitch)
		pitch = header.PitchLinearSize;

	if (flags & tDDSFlag_LinearSize)
		linearSize = header.PitchLinearSize;

	// Linear size xor pitch must be specified.
	if ((!linearSize && !pitch) || (linearSize && pitch))
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::PitchOrLinearSize, baseName);
	}
	#endif

	// Volume textures are not supported.
	if (flags & tDDSFlag_Depth)
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::VolumeTexturesNotSupported, baseName);
	}

	// Determine the expected number of layers by looking at the mipmap count if it is supplied. We assume a single layer
	// if it's not specified.
	NumMipmapLayers = 1;
	bool hasMipmaps = (header.Capabilities.FlagsCapsBasic & tDDSCapsBasic_Mipmap) ? true : false;
	if ((flags & tDDSFlag_MipmapCount) && hasMipmaps)
		NumMipmapLayers = header.MipmapCount;

	if (NumMipmapLayers > MaxMipmapLayers)
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::MaxNumMipmapLevelsExceeded);
	}

	// Determine if this is a cubemap dds with 6 images. No need to check which images are present since they are
	// required to be all there by the dds standard. All tools these days seem to write them all. If there are complaints
	// when using legacy files we can fix this.
	if (header.Capabilities.FlagsCapsExtra & tDDSCapsExtra_CubeMap)
	{
		IsCubeMap = true;
		NumImages = 6;
	}
	else
	{
		IsCubeMap = false;
		NumImages = 1;
	}

	// Determine if we support the pixel format and which one it is.
	PixelFormat = tPixelFormat::Invalid;
	tDDSPixelFormat& format = header.PixelFormat;

	if (format.Size != 32)
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::IncorrectPixelFormatSize, baseName);
	}

	// Has alpha should be true if the pixel format is uncompressed (RGB) and there is an alpha channel.
	bool rgbHasAlpha = (format.Flags & tDDSPixelFormatFlag_Alpha) ? true : false;
	bool rgbFormat = (format.Flags & tDDSPixelFormatFlag_RGB) ? true : false;
	bool fourCCFormat = (format.Flags & tDDSPixelFormatFlag_FourCC) ? true : false;

	if ((!rgbFormat && !fourCCFormat) || (rgbFormat && fourCCFormat))
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::InconsistentPixelFormat, baseName);
	}

	if (fourCCFormat)
	{
		switch (format.FourCC)
		{
			case FourCC('D','X','T','1'):
				// Note that during inspecition of the individual layer data, the DXT1 pixel format might be modified
				// to DXT1BA (binary alpha).
				PixelFormat = tPixelFormat::BC1_DXT1;
				break;

			case FourCC('D','X','T','3'):
				PixelFormat = tPixelFormat::BC2_DXT3;
				break;

			case FourCC('D','X','T','5'):
				PixelFormat = tPixelFormat::BC3_DXT5;
				break;

			case tD3DFMT_R32F:
				PixelFormat = tPixelFormat::R32F;
				break;

			case tD3DFMT_G32R32F:
				PixelFormat = tPixelFormat::G32R32F;
				break;

			case tD3DFMT_A32B32G32R32F:
				PixelFormat = tPixelFormat::A32B32G32R32F;
				break;

			case FourCC('D','X','1','0'):
			default:
				delete[] ddsData;
				throw tDDSError(tDDSError::tCode::UnsupportedFourCCPixelFormat, baseName);
		}
	}

	// It must be an RGB format.
	else
	{
		// Remember this is a little endian machine, so the masks are lying. Eg. 0xFF0000 in memory is 00 00 FF, so the red
		// is last.
		switch (format.RGBBitCount)
		{
			case 16:
				// Supports G3B5A1R5G2, G4B4A4R4, and G3B5R5G3.
				if
				(
					rgbHasAlpha &&
					(format.MaskAlpha	== 0x8000) &&
					(format.MaskRed		== 0x7C00) &&
					(format.MaskGreen	== 0x03E0) &&
					(format.MaskBlue	== 0x001F)
				)
				{
					PixelFormat = tPixelFormat::G3B5A1R5G2;
				}

				else if
				(
					rgbHasAlpha &&
					(format.MaskAlpha	== 0xF000) &&
					(format.MaskRed		== 0x0F00) &&
					(format.MaskGreen	== 0x00F0) &&
					(format.MaskBlue	== 0x000F)
				)
				{
					PixelFormat = tPixelFormat::G4B4A4R4;
				}

				else if
				(
					!rgbHasAlpha &&
					(format.MaskRed		== 0xF800) &&
					(format.MaskGreen	== 0x07E0) &&
					(format.MaskBlue	== 0x001F)
				)
				{
					PixelFormat = tPixelFormat::G3B5R5G3;
				}

				else
				{
					delete[] ddsData;
					throw tDDSError(tDDSError::tCode::UnsupportedRGBPixelFormat, baseName);
				}

				break;

			case 24:
				// Supports B8G8R8.
				if
				(
					!rgbHasAlpha &&
					(format.MaskRed		== 0xFF0000) &&
					(format.MaskGreen	== 0x00FF00) &&
					(format.MaskBlue	== 0x0000FF)
				)
				{
					PixelFormat = tPixelFormat::B8G8R8;
				}

				else
				{
					delete[] ddsData;
					throw tDDSError(tDDSError::tCode::UnsupportedRGBPixelFormat, baseName);
				}

				break;

			case 32:
				// Supports B8G8R8A8. This is a little endian machine so the masks are lying. 0xFF000000 in memory is
				// 00 00 00 FF with alpha last.
				if
				(
					rgbHasAlpha &&
					(format.MaskAlpha	== 0xFF000000) &&
					(format.MaskRed		== 0x00FF0000) &&
					(format.MaskGreen	== 0x0000FF00) &&
					(format.MaskBlue	== 0x000000FF)
				)
				{
					PixelFormat = tPixelFormat::B8G8R8A8;
				}
				else
				{
					delete[] ddsData;
					throw tDDSError(tDDSError::tCode::UnsupportedRGBPixelFormat, baseName);
				}
				break;

			default:
				delete[] ddsData;
				throw tDDSError(tDDSError::tCode::UnsupportedRGBPixelFormat, baseName);
		}
	}

	// @todo We do not yet support these formats.
	if ((PixelFormat == tPixelFormat::R32F) || (PixelFormat == tPixelFormat::G32R32F) || (PixelFormat == tPixelFormat::A32B32G32R32F))
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::UnsupportedFourCCPixelFormat, baseName);
	}

	tAssert(PixelFormat != tPixelFormat::Invalid);
	if (!rgbFormat && ((mainWidth%4) || (mainHeight%4)))
	{
		delete[] ddsData;
		throw tDDSError(tDDSError::tCode::UnsupportedDXTDimensions, baseName);
	}

	for (int image = 0; image < NumImages; image++)
	{
		int width = mainWidth;
		int height = mainHeight;

		for (int layer = 0; layer < NumMipmapLayers; layer++)
		{
			int numBytes;
			if (rgbFormat)
			{
				numBytes = width*height*format.RGBBitCount/8;

				// Deal with the reverseRowOrder for these RGB formats as well.
				if (reverseRowOrder)
				{
					uint8* reversedPixelData = new uint8[numBytes];
					uint8* dstData = reversedPixelData;

					// We only support pixel formats that contain a whole number of bytes per pixel. That will cover
					// all reasonable formats.
					int bytesPerPixel = format.RGBBitCount/8;

					for (int row = height-1; row >= 0; row--)
					{
						for (int col = 0; col < width; col++)
						{
							const uint8* srcData = pixelData + row*bytesPerPixel*width + col*bytesPerPixel;
							for (int byte = 0; byte < bytesPerPixel; byte++, dstData++, srcData++)
								*dstData = *srcData;
						}
					}

					// We can simply get the layer to steal the memory (the last true arg).
					MipmapLayers[layer][image] = new tLayer(PixelFormat, width, height, reversedPixelData, true);
				}
				else
				{
					MipmapLayers[layer][image] = new tLayer(PixelFormat, width, height, (uint8*)pixelData);
				}
				tAssert(MipmapLayers[layer][image]->GetDataSize() == numBytes);
			}
			else
			{
				// Otherwise it's a FourCC DXTn format.  Each block encodes a 4x4 square of pixels.  DXT2,3,4,5 use 128
				// bits per block.  DXT1 and DXT1BA use 64bits per block.
				int dxtBlockSize = 16;
				if ((PixelFormat == tPixelFormat::BC1_DXT1BA) || (PixelFormat == tPixelFormat::BC1_DXT1))
					dxtBlockSize = 8;

				int numBlocks = tMath::tMax(1, width/4) * tMath::tMax(1, height/4);
				numBytes = numBlocks * dxtBlockSize;

				// Here's where we possibly modify the opaque DXT1 texture to be DXT1BA if there are blocks with binary
				// transparency. We only bother checking the main layer. If it's opaque we assume all the others are too.
				if ((layer == 0) && (PixelFormat == tPixelFormat::BC1_DXT1) && DoDXT1BlocksHaveBinaryAlpha((tDXT1Block*)pixelData, numBlocks))
					PixelFormat = tPixelFormat::BC1_DXT1BA;

				// DDS files store textures upside down. In the OpenGL RH coord system, the lower left of the texture
				// is the origin and consecutive rows go up. For this reason we need to read each row of blocks from
				// the top to the bottom row. We also need to flip the rows within the 4x4 block by flipping the lookup
				// tables. This should be fairly fast as there is no encoding or encoding going on. Width and height
				// will go down to 1x1, which will still use a 4x4 DXT pixel-block.
				if (reverseRowOrder)
				{
					int heightBlocks = height / 4;
					if (height % 4)
						heightBlocks++;

					int widthBlocks = width / 4;
					if (width % 4)
						widthBlocks++;

					uint8* reversedPixelData = new uint8[numBytes];
					uint8* dstData = reversedPixelData;

					for (int row = heightBlocks-1; row >= 0; row--)
					{
						for (int col = 0; col < widthBlocks; col++)
						{
							const uint8* srcData = pixelData + row*dxtBlockSize*widthBlocks + col*dxtBlockSize;
							for (int byte = 0; byte < dxtBlockSize; byte++, dstData++, srcData++)
								*dstData = *srcData;
						}
					}

					// Now we flip the inter-block rows by messing with the block's lookup-table.  We need to handle all
					// three types of blocks: 1) DXT1, DXT1BA  2) DXT2, DXT3  3) DXT4, DXT5
					int totalBlocks = widthBlocks * heightBlocks;

					switch (PixelFormat)
					{
						case tPixelFormat::BC1_DXT1BA:
						case tPixelFormat::BC1_DXT1:
						{
							tDXT1Block* block = (tDXT1Block*)reversedPixelData;
							for (int b = 0; b < totalBlocks; b++, block++)
							{
								// Reorder each row's colour indexes.
								tStd::tSwap(block->LookupTableRows[0], block->LookupTableRows[3]);
								tStd::tSwap(block->LookupTableRows[1], block->LookupTableRows[2]);
							}
							break;
						}

						case tPixelFormat::BC2_DXT3:
						{
							tDXT3Block* block = (tDXT3Block*)reversedPixelData;
							for (int b = 0; b < totalBlocks; b++, block++)
							{
								// Reorder the explicit alphas AND the colour indexes.
								tStd::tSwap(block->AlphaTableRows[0], block->AlphaTableRows[3]);
								tStd::tSwap(block->AlphaTableRows[1], block->AlphaTableRows[2]);
								tStd::tSwap(block->ColourBlock.LookupTableRows[0], block->ColourBlock.LookupTableRows[3]);
								tStd::tSwap(block->ColourBlock.LookupTableRows[1], block->ColourBlock.LookupTableRows[2]);
							}
							break;
						}

						case tPixelFormat::BC3_DXT5:
						{
							tDXT5Block* block = (tDXT5Block*)reversedPixelData;
							for (int b = 0; b < totalBlocks; b++, block++)
							{
								// Reorder the alpha indexes AND the colour indexes.
								uint16 orig0 = block->GetAlphaRow(0);
								block->SetAlphaRow(0, block->GetAlphaRow(3));
								block->SetAlphaRow(3, orig0);

								uint16 orig1 = block->GetAlphaRow(1);
								block->SetAlphaRow(1, block->GetAlphaRow(2));
								block->SetAlphaRow(2, orig1);

								tStd::tSwap(block->ColourBlock.LookupTableRows[0], block->ColourBlock.LookupTableRows[3]);
								tStd::tSwap(block->ColourBlock.LookupTableRows[1], block->ColourBlock.LookupTableRows[2]);
							}
							break;
						}

						case tPixelFormat::R32F:
						case tPixelFormat::G32R32F:
						case tPixelFormat::A32B32G32R32F:
						{
							delete[] ddsData;
							throw tDDSError(tDDSError::tCode::UnsuportedFloatingPointPixelFormat, baseName);
						}

						default:
						{
							delete[] ddsData;
							throw tDDSError(tDDSError::tCode::UnsupportedFourCCPixelFormat, baseName);
						}
					}

					// Finally we can append a layer with the massaged dxt data. We can simply get the layer to steal the memory (the
					// last true arg).
					MipmapLayers[layer][image] = new tLayer(PixelFormat, width, height, reversedPixelData, true);
				}
				else
				{
					// If reverseRowOrder is false we want the data to go straight in so we use the pixelData directly.
					MipmapLayers[layer][image] = new tLayer(PixelFormat, width, height, (uint8*)pixelData);
				}
				tAssert(MipmapLayers[layer][image]->GetDataSize() == numBytes);
			}

			pixelData += numBytes;

			// @todo Does this assume power-of-2 dimensions?  Can we avoid this assumption in this low-level class?
			width /= 2;
			if (width < 1)
				width = 1;

			height /= 2;
			if (height < 1)
				height = 1;
		}
	}
}


bool tImageDDS::DoDXT1BlocksHaveBinaryAlpha(tDXT1Block* block, int numBlocks)
{
	// The only way to check if the DXT1 format has alpha is by checking each block individually. If the block uses
	// alpha, the min and max colours are ordered in a particular order.
	for (int b = 0; b < numBlocks; b++)
	{
		if (block->Colour0 <= block->Colour1)
		{
			// OK, well, that's annoying. It seems that at least the nVidia DXT compressor can generate an opaque DXT1
			// block with the colours in the order for a transparent one. This forces us to check all the indexes to
			// see if the alpha index (11 in binary) is used -- if not then it's still an opaque block.
			for (int row = 0; row < 4; row++)
			{
				uint8 bits = block->LookupTableRows[row];
				if
				(
					((bits & 0x03) == 0x03) ||
					((bits & 0x0C) == 0x0C) ||
					((bits & 0x30) == 0x30) ||
					((bits & 0xC0) == 0xC0)
				)
					return true;
			}
		}

		block++;
	}

	return false;
}


}


const char* tDDSError::CodeStrings[int(tCode::NumCodes)] =
{
	"Unknown.",
	"File doesn't exist.",
	"Incorrect DDS extension.",
	"Filesize incorrect.",
	"Magic FourCC Incorrect.",
	"Incorrect DDS header size.",
	"One of Pitch or LinearSize must be specified.",
	"Volume textures unsupported.",
	"Pixel format size incorrect.",
	"Pixel format must be either an RGB format or a FourCC format.",
	"Unsupported FourCC pixel format. Supported FourCC formats include DXT1, DXT3, DXT5.",
	"Unsupported RGB pixel format. Supported formats include A1R5G5B5, A4R4G4B4, R5G6B5, R8G8B8, and A8R8G8B8.",
	"Incorrect DXT pixel data size.",
	"DXT Texture dimensions must be divisible by 4.",
	"Current DDS loader only supports power-of-2 dimensions.",
	"Maximum number of mipmap levels exceeded.",
	"Floating point pixel formats not supported yet."
};
