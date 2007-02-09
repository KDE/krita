"""
Python script to export a Krita image using the Python Imaging Library.

This python script uses the Python Image Library ( PIL, see
http://www.pythonware.com/library/ ) to import and export
images between PIL and Krita.

Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
Published under the GNU GPL >=v2
"""

class Exporter:
    """ The Exporter class encapsulates the whole export from a Krita image
    to a by the PIL module supported image file format functionality. """

    def __init__(self, scriptaction):
        """ The constructor called if the Exporter class got instanciated and
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

        self.scriptaction = scriptaction
        self.showDialog()

    def showDialog(self):
        """ Shows the export-dialog and let the user define the imagefile to
        which the Krita image should be exported. """

        import Kross
        forms = Kross.module("forms")
        dialog = forms.createDialog("Python Imaging Library Export")
        try:
            dialog.setButtons("Ok|Cancel")
            page = dialog.addPage("File","Export Image to File","filesave")
            widget = forms.createFileWidget(page, "kfiledialog:///kritapilexport")
            widget.setMode("Saving")
            widget.setFilter(self.getFilters())
            if dialog.exec_loop():
                self.filename = widget.selectedFile()
                self.doExport()
        finally:
            dialog.delayedDestruct()

    def getFilters(self):
        """ Returns a filters-string of the writable fileformats supported by PIL. """

        import Image, string
        filters = []
        allfilters = ""
        for i in Image.ID:
            try:
                driver = Image.SAVE[string.upper(i)]
                factory, accept = Image.OPEN[i]
                filters.append( "*.%s|%s (*.%s)" % (factory.format,factory.format_description,factory.format.lower()) )
                allfilters += "*.%s " % factory.format
            except KeyError:
                pass
        if len(filters) > 0:
            filters.insert(0, "%s|All Supported Files" % allfilters)
        return "\n".join(filters)

    def doExport(self):
        """ Saves the image to the defined filename. """

        import Krita, Image, ImageFile

        krtimage = Krita.image()
        krtlayer = krtimage.activePaintLayer()
        #krtcolorspaceid = krtlayer.colorSpaceId()
        height = krtlayer.height()
        width = krtlayer.width()

        shell = Krita.shell()
        shell.slotSetStatusBarText("Python Imaging Library Export")
        shell.slotProgress(0)
        size = width * height
        progress = 0
        pixeldone = 0
        try:
            pilimage = Image.new( "RGB", (width,height) )

            it = krtlayer.createRectIterator(0, 0, width, height)
            finesh = it.isDone()
            while (not finesh):
                pilimage.putpixel( (it.x(),it.y()), tuple(it.pixel()) )

                percent = pixeldone * 100 / size
                if percent != progress:
                    progress = percent
                    shell.slotProgress( progress )
                pixeldone += 1

                finesh = it.next()

            #pilimage.save(self.filename,"JPEG")
            pilimage.save(self.filename)
        finally:
            shell.slotProgress(-1)
            shell.slotSetStatusBarText("")

Exporter( self )
