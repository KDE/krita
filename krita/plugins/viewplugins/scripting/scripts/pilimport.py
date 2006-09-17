"""
Python Image Library script.

This python script uses the Python Image Library ( PIL, see
http://www.pythonware.com/library/ ) to import and export
images between PIL and Krita.

Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
Published under the GNU GPL >=v2
"""

try:
	import Krita
except:
	raise "Failed to import the Krita module."

try:
	import Image, ImageFile
except:
	raise "Failed to import the Python Image Library (PIL)."

def loadFromFile(filename):
	import Krita, Image, ImageFile

	pilimage = Image.open(filename)
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

import Tkinter, tkFileDialog
Image.init()
filters = []
for i in Image.ID:
	try:
		factory, accept = Image.OPEN[i]
		filters.append( (factory.format_description,".%s .%s" % (factory.format,factory.format.lower())) )
	except KeyError:
		pass
Tkinter.Tk().withdraw()
filename = tkFileDialog.askopenfilename(filetypes=filters)
if filename:
	loadFromFile(filename)
