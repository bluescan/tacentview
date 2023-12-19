---
title: Homepage
---

![Build](https://github.com/bluescan/tacentview/workflows/Build/badge.svg) [![latest](https://img.shields.io/github/v/release/bluescan/tacentview.svg)](https://github.com/bluescan/tacentview/releases) [![tacentview](https://snapcraft.io//tacentview/badge.svg)](https://snapcraft.io/tacentview)

## Introduction

_Tacent View_ is both fast and correct. Most of the image loading code is either custom or
based on official reference implementations. Once decoded, images are marshalled to GPU memory for
fast switching/rendering. The thumbnail viewer caches images/meta-data and can handle
thousands of files in a single directory at interactive speeds.

![Tacent View Levels](https://user-images.githubusercontent.com/19311462/216225745-0ea91c61-6b07-40fc-a1b7-bd327cdded4c.gif)

---
## Download

_Tacent View_ is distributed as:
1. Source Code
2. Portable Windows Zip
3. Linux Deb File
4. Snapcraft Snap

The source code, portable Windows zip, and deb files are available from the [releases GitHub page](https://github.com/bluescan/tacentview/releases). If running Linux you may also consider [installing the snap](https://snapcraft.io/tacentview). If you want to build it yourself, instructions are available at the [Tacent View GitHub page](https://github.com/bluescan/tacentview). The CMake build-system is used and can create projects (VS2022, make, nmake, ninja) for your desired compiler (MSVC, Clang, GCC). Build yourself if you have any security concerns. The buttons below link to pre-compiled versions of the latest stable release.

<ul class="downloads">
	<li><a href="https://github.com/bluescan/tacentview/releases/download/v1.0.41/tacentview_1.0.41.zip">Download <strong>WIN Zip</strong></a></li>
	<li><a href="https://github.com/bluescan/tacentview/releases/download/v1.0.41/tacentview_1.0-41.deb">Download <strong>DEB File</strong></a></li>
	<li><a href="https://snapcraft.io/tacentview">Snapcraft <strong>SNAP</strong></a></li>
</ul>

---
## Operation

There are two main modes in which _Tacent View_ may be used: **GUI** and **CLI**.

**GUI** mode is what you would expect -- Open an image file with tacentview.exe and a window will open displaying the image. If you install the _snap_ or _deb_ file, it will register with the OS to be able to open supported image formats (but will not change your defaults). On Windows you may optionally associate _Tacent View_ to a particular image type by selecting _Open with_ and then _Choose another app_.

**CLI** mode (Command-Line Interface) allows you to perform all operations that can be done in the **GUI** from the command-line. The same executable file is used for command-line operations, whether compiled for Linux or Windows. **CLI** mode is intended to allow batch processing and conversions of images from one format to another. It allows _Tacent View_ to be used in game pipelines and image processing workflows.

<ul class="downloads">
	<li><a href="https://bluescan.github.io/tacentview/gui.html">GUI Info <strong>GUI</strong></a></li>
	<li><a href="https://bluescan.github.io/tacentview/cli.html">CLI Info <strong>CLI</strong></a></li>
	<li><a href="https://github.com/bluescan/tacentview/wiki.html">Planning <strong>WIKI</strong></a></li>
</ul>

---
## Image Formats

Loads: _TGA PNG/APNG JPG GIF WEBP QOI DDS KTX/KTX2 ASTC/ASC PKM EXR HDR/RGB BMP ICO TIF/TIFF_

Saves: _TGA PNG/APNG JPG GIF WEBP QOI BMP TIF/TIFF_

Animates: _GIF WEBP APNG TIF/TIFF_

---
## Features

* Image quantization.
* High quality image rotations including lossless JPG transformations.
* Live rotation preview.
* Viewing meta-data stored inside images (EXIF data in JPG files).
* Compensating for meta-data orientation.
* Fast thumbnail viewer that supports sorting by file properties as well as meta-data.
* Slideshow mode at speeds anywhere between 60fps to multiple minutes per slide.
* Slideshow ordering based on any sort-criteria. e.g. Order by latitude or longitude, etc.
* Random slideshow shuffle with optional reshuffle at the end of every loop.
* Live thumbnail size adjustments.
* Cropping.
* Image Levels adjustments with histogram.
* Custom key-bindings per profile.
* Small, Normal, and Large UI widget/font selection.
* Fullscreen and basic modes for UX minimalists.
* Multiple zoom modes. Includes optional per-image zoom, downscale-only, and one-to-one.
* Flip operations.
* Canvas resizing with arbitrary or preset anchor.
* Image resizing using various filters / edge modes.
* Predefined or arbitrary aspect-ratios for resize operations.
* Folder bookmarks for storing favourite locations.  
* Colour channels.
* Alpha blending.
* Brightness adjustments.
* Contrast adjustments.
* Generating flipbook/contact-sheets.
* Frame extraction. Both mipmap, cubemap, and animation frames may be extracted.
* Creation of animated images from individual frames.
* High-quality GIF file generation.
* Scrubbing of all animated image formats (both forward and backwards).
* Adjusting frame durations for all animated formats.
* Legacy and modern DDS loading.
* All BC, ASTC, and ETC pixel formats for DDS, KTX, KTX2, ASTC, and PKM files.
* HDR image adjustments including exposure for all formats that encode HDR.
* Batch conversions between image types.
* Batch operations on arbitrary input images.
* Correct transparency and alpha-channel support.
* Transparent desktop mode.
* Full per-image undo/redo stack.
* Border detection/extraction.
* Multiple monitor support.
* Direct edit of pixel colour.
* Viewing of mipmaps individually or side-by-side.
* Cubemap view in T-layout or as individual sides.
* Tile-view for checking texture edge boundaries.
* Copy images to the clipboard.
* Paste images from the clipboard. Every paste generates a new file automatically.
* Drag image files directly into viewing area.

---
## Texture Pixel Formats

Some image formats are often used to store texture data that is easily decompressable by a GPU -- DDS, KTX, KTX2, ASTC, and PKM files are examples. Tacent View supports the following texture formats for these types of files:

| **Packed**      |                 |                 |                 |
|-----------------|-----------------|-----------------|-----------------|
| R8              | R8G8            | R8G8B8          | R8G8B8A8        |
| B8G8R8          | B8G8R8A8        | G3B5R5G3*       | G4B4A4R4*       |
| G3B5A1R5G2*     | A8L8            | A8              | L8              |
| R16f            | R16G16f         | R16G16B16A16f   | B4A4R4G4        |
| R32f            | R32G32f         | R32G32B32A32f   | B10G11R11uf     |
| E5B9G9R9uf      |                 |                 |                 |
|                 |                 |                 |                 |
| **Block (BC)**  |                 |                 |                 |
|-----------------|-----------------|-----------------|-----------------|
| BC1DXT1         | BC1DXT1A        | BC2DXT2DXT3     | BC3DXT4DXT5     |
| BC4ATI1         | BC5ATI2         | BC6S            | BC6U            |
| BC7             |                 |                 |                 |
|                 |                 |                 |                 |
| **Block (ETC)** |                 |                 |                 |
|-----------------|-----------------|-----------------|-----------------|
| ETC1            | ETC2RGB         | ETC2RGBA        | ETC2RGBA1       |
| EACR11          | EACR11S         | EACRG11         | EACRG11S        |
|                 |                 |                 |                 |
| **ASTC**        |                 |                 |                 |
|-----------------|-----------------|-----------------|-----------------|
| ASTC4X4         | ASTC5X4         | ASTC5X5         | ASTC6X5         |
| ASTC6X6         | ASTC8X5         | ASTC8X6         | ASTC8X8         |
| ASTC10X5        | ASTC10X6        | ASTC10X8        | ASTC10X10       |
| ASTC12X10       | ASTC12X12       |                 |                 |

\* These formats are named like every other format in the table -- based on how the bits appear on disk and in memory. Some APIs call these B5G6R5 (BGR565), B4G4R4A4 (BGRA4444), and B5G5R5A1 (BGRA5551) but this is inconsistent with their other format names like B8G8R8 which are correct. Even taking little-endianness into account the adjusted names would be G3R5B5G3, R4A4B4G4, and G2R5A1B5G3 which does not match the in-memory representation. On the bright side, the LE-adjusted names for the PVR spec do make sense: G3B5R5G3 -> RGB565, G4B4A4R4 -> ARGB4444, and G3B5A1R5G2 -> ARGB1555.
