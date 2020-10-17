// Settings.h
//
// Viewer settings stored as human-readable symbolic expressions.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <Foundation/tString.h>


namespace Viewer
{
	struct Settings
	{
		Settings()							{ Reset(); }
		int WindowX;
		int WindowY;
		int WindowW;
		int WindowH;
		bool ShowMenuBar;
		bool ShowNavBar;
		bool ShowImageDetails;
		bool ContentViewShow;
		float ThumbnailWidth;
		enum class SortKeyEnum
		{
			Alphabetical,
			FileModTime,
			FileSize,
			FileType
		};
		int SortKey;						// Matches SortKeyEnum values.
		bool SortAscending;					// Sort direction.

		int OverlayCorner;
		bool Tile;

		enum class BGStyle
		{
			None,
			Checkerboard,
			Black,
			Grey,
			White
		};
		int BackgroundStyle;
		bool BackgroundExtend;				// Extend background past image bounds.
		bool TransparentWorkArea;
		int ResampleFilter;					// Matches tImage::tPicture::tFilter.
		bool ConfirmDeletes;
		bool ConfirmFileOverwrites;
		bool SlideshowLooping;
		bool SlideshowProgressArc;
		double SlidehowFrameDuration;

		tString SaveSubFolder;
		int SaveFileType;
		bool SaveFileTargaRLE;
		int SaveFileJpegQuality;			// E [1,100]
		enum class SizeMode
		{
			Percent,
			SetWidthAndHeight,
			SetWidthRetainAspect,
			SetHeightRetainAspect
		};
		int SaveAllSizeMode;
		int MaxImageMemMB;					// Max image mem before unloading images.
		int MaxCacheFiles;					// Max number of cache files before removing oldest.
		bool StrictLoading;					// No attempt to display ill-formed images.
		bool AutoPropertyWindow;			// Auto display property editor window for supported file types.
		bool AutoPlayAnimatedImages;		// Automatically play animated gifs and WebPs.
		float MonitorGamma;					// Used when displaying HDR formats to do gamma correction.

		void Load(const tString& filename, int screenWidth, int screenHeight);
		bool Save(const tString& filename);
		void Reset(int screenWidth = 1280, int screenHeight = 720);
		void ResetBehaviourSettings();
		void ResetUISettings(int screenWidth, int screenHeight);
	};

	extern Settings Config;
}
