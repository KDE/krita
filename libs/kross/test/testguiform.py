#!/usr/bin/env python

import Kross

#window = Kross.activeWindow()
#if window == None:
#    print "Creating new dialog"
#    window = Kross.createDialog("TestGuiFormDialog")
#else:
#    print "Using active window"

dialog = Kross.createDialog("TestGuiFormDialog")
#form = Kross.createForm(dialog)
form = dialog
print "form %s %s" % (form,dir(form))
form.loadUiFile("/home/kde4/koffice/libs/kross/test/testguiform.ui")
form.exec_loop()

#dialog.show()
#print dialog.exec_loop()

print "END!!!"
