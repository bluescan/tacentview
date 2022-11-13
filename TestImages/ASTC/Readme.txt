Licences
--------
Licensing information for the test images may be see one directory up in the Readme.txt


ASTC Test Files
---------------

All .astc test files were generated using the official ARM astcenc-sse2.exe. This is ARM's (nice and-simple) format after all.

The LDR test images were generated from TacentTestPattern32.tga
The HDR test images were generated from Desk.exr



Example
-------

astcenc-sse2.exe -ch Desk.exr ASTC4x4_HDR.astc 4x4 -thorough

The -ch means compress (c) and HDR (h). A lower-case h means RGB are HDR (linear AND may be outside 0 to 1 range) but alpha A is LDR (0 to 1 linear). A capital H would mean all 4 channels (RGBA) are HDR.

astcenc-sse2.exe -cl TacentTestPattern32.tga ASTC4x4_LDR.astc 4x4 -thorough
The 'l' means LDR. However this one is not used for the LDR test images. The test pattern is in sRGB space so we need:

astcenc-sse2.exe -cs TacentTestPattern32.tga ASTC4x4_LDR.astc 4x4 -thorough
The 's' means sRGB with a linear LDR alpha, which is what we have.
