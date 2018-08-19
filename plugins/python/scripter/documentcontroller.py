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


class DocumentController(object):

    def __init__(self):
        self._activeDocument = None

    @property
    def activeDocument(self):
        return self._activeDocument

    def openDocument(self, filePath):
        if filePath:
            newDocument = document.Document(filePath)
            newDocument.open()
            self._activeDocument = newDocument
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
