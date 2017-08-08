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

        self.loadActions()


    def initialize(self):
        self.uitenscripts = uitenscripts.UITenScripts()
        self.uitenscripts.initialize(self)

    def _executeScript(self):
        print('script executed')

    def readSettings(self):
        self.scripts = Application.readSetting("tenscripts", "scripts", "").split(',')

    def writeSettings(self):
        self.actions = []
        shortcuts = []
        scripts = []
        index = 0

        for row in range(self.scriptsLayout.rowCount()-1):
            shortcutWidget = self.scriptsLayout.itemAt(index).widget()
            textField = self.scriptsLayout.itemAt(index + 1).widget()

            if shortcutWidget.keySequence() and textField.text():
                action = Application.createAction("execute_script_" + str(row), "Execute Script " + str(row))
                action.setMenu("None")
                action.triggered.connect(self._executeScript)

                if index < len(self.scripts) and self.selectedPresets[index] in allPresets:
                    action.preset = self.selectedPresets[index]
                else:
                    action.preset = None

                self.actions.append(action)
                action.setShortcut("CTRL+SHIFT+1")
                action.script = textField.text()

                shortcuts.append(shortcutWidget.keySequence().toString())
                scripts.append(textField.text())
                self.actions.append(action)

            index += 3

        Application.writeSetting("tenscripts", "scripts", ','.join(map(str, scripts)))

    def loadActions(self):
        for index, item in enumerate(['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']):
            action = Application.createAction("execute_script_" + item, "Execute Script " + item)
            action.setMenu("None")
            action.script = None
            action.triggered.connect(self._executeScript)

            if index < len(self.scripts):
                action.script = self.scripts[index]

            self.actions.append(action)            

Scripter.addExtension(TenScriptsExtension(Application))
