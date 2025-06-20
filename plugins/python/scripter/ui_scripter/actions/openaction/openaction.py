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
from krita import FileDialog
from builtins import i18n

import os.path


class OpenAction(QAction):

    def __init__(self, scripter, parent=None):
        super(OpenAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.open)

        self.setText(i18n("Open"))
        self.setObjectName('open')
        self.setShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_O))

    @property
    def parent(self):
        return 'File',

    def open(self):
        dialog = FileDialog(self.scripter.uicontroller.mainWidget)
        dialog.setNameFilter((i18n("Python Files") + " (*.py)"))
        selectedFile = dialog.filename()
        if selectedFile:
            try:
                _, fileExtension = os.path.splitext(selectedFile)

                if fileExtension == '.py':
                    document = self.scripter.documentcontroller.openDocument(selectedFile)
                    self.scripter.uicontroller.setDocumentEditor(document)
                    self.scripter.uicontroller.setStatusBar(document.filePath)
                else:
                    raise
            except Exception:
                QMessageBox.information(self.scripter.uicontroller.mainWidget,
                                        i18n("Invalid File"),
                                        i18n("Open files with .py extension"))
