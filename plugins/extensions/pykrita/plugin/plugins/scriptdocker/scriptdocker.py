from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit)
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
       self.addButton.clicked.connect(self.addEmptyForm)

    def canvasChanged(self, canvas):
        pass

    def addEmptyForm(self):
        directorySelectorLayout = QHBoxLayout()
        directoryTextField = QLineEdit()
        directoryDialogButton = QPushButton("...")

        directorySelectorLayout.addWidget(directoryTextField)
        directorySelectorLayout.addWidget(directoryDialogButton)

        self.scriptsLayout.addRow("Script {0}".format(self.scriptsLayout.rowCount() + 1), directorySelectorLayout)


    def loadActions(self):
        pass

    def readSettings(self):
        pass

    def writeSettings(self):
        pass


Application.addDockWidgetFactory(krita.DockWidgetFactory("scriptdocker", krita.DockWidgetFactoryBase.DockRight, ScriptDocker))
