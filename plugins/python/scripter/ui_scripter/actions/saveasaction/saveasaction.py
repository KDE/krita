"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtWidgets import QAction, QFileDialog
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt
import krita


class SaveAsAction(QAction):

    def __init__(self, scripter, parent=None):
        super(SaveAsAction, self).__init__(parent)
        self.scripter = scripter
        self.editor = self.scripter.uicontroller.editor

        self.triggered.connect(self.save)

        self.setText(i18n("Save As"))
        self.setObjectName('saveas')
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.SHIFT + Qt.Key_S))

    @property
    def parent(self):
        return 'File',

    def save(self):
        text = self.editor.toPlainText()

        fileName = QFileDialog.getSaveFileName(self.scripter.uicontroller.mainWidget,
                                               i18n("Save Python File"), '',
                                               i18n("Python File (*.py)"))[0]
        if not fileName:
            return

        # don't validate file name - trust user to specify the extension they want
        # getSaveFileName will add ".py" if there is no extension.
        # It will strip a trailing period and, in each case,  test for file collisions

        document = self.scripter.documentcontroller.saveDocument(text, fileName, save_as=True)
        if document:
            self.scripter.uicontroller.setStatusBar(document.filePath)
        else:
            self.scripter.uicontroller.setStatusBar('untitled')
        self.editor._documentModified = False
        self.scripter.uicontroller.setStatusModified()
        return document
