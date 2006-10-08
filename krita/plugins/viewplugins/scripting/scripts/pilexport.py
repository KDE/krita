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
	import Image, ImageFile, string
	Image.init()
except:
	raise "Failed to import the Python Image Library (PIL)."

def saveToFile(filename):
	import Krita, Image, ImageFile

	krtimage = Krita.image()
	krtlayer = krtimage.activePaintLayer()
	#krtcolorspaceid = krtlayer.colorSpaceId()
	height = krtlayer.height()
	width = krtlayer.width()

	progress = Krita.progress()
	progress.setProgressTotalSteps(width * height)

	pilimage = Image.new("RGB",(width,height))

	it = krtlayer.createRectIterator(0, 0, width, height)
	finesh = it.isDone()
	while (not finesh):
		pilimage.putpixel( (it.x(),it.y()), tuple(it.pixel()) )
		progress.incProgress()
		finesh = it.next()

	#pilimage.save(filename,"JPEG")
	pilimage.save(filename)

#######################################################################################
# Following code uses TkInter to display a fileopen-dialog.

#import Tkinter, tkFileDialog
#filters = []
#for i in Image.ID:
	#try:
		#driver = Image.SAVE[string.upper(i)]
		#factory, accept = Image.OPEN[i]
		#filters.append( (factory.format_description,".%s .%s" % (factory.format,factory.format.lower())) )
	#except KeyError:
		#pass
#Tkinter.Tk().withdraw()
#filename = tkFileDialog.asksaveasfilename(filetypes=filters)
#if filename:
	#saveToFile(filename)

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

import Kross
dialog = Kross.forms().createDialog("Python Imaging Library Export")
try:
	dialog.setButtons("Ok|Cancel")
	page = dialog.addPage("File","Export Image to File","filesave")
	widget = Kross.forms().createFileWidget(page, "kfiledialog:///kritapilexport")
	widget.setMode("Saving")
	widget.setFilter( "\n".join(filters) )
	if dialog.exec_loop():
		file = widget.selectedFile()
		saveToFile(file)
finally:
	dialog.delayedDestruct()
