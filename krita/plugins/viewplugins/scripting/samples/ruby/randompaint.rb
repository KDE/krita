require "krosskritacore"

doc = Krosskritacore::get("KritaDocument")
script = Krosskritacore::get("KritaScript")

image = doc.getImage()
layer = image.getActivePaintLayer()
width = layer.getWidth()
height = layer.getHeight()

script.setProgressTotalSteps(110)
layer.beginPainting("random paint")

painter = layer.createPainter()

# create painting color
blackcolor = Krosskritacore::newRGBColor(0,0,0)

# set painting color
painter.setPaintColor( blackcolor )

# get the brush
brush =  Krosskritacore::getBrush("Circle (05)")

# define the brush
painter.setBrush(brush)

# define the paint operation
painter.setPaintOp("paintbrush")

# randomly paint
for i in 1..10
    # set painting color
    painter.setPaintColor( Krosskritacore::newRGBColor(rand*255,rand*255,rand*255) )
    painter.paintAt(rand * width , rand * height,1.1)
    script.incProgress()
end

# randomly rect or circle paint
for i in 1..100
    # set painting color
    painter.setPaintColor( Krosskritacore::newRGBColor(rand*255,rand*255,rand*255) )
    # set the brush
    if(rand < 0.5)
        painter.setBrush( Krosskritacore::newRectBrush(rand*20,rand*20,rand*10,rand*10) )
    else
        painter.setBrush( Krosskritacore::newCircleBrush(rand*20,rand*20,rand*10,rand*10) )
    end
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
        painter.paintEllipse(rand * width, rand * height, rand * width, rand * height, 1.1)
    elsif(shape < 6)
        xs = Array.new
        ys = Array.new
        for i in 0..6
            xs[i] = rand*width
            ys[i] = rand*height
        end
        painter.paintPolygon(xs, ys)
    elsif(shape < 7)
        painter.paintRect(rand * width, rand * height, rand * width, rand * height, 1.1)
    end
    script.incProgress()
end

layer.endPainting()
