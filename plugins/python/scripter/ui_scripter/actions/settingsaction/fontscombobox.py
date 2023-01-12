"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
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

        # Style sheet to set false on combobox-popup
        self.setMaxVisibleItems(10)
        self.setEditable(True)
        self.setInsertPolicy(QComboBox.NoInsert)
        self.currentIndexChanged.connect(self._currentIndexChanged)

    def _currentIndexChanged(self, index):
        self.editor.font = self.itemText(index)

    def readSettings(self, settings):
        fontName = settings.value('fontName', '')

        if fontName:
            self.setCurrentIndex(self.findText(fontName))

    def writeSettings(self, settings):
        settings.setValue('fontName', self.editor.font)
