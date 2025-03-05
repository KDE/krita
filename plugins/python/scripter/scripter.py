"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
try:
    from PyQt6.QtCore import QSettings, QStandardPaths
except:
    from PyQt5.QtCore import QSettings, QStandardPaths
from krita import Krita, Extension
from . import uicontroller, documentcontroller, debugcontroller


class ScripterExtension(Extension):

    def __init__(self, parent):
        super(ScripterExtension, self).__init__(parent)

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction("python_scripter", i18n("Scripter"))
        action.triggered.connect(self.initialize)

    def initialize(self):
        configPath = QStandardPaths.writableLocation(QStandardPaths.StandardLocation.GenericConfigLocation)
        self.settings = QSettings(configPath + '/krita-scripterrc', QSettings.Format.IniFormat)
        self.uicontroller = uicontroller.UIController()
        self.documentcontroller = documentcontroller.DocumentController()
        self.debugcontroller = debugcontroller.DebugController(self)
        self.uicontroller.initialize(self)


Krita.instance().addExtension(ScripterExtension(Krita.instance()))
