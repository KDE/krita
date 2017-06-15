# Description: A Python based docker that allows you to edit KPL color palettes.
# By Wolthera

# Importing the relevant dependancies:
import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
import math
from krita import * 

class palette_swatch_widget(QWidget):
    colorSelected = pyqtSignal(['ManagedColor'])
    
    def __init__(self, parent, color, mColor):
        super().__init__(parent)
        self.setMinimumHeight(12)
        self.setMinimumWidth(12)
        self.setSizePolicy(QSizePolicy().MinimumExpanding,QSizePolicy().MinimumExpanding)

        self.color = color
        self.mColor = mColor
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setBrush(self.color)
        painter.setPen(self.color)
        painter.drawRect(self.contentsRect())

    def mousePressEvent(self, event):
        print("Color selected: "+self.toolTip())
        self.colorSelected.emit(self.mColor)

    def sizeHint(self):
        return QSize(12,12);
        
        
class Palette_Docker(DockWidget):
#Init the docker
    def __init__(self):
        super().__init__()
        # make base-widget and layout
        widget =  QWidget()
        layout = QVBoxLayout(self)
        widget.setLayout(layout)
        self.setWindowTitle("Python Palette Docker")

        #Make a combobox and add palettes
        self.cmb_palettes = QComboBox()
        allPalettes = Application.resources("palette")
        for palette_name in allPalettes:
            self.cmb_palettes.addItem(palette_name)

        self.currentPalette = Palette(allPalettes["Default"])
        self.cmb_palettes.currentTextChanged.connect(self.slot_paletteChanged)
        layout.addWidget(self.cmb_palettes) # add combobox to the layout
        self.palette_frame = QScrollArea()
        self.palette_frame.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.palette_container = QWidget()
        self.palette_frame.setContentsMargins(0,0,0,0)
        self.palette_layout = QVBoxLayout()
        self.palette_layout.setSpacing(0)
        self.palette_layout.setContentsMargins(0,0,0,0)
        self.palette_container.setLayout(self.palette_layout)
        self.palette_frame.setWidget(self.palette_container)
        layout.addWidget(self.palette_frame)
        print("palette")
        self.fill_palette_frame()
        self.setWidget(widget)        # add widget to the docker
    
    
    def fill_palette_frame(self):
        for i in reversed(range(self.palette_layout.count())): 
            self.palette_layout.itemAt(i).widget().setParent(None)
        columnCount = self.currentPalette.columnCount()
        groupNames = self.currentPalette.groupNames()
        
        self.palette_container.setMinimumHeight(0)
        self.palette_container.setContentsMargins(0, 0, 0, 0)
        self.palette_container.setSizePolicy(QSizePolicy().Ignored,QSizePolicy().MinimumExpanding)
        
        swatchSize = math.floor(self.width()/(columnCount+2))
        if swatchSize < 12:
            swatchSize = 12
        
        self.palette_container.setFixedWidth((columnCount+1)*swatchSize)
        
        gb_defaultGroup = QWidget()
        gb_defaultGroup.setLayout(QGridLayout())
        gb_defaultGroup.layout().setSpacing(0)
        colorCount = self.currentPalette.colorsCountGroup("")
        rowsForGroup = math.ceil(colorCount/(columnCount+1))
        gb_defaultGroup.setMinimumWidth(columnCount*swatchSize)
        gb_defaultGroup.setContentsMargins(0, 0, 0, 0)
        gb_defaultGroup.setMinimumHeight(rowsForGroup*swatchSize+gb_defaultGroup.style().pixelMetric(QStyle.PM_TitleBarHeight))
        for c in range(columnCount):
            gb_defaultGroup.layout().setColumnMinimumWidth(c, swatchSize)
            
        for i in range(colorCount):
            entry = self.currentPalette.colorSetEntryFromGroup(i, "")
            color = self.currentPalette.colorForEntry(entry);
            swatch = palette_swatch_widget(self, color.colorForCanvas(self.canvas()), color)
            swatch.setToolTip(entry.name)
            column = i % columnCount
            row = math.floor(i/columnCount)
            gb_defaultGroup.layout().addWidget(swatch, row, column)
            #print("palette swatch added "+entry.name+" "+str(column)+", "+str(row))
            swatch.colorSelected.connect(self.slot_swatchSelected)
        self.palette_layout.addWidget(gb_defaultGroup)
        self.palette_container.setMinimumHeight(self.palette_container.minimumHeight()+gb_defaultGroup.minimumHeight())
        
        for groupName in groupNames:
            gb_groupBox = QGroupBox()
            gb_groupBox.setTitle(groupName)
            gb_groupBox.setLayout(QGridLayout())
            gb_groupBox.setFlat(True)
            gb_groupBox.setFixedWidth((columnCount)*swatchSize)
            colorCount = self.currentPalette.colorsCountGroup(groupName)
            rowsForGroup = math.ceil(colorCount/(columnCount+1))
            for c in range(columnCount):
                gb_groupBox.layout().setColumnMinimumWidth(c, swatchSize)
            gb_groupBox.layout().setSpacing(0)
            gb_groupBox.setContentsMargins(0, 0, 0, 0)
            
            for i in range(colorCount):
                entry = self.currentPalette.colorSetEntryFromGroup(i, groupName)
                color = self.currentPalette.colorForEntry(entry);
                swatch = palette_swatch_widget(self, color.colorForCanvas(self.canvas()), color)
                swatch.setToolTip(entry.name)
                swatch.setFixedHeight(swatchSize)
                column = i % columnCount
                row = math.floor(i/columnCount)
                gb_groupBox.layout().addWidget(swatch, row, column)
                #print("palette swatch added "+entry.name+" "+str(column)+", "+str(row))
                swatch.colorSelected.connect(self.slot_swatchSelected)
            
            self.palette_layout.addWidget(gb_groupBox)
            gb_groupBox.adjustSize()
            self.palette_container.setMinimumHeight(self.palette_container.minimumHeight()+gb_groupBox.height())
        self.palette_container.setMaximumHeight(self.palette_container.minimumHeight())
        
    def slot_paletteChanged(self, name):
        self.currentPalette = Palette(Application.resources("palette")[name])
        self.fill_palette_frame()

    @pyqtSlot('ManagedColor')
    def slot_swatchSelected(self, color):
        print("color "+color.toQString())
        if (self.canvas()) is not None:
            if (self.canvas().view()) is not None:
                self.canvas().view().setForeGroundColor(color)

    def canvasChanged(self, canvas):
        self.fill_palette_frame()
        pass

#Add docker to the application :)
Application.addDockWidgetFactory(DockWidgetFactory("palette_docker", DockWidgetFactoryBase.DockRight, Palette_Docker))

