"""
Python invert script.

The python invert script inverts all pixels at the activate layer in
the current image.

Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
Published under the GNU GPL >=v2
"""

class Inverter:
    def __init__(self):

        # import the Krita module.
        try:
            import Krita
            self.Krita = Krita
        except:
            raise "Import of the Krita module failed."

        # fetch the image.
        print "dir(Krita): %s" % dir(self.Krita)
        image = self.Krita.image()
        print "dir(image): %s" % dir(image)

        # we like to manipulate the active painting layer.
        layer = image.activePaintLayer()
        print "dir(layer): %s" % dir(layer)

        # currently the invert.py does not work with all supported colorspaces.
        if(layer.colorSpaceId() != "RGBA" ):
            raise("This script works only for 8bit RGBA layers")

        # get the height and the width the layer has.
        width = layer.width()
        height = layer.height()

        # we like to use the progressbar
        #script.setProgressTotalSteps(width * height)

        # tell Krita that painting starts. the whole painting session will be
        # counted till layer.endPainting() was called as one undo/redo-step.
        layer.beginPainting("invert")

        # create an iterator to walk over all pixels the layer has.
        it = layer.createRectIterator( 0, 0, width, height )

        # iterate over all pixels and invert each pixel.
        print "kikoo\n"
        finesh = it.isDone()
        while (not finesh):
            #p = it.getRGBA()
            #p[0] = 255 - p[0]
            #p[1] = 255 - p[1]
            #p[2] = 255 - p[2]
            #it.setRGBA(p)

            it.invertColor()

            # increment the progress to show, that work on this pixel is done.
            #script.incProgress()

            # go to the next pixel.
            finesh = it.next()

        # painting is done now.
        layer.endPainting()

Inverter()
