# tacit-texview
A texture viewer for tga, jpg, png, tiff, dds, gif, and bmp files. Uses Dear IMGUI, OpenGL, and the Tacent library. This viewer is useful for game devs as it displays important information like the presence of an alpha channel and querying specific pixels for their colour.

Left/right direction keys load previous and next images, semitransparent images are displayed with a checkerboard background, and you can convert images to multiple formats easily. Support for viewing dds files is implemented -- the top mipmap level and first face of cubemaps is displayed.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/TextureViewerScreenshot.png)

Zooming (Ctrl+ and Ctrl-) as well as panning using the right mouse button is supported. There are a number of different zoom modes availabe in the View menu item including Fit, Downscaling, and One-to-One pixel sizing.

Hit Alt-Enter to go into fullscreen mode. In this mode the log window, menubar, and info-overlay are invisible.

Tacit-texview can be used to generate contact sheets for flip-book animation. If any of the source images have non-unity alpha (are not fully opaque), the generated tga contact sheet will have an alpha channel (32-bit instead of a 24-bit tga). Empty frames are filled with black (with an opacity/alpha of 0 for 32-bit sheets).
