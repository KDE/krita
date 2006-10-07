#!/usr/bin/env python

import Kross

#window = Kross.activeWindow()
#if window == None:
#    print "Creating new dialog"
#    window = Kross.createDialog("TestGuiFormDialog")
#else:
#    print "Using active window"

dialog = Kross.createDialog("TestGuiFormDialog")
print "===> dialog %s %s" % (dialog,dir(dialog))

dialog.setButtons("Ok|Cancel")
dialog.loadUiFile("/home/kde4/koffice/libs/kross/test/testguiform.ui")

combo = dialog["Kross::Form"]["QWidget"]["QGroupBox"]["QComboBox"]
combo.setEditText("Hello World")

result = dialog.exec_loop()
if result:
    print "===> result=%s" % dialog.result()

    #dialog.actions()

    widget = dialog["Kross::Form"]["QWidget"]["QGroupBox"]
    for idx in range( len(widget) ):
        obj = widget[ idx:idx ][0]
        print "...... idx=%s obj=%s name=%s class=%s" % ( idx , obj , obj.__name__ , obj.__class__ )

    print ">>>>>>>>>>>>>>>>>>>>>>> %s" % combo.currentText
