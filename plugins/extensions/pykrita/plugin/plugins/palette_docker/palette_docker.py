# Description: A Python based docker that allows you to edit KPL color palettes.
# By Wolthera

# Importing the relevant dependancies:
import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.Qt import *
import math
from krita import * 

#import the exporters
from . import palette_exporter_gimppalette, palette_exporter_inkscapeSVG

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
        
        #QActions
        self.extra = QToolButton()
        self.editPaletteData = QAction()
        self.editPaletteData.setText("Edit Palette Settings")
        self.editPaletteData.triggered.connect(self.slot_edit_palette_data)
        self.extra.setDefaultAction(self.editPaletteData)
        buttonLayout.addWidget(self.extra)
        
        self.actionMenu = QMenu()
        self.exportToGimp = QAction()
        self.exportToGimp.setText("Export as GIMP palette file.")
        self.exportToGimp.triggered.connect(self.slot_export_to_gimp_palette)
        self.exportToInkscape = QAction()
        self.exportToInkscape.setText("Export as Inkscape SVG with swatches.")
        self.exportToInkscape.triggered.connect(self.slot_export_to_inkscape_svg)
        self.actionMenu.addAction(self.editPaletteData)
        self.actionMenu.addAction(self.exportToGimp)
        self.actionMenu.addAction(self.exportToInkscape)
        
        self.extra.setMenu(self.actionMenu)
        
        layout.addLayout(buttonLayout)
        self.slot_fill_combobox()
        self.setWidget(widget)        # add widget to the docker
    
    def slot_paletteChanged(self, name):
        self.currentPalette = Palette(Application.resources("palette")[name])
        self.paletteView.setPalette(self.currentPalette)
        self.slot_fill_combobox()

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
    '''
    A function for making a combobox with the available colors. We use QCompleter on the colorComboBox so that people
    can type in the name of a color to select it. This is useful for people with carefully made palettes where the colors
    are named properly, which makes it easier for them to find colors.
    '''
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
            
    '''
    A function for giving a gui to edit palette metadata... I also want this to be the way to edit the settings of the
    palette docker.
    '''
    def slot_edit_palette_data(self):
        dialog = QDialog(self)
        tabWidget = QTabWidget()
        dialog.setWindowTitle("Edit Palette Data")
        dialog.setLayout(QVBoxLayout())
        dialog.layout().addWidget(tabWidget)
        paletteWidget = QWidget()
        paletteWidget.setLayout(QVBoxLayout())
        tabWidget.addTab(paletteWidget, "Palette Data")
        paletteName = QLineEdit()
        paletteName.setText(self.cmb_palettes.currentText())
        paletteWidget.layout().addWidget(paletteName)
        paletteColumns = QSpinBox()
        paletteColumns.setValue(self.currentPalette.columnCount())
        paletteWidget.layout().addWidget(paletteColumns)
        paletteComment = QPlainTextEdit()
        paletteComment.appendPlainText(self.currentPalette.comment())
        paletteWidget.layout().addWidget(paletteComment)
        buttons = QDialogButtonBox(QDialogButtonBox.Ok)
        dialog.layout().addWidget(buttons)
        buttons.accepted.connect(dialog.accept)
        #buttons.rejected.connect(dialog.reject())
        
        if dialog.exec_()==QDialog.Accepted:
            Resource = Application.resources("palette")[self.cmb_palettes.currentText()]
            Resource.setName(paletteName.text());
            self.currentPalette = Palette(Resource)
            print(paletteColumns.value())
            self.currentPalette.setColumnCount(paletteColumns.value())
            self.paletteView.setPalette(self.currentPalette)
            self.slot_fill_combobox()
            self.currentPalette.setComment(paletteComment.toPlainText())
            
    def slot_export_to_gimp_palette(self):
        palette_exporter_gimppalette.gimpPaletteExporter(self.cmb_palettes.currentText())
        
    def slot_export_to_inkscape_svg(self):
        palette_exporter_inkscapeSVG.inkscapeSVGExporter(self.cmb_palettes.currentText())
                
    def canvasChanged(self, canvas):
        pass

#Add docker to the application :)
Application.addDockWidgetFactory(DockWidgetFactory("palette_docker", DockWidgetFactoryBase.DockRight, Palette_Docker))

