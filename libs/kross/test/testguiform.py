#!/usr/bin/env kross

import Kross, os

#window = Kross.activeWindow()
#if window == None:
#    print "Creating new dialog"
#    window = Kross.createDialog("TestGuiFormDialog")
#else:
#    print "Using active window"

print "1................"
forms = Kross.module("forms")
print "2................"
print "===================================> %s" % forms
print dir(forms)

dialog = forms.createDialog("TestGuiFormDialog")
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

page3 = dialog.addPage("Options","Options","configure")
uifile = os.path.join( self.currentPath(), "testguiform.ui" )
widget3 = forms.createWidgetFromUIFile(page3, uifile)

page1 = dialog.addPage("File","Import From Image File","fileopen")
widget1 = forms.createFileWidget(page1, "kfiledialog:///mytestthingy1")
#widget1.setMode("Saving")
widget1.setMode("Opening")
widget1.setFilter("*.cpp|C++ Source Files\n*.h|Header files")

#optionspage = dialog.addPage("Options","Import Options","configure")
#optionswidget = Kross.forms().createWidgetFromUIFile(optionspage, "/home/kde4/koffice/krita/plugins/viewplugins/scripting/scripts/pilimport.ui")

#w = widget #widget["KFileDialog::mainWidget"]
#print dir(w)
#for idx in range( len(w) ):
#    obj = w[ idx:idx ][0]
#    print "...... idx=%s obj=%s name=%s class=%s" % ( idx , obj , obj.__name__ , obj.__class__ )

##combo = dialog["QWidget"]["QWidget"]["QGroupBox"]["QComboBox"]
#combo = widget["QGroupBox"]["QComboBox"]
#combo.setEditText("Hello World")

print "######################################## self.currentPath=\"%s\"" % self.currentPath()

result = dialog.exec_loop()
if result:
    print "===> result=%s" % dialog.result()

    def getOption(widget, optionname, optionlist):
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

    colorspace = getOption(optionswidget, "Colorspace", ["RGB","CMYK"])
    destination = getOption(optionswidget, "Destination", ["NewLayer","ActiveLayer"])
    size = getOption(optionswidget, "Size", ["Resize","Scale","Ignore"])

    print "===> colorspace=%s destination=%s size=%s" % (colorspace,destination,size)

    #destinationwidget = optionswidget["Destination"]
    #if destinationwidget["NewLayer"].checked:
    #elif destinationwidget["ActiveLayer"].checked:
    #else:

    #sizebox = optionswidget["Size"]
    #if sizebox["Resize"].checked:
    #elif sizebox["Scale"].checked:
    #elif sizebox["Ignore"].checked:
    #else:

    #w = csbox
    #for idx in range( len(w) ):
    #    obj = w[ idx:idx ][0]
    #    print "...... idx=%s obj=%s name=%s class=%s" % ( idx , obj , obj.__name__ , obj.__class__ )

print dir(dialog)
dialog.delayedDestruct();
