#!/usr/bin/env python

import Kross

#window = Kross.activeWindow()
#if window == None:
#    print "Creating new dialog"
#    window = Kross.createDialog("TestGuiFormDialog")
#else:
#    print "Using active window"

dialog = Kross.forms().createDialog("TestGuiFormDialog")
dialog.setButtons("Ok|Cancel")
dialog.setFaceType("List") #Auto Plain List Tree Tabbed

#print "===> dialog %s %s" % (dialog,dir(dialog))

#page0 = dialog.addPage("Welcome","Welcome","about_kde")
##widget0 = Kross.forms().createWidget(page0, 'QWidget', 'MyForm1', {})
##widget0label = Kross.forms().createWidget(page0, 'QLabel', 'label', {'text':'Testlabel'})
#widget0 = Kross.forms().createWidgetFromUI(page0,
    #'<ui version="4.0" >'
    #' <class>Form</class>'
    #' <widget class="QWidget" name="Form" >'
    #'  <layout class="QHBoxLayout" >'
    #'   <property name="margin" >'
    #'    <number>9</number>'
    #'   </property>'
    #'   <property name="spacing" >'
    #'    <number>6</number>'
    #'   </property>'
    #'   <item>'
    #'    <widget class="QLabel" name="label" >'
    #'     <property name="text" >'
    #'      <string>Testlabel</string>'
    #'     </property>'
    #'     <property name="alignment" >'
    #'      <set>Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop</set>'
    #'     </property>'
    #'    </widget>'
    #'   </item>'
    #'  </layout>'
    #' </widget>'
    #'</ui>')

page1 = dialog.addPage("Source","Read From File","fileopen")
widget1 = Kross.forms().createFileWidget(page1, "kfiledialog:///mytestthingy1")
widget1.setMode("Saving") #("Opening")
widget1.setFilter("*.cpp|C++ Source Files\n*.h|Header files")

page2 = dialog.addPage("Options","Options","configure")
widget2 = Kross.forms().createWidgetFromUIFile(page2, "/home/kde4/koffice/libs/kross/test/testguiform.ui")

#w = widget #widget["KFileDialog::mainWidget"]
#print dir(w)
#for idx in range( len(w) ):
#    obj = w[ idx:idx ][0]
#    print "...... idx=%s obj=%s name=%s class=%s" % ( idx , obj , obj.__name__ , obj.__class__ )

##combo = dialog["QWidget"]["QWidget"]["QGroupBox"]["QComboBox"]
#combo = widget["QGroupBox"]["QComboBox"]
#combo.setEditText("Hello World")

result = dialog.exec_loop()
if result:
    print "===> result=%s" % dialog.result()
    ##print ">>>>>>>>>>>>>>>>>>>>>>> %s" % combo.currentText
    ##print dir(w)
    #print "name=%s class=%s" % (w.__name__,w.__class__)
    #print ">>>>>>>>>>>>>>>>>>>>>>> selectedFile=%s" % w.selectedFile()
    ##print ">>>>>>>>>>>>>>>>>>>>>>> selectedFile=%s selectedFiles=%s selectedUrl=%s" % (w.selectedFile(),w.selectedFiles(),w.selectedUrl())

print dir(dialog)
dialog.delayedDestruct();
