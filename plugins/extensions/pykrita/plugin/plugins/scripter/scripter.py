from PyQt5.QtWidgets import QDialog
from PyQt5.QtCore import QSettings, QStandardPaths
from krita import *
from scripter import uicontroller, documentcontroller, debugcontroller


class ScripterExtension(Extension):

    def __init__(self, parent):
        super().__init__(parent)

    def setup(self):
        action = Krita.instance().createAction("python_scripter", "Scripter")
        action.triggered.connect(self.initialize)

    def initialize(self):
        configPath = QStandardPaths.writableLocation(QStandardPaths.GenericConfigLocation)
        self.settings = QSettings(configPath + '/krita-scripterrc', QSettings.IniFormat)
        self.uicontroller = uicontroller.UIController()
        self.documentcontroller = documentcontroller.DocumentController()
        self.debugcontroller = debugcontroller.DebugController(self)
        self.uicontroller.initialize(self)


Krita.instance().addExtension(ScripterExtension(Krita.instance()))
