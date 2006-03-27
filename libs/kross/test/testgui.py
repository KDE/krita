#!/usr/bin/env python

"""
  This Python script demonstrates the usage of the Kross
  python-interface to access KexiDB functionality from
  within Python.
"""

class TkTest:
	def __init__(self):
		import Tkinter
		self.root = Tkinter.Tk()
		self.root.title("TkTest")
		self.root.deiconify()

		self.mainframe = Tkinter.Frame(self.root)
		self.mainframe.pack()

		self.button1 = Tkinter.Button(self.mainframe, text="Button1", command=self.callback1)
		self.button1.pack(side=Tkinter.LEFT)
		
		self.button2 = Tkinter.Button(self.mainframe, text="Button2", command=self.callback2)
		self.button2.pack(side=Tkinter.LEFT)

		self.exitbutton = Tkinter.Button(self.mainframe, text="Exit", command=self.root.destroy)
		self.exitbutton.pack(side=Tkinter.LEFT)

		self.root.mainloop()

	def callback1(self):
		import tkMessageBox
		tkMessageBox.showinfo("Callback1", "Callback1 called.")

	def callback2(self):
		import tkMessageBox
		tkMessageBox.showinfo("Callback2", "Callback2 called.")

class QtTest:
	def __init__(self):
		import qt

		class Button(qt.QPushButton):
			def __init__(self, *args):
				apply(qt.QPushButton.__init__, (self,) + args)

		class ComboBox(qt.QHBox):
 			def __init__(self, parent, caption, items = []):
				qt.QHBox.__init__(self, parent)
				self.setSpacing(6)
				label = qt.QLabel(str(caption), self)
				self.combobox = qt.QComboBox(self)
				self.setStretchFactor(self.combobox, 1)
				label.setBuddy(self.combobox)
				for item in items:
					self.combobox.insertItem( str(item) )

                class FileChooser(qt.QHBox):
			def __init__(self, *args):
				apply(qt.QHBox.__init__, (self,) + args)
				self.defaultfilename = "~/output.html"
				
                                self.setSpacing(6)
				label = qt.QLabel("File:", self)
				self.edit = qt.QLineEdit(self)
				self.edit.setText(self.defaultfilename)
				self.setStretchFactor(self.edit, 1)
				label.setBuddy(self.edit)
				
				browsebutton = Button("...", self)
				qt.QObject.connect(browsebutton, qt.SIGNAL("clicked()"), self.browseButtonClicked)

			def file(self):
				return self.edit.text()

			def browseButtonClicked(self):
				filename = None
				try:
					# try to use the kfile module included in pykde
					import kfile
					filename = kfile.KFileDialog.getOpenFileName(self.defaultfilename, "*.html", self, "Save to file")
				except:
					# fallback to Qt filedialog
					filename = qt.QFileDialog.getOpenFileName(self.defaultfilename, "*.html", self, "Save to file")
				if filename != None and filename != "":
					self.edit.setText(filename)

		class Dialog(qt.QDialog):
			def __init__(self, parent = None, name = None, modal = 0, fl = 0):
				qt.QDialog.__init__(self, parent, name, modal, fl)
				qt.QDialog.accept = self.accept
				self.setCaption("Export to HTML")
				#self.layout()
				
				self.layout = qt.QVBoxLayout(self)
				self.layout.setSpacing(6)
				self.layout.setMargin(11)

				infolabel = qt.QLabel("Export the data of a table or a query to a HTML-file.", self)
				self.layout.addWidget(infolabel)

				source = ComboBox(self, "Datasource:")
				self.layout.addWidget(source)

				self.exporttype = ComboBox(self, "Style:", ["Plain","Paper","Desert","Blues"])
				self.layout.addWidget(self.exporttype)

				self.filechooser = FileChooser(self)
				self.layout.addWidget(self.filechooser)

				buttonbox = qt.QHBox(self)
				buttonbox.setSpacing(6)
				self.layout.addWidget(buttonbox)

				savebutton = Button("Save", buttonbox)
				qt.QObject.connect(savebutton, qt.SIGNAL("clicked()"), self, qt.SLOT("accept()"))
				#qt.QObject.connect(savebutton, qt.SIGNAL("clicked()"), self.exportButtonClicked)

				cancelbutton = Button("Cancel", buttonbox)
				qt.QObject.connect(cancelbutton, qt.SIGNAL("clicked()"), self, qt.SLOT("close()"))
				
			def accept(self):
				print "ACCEPTTTTTTTT !!!!!!!!!!!!!!!!!!!!!!!!!!!!"
				
				file = qt.QFile( self.filechooser.file() )
				#if not file.exists():
				#	print "File '%s' does not exist." % self.filechooser.file()
				#else:
				#	print "File '%s' does exist." % self.filechooser.file()

			def exportButtonClicked(self):
				print "Export to HTML !!!!!!!!!!!!!!!!!!!!!!!!!!!!"

			def __getattr__(self, attr):
				print "=> Dialog.__getattr__(self,attr)"
			#def closeEvent(self, ev): pass
			def event(self, e):
				print "=> Dialog.event %s" % e
				#self.deleteLater()
				#support.swapThreadState() # calls appropriate c-function 
				return qt.QDialog.event(self, e)

                app = qt.qApp
		dialog = Dialog(app.mainWidget(), "Dialog", 1)
		dialog.show()

print "################## BEGIN"
#TkTest()
QtTest()
print "################## END"
