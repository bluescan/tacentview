________________________________________________________________________________________________________________________
Image Module

This module contains source files related to processing images of various formats, processing them, converting them to
textures, and saving them.

________________________________________________________________________________________________________________________
Overview

Everything is inside the tImage namespace. The main classes are as follows:

tPixel:
A raw uncompressed pixel. 32 bits RGBA.

tPicture:
This class represents a simple one layer image. It is a collection of raw uncompressed 32-bit tPixels. It can load
various formats from disk such as jpg, tga, png, etc. It intentionally _cannot_ load a dds file. More on that later.
This class can load tChunk files and tga files with a native implementation. It uses CxImage for everything else.
Saving functionality is restricted to saving tga files (targa files are not lossless when RLE compressed) and tChunk
files. Image manipulation (excluding compression) happens in a tPicture, so there are methods to crop, scale, etc.

tLayer:
This is just a data container for texture pixel data. It is used by the tTexture class to store image data. It contains
data that may be in a variety of hardware-ready formats, like dxt5. It is primarily used to store the multiple mipmap
layers of a texture. Main members are width, height, pixel format, and a function to compute data size based on those
three variables. It knows how to save and load itself in tChunk format.

tTexture:
A tTexture is a 'hardware-ready' format. tTextures contain functionality for creating mipmap layers in a variety of
block-compressed and uncompressed formats. A tTexture stores each mipmap layer in a tLayer. A tTexture can be created
from either a tPicture or a dds file. The purpose of a dds file is so that content-creators have control over the
authoring of each mipmap level and the exact pixel format used. Basically if you've created a dds file, you're saying
you want the final hardware to use the image data unchanged and as authored -- same mip levels, same pixel format,
same dimensions. For this reason, dds files should not be loaded into tPictures where image manipulation occurs and
possibly lossy block-compressed dds images would be decompressed. A dds file may contain more than one image if it is a
cubemap, but a tTexture only ever represents a single image. The tTexture dds constructor allows you to decide which
one gets loaded. tTextures can save and load to a tChunk-based format, and are therefore useful at both pipeline and
for runtime loading. To save to a tChunk file format a tTexture will call the Save method of all the tLayers.

tCubemap:
A cubemap is simply a collection of 6 tTextures. By convention each tTexture has layers of the same pixel format. A
tCubemap may be created from a dds file (assuming the dds file contains a cubemap) or from multiple tPictures. When
construction from a dds is performed, it chooses the correct image to load into the tTexture for each cubemap side.
When constructing from tPictures or image files, the internal tTextures are created for you. You just need to specify
the pixel formats to use and the like. tCubemaps can also save and load themselves into a tChunk-based format.
They do this by looping through their tTextures and calling their individual Save/Load functions.

tFileDDS:
This class is a helper class. It should not be necessary to use this class directly. It knows how to load (but not
save) a Direct Draw Surface (.dds) file. It does zero processing of image data. It knows the details of the dds file
format and loads the data into tLayers. These tLayers may be 'stolen' by the tTexture's dds file constructor. After
they are stolen the tFileDDS is invalid. This is purely for performance.

tFileTGA:
This class is a helper class. It should not be necessary to use this class directly. It knows how to load and save a
targa (.tga) file. It does zero processing of image data. It knows the details of the tga file format and loads the
data into a tPixel array. These tPixels may be 'stolen' by the tPicture's constructor if a targa file is specified.
After the array is stolen the tFileTGA is invalid. This is purely for performance. The tPicture class uses the CxImage
library for image files that are not targas.

________________________________________________________________________________________________________________________
Block Compression

During the creation of mipmap layers the tTexture class may need to perform block compression (BCnTC) on image data.
Originally this was done using either the nvDXT library (now obsolete) or Simon Brown's LibSquish (both MIT licensed).
nvDXT is now obsolete. The nVidia replacement (as of Jan 5 2017) is "Texture Tools 2" which is based on Squish. It now
makes no sense to continue to support both. Since the nVidia toolset _seems_ to be still actively maintained on GitHub
(https://github.com/castano/nvidia-texture-tools last commit 9 days ago) _and_ it supports CUDA for speed, that is what
is currently being used. There is, in fact, a version of SquishLib on SourceForge that is fairly recent, but it is not
by the original author, and doesn't support CUDA in any case.

________________________________________________________________________________________________________________________
Future Improvements

If these classes end up needing more image manipulation functions and filters, it might be a good idea to also have
tPicture be layer based. I would probably rename the current tLayer to be tTextureLayer if this were to happen, and
have tLayer be simply an array of tPixels with a width and height.
