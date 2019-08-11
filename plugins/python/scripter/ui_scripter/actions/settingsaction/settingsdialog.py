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
