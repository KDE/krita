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
