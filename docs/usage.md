
Tacent View 1.0.42 by Tristan Grimmer
Win x64 Release UTF-16 API
CLI Mode

USAGE: tacentview [options] [inputfiles] 

Options:
--autoname -a        : Autogenerate output file names
--cli -c             : Use command line mode (required when using CLI)
--earlyexit -e       : Early exit / no skipping
--examples -x        : Print examples
--help -h            : Print help/usage information
--in -i arg1         : Input file type(s)
--inASTC arg1        : Load parameters for ASTC files
--inDDS arg1         : Load parameters for DDS files
--inEXR arg1         : Load parameters for EXR files
--inHDR arg1         : Load parameters for HDR files
--inJPG arg1         : Load parameters for JPG files
--inKTX arg1         : Load parameters for KTX files
--inPKM arg1         : Load parameters for PKM files
--inPNG arg1         : Load parameters for PNG files
--markdown -m        : Print examples in markdown
--op arg1            : Operation
--out -o arg1        : Output file type(s)
--outAPNG arg1       : Save parameters for APNG files
--outBMP arg1        : Save parameters for BMP  files
--outGIF arg1        : Save parameters for GIF  files
--outJPG arg1        : Save parameters for JPG  files
--outPNG arg1        : Save parameters for PNG  files
--outQOI arg1        : Save parameters for QOI  files
--outTGA arg1        : Save parameters for TGA  files
--outTIFF arg1       : Save parameters for TIFF files
--outWEBP arg1       : Save parameters for WEBP files
--outname -n arg1    : Output file name modifications
--overwrite -w       : Overwrite existing output files
--po arg1            : Post operation
--profile -p arg1    : Launch GUI with the specified profile active.
--skipunchanged -k   : Don't save unchanged files
--syntax -s          : Print syntax help
--verbosity -v arg1  : Verbosity from 0 to 2

Parameters:
[inputfiles]         : Input image files

MODE
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
are 'main', 'basic', and 'kiosk'.

When launched with --profile the active profile is not remembered on exit.
Changes made to the profile are, however, persistent. Changes can include
anything from what dialogs are open, zoom mode, slideshow and background
settings, etc.

e.g. 'tacentview -p kiosk C:/Slidehow/' will start a slidewhow of all the
images inside the 'SlideShow' directory. By default the kiosk profile
auto-starts the slideshow in shuffle mode.

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

Supported input file types: tga png jpg gif webp qoi dds pvr ktx ktx2 astc pkm 
exr hdr apng bmp ico tif 
These cover file extensions: tga png jpg jpeg gif webp qoi dds pvr ktx ktx2 
astc atc pkm exr hdr rgbe apng bmp ico tif tiff 

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

OUTPUT IMAGES
-------------
The output files are generated based on the input files and chosen operations.
Each input image may generate one or more output images based on what the output
types are set to. For example, you might specify that both webp and bmp images
should be saved. The output types are specified with --out followed by a comma-
separated list of types. The short version -o may also be used. If no out type
is specified the default is tga. You may have multiple -o arguments or just a
single one. For example, '-o webp,bmp' is the same as '-o webp -o bmp'.

Supported output file types: tga png jpg gif webp qoi apng bmp tif 

The output filename matches the input filename except that the extension/type
may be different. Eg. Seascape.jpg would save as Seascape.tga if the out type
was tga only.

The output names may be modified by adding a prefix, suffix, or replacing part
of the name with a different string. This is done using --outname or -n.

--outname prefix=string,suffix=string,replace=old:new

for example, if suffix=_Blockout,replace=Full|Thumb and the input file was
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

EXIT CODE
---------
The return error code is 0 for success and 1 for failure. For 0 to be returned
every specified image must be successfully loaded, processed, and saved. A
failure in any step for any image results in an error. By default processing
continues to the next image even on a failure. If the --earlyexit (-e) flag is
set, processing stops immediately on any failure. Either way, any failure
returns a non-zero exit code.
