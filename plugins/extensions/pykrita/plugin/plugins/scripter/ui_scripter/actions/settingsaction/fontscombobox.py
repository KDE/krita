from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *


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
