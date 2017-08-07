from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit, QListWidget,
                             QScrollArea, QGridLayout, QFileDialog, QKeySequenceEdit,
                             QLabel, QAction, QDialogButtonBox)
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
       self.baseArea = QWidget()
       self.buttonBox = QDialogButtonBox(self)

       self.actions = []
       self.scripts = []

       self.initialize()

    def initialize(self):
        self.buttonBox.accepted.connect(self._accept)
        self.buttonBox.rejected.connect(self.close)
        self.addButton.clicked.connect(self.addNewRow)

        self.baseArea.setLayout(self.scriptsLayout)
        self.scrollArea.setWidget(self.baseArea)

        self.layout.addWidget(self.scrollArea)
        self.layout.addWidget(self.addButton)
        self.layout.addWidget(self.buttonBox)

        self.baseWidget.setLayout(self.layout)
        self.setWidget(self.baseWidget)

        self.scrollArea.setWidgetResizable(True)
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.setWindowTitle("Script Docker")

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

    def _accept(self):
        self.writeSettings()
        self.close()

    def _executeScript(self):
        print('script executed')

    def readSettings(self):
        self.scripts = Application.readSetting("scriptdocker", "scripts", "").split(',')
        self.shortcuts = Application.readSetting("scriptdocker", "shortcuts", "").split(',')

    def writeSettings(self):
        self.actions = []
        shortcuts = []
        scripts = []
        index = 0

        for row in range(self.scriptsLayout.rowCount()-1):
            print('index', index)
            shortcutWidget = self.scriptsLayout.itemAt(index).widget()
            textField = self.scriptsLayout.itemAt(index + 1).widget()

            if shortcutWidget.keySequence() and textField.text():
                action = Application.createAction("execute_script_" + str(row), "Execute Script " + str(row))
                action.setMenu("None")
                action.triggered.connect(self._executeScript)
                action.setShortcut("CTRL+SHIFT+1")
                action.script = textField.text()

                shortcuts.append(shortcutWidget.keySequence().toString())
                scripts.append(textField.text())
                self.actions.append(action)

            index += 3

        Application.writeSetting("scriptdocker", "scripts", ','.join(map(str, scripts)))
        Application.writeSetting("scriptdocker", "shortcuts", ','.join(map(str, shortcuts)))


Application.addDockWidgetFactory(krita.DockWidgetFactory("scriptdocker", krita.DockWidgetFactoryBase.DockRight, ScriptDocker))
