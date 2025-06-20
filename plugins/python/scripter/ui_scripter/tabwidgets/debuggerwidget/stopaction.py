"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
try:
    from PyQt6.QtGui import QIcon, QAction
except:
    from PyQt5.QtWidgets import QAction
    from PyQt5.QtGui import QIcon
from builtins import i18n


class StopAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(StopAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.stop)

        self.setText(i18n("Stop"))
        # path to the icon
        self.setIcon(QIcon(':/icons/stop.svg'))

    def stop(self):
        self.scripter.debugcontroller.stop()
        self.toolbar.disableToolbar(True)
