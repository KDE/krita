# Krita-QuickLook
QuickLook Generator plugin for .kra and .ora files.

This generator takes the preview.png image and uses it as the thumbnail image and the mergedimage.png file as the preview image. On files created with older versions of Krita that do not have the mergedimage.png file, QuickLook will simply fall back to using the thumbnail image instead.

# Installing
Place the generator in the `/Library/QuickLook` folder.

If you package Krita 3.0, you can include the generator file in the directory `krita.app/Contents/Library/QuickLook`. Be sure to reset QuickLook with `qlmanage -r` and `qlmanage -r cache`. If the changes don't happen right away, `killall Finder` and ensure that the app you put the generator in is the default app for opening .kra files.
