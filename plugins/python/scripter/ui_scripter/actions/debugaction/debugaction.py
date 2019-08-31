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
from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon, QPixmap, QKeySequence
from scripter import resources_rc
from PyQt5.QtCore import Qt
import krita


class DebugAction(QAction):

    def __init__(self, scripter, parent=None):
        super(DebugAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.debug)

        self.setText(i18n("Debug"))
        self.setToolTip(i18n("Debug Ctrl+D"))
        self.setIcon(QIcon(':/icons/debug.svg'))
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_D))

    @property
    def parent(self):
        return 'toolBar',

    def debug(self):
        if self.scripter.uicontroller.invokeAction('save'):
            self.scripter.uicontroller.setActiveWidget(i18n('Debugger'))
            self.scripter.debugcontroller.start(self.scripter.documentcontroller.activeDocument)
            widget = self.scripter.uicontroller.findTabWidget(i18n('Debugger'))
            widget.startDebugger()
