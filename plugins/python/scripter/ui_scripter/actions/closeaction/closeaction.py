"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt
import krita


class CloseAction(QAction):

    def __init__(self, scripter, parent=None):
        super(CloseAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.close)

        self.setText(i18n("Close"))
        self.setObjectName('close')
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_Q))

    @property
    def parent(self):
        return 'File',

    def close(self):
        msgBox = QMessageBox(self.scripter.uicontroller.mainWidget)

        msgBox.setInformativeText(i18n("Do you want to save the current document?"))
        msgBox.setStandardButtons(QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        msgBox.setDefaultButton(QMessageBox.Save)

        ret = msgBox.exec_()

        if ret == QMessageBox.Cancel:
            return
        if ret == QMessageBox.Save:
            if not self.scripter.uicontroller.invokeAction('save'):
                return

        self.scripter.uicontroller.closeScripter()
