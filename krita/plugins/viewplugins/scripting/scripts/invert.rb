# Ruby invert script.
#
# The ruby invert script inverts all pixels at the activate layer in
# the current image.
#
# Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
# Published under the GNU GPL >=v2

# import the Krita-module.
require "Krita"

# fetch the image.
image = Krita.image()

# we like to manipulate the active painting layer.
layer = image.activePaintLayer()

# currently the invert.py does not work with all supported colorspaces.
if(layer.colorSpaceId() != "RGBA" )
    raise("This script works only for 8bit RGBA layers")
end

# get the height and the width the layer has.
width = layer.width()
height = layer.height()

# we like to use the progressbar
progress = Krita.progress()
progress.setProgressTotalSteps(width * height)

# tell Krita that painting starts. the whole painting session will be
# counted till layer.endPainting() was called as one undo/redo-step.
layer.beginPainting("invert")

# create an iterator to walk over all pixels the layer has.
it = layer.createRectIterator( 0, 0, width, height )

# iterate over all pixels and invert each pixel.
while (not it.isDone())
    #p = it.getRGBA()
    #p[0] = 255 - p[0]
    #p[1] = 255 - p[1]
    #p[2] = 255 - p[2]
    #it.setRGBA(p)

    # invert the color of the pixel.
    it.invertColor()

    # increment the progress to show, that work on this pixel is done.
    progress.incProgress()

    # go to the next pixel.
    it.next()
end

# painting is done now.
layer.endPainting()
