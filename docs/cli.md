---
title: CLI

---
## Tacent View CLI
This page is an overview of the features available in _Tacent View_ while using the command line interface. All operations that may be performed in the GUI may also be accessed via the command-line allowing many images to be processed by a single command. For example, a simple CLI command can resize all png files in a list and then save them as jpg files at a desired quality level.

---
## Examples
The easiest way to begin using _Tacent View_ from the commad-line is by example. The following examples assume a) _tacentview.exe_ is in the path or in the current directory, and b) there are multiple images of various types in the current directory. The examples do not exhaustively demonstrate all options and parameters -- see the _usage_ section after the examples for all the options.\
\
\
**Example 1 - Convert to TGA**
```
tacentview.exe -c
```
This is the simplest conversion command-line. The `-c` (alternatively `--cli`) simply means do not launch the GUI. The input images are all the files in the current directory (since no directory was specified). By default all image types are processed unless you specify the input image type with `-i` or `--intype`. The output type could be specified with `-o` or `--outtype`. If this is missing the default is to output TGA files.\
\
\
**Example 2 - Convert PKM Files to PNG**
```
tacentview -c --intype pkm --outtype png
```
Similar to Example 1 except the type of the input and output files is specified explicitly. Since the `exe` extension is optional when using the command prompt, it has been removed from subsequent examples. Additionally, the executable in Linux will not have this extension.\
\
\
**Example 3 - Create GIFs from Manifest**
```
tacentview -c @manifest.txt --outtype gif --paramsGIF 8 wu 0 120 -1 * * *
```
Uses the @ symbol to specify a manifest file that lists the files to process. The contents of _manifest.txt_ could look something like this.
```
; Some individual files to process.
Flipbook.webp
Flame.apng

; A directory to process.
MoreImages/
```
The `--paramsGIF` is optional and specifies any non-default parameters for creating the GIFs. In this case an 8-bit palette is used, the _wu_ algorithm is used for colour quantization, and the transparency threshold is 120. The -1 means don't override any frame durations from the input files (if they are animated).\
\
\
**Example 4 - Resize Images Preserving Aspect**
```
tacentview -cw . --op resize[1920,-1] -o jpg
```
Resizes all images in the current directory and saves them as JPG files. The JPG images will be 1920 pixels wide. The -1 for height means compute the height so that the original aspect ratio is preserved. The `-w` or `--overwrite` means overwrite existing files that may be present. Use with caution. There are more arguments to resize that allow setting resample filters and edge modes that are not shown here.\
\
\
**Example 5 - Resize Followed by Rotation**
```
tacentview -ca --op resize[1920,-1] --op rotate[5] --paramsTGA 24 rle
```
Resizes and then applies a 5-degree anti-clockwise rotation to all images in the current directory. Saves them as TGA files. The `-a` or `--autoname` flag means if the output file exists the output filename will be modified so the existing file is not overwritten. The TGA params force a 24-bit run-length-encoded TGA to be written.\
\
\
**Example 6 - Set Pixel Colours**
```
tacentview -c --op pixel[2,1,cyan,RGB] --op pixel[-3,-2,#0000FF80,RGBA]
```
Sets the lower-left pixel (3rd to the right and 2nd up) to the colour cyan and leaves the alpha unchanged. Then sets the upper-right pixel (3rd to the left and 2nd down) to the colour blue with an alpha of 128. The pixel at (0,0) is the bottom-left. The pixel at (-1,-1) is the top-right of the current image. All images are saved as targas (the default).\
\
\
**Example 7 - Resize Canvas**
```
tacentview --cli --verbosity 2 --op canvas[640,400,mm,black]
```
Processes all images in the current directory and saves then as targas. The `--verbosity 2` means it will output the maximum amount of output text. All images are resized to 640x400 without resampling. If the new size is larger than the input, black bars are added. The anchor is set to the middle of the image. Further arguments not show allow the anchor to be set to a particular pixel position.\
\
\
**Example 8 - Set Aspect Ratio Without Fill**
```
tacentview -c --op aspect[19:10,crop,*,*,64,64]
```
Resizes image to an aspect ratio of 19:10. If the input image does not have the same aspect ratio, the sides are cropped so the resultant image takes up the full size. That is, some of the input image pixels get discarded to accomplish this without resampling. The anchor is the specific pixel position (64,64) where (0,0) is the bottom-left.\
\
\
**Example 9 - Set Aspect Ratio With Letterbox Fill**
```
tacentview -c --op aspect[2:1,letter,tr,green]
```
Resizes canvas to an aspect ratio of 2:1. No resampling is performed. In letterbox mode all source pixels are kept. If the input image has a different aspect ratio, green bars are added to the image. Since the anchor is the top-right, the green will be added to the left and bottom edges as necessary.\
\
\
**Example 10 - Remove Borders**
```
tacentview -ckv 2 --op deborder[yellow,RG]
```
Removes image borders by inspecting the rows and columns of all 4 sides. If both the red and green components are saturated (yellow), the border gets removed. Since we only check R and G component channels, the blue and alpha may be any value. The `-k` or `--skipunchanged` flag means if there would be no change to the input image, it is not saved -- even if the output image type is different than the input image type. The `-ckv 2` expands to `--cli --skipunchanged --verbosity 2` which makes it clear the 2 refers to the verbosity. The `-c` and `-k` are boolean flags and don't have arguments.\
\
\
**Example 11 - Crop**
```
tacentview -c --op crop[rel,20,10,320,200,*]
```
Crops all images in relative-mode. In this mode the crop is from the lower-left position (20,10) and the next two arguments specify the width (320) and height (200). The last argument * means use the default fill colour which is transparent black. This is used if the crop area extends beyond the image being processed. In absolute mode (abs), the 320,200 would be interpreted as the upper-right pixel position. In both modes the lower-left and upper-right positions are included in the final image. This example uses defaults for intput types (all supported) and the output type (tga).\
\
\
**Example 12 - Flip**
```
tacentview -c --op flip
tacentview -c --op flip[]
tacentview -c --op flip[*]
tacentview -c --op flip[h]
tacentview -c --op flip[H]
tacentview -c --op flip[Horizontal]
```
This example demonstrates different ways of executing the same horizontal right-to-left flip operation (about the vertical axis). Since all arguments have a default, the square brackets are optional, and if they are present, they may be empty. The * means use the default for the argument. `flip[h]`, `flip[H]`, and `flip[Horizontal]` are all synonyms. For a vertical flip you would need to use `flip[v]` or `flip[V]` or `flip[Vertical]`.\
\
\
**Example 13 - Rotate and Fill**
```
tacentview -c --op rotate[-30,fill,bilinear,bicubic_catmullrom,green]
```
The simplest rotation mode is fill. This example rotates the images clockwise by 30 degrees and does not crop. Imagine rotating an image clockwise a little and then placing a rectange around it. The bottom-left corner will be a bit farther left, the top-left a little higher, etc. This means the rotated image will be a bit larger than the original and 4 extra triangles will be present. The fill colour is used to fill those triangles (with green in this case). The up-filter (bilinear) is used to create a larger image internally so rotation quality is preserved. The down-filter (bicubic_catmullrom) is used to resample back down to the required size. In short, fill mode creates larger images than the original and preserves all image content. Supported filters include: nearest, box, bilinear, bicubic, bicubic_catmullrom, bicubic_mitchell, bicubic_cardinal, bicubic_bspline, lanczos_narrow, lanczos, and lanczos_wide.\
\
\
**Example 14 - Rotate and Crop**
```
tacentview -c --op rotate[-0.1r,crop,none]
```
Crop mode (the default) is similar to fill but instead of filling the extra area introduced by the rotation, it crops it out. The final image will have sightly smaller size than the original and no fill will be used. Not all original content is preserved. The `-0.1r` means use radians. Putting `none` for the up-filter means no resampling is performed (down-filter is ignored) and no new colours are introduced to the palette -- useful for rotating sprites or pixel-art.\
\
\
**Example 15 - Rotate and Resize**
```
tacentview -c --op rotate[12,resize,bilinear,none]
```
Rotates 12 degrees anti-clockwise. Resize mode does the same as crop mode but then resizes the image back to the original dimensions. Useful for rotating horizons in photos where you don't want to use a fill and want the final image size to be the same. Using `none` for the down-filter uses a custom down-sample method that keeps edges sharper.\
\
\
**Example 16 - Levels All Channels**
```
tacentview -c --op levels[0.1,*,0.9,*,*]
```
Adjusts image levels in a manner similar to other photo-editing software where you can specify black, mid, and white points. The first 3 areguments are the input black, mid, and white points in the 0 to 1 range. The * for the mid-point means put it halfway inbetween. The last two *'s are for the output black and white points and default to 0.0 and 1.0. There are further optional arguments to the levels operation (not shown in this example) that control which frame(s) to perform the operation on if the image is animated or has mipmaps, what colour channels to adjust, and what gamma algorithm to use. For a full explanation call `tacentview -c --help`\
\
\
**Example 17 - Levels One Frame One Channel**
```
tacentview -c --op levels[0.2,0.6,0.8,0.1,0.9,4,R,false]
```
Adjusts image levels like _Example 16_ except we specify all five black and white points explicitly. The 4 is the zero-based frame number to operate on if you have an animated or multi-page input image. If this is set to -1 or * it adjusts levels on all frames. The `R` means the red channel only. Choices are R, G, B, RGB (the default), and A. The last argument lets you specify a continuous or discontinuous function for the mid-point gamma algorithm. False means discontinuous at gamma 1 (approximates Photoshop). True means use a continuous base-10 power curve that smoothly transitions the full range (similar to the GIMP).\
\
\
**Example 18 - Contrast**
```
tacentview -ca --op contrast[0.75] -o gif --paramsJPG 90
```
Adjusts the contrast to 0.75 and saves the adjusted images as jpg files (auto-renaming them if they exist). A contrast value of 0.5 is neutral. Larger than 0.5 is more contrast up to a maximum of 1 (black and white). Lower than 0.5 is less contrast down to a minimum of 0 (all grey). Optional arguments not shown include the frame(s) to apply the adjustment to and which colour channels. The 90 is the quality setting from 1 to 100 for the jpg file.\
\
\
**Example 19 - Brightness**
```
tacentview -ca --op brightness[0.4] -o qoi --paramsQOI 32 srgb 
```
Adjusts the brightness to 0.4 and saves the adjusted images as qoi files (auto-renaming them if they exist). A brightness value of 0.5 is neutral. Larger than 0.5 is brighter up to a maximum of 1 (full white). Lower than 0.5 is dimmer down to a minimum of 0 (black). Optional arguments not shown include the frame(s) to apply the adjustment to and which colour channels. The `32 srgb` forces a 32-bit qoi to be generated in the sRGB colour-space.\
\
\
**Example 20 - Quantize using Wu Algorithm**
```
tacentview -cav 2 --op quantize[wu,256,true] -o qoi --paramsQOI 24 srgb
```
Quantizing is the (difficult) process of reducing the number of colours used in an image. There are a number of algorithms that may be applied to do this while keeping the image looking as good as possible. _Tacent View_ supports the Xiaolin Wu method (wu), a simple predefined palette (fix), a neural-net algorithm (neu), and a spatial quantization method (spc). In this example we use the default Wu method and reduce the number of colours to 256. The true means check for exactness -- when this check is on, if the number of colours currently in the image is already less-than or equal-to the number requested, the image is left unmodified. If false, the algorithm is run regardless (handy for forcing a black and white image with colours set to 2 while using 'fix' for the method). The output files are written as 24 bit qoi files in sRGB space. Verbosity is full and auto-rename is enabled.
\
\
**Example 21 - Quantize using NeuQuant Algorithm**
```
tacentview -ca --op quantize[neu,256,true,2] -o qoi --paramsQOI 24 srgb
```
Similar to _example 20_ except using the NeuQuant neural-learning algorithm. When using this algorithm the 4th argument (2) is the sample-factor. This value affects the speed/quality ratio and must be between 1 and 30 (inclusive). 1 results in the highest quality by training on the full set of input pixels, but is slower than 30 which only uses 1/30 of the input pixels. The number of colours (256) must be between 2 and 256 inclusive. There is no requirement it be a power-of or divisible-by 2.\
\
\
**Example 22 - Quantize using ScolorQ Algorithm**
```
tacentview -ca --op quantize[spc,16,true,3,8.0] -o qoi --paramsQOI 24 srgb
```
Similar to _example 20_ except using the ScolorQ spatial quantization algorithm. When using this algorithm the 4th argument (3) is the filter-size and must be 1, 3, or 5. ScolorQ supports dithering -- the 5th argument (from 0.0 to 30.0) represents the amount of dither. A value of 0.1 is essentially no dither, while 20+ is a lot. If dither is set to 0.0 a good value is computed for you based on the image size and number of requested colours. ScolorQ is a good choice for small palette sizes (here we chose 16) but can be quite slow for larger images or palette sizes bigger than 30.\
\
\
**Example 23 - Set R and B Channels**
```
tacentview -ca --op channel[set,RB,#0F00A200]
```
This example will set all red channels in all input images to 15, and all blue channels to 162.\
\
\
**Example 24 - Blend Background into RGB Channels**
```
tacentview -ca --op channel[blend,RGBA,#80808AFF]
```
Blends a grey-blue backgraound into the RGB components of the input images. Each pixel's current alpha component is used to determine how much of the backgound to blend-in. Since alpha (A) is specified in the channels (2nd argument), the final alpha will be set to what is specified in the background colour (3rd argument) -- in this case FF (255). If alpha was not specified the blend still happens but the original alpha will remain unchanged in the output image.\
\
\
**Example 25 - Spread G Channel into R and B**
```
tacentview -ca --op channel[spread,G]
```
Spreads (copies) the green (G) channel of each pixel into the red and blue channels. Spread always taks in a single channel and spreads it to the RGB channels of the image.\
\
\
**Example 26 - Set Channels to Intensity**
```
tacentview -ca --op channel[intens,RGB]
```
Computes the intensity of every pixel and sets the RGB components to the intensity value. This can be used to create greyscale images. You may set any combination of RGBA channels -- in all cases RGB is used to compute the intensity.


---
## Usage
This is a printout of the CLI usage instructions built into _Tacent View_. To get this printout simply issue the command `tacentview.exe --help`

```
Tacent View 1.0.40 in CLI Mode. Use --help for details.
tacentview Version 1.0.40 by Tristan Grimmer

USAGE: tacentview.exe [options] [inputfiles] 

Options:
--autoname -a           : Autogenerate output file names
--cli -c                : Use command line mode (required when using CLI)
--earlyexit -e          : Early exit / no skipping
--help -h               : Print help/usage information
--intype -i arg1        : Input file type(s)
--op arg1               : Operation
--outtype -o arg1       : Output file type
--overwrite -w          : Overwrite existing output files
--paramsAPNG arg1 arg2  : Save parameters for APNG files
--paramsBMP arg1        : Save parameters for BMP  files
--paramsGIF [8 args]    : Save parameters for GIF  files
--paramsJPG arg1        : Save parameters for JPG  files
--paramsPNG arg1        : Save parameters for PNG  files
--paramsQOI arg1 arg2   : Save parameters for QOI  files
--paramsTGA arg1 arg2   : Save parameters for TGA  files
--paramsTIFF [3 args]   : Save parameters for TIFF files
--paramsWEBP [3 args]   : Save parameters for WEBP files
--po arg1               : Post operation
--skipunchanged -k      : Don't save unchanged files
--syntax -s             : Print syntax help
--verbosity -v arg1     : Verbosity from 0 to 2

Parameters:
[inputfiles]            : Input image files

MODE
----
You MUST call with -c or --cli to use this program in CLI mode.

Use the --help (-h) flag to print this help. To view generic command-line
syntax help use the --syntax (-s) flag. For example, to print syntax usage you
could call tacentview.exe -cs which expands to tacentview.exe -c -s

Set output verbosity with --verbosity (-v) and a single integer value after it
from 0 to 2. 0 means no text output, 1* is normal, and 2 is full/detailed.

INPUT IMAGES
------------
Each parameter of the command line shoud be a file or directory to process. You
may enter as many as you need. If no input files are specified, the current
directory is processed. You may also specify a manifest file containing images
to process using the @ symbol.

eg. @list.txt will load files from a manifest file called list.txt. Each line
of a manifest file should be the name of a file to process, the name of a dir
to process, start with a line-comment semicolon, or simply be empty.

When processing an entire directory of images you may specify what types of
input files to process. A type like 'tif' may have more than one accepted
extension (tif and tiff). The extension is not specified, the type is.
Use the --intype (-i) option to specify an input type. You may have more than
one -i to process multiple types. No -i means all supported types.

Supported input file types: tga png jpg gif webp qoi dds ktx ktx2 astc pkm exr 
hdr apng bmp ico tif 
These cover file extensions: tga png jpg jpeg gif webp qoi dds ktx ktx2 astc 
atc pkm exr hdr rgbe apng bmp ico tif tiff 

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
or leave it out. Eg. zap[*a,b*] may be called with --op zap[] or just --op zap.

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
  mode: Coordinate mode. Either abs* or rel. In absolute mode xw and yh are the
        position of the top right extent of the crop area. Pixels outside of
        this are cropped. In relative mode xw and yh are the new width and
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

Supported filters: nearest box bilinear bicubic bicubic_catmullrom 
bicubic_mitchell bicubic_cardinal bicubic_bspline lanczos_narrow lanczos 
lanczos_wide none 

Supported edge modes: clamp wrap 

Boolean arguments: You may use "true", "t", "yes", "y", "on", "enable",
"enabled", "1", "+", and strings that represent non-zero integers as true.
These are case-insensitive. False is the result otherwise.

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
  Supported animated types: gif webp apng tif 
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

--po contact[cols*,rows*,sdir*,base*]
  Creates a single contact sheet image (AKA flipbook) from multiple input
  images. You may specify the number of columns and rows or let the operation
  determine it automatically for you based on the number of input images.
  Input images must be all the same dimensions. Use normal operations to resize
  the source images beforehand if necessary. This can be done in a single
  command if you don't mind overwriting your existing source files with the
  --overwrite flag (see below), or do it as two passes. The final output image
  width will always be the frame width times the number of columns and the
  height will be the frame height times the number of rows. If any input image
  has a transparency, the final image will have transparency (assuming the
  output format supports it). When blank pages are needed they are filled with
  either opaque black (if no transparency) or transparent black (if there is).
  Pages start at the top-left, one line at a time, from left to right.
  cols: Specify the number of columns you want in the contact sheet. This value
        should be bigger or equal to 0*. When set to 0 (the default) it will
        be computed for you based on the number of rows entered so that all
        input frames will be included. If rows is also 0, both cols and rows
        will be computed for you so that there are enough pages for all input
        images. If both are set and their product is less than the number of
        input images, not all imput images will be in the contact sheet.
  rows: The number of rows. Behaves similarly to cols above.
  sdir: The sub-directory, relative to the current directory, to place the
        contact-sheet image in. If the sub-directory does not exist, it is
        created for you. Defaults to a directory called Contact*.
  base: The base filename (not including the extension) used when saving the
        contact image. Defaults* to Contact_YYYY-MM-DD-HH-MM-SS_NNxMM where
        NN is the number of columns and MM is the number of rows. The final
        filename will have an extension based on the output image type.
  Example 1: -o tga --po contact[10,5,*,Sheet]
  will create a contact sheet image called Sheet.tga in a directory called
  Contacts. The tga file will be 10 columns by 5 rows. Assuming there are at
  least 50 input images, every page will have an image in it.

OUTPUT IMAGES
-------------
The output files are generated based on the input files and chosen operations.
The type of the output images is specified with --outtype type. The short
version -o may also be used. If no outtype is specified the default is tga.
Supported output file types: tga png jpg gif webp qoi apng bmp tif 

The output filename matches the input filename except that the extension/type
may be different. Eg. Seascape.jpg would save as Seascape.tga if the outtype
was tga.

If an output file already exists, it is not overwritten. To allow overwrite use
the --overwrite (-w) flag. To have the tool try a different filename if it
aready exists, use the --autoname (-a) flag. It will append the string _NNN to
the name where NNN is a number that keeps incrementing until no existing file
is found.

By default if no modifications are made to an input file, the file is still
saved. This is so easy batch conversions from one type to another may be
performed. Sometimes you may not want to save unmodified files. An example of
this is using the extract operation by itself. If you don't want the unmodified
input image saved, specify -k or --skipunchanged on the command line.

OUTPUT PARAMETERS
-----------------
Different output image types have different features and may support different
parameters when saving. The following options cover setting these parameters.
These options do not have single-letter short forms. In all cases entering an
asterisk (*) means use the default value. In the desriptions below the *
indicates which is the default.

--paramsAPNG bpp dur
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  dur:  Frame duration override in milliseconds. -1* means use current frame
        duration. 0 or more means override all frames to supplied value.

--paramsBMP bpp
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.

--paramsGIF bpp qan loop alp dur dith filt samp
  bpp:  Bits per pixel from 1 to 8*. Results in palette colours from 2 to 256.
        If the GIF has transparency, one less colour can be represented.
  qan:  Quantization method for palette generation.
        fixed:   Use a fixed colour palette for the chosen bpp. Low quality.
        spatial: Use scolorq algorithm. Slow but good for 32 colours or fewer.
        neu:     Use neuquant algorithm. Good for 64 colours or more.
        wu*:     Use XiaolinWu algorithm. Good for 64 colours or more.
  loop: Times to loop for animated GIFs. Choose 0* to loop forever. 
  alp:  Alpha threshold. Set to 255 to force opaque. If in [0, 255) a pixel
        alpha <= threshold results in a transparent pixel. Set to -1(*) for
        auto-mode where a threshold of 127 if used if image is not opaque -- if
        image is opaque the resultant GIF will be opaque.
  dur:  Frame duration override, -1* means no override. Otherwise in 1/100 s.
  dith: Dither level. Value in range 0.0 to 2.0+. Only applies to spatial
        quantization. 0.0* means auto-determine a good value for the current
        image based on its dimensions. Greater than zero means manually set the
        amount. A dither value of 0.1 results in no dithering. 2.0 results in
        significant dithering.
  filt: Filter size. Only applies to spatial quantization. Must be 1, 3*, or 5.
  samp: Sample factor. Only applies to neu quantization. 1* means whole image
        learning. 10 means 1/10 of image only. Max value is 30 (fastest).

--paramsJPG qual
  qual: Quality of jpeg as integer from 1 to 100. Default is 95*

--paramsPNG bpp
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.

--paramsQOI bpp spc
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  spc:  Colour space. srgb, lin, or auto*. Auto means keep the currenly loaded
        space. Use srgb for the sRGB space. Use lin for linear.

--paramsTGA bpp comp
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  comp: Compression. none* or rle. Use rle for run-length encoded.

--paramsTIFF bpp zlib dur
  bpp:  Bits per pixel. 24, 32, or auto*. Auto means decide based on opacity.
  zlib: Use Zlib Compression. T* or F. This is a boolean. See note below.
  dur:  Frame duration override, -1* means no override. Otherwise units are ms.

--paramsWEBP loss qual dur
  loss: Generate lossy image. T or F*. This is a boolean. See note below.
  qual: Quality or compression amount in range 0.0 to 100.0. Default is 90.0*.
        Interpreted as quality for lossy images. Larger looks better but bigger
        files. Interpreted as compression strength for non-lossy. Larger values
        compress more but images take longer to generate.
  dur:  Frame duration override, -1* means no override. Otherwise units are ms.

EXIT CODE
---------
The return error code is 0 for success and 1 for failure. For 0 to be returned
every specified image must be successfully loaded, processed, and saved. A
failure in any step for any image results in an error. By default processing
continues to the next image even on a failure. If the --earlyexit (-e) flag is
set, processing stops immediately on any failure. Either way, any failure
returns a non-zero exit code.
```

---
## Syntax
This is a printout of the command-line syntax used to specify arguments to _Tacent View_. To printout the syntax usage use the command `tacentview.exe -c --syntax`

```
Tacent View 1.0.40 in CLI Mode. Use --help for details.
Syntax Help:
program.exe [arg1 arg2 arg3 ...]

ARGUMENTS:
Arguments are separated by spaces. An argument must be enclosed in quotes
(single or double) if it has spaces in it or you want the argument to start
with a hyphen literal. Hat (^) escape sequences can be used to put either type
of quote inside. If you need to specify file paths you may use forward or back
slashes. An ARGUMENT is either an OPTION or PARAMETER.

OPTIONS:
An option is simply an argument that starts with a hyphen (-). An option has a
short syntax and a long syntax. Short syntax is a - followed by a single
non-hyphen character. The long form is -- followed by a word. All options
support either long, short, or both forms. Options may have 0 or more
arguments separated by spaces. Options can be specified in any order. Short
form options may be combined: Eg. -al expands to -a -l.

FLAGS:
If an option takes zero arguments it is called a flag. You can only test for a
FLAGs presence or lack thereof.

PARAMETERS:
A parameter is simply an argument that is not one of the available options. It
can be read as a string and parsed however is needed (converted to an integer,
float etc.) Order is important when specifying parameters. If you need a
hyphen in a parameter at the start you will need put the parameter in quotes.
For example, a filename _can_ start with -. Note that arguments that start
with a hyphen but are not recognized as a valid option just get turned into
parameters. This means interpreting a hyphen directly instead of as an option
specifier will happen automatically if there are no options matching what
comes after the hyphen. Eg. 'tool.exe -.85 --add 33 -87.98 --notpresent' work
just fine as long as there are no options that have a short form with digits
or a decimal. In this example the -.85 will be the first parameter,
--notpresent will be the second. The --add is assumed to take in two number
arguments.

ESCAPES:
Sometimes you need a particular character to appear inside an argument. For
example you may need a single or double quote to apprear inside a parameter.
The hat (^) followed by the character you need is used for this purpose.
Eg: ^^ yields ^ | ^' yields ' | ^" yields "

VARIABLE ARGUMENTS:
A variable number of space-separated parameters may be specified if the tool
supports them. The parsing system will collect them all up if the parameter
number is unset (-1).
A variable number of option arguments is not directly supported due to the
more complex parsing that would be needed. The same result is achieved by
entering the same option more than once. Order is preserved. This can also
be done with options that take more than one argument.
Eg. tool.exe -I /patha/include/ -I /pathb/include

EXAMPLE:
mycopy.exe -R --overwrite fileA.txt -pat fileB.txt --log log.txt

The fileA.txt and fileB.txt in the above example are parameters (assuming
the overwrite option is a flag). fileA.txt is the first parameter and
fileB.txt is the second.

The '--log log.txt' is an option with a single argument, log.txt. Flags may be
combined. The -pat in the example expands to -p -a -t. It is suggested only to
combine flag options as only the last option would get any arguments.
```
