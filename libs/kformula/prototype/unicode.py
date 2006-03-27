#!/usr/bin/env python

import sys
from qt import *
from xml.sax import saxutils, handler, make_parser

class Form1(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if name == None:
            self.setName('Form1')

        self.setCaption(self.tr('Form1'))
        grid = QGridLayout(self)
        grid.setSpacing(6)
        grid.setMargin(11)

        self.chars = {}
        self.fontName = None

        begin = 32
        end = 256
        for i in range(begin, end):

            charLabel = QLabel(self,'charLabel' + chr(i))
            charLabel.setFont(QFont("symbol", 16))
            charLabel.setText(self.tr(chr(i)))
            grid.addWidget(charLabel, i-begin, 0)
            
            number = QLineEdit(self,'Number' + chr(i))
            grid.addWidget(number, i-begin, 1)

            latexName = QLineEdit(self,'latexName' + chr(i))
            grid.addWidget(latexName, i-begin, 2)

            charClass = QLineEdit(self,'charClass' + chr(i))
            grid.addWidget(charClass, i-begin, 3)
            
            self.chars[i] = (charLabel, number, latexName, charClass)

    def fontList(self):
        list = []
        for i in self.chars:
            charLabel, number, latexName, charClass = self.chars[i]
            if str(number.text()) != "" or str(latexName.text()) != "" or str(charClass.text()) != "":
                list.append((i, str(number.text()), str(latexName.text()), str(charClass.text())))
        return list
    
    def setFont(self, fontName, font):
        fontName = fontName.replace("%20", " ")
        self.fontName = fontName
        for i in self.chars:
            charLabel, number, latexName, charClass = self.chars[i]
            charLabel.setFont(QFont(fontName, 16))
            number.setText("")
            latexName.setText("")
            charClass.setText("")

        for (key, number, latexName, charClass) in font:
            i = int(key)
            charLabel, numberWidget, latexNameWidget, charClassWidget = self.chars[i]
            numberWidget.setText(number)
            latexNameWidget.setText(latexName)
            charClassWidget.setText(charClass)
            

class Widget(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        vbox = QVBoxLayout(self)
        vbox.setSpacing(6)
        vbox.setMargin(0)

        hbox = QHBoxLayout()
        hbox.setSpacing(6)
        hbox.setMargin(0)

        loadButton = QPushButton("load", self)
        saveButton = QPushButton("save", self)

        QObject.connect(loadButton, SIGNAL("pressed()"), self.load)
        QObject.connect(saveButton, SIGNAL("pressed()"), self.save)
        
        hbox.addWidget(loadButton)
        hbox.addWidget(saveButton)
                
        vbox.addLayout(hbox)
        
        splitter = QSplitter(self)
        splitter.setOrientation(Qt.Vertical)
        
        self.listbox = QListBox(splitter)
        
        sv = QScrollView(splitter)
        big_box = QVBox(sv.viewport())
        sv.addChild(big_box, 0, 0)
        self.child = Form1(big_box)

        vbox.addWidget(splitter)

        self.connect(self.listbox, SIGNAL('highlighted( const QString& )'),
                     self.fontHighlighted)

    def fontHighlighted(self, fontStr):
        if self.child.fontName:
            self.fonts[self.child.fontName] = self.child.fontList()
            
        font = str(fontStr)
        self.child.setFont(font, self.fonts[font])
        
    def load(self):
        self.fonts = {}
        parser = make_parser()
        parser.setContentHandler(ContentGenerator(self.fonts))
        parser.parse("symbol.xml")

        self.listbox.clear()
        for font in self.fonts:
            self.listbox.insertItem(font)
        self.listbox.sort()
            
    def save(self):
        if self.child.fontName:
            self.fonts[self.child.fontName] = self.child.fontList()
    
        f = open("symbol.xml", "w")
        print >> f, '<?xml version="1.0" encoding="iso-8859-1"?>'
        print >> f, '<table>'
        for font in self.fonts:
            print >> f, '  <unicodetable font="' + font + '">'
            for (key, number, latexName, charClass) in self.fonts[font]:
                if not charClass or charClass == '':
                    charClass = 'ORDINARY'
                print >> f,  '    <entry key="' + str(key) + \
                      '" number="' + str(number) + \
                      '" name="' + str(latexName) + \
                      '" class="' + str(charClass) + \
                      '"/>'
            
            print >> f, '  </unicodetable>'
        print >> f, '</table>'
        f.close()


class ContentGenerator(handler.ContentHandler):  
    def __init__(self, fonts):
        handler.ContentHandler.__init__(self)
        self.fonts = fonts
        self.currentFont = None

    def startElement(self, name, attrs):
        if name == 'unicodetable':
            for (name, value) in attrs.items():
                if name == "font":
                    self.currentFont = value
                    self.fonts[self.currentFont] = []
        elif name == 'entry':
            if not self.currentFont:
                raise "entry must belong to a font"
            for (name, value) in attrs.items():
                if name == "key":
                    if len(value) > 1 and value[:2] == "0x":
                        key = int(value[2:], 16)
                    else:
                        key = int(value)
                elif name == "number": number = value
                elif name == "name": latexName = value
                elif name == "class": charClass = value

            self.fonts[self.currentFont].append((key, number, latexName, charClass))
            #numberWidget, latexNameWidget, charClassWidget = self.widgets[key]
            #numberWidget.setText(number)
            #latexNameWidget.setText(latexName)
            #charClassWidget.setText(charClass)
            
    
def main():
    a = QApplication(sys.argv)

    mw = Widget()
    mw.setCaption('Unicode mapping util')
    mw.show()

    a.connect(a, SIGNAL('lastWindowClosed()'), a, SLOT('quit()'))
    a.exec_loop()

if __name__ == '__main__':

    main()
