from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit, QListWidget,
                             QScrollArea, QGridLayout, QFileDialog, QKeySequenceEdit,
                             QLabel)
from PyQt5.QtCore import QObject, Qt
import krita


class ScriptDocker(krita.DockWidget):

    def __init__(self):
       super(ScriptDocker, self).__init__()

       self.baseWidget = QWidget()
       self.layout = QVBoxLayout()
       self.scrollArea =  QScrollArea()
       self.scriptsLayout = QGridLayout()
       self.addButton = QPushButton("Add Script")
       self.actions = []
       self.scripts = []
       self.baseArea = QWidget()

       self.initialize()

    def initialize(self):
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

    def selectScript(self):
        dialog = QFileDialog(self)
        dialog.setNameFilter('Python files (*.py)')

        if dialog.exec():
            selectedFile = dialog.selectedFiles()[0]
            obj = self.sender()
            textField = self.scriptsLayout.itemAt(self.scriptsLayout.indexOf(obj)-1).widget()
            textField.setText(selectedFile)

    def loadActions(self):
        pass

    def readSettings(self):
        self.scripts = Application.readSetting("scriptdocker", "scripts", "").split(',')
        self.shortcuts = Application.readSetting("scriptdocker", "shortcuts", "").split(',')

    def writeSettings(self):
        scripts = []
        shortcuts = []

        for row in range(self.scriptsLayout.rowCount()):
            shortcutWidget = self.scriptsLayout.itemAtPosition(row, 0).widget()
            textField = self.scriptsLayout.itemAtPosition(row, 1).widget()
            self.actions[row].script = textField.text()
            self.actions[row].shortcut = textField.text()
            presets.append(button.preset)

        Application.writeSetting("scriptdocker", "scripts", ','.join(map(str, scripts)))
        Application.writeSetting("scriptdocker", "shortcuts", ','.join(map(str, shortcuts)))



Application.addDockWidgetFactory(krita.DockWidgetFactory("scriptdocker", krita.DockWidgetFactoryBase.DockRight, ScriptDocker))
