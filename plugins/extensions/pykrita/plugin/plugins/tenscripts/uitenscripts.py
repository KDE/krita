from PyQt5.QtCore import Qt, QSize
from PyQt5.QtGui import QPixmap, QIcon
from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit, QListWidget,
                             QScrollArea, QGridLayout, QFileDialog, QKeySequenceEdit,
                             QLabel, QAction, QDialogButtonBox)
from tenscripts import tenscriptsdialog
import krita


class UITenScripts(object):

    def __init__(self):
        self.kritaInstance = krita.Krita.instance()
        self.mainDialog = tenscriptsdialog.TenScriptsDialog(self, self.kritaInstance.activeWindow().qwindow())

        self.buttonBox = QDialogButtonBox(self.mainDialog)
        self.layout = QVBoxLayout(self.mainDialog)
        self.baseWidget = QWidget()
        self.baseArea = QWidget()
        self.scrollArea =  QScrollArea()
        self.scriptsLayout = QGridLayout()

        self.buttonBox.accepted.connect(self.mainDialog.accept)
        self.buttonBox.rejected.connect(self.mainDialog.reject)

        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.scrollArea.setWidgetResizable(True)

    def initialize(self, tenscripts):
        self.tenscripts = tenscripts

        self.loadGridLayout()

        self.baseArea.setLayout(self.scriptsLayout)
        self.scrollArea.setWidget(self.baseArea)

        self.layout.addWidget(self.scrollArea)
        self.layout.addWidget(self.buttonBox)    

        self.mainDialog.show()
        self.mainDialog.activateWindow()
        self.mainDialog.exec_()

    def addNewRow(self):
        rowPosition = self.scriptsLayout.rowCount()
        rowLayout = QHBoxLayout()
        shortcutEdit = QKeySequenceEdit()
        directoryTextField = QLineEdit()
        directoryDialogButton = QPushButton("...")

        directoryTextField.setReadOnly(True)
        shortcutEdit.setToolTip("Shortcut Ex:CTRL + SHIFT + 1")
        directoryTextField.setToolTip("Selected Path")
        directoryDialogButton.setToolTip("Select the script")
        directoryDialogButton.clicked.connect(self.selectScript)

        self.scriptsLayout.addWidget(shortcutEdit, rowPosition, 0, Qt.AlignLeft|Qt.AlignTop)
        self.scriptsLayout.addWidget(directoryTextField, rowPosition, 1, Qt.AlignLeft|Qt.AlignTop)
        self.scriptsLayout.addWidget(directoryDialogButton, rowPosition, 2, Qt.AlignLeft|Qt.AlignTop)

    def loadGridLayout(self):
        pass
