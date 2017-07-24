# Description: A Python based docker that allows you to edit KPL color palettes.
# By Wolthera

# Importing the relevant dependancies:
import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.Qt import *
import math
from krita import * 

class Palette_Docker(DockWidget):
#Init the docker
    def __init__(self):
        super().__init__()
        # make base-widget and layout
        widget =  QWidget()
        layout = QVBoxLayout()
        buttonLayout = QHBoxLayout()
        widget.setLayout(layout)
        self.setWindowTitle("Python Palette Docker")

        #Make a combobox and add palettes
        self.cmb_palettes = QComboBox()
        allPalettes = Application.resources("palette")
        for palette_name in allPalettes:
            self.cmb_palettes.addItem(palette_name)

        self.currentPalette = Palette(allPalettes[list(allPalettes.keys())[0]])
        self.cmb_palettes.currentTextChanged.connect(self.slot_paletteChanged)
        layout.addWidget(self.cmb_palettes) # add combobox to the layout
        self.paletteView = PaletteView()
        self.paletteView.setPalette(self.currentPalette)
        layout.addWidget(self.paletteView)
        self.paletteView.entrySelectedForeGround.connect(self.slot_swatchSelected)
        
        self.colorComboBox = QComboBox()
        self.colorList = list()
        buttonLayout.addWidget(self.colorComboBox)
        self.addEntry = QPushButton()
        self.addEntry.setText("A")
        self.addEntry.setToolTip("Add Entry")
        self.addEntry.clicked.connect(self.slot_add_entry)
        buttonLayout.addWidget(self.addEntry)
        self.addGroup = QPushButton()
        self.addGroup.clicked.connect(self.slot_add_group)
        self.addGroup.setText("G")
        self.addGroup.setToolTip("Add Group")
        buttonLayout.addWidget(self.addGroup)
        self.removeEntry = QPushButton()
        self.removeEntry.setText("R")
        self.removeEntry.setToolTip("Remove Entry")
        self.removeEntry.clicked.connect(self.slot_remove_entry)
        buttonLayout.addWidget(self.removeEntry)
        
        layout.addLayout(buttonLayout)
        self.slot_fill_combobox()
        self.setWidget(widget)        # add widget to the docker
    
    def slot_paletteChanged(self, name):
        self.currentPalette = Palette(Application.resources("palette")[name])
        self.paletteView.setPalette(self.currentPalette)
        self.slot_fill_combobox()
        #self.fill_palette_frame()

    @pyqtSlot('KoColorSetEntry')
    def slot_swatchSelected(self, entry):
        print("entry "+entry.name)
        if (self.canvas()) is not None:
            if (self.canvas().view()) is not None:
                name = entry.name
                if len(entry.id)>0:
                    name = entry.id+" - "+entry.name
                if len(name)>0:
                    if name in self.colorList:
                        self.colorComboBox.setCurrentIndex(self.colorList.index(name))
                color = self.currentPalette.colorForEntry(entry)
                self.canvas().view().setForeGroundColor(color)
    
    def slot_fill_combobox(self):
        if self.currentPalette is None:
            pass
        palette = self.currentPalette
        self.colorComboBox.clear()
        self.colorList.clear()
        for i in range(palette.colorsCountTotal()):
            entry = palette.colorSetEntryByIndex(i)
            color = palette.colorForEntry(entry).colorForCanvas(self.canvas())
            colorSquare = QPixmap(12, 12)
            if entry.spotColor is True:
                img = colorSquare.toImage()
                circlePainter = QPainter()
                img.fill(self.colorComboBox.palette().color(QPalette.Base))
                circlePainter.begin(img)
                brush = QBrush(Qt.SolidPattern)
                brush.setColor(color)
                circlePainter.setBrush(brush)
                circlePainter.drawEllipse(0, 0, 11, 11)
                circlePainter.end()
                colorSquare = QPixmap.fromImage(img)
            else:
                colorSquare.fill(color)
            name = entry.name
            if len(entry.id)>0:
                name = entry.id+" - "+entry.name
            self.colorList.append(name)
            self.colorComboBox.addItem(QIcon(colorSquare), name)
        self.colorComboBox.setEditable(True)
        self.colorComboBox.setInsertPolicy(QComboBox.NoInsert)
        self.colorComboBox.completer().setCompletionMode(QCompleter.PopupCompletion)
        self.colorComboBox.completer().setCaseSensitivity(False)
        self.colorComboBox.completer().setFilterMode(Qt.MatchContains)
        self.colorComboBox.currentIndexChanged.connect(self.slot_get_color_from_combobox)

    def slot_get_color_from_combobox(self):
        if self.currentPalette is not None:
            entry = self.currentPalette.colorSetEntryByIndex(self.colorComboBox.currentIndex())
            self.slot_swatchSelected(entry)
            
    def slot_add_entry(self):
        if (self.canvas()) is not None:
            if (self.canvas().view()) is not None:
                color = self.canvas().view().foreGroundColor()
                succes = self.paletteView.addEntryWithDialog(color)
                if succes is True:
                    self.slot_fill_combobox()
    
    def slot_add_group(self):
        succes = self.paletteView.addGroupWithDialog()
        if succes is True:
            self.slot_fill_combobox()
               
    def slot_remove_entry(self):
        succes = self.paletteView.removeSelectedEntryWithDialog()
        if succes is True:
            self.slot_fill_combobox()
                
    def canvasChanged(self, canvas):
        pass

#Add docker to the application :)
Application.addDockWidgetFactory(DockWidgetFactory("palette_docker", DockWidgetFactoryBase.DockRight, Palette_Docker))

