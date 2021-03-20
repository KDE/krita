"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
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
