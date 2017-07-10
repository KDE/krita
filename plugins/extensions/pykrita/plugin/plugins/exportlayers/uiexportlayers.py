from exportlayers import exportlayersdialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QAbstractItemView,
                             QDialogButtonBox, QVBoxLayout, QFrame,
                             QPushButton, QAbstractScrollArea, QSpinBox,
                             QHBoxLayout, QMessageBox, QFileDialog, QLineEdit)
from os.path import expanduser
import krita


class UIExportLayers(object):

    def __init__(self):
        self.mainDialog = exportlayersdialog.ExportLayersDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.documentLayout = QVBoxLayout()
        self.directorySelectorLayout = QHBoxLayout()

        self.refreshButton = QPushButton("Refresh")
        self.widgetDocuments = QListWidget()
        self.directoryTextField = QLineEdit()
        self.direcoryDialogButton = QPushButton("...")
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self.documentsList = []

        self.directoryTextField.setReadOnly(True)
        self.direcoryDialogButton.clicked.connect(self._selectDir)
        self.refreshButton.clicked.connect(self.refreshButtonClicked)
        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)

    def initialize(self):
        self.loadDocuments()

        self.documentLayout.addWidget(self.widgetDocuments)
        self.documentLayout.addWidget(self.refreshButton)

        self.directorySelectorLayout.addWidget(self.directoryTextField)
        self.directorySelectorLayout.addWidget(self.direcoryDialogButton)

        self.formLayout.addRow('Documents', self.documentLayout)
        self.formLayout.addRow('Initial directory', self.directorySelectorLayout)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Export Layers")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def loadDocuments(self):
        self.widgetDocuments.clear()

        self.documentsList = [document for document in self.kritaInstance.documents() if document.fileName()]

        for document in self.documentsList:
            self.widgetDocuments.addItem(document.fileName())

    def refreshButtonClicked(self):
        self.loadDocuments()

    def confirmButton(self):
        selectedPaths = [item.text() for item in self.widgetDocuments.selectedItems()]
        selectedDocuments = [document for document in self.documentsList for path in selectedPaths if path==document.fileName()]

        self.msgBox  = QMessageBox(self.mainDialog)
        if not selectedDocuments:
            self.msgBox.setText("Select one document.")
        elif not self.directoryTextField.text():
            self.msgBox.setText("Select the initial directory.")
        else:
            self.msgBox.setText("All layers has been exported.")
        self.msgBox.exec_()

    def _exportLayers(rootNode, fileFormat, parentDir):
        pass

    def _selectDir(self):
        directory = QFileDialog.getExistingDirectory(self.mainDialog, "Open a folder", expanduser("~"), QFileDialog.ShowDirsOnly)
        self.directoryTextField.setText(directory)
