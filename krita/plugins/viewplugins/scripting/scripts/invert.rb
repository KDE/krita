# Ruby invert script.
#
# The ruby invert script inverts all pixels at the activate layer in
# the current image.
#
# Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
# Published under the GNU GPL =v2

# import the Krita-module.
require "Krita"

# fetch the image.
image = Krita.image()

# we like to manipulate the active painting layer.
layer = image.activePaintLayer()

# get the height and the width the layer has.
width = layer.width()
height = layer.height()

# we like to use the progressbar
shell = Krita.shell()
shell.slotSetStatusBarText("invert.rb")
shell.slotProgress(0)
size = width * height
progress = 0
pixeldone = 0

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
    percent = pixeldone * 100 / size
    if (percent != progress)
        progress = percent
        shell.slotProgress( progress )
    end
    pixeldone += 1

    # go to the next pixel.
    it.next()
end

# painting is done now.
layer.endPainting()

# finish progressbar
shell.slotProgress(-1)
shell.slotSetStatusBarText("")
