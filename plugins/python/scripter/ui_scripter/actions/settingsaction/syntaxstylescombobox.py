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
from PyQt5.QtWidgets import QComboBox
from PyQt5.QtGui import QPalette
from scripter.ui_scripter.syntax import syntaxstyles


class SyntaxStylesComboBox(QComboBox):

    def __init__(self, highlight, editor, parent=None):
        super(SyntaxStylesComboBox, self).__init__(parent)

        self.highlight = highlight
        self.editor = editor
        self.styleClasses = [syntaxstyles.DefaultSyntaxStyle, syntaxstyles.PythonVimSyntaxStyle, syntaxstyles.BreezeLightSyntaxStyle, syntaxstyles.BreezeDarkSyntaxStyle, syntaxstyles.BlenderSyntaxStyle, syntaxstyles.SolarizedDarkSyntaxStyle, syntaxstyles.SolarizedLightSyntaxStyle]

        for styleClass in self.styleClasses:
            className = styleClass.__name__
            self.addItem(className)

            if isinstance(self.highlight.getSyntaxStyle(), styleClass):
                self.setCurrentIndex(self.findText(className))

        self.currentIndexChanged.connect(self._currentIndexChanged)

    def _currentIndexChanged(self, index):
        syntaxStyle = getattr(syntaxstyles, self.itemText(index))()
        self.highlight.setSyntaxStyle(syntaxStyle)
        self.highlight.rehighlight()
        p = self.editor.palette()
        p.setColor(QPalette.Base, syntaxStyle['background'].foreground().color())
        p.setColor(QPalette.Text, syntaxStyle['foreground'].foreground().color())
        self.editor.setPalette(p)
        self.editor.highlightCurrentLine()

    def readSettings(self, settings):
        syntaxStyle = settings.value('syntaxStyle', '')

        if syntaxStyle:
            self.setCurrentIndex(self.findText(syntaxStyle))

    def writeSettings(self, settings):
        settings.setValue('syntaxStyle', type(self.highlight.getSyntaxStyle()).__name__)
