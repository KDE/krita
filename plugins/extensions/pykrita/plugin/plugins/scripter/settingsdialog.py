# -*- coding: utf-8 -*-

from PyQt5.QtWidgets import *
from scripter import syntaxstylescombobox


class SettingsDialog(QDialog):

    def __init__(self, scripter, parent=None):
        super(SettingsDialog, self).__init__(parent)

        self.scripter = scripter
        self.mainLayout = QFormLayout(self)
        self.mainLayout.addRow('Syntax Highlither', syntaxstylescombobox.SyntaxStylesComboBox(scripter.highlight))
        self.mainLayout.addRow('Fonts', QComboBox())
        self.mainLayout.addRow('BackGoud Color', QComboBox())
