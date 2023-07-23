---
title: Homepage
---

![Build](https://github.com/bluescan/tacentview/workflows/Build/badge.svg) [![latest](https://img.shields.io/github/v/release/bluescan/tacentview.svg)](https://github.com/bluescan/tacentview/releases) [![tacentview](https://snapcraft.io//tacentview/badge.svg)](https://snapcraft.io/tacentview)

![Tacent View Levels](https://user-images.githubusercontent.com/19311462/216225745-0ea91c61-6b07-40fc-a1b7-bd327cdded4c.gif)

## Introduction

_Tacent View_ is both fast and correct. Most of the image loading code is either custom or
based on official reference implementations. Once decoded, images are marshalled to GPU memory for
fast switching/rendering. The thumbnail viewer caches images and meta-data and is able to handle
thousands of files in a single directory at interactive speeds.

---
## Download

_Tacent View_ is distributed as:
1. **Source Code** that may be compiled directly.
2. A pre-compiled **portable application** for Windows (64-bit).
3. A Linux **deb** file.
4. A Snapcraft **snap**.

The source code, portable Windows zip, and deb files are available from the [releases GitHub page](https://github.com/bluescan/tacentview/releases). If running Linux you may also consider [installing the snap](https://snapcraft.io/tacentview). If you want to build it yourself, instructions are available at the [Tacent View GitHub page](https://github.com/bluescan/tacentview). The CMake build-system is used and can create projects (VS2022, make, nmake, ninja) for any desired compiler (MSVC, Clang, GCC).

<ul class="downloads">
	<li><a href="https://github.com/bluescan/tacentview/releases/download/v1.0.40/tacentview_1.0.40.zip">Download <strong>WIN App</strong></a></li>
	<li><a href="https://github.com/bluescan/tacentview/releases/download/v1.0.40/tacentview_1.0-40.deb">Download <strong>DEB File</strong></a></li>
	<li><a href="https://snapcraft.io/tacentview">Snapcraft <strong>SNAP</strong></a></li>
</ul>

---
## Operation

There are two main modes in which _Tacent View_ may be used: **GUI** and **CLI**.

**GUI** mode is what you would expect -- Open an image file with tacentview.exe and a window will open displaying the image. If you install the _snap_ or _deb_ file, it will register with the OS to be able to open supported image formats (but will not change your defaults). On Windows you may optionally associate _Tacent View_ to a particular image type by selecting _Open with_ and then _Choose another app_.

**CLI** mode (Command-Line Interface) allows you to perform all operations that can be done in the **GUI** from the command-line. The same executable file is used for command-line operations, whether compiled for Linux or Windows. **CLI** mode is intended to allow batch processing and conversions of images from one format to another. It allows _Tacent View_ to be easily used in game and image processing build pipelines.

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
* Slideshow ordering based on any sort-criteria. Eg. Order by latitude or longitude.
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
