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
1. **Source Code** that may be compiled directly. The CMake build-system is used and can create projects (VS2022, make, nmake, ninja) for any desired compiler (MSVC, Clang, GCC).
2. A pre-compiled **portable application** for Windows (64-bit).
3. A Linux **deb** file.
4. A Snapcraft **snap**.

The latest stable release is available on [Github](https://github.com/bluescan/tacentview/releases). The source code, portable Windows zip, and deb files are available there. If running Linux you may also consider [installing the snap](https://snapcraft.io/tacentview).
