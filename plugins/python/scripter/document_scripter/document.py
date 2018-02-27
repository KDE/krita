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

class Document(object):

    def __init__(self, filePath):
        self._document = []
        self._filePath = filePath

    def open(self, filePath=''):
        if filePath:
            self._filePath = filePath

        with open(self._filePath, 'r') as pythonFile:
            self._document = pythonFile.read()

    def save(self):
        with open(self._filePath, 'w') as pythonFile:
            pythonFile.write(self._document)

    def compare(self, new_doc):
        if len(self._document) != len(new_doc):
            return False

        if new_doc != self._document:
            return False

        return True

    @property
    def data(self):
        return self._document

    @data.setter
    def data(self, data):
        self._document = data

    @property
    def filePath(self):
        return self._filePath
