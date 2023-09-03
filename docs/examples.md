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
Uses the @ symbol to specify a manifest file that lists the files to process. The contents of _manifest.txt_ could look something like this. ``` ; Some individual files to process. Flipbook.webp Flame.apng  ; A directory to process. MoreImages/ ``` The `--outGIF` is optional and specifies any non-default parameters for creating the GIFs. In this case a 2-bit (4 colour) palette is used, the _neu_ algorithm is used for colour quantization, and the transparency threshold is 120. There are more options for GIF output not shown here including things like loop behaviour, frame duration, etc.
