---
title: Homepage
---

![Build](https://github.com/bluescan/tacentview/workflows/Build/badge.svg) [![latest](https://img.shields.io/github/v/release/bluescan/tacentview.svg)](https://github.com/bluescan/tacentview/releases) [![tacentview](https://snapcraft.io//tacentview/badge.svg)](https://snapcraft.io/tacentview)

## Introduction

_Tacent View_ is both fast and correct. Most of the image loading code is either custom or
based on official reference implementations. Once decoded, images are marshalled to GPU memory for
fast switching/rendering. The thumbnail viewer caches images and meta-data and is able to handle
thousands of files in a single directory at interactive speeds.

_Tacent View_ is distributed as:
1. **Source Code** that may be compiled directly.
2. A pre-compiled **portable application** for Windows (64-bit).
3. A Linux **deb** file.
4. A Snapcraft **snap**.

The source code, portable Windows zip, and deb files are available from the [releases GitHub page](https://github.com/bluescan/tacentview/releases). If running Linux you may also consider [installing the snap](https://snapcraft.io/tacentview). If you want to build it yourself, instructions are available at the [Tacent View GitHub page](https://github.com/bluescan/tacentview). The CMake build-system is used and can create projects (VS2022, make, nmake, ninja) for any desired compiler (MSVC, Clang, GCC).

There are two main modes in which _Tacent View_ may be used: **GUI** and **CLI**.

#### GUI Mode
The primary **GUI** mode is what you would expect. Open an image file with tacentview.exe and a window will open displaying the image.

#### CLI Mode
The secondary **CLI** mode allows you to perform all operations that can be done in the **GUI** from the command-line. The same executable file is used for command-line operations, whether compiled for Linux or Windows. **CLI** mode is intended to allow batch processing and conversions of images from one format to another. It allows _Tacent View_ to be easily used in game and image processing build pipelines.

---

## Supported Image Types

_Tacent View_ can load:
* TGA
* PNG
* JPG
* GIF
* WEBP
* QOI
* DDS
* KTX
* KTX2
* ASTC
* PKM
* EXR
* HDR
* APNG
* BMP
* ICO
* TIFF

_Tacent View_ can save:
* TGA
* PNG
* JPG
* GIF
* WEBP
* QOI
* APNG
* BMP
* TIFF

_Tacent View_ supports saving animations/multiple-pages for:
* GIF
* WEBP
* APNG
* TIFF

