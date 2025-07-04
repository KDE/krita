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


class NewAction(QAction):

    def __init__(self, scripter, parent=None):
        super(NewAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.new)

        self.setText(i18n("New"))
        self.setObjectName('new')
        self.setShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_N))

    @property
    def parent(self):
        return 'File',

    def new(self):
        msgBox = QMessageBox(self.scripter.uicontroller.mainWidget)

        msgBox.setText(i18n("The document has been modified."))
        msgBox.setInformativeText(i18n("Do you want to save your changes?"))
        msgBox.setStandardButtons(QMessageBox.StandardButton.Save | QMessageBox.StandardButton.Discard | QMessageBox.StandardButton.Cancel)
        msgBox.setDefaultButton(QMessageBox.StandardButton.Save)

        ret = msgBox.exec()

        if ret == QMessageBox.StandardButton.Cancel:
            return
        if ret == QMessageBox.StandardButton.Save:
            self.scripter.uicontroller.invokeAction('save')

        self.scripter.documentcontroller.clearActiveDocument()
        self.scripter.uicontroller.setStatusBar()
        self.scripter.uicontroller.clearEditor()
