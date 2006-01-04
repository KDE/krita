require "krosskritacore"

doc = Krosskritacore::get("KritaDocument")
script = Krosskritacore::get("KritaScript")

image = doc.getImage()
layer = image.getActivePaintLayer()
width = layer.getWidth()
height = layer.getHeight()

script.setProgressTotalSteps(10)
layer.beginPainting("random paint")

painter = layer.createPainter()

# create painting color
blackcolor = Krosskritacore::newRGBColor(0,0,0)
print blackcolor.getClassName()

# set painting color
painter.setPaintColor( blackcolor )

# get the brush
brush =  Krosskritacore::getBrush("Circle (05)")

# define the brush
painter.setBrush(brush)

# define the paint operation
painter.setPaintOp("pen")

# randomly paint
for i in 1..10
    painter.paintAt(rand * width , rand * height,1.1)
    script.incProgress()
end

layer.endPainting()
