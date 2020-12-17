"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
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
