#!/usr/bin/env kross

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

TkTest()
