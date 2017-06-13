from filtermanager import filtermanagerdialog
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QAbstractItemView, QDialogButtonBox,
                             QVBoxLayout, QFrame, QAbstractScrollArea)
import krita


class UIFilterManager(object):

    def __init__(self):
        self.mainDialog = filtermanagerdialog.FilterManagerDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)

    def initialize(self):
        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Filter Manager")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def confirmButton(self):
        pass
