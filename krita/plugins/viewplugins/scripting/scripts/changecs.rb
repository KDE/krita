# Ruby change colorspace script.
#
# Demonstrate how to change the colorspace in a ruby script.
#
# Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
# Published under the GNU GPL >=v2

# import the Krita-module.
require "Krita"

# fetch the image.
image = Krita.image()

# change the colorspace.
image.convertToColorspace("LABA")
