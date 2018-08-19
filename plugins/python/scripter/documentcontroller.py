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
from .document_scripter import document
from PyQt5.QtCore import QFileSystemWatcher, QFileInfo


class DocumentController(object):

    def __init__(self):
        self._activeDocument = None
        self._fileWatcher = QFileSystemWatcher()

        self._fileWatcher.fileChanged.connect(self._changedFile)

    @property
    def activeDocument(self):
        return self._activeDocument

    def openDocument(self, filePath):
        if filePath:
            if self._activeDocument:
                old_path = self._activeDocument.path
                self._fileWatcher.removePath(old_path)

            newDocument = document.Document(filePath)
            newDocument.open()
            self._activeDocument = newDocument

            self._fileWatcher.addPath(filePath)

            return newDocument

    def saveDocument(self, data, filePath, save_as=False):
        """
        data - data to be written
        filePath - file path to write data to
        save_as = boolean, is this call made from save_as functionality. If so, do not compare data
        against existing document before save.
        """

        if save_as or not self._activeDocument:
            self._activeDocument = document.Document(filePath)

        text = str(data)
        if save_as or not self._activeDocument.compare(text):
            # compare is not evaluated if save_as is True
            self._activeDocument.data = text
            self._activeDocument.save()

        return self._activeDocument

    def clearActiveDocument(self):
        self._activeDocument = None

    def _changedFile(self, path):
        fileSystemDocument = document.Document(path)
        fileSystemDocument.open()

        if not self._activeDocument.compare(fileSystemDocument.data):
            print('file {0} changed'.format(path))

        if QFileInfo(path).exists():
            self._fileWatcher.addPath(path)
