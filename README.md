![Build](https://github.com/bluescan/tacentview/workflows/Build/badge.svg)

# tacent view
An image and texture viewer for tga, png, exr, dds, gif, hdr, jpg, tif, ico, webp, and bmp files. Uses Dear ImGui and the Tacent library. This viewer is targeted to game devs -- It displays alpha channels correctly and can inspect the colour of individual pixels. There is basic editing functionality, batch resizing, and the ability to generate contact sheets for FX etc.

Tacent View runs on both Windows 10 and Ubuntu Linux.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_CopyColourAs.png)


Left/right direction keys show previous and next images, semi-transparent images are displayed with an optional checkerboard background, and images may be converted to different formats easily.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_SaveAs.png)


Use 'Save All As..." to batch convert all the images in a folder.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_BatchSaveAll.png)


Viewing as thumbnails is supported by the 'Content View' window. Thumbnail generation and cache retrieval are extremely fast. Tacent View can easily handle thousands of photos in a single folder.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Thumbnails.png)


Contact sheet (AKA flipbook) textures may be generated. Images may be 'played' in succession to see what they look like animated. The alpha-channel is interpreted as opacity and is properly processed if the source images have semi-transparency.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_ContactSheet.png) 


Flip, rotate-by-90-degrees, cropping and viewing textures in tiled mode is supported.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Crop.png)

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Tiled.png)


When viewing dds (direct draw surface) files, you can view any present mipmaps and see cubemaps in a 'T' layout.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Cubemap.png)

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Mipmaps.png)


Users may zoom using Ctrl+/- and pan using the right mouse button. Several zoom modes are availabe in the view-menu including Fit, Downscale, and One-to-One pixel mapping. On extreme zooms the selected pixel is drawn with an outline.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Zoom.png)


High Definition Image loading is supported for exr and hdr files. For hdr files the loading code is based on, and is functionally equivalent to, the reference code from the Radiance imaging toolset (Tacent View includes [Radiance software](http://radsite.lbl.gov/) developed by the [Lawrence Berkeley National Laboratory](http://www.lbl.gov/).) Adjustments to gamma correction and exposure are supported when loading radiance hdr files. The images below (Copyright Rafal Mantiuk under Creative Commons 3.0) show an hdr image that loads over-exposed and is subsequentially adjusted to yield a more balanced result.

![HDR Loading](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_HDR.png)


EXR support also uses reference code from OpenEXR and should be able to load most exr files, including multi-part files. There is more control for exr parameters including the ability to de-fog. The screenshot below ('desk' image courtesy OpenEXR and under their licence) shows similar successive exposure adjustments.

![EXR Loading](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_EXR.png)


Some formats store multiple images inside a single file. A dds for example may be a cubemap with 6 sides or a BC compressed image with mipmaps. The property-editor window allows you to display which image/part you see. The hypercube below (courtesy [Jason Hise](https://blog.wikimedia.org/2016/09/22/math-gifs/) under CC0) shows the options and play controls for a multi-part animated gif.

![AnimGif](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Anim.png)


Alt-Enter toggles fullscreen mode. In this mode the bottom navigation bar and top menu are invisible. If at any point you want to reset the UI or other settings like the default resample algorithm or preferred file save format, you may do so from the preferences dialog.

![Tacent View](https://raw.githubusercontent.com/bluescan/tacentview/master/Screenshots/Screenshot_Prefs.png)

# building from source

For convenience pre-built binaries are available for Windows and Ubuntu/Debian in the Releases section. Follow the instructions below to build from source.

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

The install target packages a deb file with all required content. You can pass -DCMAKE_BUILD_TYPE=Debug (or Release) to cmake if so desired.
