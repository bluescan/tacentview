Licences
--------
Licensing information for the test images may be see one directory up in the Readme.txt

KTX Test Files
--------------

NVidia Texture Tools Generator (NVTT version as of Sept 20, 2022) does not generate ktx (V1) files.
These ktx V1 files were generated using AMD's Compressonator V 4.2.5185

Note: For the BC1DXT1_RGB.ktx file, even if the src tga was 24 bit and alpha threshold was set to 0, Compressonator was still assigning GL_COMPRESSED_RGBA_S3TC_DXT1_EXT as the internal GL format. While this will display fine, the Tacent Unit tests check a match between the pixel-format in the name and what was extracted from the file itself. To fix the ill-formed BC1DXT1_RGB.ktx pixel-format was hex-edited to specify the correct GL_COMPRESSED_RGB_S3TC_DXT1_EXT so we could include the unit-test.
Note: These Compressonator ktx test files do not contain mipmaps.

Note: What Compressonator calls ARGB is actually RGBA. This goes for the UNORM 8888 format as well as the 16F and 32F formats.

Note: Compressonator does not read RLE compressed TGA files properly. Non-run-length-encoded TGA files (24 and 32 bit)