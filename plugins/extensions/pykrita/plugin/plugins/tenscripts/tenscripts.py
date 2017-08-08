from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QListView, QFormLayout,
                             QHBoxLayout, QPushButton, QLineEdit, QListWidget,
                             QScrollArea, QGridLayout, QFileDialog, QKeySequenceEdit,
                             QLabel, QAction, QDialogButtonBox)
from PyQt5.QtCore import QObject, Qt
import krita
from tenscripts import uitenscripts


class TenScriptsExtension(krita.Extension):

    def __init__(self, parent):
       super(TenScriptsExtension, self).__init__(parent)

       self.actions = []
       self.scripts = []

    def setup(self):
        action = Application.createAction("ten_scripts", "Ten Scripts")
        action.setToolTip("Assign ten scripts to ten shortcuts.")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uitenscripts = uitenscripts.UITenScripts()
        self.uitenscripts.initialize(self)

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


Scripter.addExtension(TenScriptsExtension(Application))
