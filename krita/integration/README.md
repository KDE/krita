# Krita's macOS integration

This Xcode project generates QuickLook (macOS < 10.15), QuickLook Thumbnailing
(macOS 10.15+) and Spotlight plugins for .kra and .ora files.

The QuickLook plugins take the `preview.png` image in the root of the ZIP
container and use it as the thumbnail image, and the `mergedimage.png` file as
the preview image. On files created with older versions of Krita that do not
have `mergedimage.png`, QuickLook will simply fall back to using the thumbnail
image instead.

The Spotlight plugin extracts the following metadata from the kra file, if
available:
- image dimensions, DPI, bit depth, and color space
- color space profile name (not the actual name, as it's not embedded in the container; only the file path)
- layer names
- authors, image title and description

# Installing

Compile the project using Xcode or the provided CMake file. Find the build
output folder (depends on how you configured the project), and place:
- `kritaquicklook.qlgenerator` inside `~/Library/QuickLook`
- `kritaspotlight.mdimporter` inside `~/Library/Spotlight`.

You can also place them in the system folders (`/Library/QuickLook` and
`/Library/Spotlight` respectively). Test that they have been properly installed
with:
- QuickLook: `qlmanage -d2 -p <path to a kra file>; qlmanage -d2 -t <path to a kra file>`
- Spotlight: `mdimport -d2 -t <path to a kra file>`

Be sure to reset QuickLook with `qlmanage -r` and `qlmanage -r cache`. If the
changes don't happen right away, `killall Finder`.

**NOTE 1:** `kritaquicklookng.appex` **cannot be used standalone. You must bundle it with Krita (see below).**

**NOTE 2:** If you have krita installed to `/Applications`, Quicklook will always find
first `kritaquicklook.qlgenerator` from `/Applications/krita.app` application
bundle.

# Bundling

If you package Krita, place:
- `kritaquicklook.qlgenerator` inside `krita.app/Contents/Library/QuickLook`
- `kritaspotlight.mdimporter` inside `krita.app/Contents/Library/Spotlight`
- `kritaquicklookng.appex` inside `krita.app/Contents/Library/PlugIns`
Ensure the app is defined as the default app for opening .kra and .ora files,
and codesign it whole if necessary.

# Hacking
After applying changes and compiling, test your changes as instructed below.
- Quicklook: run in terminal: `qlmanage -g <path to kritaquicklook.qlgenerator> -c image -d2 -p <path to a kra file>`
- Spotlight: install `kritaspotlight.mdimporter` inside `~/Library/Spotlight` and 
clear caches, then run `mdimport -d2 -t <path to a kra file>`. Installed 
mdimporter will be first in the list when running `mdimport -L`.