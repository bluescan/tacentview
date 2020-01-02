# tacit-texview
A texture viewer for tga, jpg, png, tiff, dds, gif, and bmp files. Uses Dear IMGUI, OpenGL, and the Tacent library. This viewer is useful for game devs as it displays important information like the presence of an alpha channel and querying specific pixels for their colour.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_CopyColourAs.png)

Left/right direction keys load previous and next images, semitransparent images are displayed with an optional checkerboard background, and you can convert images to multiple formats easily.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_SaveAs.png)

Support for viewing dds files is implemented -- you can view all mipmaps if they are present, and see cubemaps in a 'T' layout.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Cubemap.png)

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Mipmaps.png)

Zooming (Ctrl+ and Ctrl-) as well as panning using the right mouse button is supported. There are a number of different zoom modes availabe in the View menu item including Fit, Downscaling, and One-to-One pixel sizing.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_Zoom.png)

Hit Alt-Enter to go into fullscreen mode. In this mode the log window, menubar, and info-overlay are invisible.

Tacit-texview can be used to generate contact sheets for flip-book animation. If any of the source images have non-unity alpha (are not fully opaque), the generated contact sheet will have an alpha channel (32-bit instead of a 24-bit tga). Empty frames are filled with black (with an opacity/alpha of 0 for 32-bit sheets). The resample filter, if needed, can be chosen and the contact sheet can be saved in multiple file formats.

![Tacent Texture Viewer](https://raw.githubusercontent.com/bluescan/tacit-texview/master/Screenshots/Screenshot_ContactSheet.png)
 
 Finally, obligatory flip and rotate-by-90-degrees buttons are available. Tacit-TexView will not auto-save -- you must save explicitly. Note that any time you perform a save or generate a contact-sheet, if the file already exists it will not be overwritten. Select View->ShowLog to see an activity log of viewer operations and their status.
 
