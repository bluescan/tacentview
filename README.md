![Build](https://github.com/bluescan/tacentview/workflows/Build/badge.svg) [![latest](https://img.shields.io/github/v/release/bluescan/tacentview.svg)](https://github.com/bluescan/tacentview/releases) [![tacentview](https://snapcraft.io//tacentview/badge.svg)](https://snapcraft.io/tacentview)

# tacent view
An image and texture viewer for tga, png, apng, exr, dds, ktx, ktx2, astc, pkm, qoi, gif, hdr, jpg, tiff, ico, webp, and bmp files. Built on DearImGui and the Tacent library, the viewer has robust support for pixel-formats used during game/mobile development including BC1-7, ASTC and ETC. Editing functionality includes batch-resizing, rotations/flips, contact-sheet/flipbook generation, cropping, levels adjustments, etc. Tacent View can save tga, png, bmp, qoi, jpg, (animated) webp, (animated) gif, (animated) apng, and (animated) tiff files.

As a photo viewer it can handle large collections and generate/retrieve thumbnails very quickly. The slideshow mode can display photos in any order, including fields stored in the EXIF meta-data of jpeg files (like latitude, or aperture, etc).

Tacent View may also be used as a command-line tool for batch processing of images or integration into build-pipelines. All operations that can be done in the GUI are also available from the command-line -- colour quantization, crop, rotate, frame-extraction, resize, etc.

These features are described in more detail on the [Tacent View Homepage](https://bluescan.github.io/tacentview).

Tacent View runs on Windows (10 or 11) and Linux (Kubuntu etc). Browse the releases to grab a zip (Windows) or deb (Linux) package -- or install from the [Snap Store](https://snapcraft.io/tacentview).

Here is an example of adjusting levels using Tacent View. All frames of animated images can be adjusted at once, or each frame can be done individually. All operations support a per-image undo/redo stack.
![Tacent View Levels](https://user-images.githubusercontent.com/19311462/216225745-0ea91c61-6b07-40fc-a1b7-bd327cdded4c.gif)

And grabbling the exact pixel colour value to the clipboard.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_CopyColourAs.png)

Left/right direction keys show previous and next images, semi-transparent images are displayed with an optional checkerboard background, and images may be converted to different formats easily.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_SaveAs.png)

Use 'Save All As..." to batch convert all the images in a folder.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_BatchSaveAll.png)

Viewing as thumbnails is supported by the 'Thumbnail View' window. Thumbnail generation and cache retrieval are extremely fast. Tacent View can easily handle thousands of photos in a single folder.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Thumbnails.png)


Contact sheet (AKA flipbook) textures may be generated. Images may be 'played' in succession to see what they look like animated. The alpha-channel is interpreted as opacity and is properly processed if the source images have semi-transparency.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_ContactSheet.png) 


Flip, rotate-by-90-degrees, rotate-arbitrarily (with live preview), cropping, and viewing textures in tiled mode is supported. There are also some powerful tools to resize images either by resampling or adjusting the canvas size. The 'aspect' mode in resize-canvas is useful for changing a photo's aspect ratio without any non-uniform scale. You get to set the anchor and choose whether you want the image letter-boxed or cropped. The border removal tool finds all matching pixels around the edges of an image and crops them. All edit operations are backed by a full undo (ctrl-z) and redo (ctrl-y) stack.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Crop.png)

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Tiled.png)


When viewing dds, ktx, or ktx2 files, you can view any present mipmaps and see cubemaps in a 'T' layout.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Cubemap.png)

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Mipmaps.png)


Users may zoom using Ctrl+/- and pan using the right mouse button. Several zoom modes are availabe in the view-menu including Fit, Downscale, and One-to-One pixel mapping. On extreme zooms the selected pixel is drawn with an outline.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Zoom.png)


High Definition Image loading is supported for exr, hdr (rgbe), dds, ktx and ktx2 files. For hdr/rgbe files the loading code is based on, and is functionally equivalent to, the reference code from the Radiance imaging toolset. Adjustments to gamma correction and exposure are supported when loading any HDR file. The images below (Copyright Rafal Mantiuk under Creative Commons 3.0) show an image that loads over-exposed and is subsequentially adjusted to yield a more balanced result.
![HDR Loading](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_HDR.png)


EXR support uses reference code from OpenEXR and should be able to load most exr files, including multi-frame (AKA multi-part) files. There is control for various exr parameters including the ability to de-fog. The screenshot below ('desk' image courtesy OpenEXR and under their licence) shows similar successive exposure adjustments.
![EXR Loading](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_EXR.png)


Other formats store multiple images inside a single file. A dds or ktx may be a cubemap with 6 sides or a BC/ASTC compressed image with mipmaps. A tiff may also include multiple 'pages'. Webp and png/apng files may be animated. For these formats the property-editor window allows you to display which frame you see. The hypercube below (courtesy [Jason Hise](https://blog.wikimedia.org/2016/09/22/math-gifs/) under CC0) shows the options and play controls for a multi-frame animated gif. In Tacent View the duration of any individual frame may be edited and the file resaved. This works whether a gif, apng or webp is being edited.
![AnimGif](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Anim.png)


Alt-Enter toggles fullscreen mode. In this mode the bottom navigation bar and top menu are invisible. If at any point you want to reset the UI or other settings like the default resample algorithm or preferred file save format, you may do so from the preferences dialog.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Prefs.png)

The 'transparent work area' option in the preferences allows you to see right through to your desktop -- interpreting the alpha channel in the image, if present, as opacity. If you set the viewer to basic-mode (B-key, no UI widgets), and enter fullscreen mode (Alt-Enter), you can do things like play animated semitransparent webp file right over your desktop. The screenshot below shows transparent-work-area turned on while in windowed (non-fullscreen) mode. This works in both Linux and Windows. For Linux, the snap system disables this feature so you will need to install the deb file or build from source.
![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_TransparentWorkArea.png)

# building from source

For convenience pre-built binaries are available for Windows and Ubuntu/Debian in the Releases section. Other Linuxes can use the [Snap Store Release](https://snapcraft.io/tacentview). Follow the instructions below to build from source.

## windows

Building on Windows is tested with the Visual Studio 2022 MSVC compiler (choose the C++ workflow when installing). You will need to install git. Optionally you may want to use VSCode with the CMake Tools extension to build. To build from the Visual Studio x64 Command Prompt:

```
mkdir build
cd build
cmake .. -G"Visual Studio 17 2022" -Ax64
cmake --build . --config Release --target install
```

The install target will also package a zip file with all required content. This is done by an install step using a powershell script.

## ubuntu

Building for Ubuntu is tested with Clang 13 and GCC 11.2 (and is known to work with more recent releases). CMake and Git are required. Ninja is suggested but optional. The 'Software and Updates' app allows you to manage (add and remove) repositories or use the command line:
```
sudo apt-get install gdebi               # Installs gdebi to allow command-line deb file installation. Optional.
sudo apt-get install git                 # Install git or download the source as a zip.
sudo gdebi Downloads/smartgit-19_1_7.deb # Install smartgit or some other git frontend. Optional.
sudo apt-get install llvm                # The compiler. Optional if not using GCC.
sudo apt-get install clang               # The compiler. Optional if not using GCC.
sudo apt-get install lldb                # The debugger. Optional if not using GCC.
sudo apt-get install cmake               # CMake.
sudo apt-get install ninja-build         # Ninja build system.
sudo update-alternatives --config c++    # Choose clang. Optional if not using GCC.
sudo update-alternatives --config cc     # Choose clang. Optional if not using GCC.
```

I usually use VSCode with the CMake Tools to build. Alternatively from the command line:
```
mkdir ninjabuild
cd ninjabuild
cmake .. -GNinja
ninja install
```
You can pass -DCMAKE_BUILD_TYPE=Debug (or Release) to cmake if so desired. The install target places the viewer into a ViewerInstall directory along with any data resources it needs. Install is also leveraged to build packages. If you configure and build with:
```
cmake .. -GNinja -DPACKAGE_DEB=True
ninja install
```

Then a deb file with all required content will be generated.

## Credit and Thanks

This project relies on myriad 3rd-party libraries. In the Data folder you will find their licences. Some of the more notable dependencies are listed here:
* Dear ImGui
* GLAD
* GLFW
* Ico Load
* Ion Icons
* Roboto Font
* LibJPeg-Turbo
* LibTIFF
* OpenEXR
* ZLib
* GIF Load
* gifenc
* WebP Reference Library
* LibPNG
* LibKTX (KTX-Software)
* APNGDis and APNGAsm
* TinyXML2
* TinyEXIF
* BCDec from iOrange
* QOI from phoboslab
* ASTCDec from ARM
* This software is based in part on the work of the Independent JPEG Group.
* This software includes [Radiance software](http://radsite.lbl.gov/) developed by the [Lawrence Berkeley National Laboratory](http://www.lbl.gov/).

Various development tools available for open source projects bear mentioning:
* CMake
* Kubuntu
* VS Code Editor
* Visual Studio Community
* GCC, Clang, and the MSVC Compiler
* Github
* Git and the SmartGit Client
* OpenGL, GLAD, and GLFW
* NVidia Texture Tools Exporter
