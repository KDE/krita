from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit, QListWidget,
                             QScrollArea, QGridLayout, QFileDialog, QKeySequenceEdit,
                             QLabel)
from PyQt5.QtCore import QObject
import krita


class ScriptDocker(krita.DockWidget):

    def __init__(self):
       super(ScriptDocker, self).__init__()

       self.baseWidget = QWidget()
       self.layout = QVBoxLayout()
       self.scrollArea =  QScrollArea()
       self.scriptsLayout = QFormLayout()
       self.addButton = QPushButton("Add Script")
       self.actions = []
       self.baseArea = QWidget()

       self.scrollArea.setWidgetResizable(True)
       self.baseArea.setLayout(self.scriptsLayout)
       self.scrollArea.setWidget(self.baseArea)
       self.layout.addWidget(self.scrollArea)
       self.layout.addWidget(self.addButton)
       self.baseWidget.setLayout(self.layout)
       self.setWidget(self.baseWidget)

       self.setWindowTitle("Script Docker")
       self.addButton.clicked.connect(self.addNewRow)

    def canvasChanged(self, canvas):
        pass

    def addNewRow(self):
        rowPosition = self.scriptsLayout.rowCount()
        shortcutEdit = QKeySequenceEdit()
        directoryTextField = QLineEdit()
        directoryDialogButton = QPushButton("...")

        shortcutEdit.setToolTip("Shortcut (CTRL + SHIFT + 1)")
        directoryTextField.setToolTip("Selected Path")
        directoryDialogButton.setToolTip("Select the script")
        directoryDialogButton.clicked.connect(self.selectScript)

        self.scriptsLayout.addWidget(shortcutEdit)
        self.scriptsLayout.addWidget(directoryTextField)
        self.scriptsLayout.addWidget(directoryDialogButton)

    def selectScript(self):
        dialog = QFileDialog(self)
        dialog.setNameFilter('Python files (*.py)')

        if dialog.exec():
            selectedFile = dialog.selectedFiles()[0]
            print(selectedFile)

            obj = self.sender()
            print('button', obj)
            textField = self.scriptsLayout.itemAt(self.scriptsLayout.indexOf(obj)-1).widget()
            textField.setText(selectedFile)


    def loadActions(self):
        pass

    def readSettings(self):
        pass

    def writeSettings(self):
        pass


Application.addDockWidgetFactory(krita.DockWidgetFactory("scriptdocker", krita.DockWidgetFactoryBase.DockRight, ScriptDocker))
