Licences
--------
Licensing information for the test images may be see one directory up in the Readme.txt

HDR Test Images
---------------
Three tools were used when generating the dds test images (versions as of Sept 20, 2022).
1) NVidia Texture Tools Generator (NVTT). A drag-and-drop closed-source ImGui application.
2) Microsoft's texconv.exe (TEXC) from their DirectXTex tool suite.
3) AMD's Compressonator (COMP).

BC6s_HDRRGB_Modern.dds and all of the signed-floating-point dds files _were_ generated from a proper floating-point
source image (Desk.exr from a directory up). These ones will have the extra information needed since they were a
float-to-float conversion. They were all created by NVTT.

The way NVTT works is it looks at the input file-type and assumes that non-floating-point files are in sRGB. When
saving NVTT puts any floating-point output in linear-space.

NVTT does not support BC6u (unsigned). Unfortunately TEXC was unable to load the (perfectly valid) Desk.exr file.
So... to create the BC6u_HDRRGB_Modern.dds, TEXC was used with the R32G32B32 linear-space file and converted it
to the BC6u_HDRRGB_Modern.dds unsigned format. The BC6 is the only HDR format that supports unsigned floats.

Compressonator (COMP) was used for creating the legacy dds files that use the ETC encoder. There are 4 of these files.
one for each COMP-Supported ETC format: ETC1, ETC2RGB, ETC2RGBA, and ETC2RGBA1. Note that COMP encodes incorrectly. It
swaps the R and B channels. That is, they interpreted the texture-format name literally. The D3DFMT names are wrong and
MS fixed this with their DXGI names. In the case of the 4 COMP-generated test images, the swizzle to swap R and B was
performed before encoding for better-quality results (colour channels are not independant) and correct dds files.

Encoding Tools
--------------
dds files with 'Legacy' do not contain the D.X.1.0. FourCC header.
dds files with 'Modern' do     contain the D.X.1.0. FourCC header.
