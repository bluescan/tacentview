# tacit-texview
A texture viewer for tga, jpg, png, tif, dds, gif, and bmp files. Uses Dear ImGui, OpenGL, CxImage, and the Tacent library. This viewer is targeted to game devs. It displays useful details like the presence of an alpha channel and can query individual pixels for their colour.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_CopyColourAs.png)


Left/right direction keys show previous and next images, semi-transparent images are displayed with an optional checkerboard background, and images may be converted to different formats easily.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_SaveAs.png)


Use 'Save All As..." to batch convert all the images in a folder.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_BatchSaveAll.png)


Viewing as thumbnails is supported by the 'Content View' window. Thumbnail generation and cache retrieval are extremely fast. TexView can easily handle thousands of photos in a single folder.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Thumbnails.png)


Contact sheet (AKA flipbook) textures may be easily generated. TexView is fast enough to 'play' the images in succession. The alpha-channel is interpreted as opacity and is properly saved if any of the source images have semi-transparency.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_ContactSheet.png) 


Flip, rotate-by-90-degrees, cropping and viewing textures in tiled mode is supported.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Crop.png)

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Tiled.png)


When viewing dds (direct draw surface) files, you can view any present mipmaps and see cubemaps in a 'T' layout.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Cubemap.png)

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Mipmaps.png)


Zooming (Ctrl+ and Ctrl-) as well as panning using the right mouse button is supported. There are a number of different zoom modes availabe in the View menu item including Fit, Downscaling, and One-to-One pixel sizing. When zoomed-in far enough, the selected pixel gets an outline.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Zoom.png)


Alt-Enter toggles fullscreen mode. In this mode the bottom navigation bar, top menu, and details-overlay are invisible. If at any point you want to reset the UI or other settings like the default resample algorithm or preferred file save format, you may do so from the preferences dialog.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Prefs.png)
