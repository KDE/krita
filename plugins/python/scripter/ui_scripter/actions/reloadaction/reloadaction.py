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
from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt
import krita


class ReloadAction(QAction):

    def __init__(self, scripter, parent=None):
        super(ReloadAction, self).__init__(parent)
        self.scripter = scripter
        self.editor = self.scripter.uicontroller.editor

        self.triggered.connect(self.reloadFile)

        self.setText(i18n("Reload File"))
        self.setObjectName('reloadfile')
        self.setShortcut(QKeySequence(Qt.ALT + Qt.Key_R))
        self.setIcon(Krita.instance().icon("view-refresh"))        

    @property
    def parents(self):
        return ('File', 'toolBar')

    def reloadFile(self):
        # get the currently open document's path
        curr_doc_fpath = ''
        document = self.scripter.documentcontroller._activeDocument
        if document is None:
            QMessageBox.critical(self.scripter.uicontroller.mainWidget,
                                 i18n("No existing document"),
                                 i18n("Please specify a document by opening it before reloading"))
            return
        else:
            curr_doc_fpath = document.filePath

        # clear the editor
        self.scripter.documentcontroller.clearActiveDocument()
        self.scripter.uicontroller.setStatusBar()
        self.scripter.uicontroller.clearEditor()

        # reload the document
        document = self.scripter.documentcontroller.openDocument(curr_doc_fpath)
        self.scripter.uicontroller.setDocumentEditor(document)
        self.scripter.uicontroller.setStatusBar(document.filePath)

        return document
