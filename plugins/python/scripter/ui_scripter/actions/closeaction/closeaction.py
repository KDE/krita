"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
try:
    from PyQt6.QtWidgets import QMessageBox
    from PyQt6.QtGui import QKeySequence, QAction
    from PyQt6.QtCore import Qt
except:
    from PyQt5.QtWidgets import QAction, QMessageBox
    from PyQt5.QtGui import QKeySequence
    from PyQt5.QtCore import Qt

from builtins import i18n

class CloseAction(QAction):

    def __init__(self, scripter, parent=None):
        super(CloseAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.close)

        self.setText(i18n("Close"))
        self.setObjectName('close')
        self.setShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_Q))

    @property
    def parent(self):
        return 'File',

    def close(self):
        msgBox = QMessageBox(self.scripter.uicontroller.mainWidget)

        msgBox.setInformativeText(i18n("Do you want to save the current document?"))
        msgBox.setStandardButtons(QMessageBox.StandardButton.Save | QMessageBox.StandardButton.Discard | QMessageBox.StandardButton.Cancel)
        msgBox.setDefaultButton(QMessageBox.StandardButton.Save)

        ret = msgBox.exec()

        if ret == QMessageBox.StandardButton.Cancel:
            return
        if ret == QMessageBox.StandardButton.Save:
            if not self.scripter.uicontroller.invokeAction('save'):
                return

        self.scripter.uicontroller.closeScripter()
