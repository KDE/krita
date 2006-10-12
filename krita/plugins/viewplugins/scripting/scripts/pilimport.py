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

    def __init__(self, scriptaction):
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

        self.scriptaction = scriptaction
        self.showDialog()

    def showDialog(self):
        """ Shows the import-dialog and let the user define the imagefile which
        should be imported. """

        import Kross, os
        forms = Kross.module("forms")
        dialog = forms.createDialog("Python Imaging Library Import")
        try:
            dialog.setFaceType("List")
            dialog.setButtons("Ok|Cancel")

            filepage = dialog.addPage("File","Import Image From File","fileopen")
            filewidget = forms.createFileWidget(filepage, "kfiledialog:///kritapilimport")
            filewidget.setMode("Opening")
            filewidget.setFilter(self.getFilters())

            optionspage = dialog.addPage("Options","Import Options","configure")
            currentpath = self.scriptaction.currentPath()
            uifile = os.path.join(currentpath, "pilimport.ui")
            optionswidget = forms.createWidgetFromUIFile(optionspage, uifile)

            if dialog.exec_loop():
                self.filename = filewidget.selectedFile()
                self.colorspace = self.getOption(optionswidget, "Colorspace", ["RGB","CMYK"])
                self.destination = self.getOption(optionswidget, "Destination", ["NewLayer","ActiveLayer"])
                self.size = self.getOption(optionswidget, "Size", ["Resize","Scale","Ignore"])

                if not os.path.isfile(self.filename):
                    raise "No valid file to import from choosen."

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

    def getOption(self, widget, optionname, optionlist):
        try:
            childwidget = widget[ optionname ]
            for option in optionlist:
                radiobutton = childwidget[ option ]
                if radiobutton.checked:
                    return option
        except:
            import sys, traceback
            raise "\n".join( traceback.format_exception(sys.exc_info()[0],sys.exc_info()[1],sys.exc_info()[2]) )
        raise "No such option \"%s\"" % optionname

    def doImport(self):
        """ Loads the image from the defined filename and imports it. """
        print ">>>>>>>>>>>>>>>>>>>>>>>> IMPORT filename=\"%s\" colorspace=\"%s\" destination=\"%s\" size=\"%s\"" % (self.filename,self.colorspace,self.destination,self.size)

        import Krita, Image, ImageFile

        # read the imagefile which should be imported.
        pilimage = Image.open(self.filename)

        # convert the readed image into the defined colorspace.
        if not self.colorspace in ["RGB","CMYK"]:
            raise "Unknown colorspace option \"%s\"" % self.colorspace
        pilimage = pilimage.convert( self.colorspace )

        (width, height) = pilimage.size
        krtimage = Krita.image()

        # evaluate the image size options
        if self.size == "Resize":
            if krtimage.width() != width or krtimage.height() != height:
                krtimage.resize(width, height, 0, 0)
        elif self.size == "Scale":
            if krtimage.width() != width or krtimage.height() != height:
                width = krtimage.width()
                height = krtimage.height()
                pilimage = pilimage.resize( (width, height) )
        elif self.size == "Ignore":
            if width > krtimage.width():
                width = krtimage.width()
            if height > krtimage.height():
                height = krtimage.height()
        else:
            raise "Unknown size option \"%s\"" % self.size

        # evaluate the destination options.
        if self.destination == "ActiveLayer":
            krtlayer = krtimage.activePaintLayer()
        elif self.destination == "NewLayer":
            if self.colorspace == "CMYK":
                cs = "CMYK"
            else:
                cs = "RGBA"
            krtlayer = krtimage.createPaintLayer("pilimport", 100, cs)
        else:
            raise "Unknown destination option \"%s\"" % self.destination


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

        krtprogress = Krita.progress()
        krtprogress.setProgressTotalSteps(width * height)

        krtlayer.beginPainting("PIL import")
        it = krtlayer.createRectIterator(0, 0, width, height)
        finesh = it.isDone()
        while (not finesh):
            data = pilimage.getpixel( (it.x(),it.y()) )
            it.setPixel( list(data) )
            krtprogress.incProgress()
            finesh = it.next()
        krtlayer.endPainting()

Importer( self )
