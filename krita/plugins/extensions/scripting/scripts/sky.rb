require "Krita"

# fetch the image.
image = Krita.image()

countStar = 60
maxStarSize = 4

# we like to manipulate the active painting layer.
layer = image.createPaintLayer("Sky", 255).paintDevice()

# get the height and the width the image has.
width = image.width()
height = image.height()

# start drawing
layer.beginPainting("sky")

painter = layer.createPainter()

painter.setStrokeStyle(1)
painter.setFillStyle(1)
painter.setPaintOp("paintbrush")

painter.setPaintColor( Krita.createRGBColor(24,24,24) )
painter.paintRect(0.0,0.0, width, height, 0.5)

painter.setPaintColor( Krita.createRGBColor(255,255,255) )
for i in 1..countStar
  size = rand() * maxStarSize
  size = 1 if(size < 1)
  painter.setBrush(Krita.createCircleBrush(size,size, size / 2 +1, size / 2 +1) )
  painter.paintAt(rand() * width, rand() * height, 0.5)
end

# painting is done now.
layer.endPainting()
