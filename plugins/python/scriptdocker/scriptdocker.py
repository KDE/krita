# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit)
import krita


class ScriptDocker(krita.DockWidget):

    def __init__(self):
        super(ScriptDocker, self).__init__()

        self.baseWidget = QWidget()
        self.layout = QVBoxLayout()
        self.scriptsLayout = QFormLayout()
        self.addButton = QPushButton(i18n("Add Script"))
        self.actions = []

        self.layout.addLayout(self.scriptsLayout)
        self.layout.addWidget(self.addButton)
        self.baseWidget.setLayout(self.layout)
        self.setWidget(self.baseWidget)

        self.setWindowTitle(i18n("Script Docker"))
        self.addButton.clicked.connect(self.addNewRow)

    def canvasChanged(self, canvas):
        pass

    def addNewRow(self):
        directorySelectorLayout = QHBoxLayout()
        directoryTextField = QLineEdit()
        directoryDialogButton = QPushButton(i18n("..."))

        directoryDialogButton.clicked.connect(self.test)

        directorySelectorLayout.addWidget(directoryTextField)
        directorySelectorLayout.addWidget(directoryDialogButton)

        self.scriptsLayout.addRow(
            str(i18n("Script {0}")).format(self.scriptsLayout.rowCount() + 1),
            directorySelectorLayout)

    def test(self):
        obj = self.sender()
        print('button', obj)

    def loadActions(self):
        pass

    def readSettings(self):
        pass

    def writeSettings(self):
        pass
