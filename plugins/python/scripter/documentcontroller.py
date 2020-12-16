"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
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
