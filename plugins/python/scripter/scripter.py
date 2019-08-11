"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
"""
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
        configPath = QStandardPaths.writableLocation(QStandardPaths.GenericConfigLocation)
        self.settings = QSettings(configPath + '/krita-scripterrc', QSettings.IniFormat)
        self.uicontroller = uicontroller.UIController()
        self.documentcontroller = documentcontroller.DocumentController()
        self.debugcontroller = debugcontroller.DebugController(self)
        self.uicontroller.initialize(self)


Krita.instance().addExtension(ScripterExtension(Krita.instance()))
