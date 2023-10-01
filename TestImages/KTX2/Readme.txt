Licences
--------
Licensing information for the test images may be see one directory up in the Readme.txt

NVidia Texture Tools Generator (NVTT version as of Sept 20, 2022) was used to generate the ktx2 test images.

HDR Test Images
---------------

BC6s_RGB.ktx2 and all of the signed-floating-point ktx2 files were generated from a proper floating-point source image (Desk.exr from a directory up). They are all in linear-space as NVTT automatically converts sRGB source data to linear for all HRD/floating-point formats. The way NVTT works is it looks at the input file-type and assumes that non-floating-point files are in sRGB. When
saving NVTT puts any floating-point output in linear-space.

NVTT does not support unsigned floating-point exports (including BC6u).

If you run the unit-tests there will be a bunch of tga's written to the KTX2 folder. They all take the form
Written_Format_Channels_Flags.tga
The Flags field contains the load-flags when the unit-tests were performed (or an x if not present).
The flags are:
D : Decode to raw 32-bit RGBA (otherwise leaves the data in same format as in ktx2 file).
G : Gamma-correct resulting image data. Generally not needed if src file is in sRGB already.
R : Reverse rows. Useful for binding textures in OpenGL. Never affects quality. If can't do (eg BC7 and no decode) it will tell you.
S : Spead luminance. If set ktx2 files with luminance will spread (dupe) the data to the RGB channels. Uses Red-only otherwise.

Note for binary alpha formats you need a -0.5 alpha-bias in NVTT to get a 0.5 alpha cutoff threshold.

Warning: NVTT does not generate a BC1(no-binary-alpha) image even when set to BC1(no-binary-alpha) if the input image is 32bit and has an alpha channel. That's why there is a 24bit tga of the test pattern.

All images were converted at production quality. Supercompression was used for all images except for BC7_RGBANoSuper.

The 3 ktx2 files starting with 'ETC' were generated from compressonator (COMP).
COMP generates incorrect ETC2 images (RB are swapped). This fix this the source image was swizzled first. This results in correct ETC2 files.
COMP allows you to set ETC1 for the format of a KTX2 file. KTX2 does not have the ability to distinguish between ETC1 and ETC2RGB (there is no separate VK pixel format). That is why there are only 3 images.
