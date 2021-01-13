![Build](https://github.com/bluescan/tacentview/workflows/Build/badge.svg) [![latest](https://img.shields.io/github/v/release/bluescan/tacentview.svg)](https://github.com/bluescan/tacentview/releases) [![tacentview](https://snapcraft.io//tacentview/badge.svg)](https://snapcraft.io/tacentview)

# tacent view
An image and texture viewer for tga, png, apng, exr, dds, gif, hdr, jpg, tif, ico, webp, and bmp files. Uses Dear ImGui and the Tacent library. This viewer is targeted to game devs -- It displays alpha channels correctly and can inspect/edit the colour of individual pixels. There is basic editing functionality, batch resizing, rotations/flips, and the ability to generate contact sheets for FX etc. Tacent View can save tga, png, bmp, jpg, (animated) webp, and (animated) gif files.

Tacent View runs on Windows 10 and Ubuntu Linux. Browse the releases to grab a zip (Windows) or deb (Linux) package -- or install from the [Snap Store](https://snapcraft.io/tacentview).

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_CopyColourAs.png)


Left/right direction keys show previous and next images, semi-transparent images are displayed with an optional checkerboard background, and images may be converted to different formats easily.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_SaveAs.png)


Use 'Save All As..." to batch convert all the images in a folder.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_BatchSaveAll.png)


Viewing as thumbnails is supported by the 'Content View' window. Thumbnail generation and cache retrieval are extremely fast. Tacent View can easily handle thousands of photos in a single folder.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Thumbnails.png)


Contact sheet (AKA flipbook) textures may be generated. Images may be 'played' in succession to see what they look like animated. The alpha-channel is interpreted as opacity and is properly processed if the source images have semi-transparency.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_ContactSheet.png) 


Flip, rotate-by-90-degrees, rotate-arbitrarily (with live preview), cropping, and viewing textures in tiled mode is supported. There are also some powerful tools to resize images either by resampling or adjusting the canvas size. The 'aspect' mode of resize-canvas is useful for changing a photo's aspect ratio without any non-uniform scale. You get to set the anchor and choose whether you want the image letter-boxed or cropped. There is also a border removal tool that finds all matching pixels around the edges of an image and crops them.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Crop.png)

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Tiled.png)


When viewing dds (direct draw surface) files, you can view any present mipmaps and see cubemaps in a 'T' layout.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Cubemap.png)

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Mipmaps.png)


Users may zoom using Ctrl+/- and pan using the right mouse button. Several zoom modes are availabe in the view-menu including Fit, Downscale, and One-to-One pixel mapping. On extreme zooms the selected pixel is drawn with an outline.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Zoom.png)


High Definition Image loading is supported for exr and hdr files. For hdr files the loading code is based on, and is functionally equivalent to, the reference code from the Radiance imaging toolset. Adjustments to gamma correction and exposure are supported when loading radiance hdr files. The images below (Copyright Rafal Mantiuk under Creative Commons 3.0) show an hdr image that loads over-exposed and is subsequentially adjusted to yield a more balanced result.

![HDR Loading](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_HDR.png)


EXR support also uses reference code from OpenEXR and should be able to load most exr files, including multi-frame (AKA multi-part) files. There is more control for exr parameters including the ability to de-fog. The screenshot below ('desk' image courtesy OpenEXR and under their licence) shows similar successive exposure adjustments.

![EXR Loading](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_EXR.png)


Other formats store multiple images inside a single file. A dds may be a cubemap with 6 sides or a BC compressed image with mipmaps. A tiff may also include multiple 'pages'. Webp and png/apng files may be animated. For these formats the property-editor window allows you to display which frame you see. The hypercube below (courtesy [Jason Hise](https://blog.wikimedia.org/2016/09/22/math-gifs/) under CC0) shows the options and play controls for a multi-frame animated gif.

![AnimGif](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Anim.png)


Alt-Enter toggles fullscreen mode. In this mode the bottom navigation bar and top menu are invisible. If at any point you want to reset the UI or other settings like the default resample algorithm or preferred file save format, you may do so from the preferences dialog.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Prefs.png)

The 'transparent work area' option in the preferences allows you to see right through to your desktop -- interpreting the alpha channel in the image, if present, as opacity. If you set the viewer to basic-mode (B-key, no UI widgets), and enter fullscreen mode (Alt-Enter), you can do things like play animated semitransparent webp file right over your desktop. The screenshot below shows transparent-work-area turned on while in windowed (non-fullscreen) mode. This works in both Linux and Windows.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_TransparentWorkArea.png)

# building from source

For convenience pre-built binaries are available for Windows and Ubuntu/Debian in the Releases section. Other Linuxes can use the [Snap Store Release](https://snapcraft.io/tacentview). Follow the instructions below to build from source.

## windows

Building on Windows is tested with the Visual Studio 2019 MSVC compiler (choose the C++ workflow when installing). You will need to install git. Optionally you may want to use VSCode with the CMake Tools extension to build. To build from the Visual Studio x64 Command Prompt:

```
mkdir build
cd build
cmake .. -G"Visual Studio 16 2019" -Ax64
cmake --build . --config Release --target install
```

The install target will also package a zip file with all required content. This is done by an install step using a powershell script.

## ubuntu

Building for Ubuntu is tested with Clang (9 or 10) and GCC 9.3. CMake and Git are required. Ninja is suggested but optional. The 'Software and Updates' app allows you to manage (add and remove) repositories or use the command line:
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
* Gif Load
* Ico Load
* Ion Icons
* LibJPeg-Turbo
* OpenEXR
* Roboto Font
* WebP Reference Library
* LibTIFF
* ZLib
* This software is based in part on the work of the Independent JPEG Group.
* This software includes [Radiance software](http://radsite.lbl.gov/) developed by the [Lawrence Berkeley National Laboratory](http://www.lbl.gov/).

Various development tools available for open source projects bear mentioning:
* CMake
* Ubuntu
* VS Code Editor
* Visual Studio Community
* GCC, Clang, and the MSVC Compiler
* Github
* Git and the SmartGit Client
* OpenGL, GLAD, and GLFW
