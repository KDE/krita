"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtWidgets import QDialog, QFormLayout
from . import syntaxstylescombobox, fontscombobox
import krita


class SettingsDialog(QDialog):

    def __init__(self, scripter, parent=None):
        super(SettingsDialog, self).__init__(parent)

        self.scripter = scripter
        self.setWindowTitle(i18n("Settings"))
        self.mainLayout = QFormLayout(self)
        self.mainLayout.addRow(i18n("Syntax highlighter:"), syntaxstylescombobox.SyntaxStylesComboBox(self.scripter.uicontroller.highlight, self.scripter.uicontroller.editor))
        self.mainLayout.addRow(i18n("Fonts:"), fontscombobox.FontsComboBox(self.scripter.uicontroller.editor))

    def readSettings(self, settings):
        for index in range(self.mainLayout.rowCount()):
            widget = self.mainLayout.itemAt(index, QFormLayout.FieldRole).widget()
            widget.readSettings(settings)

    def writeSettings(self, settings):
        for index in range(self.mainLayout.rowCount()):
            widget = self.mainLayout.itemAt(index, QFormLayout.FieldRole).widget()
            widget.writeSettings(settings)
