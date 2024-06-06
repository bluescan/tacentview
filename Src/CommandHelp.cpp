// CommandHelp.cpp
//
// Command line help text.
//
// Copyright (c) 2023, 2024 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <System/tPrint.h>
#include <System/tFile.h>
#include <Image/tResample.h>
#include "CommandHelp.h"
#include "TacentView.h"


namespace Command
{
	struct Example
	{
		// The name of the example.
		const char8_t* Name;

		// Use a \n if you want more than one command-line listed.
		const char8_t* Line;

		// Surround words with ^ for markup ital text. Replaced with _ for markup. Replaced with ` for terminal.
		// Surround words with ` for markup code text. Replaced with ' for terminal.
		// Use a | to force a newline in markdown.
		// Desc should be 80 columns per line max. The newlines get replaced with space for markup output.
		const char8_t* Desc;
	};

	extern Example Examples[];
	extern int NumExamples;
}


Command::Example Command::Examples[] =
{

{
u8"Convert to TGA",
u8"tacentview -c",
u8R"EXAMPLE(
This is the simplest conversion command-line. The `-c` (alternatively `--cli`)
simply means do not launch the GUI. The input images are all the files in the
current directory (since no directory was specified). By default all image
types are processed unless you specify the input image types with `-i` or
`--in`. Similarly, output types can be specified with `-o` or `--out`. If this
is missing the default is to output TGA files.
)EXAMPLE"
},

{
u8"Convert Single JPG to TGA",
u8"tacentview -c TactileConceptDevelopment.jpeg",
u8R"EXAMPLE(
This example shows why the `-c` option is needed -- without it GUI-mode is
activated and the images on the command-line are opened in the full graphical
user interface. Note that some input types have multiple valid extensions. e.g.
A JPeg may have `jpg` or `jpeg` extensions and a Tiff may have `tif` or `tiff`.
)EXAMPLE"
},

{
u8"Convert PKM Files to PNG",
u8"tacentview -c --in pkm --out png",
u8R"EXAMPLE(
Here the types of the input and output files are specified explicitly. In this
case a single input type (pkm) and a single output type (png) are specified.
)EXAMPLE"
},

{
u8"Convert PKM Files to PNG with Prefix and Suffix",
u8"tacentview -c -i pkm -o png --outname prefix=T_,suffix=Saved",
u8R"EXAMPLE(
As above the types of the input and output files are specified explicitly. The
`--outname` option specifies a prefix of `T_` to be prepended to all output
filenames. At the end of the filename, before the extension, the suffix `Saved`
will be added.
)EXAMPLE"
},

{
u8"Convert PKM and JPG Files to PNG and BMP",
u8"tacentview -c --in pkm,jpg --out png,bmp\n"
u8"tacentview -c --in pkm -i jpg -o png --out bmp",
u8R"EXAMPLE(
You can specify multiple types for both the input and output. Both
command-lines above do the same thing. In the first more concise example, all
types are specified with a single `--in` or `--out` option. When using this
comma-separated list format, do not put spaces after the commas. The second
line just shows that individual specifications get combined.
)EXAMPLE"
},

{
u8"Create GIFs from Manifest",
u8"tacentview -c @manifest.txt --out gif --outGIF bpp=2,qan=neu,alp=120",
u8R"EXAMPLE(
Uses the @ symbol to specify a manifest file that lists the files to process.
The contents of ^manifest.txt^ could look something like this.|
```|
; Some individual files to process.|
Flipbook.webp|
Flame.apng|
|
; A directory to process.|
MoreImages/|
```|
The `--outGIF` is optional and specifies any non-default parameters for
creating the GIFs. In this case a 2-bit (4 colour) palette is used, the ^neu^
algorithm is used for colour quantization, and the transparency threshold is
120. There are more options for GIF output not shown here including things like
loop behaviour, frame duration, etc.
)EXAMPLE"
},

{
u8"Adjust EXR Exposure and Save to TGA",
u8"tacentview -c --in exr --inEXR gamma=1.8,expo=3.5",
u8R"EXAMPLE(
Many image formats have additional load parameters that may be specified. In
this example EXR files are loaded with a gamma-correction of 1.8 (default is
2.2) and an exposure value of 3.5. Exposure ranges from -10.0 to 10.0 with the
neutral default being 1.0. For EXR files additional load parameters, not shown
here, include defogging (`defog`), and adjusting ^knee^ values (white and
middle points) using knee-low (`knelo`), and knee-high (`knehi`).
)EXAMPLE"
},

{
u8"Convert JPG to TGA Respecting EXIF Orientation",
u8"tacentview -c --in jpg --inJPG strct=true,exifo=true",
u8R"EXAMPLE(
Some JPG files have meta-data that includes, among other things, the
orientation of the camera when the picture was taken. By specifying
`exifo=true` we can undo this orientation automatically for JPG files. The
resultant TGA will always be oriented correctly when we do this. The default is
to not take the EXIF orientation into account. The `strct=true` load parameter
means that any input JPG files must be well-defined and ^strictly^ meet the
JPeg specification. Setting this to false allows some non-conformant images to
be loaded.
)EXAMPLE"
},

{
u8"Convert DDS to TGA Specifying Load Parameters",
u8"tacentview -c --inDDS corr=gamc,gamma=1.8,tone=2.1,spred=F,strct=T A.dds B.dds",
u8R"EXAMPLE(
This example shows loading two DDS files (`A.dds` and `B.dds`) setting all the
supported input parameters. The `corr` parameter specifies the gamma-correction
mode, the `gamma` parameter specifies 1.8 to be used for the correction gamma
(default is 2.2). Assuming the DDS files are encoded with an HDR format like
BC6s or one of the ASTC encodings, the `tone` parameter controls the exposure.
The value of 2.1 pushes the exposure higher (brighter). The spread (`spred`)
parameter applies to single-colour-channel DDS files. It means do not spread
that channel to the other colour channels. Finally, `strct=T` means do not load
DDS files that have errors or inconsistencies in their header -- the default is
false which is a bit more forgiving for some DDS files found in the wild. Since
no `--out` types were specified, TGA files will be generated from the two input
DDS files.
)EXAMPLE"
},

{
u8"Resize Images Preserving Aspect",
u8"tacentview -cw . --op resize[1920,-1] -o jpg",
u8R"EXAMPLE(
Resizes all images in the current directory and saves them as JPG files. The
JPG images will be 1920 pixels wide. The -1 for height means compute the height
so that the original aspect ratio is preserved. The `-w` or `--overwrite` means
overwrite existing files that may be present. Use with caution. There are more
arguments to resize that allow setting resample filters and edge modes that are
not shown here.
)EXAMPLE"
},

{
u8"Resize Images for Thumnails",
u8"tacentview -c --op resize[128,128] -n prefix=T_,replace=Full:Thumb",
u8R"EXAMPLE(
Resizes all images in the current directory to 128x128 and saves them as TGA
files starting with the string `T_`. The `-n` or `--outname` option also
specifies that any filename with the string `Full` in it will have it replaced
with the string `Thumb`. An input file like `Car_Full_08.png` would be resized
and saved as `T_Car_Thumb_08.tga`.
)EXAMPLE"
},

{
u8"Resize Followed by Rotation",
u8"tacentview -ca --op resize[1920,-1] --op rotate[5] --outTGA bpp=24,rle=true",
u8R"EXAMPLE(
Resizes and then applies a 5-degree anti-clockwise rotation to all images in
the current directory. Saves them as TGA files. The `-a` or `--autoname` flag
means if the output file exists the output filename will be modified so the
existing file is not overwritten. The `--outTGA` params force a 24-bit
run-length-encoded TGA to be written.
)EXAMPLE"
},

{
u8"Set Pixel Colours",
u8"tacentview -c --op pixel[2,1,cyan,RGB] --op pixel[-3,-2,#0000FF80,RGBA]",
u8R"EXAMPLE(
Sets the lower-left pixel (3rd to the right and 2nd up) to the colour cyan and
leaves the alpha unchanged. Then sets the upper-right pixel (3rd to the left
and 2nd down) to the colour blue with an alpha of 128. The pixel at (0,0) is
the bottom-left. The pixel at (-1,-1) is the top-right of the current image.
All images are saved as targas (the default).
)EXAMPLE"
},

{
u8"Resize Canvas",
u8"tacentview --cli --verbosity 2 --op canvas[640,400,mm,black]",
u8R"EXAMPLE(
Processes all images in the current directory and saves then as targas. The
`--verbosity 2` means it will output the maximum amount of output text. All
images are resized to 640x400 without resampling. If the new size is larger
than the input, black bars are added. The anchor is set to the middle of the
image. Further arguments not show allow the anchor to be set to a particular
pixel position.
)EXAMPLE"
},

{
u8"Set Aspect Ratio Without Fill",
u8"tacentview -c --op aspect[19:10,crop,*,*,64,64]",
u8R"EXAMPLE(
Resizes image to an aspect ratio of 19:10. If the input image does not have the
same aspect ratio, the sides are cropped so the resultant image takes up the
full size. That is, some of the input image pixels get discarded to accomplish
this without resampling. The anchor is the specific pixel position (64,64)
where (0,0) is the bottom-left.
)EXAMPLE"
},

{
u8"Set Aspect Ratio With Letterbox Fill",
u8"tacentview -c --op aspect[2:1,letter,tr,green]",
u8R"EXAMPLE(
Resizes canvas to an aspect ratio of 2:1. No resampling is performed. In
letterbox mode all source pixels are kept. If the input image has a different
aspect ratio, green bars are added to the image. Since the anchor is the
top-right, the green will be added to the left and bottom edges as necessary.
)EXAMPLE"
},

{
u8"Remove Borders",
u8"tacentview -ckv 2 --op deborder[yellow,RG]",
u8R"EXAMPLE(
Removes image borders by inspecting the rows and columns of all 4 sides. If
both the red and green components are saturated (yellow), the border gets
removed. Since we only check R and G component channels, the blue and alpha may
be any value. The `-k` or `--skipunchanged` flag means if there would be no
change to the input image, it is not saved -- even if the output image type is
different than the input image type. The `-ckv 2` expands to
`--cli --skipunchanged --verbosity 2` which makes it clear the 2 refers to the
verbosity. The `-c` and `-k` are boolean flags and don't have arguments.
)EXAMPLE"
},

{
u8"Crop",
u8"tacentview -c --op crop[rel,20,10,320,200,*]",
u8R"EXAMPLE(
Crops all images in relative-mode. In this mode the crop is from the lower-left
position (20,10) and the next two arguments specify the width (320) and height
(200). The last argument * means use the default fill colour which is
transparent black. This is used if the crop area extends beyond the image being
processed. In absolute mode (abs), the 320,200 would be interpreted as the
upper-right pixel position. In both modes the lower-left and upper-right
positions are included in the final image. This example uses defaults for input
types (all supported) and the output type (tga).
)EXAMPLE"
},

{
u8"Flip",
u8"tacentview -c --op flip\n"
u8"tacentview -c --op flip[]\n"
u8"tacentview -c --op flip[*]\n"
u8"tacentview -c --op flip[h]\n"
u8"tacentview -c --op flip[H]\n"
u8"tacentview -c --op flip[Horizontal]",
u8R"EXAMPLE(
This example demonstrates different ways of executing the same horizontal
right-to-left flip operation (about the vertical axis). Since all arguments
have a default, the square brackets are optional, and if they are present, they
may be empty. The * means use the default for the argument. `flip[h]`,
`flip[H]`, and `flip[Horizontal]` are all synonyms. For a vertical flip you
would need to use `flip[v]` or `flip[V]` or `flip[Vertical]`.
)EXAMPLE"
},

{
u8"Rotate and Fill",
u8"tacentview -c --op rotate[-30,fill,bilinear,bicubic_catmullrom,green]",
u8R"EXAMPLE(
The simplest rotation mode is fill. This example rotates the images clockwise
by 30 degrees and does not crop. Imagine rotating an image clockwise a little
and then placing a rectange around it. The bottom-left corner will be a bit
farther left, the top-left a little higher, etc. This means the rotated image
will be a bit larger than the original and 4 extra triangles will be present.
The fill colour is used to fill those triangles (with green in this case). The
up-filter (bilinear) is used to create a larger image internally so rotation
quality is preserved. The down-filter (bicubic_catmullrom) is used to resample
back down to the required size. In short, fill mode creates larger images than
the original and preserves all image content. Supported filters include:
nearest, box, bilinear, bicubic, bicubic_catmullrom, bicubic_mitchell,
bicubic_cardinal, bicubic_bspline, lanczos_narrow, lanczos, and lanczos_wide.
)EXAMPLE"
},

{
u8"Rotate and Crop",
u8"tacentview -c --op rotate[-0.1r,crop,none]",
u8R"EXAMPLE(
Crop mode (the default) is similar to fill but instead of filling the extra
area introduced by the rotation, it crops it out. The final image will have
sightly smaller size than the original and no fill will be used. Not all
original content is preserved. The `-0.1r` means use radians. Putting `none`
for the up-filter means no resampling is performed (down-filter is ignored)
and no new colours are introduced to the palette -- useful for rotating sprites
or pixel-art.
)EXAMPLE"
},

{
u8"Rotate and Resize",
u8"tacentview -c --op rotate[12,resize,bilinear,none]",
u8R"EXAMPLE(
Rotates 12 degrees anti-clockwise. Resize mode does the same as crop mode but
then resizes the image back to the original dimensions. Useful for rotating
horizons in photos where you don't want to use a fill and want the final image
size to be the same. Using `none` for the down-filter uses a custom down-sample
method that keeps edges sharper.
)EXAMPLE"
},

{
u8"Adjust Levels",
u8"tacentview -c --op levels[0.1,*,0.9,*,*]",
u8R"EXAMPLE(
Adjusts image levels in a manner similar to other photo-editing software where
you can specify black, mid, and white points. The first 3 areguments are the
input black, mid, and white points in the 0 to 1 range. The * for the mid-point
means put it halfway inbetween. The last two *'s are for the output black and
white points and default to 0.0 and 1.0. There are further optional arguments
to the levels operation (not shown in this example) that control which frame(s)
to perform the operation on if the image is animated or has mipmaps, what
colour channels to adjust, and what gamma algorithm to use. For a full
explanation call `tacentview -c --help`.
)EXAMPLE"
},

{
u8"Levels One Frame One Channel",
u8"tacentview -c --op levels[0.2,0.6,0.8,0.1,0.9,4,R,false]",
u8R"EXAMPLE(
Adjusts image levels like the previous example except we specify all five black
and white points explicitly. The 4 is the zero-based frame number to operate on
if you have an animated or multi-page input image. If this is set to -1 or * it
adjusts levels on all frames. The `R` means the red channel only. Choices are
R, G, B, RGB (the default), and A. The last argument lets you specify a
continuous or discontinuous function for the mid-point gamma algorithm. False
means discontinuous at gamma 1 (approximates Photoshop). True means use a
continuous base-10 power curve that smoothly transitions the full range
(similar to the GIMP).
)EXAMPLE"
},

{
u8"Contrast",
u8"tacentview -ca --op contrast[0.75] -o gif --paramsJPG 90",
u8R"EXAMPLE(
Adjusts the contrast to 0.75 and saves the adjusted images as jpg files
(auto-renaming them if they exist). A contrast value of 0.5 is neutral. Larger
than 0.5 is more contrast up to a maximum of 1 (black and white). Lower than
0.5 is less contrast down to a minimum of 0 (all grey). Optional arguments not
shown include the frame(s) to apply the adjustment to and which colour
channels. The 90 is the quality setting from 1 to 100 for the jpg file.
)EXAMPLE"
},

{
u8"Brightness",
u8"tacentview -ca --op brightness[0.4] -o qoi --outQOI bpp=32,spc=srgb ",
u8R"EXAMPLE(
Adjusts the brightness to 0.4 and saves the adjusted images as qoi files
(auto-renaming them if they exist). A brightness value of 0.5 is neutral.
Larger than 0.5 is brighter up to a maximum of 1 (full white). Lower than 0.5
is dimmer down to a minimum of 0 (black). Optional arguments not shown include
the frame(s) to apply the adjustment to and which colour channels. The
`--outQOI` forces a 32-bit qoi to be generated in the sRGB colour-space.
)EXAMPLE"
},

{
u8"Quantize using Wu Algorithm",
u8"tacentview -cav 2 --op quantize[wu,256,true] -o qoi --outQOI bpp=24,spc=srgb",
u8R"EXAMPLE(
Quantizing is the (difficult) process of reducing the number of colours used in
an image. There are a number of algorithms that may be applied to do this while
keeping the image looking as good as possible. ^Tacent View^ supports the
Xiaolin Wu method (wu), a simple predefined palette (fix), a neural-net
algorithm (neu), and a spatial quantization method (spc). In this example we
use the default Wu method and reduce the number of colours to 256. The true
means check for exactness -- when this check is on, if the number of colours
currently in the image is already less-than or equal-to the number requested,
the image is left unmodified. If false, the algorithm is run regardless (handy
for forcing a black and white image with colours set to 2 while using 'fix' for
the method). The output files are written as 24 bit qoi files in sRGB space.
Verbosity is full and auto-rename is enabled.
)EXAMPLE"
},

{
u8"Quantize using NeuQuant Algorithm",
u8"tacentview -ca --op quantize[neu,256,true,2] -o qoi --outQOI bpp=24,spc=srgb",
u8R"EXAMPLE(
Similar to ^Quantize using Wu Algorithm^ except using the NeuQuant
neural-learning algorithm. When using this algorithm the 4th argument (2) is
the sample-factor. This value affects the speed/quality ratio and must be
between 1 and 30 (inclusive). 1 results in the highest quality by training on
the full set of input pixels, but is slower than 30 which only uses 1/30 of the
input pixels. The number of colours (256) must be between 2 and 256 inclusive.
There is no requirement it be a power-of or divisible-by 2.
)EXAMPLE"
},

{
u8"Quantize using ScolorQ Algorithm",
u8"tacentview -ca --op quantize[spc,16,true,3,8.0] -o qoi --outQOI bpp=24,spc=srgb",
u8R"EXAMPLE(
Similar to ^Quantize using Wu Algorithm^ except using the ScolorQ spatial
quantization algorithm. When using this algorithm the 4th argument (3) is the
filter-size and must be 1, 3, or 5. ScolorQ supports dithering -- the 5th
argument (from 0.0 to 30.0) represents the amount of dither. A value of 0.1 is
essentially no dither, while 20+ is a lot. If dither is set to 0.0 a good value
is computed for you based on the image size and number of requested colours.
ScolorQ is a good choice for small palette sizes (here we chose 16) but can be
quite slow for larger images or palette sizes bigger than 30.
)EXAMPLE"
},

{
u8"Set R and B Channels",
u8"tacentview -ca --op channel[set,RB,#0F00A200]",
u8R"EXAMPLE(
This example will set all red channels in all input images to 15, and all blue
channels to 162.
)EXAMPLE"
},

{
u8"Blend Background into RGB Channels",
u8"tacentview -ca --op channel[blend,RGBA,#80808AFF]",
u8R"EXAMPLE(
Blends a grey-blue backgraound into the RGB components of the input images.
Each pixel's current alpha component is used to determine how much of the
backgound to blend-in. Since alpha (A) is specified in the channels
(2nd argument), the final alpha will be set to what is specified in the
background colour (3rd argument) -- in this case FF (255). If alpha was not
specified the blend still happens but the original alpha will remain unchanged
in the output image.
)EXAMPLE"
},

{
u8"Spread G Channel into R and B",
u8"tacentview -ca --op channel[spread,G]",
u8R"EXAMPLE(
Spreads (copies) the green (G) channel of each pixel into the red and blue
channels. Spread always takes in a single channel and spreads it to the RGB
channels of the image.
)EXAMPLE"
},

{
u8"Set Channels to Intensity",
u8"tacentview -ca --op channel[intens,RG]",
u8R"EXAMPLE(
Computes the intensity of every pixel (always based on the RGB values) and sets
the RG components to the computed intensity. You may choose any combination of
RGBA channels to set. Use RGB to create a greyscale image with unchanged alpha.
)EXAMPLE"
},

{
u8"Swizzle RGB to BGR",
u8"tacentview -ca --op swizzle[BGR]",
u8R"EXAMPLE(
Swaps the red and the blue channels. In order, the new red channel gets the
original blue channel, the green gets green, and the new blue channel gets red.
This is the same as `swizzle[B*R]` and is also the same as `swizzle[B*R*]`. The
asterisks just grab the corresponding original channel.
)EXAMPLE"
},

{
u8"Swizzle Set Full Alpha",
u8"tacentview -ca --op swizzle[RGB1]",
u8R"EXAMPLE(
Keeps the RGB channels the same and sets the alpha channel to full. The '1'
means saturate -- since it is in the A position, alpha gets saturated.
)EXAMPLE"
},

{
u8"Swizzle Clear G and B",
u8"tacentview -ca --op swizzle[*00*]",
u8R"EXAMPLE(
Keeps the red and alpha channels the same and sets the green and blue to zero.
The '0' means, well, zero -- since it is in the G and B positions, they get set
to zero.
)EXAMPLE"
},

{
u8"Swizzle Set RGB to G with Full Alpha",
u8"tacentview -ca --op swizzle[GGG1]",
u8R"EXAMPLE(
There is no restriction on repeating colour components. That is, swizzle is not
restricted to permutations. Here the original green channel is placed in the
RGB channels and the alpha (usually interpreted as opacity) is set to full.
This will generate a grey-scale image based only on the original green channel.
If you want a grey-scale based on intensity, use the channel operation above.
)EXAMPLE"
},

{
u8"Extract All Frames",
u8"tacentview -ck --op extract -i apng -o bmp --outBMP bpp=auto",
u8R"EXAMPLE(
Extracts all frames from every APNG input file and saves them as BMP files. The
`-k` means the original input images are not resaved (this operation does not
modify the input images at all). The `auto` means auto-determine the
bits-per-pixel for the BMP based on the image opacity. You can use the extract
operation on any file that stores more than one image inside -- extract mipmaps
from ktx2 files, extract pages from tiff files, extract animation frames from
webp/gifs, etc. By default the output file names will be based on the input
with _NNN appended. The NNN is the frame number. Unless you specify otherwise,
the extracted images are placed in a subdirectory called 'Saved'.
)EXAMPLE"
},

{
u8"Extract Specific Frames",
u8"tacentview -ck --op extract[0-2+!4-6+!7-10!,FramesDir,FrameFile] -i gif InImage.gif",
u8R"EXAMPLE(
Extracts frames 0,1,2,5,6,8,9 from InImage.gif and saves them as TGA files in a
directory called FramesDir. The images will be named *FrameFile_000.tga,
FrameFile_001.tga, FrameFile_002.tga, FrameFile_005.tga*, etc. To specify the
frames, an asterisk by itself means all, `0-2` means 0 to 2 (inclusive), the +
is used to specify more ranges and the ! means exclude the end-point. The range
`!7-10!` would be 8 and 9. A number by itself is just that number as a range so
11 would translate to `11-11`. Since -k is included, InImage.gif itself will
^not^ be resaved as InImage.tga. If the input image has fewer frames than the
ranges being processed, those frames can't and won't be saved because they
don't exist. This is not considered an error. If FrameFile is * or is not
entered, the base filename of the input image(s) is used and the 3-digit
frame-number is still appended.
)EXAMPLE"
},

{
u8"Create an Animated WEBP",
u8"tacentview -ck -o webp --po combine",
u8R"EXAMPLE(
Combines multiple images into a single animated webp. This is an example of a
^post-operation^. Post operations (`--po`) run after all normal operations.
They run on the same set of original input images, in this case TGAs (the
default input type). Only if a normal operation (`--op`) modifies an input
image is the change included in the post operation. The `combine` post
operation has a number of optional arguments not shown here for conciseness --
specifically the default frame duration is `33ms`, the webp is written to a
folder called `Combined`, and the filename will take the form
`Combined_YYYY-MM-DD-HH-MM-SS_NNN.webp` where NNN is the number of frames.
)EXAMPLE"
},

{
u8"Create an Animated WEBP and APNG",
u8"tacentview -ck -o webp,apng --po combine",
u8R"EXAMPLE(
Same as previous example but creates both a WEBP animated image and an APNG
animated image from the same set of input images.
)EXAMPLE"
},

{
u8"Create an Animated GIF",
u8"tacentview -ck -o gif --po combine[0-19:20+20-39:100,ResultDir,AnimImage]",
u8R"EXAMPLE(
Creates an animated gif called `AnimImage.gif` from the input tga files. The
gif will be placed in a directory called `ResultDir`. The first argument,
`0-19:25+20-39:200` means the first 20 frames (`0-19`) will get a 25ms frame
duration, and the next 20 (`20-39`) will get a 200ms frame duration. If there
are more than 40 input images, the remainder will get the default 33ms frame
duration. If there are only 30 input images, the first 20 will be at 25ms and
the remaining 10 at 200ms. You may also leave out the frame-range before the
colon in which case the duration applies to all frames. For example,
`--po combine[100]` will set all frame durations to 100 milliseconds. The
default gif save parameters support auto-detecting transparency and output an
8-bit palette using Wu colour quantization.
)EXAMPLE"
},

{
u8"Create a Multipage TIFF",
u8"tacentview -ck -o tif --po combine[0:1000+1:2000+2:3000] page1.qoi page2.qoi page3.qoi",
u8R"EXAMPLE(
Creates a multipage tiff called `Combined_YYYY-MM-DD-HH-MM-SS_003.tif` from the
3 input qoi files. The tiff will be placed in a directory called `Combined`.
^Tacent View^ uses a tiff-tag (specifically TIFFTAG_SOFTWARE) to store
additional information in each page of the tiff. In particular the
page-duration is stored in this field while not affecting compatibility with
other software. This allows the specified 1, 2 and 3 second frame durations to
be stored with each tiff page. If this tiff is later loaded in ^Tacent View^ it
can be treated and played as an animated image. Use 16 milliseconds
(`--po combine[16]`) to achieve 60 FPS.
)EXAMPLE"
},

{
u8"Create a Flipbook TGA and PNG",
u8"tacentview -ck -o tga,png --po contact[]",
u8R"EXAMPLE(
Creates two contact sheets called `Contact_YYYY-MM-DD-HH-MM-SS_NNxMM.tga` and
`Contact_YYYY-MM-DD-HH-MM-SS_NNxMM.png` from the images in the current
directory. The number of rows and columns (`NNxMM`) are computed for you based
on the number of input images. The output tga and png files are placed in a
directory called `Contact`. All input images must be the same dimension. You
may use regular `--op` operations to resize first if desired.
)EXAMPLE"
},

{
u8"Create a Flipbook QOI with 5 Columns",
u8"tacentview -ckv 2 -o qoi --po contact[5,*,*,FlipOut,FiveColFile]",
u8R"EXAMPLE(
Creates a contact sheet called `FiveColFile.qoi` from the images in the current
directory. The number of columns is set explicitly to 5. The rows will be
computed for you so that all input images will be included. The output qoi file
will be placed in a directory called `FlipOut` which will be created for you if
it doesn't exist. All input images must be the same dimension. You may use
regular `--op` operations to resize first if desired.
)EXAMPLE"
},

{
u8"Create a 7x7 Flipbook JPG and BMP",
u8"tacentview -ckv 2 -o jpg,bmp --po contact[7,7,white]",
u8R"EXAMPLE(
In this example we set both the number of columns and the number of rows. If
there are more than 49 images in the input directory, only the first 49 will be
included. If there are fewer than 49 input images, the contact-sheet (flipbook)
will have some ^empty^ pages. Empty pages get filled with opaque white. If
white was not specified or * was used, the fill would be transparent black.
Since JPG does not support alpha, the JPG output ignores transparency (it will
be opaque). The BMP output supports alpha so transparency in both the pages and
empty-page fill-colour is taken into account. The fill colour may be precisely
specified in hexadecimal using #RRGGBBAA. If a single decimal integer is used
instead, the fill is that value for all RGBA channels. The predefined colour
names are: black, white, grey, red, green, blue, yellow, cyan, magenta, and
trans (transparent black, the default). Colour names other than `trans` all
default alpha to full (opaque).
)EXAMPLE"
},

};


int Command::NumExamples = tNumElements(Examples);


void Command::PrintExamples()
{
	for (int e = 0; e < NumExamples; e++)
	{
		tString name;
		tsPrintf(name, "Example %d - %s", e, Examples[e].Name);

		tString line;
		tsPrintf(line, "%s", Examples[e].Line);

		tString desc;
		tsPrintf(desc, "%s", Examples[e].Desc);

		desc.Replace("```", "");
		desc.Remove('|');
		desc.Replace('`', '\'');
		desc.Replace('^', '`');

		if (e)
			tPrintf("\n");
		tPrintf("\n%s\n\n", name.Chr());
		tPrintf("%s\n", line.Chr());
		tPrintf("%s", desc.Chr());
	}
}


void Command::PrintExamplesMarkdown()
{
	for (int e = 0; e < NumExamples; e++)
	{
		tString name;
		tsPrintf(name, "**Example {%% increment egnum %%} - %s**", Examples[e].Name);

		tString line;
		tsPrintf(line, "```\n%s\n```", Examples[e].Line);

		tString desc;
		tsPrintf(desc, "%s", Examples[e].Desc);
		desc.Replace('\n', ' ');
		desc.Replace("| ", "\n");
		desc.RemoveLast();
		desc.RemoveFirst();
		desc.Replace('^', '_');

		tPrintf("%s\n", name.Chr());
		tPrintf("%s\n", line.Chr());
		if (e != (NumExamples-1))
			tPrintf("%s\\\n\\\n\\\n", desc.Chr());
		else
			tPrintf("%s\n", desc.Chr());
	}
}


void Command::PrintHelp()
{
	const int maxCols = 80;
	// The +1 on the FindChar calls deals with not found as well as incrementing 1 passed the newline.
	tString intypes("Supported input file types: ");
	for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
	{
		int lineStart = intypes.FindChar('\n', true) + 1;
		if ( (tStd::tStrlen(intypes.Chr()+lineStart)%maxCols) >= (maxCols-tSystem::tGetFileTypeName(typ->FileType).Length()) )
			tsaPrintf(intypes, "\n");
		tsaPrintf(intypes, "%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
	}

	tString inexts("These cover file extensions: ");
	for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Load.First(); typ; typ = typ->Next())
	{
		tList<tStringItem> extensions;
		tSystem::tGetExtensions(extensions, typ->FileType);
		for (tStringItem* ext = extensions.First(); ext; ext = ext->Next())
		{
			int lineStart = inexts.FindChar('\n', true) + 1;
			if ( (tStd::tStrlen(inexts.Chr()+lineStart)%maxCols) >= (maxCols-ext->Length()) )
				tsaPrintf(inexts, "\n");
			tsaPrintf(inexts, "%s ", ext->Chr());
		}
	}

	tString outtypes("Supported output file types: ");
	for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_Save.First(); typ; typ = typ->Next())
	{
		int lineStart = outtypes.FindChar('\n', true) + 1;
		if ( (tStd::tStrlen(outtypes.Chr()+lineStart)%maxCols) >= (maxCols-tSystem::tGetFileTypeName(typ->FileType).Length()) )
			tsaPrintf(outtypes, "\n");
		tsaPrintf(outtypes, "%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
	}

	tString outtypesanim("  Supported animated types: ");
	for (tSystem::tFileTypes::tFileTypeItem* typ = Viewer::FileTypes_SaveMultiFrame.First(); typ; typ = typ->Next())
	{
		int lineStart = outtypesanim.FindChar('\n', true) + 1;
		if ( (tStd::tStrlen(outtypesanim.Chr()+lineStart)%maxCols) >= (maxCols-tSystem::tGetFileTypeName(typ->FileType).Length()) )
			tsaPrintf(outtypesanim, "\n");
		tsaPrintf(outtypesanim, "%s ", tSystem::tGetFileTypeName(typ->FileType).Chr());
	}

	tString filters("Supported filters: ");
	for (int f = 0; f < int(tImage::tResampleFilter::NumFilters)+1; f++)
	{
		int lineStart = filters.FindChar('\n', true) + 1;
		if ( (tStd::tStrlen(filters.Chr()+lineStart)%maxCols) >= (maxCols-tStd::tStrlen(tImage::tResampleFilterNamesSimple[f])) )
			tsaPrintf(filters, "\n");
		tsaPrintf(filters, "%s ", tImage::tResampleFilterNamesSimple[f]);
	}

	tString edgemodes("Supported edge modes: ");
	for (int e = 0; e < int(tImage::tResampleEdgeMode::NumEdgeModes); e++)
	{
		int lineStart = edgemodes.FindChar('\n', true) + 1;
		if ( (tStd::tStrlen(edgemodes.Chr()+lineStart)%maxCols) >= (maxCols-tStd::tStrlen(tImage::tResampleEdgeModeNamesSimple[e])) )
			tsaPrintf(edgemodes, "\n");
		tsaPrintf(edgemodes, "%s ", tImage::tResampleEdgeModeNamesSimple[e]);
	}

	tCmdLine::tPrintUsage();

	// In editor the column num at EOL (after last character) should be 80 or less.
	tPrintf
	(
R"USAGE010(MODE
----
You must call with -c or --cli to use this program in CLI mode.

Use the --help (-h) flag to print this help. The help option is the only one
that does not require -c. To view generic command-line syntax help use the
--syntax (-s) flag. For example, to print syntax usage you would call
'tacentview -cs' which expands to 'tacentview -c -s'. To view a list of
examples use the --examples (-x) flag with a command like 'tacentview -cx'.
For generation of the examples in markdown, use the --markdown (-m) flag. For
example 'tacentview -cm > markdown.md'

Set output verbosity with --verbosity (-v) and a single integer value after it
from 0 to 2. 0 means no text output, 1 is the default, and 2 is full/detailed.

To launch in GUI mode run without any arguments or with the file or directory
you want to open as the argument. Directories should be specified with a
trailing slash. You may optionally specify the profile to use with the
--profile (-p) command followed by the profile name. Current profile names
are 'main', 'basic', 'kiosk', and 'alt'.

When launched with --profile the active profile is not remembered on exit.
Changes made to the profile are, however, persistent. Changes can include
anything from what dialogs are open, zoom mode, slideshow and background
settings, etc.

e.g. 'tacentview -p kiosk C:/Slidehow/' will start a slidewhow of all the
images inside the 'SlideShow' directory. By default the kiosk profile
auto-starts the slideshow in shuffle mode.
)USAGE010", intypes.Chr(), inexts.Chr()
	);
	tPrintf
	(
R"NOTATION010(
NOTATION
--------
Values and arguments in this help text follow the following rules:
- Real     : Real numbers are denoted by including the decimal point.
- Integers : Integer numbers are denoted by not including the decimal point.
- Hex      : Hexadecimal values are prefixed with a hash (#).
- Names    : Some values are simple string names. These are comprised of
             alphabetic upper and lower case characters only.
- Booleans : You may use "true", "t", "yes", "y", "on", "enable", "enabled",
             "1", "+", and strings that represent non-zero integers as true.
             These are case-insensitive. False is the result otherwise.
- Defaults : Default values are denoted with an asterisk (*). Setting a value
             to * will set it to the default value.
- Ranges   : Value ranges are specified in interval notation where [a,b] means
             inclusive and (a,b) means exclusive. e.g. [2,5) -> 2,3,4.
)NOTATION010"
	);
	tPrintf
	(
R"INPUTIMAGES010(
INPUT IMAGES
------------
Each parameter of the command line should be a file or directory to process.
You may enter as many as you need. If no input files are specified, the current
directory is processed. You may also specify a manifest file containing images
to process using the @ symbol.

e.g. @list.txt will load files from a manifest file called list.txt. Each line
of a manifest file should be the name of a file to process, the name of a dir
to process, start with a line-comment semicolon, or simply be empty.

You may specify what types of input images to process. If you do not specify
any types, ALL supported imgage types are processed. A type like 'tif' may have
more than one accepted extension (tif and tiff). The extension is not
specified, the type is. Use the --in (-i) option to specify one or more input
types. You may have more than one -i to process multiple types or you may
specify multiple types with a comma-separated list. For example, '-i jpg,png'
is the same as '-i jpg -i png'. If you specify only unsupported or invalid
types a warning is printed and tga images will be processed.

%s
%s
)INPUTIMAGES010", intypes.Chr(), inexts.Chr()
	);
	tPrintf
	(
R"LOADPARAMS010(
LOAD PARAMETERS
---------------
Some image types support various parameters while being loaded. Specifying load
parameters takes the form:

 --inTTT param1=value1,param2=value2,etc

where TTT represents the image type, the lack of spaces is important, and both
param names and values are case sensitive. All loading parameters have
reasonable defaults -- there is no requirement to specify them if the defaults
are sufficient. Image types with load parameters:

--inASTC
  colp: Colour profile. Possible values:
        sRGB* - Low dynamic range RGB in sRGB space. Linear alpha.
        gRGB  - Low dynamic range RGB in gamma space. Linear alpha.
        lRGB  - Low dynamic range RGBA in linear space.
        HDRa  - High dynamic range RGB in linear space. LDR linear alpha.
        HDRA  - High dynamic range RGBA in linear space.
  corr: Gamma correction mode, Possible values:
        none  - No gamma correction is performed.
        auto* - Apply gamma correction based on colour profile set above.
        gamc  - Apply gamma compression using an encoding-gamma of 1.0/gamma.
        srgb  - Apply gamma compression by applying a Linear->sRGB transform.
  gamma:Gamma value. Used when an encoding-gamma is needed. Default is 2.2*.
        Range is [0.5,4.0]
  tone: For HDR images. Tone-map exposure applied if this is >= 0.0. The
        non-negative valid range is [0.0,4.0]. A value of 0.0 is black and 4.0
        is over-exposed. A value of 1.0 is neutral. Negative values do not
        apply tone-map exposure. Default is -1.0* for no application.

--inDDS
  corr: Gamma correction mode. Possible values:
        none  - No gamma correction is performed.
        auto* - Apply gamma correction based on colour space of pixel format.
        gamc  - Apply gamma compression using an encoding-gamma of 1.0/gamma.
        srgb  - Apply gamma compression by applying a Linear->sRGB transform.
  gamma:Gamma value. Used when an encoding-gamma is needed. Default is 2.2*.
  tone: For HDR images. Tone-map exposure applied if this is >= 0.0. The
        non-negative valid range is [0.0,4.0]. A value of 0.0 is black and 4.0
        is over-exposed. A value of 1.0 is neutral. Negative values do not
        apply tone-map exposure. Default is -1.0* for no application.
  spred:Spread single channel. Boolean true* or false. For DDS files with a
        single Red or Luminance componentconly, spread it to all the RGB
        channels if set to true. If false the red channel takes the value.
        Does not spread single-channel Alpha formats.
  strct:Strict loading. Boolean true or false*. If strict is true a DDS file
        that is not fully compliant with the standard will not be loaded.
        Setting to false allows more forgiving loading behaviour.

--inEXR
  gamma:Gamma value in range [0.6, 3.0]. Default 2.2*.
  expo: Exposure value in range [-10.0, 10.0]. Default 1.0* is neutral.
  defog:Defog value (constant colour bias removal) in range [0.0*, 0.1].
  knelo:Knee Low. Low end of the white and middle grey values in [-3.0, 3.0].
        Values between Knee Low and Knee High are compressed. Default 0.0*.
  knehi:Knee High. High end of white and middle grey values in [3.5, 7.5].
        Values between Knee Low and Knee High are compressed. Default 3.5*.

--inHDR
  gamma:Gamma value in range [0.6, 3.0]. Default 2.2*.
  expo: Exposure value in integral range [-10, 10]. Default 0* is neutral.
  
--inJPG
  strct:Strict loading. Boolean true or false*. If strict is true a JPG file
        that is not fully compliant with the standard will not be loaded.
        Setting to false allows more forgiving loading behaviour.
  exifo:EXIF metadata reorientation. Boolean true or false*. If true undo
        orientation transforms in JPG image as indicated by Exif meta-data.

--inKTX
--inKTX2
  corr: Gamma correction mode. Possible values:
        none  - No gamma correction is performed.
        auto* - Apply gamma correction based on colour space of pixel format.
        gamc  - Apply gamma compression using an encoding-gamma of 1.0/gamma.
        srgb  - Apply gamma compression by applying a Linear->sRGB transform.
  gamma:Gamma value. Used when an encoding-gamma is needed. Default is 2.2*.
  tone: For HDR images. Tone-map exposure applied if this is >= 0.0. The
        non-negative valid range is [0.0,4.0]. A value of 0.0 is black and 4.0
        is over-exposed. A value of 1.0 is neutral. Negative values do not
        apply tone-map exposure. Default is -1.0* for no application.
  spred:Spread single channel. Boolean true* or false. For KTX files with a
        single Red or Luminance componentconly, spread it to all the RGB
        channels if set to true. If false the red channel takes the value.
        Does not spread single-channel Alpha formats.

--inPKM
  corr: Gamma correction mode. Possible values:
        none  - No gamma correction is performed.
        auto* - Apply gamma correction based on colour space of pixel format.
        gamc  - Apply gamma compression using an encoding-gamma of 1.0/gamma.
        srgb  - Apply gamma compression by applying a Linear->sRGB transform.
  gamma:Gamma value. Used when an encoding-gamma is needed. Default is 2.2*.
  spred:Spread single channel. Boolean true* or false. For PKM files with a
        single Red or Luminance componentconly, spread it to all the RGB
        channels if set to true. If false the red channel takes the value.
        Does not spread single-channel Alpha formats.

--inPNG
  strct:Strict loading. Boolean true or false*. If strict is true a JPG file
        that is not fully compliant with the standard will not be loaded.
        Setting to false allows more forgiving loading behaviour. In
        particular some software saves JPG/JFIF-encoded files with the png
        extension. Setting this to false allows these 'png' files to load.
  apng: Load Animated PNG inside a PNG. Boolean true or false*. If apng is
        true the loading code will detect an animated PNG (APNG) when stored
        inside a regular PNG file. This allows the command-line to load all
        the frames of an APNG file even if it has a regular (single-frame)
        png extension.
)LOADPARAMS010"
	);
	tPrintf
	(
R"OPERATIONS010(
OPERATIONS
----------
Operations are specified using --op opname[arg1,arg2,...]
There must be no spaces between arguments. The operations get applied in the
order they were specified on the command line. The full sequence of operations
is applied to each and every input image. Optional arguments are denoted with
an asterisk and do not need to be supplied. When either optional arguments are
not provided or * is entered, the default value is used -- look for the
asterisk in the argument description. eg. --op zap[a,b,c*,d*] may be called
with --op zap[a,b] which would do the same thing as --op zap[a,b,*,*]. If the
operation has all optional arguments you may include an empty arg list with []
or leave it out. Eg. zap[a*,b*] may be called with --op zap[] or just --op zap.

--op pixel[x,y,col,chan*]
  Sets the pixel at (x,y) to the colour supplied. The chan argument lets you
  optionally select which pixel colour channels should be modified. You may
  choose any combination of RGBA colour channels.
  x:    The pixel x coordinate. 0* is the first/leftmost pixel. Specifying -1
        will be the pixel at the far right regardless of image width. A -2
        represents the second to last pixel on the right. Using negatives this
        way is handy since not all processed images are required to have the
        same width. 
  y:    The pixel y coordinate. 0* is the first pixel on the bottom. Specifying
        -1 will be the pixel at the image top regardless of image height. A -2
        represents the second to last pixel from the top. Using negatives this
        way is handy since not all processed images are required to have the
        same height.
  col:  Pixel colour. Specify the colour using a hexadecimal in the form
        #RRGGBBAA, a single integer spread to RGBA, or a predefined name:
        black*, white, grey, red, green, blue, yellow, cyan, magenta, or trans
        (transparent black).
  chan: Colour channels to set. The channels are specified with any combination
        of the letters RGBA or rgba. Default is RGBA*. At least one
        valid channel should be specified otherwise the default is used. Eg. RG
        sets the red and green channels. abG sets alpha, blue, and green.

--op resize[w,h,filt*,edge*]
  Resizes image by resampling. Allows non-uniform scale.
  w:    Width. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the height and original aspect.
  h:    Height. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the width and original aspect.
  filt: Resample filter. Default is bilinear*. Only used if dimensions changed
        for the image being processed. See below for valid filter names.
  edge: Edge mode. Default is clamp*. Only used if dimensions changed for the
        image being processed. See note below for valid edge mode names.

--op canvas[w,h,anc*,fill*,ancx*,ancy*]
  Resizes image by modifying the canvas area of the image. You specify the new
  width and height. Vertical or horizontal letterboxes may be needed. This
  operation does not perform resampling.
  w:    Width. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the height and original aspect.
  h:    Height. An int in range [4, 65536], 0*, or -1. If set to 0 or -1 it
        preserves the aspect ratio by using the width and original aspect.
  anc:  Anchor. One of tl, tm, tr, ml, mm*, mr, bl, bm. br. These are
        abbreviations for top-left, top-middle, top-right, etc.
  fill: Fill colour. If letterboxes needed this is their colour. Specify the
        colour using a hexadecimal in the form #RRGGBBAA, a single integer
        spread to RGBA, or a predefined name: black*, white, grey, red, green,
        blue, yellow, cyan, magenta, or trans (transparent black).
  ancx: Explicit anchor X position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.
  ancy: Explicit anchor Y position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.

--op aspect[asp,mode,anc*,fill*,ancx*,ancy*]
  Resizes image by modifying the canvas area of the image. You specify the new
  aspect ratio in the form NUM:DEN. Vertical or horizontal letterboxes may be
  needed. This operation does not perform resampling.
  asp:  Aspect ratio specified in form MM:NN where MM and NN are natural
        numbers. Default* for whole argument yields 16:9. Defaults for
        components are 16*:9*. These are also used if numbers <= 0 are input.
  mode: Mode when resizing. Accepts crop* or letter. In crop mode some of the
        image pixels may be cropped to get the correct aspect. In letterbox
        mode all the image pixels are guaranteed to be kept, but it may be
        necessary to add either horizontal or vertical letterboxes (not both).
  anc:  Anchor. One of tl, tm, tr, ml, mm*, mr, bl, bm. br. These are
        abbreviations for top-left, top-middle, top-right, etc.
  fill: Fill colour. If letterboxes needed this is their colour. Specify the
        colour using a hexadecimal in the form #RRGGBBAA, a single integer
        spread to RGBA, or a predefined name: black*, white, grey, red, green,
        blue, yellow, cyan, magenta, or trans (transparent black).
  ancx: Explicit anchor X position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.
  ancy: Explicit anchor Y position. An int in range [-1*, 65536]. If -1 used
        the anc argument above takes priority.

--op deborder[col*,chan*]
  Removes same-colour borders from images. Looks around the perimeter of the
  image to see if all rows or columns have the same test-colour and decides
  whether to remove or not. You may check any combination of RGBA colour
  channels. You may retrieve the test-colour from the image itself.
  col:  Test colour. Specify the colour using a hexadecimal in the form
        #RRGGBBAA, a single integer spread to RGBA, or a predefined name:
        black, white, grey, red, green, blue, yellow, cyan, magenta, or trans
        (transparent black). The default* is to get the colour from the origin
        of the image being processed. This is the bottom-left pixel.
  chan: Colour channels to test. You may test the border by looking only for
        matches in particular colour channels. These are specified with any
        combination of the letters RGBA or rgba. Default is RGBA*. At least one
        valid channel should be specified otherwise the default is used. Eg. RG
        tests the red and green channels. abG tests alpha, blue, and green.

--op crop[mode,x,y,xw,yh,fill*]
  Crops an image. You get to specify the lower-left origin and either a new
  width/height or the top-right corner. The values are pixels starting at 0.
  If the crop area you specify goes outside the image being processed, the fill
  colour is used. The resultant image must be at least 4x4.
  mode: Coordinate mode. Either abs* or rel. In absolute mode mw and mh are the
        position of the top right extent of the crop area. Pixels outside of
        this are cropped. In relative mode mw and mh are the new width and
        height of the cropped image.
  x:    The x of the lower-left origin of the crop area. Included in final
        pixels. Defaults to 0*.
  y:    The y of the lower-left origin of the crop area. Included in final
        pixels. Defaults to 0*.
  xw:   The max x of the upper-right extent of the crop area OR the new image
        width if in rel mode. Included in final pixels. Defaults to 3* in
        absolute mode or 4* in relative mode. Both defaults result in a 4x4.
  yh:   The max y of the upper-right extent of the crop area OR the new image
        height if in rel mode. Included in final pixels. Defaults to 3* in
        absolute mode or 4* in relative mode. Both defaults result in a 4x4.
  fill: Fill colour. Specify the colour using a hexadecimal in the form
        #RRGGBBAA, a single integer spread to RGBA, or a predefined name:
        black, white, grey, red, green, blue, yellow, cyan, magenta, or
        trans* (transparent black).

--op flip[mode*]
  Flips an image either horizontally or vertically.
  mode: Either horizontal or vertical. Synonyms include h, v, H, V, Horizontal
        and Vertical. If mode not specified or specified as *, default is
        horizontal* which is about the vertical axis (left becomes right and
        vica-versa.

--op rotate[ang,mode*,upft*,dnft*,fill*]
  Rotates an image. Use negative angles for clockwise rotations. At a minimum
  you must supply the rotation angle in degrees or radians. There is also the
  ability to specify different sampling filters to get quality results, or
  preserve original colours for pixel art. After rotation is complete the areas
  not containing image pixels may be filled with a specified colour, or the
  image may be cropped and possibly resized back to original dimensions.
  ang:  Rotation angle. Specified in degrees. Defaults to 0.0* degrees. To
        specify in radians include the letter 'r' after the value. eg. -1.2r.
        Negatives rotate clockwise. For exact 90 and 180 degree rotations the
        following strings should be used so it uses a faster exact algorithm:
        For 90 Degree Anticlockwise :  90  90.0 acw ccw
        For 90 Degree Clockwise     : -90 -90.0 cw
        For 180 Degree Rotation     : 180 180.0
  mode: Rotation mode. One of fill, crop*, or resize. Fill mode will result in
        larger images and the fill-colour is used because the image bounds get
        rotated outside of the original area. Preserves all image content. Crop
        mode does the rotation and then crops anywhere that went outside. This
        may crop a little of the image depending on angle, but no fill is used.
        Resize mode does the same as crop but then proceeds to resample the
        image back to the original dimensions. Resampling does lose a little
        quality so this is not the default. Use resize mode when it is
        desireable to preserve the image size.
  upft: Upsample filter. See below for filter names. Default is bilinear*.
        Upsampling is used to create a larger image before rotation to give
        much better results. It is also valid to enter 'none' in which case no
        scaling-up of the image is performed. In this case all original pixel
        colours are preserved by using nearest neighbour colours. This is fast
        and a good choice for pixel-art and sprites.
  dnft: Downsample filter. Only used if up-filter is not none. This filter is
        used to restore image size after rotation. Specifying none* here uses
        a special down-sample method that produces sharper results. Using box
        filter here is also a good choice. See below for valid filter names.
  fill: Fill colour. Only used if mode was fill. Specify the colour using a
        hexadecimal in the form #RRGGBBAA, a single integer spread to RGBA, or
        a predefined name: black*, white, grey, red, green, blue, yellow, cyan,
        magenta, or trans (transparent black).
)OPERATIONS010"
	);
	tPrintf
	(
R"OPERATIONS020(
--op levels[bp,mp,wp,obp,owp,fram*,chan*,alg*]
  Adjusts image levels in a manner similar to photo-editing software. You
  specify black, mid, and white points, whether to operate on all frames or a
  single frame, which channel to use, and the gamma algorithm to use. At a
  minimum you must specify the 5 black, mid, and white points.
  bp:   Black point. Value must be between 0.0* and 1.0.
  mp:   Mid point. Value must be between blackpoint and whitepoint. If set to *
        or -1.0 the midpoint is computed automatically as the halfway point
        between the blackpoint and whitepoint.
  wp:   White point. Value must be between midpoint and 1.0*.
  obp:  Output black point. Value must be between 0.0* and 1.0.
  owp:  Output white point. Value must be between 0.0 and 1.0* and bigger than
        the output black point.
  fram: Frame number. This is the 0-based frame number to apply the levels
        adjustments to. If set to -1* all frames are adjusted. If frame number
        is too big campared to the number of frames in the current image, it
        will get clamped to the number available (last frame).
  chan: The channels the level adjustments should be made to. Choices are RGB*,
        R, G, B, and A. Lower-case also works.
  alg:  Power midpoint gamma algorithm. Boolean defaulting to true*. Lets you
        decide between 2 algorithms for the curve on the mid-tone gamma. If
        true uses a continuous base-10 power curve that smoothly transitions
        the full range. For this algo the gamma range is [0.1, 10.0] where 1.0
        is linear. This approximates GIMP. If false it tries to mimic Photoshop
        where there is a C1 discontinuity at gamma = 1. In this mode the gamma
        range is [0.01, 9.99] where 1.0 is linear. See below for valid
        arguments to supply boolean true or false.

--op contrast[cont,fram*,chan*]
  Adjusts contrast. Specify the contrast, whether to operate on all frames or a
  single frame, and which channel(s) to use.
  cont: Contrast value from 0.0 (none) to 1.0 (full contrast). A value of 0.5*
        leaves the image unmodified.
  fram: Frame number. This is the 0-based frame number to apply the contrast
        adjustment to. If set to -1* all frames are adjusted. If frame number
        is too big campared to the number of frames in the current image, it
        will get clamped to the number available (last frame).
  chan: The channels the contrast adjustment should be made to. Choices are
        RGB*, R, G, B, and A. Lower-case also works.

--op brightness[brit,fram*,chan*]
  Adjusts brightness. Specify the brightness, whether to operate on all frames
  or a single frame, and which channel(s) to use.
  brit: Brightness value from 0.0 (black) to 1.0 (white). A value of 0.5*
        leaves the image unmodified.
  fram: Frame number. This is the 0-based frame number to apply the brightness
        adjustment to. If set to -1* all frames are adjusted. If frame number
        is too big campared to the number of frames in the current image, it
        will get clamped to the number available (last frame).
  chan: The channels the brightness adjustment should be made to. Choices are
        RGB*, R, G, B, and A. Lower-case also works.

--op quantize[algo,ncol,xact*,fsam*,dith*]
  Quantize the image to a reduced set of colours using various methods. Specify
  the algorithm, number of colours, and any optional parameters. Quantization
  is based on the colour (RGB) components. The alpha channel, if present, is
  preserved/unchanged.
  algo: The method/algorithm for quantization: fix, wu*, neu, spc.
        fix: Fixed/predefined palette used. Pure black and white are
             guaranteed to be present.
        wu*: The Xiaolin Wu algorithm. Fast and good quality.
        neu: NeuQuant algorithm. Learning neural-net. The original high-quality
             quantization method.
        spc: Scolorq algorithm. Spatial quantiation. Slow but good quality for
             low numbers of colours (< 32). Only algo to offer dithering.
  ncol: Number of colours that will be present in image afterwards. Must be
        between 2 and 256* (both inclusive).
  xact: Boolean exact. Default is true*. See below for valid boolean arguments.
        If exact is true and the number of existing colours is less or equal to
        the requested, then the image will look identical after processing. If
        exact is false, all pixels get run through the chosen algorithm.
  fsam: FilterSize or SampleFactor. Only applies to spatial and neu algorithms.
        For scolorq this is the filter-size and must be 1, 3*, or 5.
        For neu quant this is the sample-factor from 1* to 30. Smaller values
        are better quality -- more faithful representation of the original.
  dith: Dither amount. Only applies to scolorq. Values from 0.0 to 30.0.
        The default is 0.0* which means auto-determine the dither amount based
        on the image dimensions and number of colours. Otherwise, a value of
        0.1 results in essentially no dither, and at around 20.0 there is
        significant dithering.

--op channel[mode*,chan*,col*]
  Channel operations affect components of all pixels. The supported modes
  support setting channels, blending using alpha, spreading a channel, and
  writing intensity to channels.
  mode: The channel operation mode. One of:
        set:    Sets specified channels to corresponding component in the
                supplied colour value.
        blend*: Blends background colour (bg) into the specified RGB channels
                by using the pixel-alpha to modulate. The new pixel colour is
                alpha*src_comp + (1-alpha)*bg_comp. This applies to any
                combination of input RGB channels. The final alpha is left
                unmodified if A not specified in channels. If A is specified
                the alpha is set to the A component of the supplied bg colour.
        spread: Spreads a specific single channel into the RGB chanbels.
        intens: Computes pixel intensity and sets any combination of RGBA
                channels to that intensity.
  chan: Colour channels. For set and intensity modes the default is RGB*. For
        blend mode default is RGBA*. For spread mode the single-channel default
        is R. Channels are specified with any combination of the letters RGBA
        or rgba.
  col:  Colour. Only used if mode is blend or set. Specify the colour using a
        hexadecimal in the form #RRGGBBAA, a single integer spread to RGBA, or
        a predefined name: black*, white, grey, red, green, blue, yellow, cyan,
        magenta, or trans (transparent black).
  As an example, the command --op channel will create an image with
  pre-multiplied alphas. The blended in background will be black and the alpha
  will be 255 for all pixels.

--op swizzle[rgba*]
  The swizzle operation allows you to manipulate the RGBA channels of an image
  and swap, duplicate, clear or set them. You can basically take the existing
  channels and rearrange them as you see fit.
  rgba: This is the destination mapping used by the swizzle. It is always in
        the order RGBA. It is made of the characters R, G, B, A, 0, 1, and *.
        The characters are case-insensitive so r, g, b, and a may also be used.
        The asterisk means automatic channel selection. 0 means the channel is
        set to 0 for all pixels. 1 means the channel is set to full for all
        pixels. R means the destiniation channel is taken from the original red
        channel. Similarly for G, B, and A. You do not need to specify all four
        characters if you want the remaining ones to be defaulted to their
        corresponding source channel. This is what the asterisk does.
        The default is **** which is the same as RGBA, both of which leave the
        image unmodified.
  Example 1: --op swizzle[BGR] will swap the red and the blue channels. In
  order, the new red channel gets the original blue channel, the green gets
  green, and the new blue channel gets red. This is the same as swizzle[B*R]
  and is also the same as swizzle[B*R*]. The asterisks just grab the
  corresponding original channel.
  Example 2: --op swizzle[***1] keeps the colours the same but sets the alpha
  to full for all pixels. This is the same as swizzle[RGB1].
  Example 3: --op swizzle[0] clears the red channel. Same as [0GBA] and [0***]
  Example 4: --op swizzle[GGG1] places the original green channel in the new
  red, green and blue channels. It also sets the alpha to full (opaque).

--op extract[frms*,sdir*,base*]
  Extracts frames from a multiframe or animated image. Specify the frame
  numbers to extract, the base filename, and the directory to put them in. The
  output type is specified using -o or --outtype and the output parameters are
  specified using the --paramsTYPE options. See below.
  frms: The frame numbers to extract in range format. In this format you may
        specify multiple ranges separated by a + or U character. A ! means
        exclusive and a - (hyphen) specifies a range. The default is to extract
        all* frames. For example, --op extract[0-2+!4-6+!7-10!] will extract
        frames 0,1,2,5,6,8,9. This is the same as --op extract[0-2+5-6+8-9]
        If the image being processed does not have enough frames for the
        specified range, those frame files will not be created.
  sdir: The sub-directory, relative to the directory the image is in, to place
        the extracted frames. If the sub-directory does not exist, it is
        created for you. Defaults to a directory called Saved*.
  base: The base filename used when saving extracted frames. Defaults* to the
        base filename of the input image. The final filename will be of the
        form Basename_NNN.ext where NNN is the frame number and ext is the
        extension.
  Example 1: --op extract[0-4!] will extract 4 frames (0, 1, 2, and 3) and
  save them to a folder called Saved with names Base_001.tga, Base_002, and
  Base_002.tga.

%s

%s
)OPERATIONS020", filters.Chr(), edgemodes.Chr()
	);
	tPrintf
	(
R"POSTOPS010(
POST OPERATIONS
---------------
Post operations are specified using --po opname[arg1,arg2,...]
These are operations that take more than a single image as input. They are
separated out into a different pass for efficiency -- if we were to do these as
regular inline operations (--op) we would need to have all input images in
memory at the same time. The post-op pass still uses the _same_ set of input
images and it runs after all normal operations have completed. If a regular
operation modifies any of the input files, the modified file(s) are used as
input to the post operation. If a normal operation generates a new file not
included in the inputs, the new file is not used by the post operation.

--po combine[durs*,sdir*,base*]
  Combines multiple input images into a single animated image. The output file
  type must support animation or be able to store multiple sub-images / pages.
%s
  Input images must be all the same dimensions. Use normal operations to resize
  the source images beforehand if necessary. This can be done in a single
  command if you don't mind overwriting your existing source files with the
  --overwrite flag (see below), or do it as two passes.
  durs: Durations for each frame specified in milliseconds. The syntax is a
        sequence of frame-interval:duration pairs separated by + or a U. Frame
        numbers start at 0. If more than one interval overlaps the same frame
        the last overlapping interval-pair is used for the duration. A ! char
        (bang) means exclusive and a hyphen specifies a range. The default
        duration for all frames is 33*. If a frame interval is not specified,
        the supplied duration applies to all frames. For example, if you are
        combining 100 images these are equivalent: combine[], combine[*],
        combine[33], and combine[0-99:33]. To create a 2 second pause on (zero-
        based) frame 10 only: combine[10:2000]. To set the first 3 frames to
        100ms, frame 6 to 16ms, and frame 10, 11, 12 to 1.1s, the following
        could be used: [0-2:100+6:16+10-12:1100]
  sdir: The sub-directory, relative to the current directory, to place the
        combined image in. If the sub-directory does not exist, it is created
        for you. Defaults to a directory called Combined*.
  base: The base filename (not including the extension) used when saving the
        combined image. Defaults* to Combined_YYYY-MM-DD-HH-MM-SS_NNN where
        NNN is the number of frames. The final filename will include the
        correct extension based on the output image type.
  Example 1: -o webp --po combine[0-24:100+50-74:200,OutDir,Animated] creates a
  file called Animated.webp with the first 25 frames each lasting 1/10 second,
  the next 25 at 30fps, the 25 after at 1/5 second, and the remainder at 33ms.

--po contact[cols*,rows*,fill*,sdir*,base*]
  Creates a single contact sheet image (AKA flipbook) from multiple input
  images. You may specify the number of columns and rows or let the operation
  determine it automatically for you based on the number of input images.
  Input images must be all the same dimensions. Use normal operations to resize
  the source images beforehand if necessary. This can be done in a single
  command if you don't mind overwriting your existing source files with the
  --overwrite flag (see below), or do it as two passes. The final output image
  width will always be the frame width times the number of columns and the
  height will be the frame height times the number of rows. If an input image
  has transparency the whole contact sheet will have transparency if the output
  format supports it. When there are fewer input images than cols*rows, empty
  pages are needed. These empty pages are filled with a specified fill colour.
  Pages start at the top-left, one line at a time, from left to right.
  cols: Specify the number of columns you want in the contact sheet. This value
        should be bigger or equal to 0*. When set to 0 (the default) it will
        be computed for you based on the number of rows entered so that all
        input frames will be included. If rows is also 0, both cols and rows
        will be computed for you so that there are enough pages for all input
        images. If both are set and their product is less than the number of
        input images, not all imput images will be in the contact sheet.
  rows: The number of rows. Behaves similarly to cols above.
  fill: If empty pages are needed they are filled with this colour. Specify the
        colour using a hexadecimal in the form #RRGGBBAA, a single integer
        spread to the RGBA channels, or a predefined name: black, white, grey,
        red, green, blue, yellow, cyan, magenta, or trans* (transparent black).
  sdir: The sub-directory, relative to the current directory, to place the
        contact-sheet image in. If the sub-directory does not exist, it is
        created for you. Defaults to a directory called Contact*.
  base: The base filename (not including the extension) used when saving the
        contact image. Defaults* to Contact_YYYY-MM-DD-HH-MM-SS_NNxMM where
        NN is the number of columns and MM is the number of rows. The final
        filename will have an extension based on the output image type.
  Example 1: -o tga --po contact[10,5,white,*,Sheet]
  will create a contact sheet image called Sheet.tga in a directory called
  Contacts. The tga file will be 10 columns by 5 rows. If there are at least 50
  input images, every page will have an image in it. If there are fewer, the
  empty pages will be filled with opaque white.
)POSTOPS010", outtypesanim.Chr()
	);
	tPrintf
	(
R"OUTPUTIMAGES010(
OUTPUT IMAGES
-------------
The output files are generated based on the input files and chosen operations.
Each input image may generate one or more output images based on what the output
types are set to. For example, you might specify that both webp and bmp images
should be saved. The output types are specified with --out followed by a comma-
separated list of types. The short version -o may also be used. If no out type
is specified the default is tga. You may have multiple -o arguments or just a
single one. For example, '-o webp,bmp' is the same as '-o webp -o bmp'.

%s

The output filename matches the input filename except that the extension/type
may be different. Eg. Seascape.jpg would save as Seascape.tga if the out type
was tga only.

The output names may be modified by adding a prefix, suffix, or replacing part
of the name with a different string. This is done using --outname or -n.

--outname prefix=string,suffix=string,replace=old:new

For example, if suffix=_Blockout,replace=Full:Thumb and the input file was
Tree_Full_012.tga then the output file will be Tree_Thumb_012_Blockout.tga. If
replace does not specify a new replacement string, the old string is removed.

If an output file already exists, it is not overwritten. To allow overwrite use
the --overwrite (-w) flag. To have the tool try a different filename if it
aready exists, use the --autoname (-a) flag. It will append the string _NNN to
the name where NNN is a number that keeps incrementing until no existing file
is found. This is applied after any outname prefix, suffix, or replacements.

By default if no modifications are made to an input file, the file is still
saved. This is so easy batch conversions from one type to another may be
performed. Sometimes you may not want to save unmodified files. An example of
this is using the extract operation by itself. If you don't want the unmodified
input image saved, specify -k or --skipunchanged on the command line.
)OUTPUTIMAGES010", outtypes.Chr()
	);
	tPrintf
	(
R"SAVEPARAMS010(
SAVE PARAMETERS
---------------
Different output image types have different features and may support different
parameters when saving. Specifying save parameters takes the form:

--outTTT param1=value1,param2=value2,etc

where TTT represents the image type, the lack of spaces is important, and both
param names and values are case sensitive. All save parameters have reasonable
defaults -- there is no requirement to specify them if the defaults are
sufficient. An asterisk (*) denotes the default value for that parameter. You
may enter a * to set it to default or just don't set the parameter. Image types
with save parameters:

--outAPNG
  bpp:  Bits per pixel. Possible values:
        24    - Force 24 bits per pixel. Alpha channel ignored.
        32    - Force 32 bits per pixel. Alpha channel set to full if opaque.
        auto* - Decide bpp based on opacity of image being saved.
  dur:  Frame duration override in milliseconds. Use -1* for no override.

--outBMP
  bpp:  Bits per pixel. Possible values:
        24    - Force 24 bits per pixel. Alpha channel ignored.
        32    - Force 32 bits per pixel. Alpha channel set to full if opaque.
        auto* - Decide bpp based on opacity of image being saved.

--outGIF
  bpp:  Bits per pixel from 1 to 8*. Since GIFs are palettized this value
        results in images from 2 to 256 colours. If the GIF has transparency,
        one fewer colour can be represented.
  qan:  Quantization method for palette generation. Possible values:
        fix   - Use a fixed colour palette for the chosen bpp. Low quality.
        spc   - Use spatial scolorq algorithm. Slow. Good for 5 bpp or lower.
        neu   - Use neuquant algorithm. Good for 64 colours or more.
        wu*   - Use XiaolinWu algorithm. Good for 64 colours or more.
  loop: Times to loop for animated GIFs. Choose 0* to loop forever. 
  alp:  Alpha threshold. Set to 255 to force opaque. If in [0, 255) a
        pixel-alpha <= threshold results in a transparent pixel. Set to -1(*)
        for auto-mode where a threshold of 127 if used if image is not opaque.
        If set to -1 and image is opaque, the resultant GIF will be opaque.
  dur:  Frame duration override in 1/100 s. Use -1* for no override.
  dith: Dither level. Value in [0.0,2.0+]. Only applies to spatial
        quantization. 0.0* means auto-determine a good value for the current
        image based on its dimensions. Greater than 0.0 means manually set the
        amount. A dither value of 0.1 results in no dithering. 2.0 results in
        significant dithering.
  filt: Filter size. Only applies to spatial quantization. Must be 1, 3*, 5.
  samp: Sample factor. Range is [1,30]. Only applies to neu quantization. 1*
        means whole image learning. 10 means 1/10 of image only. Max value 30
        is fastest.

--outJPG
  qual: Quality of jpeg in range [1,100]. Default is 95*

--outPNG
  bpp:  Bits per pixel. Possible values:
        24    - Force 24 bits per pixel. Alpha channel ignored.
        32    - Force 32 bits per pixel. Alpha channel set to full if opaque.
        auto* - Decide bpp based on opacity of image being saved.

--outQOI
  bpp:  Bits per pixel. Possible values:
        24    - Force 24 bits per pixel. Alpha channel ignored.
        32    - Force 32 bits per pixel. Alpha channel set to full if opaque.
        auto* - Decide bpp based on opacity of image being saved.
  spc:  Colour space. srgb, lin, or auto*. Auto means keep the currenly loaded
        space. Use srgb for the sRGB space. Use lin for linear.

--outTGA
  bpp:  Bits per pixel. Possible values:
        24    - Force 24 bits per pixel. Alpha channel ignored.
        32    - Force 32 bits per pixel. Alpha channel set to full if opaque.
        auto* - Decide bpp based on opacity of image being saved.
  rle:  Run-length encoding. Boolean true or false*. This can reduce tga size
        but some software can't load RLE-compressed TGAs correctly.

--outTIFF
  bpp:  Bits per pixel. Possible values:
        24    - Force 24 bits per pixel. Alpha channel ignored.
        32    - Force 32 bits per pixel. Alpha channel set to full if opaque.
        auto* - Decide bpp based on opacity of image being saved.
  zlib: Use Zlib Compression. Boolean true* or false.
  dur:  Frame duration override in milliseconds. Use -1* for no override.

--outWEBP
  loss: Generate lossy image. Boolean true or false*.
  qual: Quality or compression amount in range [0.0,100.0]. Default is 90.0*.
        Interpreted as quality for lossy images. Larger looks better but bigger
        files. Interpreted as compression strength for non-lossy. Larger values
        compress more but images take longer to generate.
  dur:  Frame duration override in milliseconds. Use -1* for no override.
)SAVEPARAMS010"
	);
	tPrintf
	(
R"EXITCODE010(
EXIT CODE
---------
The return error code is 0 for success and 1 for failure. For 0 to be returned
every specified image must be successfully loaded, processed, and saved. A
failure in any step for any image results in an error. By default processing
continues to the next image even on a failure. If the --earlyexit (-e) flag is
set, processing stops immediately on any failure. Either way, any failure
returns a non-zero exit code.
)EXITCODE010"
	);
}
