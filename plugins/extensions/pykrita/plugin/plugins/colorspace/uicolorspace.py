from colorspace import colorspacedialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QListWidgetItem,
                             QAbstractItemView, QComboBox)
import krita


class UIColorSpace(object):
    """ @param colorModel A string describing the color model of the image:
        * <ul>
        * <li>A: Alpha mask</li>
        * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
        * <li>XYZA: XYZ with alpha channel</li>
        * <li>LABA: LAB with alpha channel</li>
        * <li>CMYKA: CMYK with alpha channel</li>
        * <li>GRAYA: Gray with alpha channel</li>
        * <li>YCbCrA: YCbCr with alpha channel</li>
        * </ul>
        * @param colorDepth A string describing the color depth of the image:
        * <ul>
        * <li>U8: unsigned 8 bits integer, the most common type</li>
        * <li>U16: unsigned 16 bits integer</li>
        * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
        * <li>F32: 32 bits floating point</li>
        * </ul>
        * @param colorProfile a valid color profile for this color model and color depth combination.
        * @return false the combination of these arguments does not correspond to a colorspace."""
    def __init__(self):
        self.mainDialog = colorspacedialog.ColorSpaceDialog()
        self.layout = QFormLayout(self.mainDialog)
        self.widgetDocuments = QListWidget()
        self.colorModelComboBox = QComboBox()
        self.colorDepthComboBox = QComboBox()
        self.colorProfileComboBox = QComboBox()

        self.listDocuments = []

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSelectionMode(QAbstractItemView.MultiSelection)

    def initialize(self):
        self.reloadDocuments()

        self.layout.addRow('Documents', self.widgetDocuments)
        self.layout.addRow('Color Model', self.colorModelComboBox)
        self.layout.addRow('Color Depth', self.colorDepthComboBox)
        self.layout.addRow('Color Profile', self.colorProfileComboBox)

        self.mainDialog.resize(400, 500)
        self.mainDialog.setWindowTitle("Color Space")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def reloadDocuments(self):
        self.widgetDocuments.clear()

        instance = krita.Krita.instance()
        self.listDocuments = [document for document in instance.documents() if document.fileName()]

        for document in self.listDocuments:
            self.widgetDocuments.addItem(document.fileName())
