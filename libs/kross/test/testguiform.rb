#!/usr/bin/env kross

require 'Kross'

dialog = Kross.createDialog("TestGuiFormDialog")
form = dialog
print "form %s %s" % (form,dir(form))
form.loadUiFile("/home/kde4/koffice/libs/kross/test/testguiform.ui")
form.exec_loop()
