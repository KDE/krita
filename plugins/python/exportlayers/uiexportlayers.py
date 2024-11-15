# SPDX-License-Identifier: CC0-1.0

from . import exportlayersdialog
from PyQt5.QtCore import (Qt, QRect)
from PyQt5.QtWidgets import (QFormLayout, QListWidget, QHBoxLayout,
                             QDialogButtonBox, QVBoxLayout, QFrame,
                             QPushButton, QAbstractScrollArea, QLineEdit,
                             QMessageBox, QFileDialog, QCheckBox, QSpinBox,
                             QComboBox, QListWidgetItem)
import os
import krita


class UIExportLayers(object):

    def __init__(self):
        self.mainDialog = exportlayersdialog.ExportLayersDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.resSpinBoxLayout = QFormLayout()
        self.documentLayout = QVBoxLayout()
        self.directorySelectorLayout = QHBoxLayout()
        self.optionsLayout = QVBoxLayout()
        self.rectSizeLayout = QHBoxLayout()

        self.refreshButton = QPushButton(i18n("Refresh"))
        self.widgetDocuments = QListWidget()
        self.directoryTextField = QLineEdit()
        self.directoryDialogButton = QPushButton(i18n("..."))
        self.exportFilterLayersCheckBox = QCheckBox(
            i18n("Export filter layers"))
        self.batchmodeCheckBox = QCheckBox(i18n("Export in batchmode"))
        self.groupAsLayer = QCheckBox(i18n("Group as layer"))
        self.ignoreInvisibleLayersCheckBox = QCheckBox(
            i18n("Ignore invisible layers"))
        self.cropToImageBounds = QCheckBox(
                i18n("Adjust export size to layer content"))
        self.addIncrementPrefixCheckBox = QCheckBox(
            i18n("Add incrementing prefix"))

        self.rectWidthSpinBox = QSpinBox()
        self.rectHeightSpinBox = QSpinBox()
        self.formatsComboBox = QComboBox()
        self.resSpinBox = QSpinBox()

        self.buttonBox = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self.documentsList = []

        self.directoryTextField.setReadOnly(True)
        self.batchmodeCheckBox.setChecked(True)
        self.directoryDialogButton.clicked.connect(self._selectDir)
        self.widgetDocuments.currentRowChanged.connect(self._setResolution)
        self.widgetDocuments.setSelectionMode(3) #multi-selection
        self.widgetDocuments.setMouseTracking(True) #enable ToolTips to show the paths
        self.refreshButton.clicked.connect(self.refreshButtonClicked)
        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)
        self.cropToImageBounds.stateChanged.connect(self._toggleCropSize)

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSizeAdjustPolicy(
            QAbstractScrollArea.AdjustToContents)

    def initialize(self):
        self.loadDocuments()

        self.rectWidthSpinBox.setRange(1, 10000)
        self.rectHeightSpinBox.setRange(1, 10000)
        self.resSpinBox.setRange(20, 1200)

        self.formatsComboBox.addItem(i18n("JPEG"))
        self.formatsComboBox.addItem(i18n("PNG"))

        self.documentLayout.addWidget(self.widgetDocuments)
        self.documentLayout.addWidget(self.refreshButton)

        self.directorySelectorLayout.addWidget(self.directoryTextField)
        self.directorySelectorLayout.addWidget(self.directoryDialogButton)

        self.optionsLayout.addWidget(self.exportFilterLayersCheckBox)
        self.optionsLayout.addWidget(self.batchmodeCheckBox)
        self.optionsLayout.addWidget(self.groupAsLayer)
        self.optionsLayout.addWidget(self.ignoreInvisibleLayersCheckBox)
        self.optionsLayout.addWidget(self.cropToImageBounds)
        self.optionsLayout.addWidget(self.addIncrementPrefixCheckBox)

        self.resSpinBoxLayout.addRow(i18n("dpi:"), self.resSpinBox)

        self.rectSizeLayout.addWidget(self.rectWidthSpinBox)
        self.rectSizeLayout.addWidget(self.rectHeightSpinBox)
        self.rectSizeLayout.addLayout(self.resSpinBoxLayout)

        self.formLayout.addRow(i18n("Documents:"), self.documentLayout)
        self.formLayout.addRow(
            i18n("Initial directory:"), self.directorySelectorLayout)
        self.formLayout.addRow(i18n("Export options:"), self.optionsLayout)
        self.formLayout.addRow(i18n("Export size:"), self.rectSizeLayout)
        self.formLayout.addRow(
            i18n("Images extensions:"), self.formatsComboBox)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)
        
        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle(i18n("Export Layers"))
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def loadDocuments(self):
        self.widgetDocuments.clear()

        self.documentsList = [
            document for document in self.kritaInstance.documents()
        ]

        currentDoc = None
        if self.kritaInstance.activeDocument():
            currentDoc = self.kritaInstance.activeDocument().fileName()
        
        activeDoc = 0
        docCount = 0
        for document in self.documentsList:
            fullName = document.fileName()
            if document.name(): #if you open a file that isn't a .kra file it won't have a name property...
                shortName = document.name()
            else:
                shortName = os.path.basename(document.fileName()) #... so just get the name from the file using os.path.basename()
            newListItem= QListWidgetItem(shortName)
            newListItem.setToolTip(fullName) # ... show the full path as a ToolTip, rather than in the list, better UX
            if fullName == currentDoc:
                activeDoc = docCount
            self.widgetDocuments.addItem(newListItem)
            docCount += 1
        if self.widgetDocuments.count():
            self.widgetDocuments.setCurrentItem(self.widgetDocuments.item(activeDoc))
        
    def refreshButtonClicked(self):
        self.loadDocuments()

    def confirmButton(self):
        selectedPaths = [
            item.toolTip() for item in self.widgetDocuments.selectedItems()]
        selectedDocuments = [
            document for document in self.documentsList
            for path in selectedPaths if path == document.fileName()
        ]
        self.msgBox = QMessageBox(self.mainDialog)

        if not selectedDocuments:
            self.msgBox.setText(i18n("Select one document."))
        elif not self.directoryTextField.text():
            self.msgBox.setText(i18n("Select the initial directory."))
        else:
            for doc in selectedDocuments:
                self.export(doc)
            self.msgBox.setText(i18n("All layers have been exported."))
        self.msgBox.exec_()

    def mkdir(self, directory):
        target_directory = self.directoryTextField.text() + directory
        if (os.path.exists(target_directory)
                and os.path.isdir(target_directory)):
            return

        try:
            os.makedirs(target_directory)
        except OSError as e:
            raise e

    def export(self, document):
        Application.setBatchmode(self.batchmodeCheckBox.isChecked())

        documentName = document.fileName() if document.fileName() else 'Untitled'  # noqa: E501
        fileName, extension = os.path.splitext(os.path.basename(documentName))
        self.mkdir('/' + fileName)

        self._exportLayers(
            document.rootNode(),
            self.formatsComboBox.currentText(),
            '/' + fileName)
        Application.setBatchmode(True)

    def _exportLayers(self, parentNode, fileFormat, parentDir):
        """ This method get all sub-nodes from the current node and export then in
            the defined format."""
        prefixNum = 0
        for node in parentNode.childNodes():
            newDir = ''
            nodeName = node.name()
            if self.addIncrementPrefixCheckBox.isChecked():
                nodeName = str(prefixNum) + "-" + nodeName

            if node.type() == 'grouplayer' and not self.groupAsLayer.isChecked():
                newDir = os.path.join(parentDir, nodeName)
                self.mkdir(newDir)
            elif (not self.exportFilterLayersCheckBox.isChecked()
                  and 'filter' in node.type()):
                continue
            elif (self.ignoreInvisibleLayersCheckBox.isChecked()
                  and not node.visible()):
                continue
            else:
                _fileFormat = self.formatsComboBox.currentText()
                if '[jpeg]' in nodeName:
                    _fileFormat = 'jpeg'
                elif '[png]' in nodeName:
                    _fileFormat = 'png'

                if self.cropToImageBounds.isChecked():
                    bounds = QRect()
                else:
                    bounds = QRect(0, 0, self.rectWidthSpinBox.value(), self.rectHeightSpinBox.value())

                layerFileName = '{0}{1}/{2}.{3}'.format(
                    self.directoryTextField.text(),
                    parentDir, nodeName, _fileFormat)
                node.save(layerFileName, self.resSpinBox.value() / 72.,
                          self.resSpinBox.value() / 72., krita.InfoObject(), bounds)
            prefixNum += 1
            if node.childNodes() and not self.groupAsLayer.isChecked():
                self._exportLayers(node, fileFormat, newDir)

    def _selectDir(self):
        directory = QFileDialog.getExistingDirectory(
            self.mainDialog,
            i18n("Select a Folder"),
            os.path.expanduser("~"),
            QFileDialog.ShowDirsOnly)
        self.directoryTextField.setText(directory)

    def _setResolution(self, index):
        document = self.documentsList[index]
        self.rectWidthSpinBox.setValue(document.width())
        self.rectHeightSpinBox.setValue(document.height())
        self.resSpinBox.setValue(document.resolution())

    def _toggleCropSize(self):
        cropToLayer = self.cropToImageBounds.isChecked()
        self.rectWidthSpinBox.setDisabled(cropToLayer)
        self.rectHeightSpinBox.setDisabled(cropToLayer)
