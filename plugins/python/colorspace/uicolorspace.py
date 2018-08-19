'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
from . import colorspacedialog
from .components import colormodelcombobox, colordepthcombobox, colorprofilecombobox
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QListWidgetItem,
                             QAbstractItemView, QComboBox, QDialogButtonBox,
                             QVBoxLayout, QFrame, QMessageBox, QPushButton,
                             QHBoxLayout, QAbstractScrollArea)
from PyQt5.QtGui import QIcon
import krita
from . import resources_rc


class UIColorSpace(object):

    def __init__(self):
        self.mainDialog = colorspacedialog.ColorSpaceDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.documentLayout = QVBoxLayout()
        self.refreshButton = QPushButton(QIcon(':/icons/refresh.svg'), i18n("Refresh"))
        self.widgetDocuments = QListWidget()
        self.colorModelComboBox = colormodelcombobox.ColorModelComboBox(self)
        self.colorDepthComboBox = colordepthcombobox.ColorDepthComboBox(self)
        self.colorProfileComboBox = colorprofilecombobox.ColorProfileComboBox(self)
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self.documentsList = []
        self.colorModelsList = []
        self.colorDepthsList = []
        self.colorProfilesList = []

        self.refreshButton.clicked.connect(self.refreshButtonClicked)
        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSelectionMode(QAbstractItemView.MultiSelection)
        self.widgetDocuments.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)

    def initialize(self):
        self.loadDocuments()
        self.loadColorModels()
        self.loadColorDepths()
        self.loadColorProfiles()

        self.documentLayout.addWidget(self.widgetDocuments)
        self.documentLayout.addWidget(self.refreshButton)

        self.formLayout.addRow(i18n("Documents:"), self.documentLayout)
        self.formLayout.addRow(i18n("Color model:"), self.colorModelComboBox)
        self.formLayout.addRow(i18n("Color depth:"), self.colorDepthComboBox)
        self.formLayout.addRow(i18n("Color profile:"), self.colorProfileComboBox)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle(i18n("Color Space"))
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def loadColorModels(self):
        self.colorModelsList = sorted(self.kritaInstance.colorModels())

        self.colorModelComboBox.addItems(self.colorModelsList)

    def loadColorDepths(self):
        self.colorDepthComboBox.clear()

        colorModel = self.colorModelComboBox.currentText()
        self.colorDepthsList = sorted(self.kritaInstance.colorDepths(colorModel))

        self.colorDepthComboBox.addItems(self.colorDepthsList)

    def loadColorProfiles(self):
        self.colorProfileComboBox.clear()

        colorModel = self.colorModelComboBox.currentText()
        colorDepth = self.colorDepthComboBox.currentText()
        self.colorProfilesList = sorted(self.kritaInstance.profiles(colorModel, colorDepth))

        self.colorProfileComboBox.addItems(self.colorProfilesList)

    def loadDocuments(self):
        self.widgetDocuments.clear()

        self.documentsList = [document for document in self.kritaInstance.documents() if document.fileName()]

        for document in self.documentsList:
            self.widgetDocuments.addItem(document.fileName())

    def refreshButtonClicked(self):
        self.loadDocuments()

    def confirmButton(self):
        selectedPaths = [item.text() for item in self.widgetDocuments.selectedItems()]
        selectedDocuments = [document for document in self.documentsList for path in selectedPaths if path == document.fileName()]

        self.msgBox = QMessageBox(self.mainDialog)
        if selectedDocuments:
            self.convertColorSpace(selectedDocuments)
            self.msgBox.setText(i18n("The selected documents has been converted."))
        else:
            self.msgBox.setText(i18n("Select at least one document."))
        self.msgBox.exec_()

    def convertColorSpace(self, documents):
        for document in documents:
            document.setColorSpace(self.colorModelComboBox.currentText(),
                                   self.colorDepthComboBox.currentText(),
                                   self.colorProfileComboBox.currentText())
