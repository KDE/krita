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

        self.readSettings()
        self.loadActions()

    def initialize(self):
        self.uitenscripts = uitenscripts.UITenScripts()
        self.uitenscripts.initialize(self)

    def _executeScript(self):
        print('script executed')

    def readSettings(self):
        self.scripts = Application.readSetting("tenscripts", "scripts", "").split(',')

    def writeSettings(self):
        saved_scripts = self.uitenscripts.saved_scripts()

        for index, script in enumerate(saved_scripts):
            self.actions[index].script = script

        Application.writeSetting("tenscripts", "scripts", ','.join(map(str, saved_scripts)))

    def loadActions(self):
        for index, item in enumerate(['1', '2', '3', '4', '5', '6', '7', '8', '9', '10']):
            action = Application.createAction("execute_script_" + item, "Execute Script " + item)
            action.setMenu("None")
            action.script = None
            action.triggered.connect(self._executeScript)

            if index < len(self.scripts):
                action.script = self.scripts[index]

            self.actions.append(action)
            print(action.shortcut())

Scripter.addExtension(TenScriptsExtension(Application))
