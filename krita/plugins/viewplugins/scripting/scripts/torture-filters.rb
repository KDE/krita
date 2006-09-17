# Torture Krita with filters.
#
# Test all filters with all colorspaces.
#
# Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
# Published under the GNU GPL >=v2

require "Krita"

class TortureFilters

    def initialize()
        image = Krita.image()
        @layer = image.activePaintLayer()

        @progress = Krita.progress()
        @progress.setProgressTotalSteps(10 * 17)

        testColorspace("RGBA")
        testColorspace("RGBA16")
        testColorspace("RGBAF16HALF")
        testColorspace("RGBAF32")
        testColorspace("CMYK")
        testColorspace("CMYKA16")
        testColorspace("CMYK")
        testColorspace("CMYKA16")
        testColorspace("LABA")
        testColorspace("LMSAF32")
    end

    def testColorspace(colorspaceid)
        print "Testing for ", colorspaceid, "\n"
        if (@layer.colorSpaceId() != colorspaceid)
            @layer.convertToColorspace(colorspaceid)
        end
        testFilter("invert")
        testFilter("bumpmap")
        testFilter("cimg")
        testFilter("desaturate")
        testFilter("autocontrast")
        testFilter("brightnesscontrast")
        testFilter("gaussian blur")
        testFilter("cubism")
        testFilter("emboss")
        testFilter("simplenoisereducer")
        testFilter("waveletnoisereducer")
        testFilter("oilpaint")
        testFilter("pixelize")
        testFilter("raindrops")
        testFilter("roundcorners")
        testFilter("smalltiles")
        testFilter("sobel")
    end

    def testFilter(filterid)
        print " applying filter ", filterid, "\n"
        begin
            filter = Krita.filter(filterid)
            filter.process(@layer)
        rescue
            print " WARNING: this filter is incompatible with this colorspace\n"
        end
        @progress.incProgress()
    end

end

TortureFilters.new()
