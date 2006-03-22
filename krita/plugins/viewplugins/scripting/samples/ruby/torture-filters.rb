# This file is part of Krita
#
# Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

require "krosskritacore"

doc = Krosskritacore::get("KritaDocument")

image = doc.getImage()
layer = image.getActivePaintLayer()

def testFilter(layer, filterid)
    print " applying filter ", filterid, "\n"
    begin
        filter = Krosskritacore::getFilter(filterid)
        filter.process(layer)
    rescue
        print " WARNING: this filter is incompatible with this colorspace\n"
    end
end


def testColorspace(layer, colorspaceid)
    print "Testing for ", colorspaceid, "\n"
    if (layer.colorSpaceId() != colorspaceid)
        layer.convertToColorspace(colorspaceid)
    end
    testFilter(layer, "invert")
    testFilter(layer, "bumpmap")
    testFilter(layer, "cimg")
    testFilter(layer, "desaturate")
    testFilter(layer, "autocontrast")
    testFilter(layer, "brightnesscontrast")
    testFilter(layer, "gaussian blur")
    testFilter(layer, "cubism")
    testFilter(layer, "emboss")
    testFilter(layer, "simplenoisereducer")
    testFilter(layer, "waveletnoisereducer")
    testFilter(layer, "oilpaint")
    testFilter(layer, "pixelize")
    testFilter(layer, "raindrops")
    testFilter(layer, "roundcorners")
    testFilter(layer, "smalltiles")
    testFilter(layer, "sobel")
end

testColorspace(layer, "RGBA")
testColorspace(layer, "RGBA16")
testColorspace(layer, "RGBAF16HALF")
testColorspace(layer, "RGBAF32")
testColorspace(layer, "CMYK")
testColorspace(layer, "CMYKA16")
testColorspace(layer, "CMYK")
testColorspace(layer, "CMYKA16")
testColorspace(layer, "LABA")
testColorspace(layer, "LMSAF32")
