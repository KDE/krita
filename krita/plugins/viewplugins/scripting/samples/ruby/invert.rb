print "begin invert script\n"

require "krosskritacore"

print "Before error\n"

doc = Krosskritacore::get("KritaDocument")
script = Krosskritacore::get("KritaScript")
image = doc.getImage()
layer = image.getActiveLayer()
width = layer.getWidth()
print width
height = layer.getHeight()
print height
script.setProgressTotalSteps(width * height)
layer.beginPainting("invert")
it = layer.createRectIterator( 0, 0, width, height )
while (it.isDone() == 0)
    r = it.getRed()
    nr = 255 - r
    it.setRed(nr)
    g = it.getGreen()
    ng = 255 - g
    it.setGreen(ng)
    b = it.getBlue()
    nb = 255-b
    it.setBlue(nb)
    script.incProgress()
    it.next()
end

layer.endPainting()


print "script is finished\n"
