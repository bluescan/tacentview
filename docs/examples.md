
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
This example shows why the `-c` option is needed -- without it GUI-mode is activated and the images on the command-line are opened in the full graphical user interface. Note that some input types have multiple valid extensions. e.g. A JPeg may have `jpg` or `jpeg` extensions and a Tiff may have `tif` or `tiff`.


