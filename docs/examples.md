**Example {% increment egnum %} - Convert to TGA**
```
tacentview -c
```
This is the simplest conversion command-line. The `-c` (alternatively `--cli`) simply means do not launch the GUI. The input images are all the files in the current directory (since no directory was specified). By default all image types are processed unless you specify the input image types with `-i` or `--in`. Similarly, output types can be specified with `-o` or `--out`. If this is missing the default is to output TGA files.\
\
\
**Example {% increment egnum %} - Convert Single JPG to TGA**
```
tacentview -c TactileConceptDevelopment.jpeg
```
This example shows why the `-c` option is needed -- without it GUI-mode is activated and the images on the command-line are opened in the full graphical user interface. Note that some input types have multiple valid extensions. e.g. A JPeg may have `jpg` or `jpeg` extensions and a Tiff may have `tif` or `tiff`.\
\
\
**Example {% increment egnum %} - Convert PKM Files to PNG**
```
tacentview -c --in pkm --out png
```
Here the types of the input and output files are specified explicitly. In this case a single input type (pkm) and a single output type (png) are specified.\
\
\
**Example {% increment egnum %} - Convert PKM and JPG Files to PNG and BMP**
```
tacentview -c --in pkm,jpg --out png,bmp
tacentview -c --in pkm -i jpg -o png --out bmp
```
You can specify multiple types for both the input and output. Both command-lines above do the same thing. In the first more concise example, all types are specified with a single `--in` or `--out` option. When using this comma-separated list format, do not put spaces after the commas. The second line just shows that individual specifications get combined.\
\
\
**Example {% increment egnum %} - Create GIFs from Manifest**
```
tacentview -c @manifest.txt --out gif --outGIF bpp=2,qan=neu,alp=120
```
Uses the @ symbol to specify a manifest file that lists the files to process. The contents of _manifest.txt_ could look something like this.
```
; Some individual files to process.
Flipbook.webp
Flame.apng

; A directory to process.
MoreImages/
```
The `--outGIF` is optional and specifies any non-default parameters for creating the GIFs. In this case a 2-bit (4 colour) palette is used, the _neu_ algorithm is used for colour quantization, and the transparency threshold is 120. There are more options for GIF output not shown here including things like loop behaviour, frame duration, etc.\
\
\
**Example {% increment egnum %} - Adjust EXR Exposure and Save to TGA**
```
tacentview -c --in exr --inEXR gamma=1.8,expo=3.5
```
Many image formats have additional load parameters that may be specified. In this example EXR files are loaded with a gamma-correction of 1.8 (default is 2.2) and an exposure value of 3.5. Exposure ranges from -10.0 to 10.0 with the neutral default being 1.0. For EXR files additional load parameters, not shown here, include defogging (`defog`), and adjusting _knee_ values (white and middle points) using knee-low (`knelo`), and knee-high (`knehi`).\
\
\
**Example {% increment egnum %} - Convert JPG to TGA Respecting EXIF Orientation**
```
tacentview -c --in jpg --inJPG strct=true,exifo=true
```
Some JPG files have meta-data that includes, among other things, the orientation of the camera when the picture was taken. By specifying `exifo=true` we can undo this orientation automatically for JPG files. The resultant TGA will always be oriented correctly when we do this. The default is to not take the EXIF orientation into account. The `strct=true` load parameter means that any input JPG files must be well-defined and _strictly_ meet the JPeg specification. Setting this to false allows some non-conformant images to be loaded.\
\
\
**Example {% increment egnum %} - Convert DDS to TGA Specifying Load Parameters**
```
tacentview -c --inDDS corr=gamc,gamma=1.8,tone=2.1,spred=F,strct=T A.dds B.dds
```
This example shows loading two DDS files (`A.dds` and `B.dds`) setting all the supported input parameters. The `corr` parameter specifies the gamma-correction mode, the `gamma` parameter specifies 1.8 to be used for the correction gamma (default is 2.2). Assuming the DDS files are encoded with an HDR format like BC6s or one of the ASTC encodings, the `tone` parameter controls the exposure. The value of 2.1 pushes the exposure higher (brighter). The spread (`spred`) parameter applies to single-colour-channel DDS files. It means do not spread that channel to the other colour channels. Finally, `strct=T` means do not load DDS files that have errors or inconsistencies in their header -- the default is false which is a bit more forgiving for some DDS files found in the wild. Since no `--out` types were specified, TGA files will be generated from the two input DDS files.\
\
\
**Example {% increment egnum %} - Resize Images Preserving Aspect**
```
tacentview -cw . --op resize[1920,-1] -o jpg
```
Resizes all images in the current directory and saves them as JPG files. The JPG images will be 1920 pixels wide. The -1 for height means compute the height so that the original aspect ratio is preserved. The `-w` or `--overwrite` means overwrite existing files that may be present. Use with caution. There are more arguments to resize that allow setting resample filters and edge modes that are not shown here.\
\
\
**Example {% increment egnum %} - Resize Followed by Rotation**
```
tacentview -ca --op resize[1920,-1] --op rotate[5] --outTGA bpp=24,rle=true
```
Resizes and then applies a 5-degree anti-clockwise rotation to all images in the current directory. Saves them as TGA files. The `-a` or `--autoname` flag means if the output file exists the output filename will be modified so the existing file is not overwritten. The `--outTGA` params force a 24-bit run-length-encoded TGA to be written.\
\
\
**Example {% increment egnum %} - Set Pixel Colours**
```
tacentview -c --op pixel[2,1,cyan,RGB] --op pixel[-3,-2,#0000FF80,RGBA]
```
Sets the lower-left pixel (3rd to the right and 2nd up) to the colour cyan and leaves the alpha unchanged. Then sets the upper-right pixel (3rd to the left and 2nd down) to the colour blue with an alpha of 128. The pixel at (0,0) is the bottom-left. The pixel at (-1,-1) is the top-right of the current image. All images are saved as targas (the default).\
\
\
**Example {% increment egnum %} - Resize Canvas**
```
tacentview --cli --verbosity 2 --op canvas[640,400,mm,black]
```
Processes all images in the current directory and saves then as targas. The `--verbosity 2` means it will output the maximum amount of output text. All images are resized to 640x400 without resampling. If the new size is larger than the input, black bars are added. The anchor is set to the middle of the image. Further arguments not show allow the anchor to be set to a particular pixel position.\
\
\
**Example {% increment egnum %} - Set Aspect Ratio Without Fill**
```
tacentview -c --op aspect[19:10,crop,*,*,64,64]
```
Resizes image to an aspect ratio of 19:10. If the input image does not have the same aspect ratio, the sides are cropped so the resultant image takes up the full size. That is, some of the input image pixels get discarded to accomplish this without resampling. The anchor is the specific pixel position (64,64) where (0,0) is the bottom-left.\
\
\
**Example {% increment egnum %} - Set Aspect Ratio With Letterbox Fill**
```
tacentview -c --op aspect[2:1,letter,tr,green]
```
Resizes canvas to an aspect ratio of 2:1. No resampling is performed. In letterbox mode all source pixels are kept. If the input image has a different aspect ratio, green bars are added to the image. Since the anchor is the top-right, the green will be added to the left and bottom edges as necessary.\
\
\
**Example {% increment egnum %} - Remove Borders**
```
tacentview -ckv 2 --op deborder[yellow,RG]
```
Removes image borders by inspecting the rows and columns of all 4 sides. If both the red and green components are saturated (yellow), the border gets removed. Since we only check R and G component channels, the blue and alpha may be any value. The `-k` or `--skipunchanged` flag means if there would be no change to the input image, it is not saved -- even if the output image type is different than the input image type. The `-ckv 2` expands to `--cli --skipunchanged --verbosity 2` which makes it clear the 2 refers to the verbosity. The `-c` and `-k` are boolean flags and don't have arguments.\
\
\
**Example {% increment egnum %} - Crop**
```
tacentview -c --op crop[rel,20,10,320,200,*]
```
Crops all images in relative-mode. In this mode the crop is from the lower-left position (20,10) and the next two arguments specify the width (320) and height (200). The last argument * means use the default fill colour which is transparent black. This is used if the crop area extends beyond the image being processed. In absolute mode (abs), the 320,200 would be interpreted as the upper-right pixel position. In both modes the lower-left and upper-right positions are included in the final image. This example uses defaults for input types (all supported) and the output type (tga).\
\
\
**Example {% increment egnum %} - Flip**
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
**Example {% increment egnum %} - Rotate and Fill**
```
tacentview -c --op rotate[-30,fill,bilinear,bicubic_catmullrom,green]
```
The simplest rotation mode is fill. This example rotates the images clockwise by 30 degrees and does not crop. Imagine rotating an image clockwise a little and then placing a rectange around it. The bottom-left corner will be a bit farther left, the top-left a little higher, etc. This means the rotated image will be a bit larger than the original and 4 extra triangles will be present. The fill colour is used to fill those triangles (with green in this case). The up-filter (bilinear) is used to create a larger image internally so rotation quality is preserved. The down-filter (bicubic_catmullrom) is used to resample back down to the required size. In short, fill mode creates larger images than the original and preserves all image content. Supported filters include: nearest, box, bilinear, bicubic, bicubic_catmullrom, bicubic_mitchell, bicubic_cardinal, bicubic_bspline, lanczos_narrow, lanczos, and lanczos_wide.\
\
\
**Example {% increment egnum %} - Rotate and Crop**
```
tacentview -c --op rotate[-0.1r,crop,none]
```
Crop mode (the default) is similar to fill but instead of filling the extra area introduced by the rotation, it crops it out. The final image will have sightly smaller size than the original and no fill will be used. Not all original content is preserved. The `-0.1r` means use radians. Putting `none` for the up-filter means no resampling is performed (down-filter is ignored) and no new colours are introduced to the palette -- useful for rotating sprites or pixel-art.\
\
\
**Example {% increment egnum %} - Rotate and Resize**
```
tacentview -c --op rotate[12,resize,bilinear,none]
```
Rotates 12 degrees anti-clockwise. Resize mode does the same as crop mode but then resizes the image back to the original dimensions. Useful for rotating horizons in photos where you don't want to use a fill and want the final image size to be the same. Using `none` for the down-filter uses a custom down-sample method that keeps edges sharper.\
\
\
**Example {% increment egnum %} - Adjust Levels**
```
tacentview -c --op levels[0.1,*,0.9,*,*]
```
Adjusts image levels in a manner similar to other photo-editing software where you can specify black, mid, and white points. The first 3 areguments are the input black, mid, and white points in the 0 to 1 range. The * for the mid-point means put it halfway inbetween. The last two *'s are for the output black and white points and default to 0.0 and 1.0. There are further optional arguments to the levels operation (not shown in this example) that control which frame(s) to perform the operation on if the image is animated or has mipmaps, what colour channels to adjust, and what gamma algorithm to use. For a full explanation call `tacentview -c --help`.\
\
\
**Example {% increment egnum %} - Levels One Frame One Channel**
```
tacentview -c --op levels[0.2,0.6,0.8,0.1,0.9,4,R,false]
```
Adjusts image levels like the previous example except we specify all five black and white points explicitly. The 4 is the zero-based frame number to operate on if you have an animated or multi-page input image. If this is set to -1 or * it adjusts levels on all frames. The `R` means the red channel only. Choices are R, G, B, RGB (the default), and A. The last argument lets you specify a continuous or discontinuous function for the mid-point gamma algorithm. False means discontinuous at gamma 1 (approximates Photoshop). True means use a continuous base-10 power curve that smoothly transitions the full range (similar to the GIMP).\
\
\
**Example {% increment egnum %} - Contrast**
```
tacentview -ca --op contrast[0.75] -o gif --paramsJPG 90
```
Adjusts the contrast to 0.75 and saves the adjusted images as jpg files (auto-renaming them if they exist). A contrast value of 0.5 is neutral. Larger than 0.5 is more contrast up to a maximum of 1 (black and white). Lower than 0.5 is less contrast down to a minimum of 0 (all grey). Optional arguments not shown include the frame(s) to apply the adjustment to and which colour channels. The 90 is the quality setting from 1 to 100 for the jpg file.\
\
\
**Example {% increment egnum %} - Brightness**
```
tacentview -ca --op brightness[0.4] -o qoi --outQOI bpp=32,spc=srgb 
```
Adjusts the brightness to 0.4 and saves the adjusted images as qoi files (auto-renaming them if they exist). A brightness value of 0.5 is neutral. Larger than 0.5 is brighter up to a maximum of 1 (full white). Lower than 0.5 is dimmer down to a minimum of 0 (black). Optional arguments not shown include the frame(s) to apply the adjustment to and which colour channels. The `--outQOI` forces a 32-bit qoi to be generated in the sRGB colour-space.\
\
\
**Example {% increment egnum %} - Quantize using Wu Algorithm**
```
tacentview -cav 2 --op quantize[wu,256,true] -o qoi --outQOI bpp=24,spc=srgb
```
Quantizing is the (difficult) process of reducing the number of colours used in an image. There are a number of algorithms that may be applied to do this while keeping the image looking as good as possible. _Tacent View_ supports the Xiaolin Wu method (wu), a simple predefined palette (fix), a neural-net algorithm (neu), and a spatial quantization method (spc). In this example we use the default Wu method and reduce the number of colours to 256. The true means check for exactness -- when this check is on, if the number of colours currently in the image is already less-than or equal-to the number requested, the image is left unmodified. If false, the algorithm is run regardless (handy for forcing a black and white image with colours set to 2 while using 'fix' for the method). The output files are written as 24 bit qoi files in sRGB space. Verbosity is full and auto-rename is enabled.\
\
\
**Example {% increment egnum %} - Quantize using NeuQuant Algorithm**
```
tacentview -ca --op quantize[neu,256,true,2] -o qoi --outQOI bpp=24,spc=srgb
```
Similar to _Quantize using Wu Algorithm_ except using the NeuQuant neural-learning algorithm. When using this algorithm the 4th argument (2) is the sample-factor. This value affects the speed/quality ratio and must be between 1 and 30 (inclusive). 1 results in the highest quality by training on the full set of input pixels, but is slower than 30 which only uses 1/30 of the input pixels. The number of colours (256) must be between 2 and 256 inclusive. There is no requirement it be a power-of or divisible-by 2.\
\
\
**Example {% increment egnum %} - Quantize using ScolorQ Algorithm**
```
tacentview -ca --op quantize[spc,16,true,3,8.0] -o qoi --outQOI bpp=24,spc=srgb
```
Similar to _Quantize using Wu Algorithm_ except using the ScolorQ spatial quantization algorithm. When using this algorithm the 4th argument (3) is the filter-size and must be 1, 3, or 5. ScolorQ supports dithering -- the 5th argument (from 0.0 to 30.0) represents the amount of dither. A value of 0.1 is essentially no dither, while 20+ is a lot. If dither is set to 0.0 a good value is computed for you based on the image size and number of requested colours. ScolorQ is a good choice for small palette sizes (here we chose 16) but can be quite slow for larger images or palette sizes bigger than 30.\
\
\
**Example {% increment egnum %} - Set R and B Channels**
```
tacentview -ca --op channel[set,RB,#0F00A200]
```
This example will set all red channels in all input images to 15, and all blue channels to 162.\
\
\
**Example {% increment egnum %} - Blend Background into RGB Channels**
```
tacentview -ca --op channel[blend,RGBA,#80808AFF]
```
Blends a grey-blue backgraound into the RGB components of the input images. Each pixel's current alpha component is used to determine how much of the backgound to blend-in. Since alpha (A) is specified in the channels (2nd argument), the final alpha will be set to what is specified in the background colour (3rd argument) -- in this case FF (255). If alpha was not specified the blend still happens but the original alpha will remain unchanged in the output image.\
\
\
**Example {% increment egnum %} - Spread G Channel into R and B**
```
tacentview -ca --op channel[spread,G]
```
Spreads (copies) the green (G) channel of each pixel into the red and blue channels. Spread always takes in a single channel and spreads it to the RGB channels of the image.\
\
\
**Example {% increment egnum %} - Set Channels to Intensity**
```
tacentview -ca --op channel[intens,RG]
```
Computes the intensity of every pixel (always based on the RGB values) and sets the RG components to the computed intensity. You may choose any combination of RGBA channels to set. Use RGB to create a greyscale image with unchanged alpha.\
\
\
**Example {% increment egnum %} - Swizzle RGB to BGR**
```
tacentview -ca --op swizzle[BGR]
```
Swaps the red and the blue channels. In order, the new red channel gets the original blue channel, the green gets green, and the new blue channel gets red. This is the same as `swizzle[B*R]` and is also the same as `swizzle[B*R*]`. The asterisks just grab the corresponding original channel.\
\
\
**Example {% increment egnum %} - Swizzle Set Full Alpha**
```
tacentview -ca --op swizzle[RGB1]
```
Keeps the RGB channels the same and sets the alpha channel to full. The '1' means saturate -- since it is in the A position, alpha gets saturated.\
\
\
**Example {% increment egnum %} - Swizzle Clear G and B**
```
tacentview -ca --op swizzle[*00*]
```
Keeps the red and alpha channels the same and sets the green and blue to zero. The '0' means, well, zero -- since it is in the G and B positions, they get set to zero.\
\
\
**Example {% increment egnum %} - Swizzle Set RGB to G with Full Alpha**
```
tacentview -ca --op swizzle[GGG1]
```
There is no restriction on repeating colour components. That is, swizzle is not restricted to permutations. Here the original green channel is placed in the RGB channels and the alpha (usually interpreted as opacity) is set to full. This will generate a grey-scale image based only on the original green channel. If you want a grey-scale based on intensity, use the channel operation above.\
\
\
**Example {% increment egnum %} - Extract All Frames**
```
tacentview -ck --op extract -i apng -o bmp --outBMP bpp=auto
```
Extracts all frames from every APNG input file and saves them as BMP files. The `-k` means the original input images are not resaved (this operation does not modify the input images at all). The `auto` means auto-determine the bits-per-pixel for the BMP based on the image opacity. You can use the extract operation on any file that stores more than one image inside -- extract mipmaps from ktx2 files, extract pages from tiff files, extract animation frames from webp/gifs, etc. By default the output file names will be based on the input with _NNN appended. The NNN is the frame number. Unless you specify otherwise, the extracted images are placed in a subdirectory called 'Saved'.\
\
\
**Example {% increment egnum %} - Extract Specific Frames**
```
tacentview -ck --op extract[0-2+!4-6+!7-10!,FramesDir,FrameFile] -i gif InImage.gif
```
Extracts frames 0,1,2,5,6,8,9 from InImage.gif and saves them as TGA files in a directory called FramesDir. The images will be named *FrameFile_000.tga, FrameFile_001.tga, FrameFile_002.tga, FrameFile_005.tga*, etc. To specify the frames, an asterisk by itself means all, `0-2` means 0 to 2 (inclusive), the + is used to specify more ranges and the ! means exclude the end-point. The range `!7-10!` would be 8 and 9. A number by itself is just that number as a range so 11 would translate to `11-11`. Since -k is included, InImage.gif itself will _not_ be resaved as InImage.tga. If the input image has fewer frames than the ranges being processed, those frames can't and won't be saved because they don't exist. This is not considered an error. If FrameFile is * or is not entered, the base filename of the input image(s) is used and the 3-digit frame-number is still appended.\
\
\
**Example {% increment egnum %} - Create an Animated WEBP**
```
tacentview -ck -o webp --po combine
```
Combines multiple images into a single animated webp. This is an example of a _post-operation_. Post operations (`--po`) run after all normal operations. They run on the same set of original input images, in this case TGAs (the default input type). Only if a normal operation (`--op`) modifies an input image is the change included in the post operation. The `combine` post operation has a number of optional arguments not shown here for conciseness -- specifically the default frame duration is `33ms`, the webp is written to a folder called `Combined`, and the filename will take the form `Combined_YYYY-MM-DD-HH-MM-SS_NNN.webp` where NNN is the number of frames.\
\
\
**Example {% increment egnum %} - Create an Animated WEBP and APNG**
```
tacentview -ck -o webp,apng --po combine
```
Same as previous example but creates both a WEBP animated image and an APNG animated image from the same set of input images.\
\
\
**Example {% increment egnum %} - Create an Animated GIF**
```
tacentview -ck -o gif --po combine[0-19:20+20-39:100,ResultDir,AnimImage]
```
Creates an animated gif called `AnimImage.gif` from the input tga files. The gif will be placed in a directory called `ResultDir`. The first argument, `0-19:25+20-39:200` means the first 20 frames (`0-19`) will get a 25ms frame duration, and the next 20 (`20-39`) will get a 200ms frame duration. If there are more than 40 input images, the remainder will get the default 33ms frame duration. If there are only 30 input images, the first 20 will be at 25ms and the remaining 10 at 200ms. You may also leave out the frame-range before the colon in which case the duration applies to all frames. For example, `--po combine[100]` will set all frame durations to 100 milliseconds. The default gif save parameters support auto-detecting transparency and output an 8-bit palette using Wu colour quantization.\
\
\
**Example {% increment egnum %} - Create a Multipage TIFF**
```
tacentview -ck -o tif --po combine[0:1000+1:2000+2:3000] page1.qoi page2.qoi page3.qoi
```
Creates a multipage tiff called `Combined_YYYY-MM-DD-HH-MM-SS_003.tif` from the 3 input qoi files. The tiff will be placed in a directory called `Combined`. _Tacent View_ uses a tiff-tag (specifically TIFFTAG_SOFTWARE) to store additional information in each page of the tiff. In particular the page-duration is stored in this field while not affecting compatibility with other software. This allows the specified 1, 2 and 3 second frame durations to be stored with each tiff page. If this tiff is later loaded in _Tacent View_ it can be treated and played as an animated image. Use 16 milliseconds (`--po combine[16]`) to achieve 60 FPS.\
\
\
**Example {% increment egnum %} - Create a Flipbook TGA and PNG**
```
tacentview -ck -o tga,png --po contact[]
```
Creates two contact sheets called `Contact_YYYY-MM-DD-HH-MM-SS_NNxMM.tga` and `Contact_YYYY-MM-DD-HH-MM-SS_NNxMM.png` from the images in the current directory. The number of rows and columns (`NNxMM`) are computed for you based on the number of input images. The output tga and png files are placed in a directory called `Contact`. All input images must be the same dimension. You may use regular `--op` operations to resize first if desired.\
\
\
**Example {% increment egnum %} - Create a Flipbook QOI with 5 Columns**
```
tacentview -ckv 2 -o qoi --po contact[5,*,*,FlipOut,FiveColFile]
```
Creates a contact sheet called `FiveColFile.qoi` from the images in the current directory. The number of columns is set explicitly to 5. The rows will be computed for you so that all input images will be included. The output qoi file will be placed in a directory called `FlipOut` which will be created for you if it doesn't exist. All input images must be the same dimension. You may use regular `--op` operations to resize first if desired.\
\
\
**Example {% increment egnum %} - Create a 7x7 Flipbook JPG and BMP**
```
tacentview -ckv 2 -o jpg,bmp --po contact[7,7,white]
```
In this example we set both the number of columns and the number of rows. If there are more than 49 images in the input directory, only the first 49 will be included. If there are fewer than 49 input images, the contact-sheet (flipbook) will have some _empty_ pages. Empty pages get filled with opaque white. If white was not specified or * was used, the fill would be transparent black. Since JPG does not support alpha, the JPG output ignores transparency (it will be opaque). The BMP output supports alpha so transparency in both the pages and empty-page fill-colour is taken into account. The fill colour may be precisely specified in hexadecimal using #RRGGBBAA. If a single decimal integer is used instead, the fill is that value for all RGBA channels. The predefined colour names are: black, white, grey, red, green, blue, yellow, cyan, magenta, and trans (transparent black, the default). Colour names other than `trans` all default alpha to full (opaque).
