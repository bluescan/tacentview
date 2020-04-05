![Build](https://github.com/bluescan/tacit-texview/workflows/Build/badge.svg)

# tacit-texview
A texture viewer for tga, png, exr, dds, gif, hdr, jpg, tif, and bmp files. Uses Dear ImGui and the Tacent library. This viewer is targeted to game devs -- It displays alpha channels correctly and can inspect the colour of individual pixels. There is basic editing functionality and the ability to generate contact sheets for FX etc.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_CopyColourAs.png)


Left/right direction keys show previous and next images, semi-transparent images are displayed with an optional checkerboard background, and images may be converted to different formats easily.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_SaveAs.png)


Use 'Save All As..." to batch convert all the images in a folder.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_BatchSaveAll.png)


Viewing as thumbnails is supported by the 'Content View' window. Thumbnail generation and cache retrieval are extremely fast. TexView can easily handle thousands of photos in a single folder.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Thumbnails.png)


Contact sheet (AKA flipbook) textures may be generated. Images may be 'played' in succession to see what they look like animated. The alpha-channel is interpreted as opacity and is properly processed if the source images have semi-transparency.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_ContactSheet.png) 


Flip, rotate-by-90-degrees, cropping and viewing textures in tiled mode is supported.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Crop.png)

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Tiled.png)


When viewing dds (direct draw surface) files, you can view any present mipmaps and see cubemaps in a 'T' layout.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Cubemap.png)

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Mipmaps.png)


Users may zoom using Ctrl+/- and pan using the right mouse button. Several zoom modes are availabe in the view-menu including Fit, Downscale, and One-to-One pixel mapping. On extreme zooms the selected pixel is drawn with an outline.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Zoom.png)


High Definition Image loading is supported for exr and hdr files. For hdr files the loading code is based on, and is functionally equivalent to, the reference code from the Radiance imaging toolset (texview includes [Radiance software](http://radsite.lbl.gov/) developed by the [Lawrence Berkeley National Laboratory](http://www.lbl.gov/).) Adjustments to gamma correction and exposure are supported when loading radiance hdr files. The images below (Copyright Rafal Mantiuk under Creative Commons 3.0) show an hdr image that loads over-exposed and is subsequentially adjusted to yield a more balanced result.

![HDR Loading](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_HDR.png)


EXR support also uses reference code from OpenEXR and should be able to load most exr files, including multi-part files. There is more control for exr parameters including the ability to de-fog. The screenshot below ('desk' image courtesy OpenEXR and under their licence) shows similar successive exposure adjustments.

![EXR Loading](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_EXR.png)


Some formats store multiple images inside a single file. A dds for example may be a cubemap with 6 sides or a BC compressed image with mipmaps. The property-editor window allows you to display which image/part you see. The hypercube below (courtesy [Jason Hise](https://blog.wikimedia.org/2016/09/22/math-gifs/) under CC0) shows the options and play controls for a multi-part animated gif.

![AnimGif](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Anim.png)


Alt-Enter toggles fullscreen mode. In this mode the bottom navigation bar, top menu, property editor window, and details-overlay are invisible. If at any point you want to reset the UI or other settings like the default resample algorithm or preferred file save format, you may do so from the preferences dialog.

![Tacit Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Prefs.png)

