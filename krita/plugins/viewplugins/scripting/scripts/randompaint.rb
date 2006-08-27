# Ruby random painting script.
#
# This ruby invert script does random painting on the activate layer in
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

# get the height and the width the layer has.
width = layer.width()
height = layer.height()

# we like to use the progressbar
progress = Krita.progress()
progress.setProgressTotalSteps(110)

# tell Krita that painting starts. the whole painting session will be
# counted till layer.endPainting() was called as one undo/redo-step.
layer.beginPainting("random paint")

# create a new painter to paint with.
painter = layer.createPainter()

# set painting color to black.
painter.setPaintColor( Krita.createRGBColor(0,0,0) )

# define the brush
brush =  Krita.brush("Circle (05)")
painter.setBrush(brush)

# define the pattern
pattern = Krita.pattern("Bricks")
painter.setPattern(pattern)

# define the paint operation
painter.setPaintOp("paintbrush")

# randomly paint
for i in 1..10
    # set random painting color
    painter.setPaintColor( Krita.createRGBColor(rand*255,rand*255,rand*255) )
    # actual paint
    painter.paintAt(rand * width , rand * height,1.1)
    # increment the progress to outline that we are one step closer.
    progress.incProgress()
end

# function to randomize the style.
def randomizeStyle(painter)
    painter.setFillStyle(4 *rand)
    painter.setStrokeStyle(2 *rand)
end

# randomly rect or circle paint
for i in 1..100

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

    # set the stroke style
    painter.setStrokeStyle(1)

    # paint a point
    shape = rand * 7
    if( shape < 1 )
        painter.paintAt(rand * width , rand * height,1.1)
    elsif(shape < 2 )
        xs = Array.new
        ys = Array.new
        for i in 0..6
            xs[i] = rand*width
            ys[i] = rand*height
        end
        painter.paintPolyline(xs,ys)
    elsif(shape < 3)
        painter.paintLine(rand * width, rand * height, 1.1, rand * width, rand * height,1.1)
    elsif(shape < 4)
        painter.paintBezierCurve(rand * width, rand * height, 1.1, rand * width, rand * height, rand * width , rand * height, rand * width, rand * height, 1.1)
    elsif(shape < 5)
        randomizeStyle(painter)
        painter.paintEllipse(rand * width, rand * height, rand * width, rand * height, 1.1)
    elsif(shape < 6)
        xs = Array.new
        ys = Array.new
        for i in 0..6
            xs[i] = rand*width
            ys[i] = rand*height
        end
        randomizeStyle(painter)
        painter.paintPolygon(xs, ys)
    elsif(shape < 7)
        randomizeStyle(painter)
        painter.paintRect(rand * width, rand * height, rand * width, rand * height, 1.1)
    end

    # increment the progress to outline that we are one step closer.
    progress.incProgress()
end

# painting is done now.
layer.endPainting()
