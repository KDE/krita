"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
try:
    from PyQt6.QtGui import QKeySequence, QAction
    from PyQt6.QtCore import Qt
except:
    from PyQt5.QtWidgets import QAction
    from PyQt5.QtGui import QKeySequence
    from PyQt5.QtCore import Qt
from krita import utils
from builtins import i18n

class DebugAction(QAction):

    def __init__(self, scripter, parent=None):
        super(DebugAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.debug)

        self.setText(i18n("Debug"))
        self.setToolTip(i18n("Debug Ctrl+D"))
        self.setIcon(utils.getThemedIcon(":/icons/debug.svg"))

        self.setShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_D))

    @property
    def parent(self):
        return 'toolBar',

    def debug(self):
        if self.scripter.uicontroller.invokeAction('save'):
            self.scripter.uicontroller.setActiveWidget(i18n('Debugger'))
            self.scripter.debugcontroller.start(self.scripter.documentcontroller.activeDocument)
            widget = self.scripter.uicontroller.findTabWidget(i18n('Debugger'))
            widget.startDebugger()
