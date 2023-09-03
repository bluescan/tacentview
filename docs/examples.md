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
This example shows loading two DDS files (`A.dds` and `B.dds`) setting all the supported input parameters. The `corr` parameter specifies the gamma-correction mode, the `gamma` parameter specifies 1.8 to be used for the correction gamma (default is 2.2). Assuming the DDS files are encoded with an HDR format like BC6s or one of the ASTC encodings, the `tone` parameter controls the exposure. The value of 2.1 pushes the exposure higher (brighter). The spread (`spred`) parameter applies to single-colour-channel DDS files. It means do not spread that channel to the other colour channels. Finally, `strct=T` means do not load DDS files that have errors or inconsistencies in their header -- the default is false which is a bit more forgiving for some DDS files found in the wild. Since no `--out` types were specified, TGA files will be generated from the two input DDS files.
