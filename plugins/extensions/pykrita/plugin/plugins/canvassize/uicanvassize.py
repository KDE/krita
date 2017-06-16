from canvassize import canvassizedialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QListWidget,QAbstractItemView,
                             QDialogButtonBox, QVBoxLayout, QFrame,
                             QPushButton, QAbstractScrollArea, QSpinBox,
                             QHBoxLayout, QMessageBox)
import krita


class UICanvasSize(object):

    def __init__(self):
        self.mainDialog = canvassizedialog.CanvasSizeDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.documentLayout = QVBoxLayout()
        self.offsetLayout = QHBoxLayout()
        self.refreshButton = QPushButton("Refresh")
        self.widgetDocuments = QListWidget()
        self.widthSpinBox = QSpinBox()
        self.heightSpinBox = QSpinBox()
        self.xOffsetSpinBox = QSpinBox()
        self.yOffsetSpinBox = QSpinBox()
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self.documentsList = []

        self.refreshButton.clicked.connect(self.refreshButtonClicked)
        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)
        self.widgetDocuments.setSelectionMode(QAbstractItemView.MultiSelection)
        self.widgetDocuments.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)

    def initialize(self):
        self.loadDocuments()

        self.widthSpinBox.setRange(1, 10000)
        self.heightSpinBox.setRange(1, 10000)
        self.xOffsetSpinBox.setRange(-10000, 10000)
        self.yOffsetSpinBox.setRange(-10000, 10000)

        self.documentLayout.addWidget(self.widgetDocuments)
        self.documentLayout.addWidget(self.refreshButton)

        self.offsetLayout.addWidget(self.xOffsetSpinBox)
        self.offsetLayout.addWidget(self.yOffsetSpinBox)

        self.formLayout.addRow('Documents', self.documentLayout)
        self.formLayout.addRow('Width', self.widthSpinBox)
        self.formLayout.addRow('Height', self.heightSpinBox)
        self.formLayout.addRow('Offset', self.offsetLayout)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Canvas Size")
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
        if selectedDocuments:
            self._resizeAllDocuments(selectedDocuments)
            self.msgBox.setText("The selected documents has been resized.")
        else:
            self.msgBox.setText("Select at least one document.")
        self.msgBox.exec_()

    def _resizeAllDocuments(self, documents):
        for document in documents:
            document.resizeImage(self.xOffsetSpinBox.value(),
                                 self.yOffsetSpinBox.value(),
                                 self.widthSpinBox.value(),
                                 self.heightSpinBox.value())
