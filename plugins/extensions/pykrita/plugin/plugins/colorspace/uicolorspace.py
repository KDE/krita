from colorspace import colorspacedialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QListWidgetItem,
                             QAbstractItemView, QComboBox, QDialogButtonBox,
                             QVBoxLayout, QFrame, QMessageBox)
import krita


class UIColorSpace(object):
    def __init__(self):
        self.mainDialog = colorspacedialog.ColorSpaceDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.widgetDocuments = QListWidget()
        self.colorModelComboBox = QComboBox()
        self.colorDepthComboBox = QComboBox()
        self.colorProfileComboBox = QComboBox()
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self.documentsList = []
        self.colorModelsList = []
        self.colorDepthsList = []
        self.colorProfilesList = []

        self.colorModelComboBox.currentTextChanged.connect(self.changedColorModelComboBox)
        self.colorDepthComboBox.currentTextChanged.connect(self.changedColorDepthComboBox)
        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSelectionMode(QAbstractItemView.MultiSelection)

    def initialize(self):
        self.loadDocuments()
        self.loadColorModels()
        self.loadColorDepths()
        self.loadColorProfiles()

        self.formLayout.addRow('Documents', self.widgetDocuments)
        self.formLayout.addRow('Color Model', self.colorModelComboBox)
        self.formLayout.addRow('Color Depth', self.colorDepthComboBox)
        self.formLayout.addRow('Color Profile', self.colorProfileComboBox)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Color Space")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def loadColorModels(self):
        self.colorModelsList = sorted(self.kritaInstance.colorModels())

        self.colorModelComboBox.addItems(self.colorModelsList)

    def loadColorDepths(self, colorModel=None):
        self.colorDepthComboBox.clear()

        if not colorModel:
            colorModel = self.colorModelComboBox.currentText()
        self.colorDepthsList = sorted(self.kritaInstance.colorDepths(colorModel))

        self.colorDepthComboBox.addItems(self.colorDepthsList)

    def loadColorProfiles(self, colorModel=None, colorDepth=None):
        self.colorProfileComboBox.clear()

        if not colorModel:
            colorModel = self.colorModelComboBox.currentText()
        if not colorDepth:
            colorDepth = self.colorDepthComboBox.currentText()

        self.colorProfilesList = sorted(self.kritaInstance.profiles(colorModel, colorDepth))
        self.colorProfileComboBox.addItems(self.colorProfilesList)

    def loadDocuments(self):
        self.widgetDocuments.clear()

        self.documentsList = [document for document in self.kritaInstance.documents() if document.fileName()]

        for document in self.documentsList:
            self.widgetDocuments.addItem(document.fileName())

    def changedColorModelComboBox(self, colorModel):
        self.loadColorDepths(colorModel=colorModel)

    def changedColorDepthComboBox(self, colorDepth):
        self.loadColorProfiles(colorDepth=colorDepth)

    def confirmButton(self):
        selectedPaths = [item.text() for item in self.widgetDocuments.selectedItems()]
        selectedDocuments = [document for document in self.documentsList for path in selectedPaths if path==document.fileName()]

        self.convertColorSpace(selectedDocuments)

        self.msgBox  = QMessageBox(self.mainDialog)
        self.msgBox.setText("The selected documents has been converted.")
        self.msgBox.exec_()


    def convertColorSpace(self, documents):
        for document in documents:
            document.setColorSpace(self.colorModelComboBox.currentText(),
                                   self.colorDepthComboBox.currentText(),
                                   self.colorProfileComboBox.currentText())
