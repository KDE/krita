from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit, QListWidget)
from PyQt5.QtCore import QObject
import krita


class ScriptDocker(krita.DockWidget):

    def __init__(self):
        super(ScriptDocker, self).__init__()

        self.baseWidget = QWidget()
        self.layout = QVBoxLayout()
        self.scriptsLayout = QFormLayout()
        self.addButton = QPushButton("Add Script")
        self.actions = []

        self.layout.addLayout(self.scriptsLayout)
        self.layout.addWidget(self.addButton)
        self.baseWidget.setLayout(self.layout)
        self.setWidget(self.baseWidget)

        self.setWindowTitle("Script Docker")
        self.addButton.clicked.connect(self.addNewRow)

    def canvasChanged(self, canvas):
        pass

    def addNewRow(self):
        directorySelectorLayout = QHBoxLayout()
        directoryTextField = QLineEdit()
        directoryDialogButton = QPushButton("...")

        directoryDialogButton.clicked.connect(self.test)

        directorySelectorLayout.addWidget(directoryTextField)
        directorySelectorLayout.addWidget(directoryDialogButton)

        self.scriptsLayout.addRow("Script {0}".format(self.scriptsLayout.rowCount() + 1), directorySelectorLayout)

    def test(self):
        obj = self.sender()
        print('button', obj)

    def loadActions(self):
        pass

    def readSettings(self):
        pass

    def writeSettings(self):
        pass


Application.addDockWidgetFactory(krita.DockWidgetFactory("scriptdocker", krita.DockWidgetFactoryBase.DockRight, ScriptDocker))
