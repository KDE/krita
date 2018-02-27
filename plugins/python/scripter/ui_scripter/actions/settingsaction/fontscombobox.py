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
from PyQt5.QtWidgets import QComboBox, QCompleter
from PyQt5.QtGui import QFontDatabase
from PyQt5.QtCore import Qt


class FontsComboBox(QComboBox):

    def __init__(self, editor, parent=None):
        super(FontsComboBox, self).__init__(parent)

        self.editor = editor

        _fontDataBase = QFontDatabase()

        self.addItems(_fontDataBase.families())
        self.setCurrentIndex(self.findText(self.editor.font))

        com = QCompleter()
        com.setCaseSensitivity(Qt.CaseInsensitive)
        com.setCompletionMode(QCompleter.PopupCompletion)

        # Style sheet to set false on combobox-popup
        self.setStyleSheet("QComboBox { combobox-popup: 0; }")
        self.setMaxVisibleItems(10)
        self.setCompleter(com)
        self.currentIndexChanged.connect(self._currentIndexChanged)

    def _currentIndexChanged(self, index):
        self.editor.font = self.itemText(index)

    def readSettings(self, settings):
        fontName = settings.value('fontName', '')

        if fontName:
            self.setCurrentIndex(self.findText(fontName))

    def writeSettings(self, settings):
        settings.setValue('fontName', self.editor.font)
