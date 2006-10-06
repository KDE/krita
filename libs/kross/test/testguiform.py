#!/usr/bin/env python

import Kross

#window = Kross.activeWindow()
#if window == None:
#    print "Creating new dialog"
#    window = Kross.createDialog("TestGuiFormDialog")
#else:
#    print "Using active window"

dialog = Kross.createDialog("TestGuiFormDialog")
dialog.setButtons("Ok|Cancel")
print "===> dialog %s %s" % (dialog,dir(dialog))

form = dialog
#print "===> form %s %s" % (form,dir(form))
form.loadUiFile("/home/kde4/koffice/libs/kross/test/testguiform.ui")

result = dialog.exec_loop()
print "===> result=%s" % result
