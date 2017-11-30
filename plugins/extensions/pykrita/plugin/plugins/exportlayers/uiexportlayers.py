from exportlayers import exportlayersdialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QHBoxLayout,
                             QDialogButtonBox, QVBoxLayout, QFrame,
                             QPushButton, QAbstractScrollArea, QLineEdit,
                             QMessageBox, QFileDialog, QCheckBox, QSpinBox,
                             QComboBox)
import os
import errno
import krita


class UIExportLayers(object):

    def __init__(self):
        self.mainDialog = exportlayersdialog.ExportLayersDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.documentLayout = QVBoxLayout()
        self.directorySelectorLayout = QHBoxLayout()
        self.optionsLayout = QVBoxLayout()
        self.resolutionLayout = QHBoxLayout()

        self.refreshButton = QPushButton("Refresh")
        self.widgetDocuments = QListWidget()
        self.directoryTextField = QLineEdit()
        self.directoryDialogButton = QPushButton("...")
        self.verifyLayerFormatCheckBox = QCheckBox("Verify layers that contains [jpeg|png]")
        self.exportFilterLayersCheckBox = QCheckBox("Export filter layers")
        self.batchmodeCheckBox = QCheckBox("Export in batchmode")
        self.ignoreInvisibleLayersCheckBox = QCheckBox("Ignore invisible layers")
        self.xResSpinBox = QSpinBox()
        self.yResSpinBox = QSpinBox()
        self.formatsComboBox = QComboBox()

        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self.documentsList = []

        self.directoryTextField.setReadOnly(True)
        self.batchmodeCheckBox.setChecked(True)
        self.directoryDialogButton.clicked.connect(self._selectDir)
        self.widgetDocuments.currentRowChanged.connect(self._setResolution)
        self.refreshButton.clicked.connect(self.refreshButtonClicked)
        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)

    def initialize(self):
        self.loadDocuments()

        self.xResSpinBox.setRange(1, 10000)
        self.yResSpinBox.setRange(1, 10000)

        self.formatsComboBox.addItem("jpeg")
        self.formatsComboBox.addItem("png")

        self.documentLayout.addWidget(self.widgetDocuments)
        self.documentLayout.addWidget(self.refreshButton)

        self.directorySelectorLayout.addWidget(self.directoryTextField)
        self.directorySelectorLayout.addWidget(self.directoryDialogButton)

        self.optionsLayout.addWidget(self.verifyLayerFormatCheckBox)
        self.optionsLayout.addWidget(self.exportFilterLayersCheckBox)
        self.optionsLayout.addWidget(self.batchmodeCheckBox)
        self.optionsLayout.addWidget(self.ignoreInvisibleLayersCheckBox)

        self.resolutionLayout.addWidget(self.xResSpinBox)
        self.resolutionLayout.addWidget(self.yResSpinBox)

        self.formLayout.addRow('Documents', self.documentLayout)
        self.formLayout.addRow('Initial directory', self.directorySelectorLayout)
        self.formLayout.addRow('Export options', self.optionsLayout)
        self.formLayout.addRow('Resolution', self.resolutionLayout)
        self.formLayout.addRow('Images Extensions', self.formatsComboBox)

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
        selectedDocuments = [document for document in self.documentsList for path in selectedPaths if path == document.fileName()]

        self.msgBox = QMessageBox(self.mainDialog)
        if not selectedDocuments:
            self.msgBox.setText("Select one document.")
        elif not self.directoryTextField.text():
            self.msgBox.setText("Select the initial directory.")
        else:
            self.export(selectedDocuments[0])
            self.msgBox.setText("All layers has been exported.")
        self.msgBox.exec_()

    def mkdir(self, directory):
        try:
            os.makedirs(self.directoryTextField.text() + directory)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    def export(self, document):
        Application.setBatchmode(self.batchmodeCheckBox.isChecked())

        documentName = document.fileName() if document.fileName() else 'Untitled'
        fileName, extension = str(documentName).rsplit('/', maxsplit=1)[-1].split('.', maxsplit=1)
        self.mkdir('/' + fileName)

        self._exportLayers(document.rootNode(), self.formatsComboBox.currentText(), '/' + fileName)
        Application.setBatchmode(True)

    def _exportLayers(self, parentNode, fileFormat, parentDir):
        """ This method get all sub-nodes from the current node and export then in
            the defined format."""

        for node in parentNode.childNodes():
            newDir = ''
            if node.type() == 'grouplayer':
                newDir = parentDir + '/' + node.name()
                self.mkdir(newDir)
            elif not self.exportFilterLayersCheckBox.isChecked() and 'filter' in node.type():
                continue
            elif self.ignoreInvisibleLayersCheckBox.isChecked() and not node.visible():
                continue
            else:
                nodeName = node.name()
                _fileFormat = self.formatsComboBox.currentText()
                if '[jpeg]' in nodeName:
                    _fileFormat = 'jpeg'
                elif '[png]' in nodeName:
                    _fileFormat = 'png'

                layerFileName = '{0}{1}/{2}.{3}'.format(self.directoryTextField.text(), parentDir, node.name(), _fileFormat)
                teste = node.save(layerFileName, self.xResSpinBox.value(), self.yResSpinBox.value())

            if node.childNodes():
                self._exportLayers(node, fileFormat, newDir)

    def _selectDir(self):
        directory = QFileDialog.getExistingDirectory(self.mainDialog, "Select a folder", os.path.expanduser("~"), QFileDialog.ShowDirsOnly)
        self.directoryTextField.setText(directory)

    def _setResolution(self, index):
        document = self.documentsList[index]
        self.xResSpinBox.setValue(document.width())
        self.yResSpinBox.setValue(document.height())
