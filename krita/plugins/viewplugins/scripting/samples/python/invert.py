class Inverter:
    def __init__(self):
        try:
            import krosskritacore
        except:
            raise "Import of the KritaCore module failed."
        doc = krosskritacore.get("KritaDocument")
        image = doc.getImage()
        layer = image.getActiveLayer()
        width = layer.getWidth()
        height = layer.getHeight()
        it = layer.createRectIterator( 0, 0, width, height )
        print "kikoo\n"
        while (not it.isDone()) :
            r = it.getRed()
            nr = 255 - r
            it.setRed(nr)
            g = it.getGreen()
            ng = 255 - g
            it.setGreen(ng)
            b = it.getBlue()
            nb = 255-b
            it.setBlue(nb)
            it.next()
        doc.notifyModification()

Inverter()
