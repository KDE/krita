# Torture Krita with painting.
#
# Paint on an image and create multiple layers.
#
# Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
# Published under the GNU GPL >=v2

require "Krita"

class TorturePainting

    def initialize()
        @image = Krita.image()
        @width = @image.getWidth()
        @height = @image.height()
        @width = @image.width()

        @progress = Krita.progress()
        @progress.setProgressTotalSteps(30 * 30)

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

    def randomizeStyle(painter)
        painter.setFillStyle(4 *rand)
        painter.setStrokeStyle(2 *rand)
    end

    def testColorspace(cs)
        print "Torturing for ", cs, "\n"
        layer = @image.createPaintLayer("torture", 255 * rand, "RGBA")
        torture(layer)
    end

    def torture(layer)
        layer.beginPainting("torture painting")

        painter = layer.createPainter()

        # create painting color
        blackcolor = Krita.createRGBColor(0,0,0)
        # set painting color
        painter.setPaintColor( blackcolor )
        # get the pattern
        pattern = Krita.pattern("Bricks")
        # set the pattern
        painter.setPattern(pattern)
        # define the paint operation
        painter.setPaintOp("paintbrush")

        # randomly rect or circle paint
        for i in 1..30
            # set painting color
            painter.setPaintColor( Krita.createRGBColor(rand*255,rand*255,rand*255) )
            painter.setBackgroundColor( Krita.createRGBColor(rand*255,rand*255,rand*255) )
            painter.setOpacity( rand*255 )
            # set the brush
            if(rand < 0.5)
                painter.setBrush( Krita.createRectBrush(rand*20,rand*20,rand*10,rand*10) )
            else
                painter.setBrush( Krita.createCircleBrush(rand*20,rand*20,rand*10,rand*10) )
            end
            # paint a point
            shape = rand * 7
            painter.setStrokeStyle(1)
            if( shape < 1 )
                painter.paintAt(rand * @width , rand * @height,1.1)
            elsif(shape < 2 )
                xs = Array.new
                ys = Array.new
                for i in 0..6
                    xs[i] = rand*@width
                    ys[i] = rand*@height
                end
                painter.paintPolyline(xs,ys)
            elsif(shape < 3)
                painter.paintLine(rand * @width, rand * @height, 1.1, rand * @width, rand * @height,1.1)
            elsif(shape < 4)
                painter.paintBezierCurve(rand * @width, rand * @height, 1.1, rand * @width, rand * @height, rand * @width , rand * @height, rand * @width, rand * @height, 1.1)
            elsif(shape < 5)
                randomizeStyle(painter)
                painter.paintEllipse(rand * @width, rand * @height, rand * @width, rand * @height, 1.1)
            elsif(shape < 6)
                xs = Array.new
                ys = Array.new
                for i in 0..6
                    xs[i] = rand*@width
                    ys[i] = rand*@height
                end
                randomizeStyle(painter)
                painter.paintPolygon(xs, ys)
            elsif(shape < 7)
                randomizeStyle(painter)
                painter.paintRect(rand * @width, rand * @height, rand * @width, rand * @height, 1.1)
            end
            @progress.incProgress()
        end
        layer.endPainting()
    end

end

TorturePainting.new()