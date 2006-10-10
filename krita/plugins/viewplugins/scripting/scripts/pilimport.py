"""
Python script to import an image into Krita using the Python Imaging Library.

This python script uses the Python Image Library ( PIL, see
http://www.pythonware.com/library/ ) to import and export
images between PIL and Krita.

Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
Published under the GNU GPL >=v2
"""

class Importer:
    """ The Importer class encapsulates the whole import a by the PIL module
    supported image file format into Krita functionality. """

    def __init__(self):
        """ The constructor called if the Importer class got instanciated and
        imports our needed modules to be sure there are available. """

        try:
            import Kross
        except:
            raise "Failed to import the Kross module."

        try:
            import Krita
        except:
            raise "Failed to import the Krita module."

        try:
            import Image, ImageFile
            Image.init()
        except:
            raise "Failed to import the Python Imaging Library (PIL)."

        self.showDialog()

    def showDialog(self):
        """ Shows the import-dialog and let the user define the imagefile which
        should be imported as well as some additional import-options. """

        import Kross
        dialog = Kross.forms().createDialog("Python Imaging Library Import")
        try:
            dialog.setButtons("Ok|Cancel")
            page = dialog.addPage("File","Import Image From File","fileopen")
            widget = Kross.forms().createFileWidget(page, "kfiledialog:///kritapilimport")
            widget.setMode("Opening")
            widget.setFilter(self.getFilters())
            if dialog.exec_loop():
                self.filename = widget.selectedFile()
                self.doImport()
        finally:
            dialog.delayedDestruct()

    def getFilters(self):
        """ Returns a filters-string of the readable fileformats supported by PIL. """

        import Image
        filters = []
        allfilters = ""
        for i in Image.ID:
            try:
                factory, accept = Image.OPEN[i]
                filters.append( "*.%s|%s (*.%s)" % (factory.format,factory.format_description,factory.format.lower()) )
                allfilters += "*.%s " % factory.format
            except KeyError:
                pass
        if len(filters) > 0:
            filters.insert(0, "%s|All Supported Files" % allfilters)
        return "\n".join(filters)

    def doImport(self):
        """ Loads the image from the defined filename and imports it. """

        import Krita, Image, ImageFile

        pilimage = Image.open(self.filename)
        pilimage = pilimage.convert("RGB")

        (height, width) = pilimage.size

        krtprogress = Krita.progress()
        krtprogress.setProgressTotalSteps(width * height)

        krtimage = Krita.image()
        if krtimage.width() != width or krtimage.height() != height:
            krtimage.resize(width, height, 0, 0)

        krtlayer = krtimage.activePaintLayer()

        #krtcolorspaceid = krtlayer.colorSpaceId()
        #krtlayer.convertToColorspace()
        #if colorspaceid == "RGBA": # RGB (8-bit integer/channel)
            #self.mode = "P" # 8-bit palette-mapped image.
        ##elif colorspaceid == "RGBA16": # RGB (16-bit integer/channel)
        ##elif colorspaceid == "RGBAF16HALF": # "RGB (16-bit float/channel)
        ##elif colorspaceid == "RGBAF32": # RGB (32-bit float/channel)
        ##elif colorspaceid == "GRAYA": # Grayscale (8-bit integer/channel)
        ##elif colorspaceid == "GRAYA16": # Grayscale (16-bit integer/channel)
        ##elif colorspaceid == "CMYK": # CMYK (8-bit integer/channel)
        ##elif colorspaceid == "CMYKA16": # CMYK (16-bit integer/channel)
        ##elif colorspaceid == "LMSAF32": # "LMS Cone Space (32-bit float/channel)"
        ##elif colorspaceid == "YCbCrAU16": # YCBCR (16-bit integer/channel)
        ##elif colorspaceid == "YCbCrAU8": # YCBCR (8-bit integer/channel)
        ##elif colorspaceid == "WET": # Watercolors
        ##elif colorspaceid == "W&S": # Wet & Sticky
        #else:
            #raise "The Krita colorspace \"%s\" is not supported by the KritaPil-plugin"

        krtlayer.beginPainting("PIL import")
        it = krtlayer.createRectIterator(0, 0, width, height)
        finesh = it.isDone()
        while (not finesh):
            data = pilimage.getpixel( (it.x(),it.y()) )
            it.setPixel( list(data) )
            krtprogress.incProgress()
            finesh = it.next()
        krtlayer.endPainting()

Importer()
