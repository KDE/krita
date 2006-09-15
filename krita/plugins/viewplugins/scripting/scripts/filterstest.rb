# Ruby filters test script.
#
# The ruby filter test script demonstrates how to use filters
# within scripts.
#
# Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
# Published under the GNU GPL >=v2

# import the Krita-module.
require "Krita"

# fetch the image.
image = Krita.image()

# we like to operate on the active painting layer.
layer = image.activePaintLayer()

# get the height and the width the layer has.
width = layer.width()
height = layer.height()

# get the invert filter.
invertfilter = Krita.filter("invert")

# apply the filter first on the whole image.
invertfilter.process(layer)
# and then again on a small part of the image.
invertfilter.process(layer, 10, 10, 20, 20)
