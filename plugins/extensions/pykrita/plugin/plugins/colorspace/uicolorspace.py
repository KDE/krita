from colorspace import colorspacedialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QListWidgetItem,
                             QAbstractItemView, QComboBox)
import krita


class UIColorSpace(object):
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
