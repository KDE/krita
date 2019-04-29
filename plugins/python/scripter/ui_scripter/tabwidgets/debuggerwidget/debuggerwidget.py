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
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QToolBar, QTableWidget, QAction
from . import stepaction, stopaction, debuggertable


class DebuggerWidget(QWidget):

    def __init__(self, scripter, parent=None):
        super(DebuggerWidget, self).__init__(parent)

        self.scripter = scripter
        self.setObjectName(i18n('Debugger'))
        self.layout = QVBoxLayout()

        self.stopAction = stopaction.StopAction(self.scripter, self)
        self.toolbar = QToolBar()
        self.stepAction = stepaction.StepAction(self.scripter, self)
        self.toolbar.addAction(self.stopAction)
        self.toolbar.addAction(self.stepAction)
        self.disableToolbar(True)

        self.table = debuggertable.DebuggerTable()

        self.layout.addWidget(self.toolbar)
        self.layout.addWidget(self.table)
        self.setLayout(self.layout)

    def startDebugger(self):
        self.disableToolbar(False)

    def disableToolbar(self, status):
        for action in self.toolbar.actions():
            action.setDisabled(status)

    def updateWidget(self):
        data = self.scripter.debugcontroller.debuggerData
        self.table.updateTable(data)
